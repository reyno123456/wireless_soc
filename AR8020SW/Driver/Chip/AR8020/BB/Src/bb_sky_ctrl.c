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
#include "gpio.h"
#include "bb_uart_com.h"

static uint32_t s_lockFlag = 0;

#define MAX_SEARCH_ID_NUM               (5)
#define SEARCH_ID_TIMEOUT               (2000)   //ms
#define	SKY_BACK_TO_2G_UNLOCK_CNT       (7)

typedef struct
{
    uint8_t id[5];
    uint8_t agc1;
    uint8_t agc2;
}SEARCH_IDS;

typedef struct
{
    SEARCH_IDS search_ids[MAX_SEARCH_ID_NUM];
    uint8_t count;
}SEARCH_IDS_LIST;

typedef struct
{
    int64_t  ll_offset;
    int      basefrq;
    int      baseOffset;
    uint8_t  u8_flagOffset;  //offset have got
    uint8_t  u8_softEnable;
    uint32_t u32_cyclecnt;
}STRU_SKY_FREQ_OFFSET;

static SEARCH_IDS_LIST search_id_list;
static uint32_t start_time_cnt = 0;
static uint8_t  sky_rc_channel = 0;

static enum EN_AGC_MODE en_agcmode = UNKOWN_AGC;

static init_timer_st sky_timer2_6;
static init_timer_st sky_timer2_7;
static STRU_RC_FRQ_MASK s_st_rcFrqMask;

static STRU_SKY_FREQ_OFFSET stru_skyfrqOffst;

static void sky_handle_one_cmd(STRU_WIRELESS_CONFIG_CHANGE* pcmd);
static void sky_handle_all_cmds(void);
static int32_t sky_chk_flash_id_validity(void);
static int32_t sky_write_id(uint8_t *u8_idArray);
static int32_t cal_chk_sum(uint8_t *pu8_data, uint32_t u32_len, uint8_t *u8_check);
extern int BB_WriteRegMask(ENUM_REG_PAGES page, uint8_t addr, uint8_t data, uint8_t mask);

static void BB_sky_SendUartData(void *p);
static void sky_calc_dist(void);

static uint16_t sky_get_rc_snr( void );
static uint16_t sky_get_snr_average(void);

static void sky_init_rc_frq_mask_func(void);
static void sky_rc_frq_status_statistics(void);

static void sky_send_session( void );
static void sky_doRfBandChange( void );
void sky_clearFrqOffset(void);
static void sky_backTo2GCheck(void);

void BB_SKY_start(void)
{   
    context.rc_skip_freq_mode = AUTO;

    context.cur_IT_ch         = 0xff;
    context.rc_unlock_cnt     = 0;
    context.dev_state         = SEARCH_ID;

    sky_id_search_init();
    
    GPIO_SetMode(RED_LED_GPIO, GPIO_MODE_2);
    GPIO_SetPinDirect(RED_LED_GPIO, GPIO_DATA_DIRECT_OUTPUT);

    GPIO_SetMode(BLUE_LED_GPIO, GPIO_MODE_2);
    GPIO_SetPinDirect(BLUE_LED_GPIO, GPIO_DATA_DIRECT_OUTPUT);
    
    GPIO_SetPin(RED_LED_GPIO, 0);   //RED LED ON
    GPIO_SetPin(BLUE_LED_GPIO, 1);  //BLUE LED OFF

    sky_Timer2_6_Init();
    sky_Timer2_7_Init();
    sky_search_id_timeout_irq_enable(); //enabole TIM1 timeout

    reg_IrqHandle(BB_RX_ENABLE_VECTOR_NUM, wimax_vsoc_rx_isr, NULL);
    INTR_NVIC_SetIRQPriority(BB_RX_ENABLE_VECTOR_NUM, INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_BB_RX,0));
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);

    context.qam_ldpc = context.u8_bbStartMcs;
    sky_set_McsByIndex(context.CH_bandwidth, context.qam_ldpc);

    BB_UARTComInit( NULL ); 

    BB_GetDevInfo();
    s_lockFlag = 0;

    sky_init_rc_frq_mask_func();
}


uint8_t sky_id_match(void)
{
    static int total_count = 0;
    static int lock_count = 0;
    static int nr_lock    = 0;

    static uint8_t pre_nrlockcnt = 0;
    static uint8_t pre_lockcnt = 0;

    uint8_t data = BB_ReadReg(PAGE2, FEC_4_RD);

    total_count ++;
    lock_count += (( (data  & 0x03 )== 0x03) ? 1 : 0);
    nr_lock    += (  (data & 0x04) ? 1 : 0);

    if(total_count > 1000)
    {   
        dlog_info("-L:%d-%d-", lock_count, nr_lock);
        pre_nrlockcnt = nr_lock;
        pre_lockcnt   = lock_count;
        total_count   = 0;
        lock_count    = 0;
        nr_lock       = 0;

        {
            STRU_SysEventSkyStatus stru_skycStatus = {
                .pid             = SKY_LOCK_STATUS,
                .par.rcLockCnt.u8_rcCrcLockCnt = pre_lockcnt,
                .par.rcLockCnt.u8_rcNrLockCnt  = pre_nrlockcnt,
            };

            //BB_UARTComSendMsg( BB_UART_COM_SESSION_0, (uint8_t *)&stru_skycStatus, sizeof(STRU_SysEventSkyStatus));
        }
    }

    return ( (data & 0x03)==0x03) ? 1 : 0;
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


void sky_agc_gain_toggle(void)
{
    static int loop = 0;
    if(loop++ > 50)
    {
        dlog_info("AGCToggle\r\n");  
        loop = 0; 
    }

    /*if(FAR_AGC == en_agcmode)
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_NEAR);
        BB_WriteReg(PAGE0, AGC_3, AAGC_GAIN_NEAR);

        BB_WriteReg(PAGE0, AGC_5G_GAIN1, AAGC_GAIN_NEAR); //add 5G agc setting
        BB_WriteReg(PAGE0, AGC_5G_GAIN2, AAGC_GAIN_NEAR);
        
        BB_WriteReg(PAGE1, 0x03, 0x40);
        BB_WriteReg(PAGE0, 0xBC, 0x40);
        en_agcmode = NEAR_AGC;
    }
    else*/
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_FAR);
        BB_WriteReg(PAGE0, AGC_3, AAGC_GAIN_FAR);

        BB_WriteReg(PAGE0, AGC_5G_GAIN1, AAGC_GAIN_FAR); //add 5G agc setting
        BB_WriteReg(PAGE0, AGC_5G_GAIN2, AAGC_GAIN_FAR);

        BB_WriteReg(PAGE1, 0x03, 0x28);
        BB_WriteReg(PAGE0, 0xBC, 0xC0);

        en_agcmode = FAR_AGC;
    }
}


uint8_t agc_value1[50];
uint8_t agc_value2[50];
uint8_t agc_idx = 0;

void sky_auto_adjust_agc_gain(void)
{
    uint8_t rx1_gain = BB_ReadReg(PAGE2, AAGC_2_RD);
    uint8_t rx2_gain = BB_ReadReg(PAGE2, AAGC_3_RD);
    uint8_t i = 0;
    
    //average 50 times AGC
    {
        if ( agc_idx >= 50)
        {
            agc_idx = 0;
        }
        agc_value1[agc_idx] = rx1_gain;
        agc_value2[agc_idx] = rx2_gain;
        agc_idx ++;
    }

    {
        uint16_t sum_1 = 0, sum_2 = 0;
        for( i = 0 ; i < 50 ; i++)
        {
            sum_1 += agc_value1[i];
            sum_2 += agc_value2[i];
        }

        rx1_gain = sum_1 / 50;
        rx2_gain = sum_2 / 50;

        {
            static int count1 = 0;
            if ( count1 ++ > 100)
            {
                count1 = 0;
                STRU_SysEventSkyStatus stru_skyAgcStatus = {
                    .pid             = SKY_AGC_STATUS,
                    .par.skyAgc.u8_skyagc1 = rx1_gain,
                    .par.skyAgc.u8_skyagc2 = rx2_gain,
                };

                //BB_UARTComSendMsg( BB_UART_COM_SESSION_0, (uint8_t *)&stru_skyAgcStatus, sizeof(STRU_SysEventSkyStatus));
                //dlog_info("aver: %d %d %x %x", sum_1, sum_2, rx1_gain, rx2_gain);
            }
        }
    }

    if((rx1_gain >= POWER_GATE)&&(rx2_gain >= POWER_GATE) && en_agcmode != FAR_AGC)
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_FAR);
        BB_WriteReg(PAGE0, AGC_3, AAGC_GAIN_FAR);
        en_agcmode = FAR_AGC;
        dlog_info("AGC switch =>F 0x%x 0x%x %d", rx1_gain, rx2_gain, en_agcmode);
    }
     
    if( ((rx1_gain < POWER_GATE)&&(rx2_gain < POWER_GATE)) \
        && (rx1_gain > 0x00) && (rx2_gain >0x00) \
        && en_agcmode != NEAR_AGC)
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_NEAR);
        BB_WriteReg(PAGE0, AGC_3, AAGC_GAIN_NEAR);
        en_agcmode = NEAR_AGC;
        dlog_info("AGC switch =>N 0x%x 0x%x %d", rx1_gain, rx2_gain, en_agcmode);
    }
}


//*********************TX RX initial(14ms irq)**************
void wimax_vsoc_rx_isr(uint32_t u32_vectorNum)
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
        
        osdptr->in_debug = context.u8_debugMode;
        pst_devInfo->isDebug = context.u8_debugMode;        
    }

    INTR_NVIC_EnableIRQ(TIMER_INTR26_VECTOR_NUM);
    TIM_StartTimer(sky_timer2_6);
}


void Sky_TIM2_6_IRQHandler(uint32_t u32_vectorNum)
{
    static uint32_t u32_cnt = 0;
    sky_search_id_timeout_irq_disable();

    Reg_Read32(BASE_ADDR_TIMER2 + TMRNEOI_6);
    INTR_NVIC_ClearPendingIRQ(BB_RX_ENABLE_VECTOR_NUM); //clear pending after TX Enable is LOW. MUST!
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);  
    
    INTR_NVIC_DisableIRQ(TIMER_INTR26_VECTOR_NUM);
    TIM_StopTimer(sky_timer2_6);
    
    if( context.u8_debugMode )
    {
        return;
    }
    sky_get_rc_snr();
    sky_calc_dist();
    

    sky_physical_link_process();

    sky_handle_all_cmds();
    sky_rc_frq_status_statistics();

    //count == 0, then change RF band
    sky_doRfBandChange();

    if (0 == (u32_cnt++ % 2))
    {
        BB_sky_GatherOSDInfo();
    }
    else
    {
        BB_GetDevInfo();
    }

    //
    BB_UARTComCycleMsgProcess();
    BB_UARTComCycleSendMsg();
}


void sky_rc_hopfreq(void)
{
    uint8_t max_ch_size = (context.e_curBand == RF_2G) ? (MAX_2G_RC_FRQ_SIZE):(MAX_5G_RC_FRQ_SIZE);
    
    sky_rc_channel++;
    if(sky_rc_channel >=  max_ch_size)
    {
        sky_rc_channel = 0;
    }

    if ( !((s_st_rcFrqMask.u64_rcvGrdMask) & (((uint64_t)1) << sky_rc_channel)) )
    {
        BB_set_Rcfrq(context.e_curBand, sky_rc_channel);
/*        
        if ( stru_skyfrqOffst.u8_flagOffset && context.locked == 1)
        {
            int curRcfrq;
            int fraction, tracking, integer;
#if 1
            curRcfrq = ((context.e_curBand == RF_2G)? 2410 : 5750) + (sky_rc_channel * 2);
#else
            curRcfrq = ((context.e_curBand == RF_2G)? 2410 : 5750);
#endif
            int64_t sw_frqOffset = -stru_skyfrqOffst.ll_offset * curRcfrq / stru_skyfrqOffst.basefrq - stru_skyfrqOffst.baseOffset;

            integer  = BB_ReadReg(PAGE2, INTEGER_OFFSET);
            
            fraction = (BB_ReadReg(PAGE2, FRACTION_OFFSET_0) << 24) | (BB_ReadReg(PAGE2, FRACTION_OFFSET_1) << 16) |
                       (BB_ReadReg(PAGE2, FRACTION_OFFSET_2) <<  8) | (BB_ReadReg(PAGE2, FRACTION_OFFSET_3));
                       
            tracking = (BB_ReadReg(PAGE2, TRACK_OFFSET_0) << 24) | (BB_ReadReg(PAGE2, TRACK_OFFSET_1) << 16) |
                       (BB_ReadReg(PAGE2, TRACK_OFFSET_2) <<  8) | (BB_ReadReg(PAGE2, TRACK_OFFSET_3));

            int curoffset = fraction + tracking - (integer << 19) ;
            dlog_info("i f t: %d %d %d %d", stru_skyfrqOffst.baseOffset, curRcfrq, curoffset / 3067, (int)sw_frqOffset);

#if 1
            BB_WriteReg(PAGE1, SW_OFFSET_0, (sw_frqOffset >> 24)&0xff);
            BB_WriteReg(PAGE1, SW_OFFSET_1, (sw_frqOffset >> 16)&0xff);
            BB_WriteReg(PAGE1, SW_OFFSET_2, (sw_frqOffset >>  8)&0xff);
#else
            uint8_t hw1 = BB_ReadReg(PAGE2, HW_OFFSET_0), hw2 = BB_ReadReg(PAGE2, HW_OFFSET_1) , hw3 = BB_ReadReg(PAGE2, HW_OFFSET_2);
            BB_WriteReg(PAGE1, SW_OFFSET_0, hw1);
            BB_WriteReg(PAGE1, SW_OFFSET_1, hw2);
            BB_WriteReg(PAGE1, SW_OFFSET_2, hw3);
            dlog_info("%x %x %x", hw1, hw2, hw3);
#endif
            if ( stru_skyfrqOffst.u8_softEnable == 0 )
            {
                BB_WriteRegMask(PAGE1, 0x1a, 0x40, 0x40);
                stru_skyfrqOffst.u8_softEnable = 1;
            }
        }   
        else if ( stru_skyfrqOffst.u8_softEnable == 1 )
        {
            BB_WriteRegMask(PAGE1, 0x1a, 0x00, 0x40);
            stru_skyfrqOffst.u8_softEnable = 0;
        }*/
    }
}

void sky_set_it_freq(ENUM_RF_BAND band, uint8_t ch)
{
    BB_set_ITfrq(band, ch);
	context.cur_IT_ch = ch;

    dlog_info("S=>%d\r\n", ch);
}

void sky_get_RC_id(uint8_t* idptr)
{
    idptr[0] = BB_ReadReg(PAGE2, FEC_1_RD);
    idptr[1] = BB_ReadReg(PAGE2, FEC_2_RD_1);
    idptr[2] = BB_ReadReg(PAGE2, FEC_2_RD_2);
    idptr[3] = BB_ReadReg(PAGE2, FEC_2_RD_3);
    idptr[4] = BB_ReadReg(PAGE2, FEC_2_RD_4);   
}

void sky_set_RC_id(uint8_t *idptr)
{
    uint8_t i;
    uint8_t addr[] = {FEC_7, FEC_8, FEC_9, FEC_10, FEC_11};

    dlog_info("RCid:%02x%02x%02x%02x%02x\r\n", idptr[0], idptr[1], idptr[2], idptr[3], idptr[4]);                
    for(i=0; i < sizeof(addr); i++)
    {
        BB_WriteReg(PAGE2, addr[i], idptr[i]);
    }
}


void sky_soft_reset(void)
{
    BB_softReset(BB_SKY_MODE);
}

void sky_physical_link_process(void)
{
    uint8_t *p_id;
    uint8_t max_ch_size = (context.e_curBand == RF_2G) ? (MAX_2G_RC_FRQ_SIZE):(MAX_5G_RC_FRQ_SIZE);

    if(context.dev_state == SEARCH_ID)
    {
        if(RC_ID_AUTO_SEARCH == (context.u8_idSrcSel)) // auto serch
        {
            if(sky_id_search_run())
            {
                p_id = sky_id_search_get_best_id();
                sky_set_RC_id(p_id);
                sky_write_id(p_id);

                context.dev_state = CHECK_ID_MATCH;
                dlog_info("use auto search id");
            }
            else
            {
                context.rc_unlock_cnt ++;
                sky_soft_reset();                
            }
        }
        else if(RC_ID_USE_FLASH_SAVE == (context.u8_idSrcSel)) // read flash 
        {
            if(0 == sky_chk_flash_id_validity()) // flash id ok
            {
                sky_set_RC_id((uint8_t *)context.u8_flashId);
                context.dev_state = CHECK_ID_MATCH;
                sky_soft_reset();
                dlog_info("use fixed id");
            }
            else // flash id error,set to auto search
            {
                context.u8_idSrcSel = RC_ID_AUTO_SEARCH;
                search_id_list.count = 0; // completely re-search.
            }       
        }
        else
        {
            context.u8_idSrcSel = RC_ID_AUTO_SEARCH;
            search_id_list.count = 0; // completely re-search.
        }
    }
    else if(context.dev_state == ID_MATCH_LOCK)
    {
        s_lockFlag = 1;
        
        if(context.rc_skip_freq_mode == AUTO)
        {
            sky_rc_hopfreq();
        }

        context.locked = sky_id_match();
        if(context.locked)
        {
            uint8_t data0, data1;
            context.rc_unlock_cnt = 0;
            if (BB_ChkSpiFlag())
            {
                sky_handle_all_spi_cmds();     
            }     
            GPIO_SetPin(BLUE_LED_GPIO, 0);  //BLUE LED ON
            GPIO_SetPin(RED_LED_GPIO, 1);   //RED LED OFF
            sky_auto_adjust_agc_gain();     //agc is valid value only when locked
        }
        else
        {
            context.rc_unlock_cnt++;
            // sky_soft_reset();
        }

        if( 80 <= context.rc_unlock_cnt )
        {            
            GPIO_SetPin(BLUE_LED_GPIO, 1);  //BLUE LED OFF
            GPIO_SetPin(RED_LED_GPIO, 0);   //RED LED ON
            
            if ( context.brc_mode == AUTO )
            {
                context.qam_ldpc = context.u8_bbStartMcs;
                sky_set_McsByIndex(context.CH_bandwidth, context.qam_ldpc );
            }
            
            context.dev_state = CHECK_ID_MATCH;
            context.rc_unlock_cnt = 0;
        }

    }
    else if(context.dev_state == CHECK_ID_MATCH)
    {
        context.locked = sky_id_match();
        if(context.locked)
        {
            if(context.rc_skip_freq_mode == AUTO)
            {
                sky_rc_hopfreq();
            }

            context.rc_unlock_cnt = 0;
            context.dev_state = ID_MATCH_LOCK;
            sky_auto_adjust_agc_gain();     //agc is valid value only when locked            
        }
        else
        {
            context.rc_unlock_cnt++;
            uint8_t data = BB_ReadReg(PAGE2, FEC_4_RD);
            if (0x80 == (data & 0x80))
            {
                sky_soft_reset();
                //sky_clearFrqOffset();
            }
        }
    }

    if ( context.rc_unlock_cnt > (max_ch_size + 1) &&
        (context.dev_state == SEARCH_ID || (context.dev_state == CHECK_ID_MATCH)) )
    {
        context.rc_unlock_cnt = 0;
        sky_search_id_timeout( 1 );

        sky_backTo2GCheck();
    }
}

void sky_id_search_init(void)
{
    search_id_list.count = 0;
}

uint8_t get_rc_status(void)
{
    return BB_ReadReg(PAGE2, FEC_4_RD);
}


uint8_t sky_id_search_run(void)
{    
    uint8_t rc_status = get_rc_status();
    
    #define SKY_RC_ERR(status)  (0x80 == status)
    #define SKY_CRC_OK(status)  ((status & 0x02) ? 1 : 0)

    context.rc_status = rc_status;
    if( SKY_RC_ERR(rc_status))
    {
        sky_soft_reset();
        return FALSE;
    }

    if(SKY_CRC_OK(rc_status))
    {
        sky_get_RC_id(search_id_list.search_ids[search_id_list.count].id);

        //record AGC.
        search_id_list.search_ids[search_id_list.count].agc1 = BB_ReadReg(PAGE2, AAGC_2_RD);
        search_id_list.search_ids[search_id_list.count].agc2 = BB_ReadReg(PAGE2, AAGC_3_RD);
        search_id_list.count += 1;

        //record first get id time
        if(search_id_list.count == 1)
        {
            start_time_cnt = SysTicks_GetTickCount();
        }

        if(search_id_list.count >= MAX_SEARCH_ID_NUM)
        {
            return TRUE;
        }
    }

    if(search_id_list.count > 0)
    {
        return (SysTicks_GetTickCount() - start_time_cnt > SEARCH_ID_TIMEOUT);
    }

    return FALSE;
}


uint8_t* sky_id_search_get_best_id(void)
{
    uint8_t i,best_id_cnt;
    uint16_t agc1_agc2;
    if(search_id_list.count == 1)
    {
        return search_id_list.search_ids[0].id;
    }

    best_id_cnt = 0;
    agc1_agc2 = search_id_list.search_ids[best_id_cnt].agc1 + search_id_list.search_ids[best_id_cnt].agc2;

    for(i=1;i<search_id_list.count;i++)
    {
        if(search_id_list.search_ids[i].agc1 + search_id_list.search_ids[i].agc2 < agc1_agc2)
        {
            best_id_cnt = i;
            agc1_agc2 = search_id_list.search_ids[i].agc1 + search_id_list.search_ids[i].agc2;
        }
    }

    return search_id_list.search_ids[best_id_cnt].id;
}


int sky_search_id_timeout_irq_enable()
{
    TIM_StartTimer(sky_timer2_7);
}

int sky_search_id_timeout_irq_disable()
{
    TIM_StopTimer(sky_timer2_7);
}


void sky_search_id_timeout(uint8_t flag_agc)
{
    if(context.rc_skip_freq_mode == AUTO)
    {
        sky_rc_hopfreq();
    }

    if ( flag_agc )
    {
        sky_agc_gain_toggle();
    }
    sky_soft_reset();
}

void Sky_TIM2_7_IRQHandler(uint32_t u32_vectorNum)
{
    static int Timer1_Delay2_Cnt = 0;    

    INTR_NVIC_ClearPendingIRQ(TIMER_INTR27_VECTOR_NUM);
    
    if(TRUE == context.u8_debugMode) 
    {
        return;
    }   
    
    if(Timer1_Delay2_Cnt < 560)
    {
        Timer1_Delay2_Cnt ++;
    }
    else
    {
        sky_search_id_timeout( 1 );
        Timer1_Delay2_Cnt = 0;
    }

}


void sky_Timer2_7_Init(void)
{
    sky_timer2_7.base_time_group = 2;
    sky_timer2_7.time_num = 7;
    sky_timer2_7.ctrl = 0;
    sky_timer2_7.ctrl |= TIME_ENABLE | USER_DEFINED;
    TIM_RegisterTimer(sky_timer2_7, 1000);
    reg_IrqHandle(TIMER_INTR27_VECTOR_NUM, Sky_TIM2_7_IRQHandler, NULL);
	INTR_NVIC_SetIRQPriority(TIMER_INTR27_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_TIMER01,0));
}

void sky_Timer2_6_Init(void)
{
    sky_timer2_6.base_time_group = 2;
    sky_timer2_6.time_num = 6;
    sky_timer2_6.ctrl = 0;
    sky_timer2_6.ctrl |= TIME_ENABLE | USER_DEFINED;

    TIM_RegisterTimer(sky_timer2_6, 3800);

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
    context.rc_skip_freq_mode = (ENUM_RUN_MODE)data0;

    if( context.rc_skip_freq_mode == (ENUM_RUN_MODE)MANUAL)
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


static void sky_handle_IT_cmd(void)
{
    uint8_t data0, data1, data2, data3;

    data0 = BB_ReadReg(PAGE2, IT_FRQ_0);
    data1 = BB_ReadReg(PAGE2, IT_FRQ_1);
    data2 = BB_ReadReg(PAGE2, IT_FRQ_2);
    data3 = BB_ReadReg(PAGE2, IT_FRQ_3);

    if ( context.stru_itRegs.frq1 != data0 || context.stru_itRegs.frq2 != data1 || 
         context.stru_itRegs.frq3 != data2 || context.stru_itRegs.frq4 != data3 )
    {
        uint32_t u32_rc = ( data0 << 24 ) | ( data1 << 16 ) | ( data2 << 8 ) | ( data3 );
        BB_write_ItRegs( u32_rc );
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
static void sky_doRfBandChange( void )
{
    if ( context.stru_bandChange.flag_bandchange )
    {
        context.stru_bandChange.u8_bandChangecount--;
        if ( 0 == context.stru_bandChange.u8_bandChangecount )
        {
            context.stru_bandChange.flag_bandchange = 0;
            context.stru_bandChange.u8_bandChangecount = 0;
            context.e_curBand = OTHER_BAND( context.e_curBand );

            dlog_info("band: %d %d %d %d",  sky_rc_channel, context.stru_bandChange.u8_bandChangecount, 
                                            context.stru_bandChange.u8_ItCh, context.e_curBand);

            BB_set_RF_Band(BB_SKY_MODE, context.e_curBand);
            sky_rc_channel = 0;
            sky_rc_hopfreq();

            BB_set_ITfrq( context.e_curBand, context.stru_bandChange.u8_ItCh);
        }
        else
        {
            //dlog_info("!! ch: %d %d", sky_rc_channel, context.stru_bandChange.u8_bandChangecount);
        }   
    }
}

static uint16_t rc_unlock_loop = 0;

static void sky_backTo2GCheck(void)
{
    //5G: 6 * 40 * 17ms = 4080ms
    if ( context.e_bandsupport == RF_2G_5G && context.e_curBand == RF_5G )
    {
        if ( rc_unlock_loop ++ >= SKY_BACK_TO_2G_UNLOCK_CNT )
        {
            //set band
            context.e_curBand = RF_2G;
            BB_set_RF_Band(BB_SKY_MODE, context.e_curBand);

            //set RC
            sky_rc_channel = 0;
            sky_rc_hopfreq();

            //set IT
            BB_set_ITfrq( context.e_curBand, 0);
    
            rc_unlock_loop = 0;
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

        if(context.CH_bandwidth != bw)
        {
            //set and soft-rest
            BB_set_RF_bandwitdh(BB_SKY_MODE, bw);
            context.CH_bandwidth = bw;
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
            sky_set_McsByIndex( context.CH_bandwidth, context.qam_ldpc );
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
        sky_set_McsByIndex(context.CH_bandwidth, data0);
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
    if ((s_st_rcFrqMask.u64_rcvGrdMask) != u64_tmpMask)
    {
        s_st_rcFrqMask.u64_rcvGrdMask = u64_tmpMask;
        dlog_info("rcv_grd_mask:0x%x,0x%x", (uint32_t)((u64_tmpMask)>>32), (uint32_t)(u64_tmpMask));
    }
}

static void sky_handle_rc_channel_sync_cmd(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, GRD_RC_CHANNEL);
    uint8_t max_ch_size = (context.e_curBand == RF_2G) ? (MAX_2G_RC_FRQ_SIZE):(MAX_5G_RC_FRQ_SIZE);
    
    data0 += 1;
    data0 %= max_ch_size;

    if (data0 != sky_rc_channel)
    {
        sky_rc_channel = data0;
        dlog_info("sync rc frq channel.");
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

    if ( stru_skyfrqOffst.u32_cyclecnt == 0)   //17 * 200 = 3.4s
    {
        int integer, fraction, tracking;
        ENUM_RF_BAND e_band;
        int basefrq;
        uint8_t ch;

        e_band = (ENUM_RF_BAND)(tmp >> 6);
        ch = tmp & 0x3f;

        if ( stru_skyfrqOffst.baseOffset == 0)
        {
            integer  = BB_ReadReg(PAGE2, INTEGER_OFFSET);
            
            fraction = (BB_ReadReg(PAGE2, FRACTION_OFFSET_0) << 24) | (BB_ReadReg(PAGE2, FRACTION_OFFSET_1) << 16) |
                       (BB_ReadReg(PAGE2, FRACTION_OFFSET_2) <<  8) | (BB_ReadReg(PAGE2, FRACTION_OFFSET_3));
                       
            tracking = (BB_ReadReg(PAGE2, TRACK_OFFSET_0) << 24) | (BB_ReadReg(PAGE2, TRACK_OFFSET_1) << 16) |
                       (BB_ReadReg(PAGE2, TRACK_OFFSET_2) <<  8) | (BB_ReadReg(PAGE2, TRACK_OFFSET_3));
        }

        if ( e_band == RF_2G && context.CH_bandwidth == BW_10M )
        {
            basefrq = 2406 + ch * 10;
        }
        else if( e_band == RF_2G && context.CH_bandwidth == BW_20M )
        {
        }
        else if ( e_band == RF_5G && context.CH_bandwidth == BW_10M )
        {
            basefrq = 5730 + ch * 10;
        }
        else if( e_band == RF_2G && context.CH_bandwidth == BW_20M )
        {
        }

        stru_skyfrqOffst.u8_flagOffset = 1;
        stru_skyfrqOffst.baseOffset = ( fraction + tracking - (integer << 19) );
        stru_skyfrqOffst.ll_offset  = frqoffset * 8;
        stru_skyfrqOffst.basefrq    = basefrq;

        dlog_info("base: %d %d %d", basefrq, frqoffset, stru_skyfrqOffst.baseOffset);
    }

    if ( stru_skyfrqOffst.u32_cyclecnt ++ > 1000)  
    {
        stru_skyfrqOffst.u32_cyclecnt = 0;
    }
};

/*
 * called the function if unlocked
*/
void sky_clearFrqOffset(void)
{
    if (stru_skyfrqOffst.u8_flagOffset)
    {        
        stru_skyfrqOffst.u8_flagOffset = 0;  //
        stru_skyfrqOffst.u32_cyclecnt  = 0;

        if ( stru_skyfrqOffst.u8_softEnable == 0 )
        {
            BB_WriteRegMask(PAGE1, 0x1a, 0x40, 0x40);
            stru_skyfrqOffst.u8_softEnable = 1;
        }
    }
}
void sky_handle_all_spi_cmds(void)
{
    sky_handle_RC_cmd();

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

    sky_handle_rc_channel_sync_cmd();
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
                context.it_skip_freq_mode = (ENUM_RUN_MODE)value;
                break;
            }
            case FREQ_CHANNEL_SELECT:
            {
                sky_set_it_freq(context.e_curBand, (uint8_t)value);
                break;
            }

            case RC_CHANNEL_MODE:
            {
                context.rc_skip_freq_mode = (ENUM_RUN_MODE)value;
                break;
            }

            case RC_CHANNEL_SELECT:
            {
                sky_rc_channel = (uint8_t)value;
                BB_set_Rcfrq(context.e_curBand, sky_rc_channel);
                break;
            }

            case RC_CHANNEL_FREQ:
            {
                context.rc_skip_freq_mode = (ENUM_RUN_MODE)MANUAL;                
                BB_write_RcRegs(value);
                
                dlog_info("RC_CHANNEL_FREQ %x\r\n", value);
                break;
            }
            
            case IT_CHANNEL_FREQ:
            {
                context.it_skip_freq_mode = MANUAL;
                BB_write_ItRegs(value);
                dlog_info("IT_CHANNEL_FREQ %x\r\n", value);                
                break;
            }

            case FREQ_BAND_WIDTH_SELECT:
            {
                context.CH_bandwidth = (ENUM_CH_BW)value;
                BB_set_RF_bandwitdh(BB_SKY_MODE, context.CH_bandwidth);
                dlog_info("FREQ_BAND_WIDTH_SELECT %x\r\n", value);         
                break;
            }

            default:
            {
                dlog_error("%s\r\n", "unknown WIRELESS_FREQ_CHANGE command");
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
                dlog_error("%s\r\n", "unknown WIRELESS_MCS_CHANGE command");
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
                dlog_error("%s\r\n", "unknown WIRELESS_ENCODER_CHANGE command");
                break;                
        }
    }
    
    if(class == WIRELESS_MISC)
    {
        BB_handle_misc_cmds(pcmd);
    }

    if(class == WIRELESS_AUTO_SEARCH_ID)
    {
        sky_set_auto_search_rc_id();
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
static void BB_sky_GatherOSDInfo(void)
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
    
    osdptr->agc_value[2] = get_rc_status();
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


void sky_set_auto_search_rc_id(void)
{
    context.u8_idSrcSel = RC_ID_AUTO_SEARCH;
    context.dev_state   = SEARCH_ID;
    search_id_list.count = 0; // completely re-search.
}

static int32_t sky_chk_flash_id_validity(void)
{
    uint8_t u8_chk = 0;
   
    cal_chk_sum((uint8_t *)&(context.u8_flashId[0]), 5, &u8_chk);
    
    if (u8_chk == (context.u8_flashId[5])) // data is valid,use flash save id
    {
        return 0;
    }
    
    return -1;
}

static int32_t sky_write_id(uint8_t *u8_idArray)
{
    STRU_SysEvent_NvMsg st_nvMsg;

    // src:cpu0 dst:cpu2
    st_nvMsg.u8_nvSrc = INTER_CORE_CPU2_ID;
    st_nvMsg.u8_nvDst = INTER_CORE_CPU0_ID;

    // parament number
    st_nvMsg.e_nvNum = NV_NUM_RCID;

    // parament set
    st_nvMsg.u8_nvPar[0] = context.u8_idSrcSel;
    memcpy(&(st_nvMsg.u8_nvPar[1]), u8_idArray, 5);
    cal_chk_sum(&(st_nvMsg.u8_nvPar[1]), 5, &(st_nvMsg.u8_nvPar[6]));

    // send msg
    SYS_EVENT_Notify(SYS_EVENT_ID_NV_MSG, (void *)(&(st_nvMsg)));

    return 0;
}

static int32_t cal_chk_sum(uint8_t *pu8_data, uint32_t u32_len, uint8_t *u8_check)
{
    uint8_t u8_i;
    uint8_t u8_chk = 0;

    if (NULL == pu8_data)
    {
        return -1;
    }

    for (u8_i = 0; u8_i < u32_len; u8_i++)
    {
        u8_chk += pu8_data[u8_i];
    }
    
    *u8_check = u8_chk;

    return 0;
}

static void sky_calc_dist(void)
{
    uint8_t u8_data[3];
    uint32_t u32_data = 0;
    static uint32_t u32_cnt = 1;

    u8_data[0] = BB_ReadReg(PAGE3, 0xA4);
    u8_data[1] = BB_ReadReg(PAGE3, 0xA3);
    u8_data[2] = BB_ReadReg(PAGE3, 0xA2);

    BB_WriteReg(PAGE0, 0x1D, u8_data[0]);
    BB_WriteReg(PAGE0, 0x1C, u8_data[1]);
    BB_WriteReg(PAGE0, 0x1B, u8_data[2]);
}


////sky snr ////

static uint16_t u16_sky_snr[8];
static uint8_t  u8_snr_idx = 0;

static uint16_t sky_get_rc_snr( void )
{
    static uint32_t cnt = 0;
    uint16_t snr = (((uint16_t)BB_ReadReg(PAGE2, SNR_REG_0)) << 8) | BB_ReadReg(PAGE2, SNR_REG_1);
    if( cnt++ > 1500 )
    {
        cnt = 0;
        dlog_info("SNR1:%0.4x\n", snr);
    }

    return snr;
}


static uint16_t sky_get_snr_average(void)
{
    uint8_t i;
    uint32_t sum = 0; 
    for (i = 0; i < sizeof(u16_sky_snr) / sizeof(u16_sky_snr[0]); i++)
    {
        sum += u16_sky_snr[i];
    }

    return (sum/i);
}

static void sky_init_rc_frq_mask_func(void)
{
    memset((uint8_t*)(&s_st_rcFrqMask), 0x00, sizeof(STRU_RC_FRQ_MASK));
}


static void sky_rc_frq_status_statistics(void)
{
    static uint16_t u16_txCnt = 0;
    uint8_t u8_rcCh = 0;
    uint8_t u8_cnt = 0;
    uint8_t u8_clcCh = 0;
    uint8_t i = 0;
    uint64_t u64_mask;
    uint64_t u64_preMask = s_st_rcFrqMask.u64_mask;
    int8_t max_ch_size = (context.e_curBand == RF_2G) ? (MAX_2G_RC_FRQ_SIZE):(MAX_5G_RC_FRQ_SIZE);
    
    if(context.dev_state == ID_MATCH_LOCK)
    {
        if (0 == sky_rc_channel)
        {
            u8_rcCh = max_ch_size - 1;
        }
        else
        {
            u8_rcCh = sky_rc_channel - 1;
        }

        if ((s_st_rcFrqMask.u64_mask) & (((uint64_t)1) << u8_rcCh))
        {
            return;
        }
        
        if(context.locked)
        {
            s_st_rcFrqMask.u8_unLock[u8_rcCh] = 0;
        }
        else
        {
            s_st_rcFrqMask.u8_unLock[u8_rcCh] += 1;
        }

        if (s_st_rcFrqMask.u8_unLock[u8_rcCh] >= RC_FRQ_MASK_THRESHOLD)
        {
            s_st_rcFrqMask.u64_mask |= ( ((uint64_t)1) << u8_rcCh );
            
            u8_clcCh = s_st_rcFrqMask.u8_maskOrder[s_st_rcFrqMask.u8_maskOrderIndex];
            s_st_rcFrqMask.u8_maskOrder[s_st_rcFrqMask.u8_maskOrderIndex] = u8_rcCh;
            s_st_rcFrqMask.u8_maskOrderIndex += 1;
            s_st_rcFrqMask.u8_maskOrderIndex %= RC_FRQ_MAX_MASK_NUM;

            u8_cnt = 0;
            u64_mask = s_st_rcFrqMask.u64_mask;
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
                s_st_rcFrqMask.u64_mask &= (~(((uint64_t)1) << (u8_clcCh)));
            }

            // mask one, then all start again.
            memset(&(s_st_rcFrqMask.u8_unLock[0]), 0x00, RF_FRQ_MAX_NUM);
            s_st_rcFrqMask.u8_unLock[u8_rcCh] = RC_FRQ_MASK_THRESHOLD;
        }

        if (u64_preMask != (s_st_rcFrqMask.u64_mask))
        {
            u16_txCnt = 169;
            dlog_info("new_calc_mask_code:0x%x,0x%x", (uint32_t)((s_st_rcFrqMask.u64_mask)>>32), (uint32_t)(s_st_rcFrqMask.u64_mask));
        }

        u16_txCnt += 1;
        if (170 == u16_txCnt) // 170 * 14 = 2380ms
        {
            STRU_SysEventSkyStatus stru_skycStatus = 
            {
                .pid             = RC_MASK_CODE,
                .par.u64_rcMask = s_st_rcFrqMask.u64_mask,
            };
            
#if 0
            BB_UARTComSendMsg( BB_UART_COM_SESSION_0, (uint8_t *)&stru_skycStatus, sizeof(STRU_SysEventSkyStatus));
#endif
            u16_txCnt = 0;
        }
    }
}
