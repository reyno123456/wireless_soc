#include <stdint.h>
#include "bb_regs.h"
#include "bb_types.h"
#include "bb_ctrl_internal.h"
#include "debuglog.h"


typedef struct
{
    int32_t      i32_frqOffset[50];
    uint8_t      u8_ch;
    ENUM_RF_BAND band;
    uint8_t      u8_contCnt;
    uint8_t      u8_flagOffset;  //offset have got
    uint8_t      u8_ChLockCnt;
}STRU_FRQ_OFFSET;


STRU_FRQ_OFFSET stru_frqOffst = 
{
    .u8_contCnt = 0,
    .u8_flagOffset = 0
};


void grd_clear_frqOffset( void )
{
    stru_frqOffst.u8_contCnt = 0;
    stru_frqOffst.u8_ChLockCnt = 0;
}


int grd_cal_frqOffset( uint8_t ch, ENUM_RF_BAND e_band )
{
    int integer, fraction, tracking, hw_frqOffset;
    int cnt = sizeof(stru_frqOffst.i32_frqOffset) /sizeof(stru_frqOffst.i32_frqOffset[0]) ;

    if ( stru_frqOffst.u8_ch != ch || stru_frqOffst.band != e_band)
    {
        grd_clear_frqOffset();
        stru_frqOffst.u8_ch = ch;
        stru_frqOffst.band  = e_band;        
        return 0;
    }

    if ( stru_frqOffst.u8_ChLockCnt <= 50)
    {
        stru_frqOffst.u8_ChLockCnt ++;
        return 0;
    }

    integer  = BB_ReadReg(PAGE2, INTEGER_OFFSET);

    fraction = (BB_ReadReg(PAGE2, FRACTION_OFFSET_0) << 24) | (BB_ReadReg(PAGE2, FRACTION_OFFSET_1) << 16) |
               (BB_ReadReg(PAGE2, FRACTION_OFFSET_2) <<  8) | (BB_ReadReg(PAGE2, FRACTION_OFFSET_3));
               
    tracking = (BB_ReadReg(PAGE2, TRACK_OFFSET_0) << 24) | (BB_ReadReg(PAGE2, TRACK_OFFSET_1) << 16) |
               (BB_ReadReg(PAGE2, TRACK_OFFSET_2) <<  8) | (BB_ReadReg(PAGE2, TRACK_OFFSET_3));

    hw_frqOffset = (BB_ReadReg(PAGE2, HW_OFFSET_0) << 24) | (BB_ReadReg(PAGE2, HW_OFFSET_1) << 16) | (BB_ReadReg(PAGE2, HW_OFFSET_2) <<  8) ;

    stru_frqOffst.i32_frqOffset[stru_frqOffst.u8_contCnt] = (fraction + tracking - (integer << 19));

    dlog_info("i f t: %x %x %x %x %d %d %d %d\n", integer, fraction, tracking, hw_frqOffset, e_band, ch, 
                                               stru_frqOffst.i32_frqOffset[stru_frqOffst.u8_contCnt], 
                                               stru_frqOffst.i32_frqOffset[stru_frqOffst.u8_contCnt] / 383);

    if ( stru_frqOffst.u8_contCnt >= cnt -1 )
    {
        uint8_t i = 0;
        int sum   = 0;

        stru_frqOffst.u8_flagOffset = 1;
        for ( ; i < stru_frqOffst.u8_contCnt; i++)
        {
            sum += stru_frqOffst.i32_frqOffset[i];
        }
        sum /= stru_frqOffst.u8_contCnt;

        BB_WriteReg(PAGE2, FRE_OFFSET_0, (sum>>24)&0xff);
        BB_WriteReg(PAGE2, FRE_OFFSET_1, (sum>>16)&0xff);
        BB_WriteReg(PAGE2, FRE_OFFSET_2, (sum>> 8)&0xff);
        BB_WriteReg(PAGE2, FRE_OFFSET_3, (sum&0xff));

        BB_WriteReg(PAGE2, FRE_OFFSET_4, (e_band<<6) | ch);

        dlog_info("i f t: %x %x %x %x %d %d \n", integer, fraction, tracking, hw_frqOffset, e_band, ch);
        dlog_info("write offset: %x %x %x %x \n", (sum>>24)&0xff, (sum>>16)&0xff, (sum>> 8)&0xff, (sum&0xff));
        
        stru_frqOffst.u8_contCnt = 0;
    }
    else
    {
        stru_frqOffst.u8_contCnt ++;
    }

    return 0;
}
