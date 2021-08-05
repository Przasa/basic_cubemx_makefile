#include "spi_handler.h"
#include "gpio_handler.h"
#include "my_stm32f103xx.h"

//overall TODO:
// 1.konfiguracja poczatkowa
//     -wybor SPI (1,2,3)       *opcjonal
//     -forma SPI: fduplex,half duplex (bidi), simplex t, simplex r
//     -remap/normap --> piny GPIO      *opcjonal
//     -tryb slave select (hardware, software)
//     -BR<CPOL.CPHA,DFF,LSBFIRTS,
// 2. wyslanie ramki
// 3. odebranie ramki
// 4. nagranei przerwan.

//     SPI_CR1:  master: BR[2:0]=xy, SPI_CPOL=xx,CPHA=xx, DFF = 16/8 bit, LSBFIRST, SSM=1, SSI=1, SSOE=1 (if req in outmode), MSTR=1, SPE=1
//     SPI_CR1:  slave: DFF, CPOL, CPHA, LSBFIRST, [hw]NSS(hwmode), SSM(softmode)=1,SS1=0, MSTR=0, SPE=1
//     CPI_CR2:  master: SSOE: 0= SS output disabled (multimaster allowed), 1=ss output enabled (mm not allwed)

// SSM,SSOE,SSI

// BIDIOE ,BIDIMODE, RXONLY,
// full-duplex:
//     master:     BIDIMODE=0,RXONLY=0,(BIDIOE=0)
//     slave:      BIDIMODE=0,RXONLY=0,(BIDIOE=0)
// half-duplex:
//     master:     BIDIMODE=1,BIDIOE=1/0,(RXONLY=0)
//     slave:     BIDIMODE=1, BIDIOE=1/0,(RXONLY=0)
// siplex-receive:
//     master:     BIDIMODE=0, RXONLY=1, (BIDIOE=0)
//     slave:      BIDIMODE=0, RXONLY=1, (BIDIOE=0)
// siplex-transmit:
//     master:     BIDIMODE=0, RXONLY=0, (BIDIOE=0)
//     slave:      BIDIMODE=0, RXONLY=0, (BIDIOE=0)


//parkowanie 05.08.21:
// a) dokoncz definicje malych funkcji ponizej
// b) zrob podobne rzecz do transmisji i odbioru
// c) zaimplementuj je configure SPI (stare rzeczy wywal)
// d) przetestuj



void _configureClock();
void _configureSpiGPIOs(SPI_NR spi_nr, SPI_ACON_REMAP remap);
void _configureSlaveGPIO(SPI_PIN spi_pin);
void _configureFrame(SPI_ACON_CPHA cpha,SPI_ACON_CPOL cpol,SPI_ACON_DFF dff,SPI_ACON_LSBF lsbf); // podzielic?
void _configureBR(SPI_ACON_BR br);
void _configureBR(SPI_ACON_BR br){
        SPI->CR1 |=  (1<< SPI_BITPOS_CR1_BR);
}

void _configureSide(SPI_SIDE side);
void _configureSide(SPI_SIDE side){
    if(side==MASTER){
        SPI->CR1 |=  (1<< SPI_BITPOS_CR1_MSTR);
    } else {
        SPI->CR1 &=  ~(1<< SPI_BITPOS_CR1_MSTR);
    }
}

void _configureSlaveSelect(SPI_SIDE side,SPI_ACON_NSSTYPE nss_type);    //multimaster sobie darujemy
void _configureSlaveSelect(SPI_SIDE side,SPI_ACON_NSSTYPE nss_type){
    if((side==SLAVE) & (nss_type=NSS_SOFT)){
        SPI->CR1 |=  (1<< SPI_BITPOS_CR1_SSM);
    } else{
        SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_SSM);
    }
}

void _configureBidi(SPI_MODE mode);
void _configureBidi(SPI_MODE mode){
    if(mode==FULL_DUPLEX){
        SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_BIDIMODE);
        SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_BIDIOE);
        SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_RXONLY);
    } else if(mode==HALF_DUPLEX){
        SPI->CR1 |=  (1<< SPI_BITPOS_CR1_BIDIMODE);
        SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_BIDIOE);
        SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_RXONLY);
    } else if (mode==SIMPLEX_RECEIVE){
        SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_BIDIMODE);
        SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_BIDIOE);
        SPI->CR1 |=  (1<< SPI_BITPOS_CR1_RXONLY);
    } else if (mode==SIMPLEX_TRANSMIT){
        SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_BIDIMODE);
        SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_BIDIOE);
        SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_RXONLY);
    }
    
}

// 5.08.21: przeniesc do _.h
void selectSlaveHW(int ENORDI,SPI_ACON_NSSTYPE nss_type, SPI_PIN spi_pin);
void selectSlaveHW(int ENORDI,SPI_ACON_NSSTYPE nss_type, SPI_PIN spi_pin){
    if(nss_type==NSS_DEFPIN){
        SPI->CR2 &= ~(1<<SPI_BITPOS_CR2_SSOE);
        if(ENORDI){
            SPI->CR2 |= (1<<SPI_BITPOS_CR2_SSOE);
        }
    } else if(nss_type==NSS_GPIO){
        gpio_set_output(spi_pin.GPIO_PORT,spi_pin.GPIO_PIN,!ENORDI);
    } else if(nss_type==NSS_SOFT){      //martwy case.
        selectSlaveSW(ENORDI);
    }
}
    
void selectSlaveSW(int ENORDI);
void selectSlaveSW(int ENORDI){
    if(!(SPI->CR1 & (1<<SPI_BITPOS_CR1_SSM) )){
        SPI->CR1 |= (1<<SPI_BITPOS_CR1_SSM);
    }

    if(ENORDI){
        SPI->CR1 |= (1<<SPI_BITPOS_CR1_SSI);
    } else{
        SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_SSI);
    }
}




//todo 20.03.21:    jakies makro na set i reset bitu?
//todo 20.03.21:    uzupelnij o remap.
//todo 25.03.21:    uzupelnij o SPI2,SPI3... (jezeli sa) (i nawet ich remap). Pewnie trzeba powiekszyc configi
//todo 25.03.21:    uniewersalnosc pinow GPIO (np MISO_PIN ustawiany w zlaeznosci od rodzaju SPI)
//todo 25.03.21:    zrib driver dla NVICa
//todo 25.03.21:    sprawdz zegar --> done
//todo 25.03.21:    zaznaczanie slav'ow przez GPIO
//todo 25.03.21:    przesylanie przez przerwania.

//TODO 23.03.2021: Use it somehow.
// const DEF_CPOL = 0; DEF_CPHA=1;DEF_DFF=0;DEF_LSBFIRST=1;DEF_BRATE=0; DEF_SSOE=1;
// const DEF_SSOE=0;





const SPI_ADVCONF ADVCONF_DEFAULT={
    SPI1,
    RMAP_DEFAULT,
    NSS_DEFPIN,
    DFF_8,
    CPHA_SECOND,
    CPOL_HIGH,
    BR_DIV32,
    LSBF_MSB
    // SSOE_DI
};
SPI_ADVCONF ADVCONF;


typedef enum{
    RECEIVE,TRANSMIT
}BIDI_DIRECTION;

void configure_spi(SPI_CONF CONF);
void configure_spi_adv(SPI_CONF CONF, SPI_ADVCONF ADC_CONF);
void enable_spi(int ENORDI);
void send_data(uint8_t* transmit_buffer, int length);
void receive_data(uint8_t* receive_buffer, int length);

void _conf_afio(SPI_CONF CONF);
void _set_gpios(SPI_CONF CONF);
void _set_gpio_assign(SPICONF);
void _switch_bidi(BIDI_DIRECTION direction);    //moze jako public?
void _set_advconf(SPI_ADVCONF CONF);

::::::::::::::

void configure_spi(SPI_CONF conf){

    if(!isConfiguredAdvanced(ADVCONF)) _set_advconf(ADVCONF_DEFAULT);
    
    configureClock();
    configure

    if(!SPI1_PCLK_GET()){SPI1_PCLK_EN();}

    // 21.07.21: uwazaj: mozesz wysypc sie przy adv_config i remapowaniu. wymysl cos.
    _set_gpios(conf);

    //todo 25.03.2021: moze to oc ponizej tez wydzielic to jakiejs funkcji?



    //MASTER/SLAVE
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_MSTR);    
    SPI->CR1 |=  (conf.SIDE<<SPI_BITPOS_CR1_MSTR);    

    //COMM_MODES
    switch (conf.MODE)
    {
        SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_BIDIMODE);   
        SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_RXONLY);   
        SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_BIDIOE);   

        case FULL_DUPLEX:
            break;
        case HALF_DUPLEX:
            //w transimicie/receive trzeba ustawiac: SPI->CR1 (&=)/(|=) (~)(1<<SPI_BITPOS_CR1_BIDIOE); 
            SPI->CR1 |=  (1<<SPI_BITPOS_CR1_BIDIMODE); 
            break;
        case SIMPLEX_RECEIVE:
            SPI->CR1 |=  (1<<SPI_BITPOS_CR1_RXONLY);   
            break;
        case SIMPLEX_TRANSMIT:
            break;
        default:
            break;
    }


static int isConfiguredAdvanced(SPI_ADVCONF spi_conf){
    
    if(
        spi_conf.SPI_NR == ADVCONF_DEFAULT.SPI_NR ||
        spi_conf.REMAP == ADVCONF_DEFAULT.REMAP ||
        spi_conf.NSS_TYPE == ADVCONF_DEFAULT.NSS_TYPE ||
        spi_conf.DFF == ADVCONF_DEFAULT.DFF ||
        spi_conf.CPHA == ADVCONF_DEFAULT.CPHA ||
        spi_conf.CPOL == ADVCONF_DEFAULT.CPOL ||
        spi_conf.BR == ADVCONF_DEFAULT.BR ||
        spi_conf.LSBF == ADVCONF_DEFAULT.LSBF    ) {        return 1   ; }
    else {return 0;}
}

//TODO 21.07.21: reconfigure_gpio();

void configure_spi_adv(SPI_CONF conf, SPI_ADVCONF adv_conf){
    //ustawianie defaultow. TODO: 23.02.2021 --> ok
    ADVCONF = adv_conf;
    configure_spi(conf);
}






    // //tylko gdy master hardwerowo zarzadza 1 slavem, moze wyorzystac swoje NSS'a jako wyjscie do selecta.
    // if(conf.SIDE==MASTER && conf.NSS_TYPE==NSS_DEFPIN && conf.SLAVE_CONN==POINT_TO_POINT){ 
    //     SPI->CR2 |=  (1<<SPI_BITPOS_CR2_SSOE); //SSOE: master's NSS pin as output -> automatically selecting the slave
    // } else{
    //     SPI->CR2 &= ~(1<<SPI_BITPOS_CR2_SSOE); //SSOE as input.
    // }

    // //SSI
    // if(conf.SIDE==SLAVE && conf.NSS_TYPE==NSS_SOFT){
    //     SPI->CR1 |=  (1<<SPI_BITPOS_CR1_SSI);    
    // }else
    // {
    //     SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_SSI);    
    // }
    

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
        if(ADVCONF.DFF==DFF_8){
            SPI->DR = *transmit_buffer;
            length--;
            transmit_buffer++;
        } else if(ADVCONF.DFF==DFF_16){
            SPI->DR = *((uint16_t*)transmit_buffer);
            length--;length--;
            (uint16_t*)transmit_buffer++;
        }
    }
};

void receive_data(uint8_t* receive_buffer, int length){

    while(_get_flag_status(SPI_BITPOS_SR_RXNE)!=1);

    while(length>0){
        if(ADVCONF.DFF==DFF_8){
            *receive_buffer=SPI->DR;
            length--;
            receive_buffer++;
        } else if(ADVCONF.DFF==DFF_16){
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
void  _set_advconf(SPI_ADVCONF adv_conf){
    ADVCONF=adv_conf;

    _set_gpios(adv_conf.SPI_NR);
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_CPOL);    //cpol
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_CPHA);    //cpha
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_DFF);   //DFF
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_LSBF);    //LSBFFIRT =1
    SPI->CR1 &=  ~(7<<SPI_BITPOS_CR1_BR);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_SSM);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)

    SPI->CR1 |=  (ADVCONF.CPOL<<SPI_BITPOS_CR1_CPOL);    //cpol
    SPI->CR1 |=  (ADVCONF.CPHA<<SPI_BITPOS_CR1_CPHA);    //cpha
    SPI->CR1 |=  (ADVCONF.DFF<<SPI_BITPOS_CR1_DFF);   //DFF
    SPI->CR1 |=  (ADVCONF.LSBF<<SPI_BITPOS_CR1_LSBF);    //LSBFFIRT =1
    SPI->CR1 |=  (ADVCONF.BR<<SPI_BITPOS_CR1_BR);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)
    
    SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_SSM);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)
    
    
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
    //TODO    
    // if(!AFIO_PCLK_GET()) {AFIO_PCLK_EN();}
    
    // if(SPICONF.REMAP==RMAP_DEFAULT){
    //     AFIO->MAPR &= ~(1<<0);
    // } else if(SPICONF.REMAP==RMAP_REMAPPED){
    //     AFIO->MAPR |=  (1<<0);
    // }
}

void _set_gpios(ADVCONF adv_conf){

    _set_gpio_assign(SPI_NR spi_nr);
    

    //TODO 25.03.2021 malo to czytelne, nie zagniezdzaj ifow. NSS jest OK

    if(!GPIOA_PCLK_GET()) {GPIOC_PCLK_EN();}

    //SCK PIN
    if(SPICONF.SIDE==MASTER){ gpio_configure(GPIOA,SCK_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);}
    else if(SPICONF.SIDE==SLAVE){ gpio_configure(GPIOA,SCK_PIN,INPUT_FLOATING);}

    // note: simplex trans i rec, programistycznie wyglada jak full-duplex. tylko fizycznie usuwasz jeden przewod.
    // MISO/MOSI pins
    if(SPICONF.MODE==FULL_DUPLEX || SPICONF.MODE==SIMPLEX_RECEIVE || SPICONF.MODE==SIMPLEX_TRANSMIT){
        if(SPICONF.SIDE==MASTER){
            gpio_configure(GPIOA,MISO_PIN,INPUT_PUPD);                 //MISO, INPUT_FLOATING possible
            gpio_configure(GPIOA,MOSI_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);  //MOSI
        } else if (SPICONF.SIDE==SLAVE){
            gpio_configure(GPIOA,MOSI_PIN,INPUT_PUPD);             //MOSI, INPUT_FLOATING also possible           
            if(SPICONF.SLAVE_CONN==POINT_TO_POINT){gpio_configure(GPIOA,MISO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ); } //MISO
            else if(SPICONF.SLAVE_CONN==MULTISLAVE){gpio_configure(GPIOA,MISO_PIN,OUTPUT_ALT_ODRAIN_10MHZ); }  //MISO
        }
    } else if(SPICONF.MODE==HALF_DUPLEX){
        if(SPICONF.SIDE==MASTER){
            gpio_configure(GPIOA,MOSI_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);  //MOSI
        } else if (SPICONF.SIDE==SLAVE){
            if(SPICONF.SLAVE_CONN==POINT_TO_POINT){gpio_configure(GPIOA,MISO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ); } //MISO
            else if(SPICONF.SLAVE_CONN==MULTISLAVE){gpio_configure(GPIOA,MISO_PIN,OUTPUT_ALT_ODRAIN_10MHZ); }  //MISO
        }
    }

    //NSS PIN
    if(SPICONF.NSS_TYPE==NSS_DEFPIN && SPICONF.SIDE==MASTER ){
        gpio_configure(GPIOA,SPI1_NSS_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
    } else if(SPICONF.NSS_TYPE==NSS_DEFPIN && SPICONF.SIDE==SLAVE){
        gpio_configure(GPIOA,SPI1_NSS_PIN,INPUT_PUPD);
    } else if(SPICONF.NSS_TYPE==NSS_GPIO){
        gpio_configure(GPIOA,SPI1_NSS_PIN,INPUT_PUPD);
    }
}

// 21.07.21: JESZCZE REMAPOWANIE.
void _set_gpio_assign(SPI_CONF spi_conf){
    
    switch(spi_conf.SPI_NR){
        case SPI1:
            PIN_NSS.GPIO_PORT=GPIOA;PIN_NSS.GPIO_PIN=4;
            PIN_CLK.GPIO_PORT=GPIOA;PIN_NSS.GPIO_PIN=5;
            PIN_MISO.GPIO_PORT=GPIOA;PIN_NSS.GPIO_PIN=6;
            PIN_MOSI.GPIO_PORT=GPIOA;PIN_NSS.GPIO_PIN=7;
            break;
        case SPI2:
            PIN_NSS.GPIO_PORT=GPIOB;PIN_NSS.GPIO_PIN=12;
            PIN_CLK.GPIO_PORT=GPIOB;PIN_NSS.GPIO_PIN=13;
            PIN_MISO.GPIO_PORT=GPIOB;PIN_NSS.GPIO_PIN=14;
            PIN_MOSI.GPIO_PORT=GPIOB;PIN_NSS.GPIO_PIN=15;
            break;
    }
}


int _get_flag_status(int bit_position){
    return ((SPI->SR & (1<<bit_position)) >> bit_position);
}