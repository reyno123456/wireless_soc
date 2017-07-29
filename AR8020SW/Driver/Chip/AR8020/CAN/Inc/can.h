/*****************************************************************************
 * Copyright: 2016-2020, Artosyn. Co., Ltd.
 * File name: can.h
 * Description: can drive function declaration
 * Author: Artosyn FW
 * Version: V0.01 
 * Date: 2016.11.29
 * History: 
 * 2016.11.29 the first edition
 * *****************************************************************************/

#ifndef  __CAN_H__
#define  __CAN_H__

#define CAN_TOTAL_CHANNEL       (4)

#define CAN_AMASK_ID10_0        (0x7FF)
#define CAN_UNAMASK_ID10_0      (0x0)
#define CAN_FRAME_LEN_AMASK     (0xF)

#define CAN_AMASK_ID28_0        (0x1FFFFFFF)
#define CAN_UNAMASK_ID28_0      (0x0)


struct STRU_CAN_CONFIG;
struct STRU_CAN_MSG;

//can read msg function define.just read u8_canRxCnt frame from pst_canRxBuf
typedef uint32_t (*CAN_RcvMsgHandler)(struct STRU_CAN_MSG *pst_canRxBuf, \
                                      uint8_t u8_canRxCnt);

typedef enum
{
    CAN_COMPONENT_0 = 0,
    CAN_COMPONENT_1,
    CAN_COMPONENT_2,
    CAN_COMPONENT_3, 
} ENUM_CAN_COMPONENT;

typedef enum
{
    CAN_BAUDR_125K = 0,
    CAN_BAUDR_250K,
    CAN_BAUDR_500K,
    CAN_BAUDR_1M, 
} ENUM_CAN_BAUDR;

typedef enum
{
    CAN_FORMAT_STD= 0,
    CAN_FORMAT_EXT,
} ENUM_CAN_FORMAT;

typedef enum
{
    CAN_TYPE_DATA= 0,
    CAN_TYPE_RMT,
} ENUM_CAN_TYPE;

typedef struct STRU_CAN_CONFIG
{
    ENUM_CAN_COMPONENT     e_canComponent;   
    ENUM_CAN_BAUDR         e_canBaudr;       
    uint32_t               u32_canAcode;     /*std bit10~0 <-> ID10~0 
                                               ext bit28~0 <-> ID28~0*/
    uint32_t               u32_canAmask;     /*std bit10~0 <-> ID10~0 
                                               ext bit28~0 <-> ID28~0*/     
    ENUM_CAN_FORMAT        e_canFormat;      
    CAN_RcvMsgHandler      pfun_canRcvMsg;   
} STRU_CAN_CONFIG;

typedef struct STRU_CAN_MSG
{
    ENUM_CAN_COMPONENT     e_canComponent;       
    uint32_t               u32_canId;            
    uint8_t                u8_canDataArray[8];    
    uint8_t                u8_canDataLen;                    
    ENUM_CAN_FORMAT        e_canFormat;          
    ENUM_CAN_TYPE          e_canType;            
} STRU_CAN_MSG;

typedef enum{
        CAN_PAR_BR,     //baud rate
        CAN_PAR_ACODE,  //acceptance code
        CAN_PAR_AMASK,  //acceptance mask
        CAN_PAR_RTIE,   //receive and transmit interrupt
} ENUM_CAN_PAR_NO;

/****************************Function declaration*****************************/
/**
* @brief    can init 
* @param    e_canComponent        CAN_COMPONENT_0 ~ 3 
* @param    e_canBaudr            CAN_BAUDR_125K ~ 1M
* @param    u32_acode             std bit10~0 <-> ID10~0
*                                 ext bit28~0 <-> ID28~0
* @param    u32_amask             std bit10~0 <-> ID10~0
*                                 ext bit28~0 <-> ID28~0
* @param    u8_rtie               bit7~bit1 <---> RIE,ROIE,
*                                 RFIE,RAFIE,TPIE,TSIE,EIE 
* @param    e_canFormat           standard or extended format 
* @retval   0                     init successed.
*           other                 init failed. 
* @note     None.
*/
int32_t CAN_InitHw(ENUM_CAN_COMPONENT e_canComponent, 
                   ENUM_CAN_BAUDR e_canBaudr, 
                   uint32_t u32_acode, 
                   uint32_t u32_amask, 
                   uint8_t u8_rtie,
                   ENUM_CAN_FORMAT e_canFormat);

/**
* @brief    send can frame 
* @param    e_canComponent        CAN_COMPONENT_0 ~ 3
* @param    u32_id:               std bit10~0 <-> ID10~0
*                                 ext bit28~0 <-> ID28~0
* @param    u32_txBuf:            send data buf for data field.
* @param    u8_len:               data length for data field in byte.
* @param    e_canFormat           standard or extended format
* @param    e_canType             data or remote frame
* @retval   0                     send can frame sucessed.
*           other                 send can frame failed. 
* @note     None.
*/
int32_t CAN_Send(ENUM_CAN_COMPONENT e_canComponent, 
                 uint32_t u32_id, 
                 uint8_t *u32_txBuf, 
                 uint8_t u8_len, 
                 ENUM_CAN_FORMAT e_canFormat, 
                 ENUM_CAN_TYPE e_canType);

/**
* @brief    receive frame from can controller.
* @param    e_canComponent        CAN_COMPONENT_0 ~ 3
* @param    u32_id:               std bit10~0 <-> ID10~0
*                                 ext bit28~0 <-> ID28~0
* @param    u32_txBuf:            receive data buf for data field.
* @param    u8_len:               receive data length for data field in byte.
* @param    e_canFormat           standard or extended format
* @param    e_canType             data or remote frame
* @retval   0                     receive can frame sucessed.
*           other                 receive can frame failed. 
* @note     None.
*/
int32_t CAN_Rcv(ENUM_CAN_COMPONENT e_canComponent, 
                uint32_t *u32_id, 
                uint8_t *u8_rxBuf, 
                uint8_t *u8_len, 
                ENUM_CAN_FORMAT *pe_canFormat, 
                ENUM_CAN_TYPE *pe_canType);

/**
* @brief  can interrupt servive function.just handled data reception.  
* @param  u32_vectorNum           Interrupt number.
* @retval None.
* @note   None.
*/
void CAN_IntrSrvc(uint32_t u32_vectorNum);

/**
* @brief  register user function for can recevie data.called in interrupt
*         service function.
* @param  u8_canCh           can channel, 0 ~ 3.
* @param  userHandle         user function for can recevie data.
* @retval 
*         -1                  register user function failed.
*         0                   register user function sucessed.
* @note   None.
*/
int32_t CAN_RegisterUserRxHandler(uint8_t u8_canCh, CAN_RcvMsgHandler userHandler);

/**
* @brief  unregister user function for can recevie data.
* @param  u8_canCh            can channel, 0 ~ 3.
* @retval 
*         -1                  unregister user function failed.
*         0                   unregister user function sucessed.
* @note   None.
*/
int32_t CAN_UnRegisterUserRxHandler(uint8_t u8_canCh);

/**
* @brief  get can control tx busy status.
* @param  e_canComponent        CAN_COMPONENT_0 ~ 3
* @retval 
*         0                     can control idle
*         1                     can control busy
* @note   None.
*/
int32_t CAN_GetTxBusyStatus(ENUM_CAN_COMPONENT e_canComponent);




#endif
