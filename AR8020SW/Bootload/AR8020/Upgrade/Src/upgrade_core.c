#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "hal_ret_type.h"
#include "hal_norflash.h"

#include "debuglog.h"
#include "upgrade_core.h"
#include "upgrade_command.h"

#ifdef UPGRADE_DEBUGE
#define DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define DLOG_INFO(...)
#endif

#ifdef UPGRADE_DEBUGE_ERROR
#define DLOG_ERROR(...) dlog_error(__VA_ARGS__)
#else
#define DLOG_ERROR(...)
#endif

int8_t UPGRADE_CopyDataToNor(FIL *MyFile,uint32_t u32_addrOffset)
{
    FRESULT           fileResult;
    uint32_t          u32_bytesRead= RDWR_SECTOR_SIZE;
    uint8_t           u8_arrayRecData[RDWR_SECTOR_SIZE];
    uint32_t          u32_recDataSum = 0;
    uint32_t          u32_norAddr = u32_addrOffset;
    int i=0;
    DLOG_INFO("Nor flash init start ...");
    HAL_NORFLASH_Init();
    DLOG_INFO("Nor flash init end   ...");
    while(RDWR_SECTOR_SIZE == u32_bytesRead)
    {
        
        memset(u8_arrayRecData,0,RDWR_SECTOR_SIZE);
        fileResult = f_read(MyFile, u8_arrayRecData, RDWR_SECTOR_SIZE, (void *)&u32_bytesRead);
        if((fileResult != FR_OK))
        {
            DLOG_ERROR("Cannot Read from the file \n");
            f_close(MyFile);
            return UPGRADE_READFILE;
        }
        else
        {
            DLOG_INFO("f_read success %d!",u32_bytesRead);
            u32_recDataSum+=u32_bytesRead;            
            DLOG_INFO("EraseSector start %x!",u32_norAddr);
            HAL_NORFLASH_Erase(HAL_NORFLASH_Sector,u32_norAddr);          
            HAL_NORFLASH_WriteByteBuffer(u32_norAddr, u8_arrayRecData, u32_bytesRead);  
                        
            u32_norAddr += RDWR_SECTOR_SIZE;            
        }        
    }
    DLOG_INFO(" number of totol data %d\n ", u32_recDataSum);

    return UPGRADE_SUCCESS;
}
#if 0
int8_t UPGRADE_CopyDataToITCM(FIL *MyFile,uint32_t u32_TCMADDR)
{
    FRESULT           fileResult;
    uint32_t          u32_cpuAppSize = 0;
    uint32_t          u32_bytesRead= RDWR_SECTOR_SIZE;
    uint8_t           u8_arrayRecData[RDWR_SECTOR_SIZE];
    uint32_t          u32_iCount = 0;
    uint32_t          u32_addr = u32_TCMADDR;
    uint32_t          u32_readCount = 0;
    uint32_t          tmp=0;
    
    while(RDWR_SECTOR_SIZE == u32_bytesRead)
    {
        
        memset(u8_arrayRecData,0,RDWR_SECTOR_SIZE);
        fileResult = f_read(MyFile, u8_arrayRecData, RDWR_SECTOR_SIZE, (void *)&u32_bytesRead);
        if((fileResult != FR_OK))
        {
            DLOG_ERROR("Cannot Read from the file \n");
            f_close(&MyFile);
            return UPGRADE_READFILE;
        }                   
        if(0 == u32_iCount)
        {
            u32_cpuAppSize = (uint32_t)(u8_arrayRecData[0]&0xFF) | ((uint32_t)(u8_arrayRecData[1]&0xFF)) <<8 | 
            (((uint32_t)(u8_arrayRecData[2]&0xFF)) <<16) | (((uint32_t)(u8_arrayRecData[3]&0xFF)) <<24);
            DLOG_INFO("u32_cpuAppSize 0x%x icount %d", u32_cpuAppSize,u32_iCount);
            memcpy((char *)(u32_addr), &u8_arrayRecData[4], RDWR_SECTOR_SIZE -4);
            u32_addr =u32_addr + (RDWR_SECTOR_SIZE -4);
            u32_readCount = u32_cpuAppSize - (RDWR_SECTOR_SIZE -4);
            u32_iCount =1;
            DLOG_INFO("u32_cpuAppSize 0x%x icount %d", u32_cpuAppSize,u32_iCount);
        }
        else
        {
            if(u32_readCount >= RDWR_SECTOR_SIZE)
            {
                u32_readCount -= RDWR_SECTOR_SIZE;
                memcpy((char *)(u32_addr), u8_arrayRecData, RDWR_SECTOR_SIZE);
                u32_addr +=RDWR_SECTOR_SIZE;
                DLOG_INFO("u32_cpuAppSize 0x%x u32_readCount %d", u32_cpuAppSize,u32_readCount);
            }
            else
            {
                tmp=u32_readCount;
                DLOG_INFO("number of data 0x%x tmp %d\n", u32_readCount,tmp);                
                memcpy((char *)(u32_addr), u8_arrayRecData, u32_readCount);
                u32_readCount =0;
                if(u32_bytesRead<RDWR_SECTOR_SIZE)
                    break;
                if(2 == u32_iCount)
                {
                    u32_addr = ITCM2_START;
                    u32_iCount =3;
                }
                else
                {
                    u32_addr = ITCM1_START;
                    u32_iCount =2;
                }
                
                u32_cpuAppSize = (uint32_t)(u8_arrayRecData[tmp]&0xFF) | ((uint32_t)(u8_arrayRecData[tmp+1]&0xFF)) <<8 | 
                (((uint32_t)(u8_arrayRecData[tmp+2]&0xFF)) <<16) | (((uint32_t)(u8_arrayRecData[tmp+3]&0xFF)) <<24);
                DLOG_INFO("u32_cpuAppSize 0x%x, %x\n", u32_cpuAppSize,RDWR_SECTOR_SIZE - tmp -4);
                u32_readCount = u32_cpuAppSize - (RDWR_SECTOR_SIZE - tmp -4);
                memcpy((char *)(u32_addr), &u8_arrayRecData[tmp+4], RDWR_SECTOR_SIZE - tmp -4);             
                u32_addr += (RDWR_SECTOR_SIZE - tmp -4);
                DLOG_INFO("u32_cpuAppSize 0x%x u32_readCount %x\n", u32_cpuAppSize,u32_readCount);                               
            }
        }
        
        dlog_info("upgrade cpu%d %d%%",u32_iCount-1,((u32_cpuAppSize-u32_readCount)*100/u32_cpuAppSize));
        dlog_output(100);
        if(0 != g_u8CheckSum)
        {
            uint32_t  u32_checkSumArray = 0;
            uint32_t  u32_checkSumNor =0;
            uint32_t          i = 0;
            for(i=0;i<u32_bytesRead;i++)
            {
                u32_checkSumArray+=u8_arrayRecData[i];
            }
            for(i=0;i<u32_bytesRead;i++)
            {
                u32_checkSumNor+=u8_arrayRecData[i];
            }
            DLOG_INFO("number of rec data %d  u32_checkSumArray %d u32_checkSumNor %d\n ", u32_bytesRead,u32_checkSumArray,u32_checkSumNor);
        }     
    }
    return UPGRADE_SUCCESS;

}
#endif