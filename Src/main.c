/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body with SPI
 ******************************************************************************
*/

#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include "my_stm32f103xx.h"
#include "gpio_handler.h"
#include "spi_handler.h"

void  __attribute__((optimize("O0"))) delay(uint32_t value);

int main(void)
{
	SPI_CONF spi_conf;
	spi_conf.MODE=SIMPLEX_TRANSMIT;
	spi_conf.SIDE=MASTER;
	spi_conf.NSS_TYPE=NSS_SOFT;
	spi_conf.SLAVE_CONN=POINT_TO_POINT;
	spi_conf.REMAP=RMAP_DEFAULT;
	configure_spi(spi_conf);
	enable_spi(1);
	

	for(;;){
		char chars[] = "jakis tekst";
		send_data((u_int8_t*)chars,strlen(chars));

	}

	return 0;

}

void  __attribute__((optimize("O0"))) delay(uint32_t value)
{
	for(uint32_t i = 0 ; i < value ; i ++);
}


