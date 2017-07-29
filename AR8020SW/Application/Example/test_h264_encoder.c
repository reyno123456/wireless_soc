#include <string.h>
#include <stdlib.h>
#include "test_h264_encoder.h"
#include "h264_encoder.h"

void command_encoder_dump_brc(void)
{
    H264_Encoder_DumpFrameCount();
}

void command_encoder_update_brc(char* br_str)
{
    unsigned char br = strtoul(br_str, NULL, 0);
    
    H264_Encoder_UpdateBitrate(0, br);
    H264_Encoder_UpdateBitrate(1, br);
}

void command_encoder_update_video(char* widthStr, char* hightStr, char* framerateStr)
{
    //H264_Encoder_UpdateVideoInfo(0, width, hight, framerate);
}
