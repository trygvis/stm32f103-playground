.syntax unified
.cpu cortex-m3
.thumb

.equ PERIPH_BASE            , 0x40000000
.equ APB2PERIPH_BASE        , PERIPH_BASE + 0x10000
.equ GPIOB_BASE             , APB2PERIPH_BASE + 0x0C00

.equ GPIO_BSRR_OFF          , 0x10
.equ GPIO_BSR_OFF           , 0x14

# .equ GPIOB 0x40010c00

.section .text

/*

User threads use the process stack (PSP register), kernel and exception code use the main stack (MSP register).

*/

.thumb_func
.global do_first_context_switch
// void do_first_context_switch(uint8_t *user_stack, void (task)(const void *arg));
do_first_context_switch:
    /* Set PSP to the user task's stack */
    msr     psp, r0

    // Set CONTROL.SPSEL=1 so that we run with two stacks
    mrs     r0, control
    orr     r0, #2
    msr     control, r0
    isb

    // Set CONTROL.nPRIV=1 so that we run with in unprivileged mode
    mrs     r0, control
    orr     r0, #1
    msr     control, r0
    isb

    // Restore the data from hardware_frame_t.
    pop     {r0 - r3, r12, lr}
    // Pop PC and PSR. PSR is ignored, but we branch to the new PC
    pop     {r4, r5}
    bx      r4

.thumb_func
.global _ZN7trygvis3os22os10rescheduleEv
_ZN7trygvis3os22os10rescheduleEv:
    svc     #0
    bx      lr

// A very simple idle task, just to know exactly which registers that are used and what their values are supposed to be.
// Toggles GPIO B, pin #6 on a STM32F103
.thumb_func
.global asm_idle_task
asm_idle_task:
    ldr     r0, =0x0020
    ldr     r0, =0xffff
    ldr     r1, =GPIOB_BASE
asm_idle_task_loop:
    str     r0, [r1, #GPIO_BSRR_OFF]
    str     r0, [r1, #GPIO_BSR_OFF]
    b       asm_idle_task_loop
.pool
.size asm_idle_task,.-asm_idle_task

/*
When this function is executed {r0-r3,r12,lr,pc} has been pushed to the stack pointed to by PSP. We push the rest of the
registers to the PSP. The current stack pointer is the MSP.
 */
.thumb_func
.global PendSV_Handler
PendSV_Handler:
    // Save the rest of the context to the current process' stack
    mrs     r0, psp
    stmdb   r0!, {r4 - r11}

    // Call select_next_task_sp. after return, r0 points to a task_t
    bl      _ZN7trygvis3os22os16select_next_taskEPh   // task_t *select_next_task(uint8_t *current_stack)

    // load task_t.exc_return and task_t.current_stack into r1 and lr
    ldm     r0, {r1, lr}

    // Restore the registers saved on the thread's stack
    ldmia   r1!, {r4 - r11}
    msr     psp, r1

    // Return, let the CPU restore the hardware part of the context
    bx      lr
.pool
.size PendSV_Handler,.-PendSV_Handler

.end
