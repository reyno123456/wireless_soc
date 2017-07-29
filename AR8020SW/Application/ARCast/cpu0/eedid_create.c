#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "macros.h"
#include "edid.h"
#include "hdmi.h"
#include "cea861.h"
#include "arcast_appcommon.h"
#include "eedid_create.h"

const uint8_t detail_timing[9][18] = {
    {0xfa, 0x1c, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c, 0x45, 0x00, 0xfd, 0x1e, 0x11, 0x00, 0x00, 0x1e},//1920x1080p@30
    {0x01, 0x1d, 0x00, 0x72, 0x51, 0xd0, 0x1e, 0x20, 0x6e, 0x28, 0x55, 0x00, 0xfd, 0x1e, 0x11, 0x00, 0x00, 0x1e},//1280x720p@60
    {0x01, 0x1d, 0x00, 0xbc, 0x52, 0xd0, 0x1e, 0x20, 0xb8, 0x28, 0x55, 0x40, 0xfd, 0x1e, 0x11, 0x00, 0x00, 0x00},//1280x720p@50
    {0x8c, 0x0a, 0xd0, 0x8a, 0x20, 0xe0, 0x2d, 0x10, 0x10, 0x3e, 0x96, 0x00, 0xfd, 0x1e, 0x11, 0x00, 0x00, 0x18},//720x480p@60
    {0x8c, 0x0a, 0xd0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0c, 0x40, 0x55, 0x00, 0xfd, 0x1e, 0x11, 0x00, 0x00, 0x18},//720x576p@50
    {0x0e, 0x1f, 0x00, 0x80, 0x51, 0x00, 0x1e, 0x30, 0x40, 0x80, 0x37, 0x00, 0xfd, 0x1e, 0x11, 0x00, 0x00, 0x18},//1280x768@60
    {0x64, 0x19, 0x00, 0x40, 0x41, 0x00, 0x26, 0x30, 0x18, 0x88, 0x36, 0x00, 0xfd, 0x1e, 0x11, 0x00, 0x00, 0x18},//1024x768@60
    {0xa0, 0x0f, 0x20, 0x00, 0x31, 0x58, 0x17, 0x20, 0x28, 0x80, 0x14, 0x00, 0xfd, 0x1e, 0x11, 0x00, 0x00, 0x1e},//800x600@60
    {0xd6, 0x09, 0x80, 0x90, 0x20, 0xe0, 0x19, 0x10, 0x08, 0x60, 0x22, 0x00, 0xfd, 0x1e, 0x11, 0x00, 0x00, 0x18},//640x480@60
};

const uint8_t vid_switch_vic[9] = {
    34,//1920x1080p@30
    4,//1280x720p@60
    19,//1280x720p@50
    3,//720x480p@60
    18,//720x576p@50
    0xff,//1280x768@60
    0xff,//1024x768@60
    0xff,//800x600@60
    0xff,//640x480@60
};

static int8_t create_edid1(struct edid * edid, const STRU_ARCAST_AVCAPABILITY *buff)
{
    uint8_t i = 0;
    uint8_t *pu8_edid = (uint8_t *)edid;
    uint8_t u8_preferred = buff->u8_VideoVidList[0];
    
    for (i = 0; i < buff->u8_VideoVidCount; i++)
    {
        switch (buff->u8_VideoVidList[i])
        {
            case 7:
                edid->established_timings.timing_1024x768_60 |= 1;
                break;
            case 8:
                edid->established_timings.timing_800x600_60 |= 1;
                break;
            case 9:
                edid->established_timings.timing_640x480_60 |= 1;
                break;
            default:
                break;
        }
    }

    memcpy((uint8_t *)(&edid->detailed_timings[0].timing), (uint8_t *)&detail_timing[u8_preferred], sizeof(struct edid_detailed_timing_descriptor));

    for (i = 0; i < ARRAY_SIZE(edid->standard_timing_id); i++) {
        struct edid_standard_timing_descriptor * desc = &edid->standard_timing_id[i];
        if (i < buff->u8_VideoVidCount)
        {
            switch (buff->u8_VideoVidList[i])
            {
                case 0:
                    {
                        //1920x1080p@60
                        desc->horizontal_active_pixels = 0xD1;
                        desc->refresh_rate = 0;
                        desc->image_aspect_ratio = EDID_ASPECT_RATIO_16_9;
                        
                    }
                    break;
                case 1:
                    {
                        //1280x720p@60
                        desc->horizontal_active_pixels = 0x81;
                        desc->refresh_rate = 0;
                        desc->image_aspect_ratio = EDID_ASPECT_RATIO_16_9;
                    }
                    break;
                default:
                    {
                        memcpy(desc, EDID_STANDARD_TIMING_DESCRIPTOR_INVALID, sizeof(*desc));
                    }
                    break;
            }
        }
        else
        {
            memcpy(desc, EDID_STANDARD_TIMING_DESCRIPTOR_INVALID, sizeof(*desc));
        }
    }

    pu8_edid[127] = 0;
    for (i = 0; i < 127; i++)
    {
        pu8_edid[127] += pu8_edid[i];
    }
    //This byte should beprogrammed such that a onebyte checksum (add all bytestogether) of the entire 128 byteblock equals “00h”.
    pu8_edid[127] = 0xff - pu8_edid[127] + 1;

    return edid->extensions;
}

static void create_cea861_audio_data(struct cea861_audio_data_block * adb, const uint8_t *buff, const uint8_t len)
{
    uint8_t i = 0;
    uint8_t descriptors = adb->header.length / sizeof(*adb->sad);

    struct cea861_short_audio_descriptor * sad = (struct cea861_short_audio_descriptor *) &adb->sad[i];
    sad->sample_rate_44_1_kHz = 0;
    sad->sample_rate_48_kHz = 0;
    for (uint8_t i = 0; i < len; i++)
    {
        switch(buff[i])
        {
            case 1:
                sad->sample_rate_44_1_kHz = 0x01;
                break;
            case 2:
                sad->sample_rate_48_kHz = 0x01;
                break;
            default:
                break;        

        }
    }
}

static void create_cea861_video_data(struct cea861_video_data_block * vdb, const uint8_t *buff, const uint8_t len)
{
    uint8_t i = 0, index = 0;
    index = buff[0];
    vdb->svd[0].native = 1;
    vdb->svd[0].video_identification_code = vid_switch_vic[index];
    printf("len %d %d\n",vdb->header.length,buff[0]);
    for (uint8_t i = 1; i < vdb->header.length; i++) {
        vdb->svd[i].native = 0;
        if (i < len)
        {
            index = buff[i];
            if (vid_switch_vic[index] != 0xff)
            {
                if (i < len)
                {
                    vdb->svd[i].video_identification_code = vid_switch_vic[index];
                }
                else
                {
                    vdb->svd[i].video_identification_code = vid_switch_vic[buff[0]];
                }
            }
            else
            {
                vdb->svd[i].video_identification_code = vid_switch_vic[buff[0]];
            }
        }
        else
        {
            vdb->svd[i].video_identification_code = vid_switch_vic[buff[0]];
        }
    }
}


static void create_cea861_detail_timing(struct cea861_timing_block * ctb, const uint8_t *buff, const uint8_t len)
{

    struct edid_detailed_timing_descriptor *dtd = NULL;
    dtd = (struct edid_detailed_timing_descriptor *) ((uint8_t *) ctb + ctb->dtd_offset);
    uint8_t index = 0,i = 0;
    printf("%x\n",sizeof(struct edid_detailed_timing_descriptor));
    for (i = 0; dtd->pixel_clock; i++, dtd++) 
    {
        printf("%x\n",i);
        if (i < len)
        {
            index = buff[i];
            memcpy((uint8_t *)dtd, &detail_timing[index], sizeof(struct edid_detailed_timing_descriptor));
        }
        else
        {
            memset((uint8_t *)dtd, 0, sizeof(struct edid_detailed_timing_descriptor));   
        }    

    }
}



static int8_t create_edid_extension(struct edid_extension * ext, const STRU_ARCAST_AVCAPABILITY *buff)
{
    uint8_t *pu8_edid = (uint8_t *)ext;
    uint8_t u8_preferred = buff->u8_VideoVidList[0];
    struct edid_detailed_timing_descriptor *dtd = NULL;
    struct cea861_timing_block * ctb = (struct cea861_timing_block *) ext;
    uint8_t offset = offsetof(struct cea861_timing_block, data);
    uint8_t index = 0, i = 0;

    printf("modify extensions\n");
    do {
            struct cea861_data_block_header * header = (struct cea861_data_block_header *) &ctb->data[index];
            switch (header->tag) 
            {
            case CEA861_DATA_BLOCK_TYPE_AUDIO:
                {
                    printf("modify audio\n");
                    struct cea861_audio_data_block * db = (struct cea861_audio_data_block *) header;
                    create_cea861_audio_data(db, (uint8_t *)&buff->u8_AudioAidList, buff->u8_AudioAidCount);
                }
                break;
            case CEA861_DATA_BLOCK_TYPE_VIDEO:
                {
                    printf("modify video\n");
                    struct cea861_video_data_block * db = (struct cea861_video_data_block *) header;
                    create_cea861_video_data(db, (uint8_t *)&buff->u8_VideoVidList, buff->u8_VideoVidCount);
                }
                break;
            default:
                break;
            }

        index = index + header->length + sizeof(*header);
    } while (index < ctb->dtd_offset - offset);

    printf("modify detail timing\n");
    create_cea861_detail_timing(ctb, (uint8_t *)&buff->u8_VideoVidList, buff->u8_VideoVidCount);
    
    printf("checksum\n");
    pu8_edid[127] = 0;
    for (i = 0; i < 127; i++)
    {
        pu8_edid[127] += pu8_edid[i];
    }
    //This byte should beprogrammed such that a onebyte checksum (add all bytestogether) of the entire 128 byteblock equals “00h”.
    pu8_edid[127] = 0xff - pu8_edid[127] + 1;

}


void exchange(uint8_t A[], uint8_t i, uint8_t j)
{
    uint8_t temp = A[i];
    A[i] = A[j];
    A[j] = temp;
}

void create_eedid(uint8_t *old_edid, STRU_ARCAST_AVCAPABILITY *st_avCapability)
{
    
    for (int j = 0; j < st_avCapability->u8_VideoVidCount -1; j++)            
    {
        for (int i = 0; i < st_avCapability->u8_VideoVidCount -1 - j; i++)    
        {
            if (st_avCapability->u8_VideoVidList[i] > st_avCapability->u8_VideoVidList[i + 1])            
            {
                exchange(st_avCapability->u8_VideoVidList, i, i + 1);        
            }
        }
    }
    
    create_edid1((struct edid *)old_edid, st_avCapability);
    create_edid_extension((struct edid_extension *) (old_edid + sizeof(struct edid)), st_avCapability);

    return ;
}