#include <inttypes.h>
#include <stdint.h>
#include <stm32f10x.h>
#include <stm32f10x_adc.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_tim.h>
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

struct hbridge_t {
    GPIO_TypeDef *gpio_port;
    const uint16_t ah;
    const uint16_t al;
};

struct motor_t {
    TIM_TypeDef *pwm_timer;
    ADC_TypeDef *adc;
    uint8_t adc_channel;
    GPIO_TypeDef *shunt_gpio_port;
    uint16_t shunt_pin;
    hbridge_t a;
    hbridge_t b;
};

static struct motor_t motor = {
        pwm_timer:  TIM2,
        adc: ADC1,
        adc_channel: ADC_Channel_4,
        shunt_gpio_port: GPIOA,
        shunt_pin: GPIO_Pin_4, // ADC12_IN4
        a: {
                gpio_port: GPIOA,
                ah : GPIO_Pin_0,
                al : GPIO_Pin_1,
        },
        b: {
                gpio_port: GPIOA,
                ah : GPIO_Pin_2,
                al : GPIO_Pin_3,
        },
};

volatile bool adc_ready;
volatile uint16_t adc_current_sample;

void adc_init(const motor_t &motor) {

    GPIO_InitTypeDef init;
    GPIO_StructInit(&init);
    init.GPIO_Mode = GPIO_Mode_AIN;
    init.GPIO_Pin = motor.shunt_pin;
    init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(motor.shunt_gpio_port, &init);

    adc_ready = false;

    ADC_InitTypeDef adcInit = {
            ADC_Mode: ADC_Mode_Independent,
            ADC_ScanConvMode: DISABLE,
            ADC_ContinuousConvMode: DISABLE,
            ADC_ExternalTrigConv: ADC_ExternalTrigConv_None,
            ADC_DataAlign: ADC_DataAlign_Right,
            ADC_NbrOfChannel: 1
    };
    ADC_Init(motor.adc, &adcInit);

    ADC_RegularChannelConfig(motor.adc, motor.adc_channel, 1, ADC_SampleTime_13Cycles5);
}

void timer_init(TIM_TypeDef *tim, uint16_t period) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
//    tim2::reset();
//    tim2::enable();

    TIM_TimeBaseInitTypeDef timerInitStructure;
    TIM_TimeBaseStructInit(&timerInitStructure);
    timerInitStructure.TIM_Prescaler = 0;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = period;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(tim, &timerInitStructure);
}

void pwm_control(TIM_TypeDef *tim, uint16_t pulse) {
    TIM_OCInitTypeDef outputChannelInit;
    outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
    outputChannelInit.TIM_Pulse = pulse;
    outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
    outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(tim, &outputChannelInit);
    TIM_OC1PreloadConfig(tim, TIM_OCPreload_Enable);

    TIM_OC2Init(tim, &outputChannelInit);
    TIM_OC2PreloadConfig(tim, TIM_OCPreload_Enable);
}

void gpio_init(GPIO_TypeDef *gpio_port, uint16_t pins, bool use_af) {
    GPIO_InitTypeDef init;
    GPIO_StructInit(&init);
    init.GPIO_Mode = use_af ? GPIO_Mode_AF_PP : GPIO_Mode_Out_PP;
    init.GPIO_Pin = pins;
    init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(gpio_port, &init);
}

volatile bool run = true;

bool use_gpio = false;

volatile uint32_t millis = 0;

int main() {
    SystemInit();

    init_printf(nullptr, dbg_putc);

    dbg_printf("stepper2\n");

    {
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
        NVIC_InitTypeDef NVIC_InitStruct = {
                NVIC_IRQChannel: DMA1_Channel3_IRQn,
                NVIC_IRQChannelPreemptionPriority: 0,
                NVIC_IRQChannelSubPriority: 0,
                NVIC_IRQChannelCmd: ENABLE,
        };
        NVIC_Init(&NVIC_InitStruct);
        NVIC_EnableIRQ(DMA1_Channel3_IRQn);
    }

    {
        NVIC_InitTypeDef NVIC_InitStruct = {
                NVIC_IRQChannel: ADC1_2_IRQn,
                NVIC_IRQChannelPreemptionPriority: 0,
                NVIC_IRQChannelSubPriority: 0,
                NVIC_IRQChannelCmd: ENABLE,
        };

        NVIC_Init(&NVIC_InitStruct);
        NVIC_EnableIRQ(ADC1_2_IRQn);
    }

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO
                           | RCC_APB2Periph_GPIOA
                           | RCC_APB2Periph_GPIOB
                           | RCC_APB2Periph_GPIOC
                           | RCC_APB2Periph_ADC1
                           | RCC_APB2Periph_ADC2
                           | RCC_APB2Periph_ADC3
                           | RCC_APB2Periph_TIM1,
                           ENABLE);

    const uint16_t period = 1000;

    const int count_max = 10000;
    int count = count_max - 1;

    uint16_t pulse = 500;

    gpio_init(motor.a.gpio_port, motor.a.ah | motor.a.al, !use_gpio);
    timer_init(motor.pwm_timer, period);
    pwm_control(motor.pwm_timer, pulse);

    TIM_ARRPreloadConfig(motor.pwm_timer, ENABLE);
    TIM_Cmd(motor.pwm_timer, ENABLE);

    adc_init(motor);

    ADC_Cmd(motor.adc, ENABLE);

    dbg_printf("Starting calibration\n");
    ADC_ResetCalibration(motor.adc);
    while (ADC_GetResetCalibrationStatus(motor.adc));
    ADC_StartCalibration(motor.adc);
    while (ADC_GetCalibrationStatus(motor.adc));
    ADC_SoftwareStartConvCmd(motor.adc, ENABLE);
    dbg_printf("ADC calibration done\n");

    if (SysTick_Config(SystemCoreClock / 1000)) {
        dbg_printf("SysTick_Config failed.\n");
    }

    ADC_ITConfig(motor.adc, ADC_IT_EOC, ENABLE);

    while (run) {
        static bool on = false;

        if (use_gpio) {
            if (on) {
                GPIO_SetBits(motor.a.gpio_port, motor.a.ah);
                GPIO_SetBits(motor.a.gpio_port, motor.a.al);
            } else {
                GPIO_ResetBits(motor.a.gpio_port, motor.a.ah);
                GPIO_ResetBits(motor.a.gpio_port, motor.a.al);
            }
            on = !on;
        } else {
            count++;

            if (count == count_max) {
                count = 0;

                pulse++;

                if (pulse > period) {
                    pulse = 0;
                }

                motor.pwm_timer->CCR1 = pulse;
                motor.pwm_timer->CCR2 = period - pulse;
            }
        }

        static int adc_count = 0;
        if (adc_ready) {
            adc_ready = false;
            ADC_SoftwareStartConvCmd(motor.adc, ENABLE);
            adc_count++;
        }

        static uint32_t last_ms = 0;
        if (millis > last_ms + 1000) {
            // adc_current_sample = [0, 4096]
            int a = adc_current_sample * 25;        // [0, 102400). 3.3V = 102400
            int b = a * 33;                         // [0, 307200). 3.3V = 3379200
            int c = b / 1024;                       // [0, 3300]
            int voltage_mv = c;
            dbg_printf("adc_count=%d, value=%" PRIu16 "\n", adc_count, adc_current_sample);
            dbg_printf("voltage=%d\n", voltage_mv);
            adc_count = 0;
            last_ms += 1000;
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

extern "C"
void ADC1_2_IRQHandler() {
    adc_ready = true;
    adc_current_sample = ADC_GetConversionValue(motor.adc);

    ADC_ClearITPendingBit(motor.adc, ADC_IT_EOC | ADC_IT_AWD | ADC_IT_JEOC);
}

extern "C"
void SysTick_Handler() {
    millis++;
}
