#ifndef SYS_EVENT_H
#define SYS_EVENT_H

#ifdef  __cplusplus
extern "C"
{
#endif


#include <stdint.h>


typedef void (*SYS_Event_Handler)(void *);

#define SYS_EVENT_HANDLER_PARAMETER_LENGTH    16
#define SYS_EVENT_LEVEL_HIGH_MASK             0x10000000
#define SYS_EVENT_LEVEL_MIDIUM_MASK           0x20000000
#define SYS_EVENT_LEVEL_LOW_MASK              0x40000000
#define SYS_EVENT_INTER_CORE_MASK             0x80000000

typedef enum
{
    INTER_CORE_CPU0_ID = 1,
    INTER_CORE_CPU1_ID = 2,
    INTER_CORE_CPU2_ID = 4,
}INTER_CORE_CPU_ID;

typedef uint32_t INTER_CORE_MSG_ID;

// Registered system event handler list

typedef struct _RegisteredSysEventHandler_Node
{
    SYS_Event_Handler handler;
    struct _RegisteredSysEventHandler_Node* prev;
    struct _RegisteredSysEventHandler_Node* next;
} STRU_RegisteredSysEventHandler_Node;

typedef STRU_RegisteredSysEventHandler_Node* STRU_RegisteredSysEventHandler_List;

// Registered system event list

typedef struct _RegisteredSysEvent_Node
{
    uint32_t event_id;
    STRU_RegisteredSysEventHandler_List handler_list;
    STRU_RegisteredSysEventHandler_Node* handler_list_tail;
    struct _RegisteredSysEvent_Node* prev;
    struct _RegisteredSysEvent_Node* next;
} STRU_RegisteredSysEvent_Node;

typedef STRU_RegisteredSysEvent_Node* STRU_RegisteredSysEvent_List;

// Notified system event list

typedef struct _NotifiedSysEvent_Node
{
    uint32_t event_id;
    uint8_t parameter[SYS_EVENT_HANDLER_PARAMETER_LENGTH];
    struct _NotifiedSysEvent_Node* prev;
    struct _NotifiedSysEvent_Node* next;
} STRU_NotifiedSysEvent_Node;

typedef STRU_NotifiedSysEvent_Node* STRU_NotifiedSysEvent_List;

// Test event

typedef struct _SysEvent_TestParameter
{
    uint8_t  p1;
    uint16_t p2;
    uint32_t p3;
    uint8_t  reserve[SYS_EVENT_HANDLER_PARAMETER_LENGTH - 7];
} STRU_SysEvent_TestParameter;

#define SYS_EVENT_ID_TEST (SYS_EVENT_LEVEL_HIGH_MASK | 0x8000)

// APIs

uint8_t SYS_EVENT_RegisterHandler(uint32_t event_id, SYS_Event_Handler event_handler);
uint8_t SYS_EVENT_UnRegisterHandler(uint32_t event_id, SYS_Event_Handler event_handler);
uint8_t SYS_EVENT_Notify(uint32_t event_id, void* parameter);
uint8_t SYS_EVENT_Notify_From_ISR(uint32_t event_id, void* parameter);
uint8_t SYS_EVENT_Process(void);
void SYS_EVENT_DumpAllListNodes(void);

// Idle event

#define SYS_EVENT_ID_IDLE (SYS_EVENT_LEVEL_LOW_MASK | 0x8000)

// Misc driver event

#define SYS_EVENT_ID_H264_INPUT_FORMAT_CHANGE         (SYS_EVENT_LEVEL_MIDIUM_MASK | 0x0001 | SYS_EVENT_INTER_CORE_MASK)
#define SYS_EVENT_ID_BB_SUPPORT_BR_CHANGE             (SYS_EVENT_LEVEL_MIDIUM_MASK | 0x0002)
#define SYS_EVENT_ID_BB_DATA_BUFFER_FULL              (SYS_EVENT_LEVEL_MIDIUM_MASK | 0x0003 | SYS_EVENT_INTER_CORE_MASK)
#define SYS_EVENT_ID_USER_CFG_CHANGE                  (SYS_EVENT_LEVEL_HIGH_MASK   | 0x0004 | SYS_EVENT_INTER_CORE_MASK)
#define SYS_EVENT_ID_USB_PLUG_OUT                     (SYS_EVENT_LEVEL_HIGH_MASK   | 0x0005)
#define SYS_EVENT_ID_NV_MSG                           (SYS_EVENT_LEVEL_MIDIUM_MASK | 0x0006 | SYS_EVENT_INTER_CORE_MASK)
#define SYS_EVENT_ID_USB_SWITCH_HOST_DEVICE           (SYS_EVENT_LEVEL_HIGH_MASK   | 0x0007)


#define SYS_EVENT_ID_UART_DATA_RCV_SESSION0           (SYS_EVENT_LEVEL_HIGH_MASK   | 0x0008)
#define SYS_EVENT_ID_UART_DATA_RCV_SESSION1           (SYS_EVENT_LEVEL_MIDIUM_MASK | 0x0009 | SYS_EVENT_INTER_CORE_MASK)
#define SYS_EVENT_ID_UART_DATA_RCV_SESSION2           (SYS_EVENT_LEVEL_MIDIUM_MASK | 0x000A | SYS_EVENT_INTER_CORE_MASK)
#define SYS_EVENT_ID_UART_DATA_RCV_SESSION3           (SYS_EVENT_LEVEL_MIDIUM_MASK | 0x000B | SYS_EVENT_INTER_CORE_MASK)
#define SYS_EVENT_ID_UART_DATA_RCV_SESSION4           (SYS_EVENT_LEVEL_MIDIUM_MASK | 0x000C | SYS_EVENT_INTER_CORE_MASK)

#define SYS_EVENT_ID_AUDIO_INPUT_CHANGE               (SYS_EVENT_LEVEL_MIDIUM_MASK | 0x000D )
#define SYS_EVENT_ID_SD_CARD_CHANGE                   (SYS_EVENT_LEVEL_MIDIUM_MASK | 0x000E )
#define SYS_EVENT_ID_H264_INPUT_FORMAT_CHANGE_ARCAST  (SYS_EVENT_LEVEL_MIDIUM_MASK | 0x000F )

typedef enum
{
    ENCODER_INPUT_SRC_HDMI_0 = 1,
    ENCODER_INPUT_SRC_HDMI_1,
    ENCODER_INPUT_SRC_DVP_0,
    ENCODER_INPUT_SRC_DVP_1,
    ENCODER_INPUT_SRC_MIPI,
} ENUM_ENCODER_INPUT_SRC;

typedef struct _SysEvent_H264InputFormatChangeParameter
{
    uint8_t  index;
    uint16_t width;
    uint16_t hight;
    uint8_t  framerate;
    uint8_t  vic;
    ENUM_ENCODER_INPUT_SRC  e_h264InputSrc;
    uint8_t  reserve[SYS_EVENT_HANDLER_PARAMETER_LENGTH - 9];
} STRU_SysEvent_H264InputFormatChangeParameter;

typedef struct _SysEvent_BB_ModulationChange
{
    uint8_t  BB_MAX_support_br; //BB_MAX_support_br: the MAX stream bitrate(MHz) 
    uint8_t  u8_bbCh; // 
    uint8_t  reserve[SYS_EVENT_HANDLER_PARAMETER_LENGTH - 2];
} STRU_SysEvent_BB_ModulationChange;

typedef struct _SysEvent_BB_DATA_BUFFER_FULL_RATIO_Change
{
    uint8_t  BB_Data_Full_Ratio; // 0x00 - 0x80 
    uint8_t  reserve[SYS_EVENT_HANDLER_PARAMETER_LENGTH - 1];
} STRU_SysEvent_BB_DATA_BUFFER_FULL_RATIO_Change;

typedef struct _SysEvent_AudioInputChangeParameter
{
    uint8_t  u8_audioSampleRate;
    uint8_t  reserve[SYS_EVENT_HANDLER_PARAMETER_LENGTH - 1];
} STRU_SysEvent_AudioInputChangeParameter;

// data struct used between cpu communication
typedef enum
{
    NV_NUM_RCID = 1,
    NV_NUM_NOTICE = 0xFF,
} ENUM_NV_NUM;

typedef struct
{
    uint8_t u8_nvSrc  : 4;
    uint8_t u8_nvDst  : 4;
    ENUM_NV_NUM e_nvNum;
    uint8_t u8_nvPar[SYS_EVENT_HANDLER_PARAMETER_LENGTH - 2];
} STRU_SysEvent_NvMsg;

typedef struct _SysEvent_OTG_HOST_DEV_SWITCH
{
    uint8_t  otg_port_id;
    uint8_t  otg_state;        // 0:device    1:host
    uint8_t  reserve[SYS_EVENT_HANDLER_PARAMETER_LENGTH - 2];
} STRU_SysEvent_OTG_HOST_DEV_SWITCH;

typedef struct _SysEvent_DEV_PLUG_OUT
{
    uint8_t  otg_port_id;
    uint8_t  reserve[SYS_EVENT_HANDLER_PARAMETER_LENGTH - 1];
} STRU_SysEvent_DEV_PLUG_OUT;

#ifdef  __cplusplus
}
#endif


#endif
