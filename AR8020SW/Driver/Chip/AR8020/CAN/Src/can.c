/*****************************************************************************
 * Copyright: 2016-2020, Artosyn. Co., Ltd.
 * File name: can.c
 * Description: can drive function implementation 
 * Author: Artosyn FW
 * Version: V0.01 
 * Date: 2016.11.29
 * History: 
 * 2016.11.29 the first edition.
 *     finished standard data frame,standard remote frame,
 *     extended data frame,extended remote frame receive and send.
 * *****************************************************************************/

#include "../Inc/canDef.h"
#include "../Inc/can.h"


static CAN_RcvMsgHandler s_pfun_canUserHandlerTbl[CAN_TOTAL_CHANNEL] =  
       { NULL, NULL, NULL, NULL };

/*******************Function declaration**************************/

static STRU_CAN_TYPE * CAN_GetBaseAddrByCh(ENUM_CAN_COMPONENT e_canComponent);

static int32_t CAN_SendArr(uint8_t u8_ch, uint8_t *u32_txBuf);

static int32_t CAN_RcvArr(uint8_t u8_ch, uint8_t *u8_rxBuf);

static int32_t CAN_Start(uint8_t u8_ch, uint32_t u32_rxLen, uint32_t u32_txLen);

static int32_t CAN_Close(uint8_t u8_ch);


/*******************Function implementation**************************/

/**
* @brief    get can base addr  
* @param    u8_ch  can0~can3
* @retval   can base addr
* @note   
*/
static STRU_CAN_TYPE * CAN_GetBaseAddrByCh(ENUM_CAN_COMPONENT e_canComponent)
{
    STRU_CAN_TYPE *pst_canReg;

    switch(e_canComponent)
    {
        case CAN_COMPONENT_0:
        {
            pst_canReg = (STRU_CAN_TYPE *)BASE_ADDR_CAN0; 
            break;
        }
        case CAN_COMPONENT_1:
        {
            pst_canReg = (STRU_CAN_TYPE *)BASE_ADDR_CAN1; 
            break;
        }
        case CAN_COMPONENT_2:
        {
            pst_canReg = (STRU_CAN_TYPE *)BASE_ADDR_CAN2; 
            break;
        }
        case CAN_COMPONENT_3:
        {
            pst_canReg = (STRU_CAN_TYPE *)BASE_ADDR_CAN3; 
            break;
        }
        default:
        {
            pst_canReg = (STRU_CAN_TYPE *)NULL;
            break;
        }
    } 

    return pst_canReg;
} 

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
                   ENUM_CAN_FORMAT e_canFormat)
{
    volatile STRU_CAN_TYPE *pst_canReg;
    unsigned int u32_tmpData;

    pst_canReg = CAN_GetBaseAddrByCh(e_canComponent); 

    pst_canReg->u32_reg3 |= (1<<7); // CFG_STAT-->RESET=1

    //clear S_Seg_1,S_Seg_2,S_SJW
    u32_tmpData = (pst_canReg->u32_reg5) & (~(0x3F | (0x1F<<8) | (0xF<<16)));
    // set S_Seg_1=12,S_Seg_2=10,S_SJW=2, BT=((S_Seg_1+2)+(S_Seg_2+1))*TQ
    u32_tmpData |= ((0xC) | (0xA<<8) | (0x2<<16));     
    pst_canReg->u32_reg5 = u32_tmpData;

    pst_canReg->u32_reg6 &= (~0xFF);    //clear S_PRESC
    switch(e_canBaudr)
    {
        case CAN_BAUDR_125K:
        {
            pst_canReg->u32_reg6 |= 0x1F;          // S_PRESC = 31 
            break;
        }
        case CAN_BAUDR_250K:
        {
            pst_canReg->u32_reg6 |= 0xF;          // S_PRESC = 15 
            break;
        }
        case CAN_BAUDR_500K:
        {
            pst_canReg->u32_reg6 |= 0x7;          // S_PRESC = 7 
            break;
        }
        case CAN_BAUDR_1M:
        {
            pst_canReg->u32_reg6 |= 0x3;          // S_PRESC = 3 
            break;
        }
        default:
        {
            pst_canReg->u32_reg6 |= 0x7;          // S_PRESC = 7 
            dlog_warning("baud rate error,set default baud rate 500K.\n");
            break;
        }
    }
    // ACFCTRL-->SELMASK=0, register ACF_x point to acceptance code
    pst_canReg->u32_reg8 &= ~(1<<5); 
    if(CAN_FORMAT_STD == e_canFormat)
    {       
        pst_canReg->u32_reg9 &= ~(CAN_AMASK_ID10_0);
        pst_canReg->u32_reg9 |= (u32_acode & CAN_AMASK_ID10_0);
    }   
    else 
    {       
        pst_canReg->u32_reg9 &= ~(CAN_AMASK_ID28_0);
        pst_canReg->u32_reg9 |= (u32_acode & CAN_AMASK_ID28_0);
    }   
    // ACFCTRL-->SELMASK=1, register ACF_x point to acceptance mask
    pst_canReg->u32_reg8 |=(1<<5 | 0x1<<16); 
    if(CAN_FORMAT_STD == e_canFormat)
    {   
        pst_canReg->u32_reg9 &= ~(CAN_AMASK_ID10_0);  
        pst_canReg->u32_reg9 |= (u32_amask & CAN_AMASK_ID10_0);
    }
    else
    {   
        pst_canReg->u32_reg9 &= ~(CAN_AMASK_ID28_0);    
        pst_canReg->u32_reg9 |= (u32_amask & CAN_AMASK_ID28_0);
    }
    
    //clear ACF_3 AIDEE,acceptance filter accepts both standard or extended frames
    pst_canReg->u32_reg9 &= ~(1 << 30); 
    
    pst_canReg->u32_reg4 &= ~(0xFE);//clear RTIE register
    pst_canReg->u32_reg4 |= (u8_rtie & 0xFE);
    
    pst_canReg->u32_reg3 &= ~(1<<7);    // CFG_STAT-->RESET=0
    
    return 0;
}

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
                 ENUM_CAN_TYPE e_canType)
{
    uint8_t u8_i;
    uint8_t u8_dlc;
    volatile STRU_CAN_TYPE *pst_canReg;

    pst_canReg = CAN_GetBaseAddrByCh(e_canComponent);

    //EDL=0 CAN2.0 frame
    pst_canReg->u32_txBuf[1] &= (~CAN_TBUF_EDL);
    
    if(CAN_FORMAT_STD == e_canFormat)// std
    {
        //IDE=0 standard u8_format
        pst_canReg->u32_txBuf[1] &= (~CAN_TBUF_IDE);
        
        // clear ID[10:0]
        pst_canReg->u32_txBuf[0] &= (~CAN_AMASK_ID10_0); 
        // set ID[10:0]
        pst_canReg->u32_txBuf[0] |= (u32_id & CAN_AMASK_ID10_0);
    }   
    else //ext
    {   
        //IDE=1 extended u8_format
        pst_canReg->u32_txBuf[1] |= (CAN_TBUF_IDE);
        
        // clear ID[28:0]
        pst_canReg->u32_txBuf[0] &= (~CAN_AMASK_ID28_0); 
        // set ID[28:0]
        pst_canReg->u32_txBuf[0] |= (u32_id & CAN_AMASK_ID28_0); 
    }
    
    // clear DLC, the number of payload bytes in a frame, valid max=8 for CAN2.0
    pst_canReg->u32_txBuf[1] &= (~CAN_FRAME_LEN_AMASK); 
    u8_dlc = (u8_len<=8) ? u8_len : 8;
    pst_canReg->u32_txBuf[1] |= u8_dlc;         
    
    if(CAN_TYPE_DATA == e_canType)//data frame
    {
        //RTR=0,data frame
        pst_canReg->u32_txBuf[1] &= (~CAN_TBUF_RTR);

        for(u8_i=0; u8_i<(u8_dlc+3)/4; u8_i++)
        {
            pst_canReg->u32_txBuf[2+u8_i] =*(uint32_t*)(u32_txBuf+u8_i*4);
        }
    }
    else//remote frame
    {
        //RTR=1,remote frame
        pst_canReg->u32_txBuf[1] |= (CAN_TBUF_RTR);
    }

    pst_canReg->u32_reg3 |= (1<<12);    // TCMD-->TPE = 1,Transmit Primary Enable

    return 0;
}

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
                ENUM_CAN_TYPE *pe_canType)
{
    uint8_t u8_tmpLen;
    uint32_t u32_data;
    volatile STRU_CAN_TYPE *pst_canReg;

    pst_canReg = CAN_GetBaseAddrByCh(e_canComponent); 
   
    //RSTAT(1:0) = 00b, receive buffer empty
    if(0 == ((pst_canReg->u32_reg3) & (3<<24)))
    {
        return -1;  //rx buffer is empty
    }
 
    //pst_canReg->u32_reg4 |= (0xf<<12);  // clear the receiver irq
    u32_data = pst_canReg->u32_rxBuf[1];

    if(0 == (u32_data & CAN_TBUF_IDE))//std frame
    {
        *u32_id = pst_canReg->u32_rxBuf[0] & CAN_AMASK_ID10_0;//u32_id
        *pe_canFormat = CAN_FORMAT_STD;//pe_canFormat
    }
    else//ext frame
    {
        *u32_id = pst_canReg->u32_rxBuf[0] & CAN_AMASK_ID28_0;//u32_id
        *pe_canFormat = CAN_FORMAT_EXT;//pe_canFormat
    }
    
    if(0 == (u32_data & CAN_TBUF_RTR))//data frame
    {
        *pe_canType = CAN_TYPE_DATA;  
    }
    else//remote frame
    {
        *pe_canType = CAN_TYPE_RMT;   
    }
    
    u8_tmpLen = u32_data & CAN_FRAME_LEN_AMASK;
    u8_tmpLen = (u8_tmpLen <= 8) ? u8_tmpLen : 8;
    
    //remote frame has no data
    if( (CAN_TYPE_RMT == (*pe_canType)) || (0x00 == u8_tmpLen) )
    {
        *u8_len = 0;   
    }       
    else
    {
        *u8_len = u8_tmpLen;
        *(uint32_t *)(u8_rxBuf) = pst_canReg->u32_rxBuf[2];  //data1~data4
        *(uint32_t *)(u8_rxBuf + 4) = pst_canReg->u32_rxBuf[3];//data5~data8
    }


    pst_canReg->u32_reg3 |= (1<<28);       // RREL, release the RBUF

    return 0;
}

/**
* @brief    send std data frame
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @param    u32_txBuf:   send data buffer
* @retval 
* @note   
*/
static int32_t CAN_SendArr(uint8_t u8_ch, uint8_t *u32_txBuf)
{
    volatile STRU_CAN_TYPE *pst_canReg;

    pst_canReg = CAN_GetBaseAddrByCh(u8_ch);

    pst_canReg->u32_txBuf[0] = *(uint32_t*)(u32_txBuf + 0);   //u32_id
    //EDL=0 CAN2.0 frame,BRS=0 nominal/slow bit rate    
    pst_canReg->u32_txBuf[1] = 0; 
    pst_canReg->u32_txBuf[1] |= *(uint32_t*)(u32_txBuf + 4);  //IDE RTR DLC 
    pst_canReg->u32_txBuf[2] = *(uint32_t*)(u32_txBuf + 8);
    pst_canReg->u32_txBuf[3] = *(uint32_t*)(u32_txBuf + 12);
    
    pst_canReg->u32_reg3 |= (1<<12);    // TCMD-->TPE = 1,Transmit Primary Enable

    return 0;
 
}

/**
* @brief    receive can frame 
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @param    u8_rxBuf:   
* @retval 
* @note   
*/
static int32_t CAN_RcvArr(uint8_t u8_ch, uint8_t *u8_rxBuf)
{
    volatile STRU_CAN_TYPE *pst_canReg;

    pst_canReg = CAN_GetBaseAddrByCh(u8_ch);

    *(uint32_t*)(u8_rxBuf + 0) = pst_canReg->u32_rxBuf[0];   //ID
    *(uint32_t*)(u8_rxBuf + 4) = pst_canReg->u32_rxBuf[1];   //DLC
    *(uint32_t*)(u8_rxBuf + 8) = pst_canReg->u32_rxBuf[2];   //data1~data4
    *(uint32_t*)(u8_rxBuf + 12) = pst_canReg->u32_rxBuf[3];  //data5~data8

    pst_canReg->u32_reg3 |= (1<<28);       // RREL, release the RBUF
    
    return 0;
}

/**
* @brief    create can rx buf,tx buf ,mutex 
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @param    u32_rxLen  can receive buffer size
* @param    u32_txLen  can send buffer size
* @retval 
* @note   
*/
static int32_t CAN_Start(uint8_t u8_ch, uint32_t u32_rxLen, uint32_t u32_txLen)
{
    int32_t s32_result = 0;
    
    //create xMutex for can operation   
    /*g_canMutex = xSemaphoreCreateMutex(); 
    
    if(NULL == g_canMutex)
    {
        s32_result = -1;
    }*/

    //check buffer length
    if((u32_rxLen < sizeof(STRU_CAN_MSG)) || (u32_txLen < sizeof(STRU_CAN_MSG)))
    {
        s32_result = -1;
    }

    //create can receive buffer
    
    //create can send buffer

    return 0;
}

/**
* @brief    delete can rx buf,tx buf ,mutex 
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @retval 
* @note   
*/
static int32_t CAN_Close(uint8_t u8_ch)
{
    //delete can receive buffer
    
    //delete can send buffer
}

/**
* @brief  can interrupt servive function.just handled data reception.  
* @param  u32_vectorNum           Interrupt number.
* @retval None.
* @note   None.
*/
void CAN_IntrSrvc(uint32_t u32_vectorNum)
{
    volatile STRU_CAN_TYPE *pst_canReg;
    STRU_CAN_MSG st_canRxBuf[5];
    uint8_t u8_canRxLen = 0;
    uint8_t u8_canCh;
    
    u8_canCh = u32_vectorNum - CAN_IRQ0_VECTOR_NUM;
    
    pst_canReg = CAN_GetBaseAddrByCh(u8_canCh);
    
     //rx interrupt
    if(0x00 != ((pst_canReg->u32_reg4) & 0x8000))   
    {
        //clear Receive Interrupt flag
        pst_canReg->u32_reg4 |= 0x8000; 

        while(0 != ((pst_canReg->u32_reg3) & (3<<24)))
        {
            st_canRxBuf[u8_canRxLen].e_canComponent = (ENUM_CAN_COMPONENT)u8_canCh;

            CAN_Rcv((st_canRxBuf[u8_canRxLen].e_canComponent), 
                    &(st_canRxBuf[u8_canRxLen].u32_canId), 
                    &(st_canRxBuf[u8_canRxLen].u8_canDataArray[0]), 
                    &(st_canRxBuf[u8_canRxLen].u8_canDataLen), 
                    &(st_canRxBuf[u8_canRxLen].e_canFormat), 
                    &(st_canRxBuf[u8_canRxLen].e_canType));

            u8_canRxLen += 1;
            if(u8_canRxLen >= 5)
            {
                break;
            }
        }
 
        if(u8_canRxLen > 0)  // call user function
        {
            if(NULL != (s_pfun_canUserHandlerTbl[u8_canCh]))
            {
                (s_pfun_canUserHandlerTbl[u8_canCh])(&st_canRxBuf[0], u8_canRxLen);
            }
        } 
    }
    //tx Primary or Secondary interrupt
    else if(0x00 != ((pst_canReg->u32_reg4) & 0x0C00))  
    {
        //clear Transmission Primary/Secondary Interrupt flag  
        pst_canReg->u32_reg4 |= 0x0C00; 
    }
    else    //other
    {
    
    }   
}

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
int32_t CAN_RegisterUserRxHandler(uint8_t u8_canCh, CAN_RcvMsgHandler userHandler)
{
    if(u8_canCh >= CAN_TOTAL_CHANNEL)
    {
        return -1;
    }

    s_pfun_canUserHandlerTbl[u8_canCh] = userHandler;

    return 0;
}

/**
* @brief  unregister user function for can recevie data.
* @param  u8_canCh            can channel, 0 ~ 3.
* @retval 
*         -1                  unregister user function failed.
*         0                   unregister user function sucessed.
* @note   None.
*/
int32_t CAN_UnRegisterUserRxHandler(uint8_t u8_canCh)
{
    if(u8_canCh >= CAN_TOTAL_CHANNEL)
    {
        return -1;
    }

    s_pfun_canUserHandlerTbl[u8_canCh] = NULL;

    return 0;
}

/**
* @brief  get can control tx busy status.
* @param  e_canComponent        CAN_COMPONENT_0 ~ 3
* @retval 
*         0                     can control idle
*         1                     can control busy
* @note   None.
*/
int32_t CAN_GetTxBusyStatus(ENUM_CAN_COMPONENT e_canComponent)
{
    volatile STRU_CAN_TYPE *pst_canReg;
    
    pst_canReg = CAN_GetBaseAddrByCh(e_canComponent); 

    if ((pst_canReg->u32_reg3) & CFG_STAT_TACTIVE)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

