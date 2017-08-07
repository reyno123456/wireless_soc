/* ----------------------------------------------------------------------
 * $Date:        2016.12.10
 * $Revision:    V0.01
 *
 * Project:      
 * Title:        
 *
 * Version 0.01
 *       
 *  
 *----------------------------------------------------------------------------
 *
 * 
 *---------------------------------------------------------------------------*/

 /**
  ******************************************************************************
  * @file    BB_SPI_Operate.c
  * @author  
  * @date    
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  *
  ******************************************************************************
  */

#include "bb_spi_com.h"
#include "bb_ctrl.h"
#include <string.h>
#include "bb_ctrl_internal.h"


static int BB_SPI_DataCheck(BB_SPI_Opr *p_stCheck,BB_SPI_Opr *p_stBuff);
//static BB_SPI_Opr g_stBuff;





/**
* @brief	  
* @param  	
* @retval 	
* @note   
*/
int32_t BB_SPI_Write(BB_SPI_Opr *p_stWrite)
{
	uint32_t u32_len;
	uint32_t u32_i;
	uint8_t u8_checkSum = 0;
	uint8_t *p_u8Addr;
	//uint8_t *p_u8Addr2;

	p_stWrite->u8_arCnt[0] += 1;
	p_stWrite->u8_arCnt[1] = ~(p_stWrite->u8_arCnt[0]);
	
	u32_len = sizeof(BB_SPI_Opr);
	p_u8Addr = (uint8_t *)p_stWrite;
	
	//calculate checkSum,jump over cnt,~cnt,checkSum,total 3 bytes
	for(u32_i=3; u32_i<u32_len; u32_i++)
	{
		u8_checkSum += *(p_u8Addr + u32_i);
	}
	p_stWrite->u8_checkSum = u8_checkSum;

	//p_u8Addr2 = (uint8_t *)(&g_stBuff);
	//write to spi
	for(u32_i=0; u32_i<u32_len; u32_i++)
	{
		/*if(p_u8Addr2[u32_i] != p_u8Addr[u32_i])
		{
		
		}
		else
		{
		
		}*/
    	BB_WriteReg(BB_SPI_OPR_BASE_PAGE, 
						BB_SPI_OPR_BASE_ADDR + u32_i, 
						*(p_u8Addr + u32_i));
		//dlog_info("%d",*(p_u8Addr + u32_i));
	}
	
	
	/*dlog_info("cnt:%d checksum:%d data:%d",
					p_stWrite->u8_arCnt[0],
					p_stWrite->u8_checkSum,
					p_u8Addr[4]);*/
		
	return 0;
} 

/**
* @brief	  
* @param  	
* @retval 	
* @note   
*/
int32_t BB_SPI_Read(BB_SPI_Opr *p_stRead)
{
	uint8_t *p_u8Addr;
	uint32_t u32_len;
	int32_t s32_result = -1;
	BB_SPI_Opr stBuffer;
	
	if(0 == BB_SPI_DataCheck(p_stRead,&stBuffer))
	{
		u32_len = sizeof(BB_SPI_Opr);
		memcpy((uint8_t *)p_stRead, (uint8_t *)(&stBuffer), u32_len);	
		s32_result = 0;
	}
	else
	{
		s32_result = -1;
	}

	return s32_result;
}

/**
* @brief	  
* @param  	
* @retval 	
* @note   
*/
static int BB_SPI_DataCheck(BB_SPI_Opr *p_stCheck,BB_SPI_Opr *p_stBuff)
{
	uint32_t u32_len;
	uint32_t u32_i;
	uint8_t u8_checkSum = 0;
	uint8_t *p_u8Addr;
	uint8_t u8_cnt;
	int32_t s32_result = -1;

	u8_cnt = BB_ReadReg(BB_SPI_OPR_BASE_PAGE, BB_SPI_OPR_BASE_ADDR); 
	//u8_cnt[1] = BB_ReadReg(BB_SPI_OPR_BASE_PAGE, BB_SPI_OPR_BASE_ADDR + 1);
	
	//dlog_info("u8_cnt[0]:%d u8_cnt[1]:%d p_stCheck->u8_arCnt[0]:%d",u8_cnt[0],u8_cnt[1],p_stCheck->u8_arCnt[0]);
	/*if( (u8_cnt[1] == (~u8_cnt[0])) && 
		(u8_cnt[0] != (p_stCheck->u8_arCnt[0])) )*/
	if(u8_cnt != (p_stCheck->u8_arCnt[0]))
	{	
		u32_len = sizeof(BB_SPI_Opr);
		p_u8Addr = (uint8_t *)(p_stBuff);

		for(u32_i=0; u32_i<u32_len; u32_i++)
		{
			*(p_u8Addr + u32_i) = BB_ReadReg(BB_SPI_OPR_BASE_PAGE, 
											BB_SPI_OPR_BASE_ADDR + u32_i);
			
			//dlog_info("%d",*(p_u8Addr + u32_i));
		}
		
		//calculate checkSum,jump over cnt,~cnt,checkSum,total 3 bytes
		for(u32_i=3; u32_i<u32_len; u32_i++)
		{
			u8_checkSum += *(p_u8Addr + u32_i);
		}

		if(u8_checkSum == (p_stBuff->u8_checkSum))
		{
			s32_result = 0;
		}
		else
		{
			s32_result = -1;
			dlog_error("check error");
		}
	}
	else
	{
		s32_result = -1;
		dlog_info("old data");
	}

	return s32_result;
}




