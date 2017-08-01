#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "driver_mutex.h"
#include "debuglog.h"
#include "memory_config.h"
#include "cpu_info.h"

PERIPERIAL_MUTEX_DATA *g_s_periMutex = (PERIPERIAL_MUTEX_DATA*)(SRAM_PERIPERIAL_MUTEX_ADDR);

void driver_mutex_free(emu_driver_mutex driver, uint32_t channel)
{
    switch (driver)
    {
        case mutex_uart:
            g_s_periMutex->uart &=~ (1 << (channel*3));
            g_s_periMutex->uart &=~ (CPUINFO_GetLocalCpuId() << (channel*3+1));
        break;

        case mutex_spi:
            g_s_periMutex->spi &=~ (1 << (channel*3));
            g_s_periMutex->spi &=~ (CPUINFO_GetLocalCpuId() << (channel*3+1));
        break;

        case mutex_can:
            g_s_periMutex->can &=~ (1 << (channel*3));
            g_s_periMutex->can &=~ (CPUINFO_GetLocalCpuId() << (channel*3+1));
       break;

        default:break;
    }
}

void driver_mutex_set(emu_driver_mutex driver, uint32_t channel)
{
    switch (driver)
    {
        case mutex_uart:
            g_s_periMutex->uart |= (1 << (channel*3));
            g_s_periMutex->uart |= (CPUINFO_GetLocalCpuId() << (channel*3+1));
        break;

        case mutex_spi:
            g_s_periMutex->spi |= (1 << (channel*3));
            g_s_periMutex->spi |= (CPUINFO_GetLocalCpuId() << (channel*3+1));
        break;

        case mutex_can:
            g_s_periMutex->can |= (1 << (channel*3));
            g_s_periMutex->can |= (CPUINFO_GetLocalCpuId() << (channel*3+1));
       break;

        default:break;
    }
}

int8_t driver_mutex_get(emu_driver_mutex driver, uint32_t channel)
{
    uint32_t cpu_id;
    uint32_t cpu_id_mask = 0;

/*
    dlog_info("addr of g_s_periMutex = %p", g_s_periMutex);
    dlog_info("addr of g_s_periMutex->uart = 0x%08x", g_s_periMutex->uart);
    dlog_info("addr of g_s_periMutex->spi = 0x%08x", g_s_periMutex->spi);
    dlog_info("addr of g_s_periMutex->can = 0x%08x", g_s_periMutex->can);
*/
    cpu_id_mask |= (3 << ((channel*3)+1));
    
    switch (driver)
    {
        case mutex_uart:
            if( g_s_periMutex->uart & (1 << channel) )
            {
                cpu_id = ( (g_s_periMutex->uart & cpu_id_mask) >> (channel*3 + 1) );
                if (cpu_id != CPUINFO_GetLocalCpuId())
                {
                    dlog_error("uart channel:%d occupied", channel);
                    return -1;
                }
            }
        break;

        case mutex_spi:
            if( g_s_periMutex->spi & (1 << channel) )
            {
                cpu_id = ( (g_s_periMutex->spi & cpu_id_mask) >> (channel*3 + 1) );
                if (cpu_id != CPUINFO_GetLocalCpuId())
                {
                    dlog_error("spi channel:%d occupied", channel);
                    return -1;
                }
            }

        break;

        case mutex_can:
            if( g_s_periMutex->can & (1 << channel) )
            {
                cpu_id = ( (g_s_periMutex->can & cpu_id_mask) >> (channel*3 + 1) );
                if (cpu_id != CPUINFO_GetLocalCpuId())
                {
                    dlog_error("can channel:%d occupied", channel);
                    return -1;
                }
            }

        break;

        default:break;
    }

    return 0;
}
