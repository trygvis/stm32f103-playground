#include <stdint.h>
#include <inttypes.h>
#include <stm32f10x.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include "debug.h"
#include "tinyprintf.h"
#include "playground.h"


#define REG_TEST 0x0F


/*
 * Code to handle unexpected errors
 */
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

extern "C"
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

volatile bool run = true;


/*
 * Delay
 */
void delay() {
	for( int i = 0; i < 1000000; i++ ) {
		__NOP()	;
	}
}

void shortDelay() {
    for( int i = 0; i < 10000; i++ ) {
        __NOP()	;
    }
}


void rccInit() {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO
                           | RCC_APB2Periph_SPI1
                           | RCC_APB2Periph_GPIOA
                           | RCC_APB2Periph_GPIOB,
                           ENABLE);
}


void gpioInit() {
    GPIO_InitTypeDef gpioInit;

    // Configure NSS pin
    GPIO_StructInit(&gpioInit);
    gpioInit.GPIO_Pin = GPIO_Pin_5;
    gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpioInit);

    // Configure SCK and MOSI pin
    GPIO_StructInit(&gpioInit);
    gpioInit.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    gpioInit.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioInit);
}


void spiInit() {
    // Configure SPI MOSI to be on pin PA7, SCK on PA5 and NSS on PB5

    SPI_InitTypeDef spiInit = {
            SPI_Direction         : SPI_Direction_1Line_Tx,
            SPI_Mode              : SPI_Mode_Master,
            SPI_DataSize          : SPI_DataSize_8b,
            SPI_CPOL              : SPI_CPOL_Low,
            SPI_CPHA              : SPI_CPHA_1Edge,
            SPI_NSS               : SPI_NSS_Soft,
            SPI_BaudRatePrescaler : SPI_BaudRatePrescaler_256,
            SPI_FirstBit          : SPI_FirstBit_MSB
    };

    SPI_I2S_DeInit(SPI1);
    SPI_Init(SPI1, &spiInit);
    SPI_Cmd(SPI1, ENABLE);
}


/*
void SPIx_Transfer(uint8_t data) {
    // Write data to be transmitted to the SPI data register
    SPI1->DR = data;
    // Wait until transmit complete
    while (!(SPI1->SR & (SPI_I2S_FLAG_TXE)));
}
 */


void send_data( uint8_t address, uint8_t data ) {
    dbg_printf("Sending data..... ");

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    shortDelay();
    SPI_I2S_SendData(SPI1, address);
    shortDelay();
    SPI_I2S_SendData(SPI1, data);
    shortDelay();
    GPIO_SetBits(GPIOB, GPIO_Pin_5);

    dbg_printf("Done\n");
}


void self_test(void) {
    send_data(REG_TEST, 0x01);
    delay();
    send_data(REG_TEST, 0x00);
}


/*
 * Entry point
 */
int main() {
    SystemInit();

    dbg_printf("MAX7219 :D\n");

    rccInit();
    gpioInit();
    spiInit();


    self_test();


    while (run) {
        delay();
    }

    return 0;
}