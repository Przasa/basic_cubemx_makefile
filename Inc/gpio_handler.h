#ifndef GPIOHANDLER_H
#define GPIOHANDLER_H

#include "my_stm32f103xx.h"

/**
 * CNFy[1:0]: Port x configuration bits (y= 0 .. 7)
These bits are written by software to configure the corresponding I/O port.
Refer to Table 20: Port bit configuration table on page 156.
In input mode (MODE[1:0]=00):
    00: Analog mode
    01: Floating input (reset state)
    10: Input with pull-up / pull-down
    11: Reserved
In output mode (MODE[1:0] > 00):
    00: General purpose output push-pull
    01: General purpose output Open-drain
    10: Alternate function output Push-pull
    11: Alternate function output Open-drain

MODEy[1:0]: Port x mode bits (y= 0 .. 7)
These bits are written by software to configure the corresponding I/O port.
Refer to Table 20: Port bit configuration table on page 156.
    00: Input mode (reset state)
    01: Output mode, max speed 10 MHz.
    10: Output mode, max speed 2 MHz.
    11: Output mode, max speed 50 MHz.
 * 
 */ 

typedef enum {
    INPUT_ANALOG=0b0000,INPUT_FLOATING=0b0100,INPUT_PUPD=0b1000,
    OUTPUT_GPIO_PUPD_10MHZ=0b0001,OUTPUT_GPIO_PUPD_2MHZ=0b0010,OUTPUT_GPIO_PUPD_50MHZ=0b0011,
    OUTPUT_GPIO_ODRAIN_10MHZ=0b0101,OUTPUT_GPIO_ODRAIN_2MHZ=0b0110,OUTPUT_GPIO_ODRAIN_50MHZ=0b0111,
    OUTPUT_ALT_PUPD_10MHZ=0b1001,OUTPUT_ALT_PUPD_2MHZ=0b1010,OUTPUT_ALT_PUPD_50MHZ=0b1011,
    OUTPUT_ALT_ODRAIN_10MHZ=0b1101,OUTPUT_ALT_ODRAIN_2MHZ=0b1110,OUTPUT_ALT_ODRAIN_50MHZ=0b1111
}GPIO_CONFIGURATION;

typedef enum{
    INT_RISING,INT_FALLING,INT_RISING_FALLING
} INTERRUPT_TYPES;


void gpio_configure( GPIO_RegDef_t *PORT, int PIN_NR, GPIO_CONFIGURATION gpio_conf);
void gpio_set_output(GPIO_RegDef_t *port, int PIN_NR, int value);
int  gpio_read_input(GPIO_RegDef_t *port, int PIN_NR);
void gpio_set_interrupt(GPIO_RegDef_t *port, int PIN_NR, INTERRUPT_TYPES itype);

#endif  // GPIOHANDLER_H



