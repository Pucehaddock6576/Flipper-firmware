/* Flipper Next-Gen OS - Main Target Implementation */

#include "stm32f410xx.h"
#include "furi.h"
#include "furi_hal.h"
#include "version.h"

// STM32F4xx startup
extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

// System clock configuration
void SystemClock_Config(void) {
    // Configure system clock to 64MHz for STM32F410
    // This is a simplified version - real implementation would be more complex
    RCC->CR |= RCC_CR_HSEON;
    while(!(RCC->CR & RCC_CR_HSERDY));
    
    RCC->CFGR |= RCC_CFGR_SW_HSE;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE);
    
    // Configure PLL
    RCC->PLLCFGR = (8 << RCC_PLLCFGR_PLLM_Pos) |  // PLLM = 8
                   (336 << RCC_PLLCFGR_PLLN_Pos) | // PLLN = 336
                   (0 << RCC_PLLCFGR_PLLP_Pos) |  // PLLP = 2
                   (7 << RCC_PLLCFGR_PLLQ_Pos) |  // PLLQ = 7
                   RCC_PLLCFGR_PLLSRC_HSE;
    
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));
    
    // Switch to PLL
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
    
    // Update system core clock
    SystemCoreClockUpdate();
}

// Reset handler
void Reset_Handler(void) {
    // Initialize data section
    uint32_t* src = &_sidata;
    uint32_t* dst = &_sdata;
    while(dst < &_edata) {
        *dst++ = *src++;
    }
    
    // Zero BSS section
    dst = &_sbss;
    while(dst < &_ebss) {
        *dst++ = 0;
    }
    
    // Configure system clock
    SystemClock_Config();
    
    // Initialize Furi HAL
    furi_hal_init();
    
    // Initialize Furi core
    furi_init();
    
    // Start main system loop
    furi_run();
}

// Default exception handlers
void HardFault_Handler(void) {
    furi_crash("Hard Fault");
}

void MemManage_Handler(void) {
    furi_crash("Memory Management Fault");
}

void BusFault_Handler(void) {
    furi_crash("Bus Fault");
}

void UsageFault_Handler(void) {
    furi_crash("Usage Fault");
}

void SVC_Handler(void) {
    // System service call handler
}

void PendSV_Handler(void) {
    // Pendable service call handler
}

void SysTick_Handler(void) {
    // System tick handler
    furi_tick();
}

// Weak symbols for optional handlers
void NMI_Handler(void) __attribute__((weak));
void DebugMon_Handler(void) __attribute__((weak));
void WWDG_IRQHandler(void) __attribute__((weak));
void PVD_IRQHandler(void) __attribute__((weak));
void TAMP_STAMP_IRQHandler(void) __attribute__((weak));
void RTC_WKUP_IRQHandler(void) __attribute__((weak));
void FLASH_IRQHandler(void) __attribute__((weak));
void RCC_IRQHandler(void) __attribute__((weak));
void EXTI0_IRQHandler(void) __attribute__((weak));
void EXTI1_IRQHandler(void) __attribute__((weak));
void EXTI2_IRQHandler(void) __attribute__((weak));
void EXTI3_IRQHandler(void) __attribute__((weak));
void EXTI4_IRQHandler(void) __attribute__((weak));
void DMA1_Stream0_IRQHandler(void) __attribute__((weak));
void DMA1_Stream1_IRQHandler(void) __attribute__((weak));
void DMA1_Stream2_IRQHandler(void) __attribute__((weak));
void DMA1_Stream3_IRQHandler(void) __attribute__((weak));
void DMA1_Stream4_IRQHandler(void) __attribute__((weak));
void DMA1_Stream5_IRQHandler(void) __attribute__((weak));
void DMA1_Stream6_IRQHandler(void) __attribute__((weak));
void ADC_IRQHandler(void) __attribute__((weak));
void TIM1_BRK_TIM9_IRQHandler(void) __attribute__((weak));
void TIM1_UP_TIM10_IRQHandler(void) __attribute__((weak));
void TIM1_TRG_COM_TIM11_IRQHandler(void) __attribute__((weak));
void TIM1_CC_IRQHandler(void) __attribute__((weak));
void TIM2_IRQHandler(void) __attribute__((weak));
void TIM3_IRQHandler(void) __attribute__((weak));
void TIM4_IRQHandler(void) __attribute__((weak));
void I2C1_EV_IRQHandler(void) __attribute__((weak));
void I2C1_ER_IRQHandler(void) __attribute__((weak));
void I2C2_EV_IRQHandler(void) __attribute__((weak));
void I2C2_ER_IRQHandler(void) __attribute__((weak));
void SPI1_IRQHandler(void) __attribute__((weak));
void SPI2_IRQHandler(void) __attribute__((weak));
void USART1_IRQHandler(void) __attribute__((weak));
void USART2_IRQHandler(void) __attribute__((weak));
void USART3_IRQHandler(void) __attribute__((weak));
void EXTI15_10_IRQHandler(void) __attribute__((weak));
void RTC_Alarm_IRQHandler(void) __attribute__((weak));
void OTG_FS_WKUP_IRQHandler(void) __attribute__((weak));
void TIM8_BRK_TIM12_IRQHandler(void) __attribute__((weak));
void TIM8_UP_TIM13_IRQHandler(void) __attribute__((weak));
void TIM8_TRG_COM_TIM14_IRQHandler(void) __attribute__((weak));
void TIM8_CC_IRQHandler(void) __attribute__((weak));
void DMA1_Stream7_IRQHandler(void) __attribute__((weak));
void FMC_IRQHandler(void) __attribute__((weak));
void SDIO_IRQHandler(void) __attribute__((weak));
void TIM5_IRQHandler(void) __attribute__((weak));
void SPI3_IRQHandler(void) __attribute__((weak));
void UART4_IRQHandler(void) __attribute__((weak));
void UART5_IRQHandler(void) __attribute__((weak));
void TIM6_DAC_IRQHandler(void) __attribute__((weak));
void TIM7_IRQHandler(void) __attribute__((weak));
void DMA2_Stream0_IRQHandler(void) __attribute__((weak));
void DMA2_Stream1_IRQHandler(void) __attribute__((weak));
void DMA2_Stream2_IRQHandler(void) __attribute__((weak));
void DMA2_Stream3_IRQHandler(void) __attribute__((weak));
void DMA2_Stream4_IRQHandler(void) __attribute__((weak));
void ETH_IRQHandler(void) __attribute__((weak));
void ETH_WKUP_IRQHandler(void) __attribute__((weak));
void CAN2_TX_IRQHandler(void) __attribute__((weak));
void OTG_FS_IRQHandler(void) __attribute__((weak));
void DMA2_Stream5_IRQHandler(void) __attribute__((weak));
void DMA2_Stream6_IRQHandler(void) __attribute__((weak));
void DMA2_Stream7_IRQHandler(void) __attribute__((weak));
void USART6_IRQHandler(void) __attribute__((weak));
void I2C3_EV_IRQHandler(void) __attribute__((weak));
void I2C3_ER_IRQHandler(void) __attribute__((weak));
void OTG_HS_EP1_OUT_IRQHandler(void) __attribute__((weak));
void OTG_HS_EP1_IN_IRQHandler(void) __attribute__((weak));
void OTG_HS_WKUP_IRQHandler(void) __attribute__((weak));
void OTG_HS_IRQHandler(void) __attribute__((weak));
void DCMI_IRQHandler(void) __attribute__((weak));
void FPU_IRQHandler(void) __attribute__((weak));

// Vector table
__attribute__((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    &_estack,                    // 0: Initial Stack Pointer
    Reset_Handler,              // 1: Reset Handler
    NMI_Handler,                // 2: NMI Handler
    HardFault_Handler,          // 3: Hard Fault Handler
    MemManage_Handler,          // 4: MPU Fault Handler
    BusFault_Handler,           // 5: Bus Fault Handler
    UsageFault_Handler,         // 6: Usage Fault Handler
    0,                          // 7: Reserved
    0,                          // 8: Reserved
    0,                          // 9: Reserved
    0,                          // 10: Reserved
    SVC_Handler,                // 11: SVCall Handler
    DebugMon_Handler,           // 12: Debug Monitor Handler
    0,                          // 13: Reserved
    PendSV_Handler,             // 14: PendSV Handler
    SysTick_Handler,            // 15: SysTick Handler
    
    // External Interrupts
    WWDG_IRQHandler,            // Window WatchDog
    PVD_IRQHandler,             // PVD through EXTI Line detection
    TAMP_STAMP_IRQHandler,       // Tamper and TimeStamp
    RTC_WKUP_IRQHandler,         // RTC Wakeup through EXTI
    FLASH_IRQHandler,            // FLASH
    RCC_IRQHandler,             // RCC
    EXTI0_IRQHandler,            // EXTI Line0
    EXTI1_IRQHandler,            // EXTI Line1
    EXTI2_IRQHandler,            // EXTI Line2
    EXTI3_IRQHandler,            // EXTI Line3
    EXTI4_IRQHandler,            // EXTI Line4
    DMA1_Stream0_IRQHandler,     // DMA1 Stream 0
    DMA1_Stream1_IRQHandler,     // DMA1 Stream 1
    DMA1_Stream2_IRQHandler,     // DMA1 Stream 2
    DMA1_Stream3_IRQHandler,     // DMA1 Stream 3
    DMA1_Stream4_IRQHandler,     // DMA1 Stream 4
    DMA1_Stream5_IRQHandler,     // DMA1 Stream 5
    DMA1_Stream6_IRQHandler,     // DMA1 Stream 6
    ADC_IRQHandler,              // ADC1, ADC2 and ADC3s
    TIM1_BRK_TIM9_IRQHandler,    // TIM1 Break and TIM9
    TIM1_UP_TIM10_IRQHandler,    // TIM1 Update and TIM10
    TIM1_TRG_COM_TIM11_IRQHandler, // TIM1 Trigger and Commutation and TIM11
    TIM1_CC_IRQHandler,         // TIM1 Capture Compare
    TIM2_IRQHandler,             // TIM2
    TIM3_IRQHandler,             // TIM3
    TIM4_IRQHandler,             // TIM4
    I2C1_EV_IRQHandler,          // I2C1 Event
    I2C1_ER_IRQHandler,          // I2C1 Error
    I2C2_EV_IRQHandler,          // I2C2 Event
    I2C2_ER_IRQHandler,          // I2C2 Error
    SPI1_IRQHandler,             // SPI1
    SPI2_IRQHandler,             // SPI2
    USART1_IRQHandler,          // USART1
    USART2_IRQHandler,          // USART2
    USART3_IRQHandler,          // USART3
    EXTI15_10_IRQHandler,        // External Line[15:10]s
    RTC_Alarm_IRQHandler,        // RTC Alarm (A and B)
    OTG_FS_WKUP_IRQHandler,      // USB OTG FS Wakeup through EXTI
    TIM8_BRK_TIM12_IRQHandler,   // TIM8 Break and TIM12
    TIM8_UP_TIM13_IRQHandler,    // TIM8 Update and TIM13
    TIM8_TRG_COM_TIM14_IRQHandler, // TIM8 Trigger and Commutation and TIM14
    TIM8_CC_IRQHandler,          // TIM8 Capture Compare
    DMA1_Stream7_IRQHandler,     // DMA1 Stream7
    FMC_IRQHandler,              // FMC
    SDIO_IRQHandler,             // SDIO
    TIM5_IRQHandler,             // TIM5
    SPI3_IRQHandler,             // SPI3
    UART4_IRQHandler,            // UART4
    UART5_IRQHandler,            // UART5
    TIM6_DAC_IRQHandler,         // TIM6 and DAC1&2 underrun errors
    TIM7_IRQHandler,             // TIM7
    DMA2_Stream0_IRQHandler,     // DMA2 Stream 0
    DMA2_Stream1_IRQHandler,     // DMA2 Stream 1
    DMA2_Stream2_IRQHandler,     // DMA2 Stream 2
    DMA2_Stream3_IRQHandler,     // DMA2 Stream 3
    DMA2_Stream4_IRQHandler,     // DMA2 Stream 4
    ETH_IRQHandler,              // Ethernet
    ETH_WKUP_IRQHandler,         // Ethernet Wakeup through EXTI
    CAN2_TX_IRQHandler,          // CAN2 TX
    OTG_FS_IRQHandler,           // USB OTG FS
    DMA2_Stream5_IRQHandler,     // DMA2 Stream 5
    DMA2_Stream6_IRQHandler,     // DMA2 Stream 6
    DMA2_Stream7_IRQHandler,     // DMA2 Stream 7
    USART6_IRQHandler,          // USART6
    I2C3_EV_IRQHandler,          // I2C3 event
    I2C3_ER_IRQHandler,          // I2C3 error
    OTG_HS_EP1_OUT_IRQHandler,   // USB OTG HS End Point 1 Out
    OTG_HS_EP1_IN_IRQHandler,    // USB OTG HS End Point 1 In
    OTG_HS_WKUP_IRQHandler,      // USB OTG HS Wakeup through EXTI
    OTG_HS_IRQHandler,           // USB OTG HS
    DCMI_IRQHandler,             // DCMI
    0,                          // Reserved
    FPU_IRQHandler,              // FPU
};
