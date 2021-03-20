#include "gpio_handler.h"
#include "my_stm32f103xx.h"




void gpio_clear_pr(int PIN_NR);
void _startclock(GPIO_RegDef_t *PORT);
int _portToDec(GPIO_RegDef_t *PORT);

void _set_interrupt_afio(GPIO_RegDef_t *port,int PIN_NR);
void _set_interrupt_exti(int PIN_NR,INTERRUPT_TYPES int_type);
void _set_interrupt_nvic(int interrupt_position, int interrupt_priority);

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

void gpio_set_interrupt(GPIO_RegDef_t *port, int PIN_NR, INTERRUPT_TYPES itype){

    //TODO: pamietaj o zwlaczzeniu clockow do peryferiow.
    
    //WARN: czy nie trzeba alt funct dla GPIO? --> chyba nie
    int interrupt_position;
    int interrupt_priority;

    if(PIN_NR==0){interrupt_position=EXTI_0_POSITION;interrupt_priority=EXTI_0_PRIORITY;}
    else if(PIN_NR==1) {{interrupt_position=EXTI_1_POSITION;interrupt_priority=EXTI_1_PRIORITY;}}
    else if(PIN_NR==2) {{interrupt_position=EXTI_2_POSITION;interrupt_priority=EXTI_2_PRIORITY;}}
    else if(PIN_NR==3) {{interrupt_position=EXTI_3_POSITION;interrupt_priority=EXTI_3_PRIORITY;}}
    else if(PIN_NR==4) {{interrupt_position=EXTI_4_POSITION;interrupt_priority=EXTI_4_PRIORITY;}}
    else if(PIN_NR<=9) {{interrupt_position=EXTI_5to9_POSITION;interrupt_priority=EXTI_5to9_PRIORITY;}}
    else if(PIN_NR<=15) {{interrupt_position=EXTI_10to15_POSITION;interrupt_priority=EXTI_10to15_PRIORITY;}}

    _set_interrupt_afio(port,PIN_NR);
    _set_interrupt_exti(PIN_NR,itype);
    _set_interrupt_nvic(interrupt_position, interrupt_priority);
    
    //TODO 19.03.2021: obluga tablicy  przerwan, clearowanie tablicy
}


void gpio_clear_pr(int PIN_NR){
    EXTI->EXTI_PR &= ~(1<< PIN_NR);
}
//::::::::::::::::::::::::::::  PRIVATES    :::::::::::::::::::::::::::::::::://
void _startclock(GPIO_RegDef_t *PORT){
    //TODO: 20.03.2021: oddac to dla my_stm32f103xx.h
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

void _set_interrupt_afio(GPIO_RegDef_t *port,int PIN_NR){
    // a) AFIO_EXTICRx         -> przypisania interrupta do gpio

    if(!AFIO_PCLK_GET()){AFIO_PCLK_EN();}

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
}
void _set_interrupt_exti(int PIN_NR,INTERRUPT_TYPES int_type){
    // b) EXTI_IMR             -> ktora linia interrupt
    EXTI->EXTI_IMR |= (1<< PIN_NR);
    // c) EXTI_RTSR EXTI_FTSR  -> typ zbocza interrupt
    switch (int_type)
    {
        case INT_RISING:
            EXTI->EXTI_RTSR |= (1<< PIN_NR);
            EXTI->EXTI_FTSR &= ~(1 << PIN_NR);
            break;
        case INT_FALLING:
            EXTI->EXTI_FTSR |= (1 << PIN_NR);
            EXTI->EXTI_RTSR &= ~(1 << PIN_NR);
            break;
        case INT_RISING_FALLING:
            EXTI->EXTI_RTSR |= (1<< PIN_NR);
            EXTI->EXTI_FTSR |= (1 << PIN_NR);
        default:
            EXTI->EXTI_RTSR &= ~(1 << PIN_NR);
            EXTI->EXTI_FTSR &= ~(1 << PIN_NR);
            break;
    }
}

void _set_interrupt_nvic(int interrupt_line, int interrupt_priority){
    // d) NVIC IRQ channel     -> podlaczenie interrupt do wektora obslugi przerwan 
    
    uint32_t* set_en_reg;
    // uint32_t* clear_en_reg;
    // uint32_t* set_pend_reg;
    // uint32_t* clear_pend_reg;
    // uint32_t* active_bit_reg;
    uint32_t* priority_reg;

    int set_reg_offset = interrupt_line % 32;
    int int_reg_offset = interrupt_line/4;
    int int_byte_offset = interrupt_line % 4;
    

    //TODO: 20.03.2021; jakis madrzejszy mechanizm wybiarania rejestru
    if(interrupt_line<=31){
        set_en_reg      =&(NVIC->ISER0);
        // clear_en_reg    =&(NVIC->ICER0);
        // set_pend_reg    =&(NVIC->ISPR0);
        // clear_pend_reg  =&(NVIC->ICPR0);
        // active_bit_reg  =&(NVIC->IABR0);
    }
    else if(interrupt_line<=63){
        set_en_reg      =&(NVIC->ISER1);
        // clear_en_reg    =&(NVIC->ICER1);
        // set_pend_reg    =&(NVIC->ISPR1);
        // clear_pend_reg  =&(NVIC->ICPR1);
        // active_bit_reg  =&(NVIC->IABR1);
    }
    else if(interrupt_line<=67){
        set_en_reg      =&(NVIC->ISER2);
        // clear_en_reg    =&(NVIC->ICER2);
        // set_pend_reg    =&(NVIC->ISPR2);
        // clear_pend_reg  =&(NVIC->ICPR2);
        // active_bit_reg  =&(NVIC->IABR2);
    }
    *set_en_reg |= (1<<set_reg_offset);

    if (interrupt_priority > 0){
        priority_reg = &(NVIC->IPR0) + (4*int_reg_offset);

        *priority_reg &= ~(15<<int_byte_offset);
        *priority_reg |= (interrupt_priority<<int_byte_offset);
    }

}

void _disable_interrupt_nvic(int interrupt_line, int interrupt_priority){
    //TODO: 19.03.2021; jakis madrzejszy mechanizm wybiarania rejestru
    //TODO: 20.03.2021; szkoda robic takie powtorzenia

    // uint32_t* set_en_reg;
    uint32_t* clear_en_reg;
    // uint32_t* set_pend_reg;
    // uint32_t* clear_pend_reg;
    // uint32_t* active_bit_reg;
    // uint32_t* priority_reg;

    int set_reg_offset = interrupt_line % 32;
    // int int_reg_offset = interrupt_line/4;
    // int int_byte_offset = interrupt_line % 4;

    if(interrupt_line<=31){
        // set_en_reg      =&(NVIC->ISER0);
        clear_en_reg    =&(NVIC->ICER0);
        // set_pend_reg    =&(NVIC->ISPR0);
        // clear_pend_reg  =&(NVIC->ICPR0);
        // active_bit_reg  =&(NVIC->IABR0);
    }
    else if(interrupt_line<=63){
        // set_en_reg      =&(NVIC->ISER1);
        clear_en_reg    =&(NVIC->ICER1);
        // set_pend_reg    =&(NVIC->ISPR1);
        // clear_pend_reg  =&(NVIC->ICPR1);
        // active_bit_reg  =&(NVIC->IABR1);
    }
    else if(interrupt_line<=67){
        // set_en_reg      =&(NVIC->ISER2);
        clear_en_reg    =&(NVIC->ICER2);
        // set_pend_reg    =&(NVIC->ISPR2);
        // clear_pend_reg  =&(NVIC->ICPR2);
        // active_bit_reg  =&(NVIC->IABR2);
    }
    *clear_en_reg |= (1<<set_reg_offset);

}



