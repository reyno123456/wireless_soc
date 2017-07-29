#ifndef TEST_H264_ENCODER_H
#define TEST_H264_ENCODER_H

void command_encoder_dump_brc(void);

void command_encoder_update_brc(char* br_str);

void command_encoder_update_video(char* widthStr, char* hightStr, char* framerateStr);

#endif

