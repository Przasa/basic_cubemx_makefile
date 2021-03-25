#include "spi_handler.h"
#include "gpio_handler.h"
#include "my_stm32f103xx.h"

//todo 20.02.21:    uzupelnij o remap.
const GPIO_RegDef_t* SPI_GPIO_PORT =GPIOA;
const int  NSS_PIN=4; SCK_PIN=5; MISO_PIN=6; MOSI_PIN=7;

//TODO 23.03.2021: Use it somehow.
const DEF_CPOL = 0; DEF_CPHA=1;DEF_DFF=0;DEF_LSBFIRST=1;DEF_BRATE=0; DEF_SSOE=1;
SPI_COMM_PARAMS COMM_PARAMS_DEF={
    DFF_8,
    CPHA_SECOND,
    CPOL_HIGH,
    BR_DIV32,
    LSBF_MSB
};



typedef enum{
    RECEIVE,TRANSMIT
}BIDI_DIRECTION;

void configure_spi(SPI_CONF CONF);
void configure_spi_adv(SPI_CONF CONF, SPI_COMM_PARAMS ADC_CONF);
void set_soft_slave(int select);
void enable_spi(int ENORDI);
void sendByte(uint8_t byte);
int receiveByte();

void _conf_afio(SPI_CONF CONF);
void _conf_gpios(SPI_CONF CONF);
void _switch_bidi(BIDI_DIRECTION direction);
void _set_comm_parameters(SPI_COMM_PARAMS CONF);




void configure_spi_adv(SPI_CONF CONF, SPI_COMM_PARAMS COMM_PARAMS){
    //ustawianie defaultow. TODO: 23.02.2021
    configure_adv(COMM_PARAMS);
    configure_spi(CONF);
}

void configure_spi(SPI_CONF CONF){
    

    _set_comm_parameters(COMM_PARAMS_DEF);
    _conf_afio(CONF);
    _conf_gpios(CONF);


    //TODO 20.03.2021 jakies makro na set i reset bitu?
    
    if(CONF.SIDE==MASTER){
        SPI->CR1 |=  (1<<SPI_BITPOS_CR1_MSTR);    //as MASTER
    } else if(CONF.SIDE==SLAVE){
        SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_MSTR);    //as SLAVE
    }

    if(CONF.NSS_TYPE==SOFTWARE && CONF.SIDE==SLAVE){
        SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_SSM);    //SSM   
    }

    switch (CONF.MODE)
    {
        case FULL_DUPLEX:
            SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_BIDIMODE);   //BIDIMODE
            SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_RXONLY);   //RXONLY
            break;
        case SIMPLEX_BIDI:
            SPI->CR1 |=  (1<<SPI_BITPOS_CR1_BIDIMODE);   //BIDIMODE
            SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_RXONLY);   //RXONLY    //moze niekonieczne
            break;
        case SIMPLEX_RECEIVE:
            SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_BIDIMODE);   //BIDIMODE
            SPI->CR1 |=  ~(1<<SPI_BITPOS_CR1_RXONLY);   //RXONLY
            break;
        case SIMPLEX_TRANSIM:
            SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_BIDIMODE);   //BIDIMODE
            SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_RXONLY);   //RXONLY
            break;
        default:
            break;
    }

    //tylko gdy master hardwerowo zarzadza 1 slavem, moze wyorzystac swoje NSS'a jako wyjscie do selecta.
    SPI->CR2 = ~(1<<2); // NSS pin as input 
    if(CONF.SIDE==MASTER && CONF.SIDE==HARDWARE && CONF.SLAVE_CONN==POINT_TO_POINT || DEF_SSOE==1){ 
        SPI->CR2 |= (1<<SPI_BITPOS_CR2_SSOE); //SSOE: master's NSS pin as output -> automatically selecting the slave
    }
    
}

void set_soft_slave(int select){

    if(select==0){
        SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_SSI);    
    } else if(select==1){
        SPI->CR1 |=  (1<<SPI_BITPOS_CR1_SSI);    
    }
}

void enable_spi(int ENORDI){
    if(ENORDI==0){
        SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_SPE);    //SPI di
    }else{
        SPI->CR1 |=  (1<<SPI_BITPOS_CR1_SPE);    //SPI en
    }
}

void sendByte(uint8_t byte){
    //BSY FLAG?
    while((SPI->SR & (1 << SPI_BITPOS_SR_TXE))!=0){};
    SPI->DR |= byte;


}
int receiveByte(){
    while((SPI->SR & (1 << SPI_BITPOS_SR_RXNE))!=0){};
    return (SPI->DR & 255);
}


//:::::::::::::::::::::PRIVATE_HELPERS::::::::::::::::::::::::::::::::::
void _set_comm_parameters(SPI_COMM_PARAMS CONF){
    
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_CPOL);    //cpol
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_CPHA);    //cpha
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_DFF);   //DFF
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_LSBF);    //LSBFFIRT =1
    SPI->CR1 &=  ~(7<<SPI_BITPOS_CR1_BR);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)

    SPI->CR1 |=  (CONF.CPOL<<SPI_BITPOS_CR1_CPOL);    //cpol
    SPI->CR1 |=  (CONF.CPHA<<SPI_BITPOS_CR1_CPHA);    //cpha
    SPI->CR1 |=  (CONF.DFF<<SPI_BITPOS_CR1_DFF);   //DFF
    SPI->CR1 |=  (CONF.LSBF<<SPI_BITPOS_CR1_LSBF);    //LSBFFIRT =1
    SPI->CR1 |=  (CONF.BR<<SPI_BITPOS_CR1_BR);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)
    
}


void _select_slave(){
    //TODO 22.03.2021: albo przez NSS, albo przez GPIO!
}

void _switch_bidi(BIDI_DIRECTION DIR){
    //todo 23.03.2021: spprwdz czy bidi smiga.
    if(DIR==RECEIVE){       SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_BIDIOE);    }
    else if(DIR==TRANSMIT){ SPI->CR1 |=  (1<<SPI_BITPOS_CR1_BIDIOE);    }
    
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

    //NSS PIN (w slave nie uzywasz)
    if(SPICONF.NSS_TYPE==HARDWARE){
        if(SPICONF.SIDE==MASTER){ gpio_configure(GPIOA,4,OUTPUT_ALT_PUSHPULL_10MHZ);}
        else if(SPICONF.SIDE==SLAVE){ gpio_configure(GPIOA,4,INPUT_PUPD);}
    }
}
