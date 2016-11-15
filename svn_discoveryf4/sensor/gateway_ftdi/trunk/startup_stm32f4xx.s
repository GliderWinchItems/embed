# 1 "startup_stm32f4xx.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "startup_stm32f4xx.S"
# 33 "startup_stm32f4xx.S"
  .syntax unified
  .cpu cortex-m3
  .fpu softvfp
  .thumb

.global g_pfnVectors
.global Default_Handler



.word _sidata

.word _sdata

.word _edata

.word _sbss

.word _ebss
# 63 "startup_stm32f4xx.S"
    .section .text.Reset_Handler
  .weak Reset_Handler
  .type Reset_Handler, %function
Reset_Handler:


  movs r1, #0
  b LoopCopyDataInit

CopyDataInit:
  ldr r3, =_sidata
  ldr r3, [r3, r1]
  str r3, [r0, r1]
  adds r1, r1, #4

LoopCopyDataInit:
  ldr r0, =_sdata
  ldr r3, =_edata
  adds r2, r0, r1
  cmp r2, r3
  bcc CopyDataInit
  ldr r2, =_sbss
  b LoopFillZerobss

FillZerobss:
  movs r3, #0
  str r3, [r2], #4

LoopFillZerobss:
  ldr r3, = _ebss
  cmp r2, r3
  bcc FillZerobss


  bl SystemInit

  bl __libc_init_array

  bl main
  bx lr
# 112 "startup_stm32f4xx.S"
    .section .text.Default_Handler,"ax",%progbits
Default_Handler2:
Infinite_Loop:
  b Infinite_Loop
# 124 "startup_stm32f4xx.S"
   .section .isr_vector,"a",%progbits
  .type g_pfnVectors, %object
  .size g_pfnVectors, .-g_pfnVectors


g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler


  .word WWDG_IRQHandler
  .word PVD_IRQHandler
  .word TAMP_STAMP_IRQHandler
  .word RTC_WKUP_IRQHandler
  .word FLASH_IRQHandler
  .word RCC_IRQHandler
  .word EXTI0_IRQHandler
  .word EXTI1_IRQHandler
  .word EXTI2_IRQHandler
  .word EXTI3_IRQHandler
  .word EXTI4_IRQHandler
  .word DMA1_Stream0_IRQHandler
  .word DMA1_Stream1_IRQHandler
  .word DMA1_Stream2_IRQHandler
  .word DMA1_Stream3_IRQHandler
  .word DMA1_Stream4_IRQHandler
  .word DMA1_Stream5_IRQHandler
  .word DMA1_Stream6_IRQHandler
  .word ADC_IRQHandler
  .word CAN1_TX_IRQHandler
  .word CAN1_RX0_IRQHandler
  .word CAN1_RX1_IRQHandler
  .word CAN1_SCE_IRQHandler
  .word EXTI9_5_IRQHandler
  .word TIM1_BRK_TIM9_IRQHandler
  .word TIM1_UP_TIM10_IRQHandler
  .word TIM1_TRG_COM_TIM11_IRQHandler
  .word TIM1_CC_IRQHandler
  .word TIM2_IRQHandler
  .word TIM3_IRQHandler
  .word TIM4_IRQHandler
  .word I2C1_EV_IRQHandler
  .word I2C1_ER_IRQHandler
  .word I2C2_EV_IRQHandler
  .word I2C2_ER_IRQHandler
  .word SPI1_IRQHandler
  .word SPI2_IRQHandler
  .word USART1_IRQHandler
  .word USART2_IRQHandler
  .word USART3_IRQHandler
  .word EXTI15_10_IRQHandler
  .word RTC_Alarm_IRQHandler
  .word OTG_FS_WKUP_IRQHandler
  .word TIM8_BRK_TIM12_IRQHandler
  .word TIM8_UP_TIM13_IRQHandler
  .word TIM8_TRG_COM_TIM14_IRQHandler
  .word TIM8_CC_IRQHandler
  .word DMA1_Stream7_IRQHandler
  .word FSMC_IRQHandler
  .word SDIO_IRQHandler
  .word TIM5_IRQHandler
  .word SPI3_IRQHandler
  .word UART4_IRQHandler
  .word UART5_IRQHandler
  .word TIM6_DAC_IRQHandler
  .word TIM7_IRQHandler
  .word DMA2_Stream0_IRQHandler
  .word DMA2_Stream1_IRQHandler
  .word DMA2_Stream2_IRQHandler
  .word DMA2_Stream3_IRQHandler
  .word DMA2_Stream4_IRQHandler
  .word ETH_IRQHandler
  .word ETH_WKUP_IRQHandler
  .word CAN2_TX_IRQHandler
  .word CAN2_RX0_IRQHandler
  .word CAN2_RX1_IRQHandler
  .word CAN2_SCE_IRQHandler
  .word OTG_FS_IRQHandler
  .word DMA2_Stream5_IRQHandler
  .word DMA2_Stream6_IRQHandler
  .word DMA2_Stream7_IRQHandler
  .word USART6_IRQHandler
  .word I2C3_EV_IRQHandler
  .word I2C3_ER_IRQHandler
  .word OTG_HS_EP1_OUT_IRQHandler
  .word OTG_HS_EP1_IN_IRQHandler
  .word OTG_HS_WKUP_IRQHandler
  .word OTG_HS_IRQHandler
  .word DCMI_IRQHandler
  .word CRYP_IRQHandler
  .word HASH_RNG_IRQHandler
  .word FPU_IRQHandler
# 239 "startup_stm32f4xx.S"
   .weak NMI_Handler
   .thumb_set NMI_Handler,Default_Handler00

   .weak HardFault_Handler
   .thumb_set HardFault_Handler,Default_Handler01

   .weak MemManage_Handler
   .thumb_set MemManage_Handler,Default_Handler02

   .weak BusFault_Handler
   .thumb_set BusFault_Handler,Default_Handler03

   .weak UsageFault_Handler
   .thumb_set UsageFault_Handler,Default_Handler04

   .weak SVC_Handler
   .thumb_set SVC_Handler,Default_Handler05

   .weak DebugMon_Handler
   .thumb_set DebugMon_Handler,Default_Handler06

   .weak PendSV_Handler
   .thumb_set PendSV_Handler,Default_Handler07

   .weak SysTick_Handler
   .thumb_set SysTick_Handler,Default_Handler08

   .weak WWDG_IRQHandler
   .thumb_set WWDG_IRQHandler,Default_Handler09

   .weak PVD_IRQHandler
   .thumb_set PVD_IRQHandler,Default_Handler10

   .weak TAMP_STAMP_IRQHandler
   .thumb_set TAMP_STAMP_IRQHandler,Default_Handler11

   .weak RTC_WKUP_IRQHandler
   .thumb_set RTC_WKUP_IRQHandler,Default_Handler12

   .weak FLASH_IRQHandler
   .thumb_set FLASH_IRQHandler,Default_Handler13

   .weak RCC_IRQHandler
   .thumb_set RCC_IRQHandler,Default_Handler14

   .weak EXTI0_IRQHandler
   .thumb_set EXTI0_IRQHandler,Default_Handler15

   .weak EXTI1_IRQHandler
   .thumb_set EXTI1_IRQHandler,Default_Handler16

   .weak EXTI2_IRQHandler
   .thumb_set EXTI2_IRQHandler,Default_Handler17

   .weak EXTI3_IRQHandler
   .thumb_set EXTI3_IRQHandler,Default_Handler18

   .weak EXTI4_IRQHandler
   .thumb_set EXTI4_IRQHandler,Default_Handler19
# 320 "startup_stm32f4xx.S"
   .weak ADC_IRQHandler
   .thumb_set ADC_IRQHandler,Default_Handler27
# 332 "startup_stm32f4xx.S"
   .weak CAN1_SCE_IRQHandler
   .thumb_set CAN1_SCE_IRQHandler,Default_Handler31

   .weak EXTI9_5_IRQHandler
   .thumb_set EXTI9_5_IRQHandler,Default_Handler32

   .weak TIM1_BRK_TIM9_IRQHandler
   .thumb_set TIM1_BRK_TIM9_IRQHandler,Default_Handler33

   .weak TIM1_UP_TIM10_IRQHandler
   .thumb_set TIM1_UP_TIM10_IRQHandler,Default_Handler34

   .weak TIM1_TRG_COM_TIM11_IRQHandler
   .thumb_set TIM1_TRG_COM_TIM11_IRQHandler,Default_Handler35

   .weak TIM1_CC_IRQHandler
   .thumb_set TIM1_CC_IRQHandler,Default_Handler36

   .weak TIM2_IRQHandler
   .thumb_set TIM2_IRQHandler,Default_Handler37

   .weak TIM3_IRQHandler
   .thumb_set TIM3_IRQHandler,Default_Handler38

   .weak TIM4_IRQHandler
   .thumb_set TIM4_IRQHandler,Default_Handler39

   .weak I2C1_EV_IRQHandler
   .thumb_set I2C1_EV_IRQHandler,Default_Handler40

   .weak I2C1_ER_IRQHandler
   .thumb_set I2C1_ER_IRQHandler,Default_Handler41

   .weak I2C2_EV_IRQHandler
   .thumb_set I2C2_EV_IRQHandler,Default_Handler42

   .weak I2C2_ER_IRQHandler
   .thumb_set I2C2_ER_IRQHandler,Default_Handler43

   .weak SPI1_IRQHandler
   .thumb_set SPI1_IRQHandler,Default_Handler44

   .weak SPI2_IRQHandler
   .thumb_set SPI2_IRQHandler,Default_Handler45
# 386 "startup_stm32f4xx.S"
   .weak EXTI15_10_IRQHandler
   .thumb_set EXTI15_10_IRQHandler,Default_Handler49

   .weak RTC_Alarm_IRQHandler
   .thumb_set RTC_Alarm_IRQHandler,Default_Handler50

   .weak OTG_FS_WKUP_IRQHandler
   .thumb_set OTG_FS_WKUP_IRQHandler,Default_Handler51

   .weak TIM8_BRK_TIM12_IRQHandler
   .thumb_set TIM8_BRK_TIM12_IRQHandler,Default_Handler52

   .weak TIM8_UP_TIM13_IRQHandler
   .thumb_set TIM8_UP_TIM13_IRQHandler,Default_Handler53

   .weak TIM8_TRG_COM_TIM14_IRQHandler
   .thumb_set TIM8_TRG_COM_TIM14_IRQHandler,Default_Handler54

   .weak TIM8_CC_IRQHandler
   .thumb_set TIM8_CC_IRQHandler,Default_Handler55

   .weak DMA1_Stream7_IRQHandler
   .thumb_set DMA1_Stream7_IRQHandler,Default_Handler56

   .weak FSMC_IRQHandler
   .thumb_set FSMC_IRQHandler,Default_Handler57

   .weak SDIO_IRQHandler
   .thumb_set SDIO_IRQHandler,Default_Handler58

   .weak TIM5_IRQHandler
   .thumb_set TIM5_IRQHandler,Default_Handler59

   .weak SPI3_IRQHandler
   .thumb_set SPI3_IRQHandler,Default_Handler60







   .weak TIM6_DAC_IRQHandler
   .thumb_set TIM6_DAC_IRQHandler,Default_Handler63

   .weak TIM7_IRQHandler
   .thumb_set TIM7_IRQHandler,Default_Handler64
# 449 "startup_stm32f4xx.S"
   .weak ETH_IRQHandler
   .thumb_set ETH_IRQHandler,Default_Handler70

   .weak ETH_WKUP_IRQHandler
   .thumb_set ETH_WKUP_IRQHandler,Default_Handler71
# 464 "startup_stm32f4xx.S"
   .weak CAN2_SCE_IRQHandler
   .thumb_set CAN2_SCE_IRQHandler,Default_Handler75

   .weak OTG_FS_IRQHandler
   .thumb_set OTG_FS_IRQHandler,Default_Handler76
# 482 "startup_stm32f4xx.S"
   .weak I2C3_EV_IRQHandler
   .thumb_set I2C3_EV_IRQHandler,Default_Handler81

   .weak I2C3_ER_IRQHandler
   .thumb_set I2C3_ER_IRQHandler,Default_Handler82

   .weak OTG_HS_EP1_OUT_IRQHandler
   .thumb_set OTG_HS_EP1_OUT_IRQHandler,Default_Handler83

   .weak OTG_HS_EP1_IN_IRQHandler
   .thumb_set OTG_HS_EP1_IN_IRQHandler,Default_Handler84

   .weak OTG_HS_WKUP_IRQHandler
   .thumb_set OTG_HS_WKUP_IRQHandler,Default_Handler85

   .weak OTG_HS_IRQHandler
   .thumb_set OTG_HS_IRQHandler,Default_Handler86

   .weak DCMI_IRQHandler
   .thumb_set DCMI_IRQHandler,Default_Handler87

   .weak CRYP_IRQHandler
   .thumb_set CRYP_IRQHandler,Default_Handler88

   .weak HASH_RNG_IRQHandler
   .thumb_set HASH_RNG_IRQHandler,Default_Handler89

   .weak FPU_IRQHandler
   .thumb_set FPU_IRQHandler,Default_Handler90
