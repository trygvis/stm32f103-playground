#ifndef INIT_HIGH_H
#define INIT_HIGH_H

extern "C" {

extern void _Reset_Handler();

extern void NMI_Handler();

extern void HardFault_Handler();

extern void MemManage_Handler();

extern void BusFault_Handler();

extern void UsageFault_Handler();

extern void SVC_Handler();

extern void DebugMon_Handler();

extern void PendSV_Handler();

extern void SysTick_Handler();

extern void WWDG_IRQHandler();

extern void PVD_IRQHandler();

extern void TAMPER_IRQHandler();

extern void RTC_IRQHandler();

extern void FLASH_IRQHandler();

extern void RCC_IRQHandler();

extern void EXTI0_IRQHandler();

extern void EXTI1_IRQHandler();

extern void EXTI2_IRQHandler();

extern void EXTI3_IRQHandler();

extern void EXTI4_IRQHandler();

extern void DMA1_Channel1_IRQHandler();

extern void DMA1_Channel2_IRQHandler();

extern void DMA1_Channel3_IRQHandler();

extern void DMA1_Channel4_IRQHandler();

extern void DMA1_Channel5_IRQHandler();

extern void DMA1_Channel6_IRQHandler();

extern void DMA1_Channel7_IRQHandler();

extern void ADC1_2_IRQHandler();

extern void USB_HP_CAN1_TX_IRQHandler();

extern void USB_LP_CAN1_RX0_IRQHandler();

extern void CAN1_RX1_IRQHandler();

extern void CAN1_SCE_IRQHandler();

extern void EXTI9_5_IRQHandler();

extern void TIM1_BRK_IRQHandler();

extern void TIM1_UP_IRQHandler();

extern void TIM1_TRG_COM_IRQHandler();

extern void TIM1_CC_IRQHandler();

extern void TIM2_IRQHandler();

extern void TIM3_IRQHandler();

extern void TIM4_IRQHandler();

extern void I2C1_EV_IRQHandler();

extern void I2C1_ER_IRQHandler();

extern void I2C2_EV_IRQHandler();

extern void I2C2_ER_IRQHandler();

extern void SPI1_IRQHandler();

extern void SPI2_IRQHandler();

extern void USART1_IRQHandler();

extern void USART2_IRQHandler();

extern void USART3_IRQHandler();

extern void EXTI15_10_IRQHandler();

extern void RTCAlarm_IRQHandler();

extern void USBWakeUp_IRQHandler();
}

#endif
