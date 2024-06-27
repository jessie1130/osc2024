#ifndef	_EXCEPTION_H_
#define	_EXCEPTION_H_

#include "bcm2837/rpi_irq.h"

#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int *)(PHYS_TO_VIRT(0x40000060)))

#define IRQ_PENDING_1_AUX_INT (1<<29)
#define INTERRUPT_SOURCE_GPU (1<<8)
#define INTERRUPT_SOURCE_CNTPNSIRQ (1<<1)

typedef struct trapframe
{
    unsigned long x0;
    unsigned long x1;
    unsigned long x2;
    unsigned long x3;
    unsigned long x4;
    unsigned long x5;
    unsigned long x6;
    unsigned long x7;
    unsigned long x8;
    unsigned long x9;
    unsigned long x10;
    unsigned long x11;
    unsigned long x12;
    unsigned long x13;
    unsigned long x14;
    unsigned long x15;
    unsigned long x16;
    unsigned long x17;
    unsigned long x18;
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long x29;
    unsigned long x30;
    unsigned long spsr_el1;
    unsigned long elr_el1;
    unsigned long sp_el0;

} trapframe_t;

//https://developer.arm.com/documentation/ddi0595/2020-12/AArch64-Registers/ESR-EL1--Exception-Syndrome-Register--EL1-?lang=en#fieldset_0-25_25
#define MEMFAIL_DATA_ABORT_LOWER 0b100100 // esr_el1    Data Abort from a lower Exception level.
#define MEMFAIL_INST_ABORT_LOWER 0b100000 // EC, bits [31:26]   Instruction Abort from a lower Exception level.

#define TF_LEVEL0 0b000100 // iss DFSC, bits [5:0] Data Fault Status Code.
#define TF_LEVEL1 0b000101  // IFSC, bits [5:0] Instruction Fault Status Code.
#define TF_LEVEL2 0b000110
#define TF_LEVEL3 0b000111

typedef struct{
    unsigned int iss : 25, // Instruction specific syndrome [0:24]
                 il : 1,   // Instruction length bit [25]
                 ec : 6;   // Exception class [26:31]
} esr_el1_t;




void irq_router(trapframe_t *tp);
void svc_router(trapframe_t *tp);

void invalid_exception_router(unsigned long long x0); // exception_handler.S

static inline void enable_irq()
{
    __asm__ __volatile__("msr daifclr, 0xf");
}

static inline void disable_irq()
{
    __asm__ __volatile__("msr daifset, 0xf");
}

void lock();
void unlock();
#endif /*_EXCEPTION_H_*/
