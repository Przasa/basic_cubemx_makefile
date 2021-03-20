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

typedef struct{
    SPI_MODE MODE;
    SPI_SIDE SIDE;
    SPI_NSS_TYPE NSS_TYPE;
}SPI_CONF;


#endif