#include <stdint.h>
#include <stdint-gcc.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stddef.h>

int init_high();

extern "C" void halt();

#include "tmp/printf/printf.h"

#include "stm32f10x_conf.h"

extern "C"
__attribute__((naked))
void HardFault_Handler_C(uint32_t *hardfault_args);

extern "C" void high();
extern "C" void low();

SCB_Type *__SCB = ((SCB_Type *) SCB_BASE);
//extern SCB_Type *__SCB;

struct hardfault_data_t {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
};

volatile struct hardfault_data_t *hardfault_data = (volatile struct hardfault_data_t *) 0x20000800;

void HardFault_Handler_C(uint32_t *hardfault_args) {
    hardfault_data->r0 = hardfault_args[0];
    hardfault_data->r1 = hardfault_args[1];
    hardfault_data->r2 = hardfault_args[2];
    hardfault_data->r3 = hardfault_args[3];
    hardfault_data->r12 = hardfault_args[4];
    hardfault_data->lr = hardfault_args[5];
    hardfault_data->pc = hardfault_args[6];
    hardfault_data->psr = hardfault_args[7];

    halt();
}

void send_command(int command, void *message) {
    bool active = (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) == CoreDebug_DHCSR_C_DEBUGEN_Msk;

    if (!active) {
        return;
    }

    __asm volatile (
    "mov r0, %[cmd];"
        "mov r1, %[msg];"
        "bkpt #0xAB" : : [cmd] "r"(command), [msg] "r"(message) : "r0", "r1", "memory"
    );
}

size_t strlen(const char *s) {
    size_t size = 0;
    while (*s++ != '\0') size++;
    return size;
}

int run = 1;

/*
 * When we get there the stack pointer is set
 */
int main() {
    SystemInit();
//    SystemCoreClockUpdate();

    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_MEMFAULTPENDED_Msk | SCB_SHCSR_BUSFAULTENA_Msk;

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA, DISABLE);

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOB, DISABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA,
                           ENABLE);

    GPIO_InitTypeDef init;
    GPIO_StructInit(&init);
    init.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &init);

    while (run) {
//        GPIO_SetBits(GPIOA, GPIO_Pin_All);
        GPIO_SetBits(GPIOB, GPIO_Pin_All);
//        GPIO_SetBits(GPIOC, GPIO_Pin_All);

//        GPIO_ResetBits(GPIOA, GPIO_Pin_All);
        GPIO_ResetBits(GPIOB, GPIO_Pin_All);
//        GPIO_ResetBits(GPIOC, GPIO_Pin_All);
    }

    return 0;
}
