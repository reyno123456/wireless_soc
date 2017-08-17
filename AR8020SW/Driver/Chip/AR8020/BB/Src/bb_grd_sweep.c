#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "systicks.h"
#include "debuglog.h"
#include "bb_ctrl_internal.h"
#include "bb_grd_sweep.h"
#include "bb_regs.h"
#include "log10.h"


#define SWEEP_FREQ_BLOCK_ROWS           (6)

#define BW_10M_START_TH                 (12)
#define BW_10M_END_TH                   (19)
#define BW_10M_MAX_CH_CNT               (BW_10M_END_TH - BW_10M_START_TH + 1)
#define BW_10M_VALID_CH_CNT             (BW_10M_MAX_CH_CNT - 2)
#define BW_10M_VALID_1_5M_CH_START      (BW_10M_START_TH + 1)


#define BW_20M_START_TH                 (8)
#define BW_20M_END_TH                   (23)
#define BW_20M_MAX_CH_CNT               (BW_20M_END_TH - BW_20M_START_TH + 1)
#define BW_20M_VALID_CH_CNT             (BW_20M_MAX_CH_CNT - 4)
#define BW_20M_VALID_1_5M_CH_START      (BW_20M_START_TH + 2)


#define NEXT_NUM(cur, max)              ( ((cur + 1) >= max) ? 0: (cur + 1) )

#define AGC_LEVEL_0                     (0x5f)  //-90dmb
#define AGC_LEVEL_1                     (0x64)  //-96dmb

#define MAX(a,b) (((a) > (b)) ?  (a) :  (b) )
#define MIN(a,b) (((a) < (b)) ?  (a) :  (b) )

#define CLEAR_MAIN_CNT  (1000)
#define GRD_AGC_BLOCK_THRESH            (0x4c)

#define GRD_BACK_TO_2G_UNLOCK_CNT       (5)


static int flaglog = 0;

typedef enum
{
    EQUAL_2G_5G = 0,
    BETTER_2G   = 1,
    BETTER_5G   = 2,
}ENUM_RF_select;


typedef struct
{
    ENUM_RF_BAND e_bandsupport;

    //////////////////
    int32_t         i32_rf0Pwr[SWEEP_FREQ_BLOCK_ROWS][MAX_IT_FRQ_SIZE];
    int32_t         i32_rf0PwrAvr[MAX_IT_FRQ_SIZE];

    //////////////////
    int32_t         i32_rf1Pwr[SWEEP_FREQ_BLOCK_ROWS][MAX_IT_FRQ_SIZE];
    int32_t         i32_rf1PwrAvr[MAX_IT_FRQ_SIZE];

    ENUM_RF_BAND e_curBand;
    ENUM_CH_BW   e_bw;
    uint8_t      u8_mainCh;          //current VT channel
    uint8_t      u8_mainSweepRow;
    uint8_t      u8_mainSweepCh;

    uint8_t      u8_optCh;           //optional VT channel
    uint8_t      u8_optSweepRow;
    uint8_t      u8_optSweepCh;
    uint8_t      u8_bestBb1ChCnt[MAX_IT_FRQ_SIZE];
    uint8_t      u8_bestBb2ChCnt[MAX_IT_FRQ_SIZE];

    uint8_t      u8_spareSweepCh;      //channel number
    uint8_t      u8_optBandSweepCh;  //use to sweep another band channel.

    uint8_t      u8_curBb1Row;
    uint8_t      u8_curBb2Row;
    ENUM_RF_BAND e_prevSweepBand;      //previous sweep band
    uint8_t      u8_prevSweepCh;     //previous sweep channel, main channel and optional channel may change

    uint8_t      u8_cycleCnt;
    uint8_t      u8_totalCyc;
    uint8_t      u8_preMainCh;
    uint16_t     u16_preMainCount;   //if cycle >= u16_preMainCount, clear the u8_preMainCh

    ENUM_RF_select band_sel[15];
    uint8_t      u8_bandSelCnt;
    uint8_t      u8_isFull;
    uint8_t      u8_bb1ItFrqSize;
    uint8_t      u8_bb2ItFrqSize;
} STRU_SWEEP_NOISE_POWER;


STRU_SWEEP_NOISE_POWER stru_sweepPower;


static int calc_power_db(ENUM_RF_BAND e_rfBand, uint8_t bw, uint32_t power_td,
                         int16_t *power_fd, int32_t *power_sum, 
                         uint8_t cnt, uint8_t sweep_ch, int flaglog);

static void calc_average_and_fluct(ENUM_RF_BAND e_rfBand, uint8_t u8_ItCh);

static uint8_t BB_GetSweepTotalCh(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw);

static uint8_t BB_UpdateOptCh(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw, uint8_t sweep_ch);

static uint8_t BB_JudgeAdjacentFrequency(uint8_t judge_ch);

static void BB_GetItMinMaxCh(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw, uint8_t *min_ch, uint8_t *max_ch);

static void BB_GetItAdjacentFrequency(uint8_t ch, uint8_t *pre, uint8_t *next);

ENUM_RF_select BB_grd_cmpBandNoise( void );



/*
 * to start sweep
 */
void BB_SweepStart(ENUM_RF_BAND e_bandsupport, ENUM_CH_BW e_bw)
{
    stru_sweepPower.u8_spareSweepCh =  0;
    stru_sweepPower.u8_optBandSweepCh = 0;
    stru_sweepPower.u8_curBb1Row   =  0;
    stru_sweepPower.u8_curBb2Row   =  0;
    stru_sweepPower.u8_isFull     =  0;
    stru_sweepPower.u8_cycleCnt   =  0;
    stru_sweepPower.u8_mainSweepRow =  0;
    stru_sweepPower.u8_optSweepRow  =  0;
    stru_sweepPower.u8_preMainCh  =  0xff;
    stru_sweepPower.u8_mainCh     =  0xff;
    stru_sweepPower.e_bw          =  e_bw;
    stru_sweepPower.u8_prevSweepCh  = 0x0;
    stru_sweepPower.u16_preMainCount= 0x0;

    stru_sweepPower.e_bandsupport = context.e_bandsupport;
    stru_sweepPower.u8_totalCyc =  (stru_sweepPower.e_bandsupport == RF_2G_5G) ? 0x04 : 0x03;

    stru_sweepPower.u8_bandSelCnt  =  0;
    
    stru_sweepPower.u8_bb1ItFrqSize = 0;
    stru_sweepPower.u8_bb2ItFrqSize = 0;
    stru_sweepPower.e_curBand   =  RF_2G;
    if (RF_2G == e_bandsupport)
    {
        stru_sweepPower.u8_bb1ItFrqSize = BB_GetItFrqNum(RF_2G);
    }
    else if (RF_5G == e_bandsupport)
    {
        stru_sweepPower.u8_bb2ItFrqSize = BB_GetItFrqNum(RF_5G);
        stru_sweepPower.e_curBand   =  RF_5G;
    }
    else if (RF_2G_5G == e_bandsupport)
    {
        stru_sweepPower.u8_bb1ItFrqSize = BB_GetItFrqNum(RF_2G);
        stru_sweepPower.u8_bb2ItFrqSize = BB_GetItFrqNum(RF_5G);
    }
    else if (RF_600M == e_bandsupport)
    {
        stru_sweepPower.u8_bb1ItFrqSize = BB_GetItFrqNum(RF_600M);
        stru_sweepPower.e_curBand   =  RF_600M;
    }
    else
    {
        ;
    }
    stru_sweepPower.e_prevSweepBand = stru_sweepPower.e_curBand;
    
    BB_set_SweepFrq( stru_sweepPower.e_curBand, e_bw, 0 );   
}



void BB_GetSweepNoise(int16_t *ptr_noise_power)
{
    uint8_t col;
    uint8_t i;
    int16_t value;

    for(col = 0; col < stru_sweepPower.u8_bb1ItFrqSize; col++)
    {
        value = (int16_t)(stru_sweepPower.i32_rf0PwrAvr[col]);
        for(i = 0; i < 8; i++)
        {
            ptr_noise_power[(col * 8) + i] = value;
        }
    }

    for(col = 0; col < stru_sweepPower.u8_bb2ItFrqSize; col++)
    {
        value = (int16_t)(stru_sweepPower.i32_rf1PwrAvr[col]);
        for(i = 0; i < 8; i++)
        {
            ptr_noise_power[(col + stru_sweepPower.u8_bb1ItFrqSize) * 8 + i] = value;
        }
    }
}


/*
 * adapt to AR8020 new sweep function
 * return 0: sweep fail. Do not switch to next sweep channel
 * return 1: sweep OK.
*/
static uint8_t BB_GetSweepPower(ENUM_RF_BAND e_rfBand, uint8_t bw, uint8_t row, uint8_t sweep_ch, uint8_t flag)
{
    uint8_t  num = 0;
    uint8_t  i   = 0;
    uint8_t  ret = 0;
    uint8_t  offset = sweep_ch;

    uint16_t ch_fd_power[16];
    uint8_t  power_fd_addr = 0x60;
    uint8_t u8_maxCh = BB_GetSweepTotalCh( e_rfBand, bw);
                                       
    if(sweep_ch >= u8_maxCh)
    {
        dlog_error("sweepCh Error:%d %d %d", e_rfBand, sweep_ch, u8_maxCh);
    }

    //get the time domain noise power
    uint32_t power_td =  (((uint32_t)(BB_ReadReg(PAGE2, SWEEP_ENERGY_HIGH)) << 16) |
                                     (BB_ReadReg(PAGE2, SWEEP_ENERGY_MID) << 8)  |
                                     (BB_ReadReg(PAGE2, SWEEP_ENERGY_LOW)));
    
    if(power_td == 0)
    {
        return 0;
    }



    num = ((bw == BW_10M) ? BW_10M_MAX_CH_CNT : BW_20M_MAX_CH_CNT);
    power_fd_addr += ( ((bw == BW_10M) ? BW_10M_END_TH : BW_20M_END_TH ) * 2 /*2 byts each snr */ );

    for(i = 0; i < num; i++)
    {
        ch_fd_power[i] = (((uint16_t)BB_ReadReg(PAGE3, power_fd_addr) << 8) | BB_ReadReg(PAGE3, power_fd_addr+1));
        power_fd_addr -= 2;
        if(ch_fd_power[i] == 0)
        {
            return 0;
        }
    }

    flaglog = 0;

    int32_t *ps32_power = (e_rfBand == RF_5G) ? stru_sweepPower.i32_rf1Pwr[row] : stru_sweepPower.i32_rf0Pwr[row];
                                                     
    ret = calc_power_db(e_rfBand, bw, power_td, 
                        ch_fd_power,  ps32_power,
                        num, sweep_ch, flag);

    num = BB_GetItFrqNum(e_rfBand);
    for(i = 0; i < num; i++)
    {
        calc_average_and_fluct(e_rfBand, i);
    }


    return ret;
}


int32_t BB_Sweep_updateCh(ENUM_RF_BAND e_rfBand, uint8_t mainch)
{
    uint8_t opt;
    stru_sweepPower.u8_preMainCh     = stru_sweepPower.u8_mainCh;
    stru_sweepPower.u16_preMainCount = 0; //clear after 1000 cycles

    stru_sweepPower.u8_mainSweepCh = (BW_10M == (stru_sweepPower.e_bw)) ? (mainch) : (mainch / 2);
    stru_sweepPower.u8_mainCh      = mainch;

    //re-select the option channel
    BB_selectBestCh(e_rfBand, SELECT_OPT, NULL, &opt, NULL, 0);

    stru_sweepPower.u8_optCh = opt;
    stru_sweepPower.u8_optSweepCh = (BW_10M == (stru_sweepPower.e_bw)) ? (opt) : (opt / 2);

    memset(stru_sweepPower.u8_bestBb1ChCnt, 0, sizeof(stru_sweepPower.u8_bestBb1ChCnt));
    memset(stru_sweepPower.u8_bestBb2ChCnt, 0, sizeof(stru_sweepPower.u8_bestBb2ChCnt));
    //dlog_info("update main opt: %d %d", mainch, opt);    
    return 0;
}


static int BB_SweepBeforeFull( void )
{
    uint8_t u8_maxCh = BB_GetSweepTotalCh( stru_sweepPower.e_curBand, stru_sweepPower.e_bw);
    uint8_t result = 0;
    uint8_t curRow = (stru_sweepPower.e_curBand == RF_5G) ? (stru_sweepPower.u8_curBb2Row) : (stru_sweepPower.u8_curBb1Row);
    
    result   = BB_GetSweepPower( stru_sweepPower.e_curBand,
                                 stru_sweepPower.e_bw,
                                 curRow,
                                 stru_sweepPower.u8_prevSweepCh,
                                 0 );

    if( result )
    {
        uint8_t nextch = NEXT_NUM(stru_sweepPower.u8_prevSweepCh, u8_maxCh);
        if ( nextch < stru_sweepPower.u8_prevSweepCh )
        {
            uint8_t flag = 0;
            
            if ( stru_sweepPower.e_curBand == RF_5G )
            {
                stru_sweepPower.u8_curBb2Row = NEXT_NUM(stru_sweepPower.u8_curBb2Row, SWEEP_FREQ_BLOCK_ROWS);
                if ( stru_sweepPower.u8_curBb2Row == 0)
                {
                    stru_sweepPower.u8_isFull |= 0x01;
                }
            }
            else
            {
                stru_sweepPower.u8_curBb1Row = NEXT_NUM(stru_sweepPower.u8_curBb1Row, SWEEP_FREQ_BLOCK_ROWS);
                if ( stru_sweepPower.u8_curBb1Row == 0)
                {
                    stru_sweepPower.u8_isFull |= 0x01;
                }
            }

            if( stru_sweepPower.u8_isFull )
            {
                uint8_t mainch = 0, opt = 0;

                BB_selectBestCh( stru_sweepPower.e_curBand, SELECT_MAIN_OPT, &mainch, &opt, NULL, 0 );

                stru_sweepPower.u8_mainSweepCh = (BW_10M == (stru_sweepPower.e_bw)) ? (mainch) : (mainch / 2);
                stru_sweepPower.u8_mainCh      = mainch;
                
                stru_sweepPower.u8_prevSweepCh = stru_sweepPower.u8_mainSweepCh;

                stru_sweepPower.u8_optCh = opt;
                stru_sweepPower.u8_optSweepCh = (BW_10M == (stru_sweepPower.e_bw)) ? (opt) : (opt / 2);
                stru_sweepPower.u8_cycleCnt = MAIN_CH_CYCLE;

                return result;
            }
            else
            {
                stru_sweepPower.u8_prevSweepCh = nextch;
            }
        }
        else
        {
            stru_sweepPower.u8_prevSweepCh = nextch;
        }

        BB_set_SweepFrq( stru_sweepPower.e_curBand, stru_sweepPower.e_bw, stru_sweepPower.u8_prevSweepCh );
    }

    return result;
}


static int BB_set_sweepChannel( void )
{
    uint8_t u8_maxCh;
    uint8_t cycle = (stru_sweepPower.u8_cycleCnt % stru_sweepPower.u8_totalCyc);
    ENUM_RF_BAND e_curBand = stru_sweepPower.e_curBand;
    ENUM_CH_BW   e_bw      = stru_sweepPower.e_bw;

    if( cycle == OTHER_CH_CYCLE )
    {
        u8_maxCh = BB_GetSweepTotalCh( e_curBand, e_bw);
        uint8_t u8_nextch = 0;
        do
        {
            u8_nextch = NEXT_NUM(stru_sweepPower.u8_spareSweepCh, u8_maxCh);
            if( u8_nextch < stru_sweepPower.u8_spareSweepCh )
            {
                if ( e_curBand == RF_5G )    //5G mode: switch to 5G next row
                {
                    stru_sweepPower.u8_curBb2Row = NEXT_NUM(stru_sweepPower.u8_curBb2Row, SWEEP_FREQ_BLOCK_ROWS);
                }
                else
                {
                    stru_sweepPower.u8_curBb1Row = NEXT_NUM(stru_sweepPower.u8_curBb1Row, SWEEP_FREQ_BLOCK_ROWS);
                }
            }
            stru_sweepPower.u8_spareSweepCh = u8_nextch;
        } while( u8_nextch == stru_sweepPower.u8_mainSweepCh || u8_nextch == stru_sweepPower.u8_optSweepCh );

        stru_sweepPower.e_prevSweepBand  = e_curBand;
        stru_sweepPower.u8_prevSweepCh = stru_sweepPower.u8_spareSweepCh;
    }
    else if( cycle == MAIN_CH_CYCLE )
    {
        stru_sweepPower.e_prevSweepBand  = e_curBand;
        stru_sweepPower.u8_prevSweepCh   = stru_sweepPower.u8_mainSweepCh;
        stru_sweepPower.u8_mainSweepRow  = NEXT_NUM(stru_sweepPower.u8_mainSweepRow, SWEEP_FREQ_BLOCK_ROWS);
    }
    else if( cycle == OPT_CH_CYCLE )
    {
        stru_sweepPower.e_prevSweepBand  = e_curBand;
        stru_sweepPower.u8_prevSweepCh   = stru_sweepPower.u8_optSweepCh;
        stru_sweepPower.u8_optSweepRow   = NEXT_NUM(stru_sweepPower.u8_optSweepRow, SWEEP_FREQ_BLOCK_ROWS);    
    }
    else if( cycle == OTHER_BAND_CH_CYCLE )  // if RF_2G or RF_5G only, no OTHER_BAND_CH_CYCLE
    {
        ENUM_RF_BAND newband = OTHER_BAND( stru_sweepPower.e_curBand );
        u8_maxCh = BB_GetSweepTotalCh(newband, e_bw);

        stru_sweepPower.u8_optBandSweepCh = NEXT_NUM( stru_sweepPower.u8_optBandSweepCh, u8_maxCh ); 
        stru_sweepPower.u8_prevSweepCh      = stru_sweepPower.u8_optBandSweepCh;
        stru_sweepPower.e_prevSweepBand     = newband;

        if ( stru_sweepPower.u8_prevSweepCh == 0 ) //start channel from 0, new row.
        {
            if ( newband == RF_5G )
            {
                stru_sweepPower.u8_curBb2Row = NEXT_NUM(stru_sweepPower.u8_curBb2Row, SWEEP_FREQ_BLOCK_ROWS);
            }
            else
            {
                stru_sweepPower.u8_curBb1Row = NEXT_NUM(stru_sweepPower.u8_curBb1Row, SWEEP_FREQ_BLOCK_ROWS);
            }
        }
    }

    BB_set_SweepFrq( stru_sweepPower.e_prevSweepBand, e_bw, stru_sweepPower.u8_prevSweepCh );

    {
        STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);
        uint8_t shift = (e_curBand != RF_5G) ? 0 : BB_GetItFrqNum(RF_2G);
        {
            static uint8_t itch_bak  = 0;
            static uint8_t itopt_bak = 0;
            osdptr->IT_channel  = stru_sweepPower.u8_mainCh + shift;
            osdptr->u8_optCh    = stru_sweepPower.u8_optCh  + shift;
            if ( osdptr->IT_channel != itch_bak ||  osdptr->u8_optCh != itopt_bak )
            {
                //dlog_info("From:(%d %d) To:(%d %d)", itch_bak, itopt_bak, osdptr->IT_channel, osdptr->u8_optCh);
                itch_bak  = osdptr->IT_channel;
                itopt_bak = osdptr->u8_optCh;
            }
        }
    }

    return 0;
}


static int BB_SweepAfterFull( uint8_t flag )
{
    ENUM_RF_BAND e_curBand   = stru_sweepPower.e_curBand;
    ENUM_RF_BAND e_sweepBand = stru_sweepPower.e_prevSweepBand;
    ENUM_CH_BW   e_bw = stru_sweepPower.e_bw;

    uint8_t result   = 0;
    uint8_t cycle  = (stru_sweepPower.u8_cycleCnt % stru_sweepPower.u8_totalCyc);
    uint8_t flag_checkBandSwitch = 0;

    uint8_t u8_mainSweepCh = stru_sweepPower.u8_mainSweepCh;
    uint8_t u8_optSweepCh  = stru_sweepPower.u8_optSweepCh;
    uint8_t u8_prevSweepCh = stru_sweepPower.u8_prevSweepCh;

    uint8_t u8_sweepRow;
    if( e_sweepBand == e_curBand && u8_prevSweepCh == u8_mainSweepCh )  //main channel sweep result
    {
        u8_sweepRow = stru_sweepPower.u8_mainSweepRow;    
    }
    else if ( e_sweepBand == e_curBand && u8_prevSweepCh == u8_optSweepCh ) //opt channel sweep result
    {
        u8_sweepRow = stru_sweepPower.u8_optSweepRow;    
    }
    else
    {  
        if ( e_sweepBand == RF_5G )
        {
            u8_sweepRow = stru_sweepPower.u8_curBb2Row;
        }
        else
        {
            u8_sweepRow = stru_sweepPower.u8_curBb1Row;
        }

        if ( e_sweepBand != e_curBand &&  (u8_prevSweepCh + 1) == BB_GetSweepTotalCh( e_sweepBand, e_bw ) ) //last channel
        {
            if ( (u8_sweepRow + 1) == SWEEP_FREQ_BLOCK_ROWS && ( stru_sweepPower.u8_isFull & 0x10 )!= 0x10 ) //last row
            {
                stru_sweepPower.u8_isFull |= 0x10;
                dlog_info("option band sweep is full !!");
            }

            //both band sweep is ready, and sweep last channel
            if ( (stru_sweepPower.u8_isFull & 0x10) && (context.locked) && 
                 (u_grdRcIdSearchStatus.stru_rcIdStatus.u8_groundSearchMode==GROUND_REQUEST_LOCK) && context.e_rfbandMode == AUTO) 
            {
                flag_checkBandSwitch = 1;
            }
        }
    }
    result = BB_GetSweepPower( e_sweepBand, e_bw, u8_sweepRow, u8_prevSweepCh, flag ); 
    if ( 0 == result )
    {
        return 0;
    }
    BB_UpdateOptCh(e_sweepBand, e_bw, u8_prevSweepCh);

    //check if band switch need
    if (flag_checkBandSwitch && context.e_rfbandMode == AUTO)
    {
        uint8_t i = 0;
        uint8_t flag_bandChange = 0, mainch = 0, optch = 0;
        ENUM_RF_BAND optband;

        stru_sweepPower.band_sel[stru_sweepPower.u8_bandSelCnt] = BB_grd_cmpBandNoise();
        stru_sweepPower.u8_bandSelCnt ++;

        //check whether switch is necessary
        uint8_t cnt = sizeof(stru_sweepPower.band_sel);

        if ( stru_sweepPower.u8_bandSelCnt >= cnt )
        {
            uint8_t better_2g_count = 0;
            uint8_t better_5g_count = 0;

            for( i = 0; i < cnt; i++)
            {
                better_2g_count += ( stru_sweepPower.band_sel[i] == BETTER_2G );
                better_5g_count += ( stru_sweepPower.band_sel[i] == BETTER_5G );
            }

            if ( stru_sweepPower.e_curBand == RF_2G && better_5g_count * 3 > (cnt << 1) )
            {
                flag_bandChange = 1;
                optband = RF_5G;
                dlog_info("sweep: change to 5G");
            }
            else if ( stru_sweepPower.e_curBand == RF_5G && better_2g_count * 3 > (cnt << 1) )
            {
                flag_bandChange = 1;
                optband = RF_2G;
                dlog_info("sweep: change to 2G");
            }

            if ( 1 == flag_bandChange && 1 == context.locked )
            {
                BB_selectBestCh( optband, SELECT_MAIN_OPT, &mainch, &optch, NULL, 0 );    //get best main channel
                grd_notifyRfbandChange( optband, mainch, optch, BAND_CHANGE_DELAY );      //start delay and notify sky to change
            }
            
            stru_sweepPower.u8_bandSelCnt = 0;
        }
    }
    //if get the right cycle result, do next cycle sweep
    if( result )
    {
        stru_sweepPower.u8_cycleCnt ++;
        BB_set_sweepChannel();
    }

    return result;
}


uint8_t BB_GetSweepedChResult( uint8_t flag )
{
    uint8_t  ret = 0;
    if ( !stru_sweepPower.u8_isFull )
    {
        ret = BB_SweepBeforeFull( );
    }
    else
    {
        ret = BB_SweepAfterFull( flag );
    }
    
    if ( stru_sweepPower.u8_preMainCh != 0xFF && stru_sweepPower.u16_preMainCount++ >= CLEAR_MAIN_CNT )
    {
        //clear the premain;
        //stru_sweepPower.u16_preMainCount = 0;
        //stru_sweepPower.u8_preMainCh     = 0xFF;
    }
    

    return ret;
}

#if 0
/*
 *  opt: 0:         Force main channel 
 *       1:         Force opt channel 
 *       other:     do not care
*/
uint8_t BB_forceSweep( uint8_t opt )
{
    if( opt == MAIN_CH_CYCLE || opt == OPT_CH_CYCLE)
    {
        stru_sweepPower.u8_cycleCnt    = opt;
        stru_sweepPower.u8_prevSweepCh = (opt == MAIN_CH_CYCLE) ? stru_sweepPower.u8_mainSweepCh : 
                                                                  stru_sweepPower.u8_optSweepCh;
        return BB_set_SweepFrq( stru_sweepPower.e_curBand, stru_sweepPower.u8_prevSweepCh );
    }

    return 0;
}
#endif


int32_t BB_CompareCh1Ch2ByPowerAver(ENUM_RF_BAND e_rfBand, uint8_t u8_itCh1, uint8_t u8_itCh2, int32_t level)
{
    int32_t value = 0;

    int32_t * pi32_power_average = (e_rfBand == RF_5G) ? stru_sweepPower.i32_rf1PwrAvr :
                                                         stru_sweepPower.i32_rf0PwrAvr;
                                                
    if (pi32_power_average[u8_itCh1] < (pi32_power_average[u8_itCh2] - level))
    {
        value = 1;
    }
    
    return value;
}

int32_t BB_CompareCh1Ch2ByPower(ENUM_RF_BAND e_rfBand, uint8_t u8_itCh1, uint8_t u8_itCh2, uint8_t u8_cnt)
{
    int32_t value = 0;
    uint8_t row;
    uint8_t tmpCnt = 0;
    int32_t *pu32_power;

    if (u8_cnt > SWEEP_FREQ_BLOCK_ROWS)
    {
        u8_cnt = SWEEP_FREQ_BLOCK_ROWS;
    }

    for( row = 0; row < SWEEP_FREQ_BLOCK_ROWS; row++)
    {
        pu32_power = (e_rfBand == RF_5G) ? stru_sweepPower.i32_rf1Pwr[row] : 
                                           stru_sweepPower.i32_rf0Pwr[row];
        
        tmpCnt += ((pu32_power[u8_itCh1] < pu32_power[u8_itCh2]) ? (1) : (0));
    }
    
    value = ((tmpCnt >= u8_cnt) ? (1) : (0));

    return value;
}


static uint8_t is_inlist(uint8_t item, uint8_t *pu8_exlude, uint8_t u8_cnt)
{
    uint8_t flag = 0;
    uint8_t i;
    for ( i = 0; i < u8_cnt && flag == 0; i++) //channel in the excluding list
    {
        flag = ( item == pu8_exlude[i] );
    }

    return flag;
}

static uint8_t find_best(uint8_t *pu8_exlude, uint8_t u8_exclude_cnt, ENUM_RF_BAND e_rfBand, uint8_t log)
{
    uint8_t u8_start = 0;
    uint8_t ch = 0;
    int16_t aver, fluct;
    uint8_t num = BB_GetItFrqNum(e_rfBand);


    for ( u8_start; 
          u8_start < num && is_inlist( u8_start, pu8_exlude, u8_exclude_cnt );  //channel in the excluding list
          u8_start++) 
    {}

    
    for( ch; ch < num; ch++)
    {
        if ( !is_inlist( ch, pu8_exlude, u8_exclude_cnt ) )
        {
            if(BB_CompareCh1Ch2ByPowerAver(e_rfBand, ch, u8_start, CMP_POWER_AVR_LEVEL))
            {
                u8_start = ch;
            }
        }
    }

    return u8_start;
}

static void BB_GetItMinMaxCh(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw, uint8_t *min_ch, uint8_t *max_ch)
{
    if (BW_10M == e_bw)
    {
        *max_ch = BB_GetItFrqNum(e_rfBand) - 1;
        *min_ch = 0;
    }
    else // 20M
    {
        *max_ch = BB_GetItFrqNum(e_rfBand) - 2;
        *min_ch = 1;
    }
}

static void BB_GetItAdjacentFrequency(uint8_t ch, uint8_t *pre, uint8_t *next)
{
    *pre  = ch - 1;
    *next = ch + 1;
}


/*
 * u8_opt: 
 *         SELECT_OPT: do not change the main channel. only select one optional channel
 *         SELECT_MAIN_OPT: select the main and opt channel
 *         CHANGE_MAIN: change the main channel, and select another one
 * return: 
 *         0: no suggest main and opt channel
 *         1: get main and opt channel
*/
uint8_t BB_selectBestCh(ENUM_RF_BAND e_band, ENUM_CH_SEL_OPT e_opt,
                        uint8_t *pu8_mainCh, uint8_t *pu8_optCh, uint8_t *pu8_other,
                        uint8_t log)
{
    uint8_t  ch = 0;
    int16_t  aver, fluct;
    uint32_t start = SysTicks_GetTickCount();
    uint8_t  exclude[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t  excludecnt = 0;
    ENUM_CH_BW e_bw = stru_sweepPower.e_bw;
    uint8_t bestCh  = 0xff;         //select one from all the channels
    uint8_t betterCh= 0xff;
    uint8_t other   = 0xff;
    //uint8_t index = 3;
    uint8_t min_ch;
    uint8_t max_ch;
    uint8_t pre_ch;
    uint8_t next_ch;
    uint8_t tmpCh;
    
    if( !stru_sweepPower.u8_isFull )
    {
        return 0;
    }
    
    BB_GetItMinMaxCh(e_band, e_bw, &min_ch, &max_ch);
    
    //select main channel
    if( e_opt == SELECT_MAIN_OPT )
    {
        if (BW_20M == (stru_sweepPower.e_bw))
		{
			exclude[0] = 0;
            exclude[1] = max_ch + 1;
            excludecnt = 2;
		}
        else
		{
		    excludecnt = 0;
		}
        bestCh = find_best(exclude, excludecnt, e_band, log);
        if ( pu8_mainCh )
        {
            *pu8_mainCh = bestCh;
        }
    }

    //select optional channel
    if ( e_opt == SELECT_MAIN_OPT || e_opt == SELECT_OPT )
    {
        exclude[0] = stru_sweepPower.u8_preMainCh;
        exclude[1] = stru_sweepPower.u8_mainCh;
        exclude[2] = bestCh;
        
        tmpCh = (e_opt == SELECT_OPT) ? (stru_sweepPower.u8_mainCh) : (bestCh);
        BB_GetItAdjacentFrequency(tmpCh, &pre_ch, &next_ch);
        exclude[3] = pre_ch;
        exclude[4] = next_ch;
        excludecnt = 5;

        if ( log )
        {
            dlog_info("exclude: %d %d %d", exclude[0], exclude[1], exclude[2]);
        }

        betterCh = find_best(exclude, excludecnt, e_band, log);

        if ((pre_ch >= min_ch) && (pre_ch <= max_ch))
        {
            //if (BB_CompareCh1Ch2ByPower(e_band, pre_ch, betterCh, SWEEP_FREQ_BLOCK_ROWS))
            if (BB_CompareCh1Ch2ByPowerAver(e_band, pre_ch, betterCh, 0))
            {
                betterCh = pre_ch;
            }
        }
        if ((next_ch >= min_ch) && (next_ch <= max_ch))
        {
            //if (BB_CompareCh1Ch2ByPower(e_band, next_ch, betterCh, SWEEP_FREQ_BLOCK_ROWS))
            if (BB_CompareCh1Ch2ByPowerAver(e_band, pre_ch, betterCh, 0))
            {
                betterCh = next_ch;
            }
        }
        
        if ( pu8_optCh )
        {
            *pu8_optCh= betterCh;
        }
    }

    if ( e_opt == SELECT_OTHER )
    {
        exclude[0] = stru_sweepPower.u8_preMainCh;
        exclude[1] = stru_sweepPower.u8_mainCh;
        exclude[2] = stru_sweepPower.u8_optCh;   
        excludecnt = 3;

        other = find_best(exclude, excludecnt, e_band, log);

        if (pu8_other)
        {
            *pu8_other = other;
        }
    }

    if ( log )
    {
        dlog_info("--ch--: %d %d %d", bestCh, betterCh, e_opt);
    }    


    return 1;
}



uint8_t get_opt_channel( void )
{
    uint8_t other;
    int32_t level;
    uint8_t ret = stru_sweepPower.u8_optCh;
    ENUM_RF_BAND e_band = stru_sweepPower.e_curBand;

    BB_selectBestCh(e_band, SELECT_OTHER, NULL, NULL, &other, 0);
    
    if ( context.agclevel >= AGC_LEVEL_1 )
    {
        level = 3;
    }
    else if( context.agclevel >= AGC_LEVEL_0 )
    {
        level = 6;
    }
    else
    {
        level = 9;
    }

    level = ((1 == BB_JudgeAdjacentFrequency(other)) ? (2 * level) : (level));
    //if (BB_CompareCh1Ch2ByPower(e_band, other, stru_sweepPower.u8_optCh, SWEEP_FREQ_BLOCK_ROWS))
    if (BB_CompareCh1Ch2ByPowerAver(e_band, other, stru_sweepPower.u8_optCh, 0))
    {
        ret = other; 
    }

    return ret;
}


/*
 * get the dbm noise power
 * power_td: total power in time domain
 * power_fd: each 1.56M bandwidth noise
 * power_db: the power in dbm in each 1.56M
*/
static int calc_power_db(ENUM_RF_BAND e_rfBand, uint8_t bw, uint32_t power_td,
                         int16_t *power_fd, int32_t *power_sum, 
                         uint8_t cnt, uint8_t sweep_ch, int flaglog)
{
    uint8_t  i = 0;
    uint8_t  offset = 0;
    uint8_t   tmpValue = 0;

#if 1
    // calcation the power average by frequency-domain
    uint64_t sum_fd = 0;
    for(i = 0 ; i < cnt; i++)
    {
        sum_fd += ((uint64_t)power_fd[i] * power_fd[i]);
    }

    uint64_t sum_fd1 = 0;
    uint64_t sum_fd2 = 0;
    if (BW_10M == bw)
    {
        offset = sweep_ch;
        for(i = 1 ; i < 7; i++)
        {
            sum_fd1 += ((uint64_t)power_fd[i] * power_fd[i]);
        }
        sum_fd1 = sum_fd1 * power_td / sum_fd;
        power_sum[offset] = get_10log10(sum_fd1);
        power_sum[offset] = BB_SweepEnergyCompensation(power_sum[offset]);
    }
    else // 20M
    {
        offset = sweep_ch * 2;
        for(i = 2 ; i < 8; i++)
        {
            sum_fd1 += ((uint64_t)power_fd[i] * power_fd[i]);
        }
        sum_fd1 = sum_fd1 * power_td / sum_fd;
        
        
        for(i = 8 ; i < 14; i++)
        {
            sum_fd2 += ((uint64_t)power_fd[i] * power_fd[i]);
        }
        sum_fd2 = sum_fd2 * power_td / sum_fd;

        power_sum[offset] = get_10log10(sum_fd1);
        power_sum[offset] = BB_SweepEnergyCompensation(power_sum[offset]);

        tmpValue = BB_GetItFrqNum(e_rfBand);
        if ((tmpValue / 2) != sweep_ch) // odd number frequency do not need update
        {
            power_sum[offset + 1] = get_10log10(sum_fd2);
            power_sum[offset + 1] = BB_SweepEnergyCompensation(power_sum[offset + 1]);
        }
    }
#else
    // calcation the power average by time-domain

    int tmp = power_td;
    tmp = get_10log10(tmp);
    tmp += ((tmp > 30) ? (-123) : (-125));

    for(i = 0 ; i < cnt; i++)
    {
        power_db[i] = tmp;
    }
#endif

    return 1;
}

static void calc_average_and_fluct(ENUM_RF_BAND e_rfBand, uint8_t u8_ItCh)
{
    uint16_t row = 0;
    int32_t *pu32_power;
    int32_t *pu32_power_average;

    pu32_power_average = (e_rfBand == RF_5G) ? stru_sweepPower.i32_rf1PwrAvr : 
                                               stru_sweepPower.i32_rf0PwrAvr;  
    pu32_power_average[u8_ItCh] = 0;
    for( row = 0; row < SWEEP_FREQ_BLOCK_ROWS; row++)
    {
        pu32_power = (e_rfBand == RF_5G) ? stru_sweepPower.i32_rf1Pwr[row] : 
                                           stru_sweepPower.i32_rf0Pwr[row];
        pu32_power_average[u8_ItCh] += pu32_power[u8_ItCh];
    }
    pu32_power_average[u8_ItCh] /= SWEEP_FREQ_BLOCK_ROWS;
}

static uint8_t BB_GetSweepTotalCh(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw)
{
    uint8_t value = 0;
    if (RF_2G == e_rfBand)
    {
        value = MAX_2G_IT_FRQ_SIZE;
    }
    else if (RF_600M == e_rfBand)
    {
        value = MAX_600M_IT_FRQ_SIZE;
    }
    else if (RF_5G == e_rfBand)// 5G
    {
        value = MAX_5G_IT_FRQ_SIZE;
    }
    else
    {
        ;
    }

    if (BW_20M == e_bw)
    {
        value = (value + 1) / 2;
    }

    return value;
}

static uint8_t BB_JudgeAdjacentFrequency(uint8_t judge_ch)
{
    if ((judge_ch == (stru_sweepPower.u8_mainCh + 1)) || (judge_ch == (stru_sweepPower.u8_mainCh - 1)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static uint8_t BB_UpdateOptCh(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw, uint8_t sweep_ch)
{
    uint8_t tmpCh = stru_sweepPower.u8_optCh;
    uint8_t u8_maxCh = BB_GetItFrqNum( e_rfBand );
    int32_t level;
    uint8_t *pu8_bestChCnt = (RF_5G == (stru_sweepPower.e_curBand) ? (stru_sweepPower.u8_bestBb2ChCnt) : 
                                                                     (stru_sweepPower.u8_bestBb1ChCnt));

    if ( e_rfBand != stru_sweepPower.e_curBand || sweep_ch == stru_sweepPower.u8_mainCh || sweep_ch == stru_sweepPower.u8_optCh)
    {
        return 0;
    }
    if (BW_10M == e_bw)
    {
        level = ((1 == BB_JudgeAdjacentFrequency(sweep_ch)) ? (2 * CMP_POWER_AVR_LEVEL) : (CMP_POWER_AVR_LEVEL));
        //if (BB_CompareCh1Ch2ByPower(e_rfBand, sweep_ch, tmpCh, SWEEP_FREQ_BLOCK_ROWS))
        if (BB_CompareCh1Ch2ByPowerAver(e_rfBand, sweep_ch, tmpCh, 0))
        {
            tmpCh = sweep_ch;
        }
    }
    else // 20M
    {
        if (0 == sweep_ch)
        {
            level = ((1 == BB_JudgeAdjacentFrequency(sweep_ch * 2 + 1)) ? (2 * CMP_POWER_AVR_LEVEL) : (CMP_POWER_AVR_LEVEL));
            if (BB_CompareCh1Ch2ByPowerAver(e_rfBand, sweep_ch * 2 + 1, tmpCh, 0))
            {
                tmpCh = sweep_ch * 2 + 1;
            }
        }
        else if ( (((u8_maxCh + 1) / 2) - 1) == sweep_ch )
        {
            if (0 == (BB_GetItFrqNum(e_rfBand) % 2))
            {
                level = ((1 == BB_JudgeAdjacentFrequency(sweep_ch * 2)) ? (2 * CMP_POWER_AVR_LEVEL) : (CMP_POWER_AVR_LEVEL));
                //if (BB_CompareCh1Ch2ByPower(e_rfBand, sweep_ch * 2, tmpCh, SWEEP_FREQ_BLOCK_ROWS))
                if (BB_CompareCh1Ch2ByPowerAver(e_rfBand, sweep_ch * 2, tmpCh, 0))
                {
                    tmpCh = sweep_ch * 2;
                }
            }
            else // the number of frequency points is odd
            {
                // do nothing
            }
        }
        else
        {
            level = ((1 == BB_JudgeAdjacentFrequency(sweep_ch * 2)) ? (2 * CMP_POWER_AVR_LEVEL) : (CMP_POWER_AVR_LEVEL));
            //if (BB_CompareCh1Ch2ByPower(e_rfBand, sweep_ch * 2, tmpCh, SWEEP_FREQ_BLOCK_ROWS))
            if (BB_CompareCh1Ch2ByPowerAver(e_rfBand, sweep_ch * 2, tmpCh, 0))
            {
                tmpCh = sweep_ch * 2;
            }

            level = ((1 == BB_JudgeAdjacentFrequency(sweep_ch * 2 + 1)) ? (2 * CMP_POWER_AVR_LEVEL) : (CMP_POWER_AVR_LEVEL));
            //if (BB_CompareCh1Ch2ByPower(e_rfBand, sweep_ch * 2 + 1, tmpCh, SWEEP_FREQ_BLOCK_ROWS)) 
            if (BB_CompareCh1Ch2ByPowerAver(e_rfBand, sweep_ch * 2 + 1, tmpCh, 0)) 
            {
                tmpCh = sweep_ch * 2 + 1;
            }
        }
    }

    if (tmpCh != (stru_sweepPower.u8_optCh))
    {
        pu8_bestChCnt[tmpCh] += 1;
        if (pu8_bestChCnt[tmpCh] > 10)
        {
            stru_sweepPower.u8_optCh = tmpCh;
            stru_sweepPower.u8_optSweepCh = ((BW_10M == e_bw) ? (tmpCh) : (tmpCh / 2));
            memset(pu8_bestChCnt, 0, u8_maxCh);
        }
    }
    else
    {
        pu8_bestChCnt[tmpCh] = 0;
    }
}
/*
 * round: round of the loop
 * sort the noise from Low -> High
*/
void bubble_sort(int * data, int datasize, int round)
{
    uint8_t i;
    uint8_t j;

    for ( i = 0 ; i < round; i++)
    {
        for( j = i+1; j < datasize; j++ )
        {
            if ( data[i] > data[j] )
            {
                int tmp = data[i];  //do swap
                data[i] = data[j];
                data[j] = tmp;
            }
        }
    }
}


/*
 *  start to sweep another band
 */
static void BB_SweepChangeBand(ENUM_RF_BAND e_toRfBand, uint8_t u8_mainCh, uint8_t u8_optCh)
{
    stru_sweepPower.u8_spareSweepCh   =  0;
    stru_sweepPower.u8_optBandSweepCh =  0;

    stru_sweepPower.u8_curBb1Row   =  0;
    stru_sweepPower.u8_curBb2Row   =  0;

    stru_sweepPower.u8_cycleCnt      =  MAIN_CH_CYCLE;
    stru_sweepPower.u8_mainSweepRow  =  0;
    stru_sweepPower.u8_optSweepRow   =  0;

    stru_sweepPower.u8_preMainCh  =  0xff;

    stru_sweepPower.u8_mainCh     =  u8_mainCh;

    stru_sweepPower.u8_prevSweepCh   = 0x0;
    stru_sweepPower.u16_preMainCount = 0x0;

    stru_sweepPower.u8_mainSweepCh = u8_mainCh;
    stru_sweepPower.u8_prevSweepCh = u8_mainCh;

    stru_sweepPower.u8_optCh       = u8_optCh;
    stru_sweepPower.u8_optSweepCh  = u8_optCh;

    stru_sweepPower.e_curBand      =  e_toRfBand;  //current rf band for sweep
    stru_sweepPower.u8_bandSelCnt  =  0;

    BB_set_SweepFrq( e_toRfBand, stru_sweepPower.e_bw ,u8_mainCh );
}


/*
 * function to compare the 2G/5G band noise
*/
ENUM_RF_select BB_grd_cmpBandNoise( void )
{
    /*5G is better: if 2G band 2/3 channel average >= 5G channel 2/3 channel average + 6dbm
      2G is better: if 2G band 2/3 channel average <= 5G channel 2/3 channel average
      else the same
    */
    int i32_2G_noisepower_average[stru_sweepPower.u8_bb1ItFrqSize];
    int i32_5G_noisepower_average[stru_sweepPower.u8_bb2ItFrqSize];
    int i32_2GAverage = 0;
    int i32_5GAverage = 0;
    int count1 = 3;             //get 3 best channel from 2G
    int count2 = 5;             //get 5 best channel from 5G

    uint8_t bad_2G_cnt = 0, bad_5G_cnt = 0;
    uint8_t tmp = 0;

    memcpy(i32_2G_noisepower_average, stru_sweepPower.i32_rf0PwrAvr, sizeof(int) * (stru_sweepPower.u8_bb1ItFrqSize));
    memcpy(i32_5G_noisepower_average, stru_sweepPower.i32_rf1PwrAvr, sizeof(int) * (stru_sweepPower.u8_bb2ItFrqSize));

    //bubble sort the Noise power, get the best channels
    bubble_sort( i32_2G_noisepower_average, stru_sweepPower.u8_bb1ItFrqSize, count1 );
    bubble_sort( i32_5G_noisepower_average, stru_sweepPower.u8_bb2ItFrqSize, count2 );

    //IT is interference
    for (tmp = 0; tmp < count1; tmp++)
    {
        i32_2GAverage += i32_2G_noisepower_average[tmp];
    }
    i32_2GAverage /= count1;

    for (tmp = 0; tmp < count2; tmp++)
    {
        i32_5GAverage += i32_5G_noisepower_average[tmp];
    }
    i32_5GAverage /= count2;

    //check 2G other frq > best 2G frq + 7db, RC is interference
    for (tmp = 0; tmp < stru_sweepPower.u8_bb1ItFrqSize; tmp++)
    {
        if ( ( i32_2G_noisepower_average[tmp] > i32_5GAverage + 7 && context.flag_signalBlock == 0 ) ||
             ( i32_2G_noisepower_average[tmp] > i32_5GAverage + 17 && context.flag_signalBlock == 1 ))
        {
            bad_2G_cnt ++;
        }
    }
#if 0
    for (tmp = 0; tmp < MAX_5G_IT_FRQ_SIZE; tmp++)
    {
        if ( ( i32_5G_noisepower_average[tmp] > i32_2GAverage *  6 && context.flag_signalBlock == 0 ) ||
             ( i32_5G_noisepower_average[tmp] > i32_2GAverage * 17 && context.flag_signalBlock == 1 ))
        {
            bad_5G_cnt ++;
        }
    }
#endif

    //compare result.
    if ( ( i32_2GAverage >= i32_5GAverage + 17 && context.flag_signalBlock  == 1) ||    //Noise(2G) > Noise(5G)+17db( 10 * log51)
         ( i32_2GAverage >= i32_5GAverage + 7  && context.flag_signalBlock  == 0) ||    //Noise(2G) > Noise(5G)+ 7db( 10 * log6) )  
         bad_2G_cnt >= ( (stru_sweepPower.u8_bb1ItFrqSize) * 2 / 3 ))
    {
        //dlog_info("!5G block = %d %d", context.flag_signalBlock, bad_2G_cnt);
        return BETTER_5G;
    }
    else if( (( i32_2GAverage <= i32_5GAverage && context.flag_signalBlock == 0 ) ||          //Noise(2G) < Noise(5G)
              ( i32_2GAverage <= i32_5GAverage + 10  && context.flag_signalBlock == 1 ))      //Noise(2G) < Noise(5G) + 10db
               && ( bad_2G_cnt <= ( (stru_sweepPower.u8_bb1ItFrqSize) * 1 / 2 )))                            // and 
    {
        //dlog_info("!2G block = %d %d", context.flag_signalBlock, bad_2G_cnt);
        return BETTER_2G;
    }
    else
    {
        return EQUAL_2G_5G;
    }
}

/*
void grd_initRfband( ENUM_RF_BAND e_band )
{
    BB_WriteReg(PAGE2, RF_BAND_CHANGE_0, e_band);
    BB_WriteReg(PAGE2, RF_BAND_CHANGE_1, e_band);
}
*/

uint8_t agc_level_min[50];
uint8_t agc_idx = 0;

void grd_checkBlockMode(void)
{
    int      dist   = 0;
    uint16_t agcsum = 0;
    uint8_t  num    = sizeof(agc_level_min);

    context.u8_agc[0] = BB_ReadReg(PAGE2, AAGC_2_RD);
    context.u8_agc[1] = BB_ReadReg(PAGE2, AAGC_3_RD);

    agc_level_min[agc_idx] = (context.u8_agc[0] < context.u8_agc[1]) ? context.u8_agc[0] : context.u8_agc[1];
    agc_idx ++;

    if ( agc_idx == num )
    {    
        uint8_t  i      = 0;
        agc_idx = 0;
        for( ; i < num; i++ )
        {
            agcsum += agc_level_min[i];
        }
        context.agclevel = agcsum / num;
        //dlog_info("Agc level: %d", context.agclevel);

        if ( context.agclevel >= GRD_AGC_BLOCK_THRESH )
        {
            int ret = grd_GetDistAverage(&dist);
            if (ret == 0 ||  (ret == 1 && dist <= 400) )    //Fail to get the distance or dist <= 200
            {
                context.flag_signalBlock = 1;
            }
            else
            {
                context.flag_signalBlock = 0;
            }
        }
        else
        {
            context.flag_signalBlock = 0;
            //dlog_info("Non-Block 0x%x", context.agclevel);
        }
    }
}



/**
 * @brief: notify sky to change band by spi
 */
void grd_notifyRfbandChange( ENUM_RF_BAND e_band, uint8_t u8_itCh, uint8_t u8_optCh, uint8_t u8_delayCnt )
{
    uint8_t data0 = (u8_delayCnt << 2) | ((uint8_t)e_band);
    uint8_t data1 = (u8_itCh     << 2) | ((uint8_t)e_band);

    BB_WriteReg(PAGE2, RF_BAND_CHANGE_0, data0);
    BB_WriteReg(PAGE2, RF_BAND_CHANGE_1, data1);

    context.stru_bandChange.flag_bandchange = 1;
    context.stru_bandChange.u8_bandChangecount = u8_delayCnt;
    context.stru_bandChange.u8_ItCh   = u8_itCh;
    context.stru_bandChange.u8_optCh  = u8_optCh;
}

extern uint8_t grd_rc_channel;

/**
 * @brief: down count, and change band
 */
int32_t grd_doRfbandChange( uint8_t *pu8_mainCh, uint8_t *pu8_optCh )
{
    if ( 1 == context.stru_bandChange.flag_bandchange )
    {
        context.stru_bandChange.u8_bandChangecount --;
        if ( 0 == context.stru_bandChange.u8_bandChangecount )  //count to 0, time to change band
        {            
            context.stru_bandChange.flag_bandchange = 0;
            context.e_curBand = OTHER_BAND(context.e_curBand);  //switch to another band

            BB_set_RF_Band(BB_GRD_MODE, context.e_curBand);

            BB_SweepChangeBand(context.e_curBand, context.stru_bandChange.u8_ItCh, context.stru_bandChange.u8_optCh);
            BB_set_sweepChannel(); //re-set the sweepChannel            

            if ( pu8_mainCh )
            {
                *pu8_mainCh = context.stru_bandChange.u8_ItCh;
            }

            if ( pu8_optCh )
            {
                *pu8_optCh = context.stru_bandChange.u8_optCh;
            }

            //clear the band change request after do band change
            BB_WriteReg(PAGE2, RF_BAND_CHANGE_0, 0);
            BB_WriteReg(PAGE2, RF_BAND_CHANGE_1, 0);

            //clear the result, re-start statistic again
            stru_sweepPower.u8_bandSelCnt = 0;

            dlog_info("Band switch: %d %d %d %d", grd_rc_channel, context.stru_bandChange.u8_bandChangecount, 
                                                  context.stru_bandChange.u8_ItCh, context.e_curBand);
            return 1;
        }
        else
        {
            uint8_t data0 = (context.stru_bandChange.u8_bandChangecount << 2) | (OTHER_BAND(context.e_curBand));
            BB_WriteReg(PAGE2, RF_BAND_CHANGE_0, data0);
        }
    }

    return 0;
}

int32_t grd_checkBackTo2G(void)
{
    context.stru_bandChange.u8_unlockLoopCnt++;
    if ( context.stru_bandChange.u8_unlockLoopCnt >= GRD_BACK_TO_2G_UNLOCK_CNT && 
         ( context.e_curBand == RF_5G && context.e_bandsupport == RF_2G_5G ) )
    {
        context.stru_bandChange.u8_unlockLoopCnt = 0;
        grd_notifyRfbandChange(RF_2G, 0, 1, BAND_CHANGE_DELAY);
        return 1;
    }

    return 0;
}
