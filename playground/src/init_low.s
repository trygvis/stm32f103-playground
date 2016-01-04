.syntax unified
.cpu cortex-m3
.thumb

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
.global NMI_Handler
NMI_Handler:
    b halt

.thumb_func
.global HardFault_Handler
HardFault_Handler:
    tst lr, #4
    ite eq
    mrseq r0, msp
    mrsne r0, psp
    b HardFault_Handler_C

.thumb_func
.global MemManage_Handler
MemManage_Handler:
    b halt

.thumb_func
.global BusFault_Handler
BusFault_Handler:
    b halt

.thumb_func
.global UsageFault_Handler
UsageFault_Handler:
    b halt

.end
