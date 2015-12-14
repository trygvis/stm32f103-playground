/*
https://github.com/dwelch67/stm32_samples
http://stackoverflow.com/questions/9565921/cortex-m3-initialisation
*/

.syntax unified
.cpu cortex-m3
.thumb

.section .isr_vectors

.global vectors
vectors:
stacktop: .word 0x20001000
.word _Reset_Handler
.word NMI_Handler
.word HardFault_Handler
.word MemManage_Handler
.word BusFault_Handler
.word UsageFault_Handler
.word hang2
.word hang2
.word hang2
.word hang2
.word hang2
.word hang2
.word hang2
.word hang2
.word hang2

/* VERY significant */
.section .text

.thumb_func
.global _Reset_Handler
_Reset_Handler:
    bl main
    b hang2

.thumb_func
hang2:
    b .

.thumb_func
NMI_Handler:
    b hang2

.thumb_func
HardFault_Handler:
    tst lr, #4
    ite eq
    mrseq r0, msp
    mrsne r0, psp
    b HardFault_Handler_C

.thumb_func
MemManage_Handler:
    b hang2

.thumb_func
BusFault_Handler:
    b hang2

.thumb_func
UsageFault_Handler:
    b hang2

.end
