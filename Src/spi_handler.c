#include "spi_handler.h"
#include "gpio_handler.h"
#include "my_stm32f103xx.h"

//todo 20.03.21:    jakies makro na set i reset bitu?
//todo 20.03.21:    uzupelnij o remap.
//todo 25.03.21:    uzupelnij o SPI2,SPI3... (jezeli sa) (i nawet ich remap). Pewnie trzeba powiekszyc configi
//todo 25.03.21:    uniewersalnosc pinow GPIO (np MISO_PIN ustawiany w zlaeznosci od rodzaju SPI)
//todo 25.03.21:    zrib driver dla NVICa
//todo 25.03.21:    sprawdz zegar --> done
//todo 25.03.21:    zaznaczanie slav'ow przez GPIO
//todo 25.03.21:    przesylanie przez przerwania.
const GPIO_RegDef_t* SPI_GPIO_PORT =GPIOA;
const int  NSS_PIN=4; SCK_PIN=5; MISO_PIN=6; MOSI_PIN=7;

//TODO 23.03.2021: Use it somehow.
// const DEF_CPOL = 0; DEF_CPHA=1;DEF_DFF=0;DEF_LSBFIRST=1;DEF_BRATE=0; DEF_SSOE=1;
// const DEF_SSOE=0;
SPI_COMM_PARAMS COMM_PARAMS;
const SPI_COMM_PARAMS COMM_PARAMS_DEF={
    DFF_8,
    CPHA_SECOND,
    CPOL_HIGH,
    BR_DIV32,
    LSBF_MSB
    // SSOE_DI
};



typedef enum{
    RECEIVE,TRANSMIT
}BIDI_DIRECTION;

void configure_spi(SPI_CONF CONF);
void configure_spi_adv(SPI_CONF CONF, SPI_COMM_PARAMS ADC_CONF);
void enable_spi(int ENORDI);
void send_data(uint8_t* transmit_buffer, int length);
void receive_data(uint8_t* receive_buffer, int length);

void _conf_afio(SPI_CONF CONF);
void _conf_gpios(SPI_CONF CONF);
void _switch_bidi(BIDI_DIRECTION direction);    //moze jako public?
void _set_comm_parameters(SPI_COMM_PARAMS CONF);

// void set_soft_slave(int select);



void configure_spi_adv(SPI_CONF conf, SPI_COMM_PARAMS comm_params){
    //ustawianie defaultow. TODO: 23.02.2021
    _set_comm_parameters(comm_params);
    configure_spi(conf);
}

void configure_spi(SPI_CONF CONF){
    
    if(!SPI1_PCLK_GET()){SPI1_PCLK_EN();}
    
    _conf_afio(CONF);
    _conf_gpios(CONF);
    _set_comm_parameters(COMM_PARAMS_DEF);

    //todo 25.03.2021: moze to oc ponizej tez wydzielic to jakiejs funkcji?


    //MASTER/SLAVE
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_MSTR);    
    SPI->CR1 |=  (CONF.SIDE<<SPI_BITPOS_CR1_MSTR);    

    //COMM_MODES
    switch (CONF.MODE)
    {
        case FULL_DUPLEX:
            SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_BIDIMODE);   
            SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_RXONLY);   
            break;
        case SIMPLEX_BIDI:
            SPI->CR1 |=  (1<<SPI_BITPOS_CR1_BIDIMODE); 
            SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_RXONLY);   //RXONLY    //moze niekonieczne
            break;
        case SIMPLEX_RECEIVE:
            SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_BIDIMODE);   
            SPI->CR1 |=  ~(1<<SPI_BITPOS_CR1_RXONLY);   
            break;
        case SIMPLEX_TRANSIM:
            SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_BIDIMODE);  
            SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_RXONLY);   
            break;
        default:
            break;
    }

    //tylko gdy master hardwerowo zarzadza 1 slavem, moze wyorzystac swoje NSS'a jako wyjscie do selecta.
    if(CONF.SIDE==MASTER && CONF.NSS_TYPE==HARDWARE_NSS && CONF.SLAVE_CONN==POINT_TO_POINT){ 
        SPI->CR2 |=  (1<<SPI_BITPOS_CR2_SSOE); //SSOE: master's NSS pin as output -> automatically selecting the slave
    } else{
        SPI->CR2 &= ~(1<<SPI_BITPOS_CR2_SSOE); //SSOE as input.
    }

    //SSI
    if(CONF.SIDE==SLAVE && CONF.NSS_TYPE==SOFTWARE){
        SPI->CR1 |=  (1<<SPI_BITPOS_CR1_SSI);    
    }else
    {
        SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_SSI);    
    }
    

}

//nie potrzebne
// void set_soft_slave(int select){

//     if(select==0){
//         SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_SSI);    
//     } else if(select==1){
//         SPI->CR1 |=  (1<<SPI_BITPOS_CR1_SSI);    
//     }
// }

void enable_spi(int ENORDI){
    while(_get_flag_status(SPI_BITPOS_SR_BSY));
    
    if(ENORDI==0){
        SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_SPE);    //SPI di
    }else{
        SPI->CR1 |=  (1<<SPI_BITPOS_CR1_SPE);    //SPI en
    }
}

//moze moglibysmy uzyc referencji zamiast wskaznika ?
void send_data(uint8_t* transmit_buffer, int length){

    while(_get_flag_status(SPI_BITPOS_SR_TXE)!=1);
    while(length>0){
        if(COMM_PARAMS.DFF==DFF_8){
            SPI->DR = *transmit_buffer;
            length--;
            transmit_buffer++;
        } else if(COMM_PARAMS.DFF==DFF_16){
            SPI->DR = *((uint16_t*)transmit_buffer);
            length--;length--;
            (uint16_t*)transmit_buffer++;
        }
    }
};

void receive_data(uint8_t* receive_buffer, int length){

    while(_get_flag_status(SPI_BITPOS_SR_RXNE)!=1);

    while(length>0){
        if(COMM_PARAMS.DFF==DFF_8){
            *receive_buffer=SPI->DR;
            length--;
            receive_buffer++;
        } else if(COMM_PARAMS.DFF==DFF_16){
            *((uint16_t*)receive_buffer) = SPI->DR;
            length--;length--;
            (uint16_t*)receive_buffer++;
        }
    }

}

// void sendByte(uint8_t byte){
//     //BSY FLAG?
//     while((SPI->SR & (1 << SPI_BITPOS_SR_TXE))!=0){};
//     SPI->DR |= byte;


// }
// int receiveByte(){
//     while((SPI->SR & (1 << SPI_BITPOS_SR_RXNE))!=0){};
//     return (SPI->DR & 255);
// }


//:::::::::::::::::::::PRIVATE_HELPERS::::::::::::::::::::::::::::::::::
void  _set_comm_parameters(SPI_COMM_PARAMS comm_params){
    COMM_PARAMS=comm_params;

    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_CPOL);    //cpol
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_CPHA);    //cpha
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_DFF);   //DFF
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_LSBF);    //LSBFFIRT =1
    SPI->CR1 &=  ~(7<<SPI_BITPOS_CR1_BR);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)

    SPI->CR1 |=  (COMM_PARAMS.CPOL<<SPI_BITPOS_CR1_CPOL);    //cpol
    SPI->CR1 |=  (COMM_PARAMS.CPHA<<SPI_BITPOS_CR1_CPHA);    //cpha
    SPI->CR1 |=  (COMM_PARAMS.DFF<<SPI_BITPOS_CR1_DFF);   //DFF
    SPI->CR1 |=  (COMM_PARAMS.LSBF<<SPI_BITPOS_CR1_LSBF);    //LSBFFIRT =1
    SPI->CR1 |=  (COMM_PARAMS.BR<<SPI_BITPOS_CR1_BR);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)
    
}

//potrzebne wogole?
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
    //TODO 25.03.2021 malo to czytelne, nie zagniezdzaj ifow. NSS jest OK

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
    if(SPICONF.NSS_TYPE==HARDWARE_NSS && SPICONF.SIDE==MASTER ){
        gpio_configure(GPIOA,NSS_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
    } else if(SPICONF.NSS_TYPE==HARDWARE_NSS && SPICONF.SIDE==SLAVE){
        gpio_configure(GPIOA,NSS_PIN,INPUT_PUPD);
    } else if(SPICONF.NSS_TYPE==HARDWARE_GPIO){
        gpio_configure(GPIOA,NSS_PIN,INPUT_PUPD);
    }
}

int _get_flag_status(int bit_position){
    return ((SPI->SR & (1<<bit_position)) >> bit_position);
}