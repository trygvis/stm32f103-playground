#include <stdint.h>
#include <stdint-gcc.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stddef.h>

#include "tmp/printf/printf.h"

#include "stm32f10x_conf.h"

extern "C"
//__attribute__((naked))
int main(void);

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

    do {
    } while (1);
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

//static
const char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n";

extern uint32_t _copy_data_load, _copy_data_store, _copy_data_store_end;
extern uint32_t _bss_start, _bss_end;

size_t strlen(const char *s) {
    size_t size = 0;
    while (*s++ != '\0') size++;
    return size;
}

void test_command() {
    char msg[100];

    tfp_sprintf(msg, "Hello World: c=%c\r\n", 'c');
    size_t len = strlen(msg);

    uint32_t message[] = {
        2,
        (uint32_t) msg,
        len
    };
    send_command(0x05, &message);

    tfp_sprintf(msg, "Hello %s\r\n", "World!");
    len = strlen(msg);

    uint32_t message3[] = {
        2,
        (uint32_t) msg,
        len
    };

    uint32_t message2[] = {
        2,
        (uint32_t) alphabet,
        28
    };
    send_command(0x05, &message2);

    send_command(0x05, &message3);

    send_command(0x05, &message2);
}

/*
 * When we get there the stack pointer is set
 */
int main() {
    // Copy data from flash to ram
    uint32_t *src = &_copy_data_load;
    uint32_t *dest = &_copy_data_store;
    uint32_t *end = &_copy_data_store_end;

    while (dest <= end) {
        *dest++ = *src++;
    }

    // Clear the BSS segment
    dest = &_bss_start;
    end = &_bss_end;
    while (dest <= end) {
        *dest++ = 0;
    }

    test_command();

    SystemInit();
    SystemCoreClockUpdate();
//    RCC->APB2ENR = RCC_APB2ENR_IOPCEN;
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_MEMFAULTPENDED_Msk | SCB_SHCSR_BUSFAULTENA_Msk;

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA, DISABLE);

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOB, DISABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA,
                           ENABLE);

    if (1) {
        GPIO_InitTypeDef init;
        GPIO_StructInit(&init);
        init.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOB, &init);

        while (1) {
            GPIO_SetBits(GPIOA, GPIO_Pin_All);
            GPIO_SetBits(GPIOB, GPIO_Pin_All);
            GPIO_SetBits(GPIOC, GPIO_Pin_All);

//            send_command(0x05, &message);

            GPIO_ResetBits(GPIOA, GPIO_Pin_All);
            GPIO_ResetBits(GPIOB, GPIO_Pin_All);
            GPIO_ResetBits(GPIOC, GPIO_Pin_All);

//            send_command(0x05, &message);
        }
    }

    return 0;
}
