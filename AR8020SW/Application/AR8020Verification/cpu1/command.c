#include "command.h"
#include "debuglog.h"
#include "interrupt.h"
#include "serial.h"
#include "stm32f746xx.h"
#include "debuglog.h"
#include <string.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "hal_sd.h"
#include "test_i2c_adv7611.h"
#include "hal_ret_type.h"
#include "hal_nvic.h"
#include "testhal_dma.h"
#include "test_sd.h"
#include "testhal_timer.h"
#include "testhal_pwm.h"

void command_readMemory(char *addr);
void command_writeMemory(char *addr, char *value);
void command_initSdcard();
void command_readSdcard(char *Dstaddr, char *BlockNum);
void command_writeSdcard(char *Dstaddr, char *BlockNum, char *SrcAddr);
void command_eraseSdcard(char *startBlock, char *blockNum);
void command_upgrade(void);
void command_sendCtrl(void);
void command_sendVideo(void);
static void command_set_loglevel(char* cpu, char* loglevel);
void command_sdMount(void);


void command_run(char *cmdArray[], uint32_t cmdNum)
{
    /* read memory: "read $(address)" */
    if ((memcmp(cmdArray[0], "read", 4) == 0) && (cmdNum == 2))
    {
        command_readMemory(cmdArray[1]);
    }
    /* write memory: "write $(address) $(data)" */
    else if ((memcmp(cmdArray[0], "write", 5) == 0) && (cmdNum == 3))
    {
        command_writeMemory(cmdArray[1], cmdArray[2]);
    }
    /* initialize sdcard: "initsd" */
    else if (memcmp(cmdArray[0], "initsd", 6) == 0)
    {
        command_initSdcard();
    }
    /* read sdcard: "readsd $(startBlock) $(blockNum)" */
    else if (memcmp(cmdArray[0], "readsd", 6) == 0)
    {
        command_readSdcard(cmdArray[1], cmdArray[2]);
    }
    /* write sdcard: "writesd $startBlock) $(blockNum) $(data)" */
    else if (memcmp(cmdArray[0], "writesd", 7) == 0)
    {
        command_writeSdcard(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "erasesd", 7) == 0)
    {
        command_eraseSdcard(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_sd", 7) == 0 && (cmdNum == 2))
    {
        command_SdcardFatFs(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "mount_sd", 8) == 0 && (cmdNum == 1))
    {
        command_sdMount();
    }
    else if (memcmp(cmdArray[0], "test_dma_cpu1", strlen("test_dma_cpu1")) == 0)
    {
        command_dma(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if ((memcmp(cmdArray[0], "set_loglevel", strlen("set_loglevel")) == 0))
    {
        command_set_loglevel(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "unmount_sd", strlen("unmount_sd")) == 0)
    {
        command_sd_release();
    }
    else if (memcmp(cmdArray[0], "Testtimer_cpu1", strlen("Testtimer_cpu1")) == 0)
    {
        commandhal_TestTim(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "Testpwm_cpu1", strlen("Testpwm_cpu1")) == 0)
    {
        commandhal_TestPwm(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
/* error command */
    else if (memcmp(cmdArray[0], "help", strlen("help")) == 0)
    {
        dlog_error("Please use the commands like:");
        dlog_error("read <address>");
        dlog_error("write <address> <data>");
        dlog_error("readsd <startBlock> <blockNum>");
        dlog_error("writesd <startBlock> <blockNum> <data>");
        dlog_error("erasesd");
        dlog_error("sendusb");
        dlog_error("hdmiinit");
        dlog_error("hdmigetvideoformat");
        dlog_error("hdmiread <slv address> <reg address>");
        dlog_error("freertos_task");
        dlog_error("freertos_taskquit");
        dlog_error("test_nor_flash_all <flash start address> <size> <value>");
        dlog_error("test_dma_cpu1 <src> <dst> <byte_num>");
        dlog_error("set_loglevel <cpuid> <loglevel>");
        dlog_error("unmount_sd");
    }
}

unsigned int command_str2uint(char *str)
{
    return strtoul(str, NULL, 0); 
}

void command_readMemory(char *addr)
{
    unsigned int readAddress;
    unsigned char row;
    unsigned char column;

    readAddress = command_str2uint(addr);

    if (readAddress == 0xFFFFFFFF)
    {

        dlog_error("read address is illegal\n");

        return;
    }

    /* align to 4 bytes */
    readAddress -= (readAddress % 4);

    /* print to serial */
    for (row = 0; row < 8; row++)
    {
        /* new line */
        dlog_info("0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n ", 
                  readAddress,
                  *(uint32_t *)readAddress,
                  *(uint32_t *)(readAddress + 4),
                  *(uint32_t *)(readAddress + 8),
                  *(uint32_t *)(readAddress + 12),
                  *(uint32_t *)(readAddress + 16),
                  *(uint32_t *)(readAddress + 20),
                  *(uint32_t *)(readAddress + 24),
                  *(uint32_t *)(readAddress + 28));

        readAddress += 32;
    }
}

void command_writeMemory(char *addr, char *value)
{
    unsigned int writeAddress;
    unsigned int writeValue;

    writeAddress = command_str2uint(addr);

    if (writeAddress == 0xFFFFFFFF)
    {

        dlog_error("write address is illegal\n");

        return;
    }

    writeValue   = command_str2uint(value);

    *((unsigned int *)(writeAddress)) = writeValue;
}

void command_readSdcard(char *DstBlkaddr, char *BlockNum)
{
    unsigned int iDstAddr;
    unsigned int iBlockNum;
    unsigned int iSrcBlkAddr;
    unsigned int rowIndex;
    unsigned int columnIndex;
    unsigned int blockIndex;
    char *readSdcardBuff;
    char *bufferPos;

    iSrcBlkAddr   = command_str2uint(DstBlkaddr);
    iBlockNum  = command_str2uint(BlockNum);

/*     readSdcardBuff = m7_malloc(iBlockNum * 512); */
    readSdcardBuff = malloc(iBlockNum * 512);
    if (readSdcardBuff == 0)
    {
        dlog_info("malloc error");
        return;
    }
    memset(readSdcardBuff, 0, iBlockNum * 512);
    bufferPos = readSdcardBuff;

    // dlog_info("iSrcBlock = 0x%08x\n", iSrcAddr);
    // dlog_info("iBlockNum = 0x%08x\n", iBlockNum);
    // dlog_info("readSdcardBuff = 0x%08x\n", readSdcardBuff);

    /* read from sdcard */
    HAL_SD_Read((uint32_t)bufferPos, iSrcBlkAddr, iBlockNum);

    /* print to serial */
    for (blockIndex = iSrcBlkAddr; blockIndex < (iSrcBlkAddr + iBlockNum); blockIndex++)
    {
        dlog_info("==================block: %d=================",blockIndex);
        for (rowIndex = 0; rowIndex < 16; rowIndex++)
        {
            /* new line */
            dlog_info("0x%x: ",(unsigned int)((rowIndex << 5) + (blockIndex << 9)));
            for (columnIndex = 0; columnIndex < 1; columnIndex++)
            {
                dlog_info("0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x", 
                           *((unsigned int *)bufferPos), 
                           *((unsigned int *)(bufferPos + 4)), 
                           *((unsigned int *)(bufferPos + 8)), 
                           *((unsigned int *)(bufferPos + 12)),
                           *((unsigned int *)(bufferPos + 16)),
                           *((unsigned int *)(bufferPos + 20)),
                           *((unsigned int *)(bufferPos + 24)),
                           *((unsigned int *)(bufferPos + 28)));
                bufferPos += 32;
            }
        }
        dlog_info("\n");
    }
/*     m7_free(readSdcardBuff); */
    free(readSdcardBuff);

}


void command_writeSdcard(char *Dstaddr, char *BytesNum, char *SrcAddr)
{
    unsigned int iDstAddr;
    unsigned int iBytesNum;
    unsigned int iSrcAddr;
    char *writeSdcardBuff;

    iDstAddr    = command_str2uint(Dstaddr);
    iBytesNum   = command_str2uint(BytesNum);
    iSrcAddr    = command_str2uint(SrcAddr);
#if 0
    writeSdcardBuff = m7_malloc(iBytesNum);
    memset(writeSdcardBuff, SrcAddr, iBytesNum);

    /* write to sdcard */
    HAL_SD_Write(&sdhandle, iDstAddr, iBytesNum, writeSdcardBuff);
    m7_free(writeSdcardBuff);
#endif


    dlog_info("writeSdcardBuff = 0x%08x\n", writeSdcardBuff);

    dlog_info("iDstAddr = 0x%08x\n", iDstAddr);

    dlog_info("iBytesNum = 0x%08x\n", iBytesNum);

    dlog_info("iSrcAddr = 0x%08x\n", iSrcAddr);

}

void command_eraseSdcard(char *startBlock, char *blockNum)
{
    unsigned int iStartBlock;
    unsigned int iBlockNum;
    iStartBlock = command_str2uint(startBlock);
    iBlockNum   = command_str2uint(blockNum);


    dlog_info("startBlock = 0x%08x\n", iStartBlock);
    // HAL_SD_Erase(&sdhandle, iStartBlock, iBlockNum);
    dlog_info("blockNum = 0x%08x\n", iBlockNum);
}

void delay_ms(uint32_t num)
{
    volatile int i;
    for (i = 0; i < num * 100; i++);
}

static void command_set_loglevel(char* cpu, char* loglevel)
{
    uint8_t level = command_str2uint(loglevel);
    if (memcmp(cpu, "cpu1", strlen("cpu1")) == 0)
    {
        dlog_set_output_level(level);
}

    return;
}

void command_sdMount(void)
{
    HAL_SD_Fatfs_Init();
}


