/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body with EXTI
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

//::::::::::::::::::EXTERNAL::INTERRUPTS::::::::::::::::::::::::::::::::
// TODO: 20.02.2021: zrobic zmienne na LED_PIN, BTN_PIN itp.

#include<stdio.h>
#include<stdint.h>
#include "my_stm32f103xx.h"
#include "gpio_handler.h"

void blinking(void);
void primitive_interrupt(void);
void  __attribute__((optimize("O0"))) delay(uint32_t value);

int main(void)
{
	gpio_configure(GPIOA,5,OUTPUT_GPIO_PUPD_10MHZ);
	gpio_configure(GPIOC,13,INPUT_PUPD);
	gpio_set_interrupt(GPIOC,13,INT_RISING);

	for(;;){

	}

	return 0;

}

void  __attribute__((optimize("O0"))) delay(uint32_t value)
{
	for(uint32_t i = 0 ; i < value ; i ++);
}


void EXTI9_5_IRQHandler(){
	//do tej funkcji wcale nie wejdziemy
	int test=2;
	test+=2;

}

void EXTI15_10_IRQHandler(){

	int button=gpio_read_input(GPIOC,13);
	int value = gpio_read_input(GPIOA,5);
	if (value==0){
		gpio_set_output(GPIOA,5,GPIO_PIN_SET);
	} else if(value==1) {
		gpio_set_output(GPIOA,5,GPIO_PIN_RESET);
	}
	delay(100000);
	gpio_clear_pr(13);

	

}