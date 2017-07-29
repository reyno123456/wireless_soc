#ifndef __TEST__USBH__H
#define __TEST__USBH__H

#include <stdio.h>
#include "cmsis_os.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "usbh_diskio.h"


#define USB_VIDEO_BYPASS_SIZE_ONCE              (8192)


typedef enum
{
    USBH_APP_START_BYPASS_VIDEO = 0,
    USBH_APP_STOP_BYPASS_VIDEO,
}USBH_APP_EVENT_DEF;


typedef enum
{
    USBH_VIDEO_BYPASS_TASK_IDLE         = 0,
    USBH_VIDEO_BYPASS_TASK_START        = 1,
    USBH_VIDEO_BYPASS_TASK_TRANS        = 2,
    USBH_VIDEO_BYPASS_TASK_STOP         = 3,
} USBH_VIDEO_BYPASS_TASK_STATE;


typedef struct
{
    volatile uint8_t        taskActivate;
    volatile uint8_t        taskExist;
    volatile uint8_t        fileOpened;
    volatile uint8_t        bypassChannel;
    osThreadId              threadID;
    osSemaphoreId           semID;
    USBH_VIDEO_BYPASS_TASK_STATE taskState;
} USBH_BypassVideoCtrl;


typedef struct
{
    osMessageQId            usbhAppEvent;
    FATFS                   usbhAppFatFs;
} USBH_AppCtrl;



void USBH_USBHostStatus(void const *argument);
void USBH_BypassVideo(void const *argument);
void USBH_MountUSBDisk(void);
void USB_MainTask(void const *argument);
void command_startBypassVideo(uint8_t *bypassChannel);
void command_stopBypassVideo(void);


extern USBH_AppCtrl             g_usbhAppCtrl;
extern USBH_BypassVideoCtrl     g_usbhBypassVideoCtrl;


#endif

