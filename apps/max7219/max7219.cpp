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
            SPI_BaudRatePrescaler : SPI_BaudRatePrescaler_64,
            SPI_FirstBit          : SPI_FirstBit_MSB,
            SPI_CRCPolynomial     : 0
    };

    SPI_I2S_DeInit(SPI1);
    SPI_Init(SPI1, &spiInit);
    SPI_Cmd(SPI1, ENABLE);
}



#define OP_NOOP         0
#define OP_DIGIT0       1
#define OP_DIGIT1       2
#define OP_DIGIT2       3
#define OP_DIGIT3       4
#define OP_DIGIT4       5
#define OP_DIGIT5       6
#define OP_DIGIT6       7
#define OP_DIGIT7       8
#define OP_DECODEMODE   9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

const int size = 32;
uint8_t buffer[size];




void spiTransfer(uint8_t opcode, uint8_t data) {
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    DelayUs(100);

    int end = opcode - OP_DIGIT0;
    int start = size + end;

    do {
        start -= 8;
        DelayUs(30);
        SPI_I2S_SendData(SPI1, opcode);
        DelayUs(30);
        SPI_I2S_SendData(SPI1, opcode <= OP_DIGIT7 ? buffer[start] : data);
        DelayUs(30);
    }
    while ( start > end );

    GPIO_SetBits(GPIOB, GPIO_Pin_5);
    DelayUs(100);
}


void write() {
    for ( uint8_t row = OP_DIGIT7; row >= OP_DIGIT0; row-- ) {
        spiTransfer(row, 0);
    }
}


/*
 * Entry point
 */
int main() {
    SystemInit();

    dbg_printf("MAX7219!\n");

    rccInit();
    gpioInit();
    spiInit();
    DelayInit();


    spiTransfer(OP_DISPLAYTEST, 0);
    spiTransfer(OP_SCANLIMIT, 7);
    spiTransfer(OP_DECODEMODE, 0);
    spiTransfer(OP_SHUTDOWN, 1);
    spiTransfer(OP_INTENSITY, 0);




    for( int i = 0; i < size; i++ ) {
        buffer[i] = 0x00;
    }


    buffer[0] = 0b11111111;
    buffer[1] = 0b00000000;
    buffer[2] = 0b00000000;
    buffer[3] = 0b00000000;
    buffer[4] = 0b00000000;
    buffer[5] = 0b00000000;
    buffer[6] = 0b00000000;
    buffer[7] = 0b00000000;

    buffer[8]  = 0b11111111;
    buffer[9]  = 0b00000000;
    buffer[10] = 0b00000000;
    buffer[11] = 0b00000000;
    buffer[12] = 0b00000000;
    buffer[13] = 0b00000000;
    buffer[14] = 0b00000000;
    buffer[15] = 0b00000000;

    buffer[16] = 0b11111111;
    buffer[17] = 0b00000000;
    buffer[18] = 0b00000000;
    buffer[19] = 0b00000000;
    buffer[20] = 0b00000000;
    buffer[21] = 0b00000000;
    buffer[22] = 0b00000000;
    buffer[23] = 0b00000000;

    buffer[24] = 0b11111111;
    buffer[25] = 0b00000000;
    buffer[26] = 0b00000000;
    buffer[27] = 0b00000000;
    buffer[28] = 0b00000000;
    buffer[29] = 0b00000000;
    buffer[30] = 0b00000000;
    buffer[31] = 0b00000000;



    write();







    while (run) {
        dbg_printf("LOOP\n");
        DelayMs(2000);
    }

    return 0;
}