#ifndef SPIHANDLER_H
#define SPIHANDLER_H

#include "my_stm32f103xx.h"


//TODO 21.07.2021: WYWAL NIEPOTRZEBNE WARTOSCI.


const SPI_CONF SPI_DEFCONF_MASTER={
    SPI_NR1,
    REMAP_DEFAULT,
    FULL_DUPLEX,
    MASTER,
    NSS_DEFPIN,
    POINT2POINT,
    DFF_8,
    CPHA_SECOND,
    CPOL_HIGH,
    BR_DIV32,
    LSBF_MSB
};

const SPI_CONF SPI_DEFCONF_SLAVE={
    SPI_NR1,
    REMAP_DEFAULT,
    FULL_DUPLEX,
    SLAVE,
    NSS_DEFPIN,
    POINT2POINT,
    DFF_8,
    CPHA_SECOND,
    CPOL_HIGH,
    BR_DIV32,
    LSBF_MSB
};



typedef enum{
    FULL_DUPLEX,HALF_DUPLEX,SIMPLEX_RECEIVE,SIMPLEX_TRANSMIT
}SPI_MODE;

typedef enum{
    SLAVE=0,MASTER=1
}SPI_SIDE;

typedef struct{
    GPIO_RegDef_t* GPIO_PORT;
    uint16_t    GPIO_PIN;
}SPI_GPIO;
SPI_GPIO PIN_NSS, PIN_MISO,PIN_MOSI,PIN_CLK;

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


typedef struct{
    SPI_NR SPI_NR;
    SPI_ACON_REMAP REMAP;    
    SPI_MODE MODE;
    SPI_SIDE SIDE;
    SPI_ACON_NSSTYPE NSS_TYPE;  
    SPI_ACON_SLAVEQTY SLAVE_QTY;
    SPI_ACON_DFF     DFF;
    SPI_ACON_CPHA    CPHA;
    SPI_ACON_CPOL    CPOL;
    SPI_ACON_BR      BR;
    SPI_ACON_LSBF    LSBF;
}SPI_CONF;


//todo 21.07.21: dla kogo to?
// typedef enum{
//     GPIO_SLV_SELECT
// }SPI_ACON_SLVSEL;

// typedef enum{
//     MULTISLAVE,POINT_TO_POINT
// }SPI_ACON_SLAVES;

// // DLA KOGO TO?
// typedef enum{
//     SSOE_DI=0,SSOE_EN=1
// }SPI_ACON_SSOE;

// typedef struct{
//     SPI_NR SPI_NR;
//     SPI_ACON_REMAP REMAP;    
//     SPI_ACON_NSSTYPE NSS_TYPE;  
//     SPI_ACON_SLAVEQTY SLAVE_QTY;
//     SPI_ACON_DFF     DFF;
//     SPI_ACON_CPHA    CPHA;
//     SPI_ACON_CPOL    CPOL;
//     SPI_ACON_BR      BR;
//     SPI_ACON_LSBF    LSBF;
//     // SPI_ACON_SSOE    SSOE;
// }SPI_ADVCONF;




//:::::::::::::::::::::::::PUBLIC METHODS:::::::::::::::::::::::::::::::::::::
void configure_spi(SPI_CONF spi_conf);
void enable_spi(int ENORDI);
void send_data(uint8_t* transmit_buffer, int length);
void receive_data(uint8_t* receive_buffer, int length);
void selectSlaveHW(int ENORDI,SPI_ACON_NSSTYPE nss_type, SPI_GPIO spi_pin);
void selectSlaveSW(int ENORDI);


#endif