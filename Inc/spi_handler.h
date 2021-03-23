#ifndef SPIHANDLER_H
#define SPIHANDLER_H

#include "my_stm32f103xx.h"


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
//advanced: TODO: 23.02.2021
typedef struct{
    
}ADV_CONF;
    

#endif