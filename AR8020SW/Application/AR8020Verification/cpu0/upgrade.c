#include "test_usbh.h"
#include "debuglog.h"
#include "cmsis_os.h"
#include "quad_spi_ctrl.h"
#include "nor_flash.h"
#include "upgrade.h"
#include "serial.h"
#include "md5.h"
#include "systicks.h"
#include <string.h>
#include "hal_usb_host.h"
#include "hal_norflash.h"

static uint8_t g_u8arrayRecData[RDWR_SECTOR_SIZE]={0};


static uint8_t  g_u8upgradeFlage;
static uint8_t  g_u8Amd5Sum[16];
static uint32_t g_u32address=0;
static uint32_t g_u32recDataSum = 0;

#define READ_DATA_SIZE  1024*4  
#define MD5_SIZE        16  

static uint8_t UPGRADE_MD5SUM(void)
{
    uint32_t i=0;
    uint32_t u32_RecCountTmp = g_u32recDataSum-34;
    uint32_t u32_AddressTmp  = g_u32address+34;
    uint32_t u32_Count=0;
    uint8_t  md5_value[MD5_SIZE];
//    uint8_t    *p8_data = (uint8_t *)(g_u32address+34);
    MD5_CTX md5;
    MD5Init(&md5);

    for(i=0;i<((u32_RecCountTmp)/RDWR_SECTOR_SIZE);i++)
    {
        HAL_NORFLASH_ReadByteBuffer((u32_AddressTmp+RDWR_SECTOR_SIZE*i),g_u8arrayRecData,RDWR_SECTOR_SIZE);
        MD5Update(&md5, g_u8arrayRecData, RDWR_SECTOR_SIZE);        
        u32_Count+=RDWR_SECTOR_SIZE;
    }
    if(0 != ((u32_RecCountTmp)%RDWR_SECTOR_SIZE))
    {
        HAL_NORFLASH_ReadByteBuffer((u32_AddressTmp+RDWR_SECTOR_SIZE*i),g_u8arrayRecData,(u32_RecCountTmp%RDWR_SECTOR_SIZE));
        MD5Update(&md5, g_u8arrayRecData, (u32_RecCountTmp%RDWR_SECTOR_SIZE));
        u32_Count+=(u32_RecCountTmp%RDWR_SECTOR_SIZE);
    }
    MD5Final(&md5, md5_value);
    for(i=0;i<16;i++)
    {
        if(md5_value[i] != g_u8Amd5Sum[i])
        {
            dlog_info("nor flash checksum .........fail\n");
            return 1;
        }
    }
    dlog_info("nor flash checksum .........ok\n"); 
    return 0;
}

static void BOOT_PrintInfo(uint32_t u32_addr)
{
    uint8_t* p8_infoAddr = (uint8_t*)(u32_addr);
    uint8_t i=0;
    dlog_info("Created:%02x%02x %02x %02x %02x:%02x:%02x\n",*(p8_infoAddr+1),*(p8_infoAddr+2),*(p8_infoAddr+3),*(p8_infoAddr+4)\
                                                        ,*(p8_infoAddr+5),*(p8_infoAddr+6),*(p8_infoAddr+7));
    dlog_info("load address:0x%02x%02x%02x%02x\n",*(p8_infoAddr+8),*(p8_infoAddr+9),*(p8_infoAddr+10),*(p8_infoAddr+11));
    dlog_info("Version:%02x.%02x\n",*(p8_infoAddr+12),*(p8_infoAddr+13));
    dlog_info("Data size:%x\n",GET_WORD_FROM_ANY_ADDR(p8_infoAddr+14));
    for(i=0;i<16;i++)
    {
        g_u8Amd5Sum[i]=*(p8_infoAddr+18+i);
    }
}


static void UPGRADE_ModifyBootInfo()
{
    uint8_t i=0;
    Boot_Info st_bootInfo;
    memset((void *)&st_bootInfo,0xff,sizeof(st_bootInfo)); 
    HAL_NORFLASH_WriteByteBuffer(0x1000,(uint8_t *)(&st_bootInfo),sizeof(st_bootInfo));
    HAL_NORFLASH_Erase(HAL_NORFLASH_Sector, 0x1000);

    st_bootInfo.apploadaddress=g_u32address + 0x10000000;
    
    HAL_NORFLASH_WriteByteBuffer(0x1000,(uint8_t *)(&st_bootInfo),sizeof(st_bootInfo)); 
}

void UPGRADE_Upgrade(void const *argument)
{

    FRESULT    fileResult;
    FIL        MyFile;
    uint32_t   u32_bytesRead= RDWR_SECTOR_SIZE;
    uint32_t   u32_recDataSumTmp =0;
    uint32_t   u32_norAddr = 0x20000;
    uint32_t   i;  
    uint8_t    md5_value[MD5_SIZE];  
    MD5_CTX md5;
    g_u8upgradeFlage =0;

    USBH_MountUSBDisk();

    dlog_info("Nor flash init start ... \n");
    NOR_FLASH_Init();
    dlog_info("Nor flash init end   ...\n");
    dlog_output(100);
    SysTicks_DelayMS(500);

    while (1)
    {
        i = HAL_USB_GetMSCPort();

        if (i < HAL_USB_PORT_NUM)
        {
            dlog_info("usb port %d is valid", i);
            break;
        }
        else
        {
            dlog_error("usb port is not valid");
        }
    }

    while (HAL_USB_HOST_STATE_READY != HAL_USB_GetHostAppState(i))
    {
        dlog_info("finding mass storage\n");
        SysTicks_DelayMS(500);
    }

    #if 1
    MD5Init(&md5);
    u32_bytesRead = RDWR_SECTOR_SIZE;
    fileResult = f_open(&MyFile,argument , FA_READ);
    if (FR_OK != fileResult)
    {
        dlog_info("open or create file error: %d\n", fileResult);
        
        vTaskDelete(NULL);
    }          
    while(RDWR_SECTOR_SIZE == u32_bytesRead)
    {            
        memset(g_u8arrayRecData,0,RDWR_SECTOR_SIZE);
        fileResult = f_read(&MyFile, (void *)g_u8arrayRecData, RDWR_SECTOR_SIZE, (void *)&u32_bytesRead);
        if((fileResult != FR_OK))
        {
            dlog_info("Cannot Read from the file \n");
            f_close(&MyFile);
            vTaskDelete(NULL);
        }
        if(g_u8upgradeFlage!=0)
        {               
           MD5Update(&md5, g_u8arrayRecData, u32_bytesRead);
        }
        else
        {                
            memset(g_u8Amd5Sum,0,16);
            BOOT_PrintInfo((uint32_t)(&g_u8arrayRecData));
            u32_norAddr = GET_WORD_BOOT_INOF(&(g_u8arrayRecData[8]));
            u32_norAddr-=0x10000000;
            g_u32address = u32_norAddr;
            MD5Update(&md5, &(g_u8arrayRecData[34]), (u32_bytesRead-34));
            dlog_info("address %x\n",u32_norAddr);  
        }
        dlog_info("checkings file\n");
        dlog_output(100); 
        SysTicks_DelayMS(1);
        g_u32recDataSum+=u32_bytesRead;   
        g_u8upgradeFlage++;           
    }
    MD5Final(&md5, md5_value); 
    f_close(&MyFile);    
    for(i=0;i<16;i++)
    {
        
        if(md5_value[i] != g_u8Amd5Sum[i])
        {
            dlog_info("checksum .........fail\n");
            vTaskDelete(NULL);
        }
    }
    dlog_info("file checksum .........ok\n");
    dlog_output(100); 
    #endif
    u32_norAddr = 0x20000; 
    u32_bytesRead = RDWR_SECTOR_SIZE;
    fileResult = f_open(&MyFile,argument , FA_READ);
    if (FR_OK != fileResult)
    {
        dlog_info("open or create file error: %d\n", fileResult);
        vTaskDelete(NULL);
    }          
    while(RDWR_SECTOR_SIZE == u32_bytesRead)
    {            
        memset(g_u8arrayRecData,0,RDWR_SECTOR_SIZE);
        fileResult = f_read(&MyFile, (void *)g_u8arrayRecData, RDWR_SECTOR_SIZE, (void *)&u32_bytesRead);
        if((fileResult != FR_OK))
        {
            dlog_info("Cannot Read from the file \n");
            f_close(&MyFile);
        }
        dlog_info("fread %d\n",u32_bytesRead);
        HAL_NORFLASH_Erase(HAL_NORFLASH_Sector, u32_norAddr);
        HAL_NORFLASH_WriteByteBuffer(u32_norAddr,g_u8arrayRecData,RDWR_SECTOR_SIZE); 
        dlog_info("write flash %d%%\n",(u32_recDataSumTmp*100/g_u32recDataSum));
        dlog_output(100);          
        u32_recDataSumTmp+=u32_bytesRead; 
        
        u32_norAddr += RDWR_SECTOR_SIZE;                      
    }    
         
    f_close(&MyFile);
    dlog_info("upgrade ok %x\n",g_u32recDataSum);
    dlog_info("start checksum nor_flash .......\n");
    
    UPGRADE_MD5SUM();
    dlog_output(100);
    vTaskDelete(NULL);

}
