/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: driver_mutex.c
Description: 
Author: Wumin @ Artosy Software Team
Version: 0.0.1
Date: 2017/17/19
History:
         0.0.1    2017/08/02    for driver mutex used
*****************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "driver_mutex.h"
#include "debuglog.h"
#include "memory_config.h"
#include "cpu_info.h"

DRIVER_MUTEX_DATA *g_s_periMutex = (DRIVER_MUTEX_DATA*)(SRAM_PERIPERIAL_MUTEX_ADDR);

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

        case mutex_i2c:
            g_s_periMutex->i2c &=~ (1 << (channel*3));
            g_s_periMutex->i2c &=~ (CPUINFO_GetLocalCpuId() << (channel*3+1));
        break;
        
        case mutex_timer:
            if (channel < 8)
            {
                g_s_periMutex->s_timer.timer0to7 &=~ (1 << (channel*3));
                g_s_periMutex->s_timer.timer0to7 &=~ (CPUINFO_GetLocalCpuId() << (channel*3+1));
            }
            else if (channel < 16)
            {
                g_s_periMutex->s_timer.timer8to15 &=~ (1 << ((channel-8)*3));
                g_s_periMutex->s_timer.timer8to15 &=~ (CPUINFO_GetLocalCpuId() << ((channel-8)*3+1));
            }
            else
            {
                g_s_periMutex->s_timer.timer16to23 &=~ (1 << ((channel-16)*3));
                g_s_periMutex->s_timer.timer16to23 &=~ (CPUINFO_GetLocalCpuId() << ((channel-16)*3+1));
            }
        break;

        case mutex_nor_flash:
            g_s_periMutex->nor_flash &=~ (1 << (channel*3));
            g_s_periMutex->nor_flash &=~ (CPUINFO_GetLocalCpuId() << (channel*3+1));
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

        case mutex_i2c:
            g_s_periMutex->i2c |= (1 << (channel*3));
            g_s_periMutex->i2c |= (CPUINFO_GetLocalCpuId() << (channel*3+1));
        break;

        case mutex_timer:
            if (channel < 8)
            {
                g_s_periMutex->s_timer.timer0to7 |= (1 << (channel*3));
                g_s_periMutex->s_timer.timer0to7 |= (CPUINFO_GetLocalCpuId() << (channel*3+1));
            }
            else if (channel < 16)
            {
                g_s_periMutex->s_timer.timer8to15 |= (1 << ((channel-8)*3));
                g_s_periMutex->s_timer.timer8to15 |= (CPUINFO_GetLocalCpuId() << ((channel-8)*3+1));
            }
            else
            {
                g_s_periMutex->s_timer.timer16to23 |= (1 << ((channel-16)*3));
                g_s_periMutex->s_timer.timer16to23 |= (CPUINFO_GetLocalCpuId() << ((channel-16)*3+1));
            }
        break;

        case mutex_nor_flash:
            g_s_periMutex->nor_flash |= (1 << (channel*3));
            g_s_periMutex->nor_flash |= (CPUINFO_GetLocalCpuId() << (channel*3+1));
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
            if( g_s_periMutex->uart & (1 << (channel*3)) )
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
            if( g_s_periMutex->spi & (1 << (channel*3)) )
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
            if( g_s_periMutex->can & (1 << (channel*3)) )
            {
                cpu_id = ( (g_s_periMutex->can & cpu_id_mask) >> (channel*3 + 1) );
                if (cpu_id != CPUINFO_GetLocalCpuId())
                {
                    dlog_error("can channel:%d occupied", channel);
                    return -1;
                }
            }

        break;

        case mutex_i2c:
            if( g_s_periMutex->i2c & (1 << (channel*3)) )
            {
                cpu_id = ( (g_s_periMutex->i2c & cpu_id_mask) >> (channel*3 + 1) );
                if (cpu_id != CPUINFO_GetLocalCpuId())
                {
                    dlog_error("i2c channel:%d occupied", channel);
                    return -1;
                }
            }
        break;

        case mutex_timer:
            if (channel < 8)
            {
                if( g_s_periMutex->s_timer.timer0to7 & (1 << (channel*3) ) )
                {
                    cpu_id = ( (g_s_periMutex->s_timer.timer0to7 & cpu_id_mask) >> (channel*3 + 1) );
                    if (cpu_id != CPUINFO_GetLocalCpuId())
                    {
                        dlog_error("timer channel:%d occupied", channel);
                        return -1;
                    }
                }
            }
            else if (channel < 16)
            {
                cpu_id_mask = 0;
                cpu_id_mask |= (3 << (((channel-8)*3)+1));
                if( g_s_periMutex->s_timer.timer8to15 & (1 << ((channel-8)*3) ) )
                {
                    cpu_id = ( (g_s_periMutex->s_timer.timer8to15 & cpu_id_mask) >> ((channel-8)*3 + 1) );
                    if (cpu_id != CPUINFO_GetLocalCpuId())
                    {
                        dlog_error("timer channel:%d occupied", channel);
                        return -1;
                    }
                }
            }
            else
            {
                cpu_id_mask = 0;
                cpu_id_mask |= (3 << (((channel-16)*3)+1));
                dlog_info("line = %d, cpu_id_mask = 0x%08x", __LINE__, cpu_id_mask);
                if( g_s_periMutex->s_timer.timer16to23 & (1 << ((channel-16)*3) ) )
                {
                    dlog_info("line = %d", __LINE__);                        
                    cpu_id = ( (g_s_periMutex->s_timer.timer16to23 & cpu_id_mask) >> ((channel-16)*3 + 1) );
                    dlog_info("line = %d, cpu_id = %d", __LINE__, cpu_id);                        
                    if (cpu_id != CPUINFO_GetLocalCpuId())
                    {
                        dlog_error("timer channel:%d occupied", channel);
                        return -1;
                    }
                }
            }
        break;

        case mutex_nor_flash:
            if( g_s_periMutex->nor_flash & (1 << (channel*3)) )
            {
                cpu_id = ( (g_s_periMutex->nor_flash & cpu_id_mask) >> (channel*3 + 1) );
                if (cpu_id != CPUINFO_GetLocalCpuId())
                {
                    dlog_error("nor flash occupied");
                    return -1;
                }
            }
        break;

        default:break;
    }

    return 0;
}
