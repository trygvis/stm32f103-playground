#include <stdint.h>
#include <stm32f10x.h>
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

struct motor_t {
    GPIO_TypeDef *gpio_port;
    const uint16_t ah;
    const uint16_t al;
};

static struct motor_t motor_a = {
        gpio_port: GPIOA,
        ah : GPIO_Pin_0,
        al : GPIO_Pin_1,
};

TIM_TypeDef *pwm_timer = TIM2;

struct apb_bus_1 {
};
struct apb_bus_2 {
};

template<typename apb_bus>
class apb_ctrl {
public:
    static void RCC_PeriphClockCmd(uint32_t peripherals, FunctionalState functionalState) {
        RCC_PeriphClockCmd(apb_bus{}, peripherals, functionalState);
    }

private:
    static void RCC_PeriphClockCmd(apb_bus_1, uint32_t peripherals, FunctionalState functionalState) {
        RCC_APB1PeriphClockCmd(peripherals, functionalState);
    }

    static void RCC_PeriphClockCmd(apb_bus_2, uint32_t peripherals, FunctionalState functionalState) {
        RCC_APB2PeriphClockCmd(functionalState, functionalState);
    }

public:
    static void RCC_PeriphResetCmd(uint32_t peripherals, FunctionalState functionalState) {
        RCC_PeriphResetCmd(apb_bus{}, peripherals, functionalState);
    }

private:
    static void RCC_PeriphResetCmd(apb_bus_1, uint32_t peripherals, FunctionalState functionalState) {
        RCC_APB1PeriphResetCmd(peripherals, functionalState);
    }

    static void RCC_PeriphResetCmd(apb_bus_2, uint32_t peripherals, FunctionalState functionalState) {
        RCC_APB2PeriphResetCmd(functionalState, functionalState);
    }
};

template<typename apb_bus, uint32_t peripheral>
class apb_device {
public:
    static void reset() {
        apb_ctrl<apb_bus>::RCC_PeriphResetCmd(peripheral, ENABLE);
        apb_ctrl<apb_bus>::RCC_PeriphResetCmd(peripheral, DISABLE);
    }

    static void enable() {
        apb_ctrl<apb_bus>::RCC_PeriphClockCmd(peripheral, ENABLE);
    }

    static void disable() {
        apb_ctrl<apb_bus>::RCC_PeriphClockCmd(peripheral, DISABLE);
    }
};

template<typename apb_bus, uint32_t peripheral>
class gpio_port : public apb_device<apb_bus, peripheral> {
public:
};

class gpio_port_a : public gpio_port<apb_bus_2, RCC_APB2Periph_GPIOA> {
};

class gpio_port_b : public gpio_port<apb_bus_2, RCC_APB2Periph_GPIOB> {
};

class gpio_port_c : public gpio_port<apb_bus_2, RCC_APB2Periph_GPIOC> {
};

class tim2 : public apb_device<apb_bus_1, RCC_APB1Periph_TIM2> {
};

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

void gpio_init(GPIO_TypeDef* gpio_port, uint16_t pins, bool use_af) {
    GPIO_InitTypeDef init;
    GPIO_StructInit(&init);
    init.GPIO_Mode = use_af ? GPIO_Mode_AF_PP : GPIO_Mode_Out_PP;
    init.GPIO_Pin = pins;
    init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(gpio_port, &init);
}

volatile bool run = true;

bool use_gpio = false;

int main() {
    SystemInit();

    init_printf(nullptr, dbg_putc);

    dbg_printf("stepper2\n");

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

    gpio_init(motor_a.gpio_port, motor_a.ah | motor_a.al, !use_gpio);
    timer_init(pwm_timer, period);
    pwm_control(pwm_timer, pulse);

    TIM_ARRPreloadConfig(pwm_timer, ENABLE);
    TIM_Cmd(pwm_timer, ENABLE);

    while (run) {
        static bool on = false;

        if (use_gpio) {
            if (on) {
                GPIO_SetBits(motor_a.gpio_port, motor_a.ah);
                GPIO_SetBits(motor_a.gpio_port, motor_a.al);
            } else {
                GPIO_ResetBits(motor_a.gpio_port, motor_a.ah);
                GPIO_ResetBits(motor_a.gpio_port, motor_a.al);
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

                pwm_timer->CCR1 = pulse;
                pwm_timer->CCR2 = period - pulse;
            }
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
