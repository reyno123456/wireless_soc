#include <string.h>
#include <stdlib.h>
#include "test_usbh.h"
#include "debuglog.h"
#include "hal_usb_host.h"
#include "hal_usb_device.h"
#include "hal_sram.h"
#include "hal.h"



/* USB Host Global Variables */
USBH_BypassVideoCtrl            g_usbhBypassVideoCtrl;
USBH_AppCtrl                    g_usbhAppCtrl;

void USBH_USBHostStatus(void const *argument)
{
    dlog_info("USBH_USBHostStatus TASK");

    while (1)
    {
        HAL_USB_HostProcess();

        HAL_Delay(5);
    }
}


void USBH_BypassVideo(void const *argument)
{
    FRESULT             fileResult;
    uint32_t            bytesread;
    uint8_t            *videoBuff;
    static FIL          usbhAppFile;
    uint8_t             i;
    static uint8_t      u8_usbPortId;

    fileResult          = FR_OK;
    bytesread           = 0;

    dlog_info("enter USBH_BypassVideo Task!\n");

    while (1)
    {
        if (osOK == osSemaphoreWait(g_usbhBypassVideoCtrl.semID, osWaitForever))
        {
            if (g_usbhBypassVideoCtrl.bypassChannel == 0)
            {
                videoBuff           = (uint8_t *)HAL_SRAM_GetVideoBypassChannelBuff(HAL_SRAM_VIDEO_CHANNEL_0);
            }
            else
            {
                videoBuff           = (uint8_t *)HAL_SRAM_GetVideoBypassChannelBuff(HAL_SRAM_VIDEO_CHANNEL_1);
            }

            for ( ;; )
            {
                if (g_usbhBypassVideoCtrl.taskActivate == 1)
                {
                    switch (g_usbhBypassVideoCtrl.taskState)
                    {
                    case USBH_VIDEO_BYPASS_TASK_IDLE:
                        u8_usbPortId = HAL_USB_GetMSCPort();

                        if (u8_usbPortId < HAL_USB_PORT_NUM)
                        {
                            dlog_info("use MSC Port: %d", u8_usbPortId);

                            g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_START;
                        }

                        break;

                    case USBH_VIDEO_BYPASS_TASK_START:
                        if ((HAL_USB_HOST_STATE_READY == HAL_USB_GetHostAppState(u8_usbPortId))&&
                            (1 == g_usbhBypassVideoCtrl.taskActivate))
                        {
                            if (g_usbhBypassVideoCtrl.fileOpened == 0)
                            {
                                fileResult = f_open(&usbhAppFile, "0:usbtest.264", FA_READ);

                                if(fileResult == FR_OK)
                                {
                                    g_usbhBypassVideoCtrl.fileOpened = 1;

                                    g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_TRANS;
                                }
                                else
                                {
                                    g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_STOP;

                                    dlog_error("open file error: %d\n", (uint32_t)fileResult);
                                }
                            }
                            else
                            {
                                g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_TRANS;
                            }
                        }
                        else
                        {
                            g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_STOP;
                        }

                        break;

                    case USBH_VIDEO_BYPASS_TASK_TRANS:
                        if ((HAL_USB_HOST_STATE_READY == HAL_USB_GetHostAppState(u8_usbPortId))&&
                            (1 == g_usbhBypassVideoCtrl.taskActivate))
                        {
                            fileResult = f_read((&usbhAppFile), videoBuff, USB_VIDEO_BYPASS_SIZE_ONCE, (void *)&bytesread);

                            if(fileResult != FR_OK)
                            {
                                g_usbhBypassVideoCtrl.fileOpened    = 0;
                                f_close(&usbhAppFile);

                                dlog_error("Cannot Read from the file \n");
                            }

                            HAL_Delay(10);

                            if (bytesread < USB_VIDEO_BYPASS_SIZE_ONCE)
                            {
                                dlog_info("a new round!\n");
                                g_usbhBypassVideoCtrl.fileOpened    = 0;
                                f_close(&usbhAppFile);

                                g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_START;
                            }
                        }
                        else
                        {
                            g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_STOP;
                        }

                        break;

                    case USBH_VIDEO_BYPASS_TASK_STOP:
                        g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_IDLE;

                        break;

                    default:
                        g_usbhBypassVideoCtrl.taskActivate  = 0;

                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
}


void USBH_MountUSBDisk(void)
{
    FRESULT             fileResult;
    static uint8_t      s_u8MountFlag = 0;

    if (s_u8MountFlag == 0)
    {
        FATFS_LinkDriver(&USBH_Driver, "0:/");

        fileResult = f_mount(&(g_usbhAppCtrl.usbhAppFatFs), "0:/", 0);

        if (fileResult != FR_OK)
        {
            dlog_error("mount fatfs error: %d\n", fileResult);

            return;
        }

        s_u8MountFlag = 1;
    }
    else
    {
        return;
    }
}


void USB_MainTask(void const *argument)
{
    osEvent event;

    dlog_info("main task");

    while (1)
    {
        event = osMessageGet(g_usbhAppCtrl.usbhAppEvent, osWaitForever);

        if (event.status == osEventMessage)
        {
            switch (event.value.v)
            {
                case USBH_APP_START_BYPASS_VIDEO:
                {
                    /* Need to start a new task */
                    if (0 == g_usbhBypassVideoCtrl.taskExist)
                    {
                        USBH_MountUSBDisk();

                        osThreadDef(BypassTask, USBH_BypassVideo, osPriorityIdle, 0, 4 * 128);
                        g_usbhBypassVideoCtrl.threadID  = osThreadCreate(osThread(BypassTask), NULL);

                        if (NULL == g_usbhBypassVideoCtrl.threadID)
                        {
                            dlog_error("create Video Bypass Task error!\n");

                            break;
                        }

                        g_usbhBypassVideoCtrl.taskExist = 1;

                        osSemaphoreDef(bypassVideoSem);
                        g_usbhBypassVideoCtrl.semID     = osSemaphoreCreate(osSemaphore(bypassVideoSem), 1);

                        if (NULL == g_usbhBypassVideoCtrl.semID)
                        {
                            osThreadTerminate(g_usbhBypassVideoCtrl.threadID);
                            g_usbhBypassVideoCtrl.taskExist = 0;

                            dlog_error("create Video Bypass Semaphore error!\n");

                            break;
                        }
                    }

                    /* activate the task */
                    if (g_usbhBypassVideoCtrl.bypassChannel == 0)
                    {
                        HAL_SRAM_EnableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_0);
                    }
                    else
                    {
                        HAL_SRAM_EnableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_1);
                    }

                    osSemaphoreRelease(g_usbhBypassVideoCtrl.semID);

                    break;
                }

                case USBH_APP_STOP_BYPASS_VIDEO:
                {
                    dlog_info("stop bypassvideo task!\n");

                    if (g_usbhBypassVideoCtrl.bypassChannel == 0)
                    {
                        HAL_SRAM_DisableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_0);
                    }
                    else
                    {
                        HAL_SRAM_DisableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_1);
                    }

                    break;
                }

                default:
                    break;
            }
        }
    }
}


void command_startBypassVideo(uint8_t *bypassChannel)
{
    USBH_APP_EVENT_DEF  usbhAppType;

    usbhAppType = USBH_APP_START_BYPASS_VIDEO;

    if (0 == g_usbhBypassVideoCtrl.taskActivate)
    {
        g_usbhBypassVideoCtrl.taskActivate  = 1;
        g_usbhBypassVideoCtrl.bypassChannel = strtoul(bypassChannel, NULL, 0);
        osMessagePut(g_usbhAppCtrl.usbhAppEvent, usbhAppType, 0);
    }
    else
    {
        dlog_info("Bypass Video Task is running\n");
    }
}


void command_stopBypassVideo(void)
{
    USBH_APP_EVENT_DEF  usbhAppType;

    usbhAppType = USBH_APP_STOP_BYPASS_VIDEO;

    if (1 == g_usbhBypassVideoCtrl.taskActivate)
    {
        g_usbhBypassVideoCtrl.taskActivate  = 0;
        osMessagePut(g_usbhAppCtrl.usbhAppEvent, usbhAppType, 0);
    }
    else
    {
        dlog_info("Bypass Video Task is not running\n");
    }
}


