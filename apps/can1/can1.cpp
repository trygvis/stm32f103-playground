#include <stdint.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_can.h>
#include <misc.h>
#include <cinttypes>

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

__attribute__((used))
size_t strlen(const char *s) {
    size_t size = 0;
    while (*s++ != '\0') size++;
    return size;
}

volatile uint8_t tx_ready = 0;

class apb_1_tag {
public:
    static const int bus_id = 1;
};

class apb_2_tag {
public:
    static const int bus_id = 2;
};

class peripheral_usage_ref {
public:
    virtual void enable() = 0;

    virtual void disable() = 0;
};

template<uint32_t peripheral_id, typename apb_tag>
class peripheral_usage final : public peripheral_usage_ref {

public:
    peripheral_usage() : count(0) {}

    void enable() {
        dbg_printf("enable: %d:%08" PRIx32 ", count=%d\n", apb_tag::bus_id, peripheral_id, count);
        if (count == 0) {
            do_enable(apb_tag{});
        }
        count++;
    }

    void disable() {
        dbg_printf("disable: %d:%08" PRIx32 ", count=%d\n", apb_tag::bus_id, peripheral_id, count);
        count--;
        if (count == 0) {
            do_disable(apb_tag{});
        }
    }

private:
    static void do_enable(apb_1_tag) {
        RCC_APB1PeriphClockCmd(peripheral_id, ENABLE);

        RCC_APB1PeriphResetCmd(peripheral_id, ENABLE);
        RCC_APB1PeriphResetCmd(peripheral_id, DISABLE);
    }

    static void do_disable(apb_1_tag) {
        RCC_APB1PeriphClockCmd(peripheral_id, DISABLE);
    }

    static void do_enable(apb_2_tag) {
        RCC_APB2PeriphClockCmd(peripheral_id, ENABLE);

        RCC_APB2PeriphResetCmd(peripheral_id, ENABLE);
        RCC_APB2PeriphResetCmd(peripheral_id, DISABLE);
    }

    static void do_disable(apb_2_tag) {
        RCC_APB2PeriphClockCmd(peripheral_id, DISABLE);
    }

    int count;
};

peripheral_usage<RCC_APB1Periph_CAN1, apb_1_tag> can1_peripheral;

peripheral_usage<RCC_APB2Periph_GPIOA, apb_2_tag> gpioa_peripheral;
peripheral_usage<RCC_APB2Periph_AFIO, apb_2_tag> afio_peripheral;

GPIO_TypeDef *debug_gpio_block = GPIOA;
const uint16_t debug_pin_a = GPIO_Pin_2;
const uint16_t debug_pin_b = GPIO_Pin_3;
//const uint16_t debug_pin_2 = GPIO_Pin_2;

CAN_TypeDef *can = CAN1;

const uint16_t can_rx_pin = GPIO_Pin_11;
const uint16_t can_tx_pin = GPIO_Pin_12;

bool debug_pins_init() {
    gpioa_peripheral.enable();

    GPIO_InitTypeDef init;
    GPIO_StructInit(&init);

    init.GPIO_Pin = debug_pin_a;
    init.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(debug_gpio_block, &init);

    init.GPIO_Pin = debug_pin_b;
    init.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(debug_gpio_block, &init);

//    init.GPIO_Pin = debug_pin_2;
//    init.GPIO_Mode = GPIO_Mode_Out_PP;
//    GPIO_Init(debug_gpio_block, &init);

    return true;
}

bool can_init(peripheral_usage_ref &gpio_peripheral, peripheral_usage_ref &afio_peripheral,
              peripheral_usage_ref &can_peripheral, GPIO_TypeDef *gpioBlock,
              uint16_t rx_pin, uint16_t tx_pin) {
    gpio_peripheral.enable();
    afio_peripheral.enable();

    GPIO_InitTypeDef gpioInit;
    gpioInit.GPIO_Pin = rx_pin;
    gpioInit.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(gpioBlock, &gpioInit);

    gpioInit.GPIO_Pin = tx_pin;
    gpioInit.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(gpioBlock, &gpioInit);

//    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);

    can_peripheral.enable();

    {
        CAN_InitTypeDef init;
        CAN_StructInit(&init);

        CAN_DeInit(can);

        init.CAN_TTCM = DISABLE;
        init.CAN_ABOM = DISABLE;
        init.CAN_AWUM = DISABLE;
        init.CAN_NART = DISABLE;
        init.CAN_RFLM = DISABLE;
        init.CAN_TXFP = DISABLE;
        init.CAN_Mode = CAN_Mode_Normal;
//        init.CAN_Mode = CAN_Mode_LoopBack;

        init.CAN_SJW = CAN_SJW_1tq;
        init.CAN_BS1 = CAN_BS1_3tq;
        init.CAN_BS2 = CAN_BS2_5tq;
        init.CAN_Prescaler = 64;
        auto ret = CAN_Init(can, &init);

        if (ret != CAN_InitStatus_Success) {
            return false;
        }

        CAN_ITConfig(can, CAN_IT_TME |
                          CAN_IT_FMP0 |
                          CAN_IT_FF0 |
                          CAN_IT_FOV0 |
                          CAN_IT_FMP1 |
                          CAN_IT_FF1 |
                          CAN_IT_FOV1 |
                          CAN_IT_EWG |
                          CAN_IT_EPV |
                          CAN_IT_LEC |
                          CAN_IT_ERR |
                          CAN_IT_WKU |
                          CAN_IT_SLK, ENABLE);
    }

    {
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

        NVIC_InitTypeDef init{
                .NVIC_IRQChannel = 0,
                .NVIC_IRQChannelPreemptionPriority = 0,
                .NVIC_IRQChannelSubPriority = 0,
                .NVIC_IRQChannelCmd = ENABLE
        };

        init.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
        NVIC_Init(&init);
        NVIC_EnableIRQ(static_cast<IRQn_Type>(init.NVIC_IRQChannel));

        init.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
        NVIC_Init(&init);
        NVIC_EnableIRQ(static_cast<IRQn_Type>(init.NVIC_IRQChannel));

        init.NVIC_IRQChannel = CAN1_SCE_IRQn;
        NVIC_Init(&init);
        NVIC_EnableIRQ(static_cast<IRQn_Type>(init.NVIC_IRQChannel));

        init.NVIC_IRQChannel = CAN1_RX1_IRQn;
        NVIC_Init(&init);
        NVIC_EnableIRQ(static_cast<IRQn_Type>(init.NVIC_IRQChannel));
    }

    return true;
}

static volatile bool run = true;

/**
 * To set this flag, set a hbreak in gdb (hb main) and then set it to 1 (set variable is_client = 1).
 */
volatile int is_client = false;

CanRxMsg msg;
volatile bool msg_valid = false;

int main() {
    SystemInit();

    is_client = is_client != 0;
    dbg_printf("can1: client=%d\n", is_client);

    init_printf(nullptr, dbg_putc);

    debug_pins_init();

    if (!can_init(gpioa_peripheral, afio_peripheral, can1_peripheral, GPIOA, can_rx_pin, can_tx_pin)) {
        dbg_printf("CAN init failed!");
    }

    for (int i = 0; i < 10; i++) {
        GPIO_SetBits(GPIOA, debug_pin_a);
        GPIO_ResetBits(GPIOA, debug_pin_a);
    }

    if (!is_client) {
        while (run) {
            GPIO_SetBits(GPIOA, debug_pin_a);
            GPIO_ResetBits(GPIOA, debug_pin_a);

            dbg_printf("CAN_TSR=%08" PRIx32 ", CAN_ESR=%08" PRIx32 "\n", can->TSR, can->ESR);

            CanTxMsg txMsg;
            txMsg.StdId = 0x321;
            txMsg.ExtId = 0x01;
            txMsg.RTR = CAN_RTR_Data;
            txMsg.IDE = CAN_Id_Extended;
            txMsg.DLC = 8;
            for (int i = 0; i < 8; i++) {
                txMsg.Data[i] = static_cast<uint8_t>(i + 1);
            }

            CAN_Transmit(can, &txMsg);
            run = false;
        }

        for (int i = 0; i < 10; i++) {
            dbg_printf("CAN_TSR=%08" PRIx32 ", CAN_ESR=%08" PRIx32 "\n", can->TSR, can->ESR);
        }
    } else {
        CAN_FilterInitTypeDef init;
        init.CAN_FilterNumber = 0;
        init.CAN_FilterMode = CAN_FilterMode_IdMask;
        init.CAN_FilterScale = CAN_FilterScale_32bit;
        init.CAN_FilterIdHigh = 0x0000;
        init.CAN_FilterIdLow = 0x0000;
        init.CAN_FilterMaskIdHigh = 0x0000;
        init.CAN_FilterMaskIdLow = 0x0000;
        init.CAN_FilterFIFOAssignment = 0;
        init.CAN_FilterActivation = ENABLE;
        CAN_FilterInit(&init);
    }

    if (!is_client) {
        dbg_printf("Waiting ...\n");
    }

    run = true;
    while (run) {
        if (is_client) {
//            dbg_printf("CAN_TSR=%08" PRIx32 ", CAN_ESR=%08" PRIx32 "\n", can->TSR, can->ESR);

            if (msg_valid) {
                dbg_printf("Got message: StdId=%" PRIu32
                                   ", ExtId=%" PRIu32
                                   ", IDE=%d"
                                   ", RTR=%d"
                                   ", DLC=%d"
                                   ", FMI=%d"
                                   ", DATA=%08" PRIx32 "%08" PRIx32 "\n",
                           msg.StdId, msg.ExtId, (int) msg.IDE, (int) msg.RTR, (int) msg.DLC, (int) msg.FMI,
                           (uint32_t) (msg.Data[0] << 24 | msg.Data[1] << 16 | msg.Data[2] << 8 | msg.Data[3]),
                           (uint32_t) (msg.Data[4] << 24 | msg.Data[5] << 16 | msg.Data[6] << 8 | msg.Data[7]));
                msg_valid = false;
            }
        }
    }

    return 0;
}

void on_intr(const char *function) {
    dbg_printf("%s: CAN_TSR=%08" PRIx32 ", CAN_ESR=%08" PRIx32 "\n", function, can->TSR, can->ESR);
    uint8_t tec = CAN_GetLSBTransmitErrorCounter(can);
    uint8_t rec = CAN_GetReceiveErrorCounter(can);
    dbg_printf("TEC: %d, REC: %d\n", tec, rec);

#define check(flag) if(CAN_GetITStatus(can, flag)) { dbg_printf(#flag "\n"); CAN_ClearITPendingBit(can, flag); }
    check(CAN_IT_TME);
    check(CAN_IT_FMP0);
    check(CAN_IT_FF0);
    check(CAN_IT_FOV0);
    check(CAN_IT_FMP1);
    check(CAN_IT_FF1);
    check(CAN_IT_FOV1);
    check(CAN_IT_WKU);
    check(CAN_IT_SLK);
    check(CAN_IT_EWG);
    check(CAN_IT_EPV);
    check(CAN_IT_BOF);
    check(CAN_IT_LEC);
#undef check

#define check(flag) if (CAN_GetFlagStatus(can, flag) == SET) { dbg_printf(#flag "\n"); CAN_ClearFlag(can, flag); }
    check(CAN_FLAG_EWG);
    check(CAN_FLAG_EPV);
    check(CAN_FLAG_BOF);
    check(CAN_FLAG_RQCP0);
    check(CAN_FLAG_RQCP1);
    check(CAN_FLAG_RQCP2);
    check(CAN_FLAG_FMP1);
    check(CAN_FLAG_FF1);
    check(CAN_FLAG_FOV1);
    check(CAN_FLAG_FMP0);
    check(CAN_FLAG_FF0);
    check(CAN_FLAG_FOV0);
    check(CAN_FLAG_WKU);
    check(CAN_FLAG_SLAK);
    check(CAN_FLAG_LEC);
#undef check
}

extern "C"
void USB_HP_CAN1_TX_IRQHandler() {
    GPIO_SetBits(GPIOA, debug_pin_b);
    GPIO_ResetBits(GPIOA, debug_pin_b);

    on_intr(__FUNCTION__);
}

extern "C"
void USB_LP_CAN1_RX0_IRQHandler() {
    GPIO_SetBits(GPIOA, debug_pin_b);
    GPIO_ResetBits(GPIOA, debug_pin_b);

    on_intr(__FUNCTION__);

    if (CAN_GetITStatus(can, CAN_IT_FMP0)) {
        CAN_Receive(can, 0, &msg);
        msg_valid = true;
        CAN_ClearITPendingBit(can, CAN_IT_FMP0);
    }
}

extern "C"
void CAN1_RX1_IRQHandler() {
    GPIO_SetBits(GPIOA, debug_pin_b);
    GPIO_ResetBits(GPIOA, debug_pin_b);

    on_intr(__FUNCTION__);
}

extern "C"
void CAN1_SCE_IRQHandler() {
    GPIO_SetBits(GPIOA, debug_pin_b);
    GPIO_ResetBits(GPIOA, debug_pin_b);

    on_intr(__FUNCTION__);
}
