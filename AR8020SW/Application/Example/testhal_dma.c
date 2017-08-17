/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: testhal_pwm.c
Description: 
Author: Wumin @ Artosy Software Team
Version: 0.0.1
Date: 2016/12/19
History:
         0.0.1    2016/12/19    test pwm
         0.0.2    2017/3/27     seperated to share
*****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "debuglog.h"
#include "hal_dma.h"
#include "md5.h"
#include "data_type.h"
#include "cmsis_os.h"
#include "systicks.h"

void command_dma(char * u32_src, char *u32_dst, char *u32_byteNum)
{
    unsigned int iSrcAddr;
    unsigned int iDstAddr;
    unsigned int iNum;

    iDstAddr    = strtoul(u32_dst, NULL, 0);
    iSrcAddr    = strtoul(u32_src, NULL, 0);
    iNum        = strtoul(u32_byteNum, NULL, 0);


    HAL_DMA_Transfer(iSrcAddr, iDstAddr, iNum, 10);
	    
    //#define MD5_SIZE 16
    uint8_t    md5_value[MD5_SIZE];
    int i = 0;
    MD5_CTX md5;
    MD5Init(&md5);
    MD5Update(&md5, (uint8_t *)iSrcAddr, iNum);
    MD5Final(&md5, md5_value);
    dlog_info("src MD5 = 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++]);

    memset(&md5, 0, sizeof(MD5_CTX));
    MD5Init(&md5);
    MD5Update(&md5, (uint8_t *)iDstAddr, iNum);
    memset(&md5_value, 0, sizeof(md5_value));
    MD5Final(&md5, md5_value);
    i = 0;
    dlog_info("dst MD5 = 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++]);
}

void command_test_dma_loop(char * u32_src, char *u32_dst, char *u32_byteNum)
{
	unsigned int i = 0;
	
	while(1)
	{
		command_dma(u32_src, u32_dst, u32_byteNum);
		dlog_info("i = %d\n", i++);
	}
}

typedef enum {
	DMA_blocked = 0,
	DMA_noneBlocked,
	DMA_blockTimer
} ENUM_blockMode;

static void delay_us(uint64_t us)
{
	uint64_t start;
	uint64_t end;
	start = SysTicks_GetUsTickCount();

	while(1)
	{
		if (SysTicks_GetUsTickCount() >= start + us)
		{
			break;
		}
	}
}

extern uint32_t DMA_forDriverTransfer(uint32_t u32_srcAddr, uint32_t u32_dstAddr, uint32_t u32_transByteNum, 
											ENUM_blockMode e_blockMode, uint32_t u32_ms);
											
void command_test_dma_driver(char * u32_src, char *u32_dst, char *u32_byteNum, 
											char* u32_ms)
{
	unsigned int iSrcAddr;
	unsigned int iDstAddr;
	unsigned int iNum;
	unsigned int iTimeout;

	iDstAddr = strtoul(u32_dst, NULL, 0);
	iSrcAddr = strtoul(u32_src, NULL, 0);
	iNum = strtoul(u32_byteNum, NULL, 0);
	iTimeout = strtoul(u32_ms, NULL, 0);

	while(1)
	{
		DMA_forDriverTransfer(iSrcAddr, iDstAddr, iNum, DMA_noneBlocked, iTimeout);
		delay_us(125);
		
		HAL_DMA_Transfer(iSrcAddr, iDstAddr, iNum, 0);
		delay_us(125);
	}
}

void command_test_dma_user(char * u32_src, char *u32_dst, char *u32_byteNum, 
											char* u32_ms)
{
	unsigned int iSrcAddr;
	unsigned int iDstAddr;
	unsigned int iNum;
	unsigned int iTimeout;

	iDstAddr = strtoul(u32_dst, NULL, 0);
	iSrcAddr = strtoul(u32_src, NULL, 0);
	iNum = strtoul(u32_byteNum, NULL, 0);
	iTimeout = strtoul(u32_ms, NULL, 0);
	
	HAL_DMA_Transfer(iSrcAddr, iDstAddr, iNum, iTimeout);
}
