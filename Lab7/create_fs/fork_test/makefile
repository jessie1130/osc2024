SRCS 			= $(wildcard ./*.c)
OBJS			= $(SRCS:%.c=%.o)

PROGRAM			= fork_test
ELF				= ./$(PROGRAM).elf
IMG				= ../$(PROGRAM).img

CFLAGS 			= -Wall -ffreestanding -nostdlib
CC 				= aarch64-linux-gnu-gcc
LINKER 			= aarch64-linux-gnu-ld
OBJ_COPY 		= aarch64-linux-gnu-objcopy

.PHONY: 		clean

all:			$(IMG)

$(IMG):			object/$(OBJS) object/syscall.o
	$(LINKER) -nostdlib object/$(OBJS) object/syscall.o -T linker.ld -o $(ELF)
	$(OBJ_COPY) -O binary $(ELF) $@

object/syscall.o:	syscall.S
	$(CC) $(CFLAGS) -c $< -o $@

object/%.o: 		%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm object/$(OBJS) object/syscall.o $(ELF) $(IMG) >/dev/null 2>/dev/null || true
	