/* STM32F410xx Device Specific Header */

#ifndef STM32F410XX_H
#define STM32F410XX_H

#include <stdint.h>

#define __STM32F4_DEVICE

/* Peripheral Memory Map */
#define FLASH_BASE            0x08000000UL
#define SRAM_BASE             0x20000000UL
#define PERIPH_BASE           0x40000000UL
#define SRAM_BB_BASE          0x22000000UL
#define PERIPH_BB_BASE        0x42000000UL
#define BKPSRAM_BASE          0x40024000UL

/* APB1 peripherals */
#define TIM2_BASE             (PERIPH_BASE + 0x00000000UL)
#define TIM3_BASE             (PERIPH_BASE + 0x00000400UL)
#define TIM4_BASE             (PERIPH_BASE + 0x00000800UL)
#define TIM5_BASE             (PERIPH_BASE + 0x00000C00UL)
#define TIM6_BASE             (PERIPH_BASE + 0x00001000UL)
#define TIM7_BASE             (PERIPH_BASE + 0x00001400UL)
#define RTC_BASE              (PERIPH_BASE + 0x00002800UL)
#define WWDG_BASE             (PERIPH_BASE + 0x00002C00UL)
#define IWDG_BASE             (PERIPH_BASE + 0x00003000UL)
#define SPI2_BASE             (PERIPH_BASE + 0x00003800UL)
#define SPI3_BASE             (PERIPH_BASE + 0x00003C00UL)
#define USART2_BASE           (PERIPH_BASE + 0x00004400UL)
#define USART3_BASE           (PERIPH_BASE + 0x00004800UL)
#define UART4_BASE            (PERIPH_BASE + 0x00004C00UL)
#define UART5_BASE            (PERIPH_BASE + 0x00005000UL)
#define I2C1_BASE             (PERIPH_BASE + 0x00005400UL)
#define I2C2_BASE             (PERIPH_BASE + 0x00005800UL)
#define I2C3_BASE             (PERIPH_BASE + 0x00005C00UL)
#define CAN1_BASE             (PERIPH_BASE + 0x00006400UL)
#define CAN2_BASE             (PERIPH_BASE + 0x00006800UL)
#define PWR_BASE              (PERIPH_BASE + 0x00007000UL)
#define DAC_BASE              (PERIPH_BASE + 0x00007400UL)

/* APB2 peripherals */
#define TIM1_BASE             (PERIPH_BASE + 0x00010000UL)
#define TIM8_BASE             (PERIPH_BASE + 0x00010400UL)
#define USART1_BASE           (PERIPH_BASE + 0x00011000UL)
#define USART6_BASE           (PERIPH_BASE + 0x00011400UL)
#define ADC1_BASE             (PERIPH_BASE + 0x00012000UL)
#define ADC2_BASE             (PERIPH_BASE + 0x00012100UL)
#define ADC3_BASE             (PERIPH_BASE + 0x00012200UL)
#define SDIO_BASE             (PERIPH_BASE + 0x00012C00UL)
#define SPI1_BASE             (PERIPH_BASE + 0x00013000UL)
#define SYSCFG_BASE           (PERIPH_BASE + 0x00013800UL)
#define EXTI_BASE             (PERIPH_BASE + 0x00013C00UL)
#define TIM9_BASE             (PERIPH_BASE + 0x00014000UL)
#define TIM10_BASE            (PERIPH_BASE + 0x00014400UL)
#define TIM11_BASE            (PERIPH_BASE + 0x00014800UL)

/* AHB1 peripherals */
#define GPIOA_BASE            (PERIPH_BASE + 0x00020000UL)
#define GPIOB_BASE            (PERIPH_BASE + 0x00020400UL)
#define GPIOC_BASE            (PERIPH_BASE + 0x00020800UL)
#define GPIOH_BASE            (PERIPH_BASE + 0x00021C00UL)
#define CRC_BASE              (PERIPH_BASE + 0x00023000UL)
#define RCC_BASE              (PERIPH_BASE + 0x00023800UL)
#define FLASH_R_BASE          (PERIPH_BASE + 0x00023C00UL)
#define DMA1_BASE             (PERIPH_BASE + 0x00026000UL)
#define DMA2_BASE             (PERIPH_BASE + 0x00026400UL)
#define ETH_MAC_BASE          (PERIPH_BASE + 0x00028000UL)
#define ETH_MAC_Tx_BASE       (PERIPH_BASE + 0x00028040UL)
#define ETH_MAC_Tx_Desc_BASE  (PERIPH_BASE + 0x00028100UL)
#define ETH_MAC_Rx_Desc_Base  (PERIPH_BASE + 0x00028280UL)
#define ETH_MAC_Rx_Base       (PERIPH_BASE + 0x00028300UL)
#define DMA2D_BASE            (PERIPH_BASE + 0x0002B000UL)

/* AHB2 peripherals */
#define OTG_FS_BASE           (PERIPH_BASE + 0x00050000UL)

/* AHB3 peripherals */
#define OTG_HS_BASE           (PERIPH_BASE + 0x00060000UL)
#define DCMI_BASE             (PERIPH_BASE + 0x00050000UL)

/* Debug MCU registers */
#define DBGMCU_BASE           0xE0042000UL

/* System Control Block */
#define SCS_BASE              0xE000E000UL
#define ITM_BASE              0xE0000000UL
#define DWT_BASE              0xE0001000UL
#define TPI_BASE              0xE0040000UL
#define CoreDebug_BASE        0xE0040000UL

/* Peripheral declarations */
#define TIM2                 ((TIM_TypeDef *) TIM2_BASE)
#define TIM3                 ((TIM_TypeDef *) TIM3_BASE)
#define TIM4                 ((TIM_TypeDef *) TIM4_BASE)
#define TIM5                 ((TIM_TypeDef *) TIM5_BASE)
#define TIM6                 ((TIM_TypeDef *) TIM6_BASE)
#define TIM7                 ((TIM_TypeDef *) TIM7_BASE)
#define RTC                  ((RTC_TypeDef *) RTC_BASE)
#define WWDG                 ((WWDG_TypeDef *) WWDG_BASE)
#define IWDG                 ((IWDG_TypeDef *) IWDG_BASE)
#define SPI2                 ((SPI_TypeDef *) SPI2_BASE)
#define SPI3                 ((SPI_TypeDef *) SPI3_BASE)
#define USART2               ((USART_TypeDef *) USART2_BASE)
#define USART3               ((USART_TypeDef *) USART3_BASE)
#define UART4                ((USART_TypeDef *) UART4_BASE)
#define UART5                ((USART_TypeDef *) UART5_BASE)
#define I2C1                 ((I2C_TypeDef *) I2C1_BASE)
#define I2C2                 ((I2C_TypeDef *) I2C2_BASE)
#define I2C3                 ((I2C_TypeDef *) I2C3_BASE)
#define CAN1                 ((CAN_TypeDef *) CAN1_BASE)
#define CAN2                 ((CAN_TypeDef *) CAN2_BASE)
#define PWR                  ((PWR_TypeDef *) PWR_BASE)
#define DAC                  ((DAC_TypeDef *) DAC_BASE)
#define TIM1                 ((TIM_TypeDef *) TIM1_BASE)
#define TIM8                 ((TIM_TypeDef *) TIM8_BASE)
#define USART1               ((USART_TypeDef *) USART1_BASE)
#define USART6               ((USART_TypeDef *) USART6_BASE)
#define ADC1                 ((ADC_TypeDef *) ADC1_BASE)
#define ADC2                 ((ADC_TypeDef *) ADC2_BASE)
#define ADC3                 ((ADC_TypeDef *) ADC3_BASE)
#define SDIO                 ((SDIO_TypeDef *) SDIO_BASE)
#define SPI1                 ((SPI_TypeDef *) SPI1_BASE)
#define SYSCFG               ((SYSCFG_TypeDef *) SYSCFG_BASE)
#define EXTI                 ((EXTI_TypeDef *) EXTI_BASE)
#define TIM9                 ((TIM_TypeDef *) TIM9_BASE)
#define TIM10                ((TIM_TypeDef *) TIM10_BASE)
#define TIM11                ((TIM_TypeDef *) TIM11_BASE)
#define GPIOA                ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB                ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC                ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOH                ((GPIO_TypeDef *) GPIOH_BASE)
#define CRC                  ((CRC_TypeDef *) CRC_BASE)
#define RCC                  ((RCC_TypeDef *) RCC_BASE)
#define FLASH                ((FLASH_TypeDef *) FLASH_R_BASE)
#define DMA1                 ((DMA_TypeDef *) DMA1_BASE)
#define DMA2                 ((DMA_TypeDef *) DMA2_BASE)
#define ETH                  ((ETH_TypeDef *) ETH_MAC_BASE)
#define DMA2D                ((DMA2D_TypeDef *) DMA2D_BASE)
#define OTG_FS               ((USB_OTG_GlobalTypeDef *) OTG_FS_BASE)
#define OTG_HS               ((USB_OTG_GlobalTypeDef *) OTG_HS_BASE)
#define DCMI                 ((DCMI_TypeDef *) DCMI_BASE)
#define DBGMCU               ((DBGMCU_TypeDef *) DBGMCU_BASE)

/* Core Debug registers */
#define CoreDebug            ((CoreDebug_TypeDef *) CoreDebug_BASE)
#define DWT                  ((DWT_TypeDef *) DWT_BASE)
#define ITM                  ((ITM_TypeDef *) ITM_BASE)
#define TPI                  ((TPI_TypeDef *) TPI_BASE)

/* Typedef definitions */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    uint32_t RESERVED0;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    volatile uint32_t AHB3RSTR;
    uint32_t RESERVED1;
    volatile uint32_t APB1RSTR;
    volatile uint32_t APB2RSTR;
    uint32_t RESERVED2[2];
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t AHB3ENR;
    uint32_t RESERVED3;
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
    uint32_t RESERVED4[2];
    volatile uint32_t AHB1LPENR;
    volatile uint32_t AHB2LPENR;
    volatile uint32_t AHB3LPENR;
    uint32_t RESERVED5;
    volatile uint32_t APB1LPENR;
    volatile uint32_t APB2LPENR;
    uint32_t RESERVED6[2];
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
    uint32_t RESERVED7[2];
    volatile uint32_t SSCGR;
    volatile uint32_t PLLI2SCFGR;
} RCC_TypeDef;

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;

/* RCC bit definitions */
#define RCC_CR_HSEON                (1 << 16)
#define RCC_CR_HSERDY              (1 << 17)
#define RCC_CR_PLLON                (1 << 24)
#define RCC_CR_PLLRDY              (1 << 25)

#define RCC_CFGR_SW_HSE            (1 << 0)
#define RCC_CFGR_SWS_HSE           (1 << 2)
#define RCC_CFGR_SW_PLL            (2 << 0)
#define RCC_CFGR_SWS_PLL           (2 << 2)

#define RCC_PLLCFGR_PLLSRC_HSE     (1 << 22)

/* System Core Clock update function */
extern uint32_t SystemCoreClock;
void SystemCoreClockUpdate(void);

#endif /* STM32F410XX_H */
