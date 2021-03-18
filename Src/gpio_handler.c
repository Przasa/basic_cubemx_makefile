#include "gpio_handler.h"
#include "my_stm32f103xx.h"




void _startclock(GPIO_RegDef_t *PORT);
void gpio_clear_pr(int PIN_NR);
int _portToDec(GPIO_RegDef_t *PORT);

void gpio_configure( GPIO_RegDef_t *PORT, int PIN_NR, GPIO_CONFIGURATION gpio_conf){
    
    _startclock(PORT);
    
    volatile uint32_t *conf_register;
    int rPIN_NR;

    if(PIN_NR<=7){
        conf_register=&(PORT->CRL);
        rPIN_NR=PIN_NR;
    }else{
        conf_register=&(PORT->CRH);
        rPIN_NR=PIN_NR-8;
    }
    *conf_register &= ~(0xf << rPIN_NR*4);
    *conf_register |= (gpio_conf << rPIN_NR *4);
}


void gpio_set_output(GPIO_RegDef_t *port, int PIN_NR, int value){
    if(value==0){
        port->ODR &= ~(1<<PIN_NR);
    } else{
        port->ODR |= (1<<PIN_NR);
    }    
}

int gpio_read_input(GPIO_RegDef_t *port, int PIN_NR){
    int value;
    value = (port->IDR & (1<<PIN_NR)) >> PIN_NR;
    return value;
}

typedef enum{
    RISING,FALLING,RISING_FALLING
} INTERRUPT_TYPES;

void gpio_set_interrupt(GPIO_RegDef_t *port, int PIN_NR, INTERRUPT_TYPES itype){
    // a) AFIO_EXTICRx         -> przypisania interrupta do gpio
    // calculate offset

    //TODO: przeniesinie:
    _set_interrupt_afio(*port,PIN_NR);
    _set_interrupt_exti(PIN_NR);
    _set_interrupt_nvic(interrupt_priority);

    volatile uint32_t *afio_configure;
    int reg_offset = PIN_NR % 4;
    int port_nr = _portToDec(port);
    if(PIN_NR<=3){
        afio_configure = &(AFIO->EXTICR1);
    } else if(PIN_NR<=7){
        afio_configure = &(AFIO->EXTICR2);
    } else if(PIN_NR<=11){
        afio_configure = &(AFIO->EXTICR3);
    } else {
        afio_configure = &(AFIO->EXTICR4);
    }
    
    *afio_configure &= ~(15<< reg_offset);
    *afio_configure |= (port_nr << reg_offset);

    // b) EXTI_IMR             -> ktora linia interrupt
    EXTI->EXTI_IMR |= (1<< PIN_NR);
    // c) EXTI_RTSR EXTI_FTSR  -> typ zbocza interrupt
    EXTI->EXTI_RTSR |= (1 << PIN_NR);
    // EXTI->EXTI_FTSR |= (1 << PIN_NR);
    // d) NVIC IRQ channel     -> podlaczenie interrupt do wektora obslugi przerwan 
    NVIC->ISER0 |= ;
    NVIC->ICER0 |=;
    NVIC->ISPR0 |=;
    NVIC->ICPR0 |=;
    NVIC->IABR0 =;
    NVIC->IPR0 |=;
    NVIC->STIR=;
    
}

void gpio_clear_pr(int PIN_NR){
    EXTI->EXTI_PR &= ~(1<< PIN_NR);
}
//::::::::::::::::::::::::::::  PRIVATES    :::::::::::::::::::::::::::::::::://
void _startclock(GPIO_RegDef_t *PORT){
    if(PORT==GPIOA){
        GPIOA_PCLK_EN();
    }
    if(PORT==GPIOB){
        GPIOB_PCLK_EN();
    }
    if(PORT==GPIOC){
        GPIOC_PCLK_EN();
    }
    if(PORT==GPIOD){
        GPIOD_PCLK_EN();
    }
    if(PORT==GPIOE){
        GPIOE_PCLK_EN();
    }
}

int _portToDec(GPIO_RegDef_t *PORT){
    //TODO 18.03.21: mozna zrobic super makro do tego
    if(PORT==GPIOA){
        return 0;
    } else if(PORT==GPIOB){
        return 1;
    } else if(PORT==GPIOC){
        return 2;
    } else if(PORT==GPIOD){
        return 3;
    } else if(PORT==GPIOE){
        return 4;
    } else if(PORT==GPIOF){
        return 5;
    }
    
}
