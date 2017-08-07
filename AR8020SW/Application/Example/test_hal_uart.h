#ifndef __TEST_HAL_UART_H__
#define __TEST_HAL_UART_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include "hal_uart.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "debuglog.h"

void command_TestHalUartIntSet(unsigned char *ch, unsigned char *flag);

void command_TestHalUartInit(unsigned char *ch, unsigned char *br);

void command_TestHalUartTx(unsigned char *ch, unsigned char *len);

void command_TestHalUartRx(unsigned char *ch);

void test_uart_with_os();


#ifdef __cplusplus
}
#endif 

#endif
