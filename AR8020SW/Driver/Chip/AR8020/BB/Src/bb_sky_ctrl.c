#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "reg_rw.h"
#include "timer.h"
#include "interrupt.h"
#include "systicks.h"
#include "bb_sky_ctrl.h"
#include "bb_regs.h"
#include "debuglog.h"
#include "sys_event.h"
#include "bb_uart_com.h"
#include "bb_ctrl_internal.h"
#include "bb_customerctx.h"
#include "rf_if.h"


#define SKY_RC_ERR(status)          (0x80 == status)
#define SKY_CRC_OK(status)          ((status & 0x02) ? 1 : 0)
#define SKY_ID_CRC_MATCH(status)    ((status & 0x03) == 0x03)

typedef struct
{
    uint8_t  u8_flagOffset;  //offset have got
    uint8_t  u8_softEnable;

    int64_t  ll_offset;
    int      basefrq;
    int      baseOffset;
}STRU_SKY_FREQ_OFFSET;


typedef struct _STRU_SKY_STATUS
{
    init_timer_st        sky_timer2_6;
    init_timer_st        sky_timer2_7;

    STRU_RC_FRQ_MASK     s_st_rcFrqMask;
    STRU_SKY_FREQ_OFFSET stru_skyfrqOffst;
    enum EN_AGC_MODE     en_agcmode;

    uint8_t              u8_rcStatus;

    uint8_t              cur_groundRcid[5];           //the current ground rc id in searching following mode
    uint8_t              flowing_rcId[5];       //the flowing ground rc id in searching following mode
    uint16_t             u16_rcUnlockCntBandSwitch;

    uint16_t             u16_sky_snr[8];
    uint8_t              u8_snr_idx;

    uint8_t              agc_value1[50];
    uint8_t              agc_value2[50];

    uint8_t              agc_idx;

    uint8_t              flag_groundRequestDisconnect;   //0x83[5]==1 means disconnect, ground rc id 0x84 ~ 0x87 match
          
    uint8_t              flag_errorConnect;             //ground in Lock mode, ground rc id [0x84 ~ 0x87] not match sky rc id

    uint8_t              flag_spiCmdValid;              //the spi command from ground is valid

    UNION_grdRcIdSearchStatus  u_rcsync;
    uint8_t              u8_syncRcId[5];

    uint8_t              u8_requestRcSearch;        //u8_requestRcSearch: request to search rc id

    uint32_t            u32_cyclecnt;
    
    uint8_t             flag_followGroundInSearching;
}STRU_SKY_STATUS;

static STRU_SKY_STATUS stru_skystatus;

static void sky_handle_one_cmd(STRU_WIRELESS_CONFIG_CHANGE* pcmd);
static void sky_handle_all_cmds(void);

static void sky_calc_dist(void);

static uint16_t sky_get_rc_snr( void );
static uint16_t sky_get_snr_average(void);

static void sky_initRcFrqMaskFunc(void);
static void sky_rcFrqStatusStatistics(void);

static void sky_doRfBandChange(uint8_t u8_lockStatus);
static void sky_backTo2GCheck(uint8_t u8_lockStatus);

static void sky_clearFrqOffset(void);

static void sky_Timer2_6_Init(void);
static void Sky_TIM2_6_IRQHandler(uint32_t u32_vectorNum);

static void sky_Timer2_7_Init(void);
static void Sky_TIM2_7_IRQHandler(uint32_t u32_vectorNum);
static void wimax_vsoc_rx_isr(uint32_t u32_vectorNum);

static void sky_getSignalStatus(void);
static void sky_rc_hopfreq(void);
static void sky_agcGainToggle(void);
static void sky_soft_reset(void);

static void sky_getGroudRcId(uint8_t* pu8_rcId);

static void BB_skyPlot(void);
static uint8_t get_rc_status(void);

static void sky_handle_all_spi_cmds(void);

static void sky_setRcId(uint8_t *idptr);
static void sky_set_McsByIndex(ENUM_CH_BW bw, uint8_t idx);

static void sky_ChgRcRate(uint8_t rate);


void BB_SKY_start(void)
{
    context.rcHopMode = AUTO;
    context.cur_IT_ch = 0;

    sky_Timer2_6_Init();
    sky_Timer2_7_Init();

    reg_IrqHandle(BB_RX_ENABLE_VECTOR_NUM, wimax_vsoc_rx_isr, NULL);
    INTR_NVIC_SetIRQPriority(BB_RX_ENABLE_VECTOR_NUM, INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_BB_RX,0));

    context.qam_ldpc = context.u8_bbStartMcs;
    sky_set_McsByIndex(context.e_bandwidth, context.qam_ldpc);

    context.dev_state = CHECK_LOCK;

    BB_UARTComInit(NULL); 
    sky_initRcFrqMaskFunc();

    sky_setRcId((uint8_t *)context.rcid);

    TIM_StartTimer(stru_skystatus.sky_timer2_7); //enabole TIM7 timeout
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);
}


/*
 * 1: lock status 
 * 0: unlock
*/
static uint8_t sky_checkRcLock(uint8_t u8_lockStatus)
{
    if (stru_skystatus.flag_followGroundInSearching)
    {
        if ( SKY_CRC_OK(u8_lockStatus) )  //crc ok
        {
            if (0 == memcmp((void *)stru_skystatus.flowing_rcId, (void *)stru_skystatus.cur_groundRcid, RC_ID_SIZE)) //the right rc id
            {
                return 1;
            }
        }
    }
    else
    {
        return SKY_ID_CRC_MATCH(u8_lockStatus);
    }

    return 0;
}

static void sky_checkRcCrcUnlockTimeOut(uint8_t u8_lockStatus)
{
	uint8_t max_ch_size = ((context.e_curBand == RF_2G) || (context.e_curBand == RF_600M)) ? (MAX_2G_RC_FRQ_SIZE):(MAX_5G_RC_FRQ_SIZE);
    static uint16_t rc_unlock_cnt = 0;

    if (SKY_CRC_OK(u8_lockStatus))
    {
        rc_unlock_cnt = 0;
    }
    else
    {
        rc_unlock_cnt ++;
    }

    if (rc_unlock_cnt > (max_ch_size + 1))
    {
        rc_unlock_cnt = 0;
        if(context.rcHopMode == AUTO)
        {
            sky_rc_hopfreq();
        }

        sky_agcGainToggle();
        sky_soft_reset();
    }
}


static void sky_agcGainToggle(void)
{
    static int loop = 0;
    if(loop++ > 50)
    {
        loop = 0; 
    }

    /*if(FAR_AGC == stru_skystatus.en_agcmode)
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_NEAR);
        BB_WriteReg(PAGE0, AGC_3, AAGC_GAIN_NEAR);

        BB_WriteReg(PAGE0, AGC_5G_GAIN1, AAGC_GAIN_NEAR); //add 5G agc setting
        BB_WriteReg(PAGE0, AGC_5G_GAIN2, AAGC_GAIN_NEAR);
        
        BB_WriteReg(PAGE1, 0x03, 0x40);
        BB_WriteReg(PAGE0, 0xBC, 0x40);
        stru_skystatus.en_agcmode = NEAR_AGC;
    }
    else*/
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_FAR);
        BB_WriteReg(PAGE0, AGC_3, AAGC_GAIN_FAR);

        BB_WriteReg(PAGE0, AGC_5G_GAIN1, AAGC_GAIN_FAR); //add 5G agc setting
        BB_WriteReg(PAGE0, AGC_5G_GAIN2, AAGC_GAIN_FAR);

        BB_WriteReg(PAGE1, 0x03, 0x28);
        BB_WriteReg(PAGE0, 0xBC, 0xC0);

        stru_skystatus.en_agcmode = FAR_AGC;
    }
}


void sky_adjustAgcGain(void)
{
    uint8_t i = 0;
    uint16_t sum_1 = 0, sum_2 = 0;
    uint8_t  aver1, aver2;
    uint8_t  num = sizeof(stru_skystatus.agc_value1);

    uint8_t  *pu8_ptr1 = stru_skystatus.agc_value1;
    uint8_t  *pu8_ptr2 = stru_skystatus.agc_value2;
    
    for( i = 0 ; i < num ; i++)
    {
        sum_1 += *(pu8_ptr1 + i);
        sum_2 += *(pu8_ptr2 + i);
    }

    aver1 = sum_1 / num;
    aver2 = sum_2 / num;

    {
        static int count1 = 0;
        if ( count1 ++ > 100)
        {
            count1 = 0;
            STRU_skyStatusMsg stru_skyAgcStatus = {
                .pid             = SKY_AGC_STATUS,
                .par.skyAgc.u8_skyagc1 = aver1,
                .par.skyAgc.u8_skyagc2 = aver2,
            };

            //BB_UARTComSendMsg( BB_UART_COM_SESSION_0, (uint8_t *)&stru_skyAgcStatus, sizeof(STRU_skyStatusMsg));
            //dlog_info("aver: %d %d %x %x", sum_1, sum_2, rx1_gain, rx2_gain);
        }
    }

#if 0
    if((aver1 >= POWER_GATE)&&(aver2 >= POWER_GATE) && stru_skystatus.en_agcmode != FAR_AGC)
    {
        sky_agcGainToggle();
    }
    else if( ((aver1 < POWER_GATE)&&(aver2 < POWER_GATE)) \
        && (aver1 > 0x00) && (aver2 >0x00) \
        && stru_skystatus.en_agcmode != NEAR_AGC)
    {
        sky_agcGainToggle();
    }
#endif
}


void sky_notify_encoder_brc(uint8_t u8_ch, uint8_t br)
{
    STRU_SysEvent_BB_ModulationChange event;
    event.BB_MAX_support_br = br;
    if (0 == u8_ch)
    {
        event.u8_bbCh = 0;
    }
    else
    {
        event.u8_bbCh = 1;
    }
    SYS_EVENT_Notify(SYS_EVENT_ID_BB_SUPPORT_BR_CHANGE, (void*)&event);	
    dlog_info("ch%d brc =%d \n", u8_ch, br);        
}

void sky_set_McsByIndex(ENUM_CH_BW bw, uint8_t idx)
{
    uint8_t value;
    uint8_t mcs_idx_reg0x0f_map[] = 
    {
        0x4f,
        0x8f,
        0x5f,
        0x3f,
        0x3f,
        0x2f
    };
    
    uint8_t map_idx_to_mode_10m[] = 
    {
        ((MOD_BPSK<<6)  | (BW_10M <<3)  | LDPC_1_2), //
        ((MOD_BPSK<<6)  | (BW_10M <<3)  | LDPC_1_2),
        ((MOD_4QAM<<6)  | (BW_10M <<3)  | LDPC_1_2),
        //((MOD_4QAM<<6)  | (BW_10M <<3)  | LDPC_2_3),
        ((MOD_16QAM<<6) | (BW_10M <<3)  | LDPC_1_2),
        //((MOD_16QAM<<6) | (BW_10M <<3)  | LDPC_2_3),
        ((MOD_64QAM<<6) | (BW_10M <<3)  | LDPC_1_2),
        ((MOD_64QAM<<6) | (BW_10M <<3)  | LDPC_2_3),
    };

    uint8_t map_idx_to_mode_20m[] = 
    {
        ((MOD_BPSK<<6)  | (BW_20M <<3)  | LDPC_1_2), //
        ((MOD_BPSK<<6)  | (BW_20M <<3)  | LDPC_1_2),
        ((MOD_4QAM<<6)  | (BW_20M <<3)  | LDPC_1_2),
        ((MOD_16QAM<<6) | (BW_20M <<3)  | LDPC_1_2),
        ((MOD_16QAM<<6) | (BW_20M <<3)  | LDPC_2_3),
    };


    BB_WriteReg( PAGE2, 0x0f, mcs_idx_reg0x0f_map[idx] );
    if (BW_10M == bw)
    {
        BB_WriteReg( PAGE2, TX_2, map_idx_to_mode_10m[idx]);
    }
    else
    {
        BB_WriteReg( PAGE2, TX_2, map_idx_to_mode_20m[idx]);
    }
    
    //if ( context.brc_mode == AUTO )
    {
        sky_notify_encoder_brc(0, BB_get_bitrateByMcs(bw, idx) );
        sky_notify_encoder_brc(1, BB_get_bitrateByMcs(bw, idx) );
    }
}



//*********************TX RX initial(14ms irq)**************
static void wimax_vsoc_rx_isr(uint32_t u32_vectorNum)
{
    INTR_NVIC_DisableIRQ(BB_RX_ENABLE_VECTOR_NUM);   
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);
    STRU_DEVICE_INFO *pst_devInfo = (STRU_DEVICE_INFO *)(DEVICE_INFO_SHM_ADDR);

    if( context.u8_flagdebugRequest & 0x80)
    {
        context.u8_debugMode = (context.u8_flagdebugRequest & 0x01);

        if( context.u8_debugMode )
        {
            osdptr->head = 0x00;
            osdptr->tail = 0xff;    //end of the writing
        }

        context.u8_flagdebugRequest = 0;

        if (context.u8_debugMode == TRUE)
        {
            BB_SPI_DisableEnable(0); //
        }
        else
        {
            BB_SPI_DisableEnable(1); //
        }

        osdptr->in_debug     = context.u8_debugMode;
        pst_devInfo->isDebug = context.u8_debugMode;        
    }

    INTR_NVIC_EnableIRQ(TIMER_INTR26_VECTOR_NUM);

    TIM_StartTimer(stru_skystatus.sky_timer2_6);
    TIM_StartTimer(stru_skystatus.sky_timer2_7);  //re-load timer7
}

/*
 * get snr, agc, ground rcid.
*/
static void sky_getSignalStatus(void)
{
    uint8_t flag_syncIdmatch;
    stru_skystatus.u16_sky_snr[stru_skystatus.u8_snr_idx] = sky_get_rc_snr();

    stru_skystatus.agc_value1[stru_skystatus.agc_idx] = BB_ReadReg(PAGE2, AAGC_2_RD);
    stru_skystatus.agc_value2[stru_skystatus.agc_idx] = BB_ReadReg(PAGE2, AAGC_3_RD);
    stru_skystatus.agc_idx ++;
    if ( stru_skystatus.agc_idx >= sizeof(stru_skystatus.agc_value2))
    {
        stru_skystatus.agc_idx = 0;
    }

    stru_skystatus.u_rcsync.u8_grdRcIdSearchStatus = BB_ReadReg(PAGE2, GRD_SEARCHING);
    if (1 == stru_skystatus.flag_followGroundInSearching)
    {
        sky_getGroudRcId(stru_skystatus.cur_groundRcid);

        if (GROUND_IN_RCID_SEARCH == stru_skystatus.u_rcsync.stru_rcIdStatus.u8_groundSearchMode)
        {
            stru_skystatus.u8_syncRcId[0] = BB_ReadReg(PAGE2, GROUND_SYNC_RC_ID_0);
            stru_skystatus.u8_syncRcId[1] = BB_ReadReg(PAGE2, GROUND_SYNC_RC_ID_1);
            stru_skystatus.u8_syncRcId[2] = BB_ReadReg(PAGE2, GROUND_SYNC_RC_ID_2);
            stru_skystatus.u8_syncRcId[3] = BB_ReadReg(PAGE2, GROUND_SYNC_RC_ID_3);
            stru_skystatus.u8_syncRcId[4] = BB_ReadReg(PAGE2, GROUND_SYNC_RC_ID_4);
        }
    }

    flag_syncIdmatch = (memcmp((void *)stru_skystatus.u8_syncRcId, (void *)context.rcid, RC_ID_SIZE) ? 0 : 1);
 
    stru_skystatus.flag_groundRequestDisconnect = (1==stru_skystatus.u_rcsync.stru_rcIdStatus.u8_flagGroundRequestDisconnect && 1==flag_syncIdmatch);
    stru_skystatus.flag_errorConnect  = ( GROUND_REQUEST_LOCK==stru_skystatus.u_rcsync.stru_rcIdStatus.u8_groundSearchMode && 
                                          (!SKY_ID_CRC_MATCH(stru_skystatus.u8_rcStatus))&& (0==flag_syncIdmatch));
}


static void Sky_TIM2_6_IRQHandler(uint32_t u32_vectorNum)
{
    static uint32_t u32_contiousUnlock = 0;
    uint8_t flag_rchop = 0;
    DEVICE_STATE pre_state=context.dev_state;

    Reg_Read32(BASE_ADDR_TIMER2 + TMRNEOI_6);

    INTR_NVIC_ClearPendingIRQ(BB_RX_ENABLE_VECTOR_NUM); //clear pending after TX Enable is LOW. MUST!
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);  

    INTR_NVIC_DisableIRQ(TIMER_INTR26_VECTOR_NUM);
    TIM_StopTimer(stru_skystatus.sky_timer2_6);
    stru_skystatus.u32_cyclecnt++;

    if(context.u8_debugMode)
    {
        return;
    }

    stru_skystatus.u8_rcStatus = get_rc_status();  //rc lock status statistic

    sky_checkRcCrcUnlockTimeOut(stru_skystatus.u8_rcStatus);
    
    stru_skystatus.flag_spiCmdValid = 0;
    if (SKY_CRC_OK(stru_skystatus.u8_rcStatus))
    {
        stru_skystatus.flag_spiCmdValid = BB_ChkSpiFlag();
        if (1 == stru_skystatus.flag_spiCmdValid)
        {
            sky_getSignalStatus();
        }
        sky_calc_dist();
    }
    else if (SKY_RC_ERR(stru_skystatus.u8_rcStatus) && context.dev_state != LOCK)
    {
        sky_soft_reset();
    }

    switch (context.dev_state)
    {
        case SEARCH_ID:
            if (SKY_CRC_OK(stru_skystatus.u8_rcStatus))
            {
                uint8_t flag_rcIdNotify = 0;
                if (stru_skystatus.flag_followGroundInSearching)
                {
                    if ( (GROUND_IN_RCID_SEARCH==stru_skystatus.u_rcsync.stru_rcIdStatus.u8_groundSearchMode && 1 != stru_skystatus.flag_groundRequestDisconnect) || 
                         (GROUND_REQUEST_LOCK==stru_skystatus.u_rcsync.stru_rcIdStatus.u8_groundSearchMode && 1 != stru_skystatus.flag_errorConnect))
                    {
                        flag_rchop = 1;
                        flag_rcIdNotify = 1;
                        context.dev_state = CHECK_LOCK;

                        memcpy((void *)stru_skystatus.flowing_rcId, (void *)stru_skystatus.cur_groundRcid, RC_ID_SIZE);
                    }
                    else
                    {
                        flag_rcIdNotify = 0;
                        sky_soft_reset();

                        //dlog_info("Rcid exclude %d %d %d "\
                        //          "%x %x %x %x %x",
                        //          stru_skystatus.u_rcsync.stru_rcIdStatus.u8_groundSearchMode, stru_skystatus.flag_groundRequestDisconnect, stru_skystatus.flag_errorConnect,
                        //          stru_skystatus.cur_groundRcid[0], stru_skystatus.cur_groundRcid[1], stru_skystatus.cur_groundRcid[2],
                        //          stru_skystatus.cur_groundRcid[3], stru_skystatus.cur_groundRcid[4]);

                    }
                }
                else //not flag_followGroundInSearching
                {
                    flag_rcIdNotify = 1;
                    sky_soft_reset();

                    dlog_info("Enter soft reset 2");
                }

                if (flag_rcIdNotify)
                {
                    STRU_SysEvent_DEV_BB_STATUS rcidEvent;
                    rcidEvent.pid = BB_GET_RCID;
                    memcpy(rcidEvent.rcid,  stru_skystatus.cur_groundRcid, RC_ID_SIZE);
                    SYS_EVENT_Notify(SYS_EVENT_ID_BB_EVENT, &rcidEvent);
                }
            }
            break;

        case CHECK_LOCK:

            if (sky_checkRcLock(stru_skystatus.u8_rcStatus)) //
            {
                flag_rchop = 1;
                context.dev_state  = LOCK;
                u32_contiousUnlock = 0;

                dlog_info("CHECK_LOCK->LOCK");
            }
            else if(stru_skystatus.flag_followGroundInSearching && u32_contiousUnlock ++ >= 80)
            {
                u32_contiousUnlock = 0;
                context.dev_state  = SEARCH_ID;

                dlog_info("CHECK_LOCK-> SEARCH_ID");
            }
            break;

        case LOCK:
            flag_rchop = 1;

            //handler in rc lock
            if (u32_contiousUnlock ++ >= 80 )           //ID_MATCH_LOCK -> CHECK_ID_MATCH
            {
                u32_contiousUnlock = 0;
                context.dev_state  = CHECK_LOCK;
                dlog_warning("LOCK->CHECK_LOCK");
            }
            else if (sky_checkRcLock(stru_skystatus.u8_rcStatus))
            {
                if (1 == stru_skystatus.flag_spiCmdValid)
                {
                    sky_handle_all_spi_cmds();
                }
                sky_adjustAgcGain();                        //agc is valid value only when locked
                u32_contiousUnlock = 0;
            }

            sky_rcFrqStatusStatistics();
            break;

       default:
            break;
    }

    if (1==stru_skystatus.flag_groundRequestDisconnect || 1==stru_skystatus.flag_errorConnect)
    {
        if (context.dev_state == LOCK)
        {
            context.dev_state = CHECK_LOCK;
        }

        flag_rchop = 0;
        sky_soft_reset();
    }

    if (stru_skystatus.u8_requestRcSearch)
    {
        stru_skystatus.u8_requestRcSearch = 0;
        context.dev_state = SEARCH_ID;
        sky_soft_reset();
    }

    //LED status switch
    if (pre_state != context.dev_state && (pre_state==LOCK || context.dev_state==LOCK))
    {
        STRU_SysEvent_DEV_BB_STATUS lockEvent ={
            .pid        = BB_LOCK_STATUS,
            .lockstatus = (context.dev_state==LOCK),
        };
        SYS_EVENT_Notify(SYS_EVENT_ID_BB_EVENT, (void *)&lockEvent);
    }

    if(AUTO == context.rcHopMode && 1 == flag_rchop)
    {
        sky_rc_hopfreq();
    }

    sky_handle_all_cmds();
    sky_doRfBandChange(stru_skystatus.u8_rcStatus);

    BB_skyPlot();
    BB_GetDevInfo();

    BB_UARTComCycleMsgProcess();
    BB_UARTComCycleSendMsg();
}


static void sky_rc_hopfreq(void)
{
	uint8_t max_ch_size = ((context.e_curBand == RF_2G) || (context.e_curBand == RF_600M)) ? (MAX_2G_RC_FRQ_SIZE):(MAX_5G_RC_FRQ_SIZE);

    context.sky_rc_channel++;
    if(context.sky_rc_channel >= max_ch_size)
    {
        context.sky_rc_channel = 0;
    }

    if ( !((stru_skystatus.s_st_rcFrqMask.u64_rcvGrdMask) & (((uint64_t)1) << context.sky_rc_channel)) )
    {
        BB_set_Rcfrq(context.e_curBand, context.sky_rc_channel);
    }
}


void sky_getGroudRcId(uint8_t* pu8_rcId)
{
    pu8_rcId[0] = BB_ReadReg(PAGE2, FEC_1_RD);
    pu8_rcId[1] = BB_ReadReg(PAGE2, FEC_2_RD_1);
    pu8_rcId[2] = BB_ReadReg(PAGE2, FEC_2_RD_2);
    pu8_rcId[3] = BB_ReadReg(PAGE2, FEC_2_RD_3);
    pu8_rcId[4] = BB_ReadReg(PAGE2, FEC_2_RD_4);   
}


static void sky_setRcId(uint8_t *pu8_rcId)
{
    uint8_t i;
    uint8_t addr[] = {FEC_7, FEC_8, FEC_9, FEC_10, FEC_11};

    dlog_critical("RCid:%02x%02x%02x%02x%02x\r\n", pu8_rcId[0], pu8_rcId[1], pu8_rcId[2], pu8_rcId[3], pu8_rcId[4]);                
    for(i=0; i < sizeof(addr); i++)
    {
        BB_WriteReg(PAGE2, addr[i], pu8_rcId[i]);
    }
}


void sky_SetSaveRCId(uint8_t *pu8_id)
{
    sky_setRcId(pu8_id);
    BB_saveRcid(pu8_id);
    memcpy( (void *)context.rcid, (void *)pu8_id, RC_ID_SIZE);
}


void sky_soft_reset(void)
{
    BB_softReset(BB_SKY_MODE);
}


static uint8_t get_rc_status(void)
{
    uint8_t lock = BB_ReadReg(PAGE2, FEC_4_RD);

    static int total_count = 0;
    static int lock_count = 0;
    static int nr_lock    = 0;

    static uint8_t pre_nrlockcnt = 0;
    static uint8_t pre_lockcnt = 0;

    total_count ++;
    lock_count += sky_checkRcLock(lock);
    nr_lock    += ((lock & 0x04) ? 1 : 0);

    if(total_count > 1000)
    {   
        dlog_warning("-L:%d-%d-%d", lock_count, nr_lock, total_count);
        pre_nrlockcnt = nr_lock;
        pre_lockcnt   = lock_count;
        total_count   = 0;
        lock_count    = 0;
        nr_lock       = 0;

        {
            STRU_skyStatusMsg stru_skycStatus = {
                .pid             = SKY_LOCK_STATUS,
                .par.rcLockCnt.u8_rcCrcLockCnt = pre_lockcnt,
                .par.rcLockCnt.u8_rcNrLockCnt  = pre_nrlockcnt,
            };

            //BB_UARTComSendMsg( BB_UART_COM_SESSION_0, (uint8_t *)&stru_skycStatus, sizeof(STRU_skyStatusMsg));
        }
    }

    return lock;
}


void Sky_TIM2_7_IRQHandler(uint32_t u32_vectorNum)
{
    INTR_NVIC_ClearPendingIRQ(TIMER_INTR27_VECTOR_NUM);  
    
    dlog_info("Toggle3");
    sky_agcGainToggle();

    //re-start timer 7
    TIM_StartTimer(stru_skystatus.sky_timer2_7);
}


static void sky_Timer2_7_Init(void)
{
    stru_skystatus.sky_timer2_7.base_time_group = 2;
    stru_skystatus.sky_timer2_7.time_num = 7;
    stru_skystatus.sky_timer2_7.ctrl = 0;
    stru_skystatus.sky_timer2_7.ctrl |= TIME_ENABLE | USER_DEFINED;
    TIM_RegisterTimer(stru_skystatus.sky_timer2_7, 560*1000);               //560ms

    reg_IrqHandle(TIMER_INTR27_VECTOR_NUM, Sky_TIM2_7_IRQHandler, NULL);
    INTR_NVIC_SetIRQPriority(TIMER_INTR27_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_TIMER01,0));
}

static void sky_Timer2_6_Init(void)
{
    stru_skystatus.sky_timer2_6.base_time_group = 2;
    stru_skystatus.sky_timer2_6.time_num = 6;
    stru_skystatus.sky_timer2_6.ctrl = 0;
    stru_skystatus.sky_timer2_6.ctrl |= TIME_ENABLE | USER_DEFINED;

    TIM_RegisterTimer(stru_skystatus.sky_timer2_6, 3800);

    reg_IrqHandle(TIMER_INTR26_VECTOR_NUM, Sky_TIM2_6_IRQHandler, NULL);
    INTR_NVIC_SetIRQPriority(TIMER_INTR26_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_TIMER00,0));
}



//////////////////////////////////////////////////////////////////////////////////////////

/*
 * read spi and handle it. interanal call in the intr.
*/


static void sky_handle_RC_cmd(void)
{
    uint8_t data0, data1, data2, data3, data4;
    data0 = BB_ReadReg(PAGE2, RC_CH_MODE_0);
    context.rcHopMode = (ENUM_RUN_MODE)data0;

    if( context.rcHopMode == (ENUM_RUN_MODE)MANUAL)
    {
        data1 = BB_ReadReg(PAGE2, RC_FRQ_0);
        data2 = BB_ReadReg(PAGE2, RC_FRQ_1);
        data3 = BB_ReadReg(PAGE2, RC_FRQ_2);
        data4 = BB_ReadReg(PAGE2, RC_FRQ_3);

        if ( context.stru_rcRegs.frq1 != data1 || context.stru_rcRegs.frq2 != data2 || 
             context.stru_rcRegs.frq3 != data3 || context.stru_rcRegs.frq4 != data4 )
        {
            uint32_t u32_rc = ( data1 << 24 ) | ( data2 << 16 ) | ( data3 << 8 ) | ( data4 );
            BB_write_RcRegs( u32_rc );
            dlog_info("--Write RC 0x%x", u32_rc);
        }
    }
}

static void sky_handle_RC_Rate_cmd(void)
{
    static uint8_t rate = 0;
    uint8_t data0 = BB_ReadReg(PAGE2, RC_RATE);

    if( rate != data0)
    {
        rate = data0;
        sky_ChgRcRate(rate);
        dlog_info("rc rate:%d", rate);
    }
}


static void sky_handle_IT_cmd(void)
{
    uint8_t data[5];

    data[0] = BB_ReadReg(PAGE2, IT_FRQ_0);
    data[1] = BB_ReadReg(PAGE2, IT_FRQ_1);
    data[2] = BB_ReadReg(PAGE2, IT_FRQ_2);
    data[3] = BB_ReadReg(PAGE2, IT_FRQ_3);
    data[4] = BB_ReadReg(PAGE2, IT_FRQ_4);

    if ( context.stru_itRegs.frq1 != data[0] || context.stru_itRegs.frq2 != data[1] || 
         context.stru_itRegs.frq3 != data[2] || context.stru_itRegs.frq4 != data[3] ||
         context.stru_itRegs.frq5 != data[4])
    {
        uint32_t u32_rc = ( data[0] << 24 ) | ( data[1] << 16 ) | ( data[2] << 8 ) | ( data[3] );
        if ( RF_600M == (context.e_curBand))
        {
            BB_write_ItRegsByArr( data );
        }
        else
        {
            BB_write_ItRegs( u32_rc );
            dlog_info("--write IT frq: 0x%08x", u32_rc);
        } 
    }
}


/*
 * check if the spi command for band change when lock
*/
static void sky_handle_RF_band_cmd(void)
{
    if ( 0 == context.stru_bandChange.flag_bandchange )
    {
        uint8_t data0 = BB_ReadReg(PAGE2, RF_BAND_CHANGE_0);
        uint8_t data1 = BB_ReadReg(PAGE2, RF_BAND_CHANGE_1);
        
        if ( (data0 & 0x03) == (data1 & 0x03) && (data0 != 0 ))
        {
            if ( context.e_curBand != (ENUM_RF_BAND)(data0 & 0x03) )
            {
                context.stru_bandChange.flag_bandchange = 1;
                context.stru_bandChange.u8_bandChangecount = ((data0 & 0xFC) >> 2);
                context.stru_bandChange.u8_ItCh = ((data1 & 0xFC) >> 2);
            }
         }
    }

}

/*
 * call every 14ms cycle in ID_MATCH_LOCK status
*/
static void sky_backTo2GCheck(uint8_t u8_lockStatus);

static void sky_doRfBandChange(uint8_t u8_lockStatus)
{
    if ( 1 == context.stru_bandChange.flag_bandchange )
    {
        context.stru_bandChange.u8_bandChangecount--;
        if ( 0 == context.stru_bandChange.u8_bandChangecount )
        {
            context.stru_bandChange.flag_bandchange = 0;
            context.stru_bandChange.u8_bandChangecount = 0;
            context.e_curBand = OTHER_BAND( context.e_curBand );

            BB_set_RF_Band(BB_SKY_MODE, context.e_curBand);

            dlog_info("band: %d %d %d %d",  context.sky_rc_channel, context.stru_bandChange.u8_bandChangecount, 
                                            context.stru_bandChange.u8_ItCh, context.e_curBand);

            context.sky_rc_channel = 0;
            sky_rc_hopfreq();
            BB_set_ItFrqByCh( context.e_curBand, context.stru_bandChange.u8_ItCh);
        }
        else
        {
            //dlog_info("!! ch: %d %d", context.sky_rc_channel, context.stru_bandChange.u8_bandChangecount);
        }   
    }

    sky_backTo2GCheck(u8_lockStatus);
}


static void sky_backTo2GCheck(uint8_t u8_lockStatus)
{
    if (context.e_bandsupport == RF_2G_5G && context.e_curBand == RF_5G)
    {
        if (SKY_CRC_OK(u8_lockStatus))
        {
            return;
        }

        stru_skystatus.u16_rcUnlockCntBandSwitch ++;         //5G: 240 * 14ms = 4080ms
        if ( stru_skystatus.u16_rcUnlockCntBandSwitch ++ >= 285 )
        {
            //switch band to 2G
            context.e_curBand = RF_2G;
            BB_set_RF_Band(BB_SKY_MODE, RF_2G);

            //set RC
            context.sky_rc_channel = 0;
            sky_rc_hopfreq();

            //set IT
            BB_set_ItFrqByCh( context.e_curBand, 0);

            stru_skystatus.u16_rcUnlockCntBandSwitch = 0;
        }
    }
}

/*
 *  handle command for 10M, 20M
*/
static void sky_handle_CH_bandwitdh_cmd(void)
{   
    uint8_t data0, data1;
    
    data0 = BB_ReadReg(PAGE2, RF_CH_BW_CHANGE_0);

    if((data0&0xc0)==0xc0)
    {
        ENUM_CH_BW bw = (ENUM_CH_BW)(data0&0x3F);

        if(context.e_bandwidth != bw)
        {
            //set and soft-rest
            BB_set_RF_bandwitdh(BB_SKY_MODE, bw);
            context.e_bandwidth = bw;
        }
    }
}

static void sky_handle_CH_qam_cmd(void)
{   
    uint8_t data0;
    
    data0 = BB_ReadReg(PAGE2, RF_CH_QAM_CHANGE_0);

    if((data0&0xc0)==0xc0)
    {
        ENUM_BB_QAM qam = (ENUM_BB_QAM)(data0 & 0x03);

        if(context.qam_mode != qam)
        {
            //set and soft-rest
            BB_set_QAM(qam);
            context.qam_mode = qam;
            //dlog_info("CH_QAM =%d\r\n", context.qam_mode);
        }
    }
}

static void sky_handle_CH_ldpc_cmd(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, RF_CH_LDPC_CHANGE_0);

    if((context.ldpc) != data0)
    {
        context.ldpc = data0;
        BB_set_LDPC(context.ldpc);
        //dlog_info("CH_ldpc=>%d\n", context.ldpc);
    }
}
static void sky_handle_brc_mode_cmd(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, ENCODER_BRC_MODE_0);
    ENUM_RUN_MODE mode = (ENUM_RUN_MODE)(data0 & 0x1f);

    if(((data0&0xe0)==0xe0) && (context.brc_mode != mode))
    {
        //dlog_info("brc_mode = %d \r\n", mode);
        context.brc_mode = mode;
        
        if ( mode == AUTO) //MANUAL - > AUTO
        {
            sky_set_McsByIndex( context.e_bandwidth, context.qam_ldpc );
        }
    }
}


/*
  * handle H264 encoder brc 
 */
static void sky_handle_brc_bitrate_cmd(void)
{
    uint8_t data0;
    uint8_t bps;
    data0 = BB_ReadReg(PAGE2, ENCODER_BRC_CHAGE_0_CH1);
    bps = data0&0x3F;

    if(((data0&0xc0)==0xc0) && (context.brc_bps[0] != bps))
    {
        context.brc_bps[0] = bps;
        sky_notify_encoder_brc(0, bps);
        dlog_info("ch1 brc_bps = %d \r\n", bps);
    }

    data0 = BB_ReadReg(PAGE2, ENCODER_BRC_CHAGE_0_CH2);
    bps = data0&0x3F;

    if(((data0&0xc0)==0xc0) && (context.brc_bps[1] != bps))
    {
        context.brc_bps[1] = bps;
        sky_notify_encoder_brc(1, bps);
        dlog_info("ch2 brc_bps = %d \r\n", bps);
    }

}

static void sky_handle_QAM_cmd(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, QAM_CHANGE_0);

    if((context.qam_ldpc) != data0)
    {
        BB_WriteReg(PAGE2, TX_2, data0);
    }
}

static void sky_handle_MCS_cmd(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, MCS_INDEX_MODE_0);

    if((context.qam_ldpc) != data0)
    {
        sky_set_McsByIndex(context.e_bandwidth, data0);
        context.qam_ldpc = data0;
    }
}

static void sky_handle_rc_rcv_grd_mask_code_cmd(void)
{
    uint8_t i;
    uint64_t u64_tmpMask = 0;
    uint8_t *pu8_tmpAddr = (uint8_t *)(&u64_tmpMask);

    for (i=0; i<sizeof(uint64_t); i++)
    {
        pu8_tmpAddr[i] = BB_ReadReg(PAGE2, GRD_MASK_CODE + i);
    }
    if ((stru_skystatus.s_st_rcFrqMask.u64_rcvGrdMask) != u64_tmpMask)
    {
        stru_skystatus.s_st_rcFrqMask.u64_rcvGrdMask = u64_tmpMask;
        dlog_info("rcv_grd_mask:0x%x,0x%x", (uint32_t)((u64_tmpMask)>>32), (uint32_t)(u64_tmpMask));
    }
}

static void sky_handle_rc_channel_sync_cmd(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, GRD_RC_CHANNEL);
    uint8_t max_ch_size = ((context.e_curBand == RF_2G) || (context.e_curBand == RF_600M)) ? (MAX_2G_RC_FRQ_SIZE):(MAX_5G_RC_FRQ_SIZE);
    
    if (data0 != context.sky_rc_channel)
    {
        context.sky_rc_channel = data0;
        dlog_info("sync rc channel %d %d", data0, context.sky_rc_channel);
    }
}


static void sky_handle_frqOffset(void)
{    
    int frqoffset = (BB_ReadReg(PAGE2, FRE_OFFSET_0) << 24) | (BB_ReadReg(PAGE2, FRE_OFFSET_1) << 16) | 
                    (BB_ReadReg(PAGE2, FRE_OFFSET_2) <<  8) |  BB_ReadReg(PAGE2, FRE_OFFSET_3);
    uint8_t tmp = BB_ReadReg(PAGE2, FRE_OFFSET_4);

    if ( frqoffset == 0)
    {
        return;
    }

    if ( stru_skystatus.u32_cyclecnt == 0)   //17 * 200 = 3.4s
    {
        int integer, fraction, tracking;
        ENUM_RF_BAND e_band;
        int basefrq;
        uint8_t ch;

        e_band = (ENUM_RF_BAND)(tmp >> 6);
        ch = tmp & 0x3f;

        if ( stru_skystatus.stru_skyfrqOffst.baseOffset == 0)
        {
            integer  = BB_ReadReg(PAGE2, INTEGER_OFFSET);
            
            fraction = (BB_ReadReg(PAGE2, FRACTION_OFFSET_0) << 24) | (BB_ReadReg(PAGE2, FRACTION_OFFSET_1) << 16) |
                       (BB_ReadReg(PAGE2, FRACTION_OFFSET_2) <<  8) | (BB_ReadReg(PAGE2, FRACTION_OFFSET_3));
                       
            tracking = (BB_ReadReg(PAGE2, TRACK_OFFSET_0) << 24) | (BB_ReadReg(PAGE2, TRACK_OFFSET_1) << 16) |
                       (BB_ReadReg(PAGE2, TRACK_OFFSET_2) <<  8) | (BB_ReadReg(PAGE2, TRACK_OFFSET_3));
        }

        if ( e_band == RF_2G && context.e_bandwidth == BW_10M )
        {
            basefrq = 2406 + ch * 10;
        }
        else if( e_band == RF_2G && context.e_bandwidth == BW_20M )
        {
        }
        else if ( e_band == RF_5G && context.e_bandwidth == BW_10M )
        {
            basefrq = 5730 + ch * 10;
        }
        else if( e_band == RF_2G && context.e_bandwidth == BW_20M )
        {
        }

        stru_skystatus.stru_skyfrqOffst.u8_flagOffset = 1;
        stru_skystatus.stru_skyfrqOffst.baseOffset = ( fraction + tracking - (integer << 19) );
        stru_skystatus.stru_skyfrqOffst.ll_offset  = frqoffset * 8;
        stru_skystatus.stru_skyfrqOffst.basefrq    = basefrq;

        dlog_info("base: %d %d %d", basefrq, frqoffset, stru_skystatus.stru_skyfrqOffst.baseOffset);
    }
};

/*
 * called the function if unlocked
*/
static void sky_clearFrqOffset(void)
{
    if (stru_skystatus.stru_skyfrqOffst.u8_flagOffset)
    {        
        stru_skystatus.stru_skyfrqOffst.u8_flagOffset = 0;  //
        stru_skystatus.u32_cyclecnt  = 0;

        if ( stru_skystatus.stru_skyfrqOffst.u8_softEnable == 0 )
        {
            BB_WriteRegMask(PAGE1, 0x1a, 0x40, 0x40);
            stru_skystatus.stru_skyfrqOffst.u8_softEnable = 1;
        }
    }
}


void sky_handleRcIdAutoSearchCmd(STRU_WIRELESS_CONFIG_CHANGE* pcmd)
{
    uint8_t u8_data;
    uint8_t class  = pcmd->u8_configClass;
    uint8_t item   = pcmd->u8_configItem;
    uint32_t value = pcmd->u32_configValue;
    uint32_t value1= pcmd->u32_configValue1;
    uint8_t  u8_rcArray[5] = { (value>>24)&0xff, (value>>16)&0xff, (value>>8)&0xff, (value)&0xff, value1&0xff};

    if(class == WIRELESS_AUTO_SEARCH_ID)
    {
        switch (item)
        {
            case RCID_AUTO_SEARCH:
                stru_skystatus.u8_requestRcSearch = 1;
                stru_skystatus.flag_followGroundInSearching = pcmd->u32_configValue;
                dlog_info("Request to search Follow: %d", stru_skystatus.flag_followGroundInSearching);
                break;

            case RCID_SAVE_RCID:
                sky_SetSaveRCId(u8_rcArray);
                break;

            default:
                dlog_warning("Error: id %x %x", class, item);
                break;
        }
    }
}


/*
 * if in searching & lock & request rc id 
*/
static void sky_handle_rcIdSync(void)
{
    if (0 == stru_skystatus.u32_cyclecnt % 3 && 1 == stru_skystatus.flag_followGroundInSearching)  //sky auto harq and in follow search mode
    {
        //dlog_info("%d %d %d %d", stru_skystatus.u_rcsync.stru_rcIdStatus.u8_groundSearchMode, stru_skystatus.u_rcsync.stru_rcIdStatus.u8_itLock, 
        //                         stru_skystatus.flag_errorConnect, stru_skystatus.flag_groundRequestDisconnect);

        if (GROUND_IN_RCID_SEARCH==stru_skystatus.u_rcsync.stru_rcIdStatus.u8_groundSearchMode && 1==stru_skystatus.u_rcsync.stru_rcIdStatus.u8_itLock && 
            stru_skystatus.flag_errorConnect == 0 && stru_skystatus.flag_groundRequestDisconnect == 0)
        {
            if (0 != memcmp( (void *)stru_skystatus.u8_syncRcId, (void *)context.rcid, RC_ID_SIZE) )
            {
                STRU_skyStatusMsg stru_rcSync;
                
                stru_rcSync.pid = RC_ID_SYNC;
                memcpy(stru_rcSync.par.rcId.u8_skyRcIdArray, (void *)context.rcid, RC_ID_SIZE);
                memcpy(stru_rcSync.par.rcId.u8_grdRcIdArray, (void *)stru_skystatus.flowing_rcId, RC_ID_SIZE);

                BB_UARTComSendMsg(BB_UART_COM_SESSION_0, (uint8_t *)&stru_rcSync, sizeof(stru_rcSync));
            }
        }
    }
}

static void sky_handle_all_spi_cmds(void)
{
    sky_handle_RC_cmd();

    sky_handle_RC_Rate_cmd();

    sky_handle_IT_cmd();

    //sky_handle_QAM_cmd();
    
    sky_handle_brc_mode_cmd();

    sky_handle_MCS_cmd();

    sky_handle_CH_bandwitdh_cmd();

    sky_handle_CH_qam_cmd();

    sky_handle_CH_ldpc_cmd();

    sky_handle_brc_bitrate_cmd();

    sky_handle_RF_band_cmd();

    sky_handle_rc_rcv_grd_mask_code_cmd();

//    sky_handle_rc_channel_sync_cmd();

    sky_handle_rcIdSync();

    //sky_handle_frqOffset();
}

static void sky_handle_one_cmd(STRU_WIRELESS_CONFIG_CHANGE* pcmd)
{
    uint8_t u8_data;
    uint8_t class  = pcmd->u8_configClass;
    uint8_t item   = pcmd->u8_configItem;
    uint32_t value = pcmd->u32_configValue;

    dlog_info("class item value %d %d 0x%0.8d \r\n", class, item, value);
    if(class == WIRELESS_FREQ_CHANGE)
    {
        switch(item)
        {
            case FREQ_BAND_SELECT:
            {
                context.e_curBand = (ENUM_RF_BAND)(value);
                BB_set_RF_Band(BB_SKY_MODE, context.e_curBand);
                dlog_info("context.e_curBand %d \r\n", context.e_curBand);
                break;
            }
            case FREQ_CHANNEL_MODE: //auto manual
            {
                context.itHopMode = (ENUM_RUN_MODE)value;
                break;
            }

            case RC_CHANNEL_MODE:
            {
                context.rcHopMode = (ENUM_RUN_MODE)value;
                break;
            }

            case RC_CHANNEL_SELECT:
            {
                context.sky_rc_channel = (uint8_t)value;
                BB_set_Rcfrq(context.e_curBand, context.sky_rc_channel);
                break;
            }

            case RC_CHANNEL_FREQ:
            {
                context.rcHopMode = (ENUM_RUN_MODE)MANUAL;                
                BB_write_RcRegs(value);
                
                dlog_info("RC_CHANNEL_FREQ %x\r\n", value);
                break;
            }
            
            case IT_CHANNEL_FREQ:
            {
                context.itHopMode = MANUAL;
                BB_write_ItRegs(value);
                dlog_info("IT_CHANNEL_FREQ %x\r\n", value);                
                break;
            }

            case FREQ_BAND_WIDTH_SELECT:
            {
                context.e_bandwidth = (ENUM_CH_BW)value;
                BB_set_RF_bandwitdh(BB_SKY_MODE, context.e_bandwidth);
                dlog_info("FREQ_BAND_WIDTH_SELECT %x\r\n", value);         
                break;
            }

            default:
            {
                dlog_warning("%s\r\n", "unknown WIRELESS_FREQ_CHANGE command");
                break;
            }
        }
    }
    
    if(class == WIRELESS_MCS_CHANGE)
    {
        switch(item)
        {
            case MCS_MODE_SELECT:
            {
                context.qam_skip_mode = (ENUM_RUN_MODE)value;
                context.brc_mode = (ENUM_RUN_MODE)value;
                dlog_info("qam_skip_mode = %d \r\n", context.qam_skip_mode);
                dlog_info("brc_mode = %d \r\n", context.brc_mode);
                break;
            }

            case MCS_MODULATION_SELECT:
            {
                break;
            }

            case MCS_CODE_RATE_SELECT:
            {
                break;
            }
                
            case MCS_IT_QAM_SELECT:
            {
                BB_set_QAM((ENUM_BB_QAM)value);
                dlog_info("MCS_IT_QAM_SELECT %x\r\n", value);                
                break;
            }
            
            case MCS_IT_CODE_RATE_SELECT:
            {
                BB_set_LDPC((ENUM_BB_LDPC)value);
                dlog_info("MCS_IT_CODE_RATE_SELECT %x\r\n", value);             
                break;
            }
            
            case MCS_RC_QAM_SELECT:
            {
                context.rc_qam_mode = (ENUM_BB_QAM)value;
                BB_WriteRegMask(PAGE2, 0x09, (value << 0) & 0x03, 0x03);
                dlog_info("MCS_RC_QAM_SELECT %x\r\n", value);                
                break;
            }
            
            case MCS_RC_CODE_RATE_SELECT:
            {
                context.rc_ldpc = (ENUM_BB_LDPC)value;
                BB_WriteRegMask(PAGE2, 0x09, (value << 2) & 0x1C, 0x1C);
                dlog_info("MCS_RC_CODE_RATE_SELECT %x\r\n", value); 
                break;               
            }
            default:
                dlog_warning("%s\r\n", "unknown WIRELESS_MCS_CHANGE command");
                break;
        }        
    }
    if(class == WIRELESS_ENCODER_CHANGE)
    {
        switch(item)
        {
            case ENCODER_DYNAMIC_BIT_RATE_MODE:
                context.brc_mode = (ENUM_RUN_MODE)value;
                break;

            case ENCODER_DYNAMIC_BIT_RATE_SELECT_CH1:
                sky_notify_encoder_brc(0,(uint8_t)value);
                break;

            case ENCODER_DYNAMIC_BIT_RATE_SELECT_CH2:
                sky_notify_encoder_brc(1, (uint8_t)value);
                break;
            default:
                dlog_warning("%s\r\n", "unknown WIRELESS_ENCODER_CHANGE command");
                break;                
        }
    }
    
    if(class == WIRELESS_MISC)
    {
        BB_handle_misc_cmds(pcmd);
    }

    if(class == WIRELESS_OTHER)
    {
        switch(item)
        {
            case GET_DEV_INFO:
            {
                BB_GetDevInfo();
                break;
            }

            case SWITCH_ON_OFF_CH1:
            {
                //BB_SwtichOnOffCh(0, (uint8_t)value);
                dlog_info("sky invalid cmd.");
                break;
            }

            case SWITCH_ON_OFF_CH2:
            {
                //BB_SwtichOnOffCh(1, (uint8_t)value);
                dlog_info("sky invalid cmd.");
                break;
            }
 
            case BB_SOFT_RESET:
            {
                dlog_info("sky bb reset.");
                BB_softReset(BB_SKY_MODE);
                break;
            }
            default:
                break;                
        }
    }

    if (class == WIRELESS_AUTO_SEARCH_ID)
    {
        sky_handleRcIdAutoSearchCmd(pcmd);
    }
}


static void sky_handle_all_cmds(void)
{   
    int ret = 0;
    int cnt = 0;
    STRU_WIRELESS_CONFIG_CHANGE cfg;
    while( (cnt++ < 5) && (1 == BB_GetCmd(&cfg)))
    {
        sky_handle_one_cmd( &cfg );
    }
}


/*
 *
*/
static void BB_skyPlot(void)
{
    uint8_t u8_data;
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);


    if (osdptr->osd_enable == 0)
    {
        return;
    }

    
    osdptr->messageId = 0x33;
    osdptr->head = 0xff; //starting writing
    osdptr->tail = 0x00;

    osdptr->IT_channel = context.cur_IT_ch;

    osdptr->agc_value[0] = BB_ReadReg(PAGE2, AAGC_2_RD);
    osdptr->agc_value[1] = BB_ReadReg(PAGE2, AAGC_3_RD);
    
    osdptr->agc_value[2] = stru_skystatus.u8_rcStatus;
    osdptr->agc_value[3] = BB_ReadReg(PAGE2, 0xd7);
    
    //osdptr->agc_value[2] = BB_ReadReg(PAGE2, RX3_GAIN_ALL_R);
    //osdptr->agc_value[3] = BB_ReadReg(PAGE2, RX4_GAIN_ALL_R);

    osdptr->snr_vlaue[0] = (((uint16_t)BB_ReadReg(PAGE2, SNR_REG_0)) << 8) | BB_ReadReg(PAGE2, SNR_REG_1);
    osdptr->snr_vlaue[1] = sky_get_snr_average();
    
    u8_data = BB_ReadReg(PAGE2, TX_2);
    osdptr->modulation_mode = (u8_data >> 6) & 0x03;
    osdptr->code_rate       = (u8_data >> 0) & 0x07;
    
    u8_data = BB_ReadReg(PAGE2, 0x09);
    osdptr->rc_modulation_mode = (u8_data >> 2) & 0x07;
    osdptr->rc_code_rate       = (u8_data >> 0) & 0x03;

    osdptr->lock_status  = get_rc_status();
    osdptr->in_debug     = context.u8_debugMode;

    osdptr->head = 0x00;
    osdptr->tail = 0xff;    //end of the writing
}



static void sky_calc_dist(void)
{
    uint8_t u8_data[3];
    uint32_t u32_data = 0;

    u8_data[0] = BB_ReadReg(PAGE3, 0xA4);
    u8_data[1] = BB_ReadReg(PAGE3, 0xA3);
    u8_data[2] = BB_ReadReg(PAGE3, 0xA2);

    BB_WriteReg(PAGE0, 0x1D, u8_data[0]);
    BB_WriteReg(PAGE0, 0x1C, u8_data[1]);
    BB_WriteReg(PAGE0, 0x1B, u8_data[2]);
}


static uint16_t sky_get_rc_snr( void )
{
    static uint32_t cnt = 0;
    uint16_t snr = (((uint16_t)BB_ReadReg(PAGE2, SNR_REG_0)) << 8) | BB_ReadReg(PAGE2, SNR_REG_1);
    if( cnt++ > 1500 )
    {
        cnt = 0;
        dlog_warning("SNR1:%0.4x\n", snr);
    }

    return snr;
}


static uint16_t sky_get_snr_average(void)
{
    uint8_t i;
    uint32_t sum = 0; 
    for (i = 0; i < sizeof(stru_skystatus.u16_sky_snr) / sizeof(stru_skystatus.u16_sky_snr[0]); i++)
    {
        sum += stru_skystatus.u16_sky_snr[i];
    }

    return (sum/i);
}

static void sky_initRcFrqMaskFunc(void)
{
    memset((void *)(&stru_skystatus.s_st_rcFrqMask), 0x00, sizeof(STRU_RC_FRQ_MASK));
}


static void sky_rcFrqStatusStatistics(void)
{
    static uint16_t u16_txCnt = 0;
    uint8_t u8_rcCh = 0;
    uint8_t u8_cnt = 0;
    uint8_t u8_clcCh = 0;
    uint8_t i = 0;
    uint64_t u64_mask;
    uint64_t u64_preMask = stru_skystatus.s_st_rcFrqMask.u64_mask;
    int8_t max_ch_size = ((context.e_curBand == RF_2G) || (context.e_curBand == RF_600M)) ? (MAX_2G_RC_FRQ_SIZE):(MAX_5G_RC_FRQ_SIZE);
    
    //if(context.dev_state == ID_MATCH_LOCK)
    {
        if (0 == context.sky_rc_channel)
        {
            u8_rcCh = max_ch_size - 1;
        }
        else
        {
            u8_rcCh = context.sky_rc_channel - 1;
        }

        if ((stru_skystatus.s_st_rcFrqMask.u64_mask) & (((uint64_t)1) << u8_rcCh))
        {
            return;
        }
        
        if(context.locked)
        {
            stru_skystatus.s_st_rcFrqMask.u8_unLock[u8_rcCh] = 0;
        }
        else
        {
            stru_skystatus.s_st_rcFrqMask.u8_unLock[u8_rcCh] += 1;
        }

        if (stru_skystatus.s_st_rcFrqMask.u8_unLock[u8_rcCh] >= RC_FRQ_MASK_THRESHOLD)
        {
            stru_skystatus.s_st_rcFrqMask.u64_mask |= ( ((uint64_t)1) << u8_rcCh );
            
            u8_clcCh = stru_skystatus.s_st_rcFrqMask.u8_maskOrder[stru_skystatus.s_st_rcFrqMask.u8_maskOrderIndex];
            stru_skystatus.s_st_rcFrqMask.u8_maskOrder[stru_skystatus.s_st_rcFrqMask.u8_maskOrderIndex] = u8_rcCh;
            stru_skystatus.s_st_rcFrqMask.u8_maskOrderIndex += 1;
            stru_skystatus.s_st_rcFrqMask.u8_maskOrderIndex %= RC_FRQ_MAX_MASK_NUM;

            u8_cnt = 0;
            u64_mask = stru_skystatus.s_st_rcFrqMask.u64_mask;
            while(i < max_ch_size)
            {
                if (u64_mask & ((uint64_t)0x01))
                {
                    u8_cnt += 1;
                }
                u64_mask = (u64_mask >> 1);
                i += 1;
            }
            
            if (u8_cnt > RC_FRQ_MAX_MASK_NUM) // unmask oldest rf frq
            {
                stru_skystatus.s_st_rcFrqMask.u64_mask &= (~(((uint64_t)1) << (u8_clcCh)));
            }

            // mask one, then all start again.
            memset(&(stru_skystatus.s_st_rcFrqMask.u8_unLock[0]), 0x00, RF_FRQ_MAX_NUM);
            stru_skystatus.s_st_rcFrqMask.u8_unLock[u8_rcCh] = RC_FRQ_MASK_THRESHOLD;
        }

        if (u64_preMask != (stru_skystatus.s_st_rcFrqMask.u64_mask))
        {
            u16_txCnt = 169;
            //dlog_info("new_calc_mask_code:0x%x,0x%x", (uint32_t)((stru_skystatus.s_st_rcFrqMask.u64_mask)>>32), (uint32_t)(stru_skystatus.s_st_rcFrqMask.u64_mask));
        }

        u16_txCnt += 1;
        if (170 == u16_txCnt) // 170 * 14 = 2380ms
        {
            STRU_skyStatusMsg stru_skycStatus = 
            {
                .pid             = RC_MASK_CODE,
                .par.u64_rcMask = stru_skystatus.s_st_rcFrqMask.u64_mask,
            };
            
#if 0
            BB_UARTComSendMsg( BB_UART_COM_SESSION_0, (uint8_t *)&stru_skycStatus, sizeof(STRU_skyStatusMsg));
#endif
            u16_txCnt = 0;
        }
    }
}

static void sky_ChgRcRate(uint8_t rate)
{
    if (0 == rate)
    {
        BB_WriteReg(PAGE2, 0x09, 0x80); // BPSK 1/2
    }
    else if(1 == rate)
    {
        BB_WriteReg(PAGE2, 0x09, 0x85); // QPSK 2/3
    }
    else
    {
        ;
    }

    BB_softReset(BB_SKY_MODE);
}

