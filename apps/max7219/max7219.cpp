#include <cstdint>
#include <stm32f10x.h>
#include <stm32f10x_spi.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include "debug.h"
#include "tinyprintf.h"
#include "playground.h"
#include "delay.h"
#include "alphabeth.h"


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
    gpioInit.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
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


#define SIZE 32


class Max7219 {

    enum op {
        NOOP = 0,
        DIGIT0 = 1,
        DIGIT1 = 2,
        DIGIT2 = 3,
        DIGIT3 = 4,
        DIGIT4 = 5,
        DIGIT5 = 6,
        DIGIT6 = 7,
        DIGIT7 = 8,
        DECODEMODE = 9,
        INTENSITY = 10,
        SCANLIMIT = 11,
        SHUTDOWN = 12,
        DISPLAYTEST = 15,
    };

public:
    Max7219(GPIO_TypeDef *csReg, uint16_t csPin);

    void spiTransfer(op opcode, uint8_t data);

    void write();

    void drawPixel(int x, int y, bool set);

    void drawCol(int row, uint8_t value, bool set);

    void drawChar(int display, char c);

    void drawString(const char *str);

    void fill(int display);

    void clear(int display);

    void setPosition(uint8_t display, uint8_t x, uint8_t y);

    void setRotation(uint8_t display, uint8_t rotation);

private:
    GPIO_TypeDef *csReg;
    uint16_t csPin;
    uint8_t buffer[SIZE];
    uint8_t matrixPosition[4];
    uint8_t matrixRotation[4];
    int hDisplays = 4;

    void setBit(uint8_t &byte, int position);

    void clearBit(uint8_t &byte, int position);

    bool getBit(uint8_t byte, int position);
};


Max7219::Max7219(GPIO_TypeDef *csReg, uint16_t csPin) {
    this->csReg = csReg;
    this->csPin = csPin;

    spiTransfer(op::DISPLAYTEST, 0);
    spiTransfer(op::SCANLIMIT, 7);
    spiTransfer(op::DECODEMODE, 0);
    spiTransfer(op::SHUTDOWN, 1);
    spiTransfer(op::INTENSITY, 2);

    for (int i = 0; i < SIZE; i++) {
        buffer[i] = 0x00;
    }
}


void Max7219::write() {
    for (int row = op::DIGIT7; row >= op::DIGIT0; row--) {
        spiTransfer((op) row, 0);
    }
}


void Max7219::spiTransfer(op opcode, uint8_t data) {
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    GPIO_ResetBits(csReg, csPin);

    int end = opcode - op::DIGIT0;
    int start = SIZE + end;
    do {
        start -= 8;

        SPI_I2S_SendData(SPI1, opcode);
        DelayUs(30);
        SPI_I2S_SendData(SPI1, opcode <= op::DIGIT7 ? buffer[start] : data);
        DelayUs(30);
    } while (start > end);

    GPIO_SetBits(csReg, csPin);

}


void Max7219::drawPixel(int x, int y, bool set) {
    int tmp;

    uint8_t display = matrixPosition[(x >> 3) + hDisplays * (y >> 3)];
    x &= 0b111;
    y &= 0b111;

    uint8_t r = matrixRotation[display];
    if (r >= 2) {                 // 180 or 270 degrees
        x = 7 - x;
    }
    if (r == 1 || r == 2) {       // 90 or 180 degrees
        y = 7 - y;
    }
    if (r & 1) {                  // 90 or 270 degrees
        tmp = x;
        x = y;
        y = tmp;
    }

    int d = display / hDisplays;
    x += (display - d * hDisplays) << 3;
    y += d << 3;

    if (set) {
        setBit(buffer[x], y);
    } else {
        clearBit(buffer[x], y);
    }
}


void Max7219::drawCol(int col, uint8_t value, bool set) {
    for (int i = 0; i < 8; i++) {
        if (getBit(value, i)) {
            drawPixel(col, i, set);
        }
    }
}


void Max7219::drawChar(int display, char c) {
    // Handle # and space separate since alphabet is 7x7 and won't fill display
    if (c == '#') {
        fill(display);
        return;
    }

    if (c == ' ') {
        clear(display);
        return;
    }

    int ascii = (int) c;
    int offset = display * 8;

    for (int i = 0; i < 7; i++) {
        uint8_t l = alphabet[((ascii - 0x20) * 7) + i];
        drawCol(i + offset, l >> 1, true);
    }
}


void Max7219::drawString(const char *str) {
    for (int i = 0; i < 4; i++) {
        drawChar(i, str[i]);
    }
}


void Max7219::fill(int display) {
    int offset = display * 8;

    for (int i = 0; i < 8; i++) {
        drawCol(i + offset, 0xFF, true);
    }
}


void Max7219::clear(int display) {
    int offset = display * 8;

    for (int i = 0; i < 8; i++) {
        drawCol(i + offset, 0xFF, false);
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


void Max7219::clearBit(uint8_t &byte, int position) {
    byte &= ~(1 << position);
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


    Max7219 d1 = Max7219(GPIOB, GPIO_Pin_5);
    d1.setPosition(0, 0, 0);
    d1.setPosition(1, 1, 0);
    d1.setPosition(2, 2, 0);
    d1.setPosition(3, 3, 0);
    d1.setRotation(0, 2);
    d1.setRotation(1, 2);
    d1.setRotation(2, 2);
    d1.setRotation(3, 2);
    d1.drawString("Hei-");
    d1.write();


    Max7219 d2 = Max7219(GPIOB, GPIO_Pin_6);
    d2.setPosition(0, 0, 0);
    d2.setPosition(1, 1, 0);
    d2.setPosition(2, 2, 0);
    d2.setPosition(3, 3, 0);
    d2.setRotation(0, 2);
    d2.setRotation(1, 2);
    d2.setRotation(2, 2);
    d2.setRotation(3, 2);
    d2.drawString("sann");
    d2.write();


    while (run) {
        DelayMs(1000);
    }

    return 0;
}

extern "C"
void SysTick_Handler() {
    DelayOnTick();
}
