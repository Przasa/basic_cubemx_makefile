#include "spi_handler.h"
#include "my_stm32f103xx.h"

//todo 20.02.21:    uzupelnij o remap.
const GPIO_RegDef_t* SPI_GPIO_PORT =GPIOA;
const int  NSS_PIN=15; SCK_PIN=5; MISO_PIN=6; MOSI_PIN=7;

void _conf_gpios();

void configure_spi(SPI_CONF CONF){
    
    //TODO 20.03.2021 jeszcze sprawdzic czy clock do afio jest ok

    _conf_gpios();

    AFIO->MAPR &= ~(1<<0);

//TODO 20.03.2021 te miejsc powinny byc czytalne.
//TODO 20.03.2021 jakies makro na set i reset bitu?
    SPI->CR1 &= ~(1<<0);    //cpol
    SPI->CR1 |=  (1<<1);    //cpha
    SPI->CR1 |= ~(1<<16);   //DFF
    SPI->CR1 |=  (1<<7);    //LSBFFIRT =1
    SPI->CR1 &= ~(2<<2);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)
    SPI->CR1 |=  (1<<9);    //SSM   //TTODO 20.03: dodaj jako parametr w conf.
    SPI->CR2 &= ~(1<<2);    //SSOE (multimaster en)

    switch (CONF.MODE)
    {
        case FULL_DUPLEX:
            SPI->CR1 &= ~(1<<15);   //BIDIMODE
            SPI->CR1 &= ~(1<<10);   //RXONLY
            // SPI->CR1 &= ~(1<<14);   //BIDIOE
            break;
        case SIMPLEX_BIDI:
            SPI->CR1 |=  (1<<15);   //BIDIMODE
            // SPI->CR1 &= ~(1<<10);   //RXONLY
            // SPI->CR1 &= ~(1<<14);   //BIDIOE
            break;
        case SIMPLEX_RECEIVE:
            SPI->CR1 &=  ~(1<<15);   //BIDIMODE
            SPI->CR1 |=  ~(1<<10);   //RXONLY
            // SPI->CR1 &= ~(1<<14);   //BIDIOE
            break;
        case SIMPLEX_TRANSIM:
            SPI->CR1 &=  ~(1<<15);   //BIDIMODE
            SPI->CR1 &= ~(1<<10);   //RXONLY
            // SPI->CR1 &= ~(1<<14);   //BIDIOE
            break;
            
        
        default:
            break;
    }


    if(CONF.SIDE==MASTER){
        SPI->CR1 |=  (1<<2);    //master
        SPI->CR1 &= ~(1<<8);    //SSI (not relevant when SSM active)
    } else
    {
        SPI->CR1 &=  ~(1<<2);    //master
        SPI->CR1 |= ~(1<<8);    //SSI (not relevant when SSM active)
    }
    
    
    SPI->CR1 |=  (1<<6);    //SPI en
    
}

void _conf_gpios(){

}
