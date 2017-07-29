#ifndef __SYS_TICKS_
#define __SYS_TICKS_

#include <stdint.h>

#define MAX_SYS_TICK_COUNT (0xFFFFFFFFUL)

uint8_t SysTicks_Init(uint32_t ticks);
uint8_t SysTicks_UnInit(void);
void SysTicks_IncTickCount(void);
uint32_t SysTicks_GetTickCount(void);
uint64_t SysTicks_GetUsTickCount(void);
void SysTicks_DelayMS(uint32_t msDelay);
void SysTicks_DelayUS(uint64_t usDelay);

void msleep(uint32_t millisecs);
void ssleep(uint32_t seconds);

uint32_t SysTicks_GetDiff(uint32_t u32_start, uint32_t u32_end);
uint64_t SysTicks_GetUsDiff(uint64_t u64_start, uint64_t u64_end);

#endif
