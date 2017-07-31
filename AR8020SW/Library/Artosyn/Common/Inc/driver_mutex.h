#ifndef _DRIVER_MUTEX_H
#define _DRIVER_MUTEX_H

/*
#define    MUTEX_UART 0
#define    MUTEX_SPI 1
#define    MUTEX_CAN 2
*/


typedef struct
{
    uint32_t uart;
    uint32_t spi;
    uint32_t can;
}PERIPERIAL_MUTEX_DATA;

extern PERIPERIAL_MUTEX_DATA *g_s_periMutex;

typedef enum _emu_Mutex_Periperal
{
    mutex_uart = 0,
    mutex_spi,
    mutex_can
}emu_Mutex_Periperal;

void periperal_set_occupied(emu_Mutex_Periperal driver, uint32_t channel);
int8_t periperal_get_free(emu_Mutex_Periperal driver, uint32_t channel);

#endif
