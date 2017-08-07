#ifndef __TEST_BYPASS_ENCODER_H
#define __TEST_BYPASS_ENCODER_H

#ifdef __cplusplus
extern "C"
{
#endif


#include "hal_sram.h"


void bitrate_change_callback(void* p);
void bypass_encoder_init(ENUM_HAL_SRAM_VIDEO_CHANNEL e_bypassVideoCh);
void receive_video_stream(void *video_buff, uint32_t length, uint8_t port_id);


#ifdef __cplusplus
}
#endif

#endif

