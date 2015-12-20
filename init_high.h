#ifndef INIT_HIGH_H
#define INIT_HIGH_H

extern "C" {

/**
 * Declare all the interrupt/event handlers as weak symbols and make them aliases of the default handler.
 */

void _Reset_Handler() __attribute__ ((weak, alias ("Default_Handler")));

void NMI_Handler() __attribute__ ((weak, alias ("Default_Handler")));

void HardFault_Handler() __attribute__ ((weak, alias ("Default_Handler")));

void MemManage_Handler() __attribute__ ((weak, alias ("Default_Handler")));

void BusFault_Handler() __attribute__ ((weak, alias ("Default_Handler")));

void UsageFault_Handler() __attribute__ ((weak, alias ("Default_Handler")));

void SVC_Handler() __attribute__ ((weak, alias ("Default_Handler")));

void DebugMon_Handler() __attribute__ ((weak, alias ("Default_Handler")));

void PendSV_Handler() __attribute__ ((weak, alias ("Default_Handler")));

void SysTick_Handler() __attribute__ ((weak, alias ("Default_Handler")));

void WWDG_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void PVD_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void TAMPER_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void RTC_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void FLASH_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void RCC_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void EXTI0_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void EXTI1_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void EXTI2_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void EXTI3_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void EXTI4_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void DMA1_Channel1_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void DMA1_Channel2_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void DMA1_Channel3_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void DMA1_Channel4_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void DMA1_Channel5_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void DMA1_Channel6_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void DMA1_Channel7_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void ADC1_2_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void USB_HP_CAN1_TX_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void USB_LP_CAN1_RX0_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void CAN1_RX1_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void CAN1_SCE_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void EXTI9_5_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void TIM1_BRK_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void TIM1_UP_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void TIM1_TRG_COM_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void TIM1_CC_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void TIM2_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void TIM3_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void TIM4_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void I2C1_EV_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void I2C1_ER_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void I2C2_EV_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void I2C2_ER_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void SPI1_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void SPI2_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void USART1_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void USART2_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void USART3_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void EXTI15_10_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void RTCAlarm_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));

void USBWakeUp_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));
}

#endif INIT_HIGH_H
