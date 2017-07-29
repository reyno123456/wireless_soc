#ifndef __TEST_HAL_SPI_H__
#define __TEST_HAL_SPI_H__

#include "hal_spi.h"
#include "debuglog.h"

void command_TestHalSpiInit(unsigned char *ch, unsigned char *br, unsigned char *polarity, unsigned char *phase);

void command_TestHalSpiTx(unsigned char *ch,  unsigned char *addr, unsigned char *wdata);

void command_TestHalSpiRx(unsigned char *ch, unsigned char *addr);

#endif
