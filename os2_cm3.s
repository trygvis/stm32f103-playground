.syntax unified
.cpu cortex-m3
.thumb

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
    mov     r0, #2
    msr     control, r0
    isb

    // Restore the data from hardware_frame_t.
    pop     {r0 - r3, r12, lr}
    // Pop PC and PSR. PSR is ignored, but we branch to the new PC
    pop     {r4, r5}
    bx      r4

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

    // load task_t.stack and task_t.lr into r1 and lr
    ldm     r0, {r1, lr}

    ldmia   r1!, {r4 - r11}
    msr     psp, r1

    // Return, let the CPU restore the hardware part of the context
    bx      lr
.pool
.size PendSV_Handler,.-PendSV_Handler

.end
