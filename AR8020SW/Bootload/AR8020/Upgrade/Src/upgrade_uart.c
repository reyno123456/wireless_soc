#include <stdint.h>
#include <string.h>

#include "interrupt.h"
#include "serial.h"
#include "debuglog.h"

#include "hal_ret_type.h"
#include "hal_nvic.h"
#include "hal_norflash.h"
#include "hal_uart.h"

#include "upgrade_command.h"
#include "upgrade_core.h"
#include "upgrade_uart.h"
#include "md5.h"

#define UPGRADE_UART_DEBUGE

#ifdef  UPGRADE_UART_DEBUGE
#define UART_UPGRADE
#define DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define DLOG_INFO(...)
#endif

static volatile uint32_t g_u32RecCount;
static volatile uint32_t g_u32LoadAddr;
static volatile uint32_t g_u32ImageSize;
static volatile uint8_t g_u32RecFlage;
static char *g_pDst = (char *)RECEIVE_ADDR;
static uint8_t g_u8Amd5Sum[16];

#define READ_DATA_SIZE  1024*4  
#define MD5_SIZE        16  

static void UPGRADE_EraseWriteFlash(uint32_t u32_addr);

static void UPGRADE_InitParament(void)
{
    g_u32RecCount = 0;
    g_u32ImageSize=0x1000;
    g_u32RecFlage=1;
    g_u32LoadAddr =0;
    memset(g_u8Amd5Sum,0,16);
} 
static int8_t UPGRADE_MD5SUM(uint32_t u32_addr)
{
    uint32_t i=0;
    uint32_t j=0;
    uint32_t u32_RecCountTmp=g_u32RecCount-34;
    uint32_t u32_Count=0;
    uint8_t  md5_value[MD5_SIZE];
    uint8_t  *p8_data = (uint8_t *)(u32_addr+34);

    MD5_CTX md5;
    MD5Init(&md5);

    for(i=0;i<((u32_RecCountTmp)/RDWR_SECTOR_SIZE);i++)
    {
        MD5Update(&md5, (p8_data+RDWR_SECTOR_SIZE*i), RDWR_SECTOR_SIZE);
        u32_Count+=RDWR_SECTOR_SIZE;
    }
    if(0 != ((u32_RecCountTmp)%RDWR_SECTOR_SIZE))
    {
        MD5Update(&md5, (p8_data+RDWR_SECTOR_SIZE*i), (u32_RecCountTmp%RDWR_SECTOR_SIZE));
        u32_Count+=(u32_RecCountTmp%RDWR_SECTOR_SIZE);
    }
    MD5Final(&md5, md5_value);
    for(i=0;i<16;i++)
    {
        if(md5_value[i] != g_u8Amd5Sum[i])
        {
            DLOG_INFO("checksum......fail\n");
            for(j=0;j<16;j++)
            {
                DLOG_INFO("cmp %02x %02x\n",md5_value[j],g_u8Amd5Sum[j]);                
            }
            dlog_output(100);
            return -1;
        }
    }
    DLOG_INFO("checksum......ok\n");
    return 0; 
}

static void UPGRADE_IRQHandler(uint32_t vectorNum)
{
    uint32_t          u32_isrType;
    uint32_t          u32_isrType2;
    uint32_t          u32_status;
    volatile uart_type   *uart_regs =(uart_type *)UART0_BASE;

    u32_isrType    = uart_regs->IIR_FCR;

    if (UART_IIR_RECEIVEDATA == (u32_isrType & 0xf))
    {
        uint8_t i = UART_RX_FIFOLEN;
        while (i--)
        {
            *(g_pDst +g_u32RecCount) = uart_regs->RBR_THR_DLL;        
            g_u32RecCount++;
        }
    }

    if (UART_IIR_DATATIMEOUT == (u32_isrType & 0xf))
    {

        *(g_pDst +g_u32RecCount) = uart_regs->RBR_THR_DLL;        
        g_u32RecCount++;
    }

    // TX empty interrupt.
    if (UART_IIR_THR_EMPTY == (u32_isrType & UART_IIR_THR_EMPTY))
    {
        uart_putFifo(0);
    }
}

uint32_t Upgrade_GetChar(uint8_t *u8_uartRxBuf, uint8_t u8_uartRxLen)
{
    memcpy(g_pDst+g_u32RecCount,u8_uartRxBuf,u8_uartRxLen);
    g_u32RecCount += u8_uartRxLen;
    //g_pDst = (char *)(RECEIVE_ADDR+g_u32RecCount);
}


static void UPGRADE_RollbackIsrHandler(void)
{
}

static void UPGRADE_UartReceive(void)
{
    //sdram init Done
    while(!(SDRAM_INIT_DONE & 0x01))
    {
        ;
    }

    HAL_NVIC_RegisterHandler(HAL_NVIC_UART_INTR0_VECTOR_NUM, UPGRADE_IRQHandler, NULL);

    DLOG_INFO("Nor flash init start ...\n");
    HAL_NORFLASH_Init();
    DLOG_INFO("Nor flash end\n");    
    DLOG_INFO("interrupt\n");    
    uint32_t i=0;
    uint32_t rec=1024*10;
    dlog_output(100);
    //UART_RegisterUserRxHandler(DEBUG_LOG_UART_PORT, Upgrade_GetChar);
        
    while(1)
    {
        if (g_u32ImageSize <= g_u32RecCount)
        {
            break;
        }

        if((1 == g_u32RecFlage)&&(g_u32RecCount>100))
        {
            uint8_t* p8_sizeAddr = (uint8_t*)(RECEIVE_ADDR+14);
            uint8_t* p8_loadAddr = (uint8_t*)(RECEIVE_ADDR+8);
            uint8_t* p8_md5Addr = (uint8_t*)(RECEIVE_ADDR+18);
            g_u32ImageSize = GET_WORD_FROM_ANY_ADDR(p8_sizeAddr);
            g_u32RecFlage =0;
            g_u32LoadAddr = GET_WORD_BOOT_INOF(p8_loadAddr);
            for(i=0;i<16;i++)
            {
                g_u8Amd5Sum[i]=*(p8_md5Addr+i);
            } 
            DLOG_INFO("imagesize %x",g_u32ImageSize);
            dlog_output(100);
        }

        if(rec < g_u32RecCount)
        {
            DLOG_INFO("rec data %d\n",g_u32RecCount);
            rec += 1024*10;
            dlog_output(100);
        }
    }
    DLOG_INFO("receive finish %d\n",g_u32RecCount);
    g_pDst = (char *)RECEIVE_ADDR;
    UPGRADE_RollbackIsrHandler();
}

static void UPGRADE_ModifyBootInfo(void)
{
    uint8_t i=0;
    Boot_Info st_bootInfo;
    memset(&st_bootInfo,0xff,sizeof(st_bootInfo)); 
    HAL_NORFLASH_ReadByteBuffer(0x1000,(uint8_t *)(&st_bootInfo),sizeof(st_bootInfo));
    DLOG_INFO("start checksum upgrade nor flash\n");
    if(0==st_bootInfo.present_boot)
    {
        st_bootInfo.present_boot=1;
        UPGRADE_EraseWriteFlash(BOOT_ADDR1-FLASH_BASE_ADDR);
        if(0 != UPGRADE_MD5SUM(BOOT_ADDR1))
        {
            return;    
        }        
    }
    else
    {
        st_bootInfo.present_boot=0;
        UPGRADE_EraseWriteFlash(BOOT_ADDR0-FLASH_BASE_ADDR);
        if(0 != UPGRADE_MD5SUM(BOOT_ADDR0))
        {
            return;    
        }                
    }
    HAL_NORFLASH_Erase(HAL_NORFLASH_Sector, 0x1000);
    
    HAL_NORFLASH_WriteByteBuffer(0x1000,(uint8_t *)(&st_bootInfo),sizeof(st_bootInfo));
     
}

static void UPGRADE_EraseWriteFlash(uint32_t u32_addr)
{
    uint32_t i=0;
    for(i=0;i<(g_u32RecCount/RDWR_SECTOR_SIZE);i++)
    {
        DLOG_INFO("upgrade  %p %d%%\n",u32_addr+RDWR_SECTOR_SIZE*i,(i*RDWR_SECTOR_SIZE)*100/g_u32RecCount);
        HAL_NORFLASH_Erase(HAL_NORFLASH_Sector, u32_addr+RDWR_SECTOR_SIZE*i);
        HAL_NORFLASH_WriteByteBuffer((u32_addr+RDWR_SECTOR_SIZE*i),(g_pDst+RDWR_SECTOR_SIZE*i),RDWR_SECTOR_SIZE);
        dlog_output(100);
    }
    if(0 != g_u32RecCount%RDWR_SECTOR_SIZE)
    {
        HAL_NORFLASH_Erase(HAL_NORFLASH_Sector, u32_addr+RDWR_SECTOR_SIZE*i);
        HAL_NORFLASH_WriteByteBuffer((u32_addr+RDWR_SECTOR_SIZE*i),(g_pDst+RDWR_SECTOR_SIZE*i),RDWR_SECTOR_SIZE);
    }
    dlog_output(100);
    DLOG_INFO("upgrade  finish\n");
}

void UPGRADE_APPFromUart(void)
{
    UPGRADE_InitParament();
    UPGRADE_UartReceive();    
    if(g_u32LoadAddr<APPLICATION_IMAGE_START)
    {
        DLOG_INFO("image upgrade address error\n");
        dlog_output(100);
        return;
    }
    
    DLOG_INFO("start checksum receive data\n");
    if(-1 != UPGRADE_MD5SUM(RECEIVE_ADDR))
    {
        UPGRADE_EraseWriteFlash(APP_ADDR_OFFSET);
        DLOG_INFO("start checksum upgrade nor flash\n");
        UPGRADE_MD5SUM(APPLICATION_IMAGE_START);

    }
    
    
}

void UPGRADE_BootloadFromUart(void)
{
    UPGRADE_InitParament();
    UPGRADE_UartReceive();
    if((g_u32LoadAddr<BOOT_ADDR0) || (g_u32LoadAddr>=APPLICATION_IMAGE_START) )
    {
        DLOG_INFO("image upgrade address error\n");
        dlog_output(100);
        return;
    }
    DLOG_INFO("start checksum receive data\n");
    if(0 == UPGRADE_MD5SUM(RECEIVE_ADDR))
    {        
        UPGRADE_ModifyBootInfo();
    }   
}
