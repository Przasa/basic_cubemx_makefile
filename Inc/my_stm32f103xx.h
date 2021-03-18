/*
 * stm32f103xx.h
 *
 *  Created on: Jan 16, 2021
 *      Author: dj6dvj
 */

#ifndef STM32F103XX_H_
#define STM32F103XX_H_


#include<stddef.h>
#include<stdint.h>

#define __vo volatile
#define __weak __attribute__((weak))

/*
 * base addresses of Flash and SRAM memories
 */

#define FLASH_BASEADDR						0x08000000U   		/*!<explain this macro briefly here  */
#define SRAM1_BASEADDR						0x20000000U  		/*!<explain this macro briefly here  */
//#define SRAM2_BASEADDR					0x2001C000U 		/*!<explain this macro briefly here  */
//#define ROM_BASEADDR						0x1FFF0000U
#define SRAM 								      SRAM1_BASEADDR
/*
 * AHBx and APBx Bus Peripheral base addresses
 */

#define PERIPH_BASEADDR 				  		0x40000000U
#define APB1PERIPH_BASEADDR						PERIPH_BASEADDR
#define APB2PERIPH_BASEADDR						0x40010000U
#define AHBPERIPH_BASEADDR						0x40018000U
//#define AHB2PERIPH_BASEADDR					0x50000000U     //TEGO NIE MAMY
/*
 * Base addresses of peripherals which are hanging on AHB1 bus
 * TODO : Complete for all other peripherals
 */
#define RCC_BASEADDR             0x40021000U
#define EXTI_BASEADDR            0x40010400U
// #define RCC_BASEADDR             (AHBPERIPH_BASEADDR + 0x9000U)
// #define EXTI_BASEADDR            (APB2PERIPH_BASEADDR + 0x0400)

#define GPIOA_BASEADDR           (APB2PERIPH_BASEADDR + 0x0800)
#define GPIOB_BASEADDR           (APB2PERIPH_BASEADDR + 0x0C00)
#define GPIOC_BASEADDR 					 (APB2PERIPH_BASEADDR + 0x1000)
#define GPIOD_BASEADDR 					 (APB2PERIPH_BASEADDR + 0x1400)
#define GPIOE_BASEADDR 					 (APB2PERIPH_BASEADDR + 0x1800)
#define GPIOF_BASEADDR 					 (APB2PERIPH_BASEADDR + 0x1C00)
#define GPIOG_BASEADDR 					 (APB2PERIPH_BASEADDR + 0x2000)
#define AFIO_BASEADDR					   (APB2PERIPH_BASEADDR)



/*
 * Note : Registers of a peripheral are specific to MCU
 * e.g : Number of Registers of SPI peripheral of STM32F4x family of MCUs may be different(more or less)
 * Compared to number of registers of SPI peripheral of STM32Lx or STM32F0x family of MCUs
 * Please check your Device RM
 */

typedef struct
{
	__vo uint32_t CRL;                        /*!< GPIO port mode register,                    	Address offset: 0x00      */
	__vo uint32_t CRH;                       /*!< TODO,     										Address offset: 0x04      */
	__vo uint32_t IDR;
	__vo uint32_t ODR;
	__vo uint32_t BSRR;
	__vo uint32_t BRR;
	__vo uint32_t LCKR;
}GPIO_RegDef_t;

/*
 * peripheral register definition structure for AFIO
 */
typedef struct
{
  __vo uint32_t EVCR;
  __vo uint32_t MAPR;
  __vo uint32_t EXTICR1;
  __vo uint32_t EXTICR2;
  __vo uint32_t EXTICR3;
  __vo uint32_t EXTICR4;
  __vo uint32_t MAPR2;

} AFIO_RegDef_t;


/*
 * peripheral register definition structure for RCC
 */
typedef struct
{
  __vo uint32_t CR;
  __vo uint32_t CFGR;
  __vo uint32_t CIR;
  __vo uint32_t APB2RSTR;
  __vo uint32_t APB1RSTR;
  __vo uint32_t AHBENR;
  __vo uint32_t APB2ENR;
  __vo uint32_t APB1ENR;
  __vo uint32_t BDCR;
  __vo uint32_t CSR;
  __vo uint32_t AHBSTR;
  __vo uint32_t CFGR2;
} RCC_RegDef_t;

/*
 * peripheral register definition structure for EXTI
 */
typedef struct
{
  __vo uint32_t EXTI_IMR;
  __vo uint32_t EXTI_EMR;
  __vo uint32_t EXTI_RTSR;
  __vo uint32_t EXTI_FTSR;
  __vo uint32_t EXTI_SWIER;
  __vo uint32_t EXTI_PR;
} EXTI_RegDef_t;

/*
 * peripheral definitions ( Peripheral base addresses typecasted to xxx_RegDef_t)
 */


//::::::::::::::::::::::::::::  DEFINITIONS :::::::::::::::::::::::::::::::::::::::://
#define GPIOA  				((GPIO_RegDef_t*)GPIOA_BASEADDR)
#define GPIOB  				((GPIO_RegDef_t*)GPIOB_BASEADDR)
#define GPIOC  				((GPIO_RegDef_t*)GPIOC_BASEADDR)
#define GPIOD  				((GPIO_RegDef_t*)GPIOD_BASEADDR)
#define GPIOE  				((GPIO_RegDef_t*)GPIOE_BASEADDR)
#define GPIOF  				((GPIO_RegDef_t*)GPIOF_BASEADDR)

#define RCC 				  ((RCC_RegDef_t*)RCC_BASEADDR)
#define EXTI				  ((EXTI_RegDef_t*)EXTI_BASEADDR)
// #define SYSCFG				((SYSCFG_RegDef_t*)SYSCFG_BASEADDR)

/********************  Bit definition for RCC_CR register  ********************/
#define RCC_CFGR_SWS        (0x3UL << 2U)
#define RCC_CFGR_PLLMULL    (0xFUL << 18U)
#define RCC_CFGR_PLLSRC     (0x1UL << 16U)
#define RCC_CFGR_PLLXTPRE   (0x1UL << 17U)   
#define RCC_CFGR_HPRE       (0xFUL << 4U)


/*
 * Clock Enable Macros for GPIOx peripherals
 */
#define GPIOA_PCLK_EN()   (RCC->APB2ENR |= (1 << 2))
#define GPIOB_PCLK_EN()		(RCC->APB2ENR |= (1 << 3))
#define GPIOC_PCLK_EN()		(RCC->APB2ENR |= (1 << 4))
#define GPIOD_PCLK_EN()		(RCC->APB2ENR |= (1 << 5))
#define GPIOE_PCLK_EN()		(RCC->APB2ENR |= (1 << 6))

#define AFIO_PCLK_EN()		(RCC->APB2ENR |= (1 << 0))

/******************** MACROS  *******************************************/

#define ENABLE 				1
#define DISABLE 			0
#define SET 				ENABLE
#define RESET 				DISABLE
#define GPIO_PIN_SET        SET
#define GPIO_PIN_RESET      RESET
#define FLAG_RESET         RESET
#define FLAG_SET 			SET

// #include "gpio_handler.h"
//#include "stm32f407xx_gpio_driver.h"

#endif /* STM32F103XX_H_ */
