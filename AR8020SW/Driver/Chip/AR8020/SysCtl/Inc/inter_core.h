#ifndef __INTER_CORE_H
#define __INTER_CORE_H

#include <stdint.h>
#include "sys_event.h"
#include "memory_config.h"

//#define INTER_CORE_DEBUG_LOG_ENABLE

/* need to modify after debug ok */
#define INTER_CORE_MSG_SHARE_MEMORY_BASE_ADDR     SRAM_INTER_CORE_MSG_SHARE_MEMORY_ST_ADDRESS
#define INTER_CORE_MSG_SHARE_MEMORY_NUMBER        5
#define INTER_CORE_MSG_SHARE_MEMORY_DATA_LENGTH   16

#define INTER_CORE_TRIGGER_REG_ADDR               0xA003004C
#define INTER_CORE_TRIGGER_IRQ0_BITMAP            0x00000001
#define INTER_CORE_TRIGGER_IRQ1_BITMAP            0x00000002

typedef struct
{
    INTER_CORE_CPU_ID    enSrcCpuID                                     __attribute__ ((aligned (4)));
    INTER_CORE_CPU_ID    enDstCpuID                                     __attribute__ ((aligned (4)));
    uint32_t             dataAccessed                                   __attribute__ ((aligned (4)));
    uint32_t             lock                                           __attribute__ ((aligned (4)));
    INTER_CORE_MSG_ID    enMsgID                                        __attribute__ ((aligned (4)));
    uint8_t              data[INTER_CORE_MSG_SHARE_MEMORY_DATA_LENGTH]  __attribute__ ((aligned (4)));
}INTER_CORE_MSG_TYPE;

void SRAM_SKY_BypassVideoConfig(uint32_t channel);

void SRAM_DCacheDisable(uint8_t type);


void InterCore_Init(void);
uint8_t InterCore_SendMsg(INTER_CORE_CPU_ID dst, INTER_CORE_MSG_ID msg, uint8_t* buf, uint32_t length);
uint8_t InterCore_GetMsg(INTER_CORE_MSG_ID* msg_p, uint8_t* buf, uint32_t max_length);

#endif
