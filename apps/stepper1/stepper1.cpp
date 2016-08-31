#include <stdint.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_dma.h>
#include <misc.h>

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

volatile bool dma_busy;

static uint32_t pwm_buffer[128];

static uint16_t pwm_gpio_pin_a = GPIO_Pin_2;
static uint16_t pwm_gpio_pin_b = GPIO_Pin_3;

static
void pwm_init() {
    int on = true;
    for (size_t i = 0; i < SizeOfArray(pwm_buffer); i++) {
        pwm_buffer[i] = on ? 0xffffffff : 0;
        on=!on;
    }

//    for (size_t i = 0; i < SizeOfArray(pwm_buffer) / 2; i++) {
//        pwm_buffer[i] = 0xffffffff;
//    }
//
//    for (size_t i = SizeOfArray(pwm_buffer) / 2; i < SizeOfArray(pwm_buffer); i++) {
//        pwm_buffer[i] = 0;
//    }
}

static
void pwm_transfer_setup() {
    DMA_Cmd(DMA1_Channel3, DISABLE);
    DMA_DeInit(DMA1_Channel3);

    DMA_InitTypeDef dmaInit;
    DMA_StructInit(&dmaInit);

    dmaInit.DMA_M2M = DMA_M2M_Enable;
    dmaInit.DMA_Mode = DMA_Mode_Normal;
    dmaInit.DMA_Priority = DMA_Priority_VeryHigh;

    dmaInit.DMA_DIR = DMA_DIR_PeripheralDST;

    dmaInit.DMA_PeripheralBaseAddr = reinterpret_cast<uint32_t>(&GPIOA->ODR);
    dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    dmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;

    dmaInit.DMA_MemoryBaseAddr = reinterpret_cast<uint32_t>(pwm_buffer);
    dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;

    dmaInit.DMA_BufferSize = SizeOfArray(pwm_buffer);
    DMA_Init(DMA1_Channel3, &dmaInit);
    DMA_ITConfig(DMA1_Channel3, DMA_IT_TC | DMA_IT_TE, ENABLE);

    dma_busy = true;

    DMA_Cmd(DMA1_Channel3, ENABLE);
}

void gpio_a_setup() {
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA, DISABLE);

    GPIO_InitTypeDef init;
    GPIO_StructInit(&init);
    init.GPIO_Mode = GPIO_Mode_Out_PP;
    init.GPIO_Pin = pwm_gpio_pin_a | pwm_gpio_pin_b;
    GPIO_Init(GPIOA, &init);
}

void gpio_a_loop() {
    static bool on = false;
    if (on) {
        GPIO_SetBits(GPIOA, pwm_gpio_pin_a);
        GPIO_SetBits(GPIOA, pwm_gpio_pin_b);
    } else {
        GPIO_ResetBits(GPIOA, pwm_gpio_pin_a);
        GPIO_ResetBits(GPIOA, pwm_gpio_pin_b);
    }
    on = !on;
}

volatile bool run = true;

bool use_dma = true;

int main() {
    SystemInit();

    init_printf(nullptr, dbg_putc);

    dbg_printf("stepper1\n");

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    NVIC_InitTypeDef NVIC_InitStruct = {
            NVIC_IRQChannel: DMA1_Channel3_IRQn,
            NVIC_IRQChannelPreemptionPriority: 0,
            NVIC_IRQChannelSubPriority: 0,
            NVIC_IRQChannelCmd: ENABLE,
    };
    NVIC_Init(&NVIC_InitStruct);
    NVIC_EnableIRQ(DMA1_Channel3_IRQn);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO
                           | RCC_APB2Periph_GPIOA
//                           | RCC_APB2Periph_GPIOB
//                           | RCC_APB2Periph_GPIOC
//                           | RCC_APB2Periph_ADC1
//                           | RCC_APB2Periph_ADC2
//                           | RCC_APB2Periph_ADC3,
            , ENABLE);

    /* ***************************************** */

    gpio_a_setup();

    pwm_init();

    while (run) {
        if (use_dma) {
//            dbg_printf("dma setup...");
            pwm_transfer_setup();
//            dbg_printf("done\n");

            int count = 0;
            while (dma_busy) {
                count++;
            }

//            dbg_printf("dma tx done: count=%d\n", count);
        } else {
            gpio_a_loop();
        }
    }

    return 0;
}

extern "C"
void DMA1_Channel3_IRQHandler() {
    ITStatus tc = DMA_GetITStatus(DMA1_IT_TC3);
    ITStatus te = DMA_GetITStatus(DMA1_IT_TE3);
    ITStatus ht = DMA_GetITStatus(DMA1_IT_HT3);

    if (tc || te || ht) {
        DMA_ClearITPendingBit(DMA1_IT_GL3);
        dma_busy = false;
    }
}
