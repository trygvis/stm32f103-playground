#include <stdint.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stddef.h>
#include <stdarg.h>
#include "printf.h"
#include "debug.h"

int init_high();

extern "C" void halt();

#include "stm32f10x_conf.h"

extern "C"
__attribute__((naked))
void HardFault_Handler_C(uint32_t *hardfault_args) {
    halt();
}

size_t strlen(const char *s) {
    size_t size = 0;
    while (*s++ != '\0') size++;
    return size;
}

int run = 1;

volatile USART_TypeDef *usart1 = (volatile USART_TypeDef *) USART1_BASE;

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

    // 8 bit mode
    USART1->CR1 &= ~USART_CR1_M;
    USART1->CR2 &= ~USART_CR2_STOP_1;

    USART1->CR1 |= USART_CR1_UE /* Set UART Enable */
                   | USART_CR1_TE; /* Set Transmission Enable */

    // Set baud rate
    int mantissa = 39;
    int fraction = static_cast<int>(16 * 0.0625); // == 1
    // 72M / (16*39.0625) = 115200
    USART1->BRR = static_cast<uint16_t >(mantissa << 4 | fraction);

    char c = 'A';
    while (run) {
        int txe = USART1->SR & USART_SR_TXE;

//        dbg_printf("1:%d?\n", x);

//        char mm[100];
//        tfp_sprintf(mm, "2:%d?\n", x);
//        send_command(SYS_WRITE0, mm);

//        printf(" %u?\n", usart1->SR);
//        printf(" %u?\n", 1);

        if (txe) {
            GPIO_SetBits(GPIOB, GPIO_Pin_All);
            GPIO_ResetBits(GPIOB, GPIO_Pin_All);

            USART1->DR = (uint16_t) c;
//            USART1->DR = 0x55;

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
    }

    return 0;
}

