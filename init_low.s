.syntax unified
.cpu cortex-m3
.thumb

/* VERY significant */
.section .text

.thumb_func
.global _Reset_Handler
_Reset_Handler:
    bl init_high
    b halt

.thumb_func
.global halt
halt:
    b .

.thumb_func
NMI_Handler:
    b halt

.thumb_func
HardFault_Handler:
    tst lr, #4
    ite eq
    mrseq r0, msp
    mrsne r0, psp
    b HardFault_Handler_C

.thumb_func
MemManage_Handler:
    b halt

.thumb_func
BusFault_Handler:
    b halt

.thumb_func
UsageFault_Handler:
    b halt

.end

