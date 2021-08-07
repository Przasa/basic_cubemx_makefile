#include "spi_handler.h"
#include "gpio_handler.h"
#include "my_stm32f103xx.h"

#include "string.h"

//parkowanie 07.08.21:
// 1. test prostej komunikacji, masz juz wlasciwie wszystko (jezeli puunkt 2. nie blokuje)
// 2. funkcje do selelkcji slave'a -> troche juz jest
// 3. wsparcie dla SPI2
// 4. interrupy
// 5. czyszczenie smieci
// 6. zestaw testow.

//todo 20.03.21:    jakies makro na set i reset bitu?
//todo 20.03.21:    uzupelnij o remap.
//todo 25.03.21:    uzupelnij o SPI_NR2,SPI3... (jezeli sa) (i nawet ich remap). Pewnie trzeba powiekszyc configi
//todo 25.03.21:    uniewersalnosc pinow GPIO (np MISO_PIN ustawiany w zlaeznosci od rodzaju SPI)
//todo 25.03.21:    zrib driver dla NVICa
//todo 25.03.21:    sprawdz zegar --> done
//todo 25.03.21:    zaznaczanie slav'ow przez GPIO
//todo 25.03.21:    przesylanie przez przerwania.

//TODO 23.03.2021: Use it somehow.
// const DEF_CPOL = 0; DEF_CPHA=1;DEF_DFF=0;DEF_LSBFIRST=1;DEF_BRATE=0; DEF_SSOE=1;
// const DEF_SSOE=0;


// 06.08.21: UWAGA UWAGA: caly czas uzywasz rejestrow z SPI_NR1, przed implementacja pod SPI_NR2 bedzie duzo pracy. --> a moze wystarczy wrzuzcic numer SPI w parametrach funkcji.

//parkowanie 05.08.21:
// a) dokoncz definicje malych funkcji ponizej
// b) zrob podobne rzecz do transmisji i odbioru
// c) zaimplementuj je configure SPI (stare rzeczy wywal)
// d) przetestuj -> moze pomysl o testach jednostkowych!

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



SPI_CONF CURRENT_CONF;
// 07.08.21: przydalaby sie funkcja do selekcji slavow
void _configureGpioClock(SPI_NR spi_nr, SPI_ACON_REMAP remap);
void _configureSpiClock(SPI_NR spi_nr);
void _configureSpiGPIOs(SPI_SIDE side, SPI_MODE mode, SPI_ACON_NSSTYPE nss_type,SPI_ACON_SLAVEQTY slave_qty); //podzielic to jeszcze?
void _setSpiPins(SPI_NR spi_nr, SPI_ACON_REMAP remap);
void _configureFrame(SPI_ACON_CPHA cpha,SPI_ACON_CPOL cpol,SPI_ACON_DFF dff,SPI_ACON_LSBF lsbf); // podzielic?
void _configureBR(SPI_ACON_BR br);
void _configureSide(SPI_SIDE side);
void _configureBidi(SPI_MODE mode);
void _configureSlaveSelect(SPI_SIDE side,SPI_ACON_NSSTYPE nss_type);    //multimaster sobie darujemy












// ::::::::::::::


// :::::::::::::::: PUBLIC FUNCTIONS ::::::::::::::::::

uint8_t* quickStart(SPI_SIDE side){
    if(side==MASTER){
        char data_transmitted[] ="wysylana ramka."; 
        configure_spi(SPI_DEFCONF_MASTER);
        enable_spi(ENABLE);
        send_data(data_transmitted,strlen(data_transmitted));
        enable_spi(DISABLE);
        return data_transmitted;
    }else{
        char data_received[16];
        configure_spi(SPI_DEFCONF_SLAVE);
        enable_spi(ENABLE);
        receive_data((uint8_t*)receive_data,16);
        enable_spi(DISABLE);
        return data_received;
    }
}

void enable_spi(int ENORDI){
    while(_get_flag_status(SPI_BITPOS_SR_BSY));
    
    if(ENORDI==0){
        SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_SPE);    //SPI di
    }else{
        SPI->CR1 |=  (1<<SPI_BITPOS_CR1_SPE);    //SPI en
    }
}

void configure_spi(SPI_CONF conf){

    CURRENT_CONF= conf;

    _setSpiPins(conf.SPI_NR, conf.REMAP);
    _configureGpioClock(conf.SPI_NR, conf.REMAP);
    _configureSpiClock(conf.SPI_NR);
    _configureSpiGPIOs(conf.SIDE, conf.MODE , conf.NSS_TYPE ,conf.SLAVE_QTY ); //podzielic to jeszcze?
    _configureSide(conf.SIDE);
    _configureFrame(conf.CPHA,conf.CPOL,conf.DFF,conf.LSBF); // podzielic?
    _configureBR(conf.BR);
    _configureBidi(conf.MODE);
    _configureSlaveSelect(conf.SIDE,conf.NSS_TYPE);    //multimaster sobie darujemy
}




// master: 
//     send: SPI_DR
//     receive: SPE=1
// slave:
//     send: received CLK, 1st bit on MISO,
//     recive: received CLK, 1st bit on MOSI

// row function
// void send_data(uint8_t* TxBuffer, uint8_t len){
//     SPI->CR1 
    
// }


//moze moglibysmy uzyc referencji zamiast wskaznika ?
void send_data(uint8_t* transmit_buffer, int length){

    SPI_MODE mode = CURRENT_CONF.MODE;
    SPI_ACON_DFF dff = CURRENT_CONF.DFF;

    if(mode==HALF_DUPLEX)    {          SPI->CR1 |= (1<< SPI_BITPOS_CR1_BIDIOE);}

    if(mode==FULL_DUPLEX || mode==HALF_DUPLEX || mode==SIMPLEX_TRANSMIT){

        while(_get_flag_status(SPI_BITPOS_SR_TXE)!=1);
        
        while(length>0){
            if(dff==DFF_8){
                SPI->DR = *transmit_buffer;
                length--;
                transmit_buffer++;
            } else if(dff==DFF_16){
                SPI->DR = *((uint16_t*)transmit_buffer);
                length--;length--;
                (uint16_t*)transmit_buffer++;
            }
        }

    }

    if(mode==HALF_DUPLEX)    {          SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_BIDIOE);}



}

void receive_data(uint8_t* receive_buffer, int length){

    SPI_MODE mode = CURRENT_CONF.MODE;
    SPI_ACON_DFF dff = CURRENT_CONF.DFF;


    if(mode==HALF_DUPLEX)    {          SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_BIDIOE);}

    if(mode==FULL_DUPLEX || mode==HALF_DUPLEX || mode==SIMPLEX_RECEIVE){
    
        while(_get_flag_status(SPI_BITPOS_SR_RXNE)!=1);

        while(length>0){
            if(dff==DFF_8){
                *receive_buffer=SPI->DR;
                length--;
                receive_buffer++;
            } else if(dff==DFF_16){
                *((uint16_t*)receive_buffer) = SPI->DR;
                length--;length--;
                (uint16_t*)receive_buffer++;
            }
        }
    }

}




//:::::::::::::::::::::PRIVATE_HELPERS::::::::::::::::::::::::::::::::::

void _configureGpioClock(SPI_NR spi_nr, SPI_ACON_REMAP remap){
    if(spi_nr==SPI_NR1 && remap == REMAP_DEFAULT){
        if( !GPIOA_PCLK_GET()) GPIOA_PCLK_EN();
    } else if (spi_nr==SPI_NR1 && remap == REMAP_REMAPPED){
        if( !GPIOA_PCLK_GET()) GPIOA_PCLK_EN();
        if( !GPIOB_PCLK_GET()) GPIOB_PCLK_EN();
    } else if (spi_nr==SPI_NR2){
        if( !GPIOB_PCLK_GET()) GPIOB_PCLK_EN();
    }     
}

void _configureSpiClock(SPI_NR spi_nr){
    
         if (spi_nr==SPI_NR1 && !SPI1_PCLK_GET()){SPI1_PCLK_EN();}
    else if (spi_nr==SPI_NR2 && !SPI2_PCLK_GET()){SPI2_PCLK_EN();}
}

void _configureSpiGPIOs(SPI_SIDE side, SPI_MODE mode, SPI_ACON_NSSTYPE nss_type,SPI_ACON_SLAVEQTY slave_qty){


    //preferujesz input floating zamiast input pull-ip lub pull-down
    //nie wspierasz multi master
    if(mode==FULL_DUPLEX && side==MASTER && nss_type==NSS_DEFPIN && (slave_qty==POINT2POINT || slave_qty==MUTLI_SLAVE)){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
    }
    if(mode==FULL_DUPLEX && side==MASTER && (nss_type==NSS_GPIO || nss_type==NSS_SOFT) && (slave_qty==POINT2POINT || slave_qty==MUTLI_SLAVE)){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        // gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
    }
    if(mode==FULL_DUPLEX && side==SLAVE && (nss_type==NSS_DEFPIN || nss_type==NSS_GPIO) && slave_qty==POINT2POINT){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,INPUT_FLOATING);
    }
    if(mode==FULL_DUPLEX && side==SLAVE && (nss_type==NSS_DEFPIN || nss_type==NSS_GPIO) && slave_qty==MUTLI_SLAVE){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,OUTPUT_GPIO_ODRAIN_10MHZ);
        gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,INPUT_FLOATING);
    }
    if(mode==FULL_DUPLEX && side==SLAVE && nss_type==NSS_SOFT && (slave_qty==MUTLI_SLAVE || slave_qty==POINT2POINT)){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,INPUT_FLOATING);
        // gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,OUTPUT_GPIO_ODRAIN_10MHZ);
        gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,INPUT_FLOATING);
    }

    if(mode==HALF_DUPLEX && side==MASTER && nss_type==NSS_DEFPIN && (slave_qty==POINT2POINT || slave_qty==MUTLI_SLAVE)){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        // gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
    }
    if(mode==HALF_DUPLEX && side==MASTER && (nss_type==NSS_GPIO || nss_type==NSS_SOFT) && (slave_qty==POINT2POINT || slave_qty==MUTLI_SLAVE)){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        // gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        // gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
    }
    if(mode==HALF_DUPLEX && side==SLAVE && (nss_type==NSS_DEFPIN || nss_type==NSS_GPIO) && slave_qty==POINT2POINT){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        // gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,INPUT_FLOATING);
    }
    if(mode==HALF_DUPLEX && side==SLAVE && (nss_type==NSS_DEFPIN || nss_type==NSS_GPIO) && slave_qty==MUTLI_SLAVE){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,OUTPUT_GPIO_ODRAIN_10MHZ);
        // gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,INPUT_FLOATING);
    }
    if(mode==HALF_DUPLEX && side==SLAVE && nss_type==NSS_SOFT && (slave_qty==MUTLI_SLAVE || slave_qty==POINT2POINT)){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,INPUT_FLOATING);
        // gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,OUTPUT_GPIO_ODRAIN_10MHZ);
        // gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,INPUT_FLOATING);
    }

    if(mode==SIMPLEX_RECEIVE && side==MASTER && nss_type==NSS_DEFPIN && (slave_qty==POINT2POINT || slave_qty==MUTLI_SLAVE)){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,INPUT_FLOATING);
        // gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
    }
    if(mode==SIMPLEX_RECEIVE && side==MASTER && (nss_type==NSS_GPIO || nss_type==NSS_SOFT) && (slave_qty==POINT2POINT || slave_qty==MUTLI_SLAVE)){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        // gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,INPUT_FLOATING);
        // gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
    }
    if(mode==SIMPLEX_RECEIVE && side==SLAVE && (nss_type==NSS_DEFPIN || nss_type==NSS_GPIO) && (slave_qty==MUTLI_SLAVE || slave_qty==POINT2POINT)){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,INPUT_FLOATING);
        // gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,OUTPUT_GPIO_ODRAIN_10MHZ);
        gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,INPUT_FLOATING);
    }
    if(mode==SIMPLEX_RECEIVE && side==SLAVE && nss_type==NSS_SOFT && (slave_qty==MUTLI_SLAVE || slave_qty==POINT2POINT)){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,INPUT_FLOATING);
        // gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,INPUT_FLOATING);
        // gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,OUTPUT_GPIO_ODRAIN_10MHZ);
        gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,INPUT_FLOATING);
    }

    if(mode==SIMPLEX_TRANSMIT && side==MASTER && nss_type==NSS_DEFPIN && (slave_qty==POINT2POINT || slave_qty==MUTLI_SLAVE)){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        // gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
    }
    if(mode==SIMPLEX_TRANSMIT && side==MASTER && (nss_type==NSS_GPIO || nss_type==NSS_SOFT) && (slave_qty==POINT2POINT || slave_qty==MUTLI_SLAVE)){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        // gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        // gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
    }
    if(mode==SIMPLEX_TRANSMIT && side==SLAVE && (nss_type==NSS_DEFPIN || nss_type==NSS_GPIO) && slave_qty==POINT2POINT){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
        // gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,INPUT_FLOATING);
    }
    if(mode==SIMPLEX_TRANSMIT && side==SLAVE && (nss_type==NSS_DEFPIN || nss_type==NSS_GPIO) && slave_qty==MUTLI_SLAVE){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,OUTPUT_GPIO_ODRAIN_10MHZ);
        // gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,INPUT_FLOATING);
    }
    if(mode==SIMPLEX_TRANSMIT && side==SLAVE && nss_type==NSS_SOFT && (slave_qty==MUTLI_SLAVE || slave_qty==POINT2POINT)){
        gpio_configure(PIN_CLK.GPIO_PORT, PIN_CLK.GPIO_PIN,INPUT_FLOATING);
        // gpio_configure(PIN_NSS.GPIO_PORT, PIN_NSS.GPIO_PIN,INPUT_FLOATING);
        gpio_configure(PIN_MISO.GPIO_PORT, PIN_MISO.GPIO_PIN,OUTPUT_GPIO_ODRAIN_10MHZ);
        // gpio_configure(PIN_MOSI.GPIO_PORT, PIN_MOSI.GPIO_PIN,INPUT_FLOATING);
    }
}

void _setSpiPins(SPI_NR spi_nr, SPI_ACON_REMAP remap){ 
    if(spi_nr==SPI_NR1 && remap == REMAP_DEFAULT){
        PIN_CLK.GPIO_PORT=GPIOA; PIN_CLK.GPIO_PIN=5;
        PIN_NSS.GPIO_PORT=GPIOA; PIN_NSS.GPIO_PIN=4;
        PIN_MISO.GPIO_PORT=GPIOA; PIN_MISO.GPIO_PIN=6;
        PIN_MOSI.GPIO_PORT=GPIOA; PIN_MOSI.GPIO_PIN=7;
    } else if (spi_nr==SPI_NR1 && remap == REMAP_REMAPPED){
        PIN_CLK.GPIO_PORT=GPIOB; PIN_CLK.GPIO_PIN=3;
        PIN_NSS.GPIO_PORT=GPIOA; PIN_NSS.GPIO_PIN=15;
        PIN_MISO.GPIO_PORT=GPIOB; PIN_MISO.GPIO_PIN=4;
        PIN_MOSI.GPIO_PORT=GPIOB; PIN_MOSI.GPIO_PIN=5;
    } else if (spi_nr==SPI_NR2){
        PIN_CLK.GPIO_PORT=GPIOB; PIN_CLK.GPIO_PIN=13;
        PIN_NSS.GPIO_PORT=GPIOB; PIN_NSS.GPIO_PIN=12;
        PIN_MISO.GPIO_PORT=GPIOB; PIN_MISO.GPIO_PIN=14;
        PIN_MOSI.GPIO_PORT=GPIOB; PIN_MOSI.GPIO_PIN=15;
    } 
    
}

void _configureSlaveGPIO(SPI_GPIO spi_pin){
         if(spi_pin.GPIO_PORT==GPIOA){ GPIOA_PCLK_EN();}  
    else if(spi_pin.GPIO_PORT==GPIOB){ GPIOB_PCLK_EN();}  
    else if(spi_pin.GPIO_PORT==GPIOC){ GPIOC_PCLK_EN();}  
    else if(spi_pin.GPIO_PORT==GPIOD){ GPIOD_PCLK_EN();}  
    else if(spi_pin.GPIO_PORT==GPIOE){ GPIOE_PCLK_EN();}  

    gpio_configure(spi_pin.GPIO_PORT,spi_pin.GPIO_PIN,OUTPUT_GPIO_PUSHPULL_10MHZ);

}


void _configureFrame(SPI_ACON_CPHA cpha,SPI_ACON_CPOL cpol,SPI_ACON_DFF dff,SPI_ACON_LSBF lsbf){
    SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_CPHA);
    SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_CPOL);
    SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_DFF);
    SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_LSBF);

    SPI->CR1 |=  (cpha<< SPI_BITPOS_CR1_CPHA);
    SPI->CR1 |=  (cpol<< SPI_BITPOS_CR1_CPOL);
    SPI->CR1 |=  (dff<< SPI_BITPOS_CR1_DFF);
    SPI->CR1 |=  (lsbf<< SPI_BITPOS_CR1_LSBF);

}


void _configureBR(SPI_ACON_BR br){
        SPI->CR1 |=  (1<< SPI_BITPOS_CR1_BR);
}


void _configureSide(SPI_SIDE side){
    if(side==MASTER){
        SPI->CR1 |=  (1<< SPI_BITPOS_CR1_MSTR);
    } else {
        SPI->CR1 &=  ~(1<< SPI_BITPOS_CR1_MSTR);
    }
}


void _configureSlaveSelect(SPI_SIDE side,SPI_ACON_NSSTYPE nss_type){
    if((side==SLAVE) & (nss_type=NSS_SOFT)){
        SPI->CR1 |=  (1<< SPI_BITPOS_CR1_SSM);
    } else{
        SPI->CR1 &= ~(1<< SPI_BITPOS_CR1_SSM);
    }
}


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
void selectSlaveHW(int ENORDI,SPI_ACON_NSSTYPE nss_type, SPI_GPIO spi_pin){
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

// 5.08.21: przeniesc do _.h
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



// :::::::::::::::: smieci :::::::::::::::::::::::::::::

// void configure_spi_old(SPI_CONF conf){

//     if(!isConfiguredAdvanced(ADVCONF)) _set_advconf(ADVCONF_DEFAULT);
    
//     configureClock();
    
//     if(!SPI1_PCLK_GET()){SPI1_PCLK_EN();}

//     // 21.07.21: uwazaj: mozesz wysypc sie przy adv_config i remapowaniu. wymysl cos.
//     _set_gpios(conf);

//     //todo 25.03.2021: moze to oc ponizej tez wydzielic to jakiejs funkcji?



//     //MASTER/SLAVE
//     SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_MSTR);    
//     SPI->CR1 |=  (conf.SIDE<<SPI_BITPOS_CR1_MSTR);    

//     //COMM_MODES
//     switch (conf.MODE)
//     {
//         SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_BIDIMODE);   
//         SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_RXONLY);   
//         SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_BIDIOE);   

//         case FULL_DUPLEX:
//             break;
//         case HALF_DUPLEX:
//             //w transimicie/receive trzeba ustawiac: SPI->CR1 (&=)/(|=) (~)(1<<SPI_BITPOS_CR1_BIDIOE); 
//             SPI->CR1 |=  (1<<SPI_BITPOS_CR1_BIDIMODE); 
//             break;
//         case SIMPLEX_RECEIVE:
//             SPI->CR1 |=  (1<<SPI_BITPOS_CR1_RXONLY);   
//             break;
//         case SIMPLEX_TRANSMIT:
//             break;
//         default:
//             break;
//     }

// }

// void _conf_afio(SPI_CONF CONF);
// void _set_gpios(SPI_CONF CONF);
// void _set_gpio_assign(SPICONF);
// void _switch_bidi(BIDI_DIRECTION direction);    //moze jako public?
// void _set_advconf(SPI_ADVCONF CONF);


// void  _set_advconf(SPI_ADVCONF adv_conf){
//     ADVCONF=adv_conf;

//     _set_gpios(adv_conf.SPI_NR);
//     SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_CPOL);    //cpol
//     SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_CPHA);    //cpha
//     SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_DFF);   //DFF
//     SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_LSBF);    //LSBFFIRT =1
//     SPI->CR1 &=  ~(7<<SPI_BITPOS_CR1_BR);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)
//     SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_SSM);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)

//     SPI->CR1 |=  (ADVCONF.CPOL<<SPI_BITPOS_CR1_CPOL);    //cpol
//     SPI->CR1 |=  (ADVCONF.CPHA<<SPI_BITPOS_CR1_CPHA);    //cpha
//     SPI->CR1 |=  (ADVCONF.DFF<<SPI_BITPOS_CR1_DFF);   //DFF
//     SPI->CR1 |=  (ADVCONF.LSBF<<SPI_BITPOS_CR1_LSBF);    //LSBFFIRT =1
//     SPI->CR1 |=  (ADVCONF.BR<<SPI_BITPOS_CR1_BR);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)
    
//     SPI->CR1 &=  ~(1<<SPI_BITPOS_CR1_SSM);    //baoud rate TODO 20.03.2021: boud rate to conf (tutaj pclk/32)
    
    
// }

// void configure_spi_adv(SPI_CONF conf, SPI_ADVCONF adv_conf){
//     //ustawianie defaultow. TODO: 23.02.2021 --> ok
//     ADVCONF = adv_conf;
//     configure_spi(conf);
// }



// //potrzebne wogole?
// void _select_slave(){
//     //TODO 22.03.2021: albo przez NSS, albo przez GPIO!
// }

// void _switch_bidi(BIDI_DIRECTION DIR){
//     //todo 23.03.2021: spprwdz czy bidi smiga.
//     if(DIR==RECEIVE){       SPI->CR1 &= ~(1<<SPI_BITPOS_CR1_BIDIOE);    }
//     else if(DIR==TRANSMIT){ SPI->CR1 |=  (1<<SPI_BITPOS_CR1_BIDIOE);    }
    
// }

// void _conf_afio(SPI_CONF SPICONF){   
//     //TODO    
//     // if(!AFIO_PCLK_GET()) {AFIO_PCLK_EN();}
    
//     // if(SPICONF.REMAP==REMAP_DEFAULT){
//     //     AFIO->MAPR &= ~(1<<0);
//     // } else if(SPICONF.REMAP==REMAP_REMAPPED){
//     //     AFIO->MAPR |=  (1<<0);
//     // }
// }

// void _set_gpios(ADVCONF adv_conf){

//     _set_gpio_assign(SPI_NR spi_nr);
    

//     //TODO 25.03.2021 malo to czytelne, nie zagniezdzaj ifow. NSS jest OK

//     if(!GPIOA_PCLK_GET()) {GPIOC_PCLK_EN();}

//     //SCK PIN
//     if(SPICONF.SIDE==MASTER){ gpio_configure(GPIOA,SCK_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);}
//     else if(SPICONF.SIDE==SLAVE){ gpio_configure(GPIOA,SCK_PIN,INPUT_FLOATING);}

//     // note: simplex trans i rec, programistycznie wyglada jak full-duplex. tylko fizycznie usuwasz jeden przewod.
//     // MISO/MOSI pins
//     if(SPICONF.MODE==FULL_DUPLEX || SPICONF.MODE==SIMPLEX_RECEIVE || SPICONF.MODE==SIMPLEX_TRANSMIT){
//         if(SPICONF.SIDE==MASTER){
//             gpio_configure(GPIOA,MISO_PIN,INPUT_PUPD);                 //MISO, INPUT_FLOATING possible
//             gpio_configure(GPIOA,MOSI_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);  //MOSI
//         } else if (SPICONF.SIDE==SLAVE){
//             gpio_configure(GPIOA,MOSI_PIN,INPUT_PUPD);             //MOSI, INPUT_FLOATING also possible           
//             if(SPICONF.SLAVE_CONN==POINT_TO_POINT){gpio_configure(GPIOA,MISO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ); } //MISO
//             else if(SPICONF.SLAVE_CONN==MULTISLAVE){gpio_configure(GPIOA,MISO_PIN,OUTPUT_ALT_ODRAIN_10MHZ); }  //MISO
//         }
//     } else if(SPICONF.MODE==HALF_DUPLEX){
//         if(SPICONF.SIDE==MASTER){
//             gpio_configure(GPIOA,MOSI_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);  //MOSI
//         } else if (SPICONF.SIDE==SLAVE){
//             if(SPICONF.SLAVE_CONN==POINT_TO_POINT){gpio_configure(GPIOA,MISO_PIN,OUTPUT_ALT_PUSHPULL_10MHZ); } //MISO
//             else if(SPICONF.SLAVE_CONN==MULTISLAVE){gpio_configure(GPIOA,MISO_PIN,OUTPUT_ALT_ODRAIN_10MHZ); }  //MISO
//         }
//     }

//     //NSS PIN
//     if(SPICONF.NSS_TYPE==NSS_DEFPIN && SPICONF.SIDE==MASTER ){
//         gpio_configure(GPIOA,SPI1_NSS_PIN,OUTPUT_ALT_PUSHPULL_10MHZ);
//     } else if(SPICONF.NSS_TYPE==NSS_DEFPIN && SPICONF.SIDE==SLAVE){
//         gpio_configure(GPIOA,SPI1_NSS_PIN,INPUT_PUPD);
//     } else if(SPICONF.NSS_TYPE==NSS_GPIO){
//         gpio_configure(GPIOA,SPI1_NSS_PIN,INPUT_PUPD);
//     }
// }

// // 21.07.21: JESZCZE REMAPOWANIE.
// void _set_gpio_assign(SPI_CONF spi_conf){
    
//     switch(spi_conf.SPI_NR){
//         case SPI_NR1:
//             PIN_NSS.GPIO_PORT=GPIOA;PIN_NSS.GPIO_PIN=4;
//             PIN_CLK.GPIO_PORT=GPIOA;PIN_NSS.GPIO_PIN=5;
//             PIN_MISO.GPIO_PORT=GPIOA;PIN_NSS.GPIO_PIN=6;
//             PIN_MOSI.GPIO_PORT=GPIOA;PIN_NSS.GPIO_PIN=7;
//             break;
//         case SPI_NR2:
//             PIN_NSS.GPIO_PORT=GPIOB;PIN_NSS.GPIO_PIN=12;
//             PIN_CLK.GPIO_PORT=GPIOB;PIN_NSS.GPIO_PIN=13;
//             PIN_MISO.GPIO_PORT=GPIOB;PIN_NSS.GPIO_PIN=14;
//             PIN_MOSI.GPIO_PORT=GPIOB;PIN_NSS.GPIO_PIN=15;
//             break;
//     }
// }


// int _get_flag_status(int bit_position){

//     return ((SPI->SR & (1<<bit_position)) >> bit_position);
// }