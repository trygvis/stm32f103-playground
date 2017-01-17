#include <inttypes.h>
#include <stm32f10x.h>
#include <stm32f10x_adc.h>
#include <stm32f10x_rcc.h>
#include "debug.h"
#include "tinyprintf.h"
#include "playground.h"


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
	for( int i = 0; i < 2000000; i++ ) {
		__NOP()	;
	}
}


/*
 * Set up ADC1
 */
void adcInit() {
    // Scale the ADC clock down by a factor of 6
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    // Enable clock for ADC1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // Put everything back to power-on defaults
    ADC_DeInit(ADC1);


    ADC_InitTypeDef adcInit = {
            ADC_Mode: ADC_Mode_Independent,                     // ADC1 and ADC2 operate independently
            ADC_ScanConvMode: DISABLE,                          // Disable the scan conversion so we do one at a time
            ADC_ContinuousConvMode: DISABLE,                    // Don't do contimuous conversions - do them on demand
            ADC_ExternalTrigConv: ADC_ExternalTrigConv_None,    // Start conversin by software, not an external trigger
            ADC_DataAlign: ADC_DataAlign_Right,                 // Conversions are 12 bit - put them in the lower 12 bits of the result
            ADC_NbrOfChannel: 1                                 // How many channels to be used by the sequencer
    };

    // Do the setup
    ADC_Init(ADC1, &adcInit);

    // Enable ADC1
    ADC_Cmd(ADC1, ENABLE);

    // Enable ADC1 reset calibaration register
    ADC_ResetCalibration(ADC1);

    // Check the end of ADC1 reset calibration register
    while(ADC_GetResetCalibrationStatus(ADC1));

    // Start ADC1 calibaration
    ADC_StartCalibration(ADC1);

    // Check the end of ADC1 calibration
    while(ADC_GetCalibrationStatus(ADC1));
}


/*
 * Read ADC1
 */
u16 readADC1(u8 channel) {
    // Set sample time to it's minimum
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_1Cycles5);

    // Start the conversion
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    // Wait until conversion completion
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

    return ADC_GetConversionValue(ADC1);
}


/*
 * Entry point
 */
int main() {
    // http://www.micromouseonline.com/2009/05/26/simple-adc-use-on-the-stm32/

    SystemInit();

    dbg_printf("INITIALIZE ADC\n");

    adcInit();


    while (run) {
        // Channel 2 is connected to PA2 on dev card
        u16 val = readADC1(2);
        dbg_printf("VAL: %u\n", val);

        delay();
    }

    return 0;
}