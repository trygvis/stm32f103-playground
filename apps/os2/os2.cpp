#include <stdint.h>
#include <cstddef>
#include <type_traits>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <inttypes.h>

#include "debug.h"
#include "tinyprintf.h"
#include "playground.h"

namespace trygvis {
namespace os2 {

/*
 * System configuration
 */
constexpr bool enable_stack_smashing_check = false;

namespace os {

enum class exc_return_t : uint32_t {
    RETURN_TO_HANDLER_MODE_USE_MSP = 0xFFFFFFF1,
    RETURN_TO_THREAD_MODE_USE_MSP = 0xFFFFFFF9,
    RETURN_TO_THREAD_MODE_USE_PSP = 0xFFFFFFFD,
};

// This is used from assembly so order is important
struct task_t {
    uint8_t *current_stack;
    // This field is used by the assembly code.
    exc_return_t exc_return;
    uint8_t *stack_start, *stack_end;
    int flags;

    void init(uint8_t *current_stack, uint8_t *stack_start, uint8_t *stack_end) {
        this->current_stack = current_stack;
        this->stack_start = stack_start;
        this->stack_end = stack_end;
        flags = 0x01;
        set_ready();
    }

    bool is_ready() {
        return (flags & 0x02) > 0;
    }

    void set_ready() {
        flags |= 0x02;
    }

    void set_blocked() {
        flags &= ~0x02;
    }
} __attribute__((packed));

// This is required for offsetof to be defined behaviour
static_assert(std::is_standard_layout<task_t>::value, "task_t has to be is_standard_layout");
static_assert(offsetof(task_t, exc_return) == 4, "task_t::exc_return has to be at offset 0");
static_assert(offsetof(task_t, current_stack) == 0, "task_t::current_stack has to be at offset 4");

const uint8_t max_task_count = 3;
const int stack_size = 256;

static_assert(stack_size % 4 == 0, "stack_size must be word-aligned.");

task_t tasks[max_task_count];
uint8_t stacks[SizeOfArray(tasks)][stack_size];
uint8_t task_count = 0;
uint32_t current_task;

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
__attribute__((naked, used))
void HardFault_Handler_C(uint32_t *stack) {
    dbg_printf("r0  = 0x%08lx (%lu)\n", stack[0], stack[0]);
    dbg_printf("r1  = 0x%08lx (%lu)\n", stack[1], stack[1]);
    dbg_printf("r2  = 0x%08lx (%lu)\n", stack[2], stack[2]);
    dbg_printf("r3  = 0x%08lx (%lu)\n", stack[3], stack[3]);
    dbg_printf("r12 = 0x%08lx (%lu)\n", stack[4], stack[4]);
    dbg_printf("lr  = 0x%08lx (%lu)\n", stack[5], stack[5]);
    dbg_printf("pc  = 0x%08lx (%lu)\n", stack[6], stack[6]);
    dbg_printf("psr = 0x%08lx (%lu)\n", stack[7], stack[7]);
    dbg_printf("\n");

    dbg_printf("current_task = %" PRIu32 "\n", current_task);
    dbg_printf("\n");

    Default_Handler();

    halt();
}

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

extern "C"
void SVC_Handler() {
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/**
 * Implemented in assembly, simply executes an SVC instruction.
 */
__attribute__((used))
extern void reschedule();

// This doesn't quite work.
void thread_end() {
    dbg_printf("thread_end(). current_task=%" PRIu32 "\n", current_task);
}

extern "C" void asm_idle_task(const void *const);

#define idle_task asm_idle_task
volatile bool idle_task_run;

void idle_task_c(const void *const) {
    GPIOB->BSRR = GPIO_Pin_6;
    GPIOB->BRR = GPIO_Pin_6;

    // wait for the PendSV handler to kick in. After the handler has completed it will do a context switch and
    // this point shouldn't be reached.

    idle_task_run = true;
    while (idle_task_run) {
        GPIOB->BSRR = GPIO_Pin_6;
        GPIOB->BRR = GPIO_Pin_6;
    }
}

static void init_stack(uint8_t *stack, size_t stack_size) {
    static int stack_pattern = -16;

    for (size_t y = 0; y < stack_size; y++) {
        stack[y] = (uint8_t) stack_pattern;
    }

    stack_pattern -= 16;

    auto x = idle_task_c;
    (void) x;
}

//static
void os_create_thread(void (task)(void const *const), bool create_sw = true) {
    uint8_t *const stack_end = stacks[task_count];
    uint8_t *const stack_start = stack_end + stack_size;

    init_stack(stack_end, stack_size);

    uint8_t *s = stack_start;

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
    t->init(s, stack_start, stack_end);
    t->exc_return = exc_return_t::RETURN_TO_THREAD_MODE_USE_PSP;

    dbg_printf("task #%d: stack=%p -> %p, current=%p, allocated=%d\n", task_count,
               static_cast<void *>(t->stack_end), static_cast<void *>(t->stack_start),
               static_cast<void *>(t->current_stack), int(t->stack_start - t->current_stack));

    task_count++;
}

static
uint32_t find_first_ready_task() {
    task_t *t;
    // Start from the task after the current one
    uint32_t idx = current_task;
    do {
        idx++;
        // If we have checked the entire array, start from the first non-idle task
        if (idx == max_task_count) {
            idx = 1;
        }

        t = &tasks[idx];
        if (idx == current_task) {
            // If we have checked all other tasks, use the current one if it is still ready. if not, use the idle task
            if (!t->is_ready()) {
                idx = 0;
            }
            break;
        }

    } while (!t->is_ready());

    return idx;
}

template<bool enable>
void check_stack_smashing(task_t *);

template<>
void check_stack_smashing<false>(task_t *) {
}

// TODO: this implementation hasn't been checked.
template<>
void check_stack_smashing<true>(task_t *t) {
    if (t->current_stack < t->stack_end) {
        dbg_printf("STACK SMASHED: task #%" PRIu32 ", end=%p, current=%p\n", current_task,
                   static_cast<void *>(t->current_stack), static_cast<void *>(t->current_stack));
    }
}

__attribute__((used))
task_t *select_next_task(uint8_t *current_stack) {

    task_t *t = &tasks[current_task];
    t->current_stack = current_stack;

    check_stack_smashing<enable_stack_smashing_check>(t);

    uint32_t new_task = find_first_ready_task();

    if (new_task != current_task) {
        t = &tasks[new_task];
        current_task = new_task;
    }

    return t;
}

volatile bool run;

extern "C" void do_first_context_switch(uint8_t *user_stack, void (task)(const void *const arg));

void os_init() {
    NVIC_SetPriority(SysTick_IRQn, 0xff);
    NVIC_SetPriority(PendSV_IRQn, 0xff);

    SysTick_Config(SystemCoreClock / SYSTICK_FREQUENCY_HZ);

    GPIO_InitTypeDef init;
    GPIO_StructInit(&init);
    init.GPIO_Mode = GPIO_Mode_Out_PP;
    init.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &init);

    for (size_t i = 0; i < SizeOfArray(tasks); i++) {
        tasks[i].flags = 0;
    }

    os_create_thread(idle_task, false);
    current_task = 0;
}

void os_start() {
    run = true;

    task_t &t = tasks[0];

    // TODO: this should jump to runnable user task if available
    do_first_context_switch(t.current_stack, idle_task);

    while (run) {
    }
}

//class DisabledInterrupts {
//public:
//    DisabledInterrupts() : primask(__get_PRIMASK()) {
//        __disable_irq();
//    }
//
//    ~DisabledInterrupts() {
//        __set_PRIMASK(primask);
//    }
//
//private:
//    uint32_t primask;
//};

class Mutex final {
public:
    Mutex() : owner(UINT32_MAX) {
    }

    void lock() {
        uint32_t old;

        do {
            // read the semaphore value
            old = __LDREXW(&owner);
            // loop again if it is locked and we are blocking
            // or setting it with strex failed
        } while ((old == current_task) || __STREXW(current_task, &owner) != 0);
    }

    void unlock() {
        owner = UINT32_MAX;
    }

private:
    uint32_t owner;
};

class LockMutex {
public:
    LockMutex(Mutex &mutex) : mutex(mutex) {
        mutex.lock();
    }

    ~LockMutex() {
        mutex.unlock();
    }

private:
    Mutex &mutex;
};

} // namespace os

namespace app {

namespace os = trygvis::os2::os;

volatile bool run1 = true;

os::Mutex mutex;

void job1(void const *const) {
    GPIO_InitTypeDef init;
    GPIO_StructInit(&init);
    init.GPIO_Mode = GPIO_Mode_Out_PP;
    init.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOB, &init);

    while (run1) {
        os::LockMutex ms(mutex);
        GPIOB->BSRR = GPIO_Pin_8;
        GPIOB->BRR = GPIO_Pin_8;
    }
}

volatile bool run2 = true;

void job2(void const *const) {
    GPIO_InitTypeDef init;
    GPIO_StructInit(&init);
    init.GPIO_Mode = GPIO_Mode_Out_PP;
    init.GPIO_Pin = GPIO_Pin_5;
    GPIO_Init(GPIOB, &init);

    while (run2) {
        os::LockMutex ms(mutex);
        GPIOB->BSRR = GPIO_Pin_5;
        GPIOB->BRR = GPIO_Pin_5;
    }
}

extern "C"
int main(void) {
    SystemInit();

    init_printf(nullptr, dbg_putc);
    dbg_printf("os2\n");

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO
                           | RCC_APB2Periph_USART1
                           | RCC_APB2Periph_GPIOA
                           | RCC_APB2Periph_GPIOB
                           | RCC_APB2Periph_GPIOC,
                           ENABLE);

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOB, DISABLE);

    os::os_init();
    os::os_create_thread(job1);
    os::os_create_thread(job2);
    os::os_start();

    return 0;
}

} // namespace app
} // namespace os2
} // namespace trygvis
