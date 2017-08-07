/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal.c
Description: The external HAL APIs for common driver functions.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/1/4
History: 
        0.0.1    2016/1/4    The initial version of hal_dma.c
                   2017/5/5    change the HAL api for users
*****************************************************************************/

#include <stdint.h>
#include "systicks.h"
#include "hal_ret_type.h"
#include "hal.h"
#include "dma.h"
#include "debuglog.h"
#include "cmsis_os.h"
#include "hal_dma.h"

HAL_RET_T HAL_DMA_init(void)
{
	DMA_initIRQ();
	
	return HAL_OK;
}

/** 
 * @brief   Start the DMA Transfer
 * @param   u32_srcAddress: The source memory Buffer address
 * @param   u32_dstAddress: The destination memory Buffer address
 * @param   u32_dataLength: The length of data to be transferred from source to destination
 * @param   u32_timeOut: timeout threshold, unit:ms
 * @return  none
 */
HAL_RET_T HAL_DMA_Transfer(uint32_t u32_srcAddr, 
											uint32_t u32_dstAddr, 
											uint32_t u32_transByteNum, 
											uint32_t u32_timeOut)
{
	uint8_t u8_chanIndex;
	uint32_t u32_start;
	
	for (u8_chanIndex = 0; u8_chanIndex < 4; u8_chanIndex++)
	{
		/* 
		 * channel 0 priority 4, 
		 * channel 1 priority 5, 
		 * channel 2 priority 6, 
		 * channel 3 priority 7
		 */
		if (DMA_Init(u8_chanIndex, 4 + u8_chanIndex) >= 0 )
		{
			DMA_transfer(u32_srcAddr, u32_dstAddr, u32_transByteNum, u8_chanIndex, LINK_LIST_ITEM);
			break;
		}
		else
		{
			//dlog_info("line = %d, no channel for channel %d\n", __LINE__, u8_chanIndex);
		}
	}

	if (u8_chanIndex >= 4)
	{
		dlog_info("line = %d, all 4 channel occupied!", __LINE__);
		return HAL_BUSY;
	}
	
	if ( 0 != u32_timeOut )
	{
		u32_start = SysTicks_GetTickCount();
		
		while( DMA_getStatus(u8_chanIndex) == 0 )
		{
            if ((SysTicks_GetDiff(u32_start, SysTicks_GetTickCount())) >= u32_timeOut)            
            {                 
                return HAL_TIME_OUT;            
            }            

            SysTicks_DelayUS(5);
		}
	}
	
	return HAL_OK;
}
