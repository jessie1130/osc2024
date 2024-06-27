#include "vfs.h"
#include "tmpfs.h"
#include "memory.h"
#include "string.h"
#include "uart1.h"
#include "initramfs.h"
#include "dev_uart.h"
#include "dev_framebuffer.h"

struct mount *rootfs;
struct filesystem reg_fs[MAX_FS_REG];
struct file_operations reg_dev[MAX_DEV_REG];

// register the file system to the kernel.
// you can also initialize memory pool of the file system here.
int register_filesystem(struct filesystem *fs)
{
    for (int i = 0; i < MAX_FS_REG; i++)
    {
        if (!reg_fs[i].name)
        {
            reg_fs[i].name = fs->name;
            reg_fs[i].setup_mount = fs->setup_mount;
            return i;
        }
    }
    // full
    return -1;
}

//A device can register itself to the VFS in its setup. 
//The VFS assigns the device a unique device id. Then the device can be recognized by the VFS.
int register_dev(struct file_operations *fo)
{
    for (int i = 0; i < MAX_FS_REG; i++)
    {
        if (!reg_dev[i].open)
        {
            // return id for device
            reg_dev[i] = *fo;
            return i;
        }
    }
    return -1;
}

struct filesystem *find_filesystem(const char *fs_name)
{
    for (int i = 0; i < MAX_FS_REG; i++)
    {
        if (strcmp(reg_fs[i].name, fs_name) == 0)
        {
            return &reg_fs[i];
        }
    }
    // can't find
    return 0;
}

// 1. Lookup pathname
// 2. Create a new file handle for this vnode if found.
// 3. Create a new file if O_CREAT is specified in flags and vnode not found
// lookup error code shows if file exist or not or other error occurs
// 4. Return error code if fails
int vfs_open(const char *pathname, int flags, struct file **target)
{
    struct vnode *node;
    // 1. Lookup pathname
    // 3. Create a new file if O_CREAT is specified in flags
    if (vfs_lookup(pathname, &node) != 0 && (flags & O_CREAT))
    {
        int last_slash_idx = 0;
        for (int i = strlen(pathname) - 1; i >= 0; i--)
        {
            if (pathname[i] == '/')
            {
                last_slash_idx = i;
                break;
            }
        }
        
        char dirname[MAX_PATH_NAME + 1] = {};
        memcpy(dirname, pathname, last_slash_idx);
        // vnode not found
        // lookup error code shows if file exist or not or other error occurs
        // update dirname to node
        if (vfs_lookup(dirname, &node) != 0)
        {
            uart_sendline("vfs_open: can't ocreate\n");
            return -1;
        }
        // create a new file node on node, &node is new file, 3rd arg is filename
        node->v_ops->create(node, &node, pathname + last_slash_idx + 1);
        // create file handle
        *target = kmalloc(sizeof(struct file));
        node->f_ops->open(node, target);
        (*target)->flags = flags;
        return 0;
    }
    // 2. Create a new file handle for this vnode if found.
    else
    {
        *target = kmalloc(sizeof(struct file));
        node->f_ops->open(node, target);
        (*target)->flags = flags;
        return 0;
    }
    // 4. Return error code if fails
    return -1;
}

// 1. release the file handle
// 2. Return error code if fails
int vfs_close(struct file *file)
{
    file->f_ops->close(file);
    return 0;
}

// 1. write len byte from buf to the opened file.
// 2. return written size or error code if an error occurs.
int vfs_write(struct file *file, const void *buf, size_t len)
{
    return file->f_ops->write(file, buf, len);
}

// 1. read min(len, readable size) byte to buf from the opened file.
// 2. block if nothing to read for FIFO type
// 2. return read size or error code if an error occurs.
int vfs_read(struct file *file, void *buf, size_t len)
{
    return file->f_ops->read(file, buf, len);
}

int vfs_mkdir(const char *pathname)
{
    char dirname[MAX_PATH_NAME] = {};
    char newdirname[MAX_PATH_NAME] = {};

    int last_slash_idx = 0;
    for (int i = strlen(pathname) - 1; i >= 0; i--)
    {
        if (pathname[i] == '/')
        {
            last_slash_idx = i;
            break;
        }
    }

    //pathname 不包含最後的dir
    memcpy(dirname, pathname, last_slash_idx);
    //last dir
    strcpy(newdirname, pathname + last_slash_idx + 1);
    // create new directory if upper directory is found
    struct vnode *node;
    if (vfs_lookup(dirname, &node) == 0)
    {
        node->v_ops->mkdir(node, &node, newdirname);
        return 0;
    }

    uart_sendline("vfs_mkdir: the path not exist\n");
    return -1;
}

int vfs_mount(const char *target, const char *filesystem)
{
    struct vnode *dirnode;
    // search for the target filesystem
    struct filesystem *fs = find_filesystem(filesystem);
    if (!fs)
    {
        uart_sendline("vfs_mount: can't find filesystem\n");
        return -1;
    }
    
    if (vfs_lookup(target, &dirnode) == -1)
    {
        uart_sendline("vfs_mount: can't find dir\n");
        return -1;
    }else
    {
        // mount fs on dirnode
        dirnode->mount = kmalloc(sizeof(struct mount));
        fs->setup_mount(fs, dirnode->mount);
    }
    return 0;
}

int vfs_lookup(const char *pathname, struct vnode **target)
{
    if (strlen(pathname) == 0)
    {
        *target = rootfs->root;
        return 0;
    }

    struct vnode *dirnode = rootfs->root;
    char component_name[MAX_FILE_NAME + 1] = {};
    int c_idx = 0;
    for (int i = 1; i < strlen(pathname); i++)
    {
        if (pathname[i] == '/')
        {
            component_name[c_idx++] = 0;
            // if directory not exist
            if (dirnode->v_ops->lookup(dirnode, &dirnode, component_name) != 0)
                return -1;
            // another mounted file system
            while (dirnode->mount)
                dirnode = dirnode->mount->root;
            c_idx = 0;
        }
        else
        {
            component_name[c_idx++] = pathname[i];
        }
    }

    component_name[c_idx++] = 0;
    if (dirnode->v_ops->lookup(dirnode, &dirnode, component_name) != 0)
        return -1;

    while (dirnode->mount)
        dirnode = dirnode->mount->root;
    *target = dirnode;
    return 0;
}

//  A user can use the device id to create a device file in a file system.
int vfs_mknod(char *pathname, int id)
{
    struct file *f = kmalloc(sizeof(struct file));
    // create leaf and its file operations
    vfs_open(pathname, O_CREAT, &f);
    f->vnode->f_ops = &reg_dev[id];
    vfs_close(f);
    return 0;
}

void init_rootfs()
{
    int idx = register_tmpfs();
    rootfs = kmalloc(sizeof(struct mount));
    reg_fs[idx].setup_mount(&reg_fs[idx], rootfs);

    vfs_mkdir("/initramfs");
    register_initramfs();
    vfs_mount("/initramfs", "initramfs");

    vfs_mkdir("/dev");
    int uart_id = init_dev_uart();
    vfs_mknod("/dev/uart", uart_id);

    int framebuffer_id = init_dev_framebuffer();
    vfs_mknod("/dev/framebuffer", framebuffer_id);

}

char *get_absolute_path(char *path, char *curr_working_dir)
{
    // path = curr_working_dir/path
    // if relative path -> add root path
    if (path[0] != '/')
    {
        char tmp[MAX_PATH_NAME];
        strcpy(tmp, curr_working_dir);
        if (strcmp(curr_working_dir, "/") != 0)
            strcat(tmp, "/");
        strcat(tmp, path);
        strcpy(path, tmp);
    }
    //uart_sendline("path: %s\n", path);
    char absolute_path[MAX_PATH_NAME + 1] = {};
    int idx = 0;
    for (int i = 0; i < strlen(path); i++)
    {
        // "/.." 往前移一層directory
        if (path[i] == '/' && path[i + 1] == '.' && path[i + 2] == '.')
        {
            for (int j = idx; j >= 0; j--)
            {
                if (absolute_path[j] == '/')
                {
                    absolute_path[j] = 0;
                    idx = j;
                }
            }
            i += 2;
            continue;
        }

        // "/." ignore
        if (path[i] == '/' && path[i + 1] == '.')
        {
            i++;
            continue;
        }
        absolute_path[idx++] = path[i];
    }
    absolute_path[idx] = 0;
    //uart_sendline("abs: %s\n", absolute_path);
    return strcpy(path, absolute_path);
}