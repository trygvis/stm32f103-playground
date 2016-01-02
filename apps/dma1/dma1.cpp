#include <stdint.h>
#include <stm32f10x.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_spi.h>
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

volatile bool run = true;

void manual_spi_write(char &c);

void dma_spi_write();

volatile bool manual = false;
volatile bool dma_busy;

static uint8_t dma_buffer[256];

/*
 * When we get there the stack pointer is set
 */
int main() {
    SystemInit();

    init_printf(nullptr, dbg_putc);

    dbg_printf("DMA with SPI sample\n");

    for (uint16_t i = 0; i < SizeOfArray(dma_buffer); i++) {
        dma_buffer[i] = uint8_t(i);
    }

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
                           | RCC_APB2Periph_SPI1
                           | RCC_APB2Periph_GPIOA
                           | RCC_APB2Periph_GPIOB
                           | RCC_APB2Periph_GPIOC,
                           ENABLE);

    /* ***************************************** */

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOB, DISABLE);

    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    GPIO_InitTypeDef gpioInit;
    GPIO_StructInit(&gpioInit);
    gpioInit.GPIO_Pin = GPIO_Pin_5;
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpioInit);

    // ----------------------------------------------------
    // SPI Configuration
    // Configure SPI MOSI to be on pin PA7, SCK on PA5 and NSS on PA15

    SPI_InitTypeDef spiInit;
    SPI_StructInit(&spiInit);
    spiInit.SPI_Direction = SPI_Direction_1Line_Tx;
    spiInit.SPI_Mode = SPI_Mode_Master;
    spiInit.SPI_DataSize = SPI_DataSize_8b;
    spiInit.SPI_CPOL = SPI_CPOL_Low;
    spiInit.SPI_CPHA = SPI_CPHA_1Edge;
    spiInit.SPI_NSS = SPI_NSS_Soft;
    spiInit.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    spiInit.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_I2S_DeInit(SPI1);
    SPI_Init(SPI1, &spiInit);
    SPI_Cmd(SPI1, ENABLE);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);

    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_GPIOA, DISABLE);
    GPIO_StructInit(&gpioInit);
    gpioInit.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_15;
    gpioInit.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioInit);

    if (manual) {
        char c = 'a';
        while (run) {
            manual_spi_write(c);
        }
    } else {
        while (run) {
            dma_spi_write();
        }
    }

    return 0;
}

/**
 * Test code to ensure that the SPI part is configured correctly
 */
void manual_spi_write(char &c) {
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    GPIO_SetBits(GPIOB, GPIO_Pin_5);
    SPI_I2S_SendData(SPI1, (uint16_t) c);

    if (c == 'Z') {
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);

        GPIO_ResetBits(GPIOB, GPIO_Pin_5);
        for (int count = 100; count > 0; count--) {
            __NOP();
        }
        GPIO_SetBits(GPIOB, GPIO_Pin_5);

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

void dma_spi_write() {
    dma_busy = true;

    DMA_InitTypeDef dmaInit;
    DMA_StructInit(&dmaInit);
    dmaInit.DMA_M2M = DMA_M2M_Disable;
    dmaInit.DMA_Mode = DMA_Mode_Normal;
    dmaInit.DMA_Priority = DMA_Priority_Low;
    dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dmaInit.DMA_DIR = DMA_DIR_PeripheralDST;
    dmaInit.DMA_BufferSize = SizeOfArray(dma_buffer);
    dmaInit.DMA_PeripheralBaseAddr = reinterpret_cast<uint32_t>(&SPI1->DR);
    dmaInit.DMA_MemoryBaseAddr = reinterpret_cast<uint32_t>(dma_buffer);
    DMA_DeInit(DMA1_Channel3);
    DMA_Init(DMA1_Channel3, &dmaInit);
    DMA_ITConfig(DMA1_Channel3, DMA_IT_TC | DMA_IT_TE, ENABLE);


    GPIO_SetBits(GPIOB, GPIO_Pin_5);
    DMA_Cmd(DMA1_Channel3, ENABLE);

//    dbg_printf("HT=%d, TE=%d, TC=%d\n", DMA_GetFlagStatus(DMA1_IT_HT3), DMA_GetFlagStatus(DMA1_IT_TE3),
//               DMA_GetFlagStatus(DMA1_IT_TC3));

//    dbg_printf("HT=%d, TE=%d, TC=%d\n", DMA_GetFlagStatus(DMA1_IT_HT3), DMA_GetFlagStatus(DMA1_IT_TE3),
//               DMA_GetFlagStatus(DMA1_IT_TC3));

    while (dma_busy) {
        __NOP();

//        dbg_printf("HT=%d, TE=%d, TC=%d\n", DMA_GetFlagStatus(DMA1_IT_HT3), DMA_GetFlagStatus(DMA1_IT_TE3),
//                   DMA_GetFlagStatus(DMA1_IT_TC3));
    }
    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
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
