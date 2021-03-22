#include "spi_handler.h"
#include "gpio_handler.h"
#include "my_stm32f103xx.h"

//todo 20.02.21:    uzupelnij o remap.
const GPIO_RegDef_t* SPI_GPIO_PORT =GPIOA;
const int  NSS_PIN=4; SCK_PIN=5; MISO_PIN=6; MOSI_PIN=7;

typedef enum{
    RECEIVE,TRANSMIT
}BIDI_DIRECTION;


void _conf_afio(SPI_CONF CONF);
void _conf_gpios(SPI_CONF CONF);
void _switch_bidi(BIDI_DIRECTION direction);
void configure_spi(SPI_CONF CONF){
    

    _conf_afio(CONF);
    _conf_gpios(CONF);


    //TODO 22.03.2021 wydzielic gdzies te wszystki rejestry CR1 i CR2
    //TODO 20.03.2021 te miejsc powinny byc czytalne.
    //TODO 20.03.2021 jakies makro na set i reset bitu?
    SPI->CR1 &= ~(1<<0);    //cpol
    SPI->CR1 |=  (1<<1);    //cpha
    SPI->CR1 |= ~(1<<16);   //DFF
    SPI->CR1 |=  (1<<7);    //LSBFFIRT =1
    SPI->CR1 &= ~(2<<2);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)

    switch (CONF.MODE)
    {
        case FULL_DUPLEX:
            SPI->CR1 &= ~(1<<15);   //BIDIMODE
            SPI->CR1 &= ~(1<<10);   //RXONLY
            break;
        case SIMPLEX_BIDI:
            SPI->CR1 |=  (1<<15);   //BIDIMODE
            SPI->CR1 &= ~(1<<10);   //RXONLY    //moze niekonieczne
            break;
        case SIMPLEX_RECEIVE:
            SPI->CR1 &=  ~(1<<15);   //BIDIMODE
            SPI->CR1 |=  ~(1<<10);   //RXONLY
            break;
        case SIMPLEX_TRANSIM:
            SPI->CR1 &= ~(1<<15);   //BIDIMODE
            SPI->CR1 &= ~(1<<10);   //RXONLY
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
    

    if(CONF.NSS_TYPE==HARDWARE){
        if(CONF.SIDE==MASTER && CONF.SLAVE_CONN==POINT_TO_POINT) {
            SPI->CR1 &=  ~(1<<9);    //SSM   
            SPI->CR1 &=  ~(1<<8);    //SSM  
            SPI->CR2 |=   (1<<2);    //SSOE (NSS)
        } else if(CONF.SIDE==MASTER && CONF.SLAVE_CONN==MULTISLAVE){
            SPI->CR1 &=  ~(1<<9);    //SSM   
            SPI->CR1 &=  ~(1<<8);    //SSM  
            SPI->CR2 &= ~(1<<2);    //SSOE (multimaster en)
        }
        //TODO 22.03.2021 pozostale przypadki...
    }
    
    //TODO: 22.03.2021: tego nie powinno tutaj byc. dopiero przy poczatku/koncu komunikacji.
    SPI->CR1 |=  (1<<6);    //SPI en
    
}

void _select_slave(){
    //TODO 22.03.2021: albo przez NSS, albo przez GPIO!
}

void _switch_bidi(BIDI_DIRECTION DIR){
    if(DIR==RECEIVE){       SPI->CR1 &= ~(1<<14);    }
    else if(DIR==TRANSMIT){ SPI->CR1 |=  (1<<14);    }
    
}

void _conf_afio(SPI_CONF SPICONF){   
    
    if(!AFIO_PCLK_GET()) {AFIO_PCLK_EN();}
    
    if(SPICONF.REMAP==DEFAULT){
        AFIO->MAPR &= ~(1<<0);
    } else if(SPICONF.REMAP==REMAPPED){
        AFIO->MAPR |=  (1<<0);
    }
}

void _conf_gpios(SPI_CONF SPICONF){
    //TODO 22.03.2021 Jeszcze opcja otwarta na  remappingu

    if(!GPIOA_PCLK_GET()) {GPIOC_PCLK_EN();}

    //SCK PIN
    if(SPICONF.SIDE==MASTER){ gpio_configure(GPIOA,SCK_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);}
    else if(SPICONF.SIDE==SLAVE){ gpio_configure(GPIOA,SCK_PIN,INPUT_FLOATING);}

    // note: simplex trans i rec, programistycznie wyglada jak full-duplex. tylko fizycznie usuwasz jeden przewod.
    // MISO/MOSI pins
    if(SPICONF.MODE==FULL_DUPLEX || SPICONF.MODE==SIMPLEX_RECEIVE || SPICONF.MODE==SIMPLEX_TRANSIM){
        if(SPICONF.SIDE==MASTER){
            gpio_configure(GPIOA,MISO_PIN,INPUT_PUPD);                 //MISO, INPUT_FLOATING possible
            gpio_configure(GPIOA,MOSI_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);  //MOSI
        } else if (SPICONF.SIDE==SLAVE){
            gpio_configure(GPIOA,MOSI_PIN,INPUT_PUPD);             //MOSI, INPUT_FLOATING also possible           
            if(SPICONF.SLAVE_CONN==POINT_TO_POINT){gpio_configure(GPIOA,MISO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ); } //MISO
            else if(SPICONF.SLAVE_CONN==MULTISLAVE){gpio_configure(GPIOA,MISO_PIN,OUTPUT_ALT_ODRAIN_10MHZ); }  //MISO
        }
    } else if(SPICONF.MODE==SIMPLEX_BIDI){
        if(SPICONF.SIDE==MASTER){
            gpio_configure(GPIOA,MOSI_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);  //MOSI
        } else if (SPICONF.SIDE==SLAVE){
            if(SPICONF.SLAVE_CONN==POINT_TO_POINT){gpio_configure(GPIOA,MISO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ); } //MISO
            else if(SPICONF.SLAVE_CONN==MULTISLAVE){gpio_configure(GPIOA,MISO_PIN,OUTPUT_ALT_ODRAIN_10MHZ); }  //MISO
        }
    }

    //NSS PIN
    if(SPICONF.NSS_TYPE==HARDWARE){
        if(SPICONF.SIDE==MASTER){ gpio_configure(GPIOA,4,OUTPUT_ALT_PUSHPULL_10MHZ);}
        else if(SPICONF.SIDE==SLAVE){ gpio_configure(GPIOA,4,INPUT_PUPD);}
    }
}
