#include <stdint.h>
#include <inttypes.h>
#include <stm32f10x.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include "debug.h"
#include "tinyprintf.h"
#include "playground.h"
#include "delay.h"


#define REG_CHANNEL0        0x01
#define REG_INTENSITY       0x0A
#define REG_SCAN_LIMIT      0x0B
#define REG_SHUTDOWN        0x0C
#define REG_TEST            0x0F
#define REG_DECODE_MODE     0x09


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


void send_data( uint8_t address, uint8_t data ) {
    dbg_printf("Sending data..... ");

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    DelayUs(100);
    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    DelayUs(100);
    SPI_I2S_SendData(SPI1, address);
    DelayUs(100);
    SPI_I2S_SendData(SPI1, data);
    DelayUs(100);
    GPIO_SetBits(GPIOB, GPIO_Pin_5);
    DelayUs(100);

    dbg_printf("Done\n");
}


void max7219_self_test(void) {
    send_data(REG_TEST, 0x01);
    DelayMs(500);
    send_data(REG_TEST, 0x00);
}


void max7219_init() {
    send_data(REG_TEST, 0x00);  // Virker
    DelayMs(10);
    send_data(REG_SCAN_LIMIT, 0x07);    // Virker
    DelayMs(10);
    send_data(REG_DECODE_MODE, 0x00);
    DelayMs(10);
    send_data(REG_SHUTDOWN, 0x01);
    DelayMs(10);
}


void max7219_reset() {
    // Shutdown
    send_data(REG_SHUTDOWN, 0x00);

    // Normal operation
    send_data(REG_SHUTDOWN, 0x01);
}


void max7219_clear_all() {
    send_data(1, 0b00000000);
    send_data(2, 0b00000000);
    send_data(3, 0b00000000);
    send_data(4, 0b00000000);
    send_data(5, 0b00000000);
    send_data(6, 0b00000000);
    send_data(7, 0b00000000);
    send_data(8, 0b00000000);
}


void max7219_turn_all() {
    send_data(1, 0b00011000);
    send_data(2, 0b00011000);
    send_data(3, 0b00011000);
    send_data(4, 0b11111111);
    send_data(5, 0b11111111);
    send_data(6, 0b00011000);
    send_data(7, 0b00011000);
    send_data(8, 0b00011000);
}


/*
 * Entry point
 */
int main() {
    SystemInit();

    dbg_printf("MAX7219!!!\n");

    rccInit();
    gpioInit();
    spiInit();
    DelayInit();

    max7219_init();

    send_data( REG_INTENSITY, 0x01 );


    while (run) {
        dbg_printf("TURN ON\n");

        max7219_turn_all();

        DelayMs(2000);

        max7219_clear_all();

        DelayMs(2000);
    }

    return 0;
}