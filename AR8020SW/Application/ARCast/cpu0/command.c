#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "debuglog.h"
#include "test_hal_nv.h"
#include "hal_dma.h"
#include "md5.h"
#include "data_type.h"
#include "cmsis_os.h"
#include "test_hal_i2c.h"
#include "test_hdmi.h"
#include "test_bb.h"
#include "hal_nvic.h"
#include "hal_rtc.h"
#include "arcast_appcommon.h"

void command_readMemory(char *addr);
void command_writeMemory(char *addr, char *value);
static void command_dma(char * u32_src, char *u32_dst, char *u32_byteNum);
static void command_test_dma_loop(char * u32_src, char *u32_dst, char *u32_byteNum);
static void command_set_loglevel(char* cpu, char* loglevel);
unsigned int command_str2uint(char *str);


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
    else if (memcmp(cmdArray[0], "sky_auto_search_rc_id", strlen("sky_auto_search_rc_id")) == 0)
    {
        command_TestNvSkyAutoSearhRcId();
    }
    else if (memcmp(cmdArray[0], "NvResetBbRcId", strlen("NvResetBbRcId")) == 0)
    {
        command_TestNvResetBbRcId();
    }
    else if (memcmp(cmdArray[0], "NvSetBbRcId", strlen("NvSetBbRcId")) == 0)
    {
        command_TestNvSetBbRcId(cmdArray[1],cmdArray[2],cmdArray[3],cmdArray[4],cmdArray[5]);
    }
    else if (memcmp(cmdArray[0], "test_dma_cpu0", strlen("test_dma_cpu0")) == 0)
    {
        command_dma(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if ((memcmp(cmdArray[0], "test_dma_loop", strlen("test_dma_loop")) == 0))
    {
        command_test_dma_loop(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "hal_i2c_init", strlen("hal_i2c_init")) == 0)
    {
        command_TestHalI2cInit(cmdArray[1],cmdArray[2],cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "hal_i2c_read", strlen("hal_i2c_read")) == 0)
    {
        command_TestHalI2cRead(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4]);
    }
    else if ((memcmp(cmdArray[0], "hal_i2c_write", strlen("hal_i2c_write")) == 0))
    {
        command_TestHalI2cWrite(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4], cmdArray[5]);
    }
    else if ((memcmp(cmdArray[0], "bb_uart", strlen("bb_uart")) == 0))
    {
        command_test_BB_uart(cmdArray[1]);
	}
    else if ((memcmp(cmdArray[0], "hdmi", strlen("hdmi")) == 0))
    {
        command_hdmiHandler(cmdArray[1]);
    }
    else if ((memcmp(cmdArray[0], "set_loglevel", strlen("set_loglevel")) == 0))
    {
        command_set_loglevel(cmdArray[1], cmdArray[2]);
	}
    else if ((memcmp(cmdArray[0], "grtc", strlen("grtc")) == 0))
    {

        STRU_HAL_UTC_CALENDAR *pst_halRtcCalendar = (STRU_HAL_UTC_CALENDAR *)malloc(sizeof(STRU_HAL_UTC_CALENDAR) * sizeof(uint8_t));
        HAL_UTC_Get(pst_halRtcCalendar);
        dlog_info("utc %d_%d_%d %d:%d:%d ", pst_halRtcCalendar->u16_year,
                                            pst_halRtcCalendar->u8_month,
                                            pst_halRtcCalendar->u8_day,
                                            pst_halRtcCalendar->u8_hour,
                                            pst_halRtcCalendar->u8_minute,
                                            pst_halRtcCalendar->u8_second);
        free(pst_halRtcCalendar);
    }
    else if ((memcmp(cmdArray[0], "srtc", strlen("srtc")) == 0))
    {
        
        STRU_HAL_UTC_CALENDAR *pst_halRtcCalendar = (STRU_HAL_UTC_CALENDAR *)malloc(sizeof(STRU_HAL_UTC_CALENDAR) * sizeof(uint8_t));
        pst_halRtcCalendar->u16_year = command_str2uint(cmdArray[1]);
        pst_halRtcCalendar->u8_month = command_str2uint(cmdArray[2]);
        pst_halRtcCalendar->u8_day = command_str2uint(cmdArray[3]);
        pst_halRtcCalendar->u8_hour = command_str2uint(cmdArray[4]);
        pst_halRtcCalendar->u8_minute = command_str2uint(cmdArray[5]);
        pst_halRtcCalendar->u8_second = command_str2uint(cmdArray[6]);
        HAL_UTC_Set(pst_halRtcCalendar);
        free(pst_halRtcCalendar);
    }
    else if (memcmp(cmdArray[0], "help", strlen("help")) == 0)
    {
        dlog_critical("Please use the commands like:");
        dlog_critical("read <address>");
        dlog_critical("write <address> <data>");
        dlog_critical("sky_auto_search_rc_id");
        dlog_critical("NvResetBbRcId");
        dlog_critical("NvSetBbRcId <id1> <id2> <id3> <id4> <id5>");
        dlog_critical("test_dma_cpu0 <src> <dst> <byte_num>");
        dlog_critical("test_dma_loop <src> <dst> <byte_num>");
        dlog_critical("hal_i2c_init <ch> <i2c_addr> <speed>");
        dlog_critical("hal_i2c_read <ch> <subAddr> <subAddrLen> <dataLen>");
        dlog_critical("hal_i2c_write <ch> <subAddr> <subAddrLen> <data> <dataLen>");
        dlog_critical("bb_uart");
        dlog_critical("hdmi <index>");
        dlog_critical("set_loglevel <cpuid> <loglevel>");
        dlog_output(1000);
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

        dlog_warning("read address is illegal\n");

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

        dlog_warning("write address is illegal\n");

        return;
    }

    writeValue   = command_str2uint(value);

    *((unsigned int *)(writeAddress)) = writeValue;
}

static void command_dma(char * u32_src, char *u32_dst, char *u32_byteNum)
{
    unsigned int iSrcAddr;
    unsigned int iDstAddr;
    unsigned int iNum;

    iDstAddr    = command_str2uint(u32_dst);
    iSrcAddr    = command_str2uint(u32_src);
    iNum        = command_str2uint(u32_byteNum);


    HAL_DMA_Transfer(iSrcAddr, iDstAddr, iNum, 0);
    
    /* use to fake the dst data */
#if 0
    unsigned char *p_reg;
    p_reg = (unsigned char *)0x81800000;
    *p_reg = 0xAA;
#endif
    /***********************/
    
    //#define MD5_SIZE 16
    uint8_t    md5_value[MD5_SIZE];
    int i = 0;
    MD5_CTX md5;
    MD5Init(&md5);
    MD5Update(&md5, (uint8_t *)iSrcAddr, iNum);
    MD5Final(&md5, md5_value);
    dlog_info("src MD5 = 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++]);

    memset(&md5, 0, sizeof(MD5_CTX));
    MD5Init(&md5);
    MD5Update(&md5, (uint8_t *)iDstAddr, iNum);
    memset(&md5_value, 0, sizeof(md5_value));
    MD5Final(&md5, md5_value);
    i = 0;
    dlog_info("dst MD5 = 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++]);
}

static void command_test_dma_loop(char * u32_src, char *u32_dst, char *u32_byteNum)
{
    unsigned int i = 0;
    
    while(1)
    {
        command_dma(u32_src, u32_dst, u32_byteNum);
        dlog_info("i = %d\n", i++);
    }
}

static void command_set_loglevel(char* cpu, char* loglevel)
{
    uint8_t level = command_str2uint(loglevel);
    if (memcmp(cpu, "cpu0", strlen("cpu0")) == 0)
    {
        dlog_set_output_level(level);
    }

    return;
}

