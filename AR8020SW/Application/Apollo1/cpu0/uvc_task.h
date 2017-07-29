#ifndef __UVC_TASK_H
#define __UVC_TASK_H


#include "hal_usb_host.h"


#define UVC_TRANSFER_SIZE_ONCE                  (8192)
#define UVC_ENDPOINT_FOR_TRANSFER               (0x85)


typedef enum
{
    USBH_UVC_TASK_IDLE                  = 0,
    USBH_UVC_TASK_START                 = 1,
    USBH_UVC_TASK_GET_FRAME             = 2,
    USBH_UVC_TASK_CHECK_FRAME_READY     = 3,
    USBH_UVC_TASK_DISCONNECT            = 4,
} USBH_UVC_TASK_STATE;

void USBH_UVCInit(uint16_t u16_width, uint16_t u16_height, ENUM_HAL_USB_UVC_DATA_TYPE e_data_type);
void USBH_ProcUVC(void);
void command_ViewUVC(void);
void USBH_UVCTask(void const *argument);
void command_startUVC(char *width, char *height);
void command_saveUVC(void);
void command_stopSaveUVC(void);
void command_showUVC(void);
void command_getUVCAttribute(char *index, char *type);
void command_setUVCAttribute(char *index, char *value);
void command_uvchelp(void);


#endif
