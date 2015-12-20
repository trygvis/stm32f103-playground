/*
https://github.com/dwelch67/stm32_samples
http://stackoverflow.com/questions/9565921/cortex-m3-initialisation
*/

.syntax unified
.cpu cortex-m3
.thumb

.section .isr_vectors

/* TODO: Move this to C code */
.global vectors
vectors:
    .word   0x20001000                      /* TODO: this should come from the linker */
    .word   _Reset_Handler
    .word   NMI_Handler
    .word   HardFault_Handler
    .word   MemManage_Handler
    .word   BusFault_Handler
    .word   UsageFault_Handler
    .word   0                               /* Not used */
    .word   0                               /* Not used */
    .word   0                               /* Not used */
    .word   0                               /* Not used */
	.word	SVC_Handler
	.word	DebugMon_Handler
	.word	0                               /* Not used */
	.word	PendSV_Handler
	.word	SysTick_Handler
	.word	WWDG_IRQHandler
	.word	PVD_IRQHandler
	.word	TAMPER_IRQHandler
	.word	RTC_IRQHandler
	.word	FLASH_IRQHandler
	.word	RCC_IRQHandler
	.word	EXTI0_IRQHandler
	.word	EXTI1_IRQHandler
	.word	EXTI2_IRQHandler
	.word	EXTI3_IRQHandler
	.word	EXTI4_IRQHandler
	.word	DMA1_Channel1_IRQHandler
	.word	DMA1_Channel2_IRQHandler
	.word	DMA1_Channel3_IRQHandler
	.word	DMA1_Channel4_IRQHandler
	.word	DMA1_Channel5_IRQHandler
	.word	DMA1_Channel6_IRQHandler
	.word	DMA1_Channel7_IRQHandler
	.word	ADC1_2_IRQHandler
	.word	USB_HP_CAN1_TX_IRQHandler
	.word	USB_LP_CAN1_RX0_IRQHandler
	.word	CAN1_RX1_IRQHandler
	.word	CAN1_SCE_IRQHandler
	.word	EXTI9_5_IRQHandler
	.word	TIM1_BRK_IRQHandler
	.word	TIM1_UP_IRQHandler
	.word	TIM1_TRG_COM_IRQHandler
	.word	TIM1_CC_IRQHandler
	.word	TIM2_IRQHandler
	.word	TIM3_IRQHandler
	.word	TIM4_IRQHandler
	.word	I2C1_EV_IRQHandler
	.word	I2C1_ER_IRQHandler
	.word	I2C2_EV_IRQHandler
	.word	I2C2_ER_IRQHandler
	.word	SPI1_IRQHandler
	.word	SPI2_IRQHandler
	.word	USART1_IRQHandler
	.word	USART2_IRQHandler
	.word	USART3_IRQHandler
	.word	EXTI15_10_IRQHandler
	.word	RTCAlarm_IRQHandler
	.word	USBWakeUp_IRQHandler

/* VERY significant */
.section .text

.thumb_func
.global _Reset_Handler
_Reset_Handler:
    bl init_high
    b halt

.thumb_func
.global halt
halt:
    b .

.thumb_func
NMI_Handler:
    b halt

.thumb_func
HardFault_Handler:
    tst lr, #4
    ite eq
    mrseq r0, msp
    mrsne r0, psp
    b HardFault_Handler_C

.thumb_func
MemManage_Handler:
    b halt

.thumb_func
BusFault_Handler:
    b halt

.thumb_func
UsageFault_Handler:
    b halt

/*
TODO: replace with functions like this:
void TIM2_IRQHandler() __attribute__ ((weak, alias ("Default_Handler")));
*/

.weak SVC_Handler
.thumb_set SVC_Handler, halt

.weak DebugMon_Handler
.thumb_set DebugMon_Handler, halt

.weak PendSV_Handler
.thumb_set PendSV_Handler, halt

.weak SysTick_Handler
.thumb_set SysTick_Handler, halt

.weak WWDG_IRQHandler
.thumb_set WWDG_IRQHandler, halt

.weak PVD_IRQHandler
.thumb_set PVD_IRQHandler, halt

.weak TAMPER_IRQHandler
.thumb_set TAMPER_IRQHandler, halt

.weak RTC_IRQHandler
.thumb_set RTC_IRQHandler, halt

.weak FLASH_IRQHandler
.thumb_set FLASH_IRQHandler, halt

.weak RCC_IRQHandler
.thumb_set RCC_IRQHandler, halt

.weak EXTI0_IRQHandler
.thumb_set EXTI0_IRQHandler, halt

.weak EXTI1_IRQHandler
.thumb_set EXTI1_IRQHandler, halt

.weak EXTI2_IRQHandler
.thumb_set EXTI2_IRQHandler, halt

.weak EXTI3_IRQHandler
.thumb_set EXTI3_IRQHandler, halt

.weak EXTI4_IRQHandler
.thumb_set EXTI4_IRQHandler, halt

.weak DMA1_Channel1_IRQHandler
.thumb_set DMA1_Channel1_IRQHandler, halt

.weak DMA1_Channel2_IRQHandler
.thumb_set DMA1_Channel2_IRQHandler, halt

.weak DMA1_Channel3_IRQHandler
.thumb_set DMA1_Channel3_IRQHandler, halt

.weak DMA1_Channel4_IRQHandler
.thumb_set DMA1_Channel4_IRQHandler, halt

.weak DMA1_Channel5_IRQHandler
.thumb_set DMA1_Channel5_IRQHandler, halt

.weak DMA1_Channel6_IRQHandler
.thumb_set DMA1_Channel6_IRQHandler, halt

.weak DMA1_Channel7_IRQHandler
.thumb_set DMA1_Channel7_IRQHandler, halt

.weak ADC1_2_IRQHandler
.thumb_set ADC1_2_IRQHandler, halt

.weak USB_HP_CAN1_TX_IRQHandler
.thumb_set USB_HP_CAN1_TX_IRQHandler, halt

.weak USB_LP_CAN1_RX0_IRQHandler
.thumb_set USB_LP_CAN1_RX0_IRQHandler, halt

.weak CAN1_RX1_IRQHandler
.thumb_set CAN1_RX1_IRQHandler, halt

.weak CAN1_SCE_IRQHandler
.thumb_set CAN1_SCE_IRQHandler, halt

.weak EXTI9_5_IRQHandler
.thumb_set EXTI9_5_IRQHandler, halt

.weak TIM1_BRK_IRQHandler
.thumb_set TIM1_BRK_IRQHandler, halt

.weak TIM1_UP_IRQHandler
.thumb_set TIM1_UP_IRQHandler, halt

.weak TIM1_TRG_COM_IRQHandler
.thumb_set TIM1_TRG_COM_IRQHandler, halt

.weak TIM1_CC_IRQHandler
.thumb_set TIM1_CC_IRQHandler, halt

.weak TIM2_IRQHandler
.thumb_set TIM2_IRQHandler, halt

.weak TIM3_IRQHandler
.thumb_set TIM3_IRQHandler, halt

.weak TIM4_IRQHandler
.thumb_set TIM4_IRQHandler, halt

.weak I2C1_EV_IRQHandler
.thumb_set I2C1_EV_IRQHandler, halt

.weak I2C1_ER_IRQHandler
.thumb_set I2C1_ER_IRQHandler, halt

.weak I2C2_EV_IRQHandler
.thumb_set I2C2_EV_IRQHandler, halt

.weak I2C2_ER_IRQHandler
.thumb_set I2C2_ER_IRQHandler, halt

.weak SPI1_IRQHandler
.thumb_set SPI1_IRQHandler, halt

.weak SPI2_IRQHandler
.thumb_set SPI2_IRQHandler, halt

.weak USART1_IRQHandler
.thumb_set USART1_IRQHandler, halt

.weak USART2_IRQHandler
.thumb_set USART2_IRQHandler, halt

.weak USART3_IRQHandler
.thumb_set USART3_IRQHandler, halt

.weak EXTI15_10_IRQHandler
.thumb_set EXTI15_10_IRQHandler, halt

.weak RTCAlarm_IRQHandler
.thumb_set RTCAlarm_IRQHandler, halt

.weak USBWakeUp_IRQHandler
.thumb_set USBWakeUp_IRQHandler, halt

.end
