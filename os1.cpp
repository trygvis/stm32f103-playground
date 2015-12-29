#include <stdint.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>

#include "debug.h"
#include "tinyprintf.h"
#include "playground.h"

extern "C"
__attribute__((naked, used))
void HardFault_Handler_C(uint32_t *hardfault_args) {
    dbg_printf("r0 = 0x%08lx (%lu)\n", hardfault_args[0], hardfault_args[0]);
    dbg_printf("r1 = 0x%08lx (%lu)\n", hardfault_args[1], hardfault_args[1]);
    dbg_printf("r2 = 0x%08lx (%lu)\n", hardfault_args[2], hardfault_args[2]);
    dbg_printf("r3 = 0x%08lx (%lu)\n", hardfault_args[3], hardfault_args[3]);
    dbg_printf("r12 = 0x%08lx (%lu)\n", hardfault_args[4], hardfault_args[4]);
    dbg_printf("lr = 0x%08lx (%lu)\n", hardfault_args[5], hardfault_args[5]);
    dbg_printf("pc = 0x%08lx (%lu)\n", hardfault_args[6], hardfault_args[6]);
    dbg_printf("psr = 0x%08lx (%lu)\n", hardfault_args[7], hardfault_args[7]);
    dbg_printf("\n");

    halt();
}

enum class exc_return_t : uint32_t {
    RETURN_TO_HANDLER_MODE_USE_MSP = 0xFFFFFFF1,
    RETURN_TO_THREAD_MODE_USE_MSP = 0xFFFFFFF9,
    RETURN_TO_THREAD_MODE_USE_PSP = 0xFFFFFFFD,
};

// This is used from assembly so order is important
struct task_t {
    uint8_t *stack;
    exc_return_t exc_return;
    int flags;

    void init(uint8_t *stack) {
        this->stack = stack;
        flags = 0x01;
        set_ready();
    }

    void deinit() {
        flags = 0;
    }

    bool is_ready() {
        return (flags & 0x02) > 0;
    }

    void set_ready() {
        flags |= 0x02;
    }
} __attribute__((packed));

const int max_task_count = 3;
const int stack_size = 100;

task_t tasks[max_task_count];
uint8_t stacks[max_task_count][stack_size];
uint8_t task_count = 0;
int current_task;

const unsigned int SYSTICK_FREQUENCY_HZ = 10;

struct hardware_frame_t {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
};

struct software_frame_t {
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
};

extern "C"
void SysTick_Handler() {
    static bool on = true;

    if (on) {
        GPIO_SetBits(GPIOB, GPIO_Pin_7);
    }
    else {
        GPIO_ResetBits(GPIOB, GPIO_Pin_7);
    }

    on = !on;

    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void thread_end() {
}

volatile bool idle_task_run;

void idle_task(const void *const) {
    // trigger PendSV to run
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;

    // wait for the PendSV handler to kick in. After the handler has completed it will do a context switch and
    // this point shouldn't be reached.

    idle_task_run = true;
    while (idle_task_run) {
        GPIOB->BSRR = GPIO_Pin_6;
        GPIOB->BRR = GPIO_Pin_6;
    }
}

static
void create_thread(void (task)(void const *const), bool create_sw);

void osKernelInitialize() {
    NVIC_SetPriority(SysTick_IRQn, 0xff);
    NVIC_SetPriority(PendSV_IRQn, 0xff);

    SysTick_Config(SystemCoreClock / SYSTICK_FREQUENCY_HZ);

    GPIO_InitTypeDef init;
    GPIO_StructInit(&init);
    init.GPIO_Mode = GPIO_Mode_Out_PP;
    init.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOB, &init);
    init.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOB, &init);

    for (int i = 0; i < max_task_count; i++) {
        tasks[i].flags = 0;
    }

    create_thread(idle_task, false);
    current_task = 0;
}

void create_thread(void (task)(void const *const)) {
    create_thread(task, true);
}

static
void create_thread(void (task)(void const *const), bool create_sw) {
    uint8_t *s = stacks[task_count] + stack_size;

    s -= sizeof(hardware_frame_t);
    hardware_frame_t *hw = reinterpret_cast<hardware_frame_t *>(s);

    hw->r0 = 0x00000000;
    hw->r1 = 0x01010101;
    hw->r2 = 0x02020202;
    hw->r3 = 0x03030303;
    hw->r12 = 0x0c0c0c0c;
    hw->lr = reinterpret_cast<uint32_t>(thread_end);

    hw->pc = reinterpret_cast<uint32_t>(task);
    hw->psr = 0x01000000;

    if (create_sw) {
        s -= sizeof(software_frame_t);
        software_frame_t *sw = reinterpret_cast<software_frame_t *>(s);

        sw->r4 = 0x04040404;
        sw->r5 = 0x05050505;
        sw->r6 = 0x06060606;
        sw->r7 = 0x07070707;
        sw->r8 = 0x08080808;
        sw->r9 = 0x09090909;
        sw->r10 = 0x0a0a0a0a;
        sw->r11 = 0x0b0b0b0b;
    }

    task_t *t = &tasks[task_count];
    t->init(s);
    t->exc_return = exc_return_t::RETURN_TO_THREAD_MODE_USE_PSP;

    task_count++;
}

static
int find_first_ready_task() {
    task_t *t;
    int idx = current_task + 1;
    do {
        if (idx == max_task_count) {
            idx = 1;
        } else if (idx == current_task) {
            return 0;
        }

        t = &tasks[idx];
    } while (!t->is_ready());

    return idx;
}

__attribute__((used))
task_t *select_next_task(uint8_t *current_stack) {

    task_t *t = &tasks[current_task];
    int new_task = find_first_ready_task();

    if (new_task != current_task) {
        t->stack = current_stack;
        t = &tasks[new_task];
        current_task = new_task;
    }

    return t;
}

volatile bool run;

extern "C" void do_first_context_switch(uint8_t *user_stack, void (task)(const void *const arg));

void osKernelStart() {
    run = true;

    task_t &t = tasks[0];

    do_first_context_switch(t.stack, idle_task);

    while (run) {
    }
}


volatile bool run1 = true;

void job1(void const *const) {
    while (run1) {
        GPIO_SetBits(GPIOB, GPIO_Pin_8);
        GPIO_ResetBits(GPIOB, GPIO_Pin_8);
    }
}

volatile bool run2 = true;

void job2(void const *const) {
    while (run2) {
        GPIO_SetBits(GPIOB, GPIO_Pin_5);
        GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    }
}

int main(void) {
    SystemInit();

    init_printf(nullptr, dbg_putc);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO
                           | RCC_APB2Periph_USART1
                           | RCC_APB2Periph_GPIOA
                           | RCC_APB2Periph_GPIOB
                           | RCC_APB2Periph_GPIOC,
                           ENABLE);

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOB, DISABLE);

    // Make Port B's pin #5 the debug output pin
    GPIO_InitTypeDef init;
    GPIO_StructInit(&init);
    init.GPIO_Mode = GPIO_Mode_Out_PP;
    init.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_8;
    GPIO_Init(GPIOB, &init);

    osKernelInitialize();
    create_thread(job1);
    create_thread(job2);
    osKernelStart();

    return 0;
}
