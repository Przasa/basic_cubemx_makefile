#ifndef SPIHANDLER_H
#define SPIHANDLER_H

#include "my_stm32f103xx.h"

// TODO 20.03.2021: zrob podobna konktrukcje jak przy SPI_COMM_PARAMS
typedef enum{
    FULL_DUPLEX,SIMPLEX_BIDI,SIMPLEX_RECEIVE,SIMPLEX_TRANSIM
}SPI_MODE;

typedef enum{
    MASTER,SLAVE
}SPI_SIDE;

typedef enum{
    HARDWARE,SOFTWARE
}SPI_NSS_TYPE;

typedef enum{
    MULTISLAVE,POINT_TO_POINT
}SPI_SLAVES_CONN;

typedef enum{
    DEFAULT,REMAPPED
}SPI_REMAP;

typedef struct{
    SPI_MODE MODE;
    SPI_SIDE SIDE;
    SPI_NSS_TYPE NSS_TYPE;
    SPI_SLAVES_CONN SLAVE_CONN;
    SPI_REMAP REMAP;
}SPI_CONF;

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
typedef enum
{
    DFF_8=0,DFF_16=1
}SPI_DFF;
typedef enum
{
    CPHA_FIRST=0,CPHA_SECOND=1
}SPI_CPHA;
typedef enum
{
    CPOL_LOW=0,CPOL_HIGH=1
}SPI_CPOL;
typedef enum
{
    BR_DIV2=0,BR_DIV4=1,BR_DIV8=2,BR_DIV16=3,BR_DIV32=4,BR_DIV64=5,BR_DIV128=6,BR_DIV256=7
}SPI_BR;
typedef enum
{
    LSBF_MSB=0,LSBF_LSB=1
}SPI_LSBF;

typedef struct{
    SPI_DFF     DFF;
    SPI_CPHA    CPHA;
    SPI_CPOL    CPOL;
    SPI_BR      BR;
    SPI_LSBF    LSBF;
}SPI_COMM_PARAMS;
    

#endif