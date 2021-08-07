#ifndef SPIHANDLER_H
#define SPIHANDLER_H

#include "my_stm32f103xx.h"




// TODO 20.03.2021: zrob podobna konktrukcje jak przy SPI_ADVCONF
typedef enum{
    FULL_DUPLEX,HALF_DUPLEX,SIMPLEX_RECEIVE,SIMPLEX_TRANSMIT
}SPI_MODE;

typedef enum{
    SLAVE=0,MASTER=1
}SPI_SIDE;




typedef struct{
    SPI_MODE MODE;
    SPI_SIDE SIDE;
    // SPI_SLVSEL SLVSEL;           // to nie konfigu ale osobnej funkcji
    // SPI_ACON_NSSTYPE NSS_TYPE;  // do advanced config
    // SPI_ACON_SLAVES SLAVE_CONN; // do advanced config
    // SPI_ACON_REMAP REMAP;    // to bym dal do advanced config
}SPI_CONF;

//:::::::::::::::::::::::::ADVANCED CONFIG:::::::::::::::::::::::::::::::::::::
//TODO 21.07.2021: WYWAL NIEPOTRZEBNE WARTOSCI.

// konfiguracja roznych SPI na pozniej
// lepiej to nazwac SPI_GPIO
typedef struct{
    GPIO_RegDef_t* GPIO_PORT;
    uint16_t    GPIO_PIN;
}SPI_PIN;
SPI_PIN PIN_NSS, PIN_MISO,PIN_MOSI,PIN_CLK;

// SPI_PIN PIN_SET[4] = {PIN_NSS,PIN_CLK,PIN_MISO,PIN_MOSI};

typedef struct{
    uint16_t SLVSEL_GPIO_PORT;
    uint16_t SLVSEL_GPIO_PIN;
}SPI_SLVSEL;

typedef enum{
    SPI_NR1=1,SPI_NR2=2
}SPI_NR;

typedef enum{
    POINT2POINT, MUTLI_SLAVE
}SPI_ACON_SLAVEQTY;

typedef enum{
    REMAP_DEFAULT,REMAP_REMAPPED
}SPI_ACON_REMAP;

typedef enum{
    NSS_GPIO,NSS_DEFPIN,NSS_SOFT
}SPI_ACON_NSSTYPE;

//todo 21.07.21: dla kogo to?
// typedef enum{
//     GPIO_SLV_SELECT
// }SPI_ACON_SLVSEL;

// typedef enum{
//     MULTISLAVE,POINT_TO_POINT
// }SPI_ACON_SLAVES;

typedef enum
{
    DFF_8=0,DFF_16=1
}SPI_ACON_DFF;

typedef enum
{
    CPHA_FIRST=0,CPHA_SECOND=1
}SPI_ACON_CPHA;

typedef enum
{
    CPOL_LOW=0,CPOL_HIGH=1
}SPI_ACON_CPOL;

typedef enum
{
    BR_DIV2=0,BR_DIV4=1,BR_DIV8=2,BR_DIV16=3,BR_DIV32=4,BR_DIV64=5,BR_DIV128=6,BR_DIV256=7
}SPI_ACON_BR;

typedef enum
{
    LSBF_MSB=0,LSBF_LSB=1
}SPI_ACON_LSBF;

// // DLA KOGO TO?
// typedef enum{
//     SSOE_DI=0,SSOE_EN=1
// }SPI_ACON_SSOE;

typedef struct{
    SPI_NR SPI_NR;
    SPI_ACON_REMAP REMAP;    
    SPI_ACON_NSSTYPE NSS_TYPE;  
    SPI_ACON_SLAVEQTY SLAVE_QTY;
    // SPI_ACON_SLVSEL SLAVE_SELECT;
    // SPI_ACON_SLAVES SLAVE_CONN; 
    SPI_ACON_DFF     DFF;
    SPI_ACON_CPHA    CPHA;
    SPI_ACON_CPOL    CPOL;
    SPI_ACON_BR      BR;
    SPI_ACON_LSBF    LSBF;
    // SPI_ACON_SSOE    SSOE;
}SPI_ADVCONF;




//:::::::::::::::::::::::::PUBLIC METHODS:::::::::::::::::::::::::::::::::::::
void configure_spi(SPI_CONF CONF);
void configure_spi_adv(SPI_CONF CONF, SPI_ADVCONF ADC_CONF);
void enable_spi(int ENORDI);
void send_data(uint8_t* transmit_buffer, int length);
void receive_data(uint8_t* receive_buffer, int length);


#endif