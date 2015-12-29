#include <cstdint>
#include <cstddef>
#include "stm32f10x.h"
#include "debug.h"
#include "init_high.h"
#include "playground.h"

/**
 * Symbols that are defined by the linker
 */
extern uint32_t _copy_data_load, _copy_data_store, _copy_data_store_end;
extern uint32_t _bss_start, _bss_end;

extern int main();

template<typename T, size_t N>
static inline
size_t SizeOfArray(const T(&)[N]) {
    return N;
}

void init_high() {
    // Copy data from flash to ram
    uint32_t *src = &_copy_data_load;
    uint32_t *dest = &_copy_data_store;
    uint32_t *end = &_copy_data_store_end;

    while (dest <= end) {
        *dest++ = *src++;
    }

    // Clear the BSS segment
    dest = &_bss_start;
    end = &_bss_end;
    while (dest <= end) {
        *dest++ = 0;
    }

    main();
}

__attribute__((used))
struct {
    uint32_t CFSR;
    uint32_t HFSR;
    uint32_t DFSR;
    uint32_t AFSR;
//    uint32_t MMAR;
    uint32_t BFAR;
} Default_Handler_Info;

__attribute__((used))
void Default_Handler() {
    Default_Handler_Info = {
        CFSR: SCB->CFSR,
        HFSR: SCB->HFSR,
        DFSR: SCB->DFSR,
        AFSR: SCB->AFSR,
        BFAR: SCB->BFAR,
    };

    dbg_printf("Default handler:\n");

    dbg_printf("HFSR: 0x%08lx\n", Default_Handler_Info.HFSR);
    if (Default_Handler_Info.HFSR & SCB_HFSR_DEBUGEVT) {
        dbg_printf("      HFSR.DEBUGEVT\n");
    }
    if (Default_Handler_Info.HFSR & SCB_HFSR_FORCED) {
        dbg_printf("      HFSR.FORCED\n");
    }
    if (Default_Handler_Info.HFSR & SCB_HFSR_VECTTBL) {
        dbg_printf("      HFSR.VECTTBL\n");
    }

    dbg_printf("CFSR: 0x%08lx\n", Default_Handler_Info.CFSR);
    if (Default_Handler_Info.CFSR & SCB_CFSR_DIVBYZERO) {
        dbg_printf("      UFSR.DIVBYZERO\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_UNALIGNED) {
        dbg_printf("      UFSR.UNALIGED\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_NOCP) {
        dbg_printf("      UFSR.NOCP\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_INVPC) {
        dbg_printf("      UFSR.INVPC\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_INVSTATE) {
        dbg_printf("      UFSR.INVSTATE\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_UNDEFINSTR) {
        dbg_printf("      UFSR.UNDEFINSTR\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_BFARVALID) {
        dbg_printf("      BFSR.BFARVALID\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_STKERR) {
        dbg_printf("      BFSR.STKERR\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_UNSTKERR) {
        dbg_printf("      BFSR.UNSTKERR\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_IMPRECISERR) {
        dbg_printf("      BFSR.IMPRECISERR\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_IMPRECISERR) {
        dbg_printf("      BFSR.IMPRECISERR\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_PRECISERR) {
        dbg_printf("      BFSR.PRECISERR\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_IBUSERR) {
        dbg_printf("      BFSR.IBUSERR\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_MMARVALID) {
        dbg_printf("      MMFSR.MMARVALID\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_MSTKERR) {
        dbg_printf("      MMFSR.MSTKERR\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_MUNSTKERR) {
        dbg_printf("      MMFSR.MUNSTKERR\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_DACCVIOL) {
        dbg_printf("      MMFSR.DACCVIOL\n");
    }
    if (Default_Handler_Info.CFSR & SCB_CFSR_IACCVIOL) {
        dbg_printf("      MMFSR.IACCVIOL\n");
    }
    dbg_printf("DFSR: 0x%08lx\n", Default_Handler_Info.DFSR);
    dbg_printf("AFSR: 0x%08lx\n", Default_Handler_Info.AFSR);

    if (Default_Handler_Info.CFSR & SCB_CFSR_BFARVALID) {
        dbg_printf("BFAR: 0x%08lx\n", Default_Handler_Info.BFAR);
    } else {
        dbg_printf("BFAR: <invalid>\n");
    }

    dbg_printf("NVIC:\n");
    for (size_t i = 0; i < SizeOfArray(NVIC->IABR); i++) {
        dbg_printf("  IABR[%d]: 0x%08lx\n", i, NVIC->IABR[i]);
    }

    halt();
}

void _Reset_Handler() __attribute__ ((weak, alias("Default_Handler")));

void NMI_Handler() __attribute__ ((weak, alias("Default_Handler")));

void HardFault_Handler() __attribute__ ((weak, alias("Default_Handler")));

void MemManage_Handler() __attribute__ ((weak, alias("Default_Handler")));

void BusFault_Handler() __attribute__ ((weak, alias("Default_Handler")));

void UsageFault_Handler() __attribute__ ((weak, alias("Default_Handler")));

void SVC_Handler() __attribute__ ((weak, alias("Default_Handler")));

void DebugMon_Handler() __attribute__ ((weak, alias("Default_Handler")));

void PendSV_Handler() __attribute__ ((weak, alias("Default_Handler")));

void SysTick_Handler() __attribute__ ((weak, alias("Default_Handler")));

void WWDG_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void PVD_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void TAMPER_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void RTC_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void FLASH_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void RCC_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void EXTI0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void EXTI1_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void EXTI2_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void EXTI3_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void EXTI4_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void DMA1_Channel1_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void DMA1_Channel2_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void DMA1_Channel3_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void DMA1_Channel4_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void DMA1_Channel5_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void DMA1_Channel6_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void DMA1_Channel7_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void ADC1_2_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void USB_HP_CAN1_TX_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void USB_LP_CAN1_RX0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void CAN1_RX1_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void CAN1_SCE_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void EXTI9_5_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void TIM1_BRK_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void TIM1_UP_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void TIM1_TRG_COM_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void TIM1_CC_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void TIM2_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void TIM3_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void TIM4_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void I2C1_EV_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void I2C1_ER_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void I2C2_EV_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void I2C2_ER_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void SPI1_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void SPI2_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void USART1_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void USART2_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void USART3_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void EXTI15_10_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void RTCAlarm_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

void USBWakeUp_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));

__attribute__((section(".isr_vectors"), used))
uint32_t isr_vectors[74] = {
    (uint32_t) _Reset_Handler,
    (uint32_t) NMI_Handler,
    (uint32_t) HardFault_Handler,
    (uint32_t) MemManage_Handler,
    (uint32_t) BusFault_Handler,
    (uint32_t) UsageFault_Handler,
    0,
    0,
    0,
    0,
    (uint32_t) SVC_Handler,
    (uint32_t) DebugMon_Handler,
    0,
    (uint32_t) PendSV_Handler,
    (uint32_t) SysTick_Handler,
    (uint32_t) WWDG_IRQHandler,
    (uint32_t) PVD_IRQHandler,
    (uint32_t) TAMPER_IRQHandler,
    (uint32_t) RTC_IRQHandler,
    (uint32_t) FLASH_IRQHandler,
    (uint32_t) RCC_IRQHandler,
    (uint32_t) EXTI0_IRQHandler,
    (uint32_t) EXTI1_IRQHandler,
    (uint32_t) EXTI2_IRQHandler,
    (uint32_t) EXTI3_IRQHandler,
    (uint32_t) EXTI4_IRQHandler,
    (uint32_t) DMA1_Channel1_IRQHandler,
    (uint32_t) DMA1_Channel2_IRQHandler,
    (uint32_t) DMA1_Channel3_IRQHandler,
    (uint32_t) DMA1_Channel4_IRQHandler,
    (uint32_t) DMA1_Channel5_IRQHandler,
    (uint32_t) DMA1_Channel6_IRQHandler,
    (uint32_t) DMA1_Channel7_IRQHandler,
    (uint32_t) ADC1_2_IRQHandler,
    (uint32_t) USB_HP_CAN1_TX_IRQHandler,
    (uint32_t) USB_LP_CAN1_RX0_IRQHandler,
    (uint32_t) CAN1_RX1_IRQHandler,
    (uint32_t) CAN1_SCE_IRQHandler,
    (uint32_t) EXTI9_5_IRQHandler,
    (uint32_t) TIM1_BRK_IRQHandler,
    (uint32_t) TIM1_UP_IRQHandler,
    (uint32_t) TIM1_TRG_COM_IRQHandler,
    (uint32_t) TIM1_CC_IRQHandler,
    (uint32_t) TIM2_IRQHandler,
    (uint32_t) TIM3_IRQHandler,
    (uint32_t) TIM4_IRQHandler,
    (uint32_t) I2C1_EV_IRQHandler,
    (uint32_t) I2C1_ER_IRQHandler,
    (uint32_t) I2C2_EV_IRQHandler,
    (uint32_t) I2C2_ER_IRQHandler,
    (uint32_t) SPI1_IRQHandler,
    (uint32_t) SPI2_IRQHandler,
    (uint32_t) USART1_IRQHandler,
    (uint32_t) USART2_IRQHandler,
    (uint32_t) USART3_IRQHandler,
    (uint32_t) EXTI15_10_IRQHandler,
    (uint32_t) RTCAlarm_IRQHandler,
    (uint32_t) USBWakeUp_IRQHandler,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};
