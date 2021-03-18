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
#include "gpio_handler.h"

void blinking(void);
void primitive_interrupt(void);
void  __attribute__((optimize("O0"))) delay(uint32_t value);

// void my_delay(void);

//stabilne miganie dioda. nadaje sie jako baza

//:::::::::DEBUG_SETTINGS:::::::::::::://
const int MIGACZ =0;


int main(void)
{
	gpio_configure(GPIOA,5,OUTPUT_GPIO_PUPD_10MHZ);
	gpio_configure(GPIOC,13,INPUT_PUPD);

	for(;;){

		if(MIGACZ){			blinking();
		} else	{			primitive_interrupt();		}
	}

	return 0;

}

void  __attribute__((optimize("O0"))) delay(uint32_t value)
{
	for(uint32_t i = 0 ; i < value ; i ++);
}

void blinking(void){
	int value;
	value = gpio_read_input(GPIOC,13);
	if(value==1){
		gpio_set_output(GPIOA,5,GPIO_PIN_SET);
		delay(100000);
		gpio_set_output(GPIOA,5,GPIO_PIN_RESET);
		delay(100000);
	} 
}

void primitive_interrupt(void){
	int value;
	value = gpio_read_input(GPIOC,13);
	delay(10000);
	if (value==0){
		gpio_set_output(GPIOA,5,GPIO_PIN_SET);
	} else if(value==1) {
		gpio_set_output(GPIOA,5,GPIO_PIN_RESET);
	}
}