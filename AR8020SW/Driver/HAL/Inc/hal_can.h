/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_can.h
Description: The external HAL APIs to use the CAN controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/29
History: 
        0.0.1    2016/12/29    The initial version of hal_can.h
*****************************************************************************/
#ifndef __HAL_CAN_H__
#define __HAL_CAN_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "hal_ret_type.h"


struct STRU_HAL_CAN_CONFIG;
struct STRU_HAL_CAN_MSG;

//can read msg function define.just read u8_canRxCnt frame from pst_canRxBuf
typedef uint32_t (*HAL_CAN_RcvMsgHandler)(struct STRU_HAL_CAN_MSG *pst_canRxBuf, \
                                          uint8_t u8_canRxCnt);


typedef enum
{
    HAL_CAN_COMPONENT_0 = 0,
    HAL_CAN_COMPONENT_1,
    HAL_CAN_COMPONENT_2,
    HAL_CAN_COMPONENT_3, 
} ENUM_HAL_CAN_COMPONENT;

typedef enum
{
    HAL_CAN_BAUDR_125K = 0,
    HAL_CAN_BAUDR_250K,
    HAL_CAN_BAUDR_500K,
    HAL_CAN_BAUDR_1M, 
} ENUM_HAL_CAN_BAUDR;

typedef enum
{
    HAL_CAN_FORMAT_STD= 0,
    HAL_CAN_FORMAT_EXT,
} ENUM_HAL_CAN_FORMAT;

typedef enum
{
    HAL_CAN_TYPE_DATA= 0,
    HAL_CAN_TYPE_RMT,
} ENUM_HAL_CAN_TYPE;

typedef struct STRU_HAL_CAN_CONFIG
{
    ENUM_HAL_CAN_COMPONENT     e_halCanComponent;   
    ENUM_HAL_CAN_BAUDR         e_halCanBaudr;       
    uint32_t                   u32_halCanAcode;     /*std bit10~0 <-> ID10~0 
                                                      ext bit28~0 <-> ID28~0*/
    uint32_t                   u32_halCanAmask;     /*std bit10~0 <-> ID10~0 
                                                      ext bit28~0 <-> ID28~0*/     
    ENUM_HAL_CAN_FORMAT        e_halCanFormat;      
    HAL_CAN_RcvMsgHandler      pfun_halCanRcvMsg;   
} STRU_HAL_CAN_CONFIG;

typedef struct STRU_HAL_CAN_MSG
{
    ENUM_HAL_CAN_COMPONENT     e_halCanComponent;       
    uint32_t                   u32_halCanId;            
    uint8_t                    u8_halCanDataArray[8];   
    uint8_t                    u8_halCanDataLen;                   
    ENUM_HAL_CAN_FORMAT        e_halCanFormat;         
    ENUM_HAL_CAN_TYPE          e_halCanType;           
} STRU_HAL_CAN_MSG;


/**
* @brief   can controller initialization. 
* @param   st_halCanConfig        init need info.  
*          u32_timeOut            timeout threshold, unit:ms                 
* @retval HAL_OK                  init can controller successed. 
*         HAL_CAN_ERR_INIT        init can controller failed.
*         HAL_CAN_ERR_COMPONENTi  can channel error.init failed.
* @note   None. 
*/
HAL_RET_T HAL_CAN_Init(STRU_HAL_CAN_CONFIG *st_halCanConfig);

/**
* @brief  send can frame.include standard data frame,standard remote frame,
*         extended data frame, extended remote frame
* @param  st_halCanMsg       pointer to can message for send. 
*                           
* @retval HAL_OK                can message send successed. 
*         HAL_CAN_ERR_SEND_MSG  can message send failed.
* @note   None. 
*/
HAL_RET_T HAL_CAN_Send(STRU_HAL_CAN_MSG *st_halCanMsg, uint32_t u32_timeOut);

#ifdef __cplusplus
}
#endif

#endif
