#include "test_bypass_encoder.h"
#include "sys_event.h"
#include "debuglog.h"
#include "hal_usb_device.h"


uint8_t    g_bitrate_info[8];


void bitrate_change_callback(void* p)
{
    uint8_t br_idx  = ((STRU_SysEvent_BB_ModulationChange *)p)->BB_MAX_support_br;
    uint8_t ch      = ((STRU_SysEvent_BB_ModulationChange *)p)->u8_bbCh;

    if (0 == ch)
    {
        g_bitrate_info[0] = br_idx;

        dlog_info("H264 bitidx ch0: %d \r\n", br_idx);
    }
}


void bypass_encoder_init(ENUM_HAL_SRAM_VIDEO_CHANNEL e_bypassVideoCh)
{
    /* open channel to transfer */
    HAL_SRAM_EnableSkyBypassVideo(e_bypassVideoCh);

    /* register receive callback to receive video stream from USB host */
    HAL_USB_RegisterCustomerRecvData(receive_video_stream);

    /* register callback to receive bitrate info */
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_BB_SUPPORT_BR_CHANGE, bitrate_change_callback);
}


void receive_video_stream(void *video_buff, uint32_t length, uint8_t port_id)
{
    /* received video, now transfer video stream to ground */
    HAL_SRAM_TransferBypassVideoStream(HAL_SRAM_VIDEO_CHANNEL_0, video_buff, length);

    dlog_info("len: %d, port: %d", length, port_id);
}


