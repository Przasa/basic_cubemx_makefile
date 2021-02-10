/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */


//#if !defined(__SOFT_FP__) && defined(__ARM_FP)
//  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
//#endif


#include<stdio.h>
#include<stdint.h>
#include "my_stm32f103xx.h"

// void my_delay(void);

//stabilne miganie dioda. nadaje sie jako baza

void  __attribute__((optimize("O0"))) delay(uint32_t value)
{
	for(uint32_t i = 0 ; i < value ; i ++);
}


int main(void)
{

	GPIOA_PCLK_EN();

	GPIOA->CRL |= (1<< 20);
	GPIOA->CRL &= ~(1<< 21);
	GPIOA->CRL &= ~(1<< 22);
	GPIOA->CRL &= ~(1<< 23);


	for(;;){
		GPIOA->ODR |= (1<<5);
		delay(500000);
		GPIOA->ODR &=~(1<<5);
		delay(50000);
	}

	return 0;

}
