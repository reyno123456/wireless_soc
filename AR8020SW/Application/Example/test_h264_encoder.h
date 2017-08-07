#ifndef TEST_H264_ENCODER_H
#define TEST_H264_ENCODER_H

#ifdef __cplusplus
extern "C"
{
#endif


void command_encoder_dump_brc(void);

void command_encoder_update_brc(char* br_str);

void command_encoder_update_video(char* widthStr, char* hightStr, char* framerateStr);


#ifdef __cplusplus
}
#endif

#endif

