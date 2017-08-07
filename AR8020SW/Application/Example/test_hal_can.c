#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debuglog.h"
#include "hal_can.h"
#include "test_hal_can.h"
#include "hal_nvic.h"

#define CAN_MAX_TIEM_MS     (2) 


static STRU_HAL_CAN_MSG s_st_canRxBuf[5];
static  uint8_t s_u8_canRxCnt = 0;

static void printCanConfigInfo(STRU_HAL_CAN_CONFIG *pst_canConfig);
static void printCanMsg(STRU_HAL_CAN_MSG *pst_canSengMsg);
static uint32_t test_CAN_RcvMsgHandler(STRU_HAL_CAN_MSG *pst_canRxBuf, uint8_t u8_canRxCnt);

static void printCanConfigInfo(STRU_HAL_CAN_CONFIG *pst_canConfig)
{
    dlog_info("e_halCanComponent:%d e_halCanBaudr:%d u32_halCanAcode:0x%x u32_halCanAmask:0x%x e_halCanFormat:%d\n",
               pst_canConfig->e_halCanComponent, 
               pst_canConfig->e_halCanBaudr,
               pst_canConfig->u32_halCanAcode,
               pst_canConfig->u32_halCanAmask,
               pst_canConfig->e_halCanFormat);
}

static void printCanMsg(STRU_HAL_CAN_MSG *pst_canSengMsg)
{
    uint8_t u8_len;

    dlog_info("e_halCanComponent:%d",
              pst_canSengMsg->e_halCanComponent);
    
    dlog_info("u32_halCanId:0x%x",
              pst_canSengMsg->u32_halCanId);
    
    dlog_info("u8_halCanDataLen:%d",
              pst_canSengMsg->u8_halCanDataLen);
   
    if(HAL_CAN_TYPE_DATA == (pst_canSengMsg->e_halCanType))   
    {   
        for(u8_len = 0; u8_len < (pst_canSengMsg->u8_halCanDataLen); u8_len++)
        {
            dlog_info("data%d:0x%x", 
                       u8_len+1, 
                       pst_canSengMsg->u8_halCanDataArray[u8_len]);
        }
    }   

    if(HAL_CAN_FORMAT_STD == (pst_canSengMsg->e_halCanFormat))    
    {   
        dlog_info(" e_halCanFormat:std");
    }   
    else if(HAL_CAN_FORMAT_EXT == (pst_canSengMsg->e_halCanFormat))   
    {   
        dlog_info(" e_halCanFormat:ext");
    }   
    else    
    {   
        dlog_info(" e_halCanFormat:error");
    }   
    
    if(HAL_CAN_TYPE_DATA == (pst_canSengMsg->e_halCanType))   
    {   
        dlog_info(" e_halCanType:data ");
    }   
    else if(HAL_CAN_TYPE_RMT == (pst_canSengMsg->e_halCanType))   
    {   
        dlog_info(" e_halCanType:rmt ");
    }   
    else    
    {   
        dlog_warning(" e_halCanType:error ");
    }
    
}

static uint32_t test_CAN_RcvMsgHandler(STRU_HAL_CAN_MSG *pst_canRxBuf, uint8_t u8_canRxCnt)
{
    
    if(s_u8_canRxCnt  >= 5)
    {
        return 1;
    }    
    if((u8_canRxCnt + s_u8_canRxCnt)  > 5)
    {
        u8_canRxCnt = 5 - s_u8_canRxCnt;
    }    


    memcpy(&s_st_canRxBuf[s_u8_canRxCnt], 
           pst_canRxBuf, 
           u8_canRxCnt*sizeof(STRU_HAL_CAN_MSG));

    s_u8_canRxCnt += u8_canRxCnt;

    return 0;
}


void command_TestCanInit(unsigned char *ch, unsigned char *br, unsigned char *acode,unsigned char *amask, unsigned char *format)
{
    STRU_HAL_CAN_CONFIG st_canConfig;

    st_canConfig.e_halCanComponent = strtoul(ch, NULL, 0);       //
    st_canConfig.e_halCanBaudr = strtoul(br, NULL, 0);           //
    st_canConfig.u32_halCanAcode = strtoul(acode, NULL, 0);      //
    st_canConfig.u32_halCanAmask = strtoul(amask, NULL, 0);      //            
    st_canConfig.e_halCanFormat = strtoul(format, NULL, 0);      //
    st_canConfig.pfun_halCanRcvMsg = test_CAN_RcvMsgHandler;
     
    HAL_CAN_Init(&st_canConfig);
    
    printCanConfigInfo(&st_canConfig);
}


void command_TestCanSetInt(unsigned char *ch, unsigned char *flag)
{
    unsigned int u32_ch = strtoul(ch, NULL, 0);
    unsigned int u32_flag = strtoul(flag, NULL, 0);

    if(0 == u32_flag)
    {
       HAL_NVIC_DisableIrq(u32_ch + HAL_NVIC_CAN_IRQ0_VECTOR_NUM);
    }
    else
    {
        HAL_NVIC_EnableIrq(u32_ch + HAL_NVIC_CAN_IRQ0_VECTOR_NUM);
    }
}


void command_TestCanTx(unsigned char *ch, unsigned char *id, unsigned char *len,unsigned char *format, unsigned char *type)
{
    uint8_t u8_ii;
    STRU_HAL_CAN_MSG st_canSendMsg;
    
    st_canSendMsg.e_halCanComponent = strtoul(ch, NULL, 0);       //
    st_canSendMsg.u32_halCanId = strtoul(id, NULL, 0);            //
    st_canSendMsg.u8_halCanDataLen = strtoul(len, NULL, 0);       //            
    st_canSendMsg.e_halCanFormat = strtoul(format, NULL, 0);          //
    st_canSendMsg.e_halCanType = strtoul(type,  NULL, 0);            //
    memset(st_canSendMsg.u8_halCanDataArray, 0x11, 8);
    HAL_CAN_Send(&st_canSendMsg, CAN_MAX_TIEM_MS);
    
    memset(st_canSendMsg.u8_halCanDataArray, 0x22, 8);
    HAL_CAN_Send(&st_canSendMsg, CAN_MAX_TIEM_MS);
    
    memset(st_canSendMsg.u8_halCanDataArray, 0x33, 8);
    HAL_CAN_Send(&st_canSendMsg, CAN_MAX_TIEM_MS);
    
    memset(st_canSendMsg.u8_halCanDataArray, 0x44, 8);
    HAL_CAN_Send(&st_canSendMsg, CAN_MAX_TIEM_MS);
    
    memset(st_canSendMsg.u8_halCanDataArray, 0x55, 8);
    HAL_CAN_Send(&st_canSendMsg, CAN_MAX_TIEM_MS);

    printCanMsg(&st_canSendMsg);

}

void command_TestCanRx(void)
{
    uint8_t u8_len;
 
    if(s_u8_canRxCnt  > 0)
    {
        for(u8_len = 0; u8_len < s_u8_canRxCnt; u8_len++)
        {
            printCanMsg(&s_st_canRxBuf[u8_len]);
        }
    }  
    s_u8_canRxCnt = 0;  
}
