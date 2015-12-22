#include <stdint.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <misc.h>

#include "debug.h"
#include "tinyprintf.h"

int init_high();

extern "C" void halt();

extern "C"
__attribute__((naked))
void HardFault_Handler_C(uint32_t *hardfault_args) {
    dbg_printf("r0 = 0x%08x (%d)\n", hardfault_args[0], hardfault_args[0]);
    dbg_printf("r1 = 0x%08x (%d)\n", hardfault_args[1], hardfault_args[1]);
    dbg_printf("r2 = 0x%08x (%d)\n", hardfault_args[2], hardfault_args[2]);
    dbg_printf("r3 = 0x%08x (%d)\n", hardfault_args[3], hardfault_args[3]);
    dbg_printf("r12 = 0x%08x (%d)\n", hardfault_args[4], hardfault_args[4]);
    dbg_printf("lr = 0x%08x (%d)\n", hardfault_args[5], hardfault_args[5]);
    dbg_printf("pc = 0x%08x (%d)\n", hardfault_args[6], hardfault_args[6]);
    dbg_printf("psr = 0x%08x (%d)\n", hardfault_args[7], hardfault_args[7]);
    dbg_printf("\n");

    halt();
}

size_t strlen(const char *s) {
    size_t size = 0;
    while (*s++ != '\0') size++;
    return size;
}

int run = 1;

volatile USART_TypeDef *usart1 = (volatile USART_TypeDef *) USART1_BASE;

volatile uint8_t tx_ready = 0;

/*
 * When we get there the stack pointer is set
 */
int main() {
    SystemInit();

    init_printf(nullptr, dbg_putc);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO
                           | RCC_APB2Periph_USART1
                           | RCC_APB2Periph_GPIOA
                           | RCC_APB2Periph_GPIOB
                           | RCC_APB2Periph_GPIOC,
                           ENABLE);

    /* ***************************************** */

    // Debug on port B

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOB, DISABLE);

    // Make Port B's pin #5 the debug output pin
    GPIO_InitTypeDef init;
    GPIO_StructInit(&init);
    init.GPIO_Pin = GPIO_Pin_5;
    init.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &init);

    /* ***************************************** */

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA, DISABLE);

    /*
     * PA9	USART1_TX
     * PA10	USART1_RX
     */

    // Enable USART1
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1, DISABLE);

    // Make the TX pin an output
    GPIO_StructInit(&init);
    init.GPIO_Pin = GPIO_Pin_9;
    init.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &init);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    NVIC_InitTypeDef NVIC_InitStruct = {
        NVIC_IRQChannel: USART1_IRQn,
        NVIC_IRQChannelPreemptionPriority: 0,
        NVIC_IRQChannelSubPriority: 0,
        NVIC_IRQChannelCmd: ENABLE,
    };
    NVIC_Init(&NVIC_InitStruct);
    NVIC_EnableIRQ(USART1_IRQn);

    // 8 bit mode
    USART1->CR1 &= ~USART_CR1_M;
    USART1->CR2 &= ~USART_CR2_STOP_1;

    // Set baud rate
    int mantissa = 39;
    int fraction = static_cast<int>(16 * 0.0625); // == 1
    // 72M / (16*39.0625) = 115200
    USART1->BRR = static_cast<uint16_t >(mantissa << 4 | fraction);

    USART1->CR1 |= USART_CR1_UE /* Set UART Enable */
                   | USART_CR1_TE /* Set Transmission Enable */
                   | USART_CR1_TXEIE; /* Set TX buffer Empty Interrupt Enable */

    char c = 'A';
    tx_ready = 1;
    while (run) {
        // wait for TX to be ready
        while (!tx_ready);
        tx_ready = 0;

        GPIO_SetBits(GPIOB, GPIO_Pin_All);
        GPIO_ResetBits(GPIOB, GPIO_Pin_All);

        USART1->DR = (uint16_t) c;
        USART_ITConfig(USART1, USART_IT_TXE, ENABLE);

        if (c == 'Z') {
            c = 'a';
        } else if (c == 'z') {
            c = '0';
        } else if (c == '9') {
            c = '\n';
        } else if (c == '\n') {
            c = 'A';
        } else {
            c++;
        }
    }

    return 0;
}

extern "C"
void USART1_IRQHandler() {
    tx_ready = 1;

    if (USART_GetITStatus(USART1, USART_IT_TXE) == SET) {
        // Disable the interrupt
        USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    }
}
