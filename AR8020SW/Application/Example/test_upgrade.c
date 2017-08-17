#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "upgrade.h"
#include "hal_bb.h"
#include "hal.h"
#include "debuglog.h"
#include "ff.h"
#include "md5.h"
#include "hal_usb_host.h"
#include "test_usbh.h"
#include "test_upgrade.h"
#include "nor_flash.h"


/**
* @brief  check gnd lock.
* @param  none.
* @retval 1        ok.
* @retval 0        lose.
* @note   None.
*/
int8_t UPGRADE_GndAndSkyLockStatus(void)
{
    STRU_WIRELESS_INFO_DISPLAY *pst_bb_info;
    HAL_BB_GetInfo(&pst_bb_info);
    uint8_t lock_status = pst_bb_info->lock_status;
    return (lock_status & 0x01);
}

void UPGRADE_GndForSky(void const *argument)
{
    uint32_t i = 0;
    uint8_t u8_sendDataArray[UPGRADE_DATACHUNK_SIZE];

    FRESULT    fileResult;
    FIL        MyFile;
    uint32_t   u32_bytesRead = UPGRADE_DATACHUNK_SIZE;
    
    UPGRADE_UseUsbUpgrade(argument);
    
    UPGRADE_Start();    
    
    while (0x01 != UPGRADE_GndAndSkyLockStatus())
    {
        HAL_Delay(500);
        dlog_info("wait lock");
    }

    while(1 != UPGRADE_GndSendInit())
    {
        dlog_warning("wait sky init");
        HAL_Delay(1000);
    }

    fileResult = f_open(&MyFile, "gnd.bin", FA_READ);
    if (FR_OK != fileResult)
    {
        dlog_info("open or create file error: %d\n", fileResult);
        return;
    }

    u32_bytesRead = UPGRADE_DATACHUNK_SIZE;
    s_u16_upgradeReturnStatus = 0;
    while(UPGRADE_DATACHUNK_SIZE == u32_bytesRead)
    {            
        fileResult = f_read(&MyFile, (void *)u8_sendDataArray, UPGRADE_DATACHUNK_SIZE, (void *)&u32_bytesRead);
        if((fileResult != FR_OK))
        {
            dlog_info("Cannot Read from the file \n");
            f_close(&MyFile);
            return;
        }
        while (0 != UPGRADE_SendDataBlock(UPGRADE_DATA_DATA, u8_sendDataArray, u32_bytesRead, s_u16_upgradeReturnStatus))
        {
            HAL_Delay(UPGRADE_DELAY);
        }
        
        s_u16_upgradeReturnStatus++;
        dlog_error("upgrade %d%%\n",(s_u16_upgradeReturnStatus*UPGRADE_DATACHUNK_SIZE)*100/s_u32_upgradeTotalDataLen);         
    }
   
    dlog_info("send data finish");
    
    while((0 == UpgradeGndStatus_Get(UPGRADE_ACK_END)) && (0 == UpgradeGndStatus_Get(UPGRADE_ACK_FAIL)))
    {
        if (UpgradeGndStatus_Get(UPGRADE_ACK_LOSTDATA))        
        {
            for (i = 0; i <  s_u8_lostDataBlockCount; i++)
            {
                if (s_u16_lostDataBlockArray[i] != 0xffff)
                {
                    dlog_warning("re-send data block %d",s_u16_lostDataBlockArray[i]);
                    fileResult = f_lseek(&MyFile, s_u16_lostDataBlockArray[i] * UPGRADE_DATACHUNK_SIZE);
                    if((fileResult != FR_OK))
                    {
                        dlog_info("Cannot lseek the file \n");
                        f_close(&MyFile);
                        return;
                    }
                    fileResult = f_read(&MyFile, (void *)u8_sendDataArray, UPGRADE_DATACHUNK_SIZE, (void *)&u32_bytesRead);
                    if((fileResult != FR_OK))
                    {
                        dlog_info("Cannot Read from the file \n");
                        f_close(&MyFile);
                        return;
                    }
                    while (0 != UPGRADE_SendDataBlock(UPGRADE_LOSTBLOCK_DATA, u8_sendDataArray, u32_bytesRead, s_u16_lostDataBlockArray[i]))
                    {
                        HAL_Delay(UPGRADE_DELAY);
                    }
                }    
            }
          
            memset(s_u16_lostDataBlockArray, 0xff, sizeof(s_u16_lostDataBlockArray));
            s_u8_lostDataBlockCount = 0;
        }

        while (0 != UPGRADE_GndSendEnd())
        {
            HAL_Delay(UPGRADE_DELAY);
        }

        dlog_info("wait sky receive data finish %d", s_u8_lostDataBlockCount);
        HAL_Delay(2000);
    }

    if (UpgradeGndStatus_Get(UPGRADE_ACK_END))
    {
        dlog_error("upgrade ok");
    }
    else if (UpgradeGndStatus_Get(UPGRADE_ACK_FAIL))
    {
        dlog_error("upgrade fail");
    }
    return;
}

void UPGRADE_LocalUpgrade(void const *argument)
{

    FRESULT    fileResult;
    FIL        MyFile;
    uint32_t   u32_bytesRead= RDWR_SECTOR_SIZE;
    uint32_t   u32_recDataSumTmp =0;
    uint32_t   u32_norAddr = APP_ADDR_OFFSET;
    uint32_t   i;  
    uint8_t g_u8arrayRecData[RDWR_SECTOR_SIZE]={0};

    UPGRADE_UseUsbUpgrade(argument);

    fileResult = f_open(&MyFile,argument , FA_READ);
    if (FR_OK != fileResult)
    {
        dlog_error("open or create file error: %d\n", fileResult);
        vTaskDelete(NULL);
    }          
    while(RDWR_SECTOR_SIZE == u32_bytesRead)
    {            
        memset(g_u8arrayRecData,0,RDWR_SECTOR_SIZE);
        fileResult = f_read(&MyFile, (void *)g_u8arrayRecData, RDWR_SECTOR_SIZE, (void *)&u32_bytesRead);
        if((fileResult != FR_OK))
        {
            dlog_error("Cannot Read from the file \n");
            f_close(&MyFile);
        }
        dlog_info("fread %d\n",u32_bytesRead);
        NOR_FLASH_EraseSector(u32_norAddr);
        NOR_FLASH_WriteByteBuffer(u32_norAddr,g_u8arrayRecData,RDWR_SECTOR_SIZE); 
        dlog_info("write flash %d%%\n",(u32_recDataSumTmp*100/s_u32_upgradeTotalDataLen));
        dlog_output(100);          
        u32_recDataSumTmp+=u32_bytesRead; 
        
        u32_norAddr += RDWR_SECTOR_SIZE;                      
    }    
         
    f_close(&MyFile);
    dlog_info("upgrade ok %x\n",s_u32_upgradeTotalDataLen);
    
    dlog_output(100);
    vTaskDelete(NULL);

}
