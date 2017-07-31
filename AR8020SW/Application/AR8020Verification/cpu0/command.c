#include "command.h"
#include "debuglog.h"
#include "interrupt.h"
#include "serial.h"
#include "debuglog.h"
#include <string.h>
#include <stdlib.h>
#include "test_i2c_adv7611.h"
#include "test_freertos.h"
#include "test_hal_can.h"
#include "test_sd.h"
#include "hal_sd.h"
#include "hal_adc.h"
#include "test_quadspi.h"
#include "test_usbh.h"
#include "test_float.h"
#include "test_hal_camera.h"
#include "hal_dma.h"
#include "upgrade.h"
#include "test_bb.h"
#include "testhal_gpio.h"
#include "testhal_timer.h"
#include "testhal_pwm.h"
#include "testhal_softpwm.h"
#include "test_hal_i2c_24c256.h"
#include "test_hal_i2c.h"
#include "test_hal_uart.h"
#include "test_hal_spi.h"
#include "memory_config.h"
#include "hal_ret_type.h"
#include "hal_nvic.h"
#include "hal_usb_host.h"
#include "test_hal_mipi.h"
#include "test_mp3.h"
#include "test_hal_nv.h"
#include "test_hal_spi_flash.h"
#include "md5.h"
#include "test_localirq.h"
#include "testhal_dma.h"
#include "ar_freertos_specific.h"
#include "hal.h"


void command_readMemory(char *addr);
void command_writeMemory(char *addr, char *value);
void command_writeMemory_array(char *addr, char *value, char *len);
void command_readSdcard(char *Dstaddr, char *BlockNum);
void command_writeSdcard(char *Dstaddr, char *BlockNum, char *SrcAddr);
void command_eraseSdcard(char *startBlock, char *blockNum);
void command_upgrade(void);
void command_sendCtrl(void);
void command_sendVideo(void);
void command_adc(char * channel);
void command_usbHostEnterTestMode(void);
void command_malloc(char *size);
void command_sdMount(void);
static void command_set_loglevel(char* cpu, char* loglevel);




void command_run(char *cmdArray[], uint32_t cmdNum)
{
    if ((memcmp(cmdArray[0], "read", 4) == 0) && (cmdNum == 2))
    {
        command_readMemory(cmdArray[1]);
    }
    else if ((memcmp(cmdArray[0], "malloc", strlen("malloc")) == 0) && (cmdNum == 2))
    {
        command_malloc(cmdArray[1]);
    }
    /* write memory: "write $(address) $(data)" */
    else if ((memcmp(cmdArray[0], "write", 5) == 0) && (cmdNum == 3))
    {
        command_writeMemory(cmdArray[1], cmdArray[2]);
    }
    /* write memory array: "write $(address) $(data) $(len)" */
    else if ((memcmp(cmdArray[0], "write_array", 11) == 0) && (cmdNum == 4))
    {
        command_writeMemory_array(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
#if 0
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
#endif
    else if (memcmp(cmdArray[0], "startbypassvideo", strlen("startbypassvideo")) == 0)
    {
        command_startBypassVideo(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "stopbypassvideo", strlen("stopbypassvideo")) == 0)
    {
        command_stopBypassVideo();
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
    else if (memcmp(cmdArray[0], "test_os", strlen("test_os")) == 0)
    {
        command_TestTask(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "freertos_taskquit", strlen("freertos_taskquit")) == 0)
    {
        command_TestTaskQuit();
    }
    else if (memcmp(cmdArray[0], "test_hal_spi_flash_id", strlen("test_hal_spi_flash_id")) == 0)
    {
       command_WbFlashID(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "test_hal_spi_flash_write_read", strlen("test_hal_spi_flash_write_read")) == 0)
    { 
        command_TestWbFlash(cmdArray[1]);                
    }
    else if (memcmp(cmdArray[0], "test_hal_spi_flash_erase", strlen("test_hal_spi_flash_erase")) == 0)
    {
       command_TestWbBlockErase(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "test_hal_spi_flash_loop", strlen("test_hal_spi_flash_loop")) == 0)
    { 
        command_TestWbFlashWrite_loop(cmdArray[1], cmdArray[2], cmdArray[3]);                
    }
    else if (memcmp(cmdArray[0], "test_hal_spi_flash_write", strlen("test_hal_spi_flash_write")) == 0)
    { 
        command_TestWbFlashWrite(cmdArray[1], cmdArray[2], cmdArray[3]);                
    }
    else if (memcmp(cmdArray[0], "test_hal_spi_flash_read", strlen("test_hal_spi_flash_read")) == 0)
    {  
        command_TestWbFlashRead(cmdArray[1], cmdArray[2], cmdArray[3]);                
    }
    else if (memcmp(cmdArray[0], "test_hal_spi_set_flash_clk", strlen("test_hal_spi_set_flash_clk")) == 0)
    {  
        command_TestSetWbFlashClk(cmdArray[1]);                
    }
    else if (memcmp(cmdArray[0], "test_quadspi_speed", strlen("test_quadspi_speed")) == 0)
    {
        command_setQuadSPISpeed(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "test_quadspi_data", strlen("test_quadspi_data")) == 0)
    {
        command_testQuadSPISpeedData();
    }
    else if (memcmp(cmdArray[0], "test_init_flash", strlen("test_init_flash")) == 0)
    {
        command_initWinbondNorFlash();
    }
    else if (memcmp(cmdArray[0], "test_erase_flash", strlen("test_erase_flash")) == 0)
    {
        command_eraseWinbondNorFlash(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_write_flash", strlen("test_write_flash")) == 0)
    {
        command_writeWinbondNorFlash(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "test_read_flash", strlen("test_read_flash")) == 0)
    {
        command_readWinbondNorFlash(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_nor_flash_all", strlen("test_nor_flash_all")) == 0)
    {
        command_testAllNorFlashOperations(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "test_float_calculate_pi", strlen("test_float_calculate_pi")) == 0)
    {
        test_float_calculate_pi();
    }
    else if (memcmp(cmdArray[0], "test_can_init", strlen("test_can_init")) == 0)
    {
         command_TestCanInit(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4], cmdArray[5]);
    } 
    else if (memcmp(cmdArray[0], "test_can_set_int", strlen("test_can_set_int")) == 0)
    {
         command_TestCanSetInt(cmdArray[1], cmdArray[2]);
    } 
    else if (memcmp(cmdArray[0], "test_can_tx", strlen("test_can_tx")) == 0)
    {
        command_TestCanTx(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4], cmdArray[5]);
    }
    else if (memcmp(cmdArray[0], "test_can_rx", strlen("test_can_rx")) == 0)
    {
        command_TestCanRx();
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
    else if(memcmp(cmdArray[0], "command_test_BB_uart", strlen("command_test_BB_uart")) == 0)
    {
        command_test_BB_uart(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "testhal_TestGpioNormal", strlen("testhal_TestGpioNormal")) == 0)
    {
        commandhal_TestGpioNormal(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "testhal_TestGpioInterrupt", strlen("testhal_TestGpioInterrupt")) == 0)
    {
        commandhal_TestGpioInterrupt(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "testhal_TestGetGpio", strlen("testhal_TestGetGpio")) == 0)
    {
        commandhal_TestGetGpio(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "testhal_Testtimer", strlen("testhal_Testtimer")) == 0)
    {
        commandhal_TestTim(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "testhal_Testtiemrall", strlen("testhal_Testtiemrall")) == 0)
    {
        commandhal_TestTimAll();
    }
    else if (memcmp(cmdArray[0], "testhal_Testpwm", strlen("testhal_Testpwm")) == 0)
    {
        commandhal_TestPwm(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "testhal_pwmall", strlen("testhal_pwmall")) == 0)
    {
        commandhal_TestPwmAll();
    }
    else if (memcmp(cmdArray[0], "testhal_simulatepwm", strlen("testhal_simulatepwm")) == 0)
    {
        commandhal_TestSimulatePwm();
    }
    else if (memcmp(cmdArray[0], "testhal24c256", strlen("testhal24c256")) == 0)
    {
        commandhal_Test24C256(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_hal_i2c_init", strlen("test_hal_i2c_init")) == 0)
    {
        command_TestHalI2cInit(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "test_hal_i2c_write", strlen("test_hal_i2c_write")) == 0)
    {
        command_TestHalI2cWrite(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4], cmdArray[5]);
    }
    else if (memcmp(cmdArray[0], "test_hal_i2c_read", strlen("test_hal_i2c_read")) == 0)
    {
        command_TestHalI2cRead(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4]);
    }
    else if (memcmp(cmdArray[0], "test_hal_uart_init", strlen("test_hal_uart_init")) == 0)
    {
        command_TestHalUartInit(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_hal_uart_set_int", strlen("test_hal_uart_set_int")) == 0)
    {
        command_TestHalUartIntSet(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_hal_uart_tx", strlen("test_hal_uart_tx")) == 0)
    {
        command_TestHalUartTx(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_hal_uart_rx", strlen("test_hal_uart_rx")) == 0)
    {
        command_TestHalUartRx(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "test_hal_spi_init", strlen("test_hal_spi_init")) == 0)
    {
        command_TestHalSpiInit(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4]);
    }
    else if (memcmp(cmdArray[0], "test_hal_spi_write", strlen("test_hal_spi_write")) == 0)
    {
        command_TestHalSpiTx(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "test_hal_spi_read", strlen("test_hal_spi_read")) == 0)
    {
        command_TestHalSpiRx(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_dma_cpu0", strlen("test_dma_cpu0")) == 0)
    {
        command_dma(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if ((memcmp(cmdArray[0], "test_dma_loop", strlen("test_dma_loop")) == 0) && (cmdNum == 4))
    {
        command_test_dma_loop(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if ((memcmp(cmdArray[0], "test_dma_driver", strlen("test_dma_driver")) == 0))
    {
        command_test_dma_driver(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4]);
    }
    else if ((memcmp(cmdArray[0], "test_dma_user", strlen("test_dma_user")) == 0))
    {
        command_test_dma_user(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4]);
    }
    else if (memcmp(cmdArray[0], "test_adc", 8) == 0)
    {
        command_adc(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "configure", 8) == 0)
    {
        STRU_SettingConfigure *configure=NULL;
        uint32_t i =0;
        uint32_t j =0;
        GET_CONFIGURE_FROM_FLASH(configure);

        dlog_info("****************        %p       ******************",configure);
        dlog_info("****************        HDMI       ******************");
        dlog_info("%02x %02x %02x",configure->hdmi_configure[0][0],configure->hdmi_configure[0][1],configure->hdmi_configure[0][2]);
        dlog_info("%02x %02x %02x",configure->hdmi_configure[262][0],configure->hdmi_configure[262][1],configure->hdmi_configure[262][2]);
        dlog_info("****************        bb_sky       ******************");
        for(j=0;j<4;j++)
        {
            for(i=0;i<16;i++)
            {
                dlog_info("%02x",configure->bb_sky_configure[j][i*16]);
                dlog_output(100);
            }
            dlog_info("***********************************************");
        }
        dlog_info("***************      bb_grd            ********************");
        for(j=0;j<4;j++)
        {
            for(i=0;i<16;i++)
            {
                dlog_info("%02x",configure->bb_grd_configure[j][i*16]);
                dlog_output(100);
            }
            dlog_info("***********************************************");
        }
        dlog_info("*********        rf           **************");
        for(i=0;i<8;i++)
        {
            dlog_info("%02x",configure->rf1_configure[i*16]);
            dlog_output(100);
        }
        /*for(i=0;i<263;i++)
        {
            dlog_error("%x %x %x",configure->hdmi_configure[i][0],configure->hdmi_configure[i][1],configure->hdmi_configure[i][2]);
            dlog_output(100);
        }
        dlog_error("***********************************************");
        for(i=0;i<263;i++)
        {
            dlog_error("%x %x %x",configure->hdmi_configure1[i][0],configure->hdmi_configure1[i][1],configure->hdmi_configure1[i][2]);
            dlog_output(100);
        }*/
    }
    /* error command */
    else if (memcmp(cmdArray[0], "test_hal_mipi_init", strlen("test_hal_mipi_init")) == 0)
    {
        command_TestHalMipiInit(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "host_test_mode", strlen("host_test_mode")) == 0)
    {
        command_usbHostEnterTestMode();
    }
    else if (memcmp(cmdArray[0], "test_mp3_encoder", strlen("test_mp3_encoder")) == 0)
    {
        command_TestMP3Encoder(cmdArray[1]);
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
    else if (memcmp(cmdArray[0], "test_local_irq", strlen("test_local_irq")) == 0)
    {
        command_TestLocalIrq();
    }  
    else if ((memcmp(cmdArray[0], "set_loglevel", strlen("set_loglevel")) == 0))
    {
        command_set_loglevel(cmdArray[1], cmdArray[2]);
    }
    else if ((memcmp(cmdArray[0], "top", strlen("top")) == 0))
    {
        /* like linux busybox top system call */
        ar_top();
    }
    else if (memcmp(cmdArray[0], "help", strlen("help")) == 0)
    {
        dlog_error("Please use the commands like:");
        dlog_error("read <address>");
        dlog_error("write <address> <data>");
        dlog_error("write_array <address> <data><len>");
        //dlog_error("initsd");
        //dlog_error("readsd <SrcAddr:0x> <SectorNum:0x>");
        //dlog_error("writesd <DstAddr:0x> <SectorNum:0x> <Srcaddr:0x>");
        //dlog_error("erasesd <startSector> <SectorNum>");
        //dlog_error("test_sd <choise>");
        //dlog_error("mount_sd");
        dlog_error("hdmiinit <index>");
        dlog_error("hdmidump <index>");
        dlog_error("hdmigetvideoformat <index>");
        dlog_error("hdmiread <slv address> <reg address>");
        dlog_error("hdmiwrite <slv address> <reg address> <reg value>");
        dlog_error("test_os <choise>");
        dlog_error("freertos_taskquit");
        dlog_error("test_hal_spi_flash_id <spi_port>");
        dlog_error("test_hal_spi_flash_write_read <spi_port>");
        dlog_error("test_hal_spi_flash_erase <spi_port>");
        dlog_error("test_hal_spi_flash_write <spi_port> <start_addr> <data_len>");
        dlog_error("test_hal_spi_flash_read <spi_port> <start_addr> <data_len>");
        dlog_error("test_hal_spi_set_flash_clk <clk Mhz>");
        dlog_error("test_quadspi_speed <speed enum>");
        dlog_error("test_quadspi_data"); 
        dlog_error("test_init_flash");
        dlog_error("test_erase_flash <erase type> <flash start address>");
        dlog_error("test_write_flash <flash start address> <size> <value>");
        dlog_error("test_read_flash <flash start address> <size>");
        dlog_error("test_nor_flash_all <flash start address> <size> <value>");
        dlog_output(1000);
        HAL_Delay(100);
        dlog_error("startbypassvideo");
        dlog_error("stopbypassvideo");
        dlog_error("test_float_calculate_pi");
        dlog_error("command_test_BB_uart <option>");
        dlog_error("upgrade <filename>");
        dlog_error("test_can_init <ch> <br> <acode> <amsk> <format>");
        dlog_error("test_can_set_int <ch> <flag>");
        dlog_error("test_can_tx <ch> <id> <len> <format> <type>");
        dlog_error("test_can_rx");
        dlog_error("testhal_TestGpioNormal <gpionum> <highorlow>");
        dlog_error("testhal_TestGetGpio <gpionum>");
        dlog_error("testhal_TestGpioInterrupt <gpionum> <inttype> <polarity>");        
        dlog_error("testhal_Testtimer <TIM Num> <TIM Count>");
        dlog_error("testhal_Testtiemrall");
        dlog_error("testhal_Testpwm <PWM Num> <PWM low> <PWM high>");
        dlog_error("testhal_pwmall");
        dlog_error("testhal_simulatepwm");
        dlog_error("testhal24c256 <i2c port> <i2c_value>");
        dlog_error("test_hal_i2c_init <ch> <i2c_addr> <speed>");
        dlog_error("test_hal_i2c_write <ch> <subAddr> <subAddrLen> <data> <dataLen>");
        dlog_error("test_hal_i2c_read <ch> <subAddr> <subAddrLen> <dataLen>");
        dlog_error("test_hal_uart_init <ch> <baudr>");
        dlog_error("test_hal_uart_set_int <ch> <flag>");
        dlog_error("test_hal_uart_tx <ch> <len>");
        dlog_error("test_hal_uart_rx <ch>");
        dlog_error("test_hal_spi_init <ch> <baudr> <polarity> <phase>");
        dlog_error("test_hal_spi_write <ch> <addr> <wdata>");
        dlog_error("test_hal_spi_read <ch> <addr>");
        dlog_error("configure");
        dlog_error("test_adc <channel>");
        dlog_error("test_dma_cpu0 <src> <dst> <byte_num>");
        dlog_error("test_dma_loop <src> <dst> <byte_num>");
        dlog_error("test_dma_driver <src> <dst> <byte_num><mode><ms>");        
        dlog_error("test_dma_user <src> <dst> <byte_num><ms>");
        dlog_error("test_camera_init <rate 0~1> <mode 0~8> <toEncoderCh 0~1>");
        dlog_error("test_write_camera <subAddr(hex)> <value>(hex)");
        dlog_error("test_read_camera <subAddr(hex)>");
        dlog_error("test_hal_mipi_init <toEncoderCh 0~1>");
        dlog_error("test_mp3_encoder <0(PCM):1(WAV)>");
        dlog_error("sky_auto_search_rc_id");
        dlog_error("NvResetBbRcId");
        dlog_error("NvSetBbRcId <id1> <id2> <id3> <id4> <id5>");
        dlog_error("test_local_irq");
        dlog_error("malloc <size>");
        dlog_error("set_loglevel <cpuid> <loglevel>");
        dlog_error("top");
        dlog_output(1000);
        HAL_Delay(100);
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

void command_writeMemory_array(char *addr, char *value, char *len)
{
    unsigned int writeAddress;
    unsigned int writeValue;
    unsigned int writeLen;
    unsigned int i;
    unsigned writeAddressIncrease;

    writeAddress = command_str2uint(addr);

    if (writeAddress == 0xFFFFFFFF)
    {

        dlog_error("write address is illegal\n");

        return;
    }

    writeValue   = command_str2uint(value);
    writeLen = command_str2uint(len);

    for (i = 0; i < writeLen; i++)
    {
        writeAddressIncrease = writeAddress + i*sizeof(unsigned int);
        *((unsigned int *)(writeAddressIncrease)) = writeValue;
    }    
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

void command_writeSdcard(char *DstBlkAddr, char *BlockNum, char *SrcAddr)
{
    unsigned int iDstBlkAddr;
    unsigned int iBlockNum;
    unsigned int iSrcAddr;

    iDstBlkAddr    = command_str2uint(DstBlkAddr);
    iBlockNum   = command_str2uint(BlockNum);
    iSrcAddr    = command_str2uint(SrcAddr);

    /* write to sdcard */
    HAL_SD_Write(iDstBlkAddr, iSrcAddr, iBlockNum);

}

void command_eraseSdcard(char *startBlock, char *blockNum)
{
    unsigned int iStartBlock;
    unsigned int iBlockNum;
    iStartBlock = command_str2uint(startBlock);
    iBlockNum   = command_str2uint(blockNum);


    // dlog_info("startBlock = 0x%08x\n", iStartBlock);
    // dlog_info("blockNum = %d\n", iBlockNum);
    HAL_SD_Erase(iStartBlock, iBlockNum);
}

void delay_ms(uint32_t num)
{
    volatile int i;
    for (i = 0; i < num * 100; i++);
}


void command_adc(char * channel)
{
    uint32_t u32_channel;
    u32_channel = command_str2uint(channel);
    uint32_t u32_index = 0;
    while(1)
    {
        if (u32_index++ >= 100)
            break;

        dlog_info("channel %d value: %d", u32_channel, HAL_ADC_Read(u32_channel,u32_channel));
    }
}

void command_usbHostEnterTestMode(void)
{
    dlog_info("enter usb host test mode");

    HAL_USB_EnterUSBHostTestMode();
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

    extern    uint32_t read_reg32(uint32_t *addr);

    dlog_info("INTMASK = 0x%08x", read_reg32((uint32_t *)(0x42000000 + 0x24)));
    dlog_info("MINTSTS = 0x%08x", read_reg32((uint32_t *)(0x42000000 + 0x40)));
    dlog_info("RINTSTS = 0x%08x", read_reg32((uint32_t *)(0x42000000 + 0x44)));
    dlog_info("cdetect = 0x%08x", read_reg32((uint32_t *)(0x42000000 + 0x50)));

	return;
}

void command_sdMount(void)
{
    HAL_SD_Fatfs_Init();
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

