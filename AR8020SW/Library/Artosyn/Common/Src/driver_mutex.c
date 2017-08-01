#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "driver_mutex.h"
#include "debuglog.h"
#include "memory_config.h"

PERIPERIAL_MUTEX_DATA *g_s_periMutex = (PERIPERIAL_MUTEX_DATA*)(SRAM_PERIPERIAL_MUTEX_ADDR);

void driver_mutex_set(emu_Mutex_Periperal driver, uint32_t channel)
{
    switch (driver)
    {
        case mutex_uart:
            g_s_periMutex->uart |= (1 << channel);
        break;

        case mutex_spi:
            g_s_periMutex->spi |= (1 << channel);
        break;

        case mutex_can:
            g_s_periMutex->can |= (1 << channel);
        break;

        default:break;
    }
}

int8_t driver_mutex_get(emu_Mutex_Periperal driver, uint32_t channel)
{
    switch (driver)
    {
        case mutex_uart:
            if( g_s_periMutex->uart & (1 << channel) )
            {
                dlog_error("uart channel:%d occupied", channel);
                return -1;
            }
        break;

        case mutex_spi:
            if (g_s_periMutex->spi & (1 << channel))
            {
                dlog_error("spi channel:%d occupied", channel);
                return -1;
            }
        break;

        case mutex_can:
            if (g_s_periMutex->can & (1 << channel))
            {
                dlog_error("can channel:%d occupied", channel);
                return -1;
            }
        break;

        default:break;
    }

    return 0;
}
