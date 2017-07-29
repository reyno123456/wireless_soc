#ifndef __ARCAST_APPCOMMON_H__
#define __ARCAST_APPCOMMON_H__

#define ARCAST_COMMAND_STATUS                   (1)


#define ARCAST_COMMAND_ACK                          (0)
#define ARCAST_COMMAND_REQUEST_EDID                 (1)
#define ARCAST_COMMAND_REPLY_EDID                   (2)
#define ARCAST_COMMAND_FORMAT                       (3)
//sky<->gnd
#define ARCAST_COMMAND_SKY_FORMAT                   (0)
#define ARCAST_COMMAND_GND_CAPABILITY               (1)
#define ARCAST_COMMAND_SKY_REQUEST_CAPABILITY       (2)
#define ARCAST_COMMAND_SKY_AVDATA                   (3)
#define ARCAST_SKY_ENCODER                          (4)

#define ARCAST_SKY_FORMAT_STABLE                    (0)
#define ARCAST_SKY_FORMAT_CHANGE                    (1)


#define ARCAST_COMMAND_ERROR                        (0xff)

#define ARRAY_COUNT_OF(x)                   ((sizeof(x)/sizeof(0[x])) / ((!(sizeof(x) % sizeof(0[x])))))

typedef struct 
{
    uint8_t  u8_VideoVidCount;
    uint8_t  u8_VideoVidList[16];
    uint8_t  u8_AudioAidCount;
    uint8_t  u8_AudioAidList[2];
} STRU_ARCAST_AVCAPABILITY;

typedef struct 
{
    uint8_t  u8_ATMStatus;
    uint8_t  u8_ARStatus;
    uint8_t  u8_AVFormat[2];
    STRU_ARCAST_AVCAPABILITY st_avCapability;
} STRU_ARCAST_AVSTAUTS;



void Common_AVFORMATSysEventGroundInit(void);
void Common_AVFORMATSysEventSKYInit(void);
void ATM_StatusCallBack(uint8_t *st_status, uint32_t data_len);
//uint8_t Command_FormatStatus(void);

#endif