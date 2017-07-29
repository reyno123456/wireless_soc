#include <stdlib.h>
#include "test_hal_uart.h"
#include "hal_nvic.h"
#include "debuglog.h"

uint8_t s_u8_uartRxBuf[64];
uint8_t s_u8_uartRxLen = 0;

static uint32_t uartRxCallBack(uint8_t *pu8_rxBuf, uint8_t u8_len);

void command_TestHalUartIntSet(unsigned char *ch, unsigned char *flag)
{
    unsigned int u32_ch = strtoul(ch, NULL, 0);
    unsigned int u32_flag = strtoul(flag, NULL, 0);

    if(0 == u32_flag)
    {
       HAL_NVIC_DisableIrq(u32_ch + HAL_NVIC_UART_INTR0_VECTOR_NUM);
    }
    else
    {
        HAL_NVIC_EnableIrq(u32_ch + HAL_NVIC_UART_INTR0_VECTOR_NUM);
    }
}

void command_TestHalUartInit(unsigned char *ch, unsigned char *br)
{
    unsigned int u32_ch = strtoul(ch, NULL, 0);
    unsigned int u32_br = strtoul(br, NULL, 0);

    memset(s_u8_uartRxBuf, sizeof(s_u8_uartRxBuf), 0x55);

    HAL_UART_Init((ENUM_HAL_UART_COMPONENT)(u32_ch), 
                  (ENUM_HAL_UART_BAUDR)(u32_br), 
                  uartRxCallBack);
}

void command_TestHalUartTx(unsigned char *ch, unsigned char *len)
{
    uint16_t u16_i;
    unsigned int u32_ch = strtoul(ch, NULL, 0);
    unsigned int u32_len = strtoul(len, NULL, 0);
    uint8_t *u8_data = malloc(u32_len);

    if (NULL == u8_data)
    {
        dlog_error("malloc error");
    }

    for(u16_i = 0; u16_i < (uint16_t)u32_len; u16_i++)
    {
        u8_data[u16_i] = u16_i;
        
    }

    HAL_UART_TxData((ENUM_HAL_UART_COMPONENT)(u32_ch), 
                    u8_data, 
                    u16_i,
                    HAL_UART_DEFAULT_TIMEOUTMS);

    free(u8_data);
}

void command_TestHalUartRx(unsigned char *ch)
{
    unsigned int u32_ch = strtoul(ch, NULL, 0);
    uint16_t u16_i;
    
    HAL_UART_TxData((ENUM_HAL_UART_COMPONENT)(u32_ch), 
                     s_u8_uartRxBuf, 
                     s_u8_uartRxLen,
                     HAL_UART_DEFAULT_TIMEOUTMS);

    s_u8_uartRxLen = 0;
}

static uint32_t uartRxCallBack(uint8_t *pu8_rxBuf, uint8_t u8_len)
{
    if (u8_len > 64)
    {
        u8_len = 64;
    }
    
    memcpy(s_u8_uartRxBuf + s_u8_uartRxLen, pu8_rxBuf, u8_len);

    s_u8_uartRxLen += u8_len;
}
