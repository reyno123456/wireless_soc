#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "bb_snr_service.h"
#include "bb_ctrl_internal.h"
#include "bb_regs.h"
#include "systicks.h"
#include "debuglog.h"

#define     WORK_FREQ_SNR_BLOCK_ROWS        (4)
#define     WORK_FREQ_SNR_DAQ_CNT           (16)
#define     WORK_FREQ_SNR_BLOCK_COLS        (WORK_FREQ_SNR_DAQ_CNT + 1) 
#define     FAIL_TIMES_THLD                 (3)


#define     HARQ_STATUS_CNT                 (200)   //14ms cycle count, 2.8s
#define     MCS0_UP_HARQ_CNT_MAX            (60)
#define     OTHER_MCS_UP_HARQ_CNT_MAX       (70)

#define     MCS_SNR_BLOCK_ROWS              (8)
#define     IT_SKIP_SNR_WINDOW_SIZE         (4)
#define     SNR_CMP_CNT                     (100)


#define     MCS1_DOWN_CONT_SNR              (5)
#define     MCS1_DOWN_SNR                   (20)
#define     MCS2_DOWN_CONT_SNR              (4)
#define     MCS3_DOWN_CONT_SNR              (3)
#define     MCS4_DOWN_CONT_SNR              (2)
#define     MCS5_DOWN_CONT_SNR              (1)


typedef struct
{
    uint16_t  u16_pieceSNR[WORK_FREQ_SNR_DAQ_CNT];
    uint8_t   u8_idx;
    uint8_t   u8_failCount0;
    uint8_t   u8_failCount1;
}STRU_pieceSNR;

STRU_pieceSNR stru_snr;

/*
  * return type: 1:  snr check pass
  *              0:  snr check Fail
  *              2:  need to check in next cycle.
*/
int grd_check_piecewiseSnrPass(uint8_t u8_flag_start, uint16_t u16_thld)
{
    uint8_t ret = 0;

    if( u8_flag_start )
    {
        stru_snr.u8_idx        = 0;
        stru_snr.u8_failCount0 = 0;
        stru_snr.u8_failCount1 = 0;
    }

    uint8_t loc = BB_ReadReg(PAGE2, 0xc4) & 0x0F;
    uint8_t startaddr = (0xE0 + stru_snr.u8_idx * 2);
    uint8_t i = stru_snr.u8_idx;

    if(loc == WORK_FREQ_SNR_DAQ_CNT -2) //fix: the last one snr can't get at this time, get the snr in past 14ms
    {
        loc = (WORK_FREQ_SNR_DAQ_CNT - 1);
    }
    for(; i <= loc; i++)
    {
        stru_snr.u16_pieceSNR[stru_snr.u8_idx] = ((uint16_t)BB_ReadReg(PAGE2, startaddr) << 8) | (BB_ReadReg(PAGE2, startaddr + 1));
        startaddr += 2;
        uint8_t fail = (stru_snr.u16_pieceSNR[stru_snr.u8_idx] < u16_thld);
        if(stru_snr.u8_idx < WORK_FREQ_SNR_DAQ_CNT / 2)
        {
            if (stru_snr.u8_failCount0 < FAIL_TIMES_THLD)
            {
                if( fail )
                {
                    stru_snr.u8_failCount0 += 1;
                }
                else
                {
                    stru_snr.u8_failCount0 = 0;
                }
            }
        }
        else
        {
            if (stru_snr.u8_failCount1 < FAIL_TIMES_THLD)
            {
                if( fail )
                {
                    stru_snr.u8_failCount1 += 1;
                }
                else
                {
                    stru_snr.u8_failCount1 = 0;
                }                
            }
        }

        stru_snr.u8_idx++;
    }
    
    if( stru_snr.u8_failCount0 >= FAIL_TIMES_THLD && stru_snr.u8_failCount1 >= FAIL_TIMES_THLD )
    {
        ret = 0;
    }
    else if( stru_snr.u8_failCount0 < FAIL_TIMES_THLD || (stru_snr.u8_failCount1 + (WORK_FREQ_SNR_DAQ_CNT -1 - loc) < FAIL_TIMES_THLD))
    {
        ret = 1;
    }
    else
    {
        ret = 2;
    }

    if( stru_snr.u8_failCount0 > 0 || stru_snr.u8_failCount1 > 0 )
    {
        /*dlog_info("sliceSNR Loc:%d thld:0x%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n", loc, u16_thld,
                    stru_snr.u16_pieceSNR[0],  stru_snr.u16_pieceSNR[1],  stru_snr.u16_pieceSNR[2],  stru_snr.u16_pieceSNR[3],
                    stru_snr.u16_pieceSNR[4],  stru_snr.u16_pieceSNR[5],  stru_snr.u16_pieceSNR[6],  stru_snr.u16_pieceSNR[7],
                    stru_snr.u16_pieceSNR[8],  stru_snr.u16_pieceSNR[9],  stru_snr.u16_pieceSNR[10], stru_snr.u16_pieceSNR[11],
                    stru_snr.u16_pieceSNR[12], stru_snr.u16_pieceSNR[13], stru_snr.u16_pieceSNR[14], stru_snr.u16_pieceSNR[15]
                );

        dlog_info("SNR Fail=%d %d %d ret=%d \n", u8_flag_start, stru_snr.u8_failCount0, stru_snr.u8_failCount1, ret);   */ 
    }
    return ret;    
}


//////////////////////////////////////////////////////
//// SNR use for the mcs.

typedef struct
{
    uint16_t snr[MCS_SNR_BLOCK_ROWS];           //for average SNR  
    uint8_t  snr_cmpResult[SNR_CMP_CNT];        //the compare result between threshhold
    uint8_t  row_index;
    uint16_t u16_cmpCnt;
    uint8_t  u8_isFull;
    uint16_t snr_avg;
    uint16_t u16_upCount;                       //continuous snr > threshhold count number
    uint16_t u16_downCount;                     //continuous snr < threshhold count number
}STRU_MCS_WORK_CH_SNR;

static STRU_MCS_WORK_CH_SNR work_ch_snr;


uint16_t calu_average(uint16_t *p_data, uint8_t len)
{
    uint8_t i;
    uint32_t sum = 0;
    for(i=0; i<len; i++)
    {
        sum += p_data[i];
    }

    return (uint16_t)(sum / len);
}


uint16_t grd_get_it_snr( void )
{
    uint16_t snr;
    snr = (((uint16_t)BB_ReadReg(PAGE2, SNR_REG_0)) << 8) | BB_ReadReg(PAGE2, SNR_REG_1);

    static uint32_t cnt = 0;
    if( cnt++ > 1000 )
    {
        cnt = 0;
        dlog_info("SNR1:%0.4x\n", snr);
    }

    return snr;
}


uint16_t get_snr_average(void)
{
    return calu_average( work_ch_snr.snr, MCS_SNR_BLOCK_ROWS );
}


/**
 * buf : data buffer to count
 * idx : start count index
 * len : total len
 * cnt : number of item to count
*/
static uint16_t count_num_inbuf(uint8_t *buf, uint16_t len)
{
    uint16_t cnt = 0;
    uint16_t i;
    for ( i = 0 ;i < len; i++ )
    {
        cnt += buf[i];
    }

    return cnt;
}


/** 
 * buf : data buffer to count
 * idx : start count index
 * len : total len
 * cnt : number of item to count
*/
static uint32_t count_num_inbuf_1(uint8_t *buf, uint16_t len, uint8_t cnt, uint8_t idx)
{
    if ( len == cnt )
    {
        return count_num_inbuf( buf, len );
    }
    else
    {
        uint32_t errcnt = 0;
        uint16_t i = 0;

        if ( idx >= cnt )
        {
            for ( i = 0 ;i < cnt; i++ )
            {
                errcnt += buf[idx-cnt + i];
            }
        }
        else
        {
            for ( i = 0 ;i < idx; i++ )
            {
                errcnt += buf[i];
            }
            for ( i = 0 ;i < (cnt - idx); i++ )
            {
                errcnt += buf[len - 1 - i];
            }
        }
        return errcnt;
    }
}


QAMUPDONW snr_static_for_qam_change(uint16_t threshod_left_section, uint16_t threshold_right_section)
{
    uint16_t aver;
    uint8_t ret;

    if ( !work_ch_snr.u8_isFull )
    {
        return QAMKEEP;
    }

    if( work_ch_snr.u16_cmpCnt >= SNR_CMP_CNT )
    {
        work_ch_snr.u16_cmpCnt = 0;
    }

    aver = calu_average( work_ch_snr.snr, MCS_SNR_BLOCK_ROWS );
    if( aver > threshold_right_section )
    {
        work_ch_snr.u16_upCount ++;
        work_ch_snr.u16_downCount = 0;

        work_ch_snr.snr_cmpResult[ work_ch_snr.u16_cmpCnt ] = QAMUP;
        if( ( context.qam_ldpc == 0 && work_ch_snr.u16_upCount > 500 ) || 
            ( context.qam_ldpc != 0 && work_ch_snr.u16_upCount > 300 ) )
        {
            work_ch_snr.u16_upCount = 0;            
            ret = QAMUP;        //up
        }
        else
        {
            ret = QAMKEEP;     //keep
        }
    }
    else if( aver < threshod_left_section )
    {
        work_ch_snr.u16_downCount ++;

        work_ch_snr.snr_cmpResult[ work_ch_snr.u16_cmpCnt ] = QAMDOWN;

        if (  ( context.qam_ldpc == 1 && work_ch_snr.u16_downCount >= MCS1_DOWN_CONT_SNR ) || 
              /*( context.qam_ldpc == 1 && cnt >= MCS1_DOWN_SNR ) ||*/
              ( context.qam_ldpc == 2 && work_ch_snr.u16_downCount >= MCS2_DOWN_CONT_SNR ) ||
              ( context.qam_ldpc == 3 && work_ch_snr.u16_downCount >= MCS3_DOWN_CONT_SNR ) ||
              ( context.qam_ldpc == 4 && work_ch_snr.u16_downCount >= MCS4_DOWN_CONT_SNR ) ||
              ( context.qam_ldpc == 5 && work_ch_snr.u16_downCount >= MCS5_DOWN_CONT_SNR )
              )
        {
            work_ch_snr.u16_downCount = 0;
            ret = QAMDOWN;
        }
        else
        {
            ret = QAMKEEP;
        }
    }
    else
    {
        work_ch_snr.u16_downCount = 0;
        work_ch_snr.u16_upCount = 0;
        work_ch_snr.snr_cmpResult[ work_ch_snr.u16_cmpCnt ] = QAMKEEP;
        ret = QAMKEEP;
    }

    work_ch_snr.u16_cmpCnt ++;
    return ret;
}


void grd_get_snr(void)
{
    if( work_ch_snr.row_index >= MCS_SNR_BLOCK_ROWS )
    {
        work_ch_snr.row_index = 0;
    }
    work_ch_snr.snr[work_ch_snr.row_index] = grd_get_it_snr();
    work_ch_snr.row_index ++;

    if( work_ch_snr.row_index == MCS_SNR_BLOCK_ROWS )
    {
        work_ch_snr.u8_isFull = 1;
        work_ch_snr.row_index = 0;
    }
}


//////////////////////////////////////////////////////////////////////////////////////
//Harq statistic for mcs 

typedef struct
{
    uint8_t  u8_flagharq[HARQ_STATUS_CNT];  //1: means harq happen.  0: not happen
    uint8_t  u8_isFull;                     //
    uint8_t  u8_idx;
}STRU_HARQ_STATUS;

STRU_HARQ_STATUS stru_harqStatus;

uint8_t grd_get_harqCnt( void )
{
    uint8_t u8_harqCnt = ((BB_ReadReg(PAGE2, FEC_5_RD)& 0xF0) >> 4);

    if ( stru_harqStatus.u8_idx >= HARQ_STATUS_CNT )
    {
        stru_harqStatus.u8_idx = 0;
        stru_harqStatus.u8_isFull = 1;
    }

    stru_harqStatus.u8_flagharq[ stru_harqStatus.u8_idx ] = ( u8_harqCnt > 0 );
    stru_harqStatus.u8_idx ++;

    if ( stru_harqStatus.u8_isFull )
    {
        uint8_t cnt = count_num_inbuf( stru_harqStatus.u8_flagharq, HARQ_STATUS_CNT ); //count the harq times
        //dlog_info("Harq: %d", cnt);
        return QAMUP;
    }

    return 0xff;
}


static uint8_t mcs_10m_registers[][12] = 
{
    {0xff, 0xff, 0xff, 0x4c, 0x71, 0x60, 0xff, 0xff, 0xff, 0x29, 0x00, 0x20},  //For mcs 0(BP 1/2),1(BP 1/2),2(QP 1/2), 3(16QAM 1/2)
    {0xff, 0xf5, 0x00, 0x38, 0x00, 0xC0, 0xff, 0xf1, 0x20, 0x0C, 0x00, 0x20},  //For (BP 2/3), (QP 2/3)
    {0xff, 0xf5, 0x00, 0x20, 0x00, 0xc0, 0xff, 0xf1, 0x20, 0x06, 0x00, 0x20},  //For mcs 4(64qam, 1/2) AND (16QAM, 2/3)
    {0x50, 0x02, 0x00, 0x0c, 0x00, 0x00, 0x12, 0x00, 0x60, 0x02, 0x00, 0x00},  //For mcs 5(64qam, 2/3)
};

static uint8_t mcs_20m_registers[][12] = 
{
    {0xff, 0xf5, 0x00, 0x22, 0x00, 0x30, 0xff, 0xf2, 0x20, 0x0C, 0x00, 0x10},  //For (BP 1/2),(BP 2/3), (QP 1/2),(QP 2/3),(16QAM 1/2),(64qam, 1/2)
    {0xff, 0xf5, 0x00, 0x18, 0x00, 0x00, 0xff, 0xf1, 0x20, 0x05, 0x00, 0x00},  //For (16QAM 2/3),(64qam 2/3)
};

void grd_set_mcs_registers(ENUM_BB_QAM e_qam, ENUM_BB_LDPC e_ldpc, ENUM_CH_BW e_bw)
{
    uint8_t addr = 0x66;
    uint8_t *regdata;
    uint8_t cnt;
    uint8_t size;

    if (BW_10M == e_bw)
    {
        if ( e_qam == MOD_64QAM && e_ldpc == LDPC_2_3)
        {
            regdata = mcs_10m_registers[3];
        }
        else if ( (e_qam == MOD_64QAM && e_ldpc == LDPC_1_2)  || (e_qam == MOD_16QAM && e_ldpc == LDPC_2_3))
        {
            regdata = mcs_10m_registers[2];
        }
        else if ( (e_qam == MOD_BPSK && e_ldpc == LDPC_2_3)  || (e_qam == MOD_4QAM && e_ldpc == LDPC_2_3))
        {
            regdata = mcs_10m_registers[1];
        }
        else
        {
            regdata = mcs_10m_registers[0];
        }

        size = sizeof(mcs_10m_registers[0]);
    }
    else // 20M
    {
        if ((e_ldpc == LDPC_2_3) && ((e_qam == MOD_16QAM) || (e_qam == MOD_64QAM)))
        {
            regdata = mcs_20m_registers[1];
        }
        else
        {
            regdata = mcs_20m_registers[0];
        }

        size = sizeof(mcs_20m_registers[0]);
    }

    for (cnt = 0; cnt < size; cnt++)
    {
        BB_WriteReg(PAGE1, (addr + cnt), regdata[cnt]);
    } 
}

void grd_set_txmsg_mcs_change(ENUM_CH_BW bw, uint8_t index)
{
    uint8_t addr = 0x66;
    uint8_t *regdata;
    uint8_t cnt;
    uint8_t size;

    BB_WriteReg(PAGE2, MCS_INDEX_MODE_0, index);

    if (BW_10M == bw)
    {
        if ( index <= 3 )
        {
            regdata = mcs_10m_registers[0];
        }
        else if( index == 4 )
        {
            regdata = mcs_10m_registers[2];
        }    
        else if( index == 5 )
        {
            regdata = mcs_10m_registers[3];
        }
        
        size = sizeof(mcs_10m_registers[0]);
    }
    else // 20M
    {
        if( index == 4 )
        {
            regdata = mcs_20m_registers[1];
        }    
        else
        {
            regdata = mcs_20m_registers[0];
        }

        size = sizeof(mcs_20m_registers[0]);
    }

    for (cnt = 0; cnt < size; cnt++)
    {
        BB_WriteReg(PAGE1, (addr + cnt), regdata[cnt]);
    }

    dlog_info("MCS2=> 0x%x\n", index);
}


QAMUPDONW harq_static_for_qam_up( void )
{
    QAMUPDONW qamupdown = QAMKEEP;
    uint8_t u8_harqCnt = grd_get_harqCnt();

    if ( context.qam_ldpc == 0 && u8_harqCnt <= MCS0_UP_HARQ_CNT_MAX )
    {
        qamupdown = QAMUP;
    }

    //dlog_info(" qamupdown %d", qamupdown);

    return qamupdown;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//LDPC error for MCS.

#define TOTAL_LDPC_ERR_CNT     (200)
#define MCS1_UP_ERR_THLD       (960)     //(BPSK, 1/2)        32 * 200 * 15% = 960;
#define MCS2_UP_ERR_THLD       (1280)    //10%  (QPSK, 1/2)   64 * 200 * 10% = 1280;
#define MCS3_UP_ERR_THLD       (1280)    //5%   (16QAM, 1/2)  128* 200 * 5%  = 1280
#define MCS4_UP_ERR_THLD       (5)       //2.5% (64QAM, 1/2)  192* 200 * 2.5%= 960


#define MCS_DOWN_CNT             (100)
#define MCS1_DOWN_ERR_THLD       (2240)     //70%  (BPSK,  1/2) -> (QPSK, 1/2):    32  * 100 * 70% = 2240
#define MCS2_DOWN_ERR_THLD       (2880)     //45%  (QPSK,  1/2) -> (BPSK, 1/2):    64  * 100 * 45% = 2880
#define MCS3_DOWN_ERR_THLD       (5120)     //40%  (16QAM, 1/2) -> (QPSK, 1/2):    128 * 100 * 40% = 5120
#define MCS4_DOWN_ERR_THLD       (5760)     //30%  (64QAM, 1/2) -> (16QAM, 1/2)    192 * 100 * 30% = 5760
#define MCS5_DOWN_ERR_THLD       (4800)     //25%  (64QAM, 2/3) -> (64QAM, 1/2)    192 * 100 * 25% = 4800

typedef struct
{   
    uint8_t  u8_flagErr[TOTAL_LDPC_ERR_CNT];  //1: means err happen.  0: not happen
    uint8_t  u8_isFull;                       //
    uint8_t  u8_idx;    
}STRU_LDPC_ERROR;


STRU_LDPC_ERROR stru_ldpcErr;

uint16_t ldpc_total[] = 
{
    32 * 200,
    32 * 200,
    64 * 200,
    128 * 200,
    192 * 200,
    192 * 200,
};

QAMUPDONW ldpc_static_for_qam_change( void )
{
    QAMUPDONW updown = QAMKEEP;
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);

    if ( stru_ldpcErr.u8_idx >= TOTAL_LDPC_ERR_CNT)
    {
        stru_ldpcErr.u8_idx = 0;
        stru_ldpcErr.u8_isFull = 1;
    }

    uint8_t err = /*(((uint16_t)BB_ReadReg(PAGE2, LDPC_ERR_HIGH_8)) << 8) | */BB_ReadReg(PAGE2, LDPC_ERR_LOW_8);
    stru_ldpcErr.u8_flagErr[ stru_ldpcErr.u8_idx ] = err;
    stru_ldpcErr.u8_idx ++;

    if ( stru_ldpcErr.u8_isFull )
    {
        uint32_t cnt  = count_num_inbuf  (stru_ldpcErr.u8_flagErr, TOTAL_LDPC_ERR_CNT );                                     //count the err times
        uint32_t cnt1 = count_num_inbuf_1(stru_ldpcErr.u8_flagErr, TOTAL_LDPC_ERR_CNT, MCS_DOWN_CNT, stru_ldpcErr.u8_idx);   //count the err times

        osdptr->errcnt1 =   (uint32_t)cnt  * 100 / ldpc_total[context.qam_ldpc];
        osdptr->errcnt2 =   (uint32_t)cnt1 * 100 * 2 / ldpc_total[context.qam_ldpc];

        //dlog_info( "%d %d %d", context.qam_ldpc, cnt, cnt1 );

        if ( (context.qam_ldpc == 1 && cnt <= MCS1_UP_ERR_THLD ) ||
             (context.qam_ldpc == 2 && cnt <= MCS2_UP_ERR_THLD ) ||
             (context.qam_ldpc == 3 && cnt <= MCS3_UP_ERR_THLD ) ||
             (context.qam_ldpc == 4 && cnt <= MCS4_UP_ERR_THLD ))
        {
            updown = QAMUP;
            //dlog_info( "LDPC UP %d %d", context.qam_ldpc, cnt );
        }

        if ( (context.qam_ldpc == 1 && cnt1 >= MCS1_DOWN_ERR_THLD ) ||
             (context.qam_ldpc == 2 && cnt1 >= MCS2_DOWN_ERR_THLD ) ||
             (context.qam_ldpc == 3 && cnt1 >= MCS3_DOWN_ERR_THLD ) ||
             (context.qam_ldpc == 4 && cnt1 >= MCS4_DOWN_ERR_THLD ) || 
             (context.qam_ldpc == 5 && cnt1 >= MCS5_DOWN_ERR_THLD ) )
        {
            updown = QAMDOWN;
            //dlog_info( "LDPC DOWN: %d %d", context.qam_ldpc, cnt1 );
        }
    }

    return updown;
}




//call after channel change, and qam change
void grd_start_SnrHarqCnt( void )
{
    work_ch_snr.u8_isFull = 0;
    work_ch_snr.row_index = 0;

    stru_harqStatus.u8_isFull = 0;   
    stru_harqStatus.u8_idx  = 0;
    
    stru_ldpcErr.u8_isFull = 0;
    stru_ldpcErr.u8_idx = 0;   
}


void grd_judge_qam_mode(void)
{
    QAMUPDONW snr_qamupdown  = QAMKEEP;
    QAMUPDONW harq_qamupdown = QAMKEEP;
    QAMUPDONW ldpc_qamupdown = QAMKEEP;
    uint8_t thresh = ((BW_20M == (context.e_bandwidth)) ? (QAM_CHANGE_THRESHOLD_COUNT - 2) : (QAM_CHANGE_THRESHOLD_COUNT - 1));
    

    grd_get_snr();
    snr_qamupdown = snr_static_for_qam_change( context.qam_threshold_range[context.qam_ldpc][0], 
                                               context.qam_threshold_range[context.qam_ldpc][1]);
    if ( context.qam_ldpc == 0)
    {
        harq_qamupdown = harq_static_for_qam_up();
    }

    ldpc_qamupdown = ldpc_static_for_qam_change();
    if (context.qam_skip_mode == MANUAL)
    {
        static int loop = 0;
        if ( loop ++ > 1000)
        {
            dlog_info("-BRC MANUAL-");
            loop = 0;
        }

        return;
    }

        if( (snr_qamupdown == QAMUP ) && ( context.qam_ldpc < thresh) && ( ( harq_qamupdown == QAMUP) || (ldpc_qamupdown == QAMUP) ) )
        {
            context.qam_ldpc++;
        }
        else if ( ( QAMDOWN == snr_qamupdown || QAMDOWN == ldpc_qamupdown )  //SNR or LDPC error 
                   && context.qam_ldpc > context.u8_bbStartMcs)
        {
            context.qam_ldpc--;
        }
        else
        {
            return;
        }
    
    grd_set_txmsg_mcs_change(context.e_bandwidth, context.qam_ldpc);
    grd_start_SnrHarqCnt( );
}
