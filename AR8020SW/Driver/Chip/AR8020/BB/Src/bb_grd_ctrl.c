#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "debuglog.h"
#include "interrupt.h"
#include "timer.h"
#include "reg_rw.h"
#include "bb_regs.h"
#include "bb_ctrl_internal.h"
#include "bb_snr_service.h"
#include "bb_grd_ctrl.h"
#include "bb_grd_sweep.h"
#include "bb_uart_com.h"

#define SNR_STATIC_START_VALUE          (100)
#define SNR_STATIC_UP_THRESHOD          (5)
#define SNR_STATIC_DOWN_THRESHOD        (2)
#define DEFAULT_DIST_ZERO_10M           (1560)
#define DEFAULT_DIST_ZERO_20M           (4386)


static init_timer_st grd_timer2_6;
static init_timer_st grd_timer2_7;

static uint8_t  Timer1_Delay1_Cnt = 0;
static uint8_t  snr_static_count  = SNR_STATIC_START_VALUE;
static uint8_t  hop_count = 0;
static uint8_t  flag_itFreqskip = 0;
static uint8_t  flag_snrPostCheck;
static uint64_t s_u64_rcMask = 0;
static uint8_t  s_u8_rcMaskEnable = 0;
static STRU_DELAY_CMD grd_RcChgRate = {0, 0, 0};


static STRU_SkyStatus g_stru_skyStatus;

UNION_grdRcIdSearchStatus u_grdRcIdSearchStatus;

uint8_t grd_rc_channel = 0;

//uint8_t u8_preRcIdArray[5];

STRU_CALC_DIST_DATA s_st_calcDistData = 
{
    .e_status = INVALID,
    .u32_calcDistValue = 0,
    .u32_calcDistZero = 0,
    .u32_cnt = 0,
    .u32_lockCnt = 0
};

static void BB_grd_uartDataHandler(void *p);

static uint32_t grd_calc_dist_get_avg_value(uint32_t *u32_dist);

static void grd_disable_enable_rc_frq_mask_func(uint8_t flag);

static void grd_init_rc_frq_mask_func(void);

static void grd_write_mask_code_to_sky(uint8_t enable, uint64_t *mask);

static void grd_set_ItFrq(ENUM_RF_BAND e_band, uint8_t ch);

static void BB_grd_OSDPlot(void);

static void grd_handle_IT_mode_cmd(ENUM_RUN_MODE mode);

static void grd_calc_dist_zero_calibration(void);

static void grd_calc_dist(void);

static void grd_handle_all_cmds(void);

static void Grd_Timer2_6_Init(void);

static void Grd_Timer2_7_Init(void);

static void grd_handle_RF_band_cmd(ENUM_RF_BAND rf_band);

static void grd_handle_CH_bandwitdh_cmd(ENUM_CH_BW bw);

static void grd_handle_MCS_mode_cmd(ENUM_RUN_MODE mode);

static void grd_handle_MCS_cmd(ENUM_BB_QAM qam, ENUM_BB_LDPC ldpc);

static void grd_handle_brc_mode_cmd(ENUM_RUN_MODE mode);

static void grd_handle_brc_cmd(uint8_t coderate);

static void grd_rc_hopfreq(void);

static void reset_it_span_cnt(void);

static void grd_set_txmsg_qam_change(ENUM_BB_QAM qam, ENUM_CH_BW bw, ENUM_BB_LDPC ldpc);

static void grd_handle_IT_CH_cmd(uint8_t ch);

static void wimax_vsoc_tx_isr(uint32_t u32_vectorNum);

static void grd_set_calc_dist_zero_point(uint32_t value);

static void grd_ChgRcRate(uint8_t rate);

void BB_GRD_start(void)
{
    context.dev_state = INIT_DATA;
    context.flag_signalBlock = 1;       //default signal is block case.
    context.qam_ldpc = context.u8_bbStartMcs;
    context.flag_mrc = 0;

    grd_set_txmsg_mcs_change(context.e_bandwidth, context.qam_ldpc);

    Grd_Timer2_6_Init();
    Grd_Timer2_7_Init();

    BB_set_Rcfrq(context.e_curBand, 0);

    //do not notify sky until sweep end and get the best channel from sweep result
    context.cur_IT_ch = 1;
    grd_set_ItFrq(context.e_curBand, 1);
    BB_grd_NotifyItFreqByCh(context.e_curBand, 1);

    BB_SweepStart(context.e_curBand, context.e_bandwidth);

    grd_init_rc_frq_mask_func();

    BB_UARTComInit( BB_grd_uartDataHandler ); 
    BB_GetDevInfo();

    grd_SetRCId((uint8_t *)context.rcid);

    reg_IrqHandle(BB_TX_ENABLE_VECTOR_NUM, wimax_vsoc_tx_isr, NULL);
    INTR_NVIC_SetIRQPriority(BB_TX_ENABLE_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_BB_TX,0));
    INTR_NVIC_EnableIRQ(BB_TX_ENABLE_VECTOR_NUM);}


static void BB_setWriteSkyRcId(uint8_t *pu8_rcId)
{
    BB_WriteReg(PAGE2, GROUND_SYNC_RC_ID_0, pu8_rcId[0]);
    BB_WriteReg(PAGE2, GROUND_SYNC_RC_ID_1, pu8_rcId[1]);
    BB_WriteReg(PAGE2, GROUND_SYNC_RC_ID_2, pu8_rcId[2]);
    BB_WriteReg(PAGE2, GROUND_SYNC_RC_ID_3, pu8_rcId[3]);
    BB_WriteReg(PAGE2, GROUND_SYNC_RC_ID_4, pu8_rcId[4]);
}


static int BB_setDisconnectRcId(uint8_t *pu8_rcId)
{
    if (u_grdRcIdSearchStatus.stru_rcIdStatus.u8_groundSearchMode == GROUND_IN_RCID_SEARCH)
    {
        u_grdRcIdSearchStatus.stru_rcIdStatus.u8_flagGroundRequestDisconnect = 1;
        BB_WriteReg(PAGE2, GRD_SEARCHING, u_grdRcIdSearchStatus.u8_grdRcIdSearchStatus);
        
        BB_setWriteSkyRcId(pu8_rcId);
        
        dlog_info("disconnect id %x %x %x %x %x %x", pu8_rcId[0], pu8_rcId[1], pu8_rcId[2], pu8_rcId[3], pu8_rcId[4], u_grdRcIdSearchStatus.u8_grdRcIdSearchStatus);
        return 0;
    }

    return 1;
}


static void BB_setConnectRcId(uint8_t *pu8_rcId)
{
    BB_setWriteSkyRcId(pu8_rcId);

    u_grdRcIdSearchStatus.stru_rcIdStatus.u8_groundSearchMode = GROUND_REQUEST_LOCK;
    u_grdRcIdSearchStatus.stru_rcIdStatus.u8_flagGroundRequestDisconnect = 0;

    BB_WriteReg(PAGE2, GRD_SEARCHING, u_grdRcIdSearchStatus.u8_grdRcIdSearchStatus);
    if (context.e_bandsupport == RF_2G_5G  || context.e_bandsupport == RF_2G)
    {
        context.e_curBand = RF_2G;
        grd_handle_RF_band_cmd(RF_2G);
    }

    dlog_info("Lock id %x %x %x %x %x %x", pu8_rcId[0], pu8_rcId[1], pu8_rcId[2], pu8_rcId[3], pu8_rcId[4], u_grdRcIdSearchStatus.u8_grdRcIdSearchStatus);
}


static void BB_setSearchSkyRcId(void)
{
    u_grdRcIdSearchStatus.stru_rcIdStatus.u8_flagGroundRequestDisconnect    = 1;        //notify the current sky to disconnect
    u_grdRcIdSearchStatus.stru_rcIdStatus.u8_groundSearchMode  = GROUND_IN_RCID_SEARCH;

    BB_WriteReg(PAGE2, GRD_SEARCHING, u_grdRcIdSearchStatus.u8_grdRcIdSearchStatus);
}


static void BB_grd_uartDataHandler(void *p)
{
    uint64_t tmpMaskCode = 0;
    STRU_skyStatusMsg skyStatus;
    uint32_t u32_rcvLen = BB_UARTComReceiveMsg(BB_UART_COM_SESSION_0, (uint8_t *)&skyStatus, sizeof(skyStatus));

    if ( u32_rcvLen >= sizeof(STRU_skyStatusMsg))
    {
        if (SKY_LOCK_STATUS == skyStatus.pid)
        {
            g_stru_skyStatus.u8_rcCrcLockCnt = skyStatus.par.rcLockCnt.u8_rcCrcLockCnt;
            g_stru_skyStatus.u8_rcNrLockCnt  = skyStatus.par.rcLockCnt.u8_rcNrLockCnt;
        }
        else if (RC_MASK_CODE == skyStatus.pid)
        {
            tmpMaskCode = s_u64_rcMask;
            s_u64_rcMask = skyStatus.par.u64_rcMask;

            grd_write_mask_code_to_sky(s_u8_rcMaskEnable, &s_u64_rcMask);
            if (tmpMaskCode != s_u64_rcMask)
            {
                dlog_info("rcv_sky_calc_mask_code:0x%x,0x%x", (uint32_t)((s_u64_rcMask)>>32), (uint32_t)(s_u64_rcMask));
            }
        }
        else if (SKY_AGC_STATUS == skyStatus.pid)
        {
            g_stru_skyStatus.u8_skyagc1 = skyStatus.par.skyAgc.u8_skyagc1;
            g_stru_skyStatus.u8_skyagc2 = skyStatus.par.skyAgc.u8_skyagc2;
        }
        else if(RC_ID_SYNC == skyStatus.pid)
        {
            //dlog_info("Get rc id 0x%x 0x%x 0x%x 0x%x 0x%x",  skyStatus.par.rcId.u8_skyRcIdArray[0], skyStatus.par.rcId.u8_skyRcIdArray[1],
            //                                                 skyStatus.par.rcId.u8_skyRcIdArray[2], skyStatus.par.rcId.u8_skyRcIdArray[3],
            //                                                 skyStatus.par.rcId.u8_skyRcIdArray[4]);
            //if (0 != memcmp( (void *)u8_preRcIdArray, (void *)skyStatus.par.rcId.u8_skyRcIdArray, RC_ID_SIZE))
            {
                //memcpy((void *)u8_preRcIdArray, (void *)skyStatus.par.rcId.u8_skyRcIdArray, RC_ID_SIZE);
                BB_setWriteSkyRcId(skyStatus.par.rcId.u8_skyRcIdArray);

                STRU_SysEvent_DEV_BB_STATUS status;
                status.pid = BB_GET_RCID;
                memcpy((void *)status.rcid, (void *)skyStatus.par.rcId.u8_skyRcIdArray, RC_ID_SIZE);
                SYS_EVENT_Notify(SYS_EVENT_ID_BB_EVENT, (void *)&status);
            }
        }
    }
}


void grd_SetRCId(uint8_t *pu8_id)
{
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT39_32_REG, pu8_id[0]);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT31_24_REG, pu8_id[1]);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT23_16_REG, pu8_id[2]);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT15_08_REG, pu8_id[3]);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT07_00_REG, pu8_id[4]);

    dlog_critical("id[0~4]:0x%x 0x%x 0x%x 0x%x 0x%x", pu8_id[0], pu8_id[1], pu8_id[2], pu8_id[3], pu8_id[4]);
}


void grd_SetSaveRCId(uint8_t *pu8_id)
{
    grd_SetRCId(pu8_id);

    BB_saveRcid(pu8_id);
}


//---------------IT grd hop change--------------------------------

uint32_t it_span_cnt = 0;
void reset_it_span_cnt(void)
{
    it_span_cnt = 0;
}

void grd_fec_judge(void)
{
    if( context.dev_state == INIT_DATA )
    {
        context.locked = grd_is_bb_fec_lock();
        if( !context.locked )
        {
            if (context.fec_unlock_cnt ++ > 20)
            {
                if( context.itHopMode == AUTO )
                {
                    uint8_t mainch, optch;
                    if (BB_selectBestCh(context.e_curBand, SELECT_MAIN_OPT, &mainch, &optch, (uint8_t *)NULL, 0))
                    {
                        context.cur_IT_ch  = mainch;
                        BB_Sweep_updateCh(context.e_curBand, mainch);
                        grd_set_ItFrq(context.e_curBand, mainch);
                        //dlog_info("Select %d %d", mainch, optch);
                        BB_grd_NotifyItFreqByCh(context.e_curBand, context.cur_IT_ch);
                    }
                }
                context.fec_unlock_cnt = 0;
            }
        }
        else
        {
            context.dev_state = CHECK_FEC_LOCK;
        }
    }
    else if(context.dev_state == CHECK_FEC_LOCK || context.dev_state == FEC_LOCK)
    {
        context.locked = grd_is_bb_fec_lock();
        if(context.locked)
        {
            context.dev_state = FEC_LOCK;
            context.fec_unlock_cnt = 0;
        }
        else
        {
            #define     UNLOCK_CNT      (64)
            context.fec_unlock_cnt++;
            if(context.fec_unlock_cnt > UNLOCK_CNT)
            {
                grd_checkBackTo2G();

                if( context.itHopMode == AUTO )
                {
                    uint8_t mainch, optch;
                    BB_selectBestCh(context.e_curBand, SELECT_MAIN_OPT, &mainch, &optch, NULL, 0);
                
                    context.cur_IT_ch  = mainch;
                    BB_Sweep_updateCh(context.e_curBand, mainch);
                    grd_set_ItFrq(context.e_curBand, mainch);
                    BB_grd_NotifyItFreqByCh(context.e_curBand, context.cur_IT_ch);
                    //dlog_info("unlock: select channel %d %d", mainch, optch);
                }

                context.fec_unlock_cnt = 0;

                if(context.qam_skip_mode == AUTO && context.qam_ldpc > context.u8_bbStartMcs)
                {
                    context.qam_ldpc = context.u8_bbStartMcs;
                    grd_set_txmsg_mcs_change(context.e_bandwidth, context.qam_ldpc);
                }
            }
        }
    }
    else if(context.dev_state == DELAY_14MS)
    {
        BB_set_ItFrqByCh(context.e_curBand, context.cur_IT_ch);
        reset_it_span_cnt();
        context.dev_state = CHECK_FEC_LOCK;
        //dlog_info("Hop:%d \n", context.cycle_count);
    }

}



const uint16_t snr_skip_threshold[] = { 0x23,    //bpsk 1/2
                                        0x2d,    //bpsk 1/2
                                        0x6c,    //qpsk 1/2
                                        0x181,   //16QAM 1/2
                                        0x492,   //64QAM 1/2
                                        0x52c};  //64QAM 2/3

int grd_freq_skip_pre_judge(void)
{
    uint8_t flag = 0;
    int16_t aver, fluct;
    uint8_t bestch, optch;    
    
    if ( context.dev_state != FEC_LOCK )
    {
        return 0;
    }
    
    if( it_span_cnt < 16 )
    {
        it_span_cnt ++;
        return 0;
    }

    uint8_t Harqcnt  = ((context.u8_harqcnt_lock & 0xF0) >> 4);
    uint8_t Harqcnt1 = ((BB_ReadReg(PAGE2, 0xd1)& 0xF0) >> 4);
    //if(Harqcnt > 0)
    //{
    //    dlog_info("Harq %d:%d %d snr:%x %x %d ret=%d", Harqcnt, Harqcnt1, context.cycle_count, grd_get_it_snr(), 
    //                                        (((uint16_t)BB_ReadReg(PAGE2, 0xc2)) << 8) | BB_ReadReg(PAGE2, 0xc3),
    //                                        (((uint16_t)BB_ReadReg(PAGE2, 0xd7)) << 8) | BB_ReadReg(PAGE2, 0xd8));
    //}

    if (Harqcnt >= 7 && Harqcnt1 >= 7)      //check LDPC error
    {
        //dlog_info("Harq: %d %d", Harqcnt, Harqcnt1);
        flag = 0;
    }
    else if (Harqcnt >= 3 && Harqcnt1 >= 3) //check snr.
    {
        flag = grd_check_piecewiseSnrPass(1, snr_skip_threshold[context.qam_ldpc]);
        if ( 1 == flag ) //snr pass
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }

    optch = get_opt_channel();
    
    if (BB_CompareCh1Ch2ByPowerAver(context.e_curBand, optch, context.cur_IT_ch, CMP_POWER_AVR_LEVEL))
    {
        if( 0 == flag ) //already Fail
        {
            reset_it_span_cnt( );
            context.cur_IT_ch  = optch;
            BB_Sweep_updateCh(context.e_curBand, context.cur_IT_ch );

            BB_grd_NotifyItFreqByCh(context.e_curBand, context.cur_IT_ch);
            
            //dlog_info("Set Ch0:%d %d %d %x\n",
            //           context.cur_IT_ch, context.cycle_count, ((BB_ReadReg(PAGE2, FEC_5_RD)& 0xF0) >> 4), grd_get_it_snr());
            context.dev_state = DELAY_14MS;
            return 0;
        }
        else
        {
            context.next_IT_ch = optch;
            return 1;  //need to check in the next 1.25ms
        }
    }
    else
    {
        return 0;
    }
}


void grd_freq_skip_post_judge(void)
{
    if(context.dev_state != FEC_LOCK )
    {
        return;
    }

    if ( 0 == grd_check_piecewiseSnrPass( 0, snr_skip_threshold[context.qam_ldpc] ) ) //Fail, need to hop
    {
        if( context.cur_IT_ch != context.next_IT_ch )
        {
            reset_it_span_cnt( );
            context.cur_IT_ch = context.next_IT_ch;
            BB_Sweep_updateCh(context.e_curBand, context.cur_IT_ch );
            BB_grd_NotifyItFreqByCh(context.e_curBand, context.cur_IT_ch);
            //dlog_info("Set Ch1:%d %d %d %x\n", context.cur_IT_ch, context.cycle_count,
            //                                 ((BB_ReadReg(PAGE2, FEC_5_RD)& 0xF0) >> 4), grd_get_it_snr());
        }

        context.dev_state = DELAY_14MS;        
    }
}


uint8_t grd_is_bb_fec_lock(void)
{
    uint8_t data = BB_ReadReg(PAGE2, FEC_5_RD) & 0x01;

    if(u_grdRcIdSearchStatus.stru_rcIdStatus.u8_itLock != data)
    {
        u_grdRcIdSearchStatus.stru_rcIdStatus.u8_itLock = data;
        BB_WriteReg(PAGE2, GRD_SEARCHING, u_grdRcIdSearchStatus.u8_grdRcIdSearchStatus);

        STRU_SysEvent_DEV_BB_STATUS lockEvent = 
        {
            .pid = BB_LOCK_STATUS,
            .lockstatus = data,
        };

        SYS_EVENT_Notify(SYS_EVENT_ID_BB_EVENT, (void *)&lockEvent);
        if (data)
        {
            dlog_warning("LOCK");
        }
        else
        {
            dlog_warning("UNLOCK");
        }        
    }

    return data;
}

//---------------QAM change--------------------------------
ENUM_BB_QAM Grd_get_QAM(void)
{
    static uint8_t iqam = 0xff;

    ENUM_BB_QAM qam = (ENUM_BB_QAM)(BB_ReadReg(PAGE2, GRD_FEC_QAM_CR_TLV) & 0x03);
    if(iqam != qam)
    {
        iqam = qam;
        dlog_info("-QAM:%d ",qam);
    }

    return qam;
}


static void grd_set_txmsg_qam_change(ENUM_BB_QAM qam, ENUM_CH_BW bw, ENUM_BB_LDPC ldpc)
{
    uint8_t data = (qam << 6) | (bw << 3) | ldpc;
    dlog_info("MCS1=>%d", data);

    BB_WriteReg(PAGE2, QAM_CHANGE_0, data);
}


///////////////////////////////////////////////////////////////////////////////////

static void wimax_vsoc_tx_isr(uint32_t u32_vectorNum)
{
    INTR_NVIC_DisableIRQ(BB_TX_ENABLE_VECTOR_NUM);
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);
    STRU_DEVICE_INFO *pst_devInfo      = (STRU_DEVICE_INFO *)(DEVICE_INFO_SHM_ADDR);

    if( context.u8_flagdebugRequest & 0x80)
    {
        context.u8_debugMode = context.u8_flagdebugRequest & 0x01;

        if( context.u8_debugMode != FALSE )
        {
            grd_handle_RC_mode_cmd(MANUAL);
            grd_handle_MCS_mode_cmd(MANUAL);
            grd_handle_IT_mode_cmd(MANUAL);

            osdptr->head = 0x00;
            osdptr->tail = 0xff;    //end of the writing
        }

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

        context.u8_flagdebugRequest = 0;
        dlog_info("DebugMode %d %d\n", osdptr->in_debug, context.u8_debugMode);
    }

    {
        TIM_StartTimer(grd_timer2_6);
        INTR_NVIC_EnableIRQ(TIMER_INTR26_VECTOR_NUM);
    }
    context.cycle_count ++;

    BB_WrSpiChkFlag();
}


void Grd_TIM2_6_IRQHandler(uint32_t u32_vectorNum)
{
    Reg_Read32(BASE_ADDR_TIMER2 + TMRNEOI_6);

    if( context.flag_mrc == 1 )
    {
        context.flag_mrc = 2;
        BB_WriteRegMask(PAGE1, 0x83, 0x01, 0x01); 
    }
    else if( context.flag_mrc == 2 )
    {
        context.flag_mrc = 0;
        BB_WriteRegMask(PAGE1, 0x83, 0x00, 0x01);
    }

    //Enable BB_TX intr
    INTR_NVIC_ClearPendingIRQ(BB_TX_ENABLE_VECTOR_NUM); //clear pending after TX Enable is LOW. MUST
    INTR_NVIC_EnableIRQ(BB_TX_ENABLE_VECTOR_NUM);

    //Disable TIM0 intr
    INTR_NVIC_DisableIRQ(TIMER_INTR26_VECTOR_NUM);
    TIM_StopTimer(grd_timer2_6);
    
    //Enable TIM1 intr
    TIM_StartTimer(grd_timer2_7);
    INTR_NVIC_EnableIRQ(TIMER_INTR27_VECTOR_NUM);

    if ( FALSE == context.u8_debugMode )
    {
        grd_handle_all_cmds();
        //grd_CheckSdramBuffer();
    }

    Timer1_Delay1_Cnt = 0;
    //
    BB_UARTComCycleMsgProcess();
    BB_UARTComCycleSendMsg();
}


void Grd_TIM2_7_IRQHandler(uint32_t u32_vectorNum)
{
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);
    Reg_Read32(BASE_ADDR_TIMER2 + TMRNEOI_7); //disable the intr.

    if ( context.u8_debugMode )
    {
        INTR_NVIC_DisableIRQ(TIMER_INTR27_VECTOR_NUM);                
        TIM_StopTimer(grd_timer2_7);    
        return;
    }

    switch (Timer1_Delay1_Cnt)
    {
        case 0:
            BB_GetSweepedChResult( 0 );
            Timer1_Delay1_Cnt++;
            break;

        case 1:
            grd_fec_judge();
            Timer1_Delay1_Cnt++;
            /*if ( context.locked )
            {
                grd_cal_frqOffset(context.cur_IT_ch, context.e_curBand);
            }*/
            break;

        case 2:
            if(context.rcHopMode == AUTO)
            {
                grd_rc_hopfreq();
            }
            if ( context.locked )
            {
                grd_checkBlockMode();
            }

            if ( context.flag_updateRc > 0 )
            {
                if ( context.flag_updateRc ++ > 100 )
                {
                    context.flag_updateRc = 0;
                    if( context.u32_rcValue )
                    {
                        BB_write_RcRegs( context.u32_rcValue );
                        context.rcHopMode = MANUAL;
                    }
                    else //if context.u32_rcValue ==0, means the auto mode
                    {
                        context.rcHopMode = AUTO;
                    }
                }
            }

            Timer1_Delay1_Cnt++;
            break;

        case 3:
            Timer1_Delay1_Cnt++;
            BB_GetDevInfo();
            grd_judge_qam_mode();
            uint8_t u8_mainCh, u8_optCh;
            if (1 == grd_doRfbandChange( &u8_mainCh, &u8_optCh) )
            {
                context.cur_IT_ch  = u8_mainCh;

                grd_rc_channel = 0;
                grd_rc_hopfreq();

                BB_set_ItFrqByCh( context.e_curBand, context.stru_bandChange.u8_ItCh );
                BB_grd_NotifyItFreqByCh( context.e_curBand, context.stru_bandChange.u8_ItCh );
            }
            break;

        case 4:
            Timer1_Delay1_Cnt++;
            if ((context.dev_state == FEC_LOCK) && (context.locked))
            {
                s_st_calcDistData.u32_lockCnt += 1;
                if (s_st_calcDistData.u32_lockCnt >= 3)
                {
                    s_st_calcDistData.u32_lockCnt = 3;
                    grd_calc_dist();
                }
                
            }
            else
            {
                s_st_calcDistData.u32_lockCnt = 0;
            }
            break;

        case 5:
            Timer1_Delay1_Cnt++;
            BB_grd_OSDPlot();
            break;

        case 6:
            Timer1_Delay1_Cnt++;
            context.u8_harqcnt_lock = BB_ReadReg(PAGE2, FEC_5_RD);
            if(context.itHopMode == AUTO && context.locked )
            {
                flag_snrPostCheck = grd_freq_skip_pre_judge( );
            }
            else
            {
                flag_snrPostCheck = 0;
            }
            break;

        case 7:
            Timer1_Delay1_Cnt++;
            INTR_NVIC_DisableIRQ(TIMER_INTR27_VECTOR_NUM);
            TIM_StopTimer(grd_timer2_7);
            if( flag_snrPostCheck )
            {
                grd_freq_skip_post_judge( );
            }

            break;

        default:
            Timer1_Delay1_Cnt = 0;
            break;
    }
}

static void Grd_Timer2_7_Init(void)
{
    grd_timer2_7.base_time_group = 2;
    grd_timer2_7.time_num = 7;
    grd_timer2_7.ctrl = 0;
    grd_timer2_7.ctrl |= TIME_ENABLE | USER_DEFINED;
    TIM_RegisterTimer(grd_timer2_7, 1250); //1.25ms
    reg_IrqHandle(TIMER_INTR27_VECTOR_NUM, Grd_TIM2_7_IRQHandler, NULL);
    INTR_NVIC_SetIRQPriority(TIMER_INTR27_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_TIMER01,0));
}

void Grd_Timer2_6_Init(void)
{
    grd_timer2_6.base_time_group = 2;
    grd_timer2_6.time_num = 6;
    grd_timer2_6.ctrl = 0;
    grd_timer2_6.ctrl |= TIME_ENABLE | USER_DEFINED;
    
    TIM_RegisterTimer(grd_timer2_6, 3500); //2.5s
    reg_IrqHandle(TIMER_INTR26_VECTOR_NUM, Grd_TIM2_6_IRQHandler, NULL);
    INTR_NVIC_SetIRQPriority(TIMER_INTR26_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_TIMER00,0));
}


//=====================================Grd RC funcions =====
void grd_rc_hopfreq(void)
{
	uint8_t max_ch_size = ((context.e_curBand == RF_2G) || (context.e_curBand == RF_600M)) ? MAX_2G_RC_FRQ_SIZE : MAX_5G_RC_FRQ_SIZE;

    grd_rc_channel++;
    if(grd_rc_channel >= max_ch_size)
    {
        grd_rc_channel = 0;
    }

    BB_WriteReg(PAGE2, GRD_RC_CHANNEL, grd_rc_channel);
    
    if (s_u8_rcMaskEnable)
    {
        if ( !(s_u64_rcMask & (((uint64_t)1) << grd_rc_channel)) )
        {
            BB_set_Rcfrq(context.e_curBand, grd_rc_channel);
        }
    }
    else
    {
        BB_set_Rcfrq(context.e_curBand, grd_rc_channel);
    }
}


///////////////////////////////////////////////////////////////////////////////////

/*
 * ground get the sky IT QAM mode
*/
ENUM_BB_QAM grd_get_IT_QAM(void)
{
    return (ENUM_BB_QAM)(BB_ReadReg(PAGE2, GRD_FEC_QAM_CR_TLV) & 0x03);
}

/*
 * ground get the sky IT LDPC mode
*/
ENUM_BB_LDPC grd_get_IT_LDPC(void)
{
    return (ENUM_BB_LDPC)( (BB_ReadReg(PAGE2, GRD_FEC_QAM_CR_TLV) >>2) & 0x07);
}

static void grd_handle_IT_mode_cmd(ENUM_RUN_MODE mode)
{
    context.itHopMode = mode;
    dlog_info("mode= %d ", mode);
}


/*
  *   only set value to context, the request will be handle in 14ms interrupt.
 */
static void grd_handle_IT_CH_cmd(uint8_t ch)
{
    context.it_manual_ch = ch;
    dlog_info("ch= %d", ch);    
}


/*
  * mode: set RC to auto or Manual
  * ch: the requested channel from  
  * 
 */
static void grd_handle_RC_mode_cmd(ENUM_RUN_MODE mode)
{
    if ( mode == AUTO )
    {
        context.flag_updateRc = 1;
        context.u32_rcValue = 0;

        BB_WriteReg(PAGE2, RC_CH_MODE_0, AUTO);
        dlog_info("Set RC auto");
    }
}


static void grd_handle_RC_CH_cmd(uint8_t ch)
{
    BB_set_Rcfrq(context.e_curBand, ch);

    BB_WriteReg(PAGE2, RC_CH_CHANGE_0, 0x80+ch);

    dlog_info("ch =%d", ch);
}

/*
 * switch between 2.5G and 5G.
 */
static void grd_handle_RF_band_cmd(ENUM_RF_BAND rf_band)
{
    if(context.e_curBand != rf_band)
    {
        grd_notifyRfbandChange(rf_band, 0, 1, BAND_CHANGE_DELAY);

        dlog_info("To rf_band %d %d", context.e_curBand, rf_band);
    }    
}


/*
 *  handle command for 10M, 20M
*/
static void grd_handle_CH_bandwitdh_cmd(ENUM_CH_BW bw)
{
    if(context.e_bandwidth != bw)
    {
        BB_set_RF_bandwitdh(BB_GRD_MODE, bw);

        BB_WriteReg(PAGE2, RF_CH_BW_CHANGE_0, 0xc0 | (uint8_t)bw);      

        context.e_bandwidth = bw; 
    }

    dlog_info("e_bandwidth =%d", context.e_bandwidth);    
}

static void grd_handle_CH_qam_cmd(ENUM_BB_QAM qam)
{
    //set and soft-rest
    if(context.qam_mode != qam)
    {
        BB_WriteReg(PAGE2, RF_CH_QAM_CHANGE_0, 0xc0 | (uint8_t)qam);      

        context.qam_mode = qam; 
        context.ldpc = grd_get_IT_LDPC();   
        grd_set_mcs_registers(context.qam_mode, context.ldpc, context.e_bandwidth);
    }

    dlog_info("CH_QAM =%d", context.qam_mode);    
}

void grd_handle_CH_ldpc_cmd(ENUM_BB_LDPC e_ldpc)
{
    if(context.ldpc != e_ldpc)
    {
        BB_WriteReg(PAGE2, RF_CH_LDPC_CHANGE_0, e_ldpc);
        context.ldpc = e_ldpc;
        context.qam_mode = grd_get_IT_QAM();

        grd_set_mcs_registers(context.qam_mode, context.ldpc, context.e_bandwidth);
    }
    dlog_info("CH_LDPC =%d", context.ldpc);
}

        
static void grd_handle_MCS_mode_cmd(ENUM_RUN_MODE mode)
{
    context.qam_skip_mode = mode;
    dlog_info("qam_skip_mode = %d\n", context.qam_skip_mode);
    
    if ( mode == AUTO )
    {
        ENUM_BB_QAM qam = grd_get_IT_QAM();
        ENUM_BB_LDPC ldpc = grd_get_IT_LDPC();

        if( qam == MOD_BPSK )
        {
            context.qam_ldpc = 1;
        }
        else if( qam == MOD_4QAM )
        {
            context.qam_ldpc = 2;
        }
        else if( qam == MOD_16QAM )
        {
            context.qam_ldpc = 3;
        }
        else if( qam == MOD_64QAM && ldpc == LDPC_1_2)
        {
            context.qam_ldpc = 4;
        }
        else if( qam == MOD_64QAM)
        {
            context.qam_ldpc = 5;
        }

        grd_set_txmsg_mcs_change(context.e_bandwidth, context.qam_ldpc);        
    }
}


/*
  * handle set MCS mode: QAM, LDPC, encoder rate
*/
static void grd_handle_MCS_cmd(ENUM_BB_QAM qam, ENUM_BB_LDPC ldpc)
{
    grd_set_txmsg_qam_change(qam, context.e_bandwidth, ldpc);
    dlog_info("qam, ldpc =%d %d", qam, ldpc);
}


/*
 * handle H264 encoder brc 
*/
static void grd_handle_brc_mode_cmd(ENUM_RUN_MODE mode)
{
    context.brc_mode = mode;

    BB_WriteReg(PAGE2, ENCODER_BRC_MODE_0, 0xe0+mode);

    dlog_info("brc mode =%d", mode);
}


/*
  * handle H264 encoder brc 
 */
static void grd_handle_brc_bitrate_cmd(uint8_t u8_ch, uint8_t brc_coderate)
{
    if (0 == u8_ch)
    {
        BB_WriteReg(PAGE2, ENCODER_BRC_CHAGE_0_CH1, (0xc0 | brc_coderate));
        context.brc_bps[0] = brc_coderate;

        dlog_info("brc_coderate_ch1 = %d ", brc_coderate);
    }
    else
    {
        BB_WriteReg(PAGE2, ENCODER_BRC_CHAGE_0_CH2, (0xc0 | brc_coderate));
        context.brc_bps[1] = brc_coderate;

        dlog_info("brc_coderate_ch2 = %d ", brc_coderate);
    }
}


void BB_grd_notify_rc_skip_freq(uint32_t u32_rcfrq)
{
    BB_WriteReg(PAGE2, RC_CH_MODE_0, MANUAL);

    BB_WriteReg(PAGE2, RC_FRQ_0, (u32_rcfrq >> 24 ) & 0xff);
    BB_WriteReg(PAGE2, RC_FRQ_1, (u32_rcfrq >> 16 ) & 0xff);
    BB_WriteReg(PAGE2, RC_FRQ_2, (u32_rcfrq >>  8 ) & 0xff);
    BB_WriteReg(PAGE2, RC_FRQ_3, (u32_rcfrq) & 0xff);
}



void BB_grd_handleRcSearchCmd(STRU_WIRELESS_CONFIG_CHANGE* pcmd)
{
    uint8_t class   = pcmd->u8_configClass;
    uint8_t item    = pcmd->u8_configItem;
    uint32_t value  = pcmd->u32_configValue;
    uint32_t value1 = pcmd->u32_configValue1;

    uint8_t u8_rcArray[5] = {
                        (value>>24)&0xff, (value>>16)&0xff, (value>>8)&0xff, value&0xff, 
                        (value1&0xff)
                      };

    dlog_info("%d %0.8x %0.2x", item, value, value1);

    if (WIRELESS_AUTO_SEARCH_ID == class)
    {
        switch (item)
        {
            case RCID_DISCONNECT:
                BB_setDisconnectRcId(u8_rcArray);
                break;

            case RCID_CONNECT_ID:
                BB_setConnectRcId(u8_rcArray);
                break;

            case RCID_AUTO_SEARCH:
                BB_setSearchSkyRcId();
                break;

            case RCID_SAVE_RCID:
                grd_SetSaveRCId(u8_rcArray);
                break;
    
           default:
                dlog_warning("error item=%d", item);
                break;
        }
    }
}

void grd_handle_one_cmd(STRU_WIRELESS_CONFIG_CHANGE* pcmd)
{
    uint8_t class  = pcmd->u8_configClass;
    uint8_t item   = pcmd->u8_configItem;
    uint32_t value = pcmd->u32_configValue;

    //dlog_info("class item value %d %d 0x%0.8x ", class, item, value);
    if(class == WIRELESS_FREQ_CHANGE)
    {
        switch(item)
        {
            case FREQ_BAND_MODE:
            {
                context.e_rfbandMode = (ENUM_RUN_MODE)value;
                break;
            }

            case FREQ_BAND_SELECT:
            {
                grd_handle_RF_band_cmd((ENUM_RF_BAND)value);
                break;
            }

            case FREQ_CHANNEL_MODE: //auto manual
            {
                grd_handle_IT_mode_cmd((ENUM_RUN_MODE)value);
                break;
            }
            
            case FREQ_CHANNEL_SELECT:
            {
                grd_handle_IT_CH_cmd((uint8_t)value);
                break;
            }

            case RC_CHANNEL_MODE:
            {
                grd_handle_RC_mode_cmd( (ENUM_RUN_MODE)value);
                break;
            }

            case RC_CHANNEL_SELECT:
            {
                grd_handle_RC_mode_cmd( (ENUM_RUN_MODE)MANUAL);
                grd_handle_RC_CH_cmd((uint8_t)value);
                break;
            }

            case RC_CHANNEL_FREQ:
            {
                grd_handle_RC_mode_cmd( (ENUM_RUN_MODE)MANUAL );
                BB_grd_notify_rc_skip_freq( value );    //set to manual and set the regigsters
                context.flag_updateRc = 1;              //to set the remote control
                context.u32_rcValue   = value;
                dlog_info("set RC FREQ %x", value);
                break;
            }

            case IT_CHANNEL_FREQ:
            {
                grd_handle_IT_mode_cmd( (ENUM_RUN_MODE)MANUAL);
                BB_write_ItRegs( value );
                BB_grd_NotifyItFreqByValue( value );

                dlog_info("IT_CHANNEL_FREQ 0x%0.8x ", value);                
                break;
            }

            case FREQ_BAND_WIDTH_SELECT:
            {
                grd_handle_CH_bandwitdh_cmd((ENUM_CH_BW)value);
                dlog_info("FREQ_BAND_WIDTH_SELECT %x", value);                
                break;
            }

                
            default:
            {
                dlog_warning("%s", "unknown WIRELESS_FREQ_CHANGE command");
                break;
            }
        }
    }

    if (class == WIRELESS_AUTO_SEARCH_ID)
    {
        BB_grd_handleRcSearchCmd(pcmd);
    }

    if(class == WIRELESS_MCS_CHANGE)
    {
        switch(item)
        {
            case MCS_MODE_SELECT:
            {
                grd_handle_brc_mode_cmd( (ENUM_RUN_MODE)value);
                grd_handle_MCS_mode_cmd( (ENUM_RUN_MODE)value);
                
                //For osd information
                if ( context.brc_mode == MANUAL)
                {
                    context.brc_bps[0] = BB_get_bitrateByMcs(context.e_bandwidth, context.qam_ldpc);
                    context.brc_bps[1] = BB_get_bitrateByMcs(context.e_bandwidth, context.qam_ldpc);                
                }
            }
    
            break;

            case MCS_MODULATION_SELECT:
            {
                    context.qam_ldpc = value;
                    grd_set_txmsg_mcs_change(context.e_bandwidth, value);
            }
            break;

            case MCS_CODE_RATE_SELECT:
            {
                //grd_set_txmsg_ldpc(value);
                break;
            }
            case MCS_IT_QAM_SELECT:
            {
                grd_handle_CH_qam_cmd((ENUM_BB_QAM)value);
                dlog_info("MCS_IT_QAM_SELECT %x", value);                
                break;
            }
            
            case MCS_IT_CODE_RATE_SELECT:
            {
                grd_handle_CH_ldpc_cmd((ENUM_BB_LDPC)value);
                dlog_info("MCS_IT_CODE_RATE_SELECT %x", value);  
                break;              
            }
                
            case MCS_RC_QAM_SELECT:
            {
                context.rc_qam_mode = (ENUM_BB_QAM)(value & 0x01);
                BB_set_QAM(context.rc_qam_mode);
                dlog_info("MCS_RC_QAM_SELECT %x", value);                
                break;
            }
            
            case MCS_RC_CODE_RATE_SELECT:
            {
                context.rc_ldpc = (ENUM_BB_LDPC)(value & 0x01);
                BB_set_LDPC(context.rc_ldpc);
                dlog_info("MCS_RC_CODE_RATE_SELECT %x", value); 
                break;               
            }
            case MCS_CHG_RC_RATE:
            {
                grd_RcChgRate.cnt = 20;
                grd_RcChgRate.flag = 1;
                grd_RcChgRate.par = value;
                BB_WriteReg(PAGE2, RC_RATE, value);
                dlog_info("MCS_CHG_RC_RATE %d", value); 
                break;               
            }
            default:
                //dlog_warning("%s", "unknown WIRELESS_MCS_CHANGE command");
                break;
        }        
    }

    if(class == WIRELESS_ENCODER_CHANGE)
    {
        switch(item)
        {
            case ENCODER_DYNAMIC_BIT_RATE_MODE:
                grd_handle_brc_mode_cmd( (ENUM_RUN_MODE)value);
                break;

            case ENCODER_DYNAMIC_BIT_RATE_SELECT_CH1:
                grd_handle_brc_bitrate_cmd(0, (uint8_t)value);
                dlog_info("ch1:%d",value);
                break;

            case ENCODER_DYNAMIC_BIT_RATE_SELECT_CH2:
                grd_handle_brc_bitrate_cmd(1, (uint8_t)value);
                dlog_info("ch2:%d",value);
                break;

            default:
                //dlog_warning("%s", "unknown WIRELESS_ENCODER_CHANGE command");
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
                BB_SwtichOnOffCh(0, (uint8_t)value);
                break;
            }

            case SWITCH_ON_OFF_CH2:
            {
                BB_SwtichOnOffCh(1, (uint8_t)value);
                break;
            }

            case BB_SOFT_RESET:
            {
                dlog_info("grd bb reset.");
                BB_softReset(BB_GRD_MODE);
                break;
            }

            case CALC_DIST_ZERO_CALI:
            {
                //dlog_info("calculation distance zero calibration.");
                grd_calc_dist_zero_calibration();
                break;
            }

            case SET_CALC_DIST_ZERO_POINT:
            {
                //dlog_info("set distance zero point.");
                grd_set_calc_dist_zero_point(value);
                break;
            }

            case SET_RC_FRQ_MASK:
            {
                grd_disable_enable_rc_frq_mask_func(value);
                break;
            }
            default:
                break;                
        }
    }
}


static void grd_handle_all_cmds(void)
{   
    int ret = 0;
    int cnt = 0;
    STRU_WIRELESS_CONFIG_CHANGE cfg;
    while( (cnt++ < 5) && (1 == BB_GetCmd(&cfg)))
    {
        grd_handle_one_cmd( &cfg );
    }

    if (grd_RcChgRate.cnt > 0)
    {
        grd_RcChgRate.cnt -= 1;
    }

    if ((1 == grd_RcChgRate.flag) && (0 == grd_RcChgRate.cnt))
    {
        grd_ChgRcRate((uint8_t)grd_RcChgRate.par);
        memset((uint8_t *)(&grd_RcChgRate), 0x00, sizeof(STRU_DELAY_CMD));
    }
}

/*
 *
*/
static void BB_grd_OSDPlot(void)
{
    uint8_t u8_data;
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);

    if (osdptr->osd_enable == 0)
    {
        return;
    }

    osdptr->messageId    = 0x33;
    osdptr->head         = 0xff; //starting writing
    osdptr->tail         = 0x00;

    osdptr->agc_value[0] = context.u8_agc[0];
    osdptr->agc_value[1] = context.u8_agc[1];
    /*
    osdptr->agc_value[2] = BB_ReadReg(PAGE2, RX3_GAIN_ALL_R);
    osdptr->agc_value[3] = BB_ReadReg(PAGE2, RX4_GAIN_ALL_R);
    */

    osdptr->lock_status  = context.u8_harqcnt_lock;
    
    osdptr->snr_vlaue[0] = grd_get_it_snr();
    osdptr->snr_vlaue[1] = get_snr_average();

    //masoic
    osdptr->u16_afterErr = (((uint16_t)BB_ReadReg(PAGE2, LDPC_ERR_AFTER_HARQ_HIGH_8)) << 8) | BB_ReadReg(PAGE2, LDPC_ERR_AFTER_HARQ_LOW_8);
    osdptr->ldpc_error   = (((uint16_t)BB_ReadReg(PAGE2, LDPC_ERR_HIGH_8)) << 8) | BB_ReadReg(PAGE2, LDPC_ERR_LOW_8); //1byte is enough
    osdptr->harq_count   = (context.u8_harqcnt_lock & 0xF0) | ((BB_ReadReg(PAGE2, 0xd1)& 0xF0) >> 4);

    osdptr->u8_mcs          = context.qam_ldpc;
    osdptr->modulation_mode = grd_get_IT_QAM();
    osdptr->code_rate       = grd_get_IT_LDPC();

    u8_data = BB_ReadReg(PAGE2, TX_2);
    osdptr->rc_modulation_mode = (u8_data >> 6) & 0x01;
    osdptr->rc_code_rate       = (u8_data >> 0) & 0x01;
    osdptr->e_bandwidth        = context.e_bandwidth;         
    osdptr->in_debug           = context.u8_debugMode;

    memset(osdptr->sweep_energy, 0, sizeof(osdptr->sweep_energy));
    BB_GetSweepNoise(osdptr->sweep_energy);

    if(context.brc_mode == AUTO)
    {
        osdptr->encoder_bitrate[0] = BB_get_bitrateByMcs(context.e_bandwidth, context.qam_ldpc);
        osdptr->encoder_bitrate[1] = BB_get_bitrateByMcs(context.e_bandwidth, context.qam_ldpc);
    }
    else
    {
        osdptr->encoder_bitrate[0] = context.brc_bps[0];
        osdptr->encoder_bitrate[1] = context.brc_bps[1];
    }

    osdptr->u8_rclock =  g_stru_skyStatus.u8_rcCrcLockCnt;
    osdptr->u8_nrlock =  g_stru_skyStatus.u8_rcNrLockCnt;

    osdptr->sky_agc[0]=  g_stru_skyStatus.u8_skyagc1;
    osdptr->sky_agc[1]=  g_stru_skyStatus.u8_skyagc2;

    osdptr->dist_zero = (uint16_t)(s_st_calcDistData.u32_calcDistZero);
    osdptr->dist_value= (uint16_t)(s_st_calcDistData.u32_calcDistValue);

    osdptr->head = 0x00;
    osdptr->tail = 0xff;    //end of the writing
}

static void grd_set_ItFrq(ENUM_RF_BAND e_band, uint8_t ch)
{
    BB_set_ItFrqByCh(e_band, ch);
    context.flag_mrc = 1;
}

static void grd_calc_dist(void)
{
    uint8_t u8_data2[3];
    uint32_t u32_data2 = 0;
    uint32_t u32_i;
    static uint32_t u32_cnt = 1;
    uint32_t cmpData;
    static uint32_t cmpCnt = 0;

    u8_data2[2] = BB_ReadReg(PAGE3, 0xA7);
    u8_data2[1] = BB_ReadReg(PAGE3, 0xA6);
    u8_data2[0] = BB_ReadReg(PAGE3, 0xA5);

    cmpData = (u8_data2[0] << 16) | (u8_data2[1] << 8) | (u8_data2[2] << 0);

    switch(s_st_calcDistData.e_status)
    {
        case INVALID:
        {
            s_st_calcDistData.e_status = CALC_DIST_PREPARE;
            s_st_calcDistData.u32_cnt = 0;
            s_st_calcDistData.u32_calcDistZero = ((BW_20M == (context.e_bandwidth)) ? (DEFAULT_DIST_ZERO_20M) : (DEFAULT_DIST_ZERO_10M));
            break;
        }
        case CALI_ZERO:
        {
            memcpy(s_st_calcDistData.u8_rawData[s_st_calcDistData.u32_cnt % CALC_DIST_RAW_DATA_MAX_RECORD], u8_data2, 3);
            s_st_calcDistData.u32_cnt += 1;
            if (s_st_calcDistData.u32_cnt >= (CALC_DIST_RAW_DATA_MAX_RECORD))
            {
                s_st_calcDistData.u32_calcDistZero = grd_calc_dist_get_avg_value(&u32_data2);
                s_st_calcDistData.u32_cnt = 0;
                s_st_calcDistData.e_status = CALC_DIST_PREPARE;
                //dlog_info("CALI_ZERO");
                //dlog_info("Zero:%d", s_st_calcDistData.u32_calcDistZero);
            }
            break;
        }
        case CALC_DIST_PREPARE:
        {
            memcpy(s_st_calcDistData.u8_rawData[s_st_calcDistData.u32_cnt % CALC_DIST_RAW_DATA_MAX_RECORD], u8_data2, 3);
            s_st_calcDistData.u32_cnt += 1;
            if (s_st_calcDistData.u32_cnt >= CALC_DIST_RAW_DATA_MAX_RECORD)
            {
                grd_calc_dist_get_avg_value(&(s_st_calcDistData.u32_calcDistValue));

                s_st_calcDistData.u32_cnt = 0;
                s_st_calcDistData.e_status = CALC_DIST;
                //dlog_info("CALC_DIST_PREPARE");
                //dlog_info("value:%d", s_st_calcDistData.u32_calcDistValue);
            }
            break;
        }
        case CALC_DIST:
        {
            cmpData = abs(cmpData - s_st_calcDistData.u32_calcDistZero) * 3 / 2;
            if (abs(cmpData - s_st_calcDistData.u32_calcDistValue) < 50)
            {
                cmpCnt = 0;
                memcpy(s_st_calcDistData.u8_rawData[s_st_calcDistData.u32_cnt % CALC_DIST_RAW_DATA_MAX_RECORD], u8_data2, 3);
                s_st_calcDistData.u32_cnt += 1;
                
                grd_calc_dist_get_avg_value(&(s_st_calcDistData.u32_calcDistValue));

                u32_cnt += 1;
                if (0 == (u32_cnt % 500))
                {
                    dlog_info("Zero:%d value:%d", s_st_calcDistData.u32_calcDistZero, s_st_calcDistData.u32_calcDistValue);
                } 
            }
            else
            {
                cmpCnt++;
                if (cmpCnt > 5)
                {
                    cmpCnt = 0;
                    s_st_calcDistData.u32_cnt = 0;
                    s_st_calcDistData.e_status = CALC_DIST_PREPARE;
                }
            }
            break;
        }
        default:
        {
            s_st_calcDistData.e_status = CALI_ZERO;
            s_st_calcDistData.u32_cnt = 0;
            break;
        }     
    }
}

static void grd_calc_dist_zero_calibration(void)
{
    s_st_calcDistData.e_status = CALI_ZERO;
    s_st_calcDistData.u32_calcDistValue = 0;
    s_st_calcDistData.u32_calcDistZero = 0;
    s_st_calcDistData.u32_cnt = 0;
    s_st_calcDistData.u32_lockCnt = 0;
}

static void grd_set_calc_dist_zero_point(uint32_t value)
{
    s_st_calcDistData.u32_calcDistZero = value;
}


static uint32_t grd_calc_dist_get_avg_value(uint32_t *u32_dist)
{
    uint32_t u32_data2 = 0;
    uint32_t u32_i;
    
    for(u32_i = 0; u32_i < CALC_DIST_RAW_DATA_MAX_RECORD; u32_i++)
    {
        u32_data2 += ((s_st_calcDistData.u8_rawData[u32_i][0] << 16) | 
                     (s_st_calcDistData.u8_rawData[u32_i][1] << 8) | 
                     (s_st_calcDistData.u8_rawData[u32_i][2] << 0));
    }
    u32_data2 = u32_data2 / CALC_DIST_RAW_DATA_MAX_RECORD;

    if (u32_data2 > s_st_calcDistData.u32_calcDistZero)
    {
        *u32_dist = (u32_data2 - s_st_calcDistData.u32_calcDistZero) * 3 / 2;
    }
    else
    {
        *u32_dist = 0;
    }

    return u32_data2;
}

static void grd_init_rc_frq_mask_func(void)
{
    grd_write_mask_code_to_sky(s_u8_rcMaskEnable, &s_u64_rcMask);
}

static void grd_disable_enable_rc_frq_mask_func(uint8_t flag)
{
    if (flag)
    {
        s_u8_rcMaskEnable = 1;
        dlog_info("enable rc frq mask");
    }
    else
    {
        s_u8_rcMaskEnable = 0;
        dlog_info("disable rc frq mask");
    }
}

static void grd_write_mask_code_to_sky(uint8_t enable, uint64_t *mask)
{
    uint8_t *pu8_tmpAddr;
    uint8_t i;
    uint64_t u64_tmpData = 0;

    if (0 == enable) // disable rc frq mask function
    {
        pu8_tmpAddr = (uint8_t *)(&u64_tmpData);
    }
    else
    {
        pu8_tmpAddr = (uint8_t *)(mask);
    }
    
    for (i=0; i<sizeof(uint64_t); i++)
    {
        BB_WriteReg(PAGE2, GRD_MASK_CODE + i, pu8_tmpAddr[i]);
    }
}



int grd_GetDistAverage(int *pDist)
{
    if ( s_st_calcDistData.e_status == CALC_DIST )
    {
        *pDist = s_st_calcDistData.u32_calcDistValue;  //
        return 1;
    }
    return 0;
}

static void grd_ChgRcRate(uint8_t rate)
{
    if (0 == rate)
    {
        BB_WriteReg(PAGE2, 0x04, 0x38); // BPSK 1/2
    }
    else if(1 == rate)
    {
        BB_WriteReg(PAGE2, 0x04, 0x79); // QPSK 2/3
    }
    else
    {
        ;
    }

    BB_softReset(BB_GRD_MODE);
}


