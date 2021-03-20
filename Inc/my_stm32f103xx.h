/*
 * stm32f103xx.h
 *
 *  Created on: Jan 16, 2021
 *      Author: dj6dvj (Piotr Rzasa)
 */

#ifndef STM32F103XX_H_
#define STM32F103XX_H_


#include<stddef.h>
#include<stdint.h>

#define __vo volatile
#define __weak __attribute__((weak))

//TODO 19.03.2021: crete nice looking comments (based on Nikarian file)

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
#define SPI_BASEADDR             0x40013000U
#define NVIC_BASEADDR            0xE000E100
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
 * peripheral register definition structure for SPI
 */
typedef struct
{
	__vo uint32_t CR1;        /*!< TODO,     										Address offset: 0x00 */
	__vo uint32_t CR2;        /*!< TODO,     										Address offset: 0x04 */
	__vo uint32_t SR;         /*!< TODO,     										Address offset: 0x08 */
	__vo uint32_t DR;         /*!< TODO,     										Address offset: 0x0C */
	__vo uint32_t CRCPR;      /*!< TODO,     										Address offset: 0x10 */
	__vo uint32_t RXCRCR;     /*!< TODO,     										Address offset: 0x14 */
	__vo uint32_t TXCRCR;     /*!< TODO,     										Address offset: 0x18 */
	__vo uint32_t I2SCFGR;    /*!< TODO,     										Address offset: 0x1C */
	__vo uint32_t I2SPR;      /*!< TODO,     										Address offset: 0x20 */
} SPI_RegDef_t;


//TODO: 20.02.2021: moze lepiej wskazac zerowe rejestry i obsluzyc obliczanie rejetrow w driverach?
//albo tak:
  // 	__vo uint32_t LCKR;
  // 	__vo uint32_t AFR[2];					 /*!< AFR[0] : GPIO alternate function low register, AF[1] : GPIO alternate function high register    		Address offset: 0x20-0x24 */
  // }GPIO_RegDef_t;


typedef struct
{
  __vo uint32_t ISER0;
  __vo uint32_t ISER1;
  __vo uint32_t ISER2;
  __vo uint32_t ICER0;
  __vo uint32_t ICER1;
  __vo uint32_t ICER2;
  __vo uint32_t ISPR0;
  __vo uint32_t ISPR1;
  __vo uint32_t ISPR2;
  __vo uint32_t ICPR0;
  __vo uint32_t ICPR1;
  __vo uint32_t ICPR2;
  __vo uint32_t IABR0;
  __vo uint32_t IABR1;
  __vo uint32_t IABR2;
  __vo uint32_t IABR3;
  __vo uint32_t IPR0;
  __vo uint32_t IPR1;
  __vo uint32_t IPR2;
  __vo uint32_t IPR3;
  __vo uint32_t IPR4;
  __vo uint32_t IPR5;
  __vo uint32_t IPR6;
  __vo uint32_t IPR7;
  __vo uint32_t IPR8;
  __vo uint32_t IPR9;
  __vo uint32_t IPR10;  
  __vo uint32_t IPR11;
  __vo uint32_t IPR12;
  __vo uint32_t IPR13;
  __vo uint32_t IPR14;
  __vo uint32_t IPR16;
  __vo uint32_t IPR17;
  __vo uint32_t IPR18;
  __vo uint32_t IPR19;
  __vo uint32_t IPR20;
  __vo uint32_t STIR;
} NVIC_RegDef_t;

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
#define SPI				    ((SPI_RegDef_t*)SPI_BASEADDR)
#define AFIO          ((AFIO_RegDef_t*)AFIO_BASEADDR)
#define NVIC          ((NVIC_RegDef_t*)NVIC_BASEADDR)
// #define SYSCFG				((SYSCFG_RegDef_t*)SYSCFG_BASEADDR)

/********************  Bit definition for RCC_CR register  ********************/
#define RCC_CFGR_SWS        (0x3UL << 2U)
#define RCC_CFGR_PLLMULL    (0xFUL << 18U)
#define RCC_CFGR_PLLSRC     (0x1UL << 16U)
#define RCC_CFGR_PLLXTPRE   (0x1UL << 17U)   
#define RCC_CFGR_HPRE       (0xFUL << 4U)


/********************  MACROS  ************************************************/
/*
 * Clock Enable/Disable/Read Macros for GPIOx peripherals
 */
#define GPIOA_PCLK_EN()   (RCC->APB2ENR |= (1 << 2))
#define GPIOB_PCLK_EN()		(RCC->APB2ENR |= (1 << 3))
#define GPIOC_PCLK_EN()		(RCC->APB2ENR |= (1 << 4))
#define GPIOD_PCLK_EN()		(RCC->APB2ENR |= (1 << 5))
#define GPIOE_PCLK_EN()		(RCC->APB2ENR |= (1 << 6))
#define AFIO_PCLK_EN()		(RCC->APB2ENR |= (1 << 0))
#define SPI_PCLK_EN() 		(RCC->APB2ENR |= (1 << 12))

#define GPIOA_PCLK_DI()   (RCC->APB2ENR &= ~(1 << 2))
#define GPIOB_PCLK_DI()		(RCC->APB2ENR &= ~(1 << 3))
#define GPIOC_PCLK_DI()		(RCC->APB2ENR &= ~(1 << 4))
#define GPIOD_PCLK_DI()		(RCC->APB2ENR &= ~(1 << 5))
#define GPIOE_PCLK_DI()		(RCC->APB2ENR &= ~(1 << 6))
#define AFIO_PCLK_DI()		(RCC->APB2ENR &= ~(1 << 0))
#define SPI_PCLK_DI()		  (RCC->APB2ENR &= ~(1 << 12))

#define GPIOA_PCLK_GET()  ((RCC->APB2ENR & (1 << 2)) >> 2)
#define GPIOB_PCLK_GET()	((RCC->APB2ENR & (1 << 3)) >> 3)
#define GPIOC_PCLK_GET()	((RCC->APB2ENR & (1 << 4)) >> 4)
#define GPIOD_PCLK_GET()	((RCC->APB2ENR & (1 << 5)) >> 5)
#define GPIOE_PCLK_GET()	((RCC->APB2ENR & (1 << 6)) >> 6)
#define AFIO_PCLK_GET()	  ((RCC->APB2ENR & (1 << 0)) >> 0)
#define SPI_PCLK_GET()	  ((RCC->APB2ENR & (1 << 12)) >> 12)


//TODO 20.03.21:
// /*
//  *  Macros to reset GPIOx peripherals
//  */
// #define GPIOA_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 0)); (RCC->AHB1RSTR &= ~(1 << 0)); }while(0)
// #define GPIOB_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 1)); (RCC->AHB1RSTR &= ~(1 << 1)); }while(0)
// #define GPIOC_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 2)); (RCC->AHB1RSTR &= ~(1 << 2)); }while(0)
// #define GPIOD_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 3)); (RCC->AHB1RSTR &= ~(1 << 3)); }while(0)
// #define GPIOE_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 4)); (RCC->AHB1RSTR &= ~(1 << 4)); }while(0)
// #define GPIOF_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 5)); (RCC->AHB1RSTR &= ~(1 << 5)); }while(0)
// #define GPIOG_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 6)); (RCC->AHB1RSTR &= ~(1 << 6)); }while(0)
// #define GPIOH_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 7)); (RCC->AHB1RSTR &= ~(1 << 7)); }while(0)
// #define GPIOI_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 8)); (RCC->AHB1RSTR &= ~(1 << 8)); }while(0)


// /*
//  *  returns port code for given GPIOx base address
//  */
// /*
//  * This macro returns a code( between 0 to 7) for a given GPIO base address(x)
//  */
// #define GPIO_BASEADDR_TO_CODE(x)      ( (x == GPIOA)?0:\
// 										(x == GPIOB)?1:\
// 										(x == GPIOC)?2:\
// 										(x == GPIOD)?3:\
// 								        (x == GPIOE)?4:\
// 								        (x == GPIOF)?5:\
// 								        (x == GPIOG)?6:\
// 								        (x == GPIOH)?7: \
// 								        (x == GPIOI)?8:0)



/******************** ADDITIONAL VALUES  **************************************/
#define EXTI_0_POSITION 6
#define EXTI_1_POSITION 7
#define EXTI_2_POSITION 8
#define EXTI_3_POSITION 9
#define EXTI_4_POSITION 10  
#define EXTI_5to9_POSITION 23  // WARN: exti 5 to 9 share the same interrupt line
#define EXTI_10to15_POSITION 40 // WARN: exti 10 to 15 share the same interrupt line

#define EXTI_0_PRIORITY 13
#define EXTI_1_PRIORITY 14
#define EXTI_2_PRIORITY 15
#define EXTI_3_PRIORITY 16
#define EXTI_4_PRIORITY 17  
#define EXTI_5to9_PRIORITY 30  // WARN: exti 5 to 9 share the same interrupt line
#define EXTI_10to15_PRIORITY 47 // WARN: exti 10 to 15 share the same interrupt line

#define EXTI_0_VECTORADDR        0x00000058U
#define EXTI_1_VECTORADDR        0x0000005CU
#define EXTI_2_VECTORADDR        0x00000060U
#define EXTI_3_VECTORADDR        0x00000064U
#define EXTI_4_VECTORADDR        0x00000068U
#define EXTI_5to9_VECTORADDR     0x0000009CU
#define EXTI_10to15_VECTORADDR   0x000000E0U

//************************ 
#define ENABLE 				1
#define DISABLE 			0
#define SET 				ENABLE
#define RESET 				DISABLE
#define GPIO_PIN_SET        SET
#define GPIO_PIN_RESET      RESET
#define FLAG_RESET         RESET
#define FLAG_SET 			SET


//TODO: 20.03.2021: Ogarnij te moduly
// #include "gpio_handler.h"
//#include "stm32f407xx_gpio_driver.h"

#endif /* STM32F103XX_H_ */
