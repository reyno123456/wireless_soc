#include <stddef.h>
#include <stdint.h>
#include "debuglog.h"
#include "systicks.h"
#include "data_type.h"
#include "enc_internal.h"
#include "brc.h"
#include "vsoc_enc.h"
#include "h264_encoder.h"
#include "interrupt.h"
#include "reg_map.h"
#include "sys_event.h"
#include "reg_rw.h"
#include "bb_types.h"
#include "rtc.h"

#include "memory_config.h"
#include "cfg_parser.h"
extern RC_DATA rca[];
#define H264_ENCODER_BUFFER_HIGH_LEVEL    (1<<19)
#define H264_ENCODER_BUFFER_LOW_LEVEL     (1<<17)

static STRU_EncoderStatus g_stEncoderStatus[2] = { 0 };

static int H264_Encoder_UpdateGop(unsigned char view, unsigned char gop);

static int H264_Encoder_StartView(unsigned char view, unsigned int resW, unsigned int resH, unsigned int gop, unsigned int framerate, unsigned int bitrate, ENUM_ENCODER_INPUT_SRC src)
{
    if (view >= 2)
    {
        return 0;
    }

    INTR_NVIC_DisableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);

    if (view == 0)
    {
        init_view0(resW, resH, gop, framerate, bitrate, src);
        my_initial_all( view );
        open_view0(g_stEncoderStatus[view].brc_enable);
    }
    else if (view == 1)
    {
        init_view1(resW, resH, gop, framerate, bitrate, src);
        my_initial_all( view );
        open_view1(g_stEncoderStatus[view].brc_enable);
    }

    g_stEncoderStatus[view].resW = resW;
    g_stEncoderStatus[view].resH = resH;
    g_stEncoderStatus[view].framerate = framerate;
    g_stEncoderStatus[view].running = 1;

    H264_Encoder_UpdateBitrate(view, bitrate);
    H264_Encoder_UpdateGop(view, framerate);
        
    if ((g_stEncoderStatus[0].brc_enable && g_stEncoderStatus[0].running) || 
        (g_stEncoderStatus[1].brc_enable && g_stEncoderStatus[1].running)) // view0 and view1 share the same BRC interrupt
    {
        INTR_NVIC_EnableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);
    }

    return 1;
}

static int H264_Encoder_RestartView(unsigned char view, unsigned int resW, unsigned int resH, unsigned int gop, unsigned int framerate, unsigned int bitrate, ENUM_ENCODER_INPUT_SRC src)
{
    if (view >= 2)
    {
        return 0;
    }
    
    INTR_NVIC_DisableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);

    if (view == 0)
    {
        close_view0();
        H264_Encoder_StartView(view, resW, resH, gop, framerate, bitrate, src);
    }
    else if (view == 1)
    {
        close_view1();
        H264_Encoder_StartView(view, resW, resH, gop, framerate, bitrate, src);
    }

    return 1;
}

static int H264_Encoder_CloseView(unsigned char view)
{
    if (view >= 2)
    {
        return 0;
    }

    INTR_NVIC_DisableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);
    
    if (view == 0)
    {
        close_view0();
    }
    else if (view == 1)
    {
        close_view1();
    }

    g_stEncoderStatus[view].resW = 0;
    g_stEncoderStatus[view].resH = 0;
    g_stEncoderStatus[view].framerate = 0;
    g_stEncoderStatus[view].running = 0;

    if ((g_stEncoderStatus[0].brc_enable && g_stEncoderStatus[0].running) || 
        (g_stEncoderStatus[1].brc_enable && g_stEncoderStatus[1].running)) // view0 and view1 share the same BRC interrupt
    {
        INTR_NVIC_EnableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);
    }

    return 1;
}

static int H264_Encoder_UpdateVideoInfo(unsigned char view, unsigned int resW, unsigned int resH, unsigned int framerate, ENUM_ENCODER_INPUT_SRC src)
{
    unsigned int u32_data; 
    unsigned int tmp_resW;
    unsigned int tmp_resH;
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);

    if (view >= 2)
    {
        return 0;
    }
    
    if(g_stEncoderStatus[view].resW != resW ||
       g_stEncoderStatus[view].resH != resH ||
       g_stEncoderStatus[view].framerate != framerate || 
       g_stEncoderStatus[view].src != src)
    {
        if ((resW == 0) || (resH == 0) || (framerate == 0) || (src == 0))
        {
            H264_Encoder_CloseView(view);
        }
        else
        {
            if (g_stEncoderStatus[view].over_flow == 0)
            {
            H264_Encoder_RestartView(view, resW, resH, g_stEncoderStatus[view].gop, 
                                     framerate, g_stEncoderStatus[view].bitrate, src);
			}
            else
            {
                g_stEncoderStatus[view].resW = resW;
                g_stEncoderStatus[view].resH = resH;
                g_stEncoderStatus[view].framerate = framerate;
            }
        }
        
        dlog_critical("Video format change: %d, %d, %d, %d\n", view, resW, resH, framerate);
    }

    osdptr->video_width[view] = resW;
    osdptr->video_height[view] = resH;
    osdptr->frameRate[view] = framerate;
    
    READ_WORD((ENC_REG_ADDR+(0x01<<2)), u32_data);
    tmp_resW = (u32_data >> 16) & 0xFFFF;
    tmp_resH = (u32_data >> 0) & 0xFFFF;
    if ((tmp_resW == (g_stEncoderStatus[0].resW)) && (tmp_resH == (g_stEncoderStatus[0].resH)))
    {
        osdptr->encoder_status |= 0x01;
    }
    else
    {
        osdptr->encoder_status &= ~0x01;
    }
    
    READ_WORD((ENC_REG_ADDR+(0x1a<<2)), u32_data);
    tmp_resW = (u32_data >> 16) & 0xFFFF;
    tmp_resH = (u32_data >> 0) & 0xFFFF;
    if ((tmp_resW == (g_stEncoderStatus[1].resW)) && (tmp_resH == (g_stEncoderStatus[1].resH)))
    {
        osdptr->encoder_status |= 0x02;
    }
    else
    {
        osdptr->encoder_status &= ~0x02;
    }

    return 1;
}

static uint32_t H264_Encoder_GetBufferLevel(unsigned char view)
{
    uint32_t buf_level = 0;

    Reg_Write32_Mask(ENC_REG_ADDR + 0xDC, (unsigned int)(0x21 << 24), BIT(31)|BIT(30)|BIT(29)|BIT(28)|BIT(27)|BIT(26)|BIT(25)|BIT(24));
    
    //read buffer counter

    if (view == 0)
    {
        Reg_Write32_Mask(ENC_REG_ADDR + 0xD8, (unsigned int)(0x04 <<  8), BIT(11)|BIT(10)|BIT(9)|BIT(8));       // Switch to vdb debug register
    }
    else
    {
        Reg_Write32_Mask(ENC_REG_ADDR + 0xD8, (unsigned int)(0x05 <<  8), BIT(11)|BIT(10)|BIT(9)|BIT(8));       // Switch to vdb debug register
    }
    
    buf_level = Reg_Read32(ENC_REG_ADDR + 0xF8);
    Reg_Write32_Mask(ENC_REG_ADDR + 0xDC, (unsigned int)(0x00 << 24), BIT(31)|BIT(30)|BIT(29)|BIT(28)|BIT(27)|BIT(26)|BIT(25)|BIT(24));

    return buf_level;
}

static void H264_Encoder_IdleCallback(void* p)
{
    uint32_t buf_level;
    if ((g_stEncoderStatus[0].over_flow == 1) && (g_stEncoderStatus[0].running == 0))
    {
        buf_level = H264_Encoder_GetBufferLevel(0);
        if (buf_level <= H264_ENCODER_BUFFER_LOW_LEVEL)
        {
            Reg_Write32_Mask( 0xa003008c, 0x0, 0x01);
            H264_Encoder_StartView(0, g_stEncoderStatus[0].resW, g_stEncoderStatus[0].resH, 
                                      g_stEncoderStatus[0].gop, g_stEncoderStatus[0].framerate, 
                                      g_stEncoderStatus[0].bitrate,
                                      g_stEncoderStatus[0].src
                                      );

            g_stEncoderStatus[0].over_flow = 0;
            
            dlog_info("Buffer level %d, open view 0", buf_level);

        }
    }
    else if ((g_stEncoderStatus[1].over_flow == 1) && (g_stEncoderStatus[1].running == 0))
    {
        buf_level = H264_Encoder_GetBufferLevel(1);
        if (buf_level <= H264_ENCODER_BUFFER_LOW_LEVEL)
        {
            Reg_Write32_Mask(0xa003004c, 0x0, 0x04);
            H264_Encoder_StartView(1, g_stEncoderStatus[1].resW, g_stEncoderStatus[1].resH, 
                                      g_stEncoderStatus[1].gop,  g_stEncoderStatus[1].framerate, 
                                      g_stEncoderStatus[1].bitrate,
                                      g_stEncoderStatus[1].src
                                      );

            g_stEncoderStatus[1].over_flow = 0;
            
            dlog_info("Buffer level %d, open view 1", buf_level);
        }        
    }
}


static void H264_Encoder_InputVideoFormatChangeCallback(void* p)
{
    uint8_t index  = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->index;
    uint16_t width = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->width;
    uint16_t hight = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->hight;
    uint8_t framerate = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->framerate;
    ENUM_ENCODER_INPUT_SRC src = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->e_h264InputSrc;

    // ADV7611 0,1 is connected to H264 encoder 1,0
    H264_Encoder_UpdateVideoInfo(index, width, hight, framerate, src);
}

static int H264_Encoder_UpdateGop(unsigned char view, unsigned char gop)
{
    uint32_t addr;

    if(view == 0 )
    {
        addr = ENC_REG_ADDR + (0x02 << 2);
    }
    else
    {
        addr = ENC_REG_ADDR + (0x1b << 2);
    }

    Reg_Write32_Mask(addr, (unsigned int)(gop << 24), BIT(31)|BIT(30)|BIT(29)|BIT(28)|BIT(27)|BIT(26)|BIT(25)|BIT(24));
}


int H264_Encoder_UpdateIpRatioAndIpRatioMax(unsigned char view, unsigned char ipratio, unsigned char ipratiomax)
{
    uint32_t addr;

    if(view == 0 )
    {
        addr = ENC_REG_ADDR + (0x08 << 2);
    }
    else
    {
        addr = ENC_REG_ADDR + (0x21 << 2);
    }
    
    Reg_Write32_Mask(addr, (unsigned int)(ipratio << 24), BIT(27)|BIT(26)|BIT(25)|BIT(24));
    Reg_Write32_Mask(addr, (unsigned int)(ipratiomax << 8), BIT(15)|BIT(14)|BIT(13)|BIT(12)|BIT(11)|BIT(10)|BIT(9)|BIT(8));
}

static int H264_Encoder_UpdateMinAndMaxQP(unsigned char view, unsigned char br)
{
    unsigned int addr, addr_arcast, minqp, maxqp;
    if(view==0) {
    	addr = ENC_REG_ADDR+(0x06<<2);
    	addr_arcast = ENC_REG_ADDR+(0x18<<2);
    } else {
        addr = ENC_REG_ADDR+(0x1F<<2);
    	addr_arcast = ENC_REG_ADDR+(0x31<<2);
    }
    switch( br ) { // update minqp value, lhu, 2017/04/21
        case 0 : {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x11; else minqp = 0x13 ;} break; // 8Mbps
        case 1 : {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x17; else minqp = 0x19 ;} break; // 600kbps
        case 2 : {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x16; else minqp = 0x18 ;} break; // 1.2Mbps
        case 3 : {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x15; else minqp = 0x17 ;} break; // 2.4Mbps
        case 4 : {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x15; else minqp = 0x17 ;} break; // 3Mbps
        case 5 : {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x14; else minqp = 0x16 ;} break; // 3.5Mbps
        case 6 : {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x14; else minqp = 0x16 ;} break; // 4Mbps
        case 7 : {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x13; else minqp = 0x15 ;} break; // 4.8Mbps
        case 8 : {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x13; else minqp = 0x15 ;} break; // 5Mbps
        case 9 : {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x12; else minqp = 0x14 ;} break; // 6Mbps
        case 10: {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x12; else minqp = 0x14 ;} break; // 7Mbps
        case 11: {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x11; else minqp = 0x13 ;} break; // 7.5Mbps
        case 12: {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x10; else minqp = 0x12 ;} break; // 9Mbps
        case 13: {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x10; else minqp = 0x12 ;} break; // 10Mbps
        default: {if (((Reg_Read32(addr_arcast))>>2)&0x01) minqp = 0x15; else minqp = 0x17 ;} break; // 3Mbps
    }
    Reg_Write32_Mask(addr, (unsigned int)(minqp<<8), BIT(15)|BIT(14)|BIT(13)|BIT(12)|BIT(11)|BIT(10)|BIT(9)|BIT(8)); // set minqp, lhu
    // set maxqp in arcast mode (maxqp-minqp:0x13) and non-arcast mode (maxqp-minqp:0x19) when bit-rate changed, lhu, 2017/05/05
    {
    	maxqp = minqp + ((((Reg_Read32(addr_arcast))>>2)&0x01)? 0x13:0x19);
        Reg_Write32_Mask(addr, (unsigned int)(maxqp<<16), BIT(23)|BIT(22)|BIT(21)|BIT(20)|BIT(19)|BIT(18)|BIT(17)|BIT(16));
    }
}

int H264_Encoder_UpdateBitrate(unsigned char view, unsigned char br_idx)
{
    uint8_t ratio = 0, ratiomax = 0;
    unsigned int frame_rate;

    if (view >= 2)
    {
        return 0;
    }

    frame_rate = (view == 0) ? g_stEncoderStatus[0].framerate: g_stEncoderStatus[1].framerate;
    switch( br_idx ) { // update ipratio and ipratiomax value, lhu, 2017/04/21
        case 0 : {ratio =  4 ; if (frame_rate == 30) ratiomax = 10; else ratiomax =  8;} break; // 8Mbps
        case 1 : {ratio = 12 ; if (frame_rate == 30) ratiomax = 26; else ratiomax = 18;} break; // 600kbps
        case 2 : {ratio = 11 ; if (frame_rate == 30) ratiomax = 23; else ratiomax = 17;} break; // 1.2Mbps
        case 3 : {ratio =  9 ; if (frame_rate == 30) ratiomax = 23; else ratiomax = 17;} break; // 2.4Mbps
        case 4 : {ratio =  8 ; if (frame_rate == 30) ratiomax = 23; else ratiomax = 17;} break; // 3Mbps
        case 5 : {ratio =  7 ; if (frame_rate == 30) ratiomax = 22; else ratiomax = 16;} break; // 3.5Mbps
        case 6 : {ratio =  7 ; if (frame_rate == 30) ratiomax = 21; else ratiomax = 16;} break; // 4Mbps
        case 7 : {ratio =  6 ; if (frame_rate == 30) ratiomax = 15; else ratiomax = 12;} break; // 4.8Mbps
        case 8 : {ratio =  6 ; if (frame_rate == 30) ratiomax = 15; else ratiomax = 12;} break; // 5Mbps
        case 9 : {ratio =  6 ; if (frame_rate == 30) ratiomax = 15; else ratiomax = 12;} break; // 6Mbps
        case 10: {ratio =  5 ; if (frame_rate == 30) ratiomax = 12; else ratiomax = 10;} break; // 7Mbps
        case 11: {ratio =  5 ; if (frame_rate == 30) ratiomax = 11; else ratiomax =  9;} break; // 7.5Mbps
        case 12: {ratio =  4 ; if (frame_rate == 30) ratiomax =  8; else ratiomax =  7;} break; // 9Mbps
        case 13: {ratio =  4 ; if (frame_rate == 30) ratiomax =  7; else ratiomax =  7;} break; // 10Mbps
        default: {ratio =  8 ; if (frame_rate == 30) ratiomax = 23; else ratiomax = 17;} break; // 3Mbps
    }

    dlog_info("%d %d %d %d %d %d\n", g_stEncoderStatus[0].running, g_stEncoderStatus[1].running, 
                                     g_stEncoderStatus[0].brc_enable, g_stEncoderStatus[1].brc_enable, 
                                     g_stEncoderStatus[0].bitrate, g_stEncoderStatus[1].bitrate);
    if (g_stEncoderStatus[view].running && g_stEncoderStatus[view].brc_enable /*&& (g_stEncoderStatus[view].bitrate != br_idx)*/)
    {
        if (view == 0)
        {
            Reg_Write32_Mask(ENC_REG_ADDR+(0xA<<2), (unsigned int)(br_idx << 26), BIT(26) | BIT(27) | BIT(28) | BIT(29) | BIT(30));
            //H264_Encoder_UpdateGop(0, g_stEncoderStatus[0].framerate); // comment out by lhu, 2017/04/21
            H264_Encoder_UpdateIpRatioAndIpRatioMax(0, ratio, ratiomax);
            H264_Encoder_UpdateMinAndMaxQP(0, br_idx);
        }
        else
        {
            Reg_Write32_Mask(ENC_REG_ADDR+(0x23<<2), (unsigned int)(br_idx << 26), BIT(26) | BIT(27) | BIT(28) | BIT(29) | BIT(30));
            //H264_Encoder_UpdateGop(1, g_stEncoderStatus[1].framerate); // comment out by lhu, 2017/04/21
            H264_Encoder_UpdateIpRatioAndIpRatioMax(1, ratio, ratiomax);
            H264_Encoder_UpdateMinAndMaxQP(1, br_idx);
        }

        dlog_info("Encoder bitrate change: %d, %d, %d, %d\n", view, br_idx, ratio, ratiomax);
    }
    
    g_stEncoderStatus[view].bitrate = br_idx;
    return 1;
}

static void H264_Encoder_BBModulationChangeCallback(void* p)
{
    uint8_t br_idx = ((STRU_SysEvent_BB_ModulationChange *)p)->BB_MAX_support_br;
    uint8_t ch = ((STRU_SysEvent_BB_ModulationChange *)p)->u8_bbCh;
    
    if (0 == ch)
    {
        H264_Encoder_UpdateBitrate(0, br_idx);
        dlog_info("H264 bitidx ch1: %d \r\n", br_idx);
    }
    else if (1 == ch)
    {
        H264_Encoder_UpdateBitrate(1, br_idx);
        dlog_info("H264 bitidx ch2: %d \r\n", br_idx);
    }
    else
    {
    }
}

static void VEBRC_IRQ_Wrap_Handler(uint32_t u32_vectorNum)
{
    uint32_t v0_last_row;
	uint32_t v1_last_row;
    uint32_t view0_feedback = Reg_Read32(ENC_REG_ADDR+(0x09<<2));
    uint32_t view1_feedback = Reg_Read32(ENC_REG_ADDR+(0x22<<2));

    if(g_stEncoderStatus[0].running == 1)  // View0 is opened
    {
        // check if this is the last row
        v0_last_row = ((view0_feedback >> 8) & 0x01);

        if(v0_last_row)
        {
            //dlog_info("%d,%d\n", rca.v0_frame_cnt, rca.v0_intra_period );
            uint32_t buf_level = H264_Encoder_GetBufferLevel(0);
            if(buf_level >= H264_ENCODER_BUFFER_HIGH_LEVEL)
            {
                //Close Encoder
                Reg_Write32_Mask( (unsigned int) 0xa003008c, 0x01, 0x01);
                g_stEncoderStatus[0].over_flow = 1;
                close_view0();
                g_stEncoderStatus[0].running = 0;
            }
        }
    }

    if(g_stEncoderStatus[1].running == 1)  // View1 is opened
    {
        // check if this is the last row
        v1_last_row = ((view1_feedback >> 8) & 0x01);

        if(v1_last_row)
        {
            //dlog_info("%d %d\n", rca.v1_frame_cnt,rca.v1_intra_period );
            uint32_t buf_level = H264_Encoder_GetBufferLevel(1);
            if(buf_level >= H264_ENCODER_BUFFER_HIGH_LEVEL)
            {
                //Close Encoder
                Reg_Write32_Mask( (unsigned int) 0xa003004c, 0x04, 0x04);

                g_stEncoderStatus[1].over_flow = 1;
                close_view1();
                g_stEncoderStatus[1].running = 0;
                dlog_info("Buffer level %d, close view 1.", buf_level);
            }

#ifdef ARCAST
            if( ( (view1_feedback >> 1) & 0x01) == 1)   // Check if it is one I frame.
            {
                // Switch to DVP1 input debug register
                //  WRITE_WORD( (ENC_REG_ADDR+ (0x37<<2)), 0x12000000);
                //  unsigned int lb_freeblk = Reg_Read32( (unsigned int)(ENC_REG_ADDR + 0xE4) );

                WRITE_WORD( (ENC_REG_ADDR+ (0x37<<2)), 0x0a000000);
                unsigned int freespace = Reg_Read32( (unsigned int)(ENC_REG_ADDR + 0xFC) );

                //dlog_info("xxx%08x %08x\n", lb_freeblk, freespace);
                if( (freespace & 0xFFFF) == rca[1].dvp_lb_freesize ) 
                {
                    // insert timestamp data, not only consider view1.
                    // close encoder channel
                    volatile uint32_t tick;
                    uint32_t tmp;
                    uint8_t  sum = 0;

                    Reg_Write32( (unsigned int) 0xa003004c, 0x04);
                    
                    tick =  *((volatile uint32_t *)(SRAM_MODULE_SHARE_AVSYNC_TICK));
                    
                    //head: 0x35 + 0x53 + 0x55 + sum
                    sum += (0x35+0x53+0x55+((tick>>24)&0xff) + ((tick>>16) & 0xff) + ((tick>>8)& 0xff) + (tick& 0xff));
                    tmp = (sum << 24) + (0x55 << 16) + (0x53 <<8) + 0x35;

                    Reg_Write32( (unsigned int) 0xb1800000, 0x7F010000);
                    Reg_Write32( (unsigned int) 0xb1800000, tmp);
                    tmp = ( ((tick & 0xff)<<24) + ((tick & 0xff00)<<8) + ((tick & 0xff0000)>>8) + ((tick & 0xff000000)<<24));
                    Reg_Write32( (unsigned int) 0xb1800000, tmp);

                    //
                    Reg_Write32( (unsigned int) 0xa003004c, 0x00); // back to encoder channel
                    //dlog_error("TS tick = 0x%08x 0x%02x\n", tick, sum);
                }
           }
#endif
        }
    }

    if( (g_stEncoderStatus[0].running == 1 && (view0_feedback & BIT(6))) || (g_stEncoderStatus[1].running == 1 && (view1_feedback & BIT(6))) )
    {
        // Encoder hang happens, then do reset.
        Reg_Write32(VSOC_SOFT_RESET, (~(BIT(3))));
        Reg_Write32(VSOC_SOFT_RESET, 0xFF);

        if (g_stEncoderStatus[0].running == 1)
        {
            H264_Encoder_RestartView(0, g_stEncoderStatus[0].resW, g_stEncoderStatus[0].resH, 
                                     g_stEncoderStatus[0].gop, g_stEncoderStatus[0].framerate, 
                                     g_stEncoderStatus[0].bitrate, g_stEncoderStatus[0].src);
            dlog_info("Reset view0");
        }

        if (g_stEncoderStatus[1].running == 1)
        {
            H264_Encoder_RestartView(1, g_stEncoderStatus[1].resW, g_stEncoderStatus[1].resH, 
                                     g_stEncoderStatus[1].gop, g_stEncoderStatus[1].framerate, 
                                     g_stEncoderStatus[1].bitrate, g_stEncoderStatus[1].src);
            dlog_info("Reset view1");
        }
    }
    else
    {
        uint8_t chReset = 0;
        
        if ((g_stEncoderStatus[0].running == 1) && (Reg_Read32(ENC_REG_ADDR+(0x34<<2)) & (BIT(24)|BIT(25)|BIT(26))))
        {
            // View0 encoder error, then do channel reset.
            Reg_Write32_Mask(ENC_REG_ADDR+(0x00<<2), 0, BIT(24));
            my_initial_all( 0 );
            Reg_Write32_Mask(ENC_REG_ADDR+(0x00<<2), BIT(24), BIT(24));
            Reg_Write32_Mask(ENC_REG_ADDR+(0x34<<2), 0, (BIT(24)|BIT(25)|BIT(26)|BIT(27)));
            chReset = 1;
            dlog_info("Reset channel 0");
        }

        if ((g_stEncoderStatus[1].running == 1) && (Reg_Read32(ENC_REG_ADDR+(0x34<<2)) & (BIT(28)|BIT(29)|BIT(30))))
        {
            // View1 encoder error, then do channel reset.
            Reg_Write32_Mask(ENC_REG_ADDR+(0x19<<2), 0, BIT(24));
            my_initial_all( 1 );
            Reg_Write32_Mask(ENC_REG_ADDR+(0x19<<2), BIT(24), BIT(24));
            Reg_Write32_Mask(ENC_REG_ADDR+(0x34<<2), 0, (BIT(28)|BIT(29)|BIT(30)|BIT(31)));
            chReset = 1;
            dlog_info("Reset channel 1");
        }
        if (chReset == 0)
        {
            // Normal status, then do bitrate control.
            VEBRC_IRQ_Handler(view0_feedback, view1_feedback);
        }
    }
}

static STRU_H264_REG * H264_getCfgData(STRU_cfgBin *cfg, STRU_cfgNode *pnode)
{
    int i;

    pnode = CFGBIN_GetNode(cfg, VSOC_ENC_INIT_ID);
    dlog_info("h264 init size %d", pnode->nodeDataSize);
    return (STRU_H264_REG *)(pnode+1);
}

int H264_Encoder_Init(uint8_t gop0, uint8_t br0, uint8_t brc0_e, uint8_t gop1, uint8_t br1, uint8_t brc1_e)
{
    // variable Declaraton 
    char spi_rd_dat, i2c_rd_dat;
    unsigned int wait_cnt, i;
    unsigned char read_cnt ;

    // Video_Soc Wait SDRAM INIT_DONE
    sdram_init_check(); 

    reg_IrqHandle(VIDEO_ARMCM7_IRQ_VECTOR_NUM, VEBRC_IRQ_Wrap_Handler, NULL);
    INTR_NVIC_SetIRQPriority(VIDEO_ARMCM7_IRQ_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_VIDEO_ARMCM7,0));
    INTR_NVIC_DisableIRQ(VIDEO_ARMCM7_IRQ_VECTOR_NUM);

    g_stEncoderStatus[0].gop = gop0;
    g_stEncoderStatus[0].bitrate = br0;
    g_stEncoderStatus[0].brc_enable = brc0_e;
    rca[0].poweron_rc_params_set = 1;

    g_stEncoderStatus[1].gop = gop1;
    g_stEncoderStatus[1].bitrate = br1;
    g_stEncoderStatus[1].brc_enable = brc1_e;
    rca[1].poweron_rc_params_set = 1;
    //load from cfg.bin and write register
    {
        int cnt = 0;
        STRU_cfgNode node;
        STRU_H264_REG *preg = H264_getCfgData((STRU_cfgBin *)SRAM_CONFIGURE_MEMORY_ST_ADDR, &node);

        for (cnt = 0; cnt < node.nodeElemCnt; cnt++)
        {
            Reg_Write32_Mask(preg->u32_regAddr, preg->u32_regValue, preg->u32_regdataMask);
        }
     }

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_IDLE, H264_Encoder_IdleCallback);
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_H264_INPUT_FORMAT_CHANGE, H264_Encoder_InputVideoFormatChangeCallback);
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_BB_SUPPORT_BR_CHANGE, H264_Encoder_BBModulationChangeCallback);

    dlog_info("h264 encoder init OK\n");

    return 1;
}

void H264_Encoder_DumpFrameCount(void)
{
    //waiting for irq. update QP
    unsigned char i;
    static int saved_view0_frame_cnt_lyu=0;
    static int saved_view1_frame_cnt_lyu=0;

    /* printf gop_cnt & frame_cnt */
    if(rca[0].frame_cnt != saved_view0_frame_cnt_lyu) 
    {
        dlog_info("G: %d\n", rca[0].gop_cnt);
        dlog_info("F: %d\n", rca[0].frame_cnt);
        saved_view0_frame_cnt_lyu=rca[0].frame_cnt;
    }

    if(rca[1].frame_cnt != saved_view1_frame_cnt_lyu) 
    {
        dlog_info("G: %d\n", rca[1].gop_cnt);
        dlog_info("F: %d\n", rca[1].frame_cnt);
        saved_view1_frame_cnt_lyu=rca[1].frame_cnt;
    }
    // update_aof();
}

