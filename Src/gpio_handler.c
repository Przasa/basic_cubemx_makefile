#include "gpio_handler.h"
// #include "my_stm32f103xx.h"

enum GPIO_TYPES{INPUT,OUTPUT};

/**
CNFy[1:0]: Port x configuration bits (y= 0 .. 7)
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
*/
void _startclock(GPIO_RegDef_t *PORT);

void configure_gpio( GPIO_RegDef_t *PORT, int PIN_NR, GPIO_CONFIGURATION gpio_conf){
    
    _startclock(&PORT);
    
    // 1000 -  INPUT_PUPD
    // 0001 - OUTPUT_GPIO_PUPD_10MHZ
    uint32_t *conf_register;
    int rPIN_NR;

    if(PIN_NR<=7){
        conf_register=&(PORT->CRL);
        rPIN_NR=PIN_NR;
    }else{
        conf_register=&(PORT->CRH);
        rPIN_NR=PIN_NR-8;
    }
    *conf_register &= ~(0xf << PIN_NR*4);

    switch (gpio_conf){
        case INPUT_PUPD:
            *conf_register |= (0x8 << rPIN_NR*4) ;
            break;
        case OUTPUT_GPIO_PUPD_10MHZ:
            *conf_register |= (0x1 << rPIN_NR*4);
        default:
            break;
    }

}

void _startclock(GPIO_RegDef_t *PORT){
    if(PORT==GPIOA){
        GPIOE_PCLK_EN();
    }
    if(PORT==GPIOB){
        GPIOE_PCLK_EN();
    }
    if(PORT==GPIOC){
        GPIOE_PCLK_EN();
    }
    if(PORT==GPIOD){
        GPIOE_PCLK_EN();
    }
    if(PORT==GPIOE){
        GPIOE_PCLK_EN();
    }
}

void set_gpio_output(GPIO_RegDef_t *port, int PIN_NR, int value){
    if(value==0){
        port->ODR &= ~(1<PIN_NR);
    } else{
        port->ODR |= (1<PIN_NR);
    }    
}

int read_gpio_input(GPIO_RegDef_t *port, int PIN_NR){
    int value;
    value = (port->IDR & (1<<PIN_NR)) >> PIN_NR;
    return value;
}