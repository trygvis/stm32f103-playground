#include <stdint.h>
#include <stdint-gcc.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>

#include "stm32f10x_conf.h"

extern "C"
//__attribute__((naked))
int main(void);

extern "C"
__attribute__((naked))
void HardFault_Handler_C(uint32_t * hardfault_args);

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

volatile struct hardfault_data_t *hardfault_data = (volatile struct hardfault_data_t *)0x20000800;

void HardFault_Handler_C(uint32_t * hardfault_args) {
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

__attribute__((naked))
void send_command(int command, void* message) {
  __asm volatile (
    "mov r0, %[cmd] \n\t"
    "mov r1, %[msg] \n\t"
    "bkpt #0xAB" : : [cmd] "r" (command), [msg] "r" (message) : "r0", "r1", "memory"
  );
}

int main() {
    uint32_t message[] = {
        2,
        (uint32_t)"Hello World!\r\n",
        14
    };
    send_command(0x05, &message);

    SystemInit();
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_MEMFAULTPENDED_Msk | SCB_SHCSR_BUSFAULTENA_Msk;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE);

//    GPIO_InitTypeDef init;
//    GPIO_StructInit(&init);
//    init.GPIO_Mode = GPIO_Mode_Out_PP;
//    init.GPIO_Pin =

    do {
        volatile uint32_t *port_b = (uint32_t *) (0x40010c00);
        volatile uint32_t *port_b_crl = (uint32_t *) (port_b + 0x00);
        volatile uint32_t *port_b_crh = (uint32_t *) (port_b + 0x04);
        volatile uint32_t *port_b_idr = (uint32_t *) (port_b + 0x08);
        volatile uint32_t *port_b_odr = (uint32_t *) (port_b + 0x0c);
        volatile uint32_t *port_b_bsrr = (uint32_t *) (port_b + 0x10);
        volatile uint32_t *port_b_brr = (uint32_t *) (port_b + 0x14);

        // mode=output, max speed 10MHz
        *port_b_crl = 0x11111111;
        *port_b_crh = 0x11111111;

        *port_b_bsrr = 0xffff0000;

        *port_b_bsrr = 0x0000ffff;
    } while (1);

    return 0;
}

extern "C" void high() {

    do {
        volatile uint32_t *port_b = (uint32_t *) (0x40010c00);
        volatile uint32_t *port_b_crl = (uint32_t *) (port_b + 0x00);
        volatile uint32_t *port_b_crh = (uint32_t *) (port_b + 0x04);
        volatile uint32_t *port_b_bsrr = (uint32_t *) (port_b + 0x10);

        *port_b_crl = 0x11111111;
        *port_b_crh = 0x11111111;
        *port_b_bsrr = 0xffff0000;

        *port_b_bsrr = 0x0000ffff;
    } while (1);
}

extern "C" void low() {

    do {
        volatile uint32_t *port_b = (uint32_t *) (0x40010c00);
        volatile uint32_t *port_b_crl = (uint32_t *) (port_b + 0x00);
        volatile uint32_t *port_b_crh = (uint32_t *) (port_b + 0x04);
        volatile uint32_t *port_b_bsrr = (uint32_t *) (port_b + 0x10);

        *port_b_crl = 0x11111111;
        *port_b_crh = 0x11111111;
        *port_b_bsrr = 0xffff0000;

        *port_b_bsrr = 0xffff0000;
    } while (1);
}

//extern "C" void _Reset_Handler() {
//
//}
