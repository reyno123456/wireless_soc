#include "command.h"
#include "debuglog.h"
#include "interrupt.h"
#include "debuglog.h"
#include <string.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "test_i2c_adv7611.h"
#include "test_hal_camera.h"
#include "hal_dma.h"
#include "upgrade.h"
#include "memory_config.h"
#include "hal_ret_type.h"
#include "test_hal_mipi.h"
#include "test_usbh.h"
#include "test_hal_nv.h"
#include "ar_freertos_specific.h"
#include "test_bb.h"
#include "test_sd.h"

void command_readMemory(char *addr);
void command_writeMemory(char *addr, char *value);
void command_upgrade(void);
void command_malloc(char *size);
static void command_set_loglevel(char* cpu, char* loglevel);


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
    else if (memcmp(cmdArray[0], "upgrade", strlen("upgrade")) == 0)
    {
        char path[128];
        memset(path,'\0',128);
        if(strlen(cmdArray[1])>127)
        {
            return;
        }
        memcpy(path,cmdArray[1],strlen(cmdArray[1]));
        path[strlen(cmdArray[1])]='\0';
        osThreadDef(UsbUpgrade, UPGRADE_Upgrade, osPriorityNormal, 0, 15 * 128);
        osThreadCreate(osThread(UsbUpgrade), path);
        vTaskDelay(100);       
    }
    else if (memcmp(cmdArray[0], "hdmiinit", strlen("hdmiinit")) == 0)
    {
        command_initADV7611(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "hdmidump", strlen("hdmidump")) == 0)
    {
        command_dumpADV7611Settings(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "hdmigetvideoformat", strlen("hdmigetvideoformat")) == 0)
    {
        uint16_t width, hight;
        uint8_t framterate;
        command_readADV7611VideoFormat(cmdArray[1], &width, &hight, &framterate);
        dlog_info("width %d, hight %d, framterate %d\n", width, hight, framterate);
    }
    else if (memcmp(cmdArray[0], "hdmiread", strlen("hdmiread")) == 0)
    {
        command_readADV7611(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "hdmiwrite", strlen("hdmiwrite")) == 0)
    {
        command_writeADV7611(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if(memcmp(cmdArray[0], "test_camera_init", strlen("test_camera_init")) == 0)
    {
        command_TestHalCameraInit(cmdArray[1], cmdArray[2]);
        command_TestHalMipiInit(cmdArray[3]);
    }
    else if(memcmp(cmdArray[0], "test_write_camera", strlen("test_write_camera")) == 0)
    {
        command_TestCameraWrite(cmdArray[1], cmdArray[2]);
    }
    else if(memcmp(cmdArray[0], "test_read_camera", strlen("test_camera_read")) == 0)
    {
        command_TestCameraRead(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "test_hal_mipi_init", strlen("test_hal_mipi_init")) == 0)
    {
        command_TestHalMipiInit(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "startbypassvideo", strlen("startbypassvideo")) == 0)
    {
        command_startBypassVideo(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "stopbypassvideo", strlen("stopbypassvideo")) == 0)
    {
        command_stopBypassVideo();
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
    else if(memcmp(cmdArray[0], "command_test_BB_uart", strlen("command_test_BB_uart")) == 0)
    {
        command_test_BB_uart(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "initsd", 6) == 0)
    {
        command_initSdcard();
    }
    else if (memcmp(cmdArray[0], "test_sd", 7) == 0 && (cmdNum == 2))
    {
        command_SdcardFatFs(cmdArray[1]);
    }
    else if ((memcmp(cmdArray[0], "malloc", strlen("malloc")) == 0) && (cmdNum == 2))
    {
        command_malloc(cmdArray[1]);
    }
    else if ((memcmp(cmdArray[0], "top", strlen("top")) == 0))
    {
        /* like linux busybox top system call */
        ar_top();
    }
    else if ((memcmp(cmdArray[0], "set_loglevel", strlen("set_loglevel")) == 0))
    {
        command_set_loglevel(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "help", strlen("help")) == 0)
    {
        dlog_critical("Please use the commands like:");
        dlog_critical("read <address>");
        dlog_critical("write <address> <data>");
        dlog_critical("hdmiinit <index>");
        dlog_critical("hdmidump <index>");
        dlog_critical("hdmigetvideoformat <index>");
        dlog_critical("hdmiread <slv address> <reg address>");
        dlog_critical("hdmiwrite <slv address> <reg address> <reg value>");
        dlog_critical("upgrade <filename>");
        dlog_critical("test_camera_init <rate 0~1> <mode 0~8> <toEncoderCh 0~1>");
        dlog_critical("test_write_camera <subAddr(hex)> <value>(hex)");
        dlog_critical("test_read_camera <subAddr(hex)>");
        dlog_critical("test_hal_mipi_init <toEncoderCh 0~1>");
        dlog_critical("startbypassvideo");
        dlog_critical("stopbypassvideo");
        dlog_critical("sky_auto_search_rc_id");
        dlog_critical("NvResetBbRcId");
        dlog_critical("NvSetBbRcId <id1> <id2> <id3> <id4> <id5>");
        dlog_critical("command_test_BB_uart <param>");
		dlog_critical("initsd");
        dlog_critical("test_sd <choise>");
        dlog_critical("malloc <size>");
        dlog_critical("top");
        dlog_critical("set_loglevel <cpuid> <loglevel>");
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

void delay_ms(uint32_t num)
{
    volatile int i;
    for (i = 0; i < num * 100; i++);
}

void command_malloc(char *size)
{
    unsigned int mallocSize;
	char *malloc_addr;
	
    mallocSize = command_str2uint(size);
	malloc_addr = malloc(mallocSize);

	if (malloc_addr != 0)
	{
		dlog_info("0x%08x\n", malloc_addr);
	}

	return;
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

