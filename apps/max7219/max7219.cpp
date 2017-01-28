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

#define SIZE 32


class Max7219 {

public:
    Max7219();
    void spiTransfer(uint8_t opcode, uint8_t data);
    void write();
    void drawPixel(int x, int y);
    void drawCol(int row, uint8_t value);
    void setPosition(uint8_t display, uint8_t x, uint8_t y);
    void setRotation(uint8_t display, uint8_t rotation);

private:
    uint8_t buffer[SIZE];
    uint8_t matrixPosition[4];
    uint8_t matrixRotation[4];
    int hDisplays = 4;

    void setBit(uint8_t &byte, int position);
    bool getBit(uint8_t byte, int position);
};


Max7219::Max7219() {
    spiTransfer(OP_DISPLAYTEST, 0);
    spiTransfer(OP_SCANLIMIT, 7);
    spiTransfer(OP_DECODEMODE, 0);
    spiTransfer(OP_SHUTDOWN, 1);
    spiTransfer(OP_INTENSITY, 0);

    for( int i = 0; i < SIZE; i++ ) {
        buffer[i] = 0x00;
    }

    /*
    buffer[0] = 0b01111000;
    buffer[1] = 0b01000100;
    buffer[2] = 0b01000100;
    buffer[3] = 0b01111000;
    buffer[4] = 0b01000100;
    buffer[5] = 0b01000100;
    buffer[6] = 0b01111000;
    buffer[7] = 0b00000000;

    buffer[8]  = 0b01111000;
    buffer[9]  = 0b01000100;
    buffer[10] = 0b01000100;
    buffer[11] = 0b01111000;
    buffer[12] = 0b01000100;
    buffer[13] = 0b01000100;
    buffer[14] = 0b01111000;
    buffer[15] = 0b00000000;

    buffer[16] = 0b00000000;
    buffer[17] = 0b00000000;
    buffer[18] = 0b00000000;
    buffer[19] = 0b00000000;
    buffer[20] = 0b00000000;
    buffer[21] = 0b00000000;
    buffer[22] = 0b00000000;
    buffer[23] = 0b00000000;

    buffer[24] = 0b00000000;
    buffer[25] = 0b00000000;
    buffer[26] = 0b00000000;
    buffer[27] = 0b00000000;
    buffer[28] = 0b00000000;
    buffer[29] = 0b00000000;
    buffer[30] = 0b00000000;
    buffer[31] = 0b00000000;
     */
}


void Max7219::write() {
    for ( uint8_t row = OP_DIGIT7; row >= OP_DIGIT0; row-- ) {
        spiTransfer(row, 0);
    }
}


void Max7219::spiTransfer(uint8_t opcode, uint8_t data) {
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    GPIO_ResetBits(GPIOB, GPIO_Pin_5);

    uint8_t end = opcode - OP_DIGIT0;
    uint8_t start = SIZE + end;
    do {
        start -= 8;

        SPI_I2S_SendData(SPI1, opcode);
        DelayUs(30);
        SPI_I2S_SendData(SPI1, opcode <= OP_DIGIT7 ? buffer[start] : data);
        DelayUs(30);
    }
    while ( start > end );

    GPIO_SetBits(GPIOB, GPIO_Pin_5);

}


void Max7219::drawPixel(int x, int y) {
    int tmp;

    uint8_t display = matrixPosition[(x >> 3) + hDisplays * (y >> 3)];
    x &= 0b111;
    y &= 0b111;

    uint8_t r = matrixRotation[display];
    if ( r >= 2 ) {                 // 180 or 270 degrees
        x = 7 - x;
    }
    if ( r == 1 || r == 2 ) {       // 90 or 180 degrees
        y = 7 - y;
    }
    if ( r & 1 ) {                  // 90 or 270 degrees
        tmp = x; x = y; y = tmp;
    }

    int d = display / hDisplays;
    x += (display - d * hDisplays) << 3;
    y += d << 3;

    setBit( buffer[ x ], y );
}


void Max7219::drawCol( int col, uint8_t value) {
    for( int i = 0; i < 8; i++ ) {
        if( getBit( value, i ) ) {
            drawPixel( col, i );
        }
    }
}


/*
 * Define how the displays are ordered. The first display (0)
 * is the one closest to the MCU.
 */
void Max7219::setPosition(uint8_t display, uint8_t x, uint8_t y) {
    matrixPosition[x + hDisplays * y] = display;
}


/*
 * Define if and how the displays are rotated. The first display
 * (0) is the one closest to the MCU. rotation can be:
 *   0: no rotation
 *   1: 90 degrees clockwise
 *   2: 180 degrees
 *   3: 90 degrees counter clockwise
 */
void Max7219::setRotation(uint8_t display, uint8_t rotation) {
    matrixRotation[display] = rotation;
}


void Max7219::setBit(uint8_t &byte, int position) {
    byte |= 1 << position;
}


bool Max7219::getBit(uint8_t byte, int position) {
    return (byte >> position) & 0x1;
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



    Max7219 display = Max7219();
    display.setPosition(0, 0, 0);
    display.setPosition(1, 1, 0);
    display.setPosition(2, 2, 0);
    display.setPosition(3, 3, 0);
    display.setRotation( 0, 1 );
    display.setRotation( 1, 1 );
    display.setRotation( 2, 1 );
    display.setRotation( 3, 1 );

    display.drawPixel(1, 0);
    display.drawPixel(9, 0);
    display.drawPixel(17, 0);
    display.drawPixel(25, 0);


    display.write();


    int i = 0;

    while (run) {
        display.drawCol( i++, 0b11110001 );
        display.write();

        DelayMs(200);
    }

    return 0;
}