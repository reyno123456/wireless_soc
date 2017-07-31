#include <stdint.h> 
#include <stdlib.h>
#include "hal_spi.h"
#include "test_hal_spi_flash.h" 
#include "debuglog.h" 
#include "hal.h"
#include "systicks.h"

uint16_t CLKRATE_MHZ = 6;

static void Test_WbFlashCommand (ENUM_SPI_COMPONENT SPI_BASE_ADDR, uint8_t cmd);
static uint8_t Test_WbFlashReadReg (ENUM_SPI_COMPONENT SPI_BASE_ADDR, uint8_t cmd);
static void Test_WbFlashWriteData (ENUM_SPI_COMPONENT SPI_BASE_ADDR,uint8_t wr_instruc, uint32_t addr, uint8_t data);
static uint8_t Test_WbFlashReadData (ENUM_SPI_COMPONENT SPI_BASE_ADDR,uint8_t rd_instruc, uint32_t addr);
static uint8_t Test_WbBlockErase(ENUM_SPI_COMPONENT SPI_BASE_ADDR, uint8_t cmd, uint32_t addr);
static uint16_t Test_WbFlashID(uint8_t SPI_BASE_ADDR);

/*====================================================*/
/*             WinBond SPI_FLASH Test FUNC            */
/*====================================================*/ 
//Read W25QXX_FLASH ID
void TEST_SPI_init(uint8_t index)
{
    STRU_HAL_SPI_INIT st_spiInitInfo = 
    {
        .u16_halSpiBaudr = CLKRATE_MHZ,
        .e_halSpiPolarity = HAL_SPI_POLARITY_HIGH,
        .e_halSpiPhase = HAL_SPI_PHASE_2EDGE,
    };
    HAL_SPI_MasterInit(index, &st_spiInitInfo);
}

static uint16_t Test_WbFlashID(uint8_t SPI_BASE_ADDR)
{
    uint16_t device_id = 0;
    uint8_t u8Wdate[] = {0x90, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t u8Rbuf[6]; 
   // while(1)
    {
    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8Wdate, 4,u8Rbuf, 2, MAX_TIEM_MS);  
    }
    device_id |= u8Rbuf[0]<< 8;     //DeviceID: 16bits
    device_id |= u8Rbuf[1];

    return device_id ;
}

//
static void Test_WbFlashCommand (ENUM_SPI_COMPONENT SPI_BASE_ADDR, uint8_t cmd)
{
    uint8_t u8Wdate = cmd;
    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR, &u8Wdate, 1, NULL, 0, MAX_TIEM_MS);
}

static uint8_t Test_WbFlashReadReg (ENUM_SPI_COMPONENT SPI_BASE_ADDR, uint8_t cmd)
{
    uint8_t u8Wdate[] = {cmd, 0};
    uint8_t u8Rbuf[2];
    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8Wdate, 1,u8Rbuf, 1, MAX_TIEM_MS);
    
    return u8Rbuf[0];
}

static void Test_WbFlashWriteData (ENUM_SPI_COMPONENT SPI_BASE_ADDR,uint8_t wr_instruc, uint32_t addr, uint8_t data)
{
    uint8_t u8Wdate[] = {wr_instruc, (addr >>16) & 0x000000FF, (addr >> 8) & 0x000000FF, (addr & 0x000000FF),data};    
    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8Wdate, sizeof(u8Wdate),NULL, 0, MAX_TIEM_MS);
}

static uint8_t Test_WbFlashReadData (ENUM_SPI_COMPONENT SPI_BASE_ADDR,uint8_t rd_instruc, uint32_t addr)
{
    uint8_t u8Wdate[] = {rd_instruc,  (addr >>16) & 0x000000FF, (addr >> 8) & 0x000000FF,(addr & 0x000000FF), 0x00};
    uint8_t u8Rbuf[5]; 
    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8Wdate, 4,u8Rbuf, 1, MAX_TIEM_MS);

    return u8Rbuf[0] ;
}

static uint8_t Test_WbBlockErase(ENUM_SPI_COMPONENT SPI_BASE_ADDR, uint8_t cmd, uint32_t addr)
{ 
    uint8_t u8Wdate[] = {cmd, (addr >>16) & 0x000000FF, (addr >> 8) & 0x000000FF, (addr & 0x000000FF)}; 

    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8Wdate, sizeof(u8Wdate),NULL, 0, MAX_TIEM_MS);

}

void command_WbFlashID(char *SPI_BASE_ADDR)
{

    uint8_t u8SpiAddr = strtoul(SPI_BASE_ADDR, NULL, 0);
    TEST_SPI_init(u8SpiAddr);
    dlog_info("ID %d \n",Test_WbFlashID(u8SpiAddr));

}

void command_TestWbBlockErase(char *SPI_BASE_ADDR, char *cmd, char *addr)
{
    uint8_t u8Cmd = strtoul(cmd, NULL, 0);
    uint32_t u32Addr = strtoul(addr, NULL, 0);
    ENUM_SPI_COMPONENT u8SpiAddr = strtoul(SPI_BASE_ADDR, NULL, 0);
    TEST_SPI_init(u8SpiAddr);
    Test_WbBlockErase(u8SpiAddr, u8Cmd, u32Addr);
 
}

void command_TestWbFlash(char *spi_base)
{
    uint8_t u8WEL = 0;
    uint8_t u8BUSY = 0;
    uint8_t u8WPS = 0;
    uint8_t u8S1 = 0;
    uint8_t u8S3 = 0;
    uint8_t u8read_val = 0;
    uint8_t i = 0;
    uint16_t  u8wb_flash_id = 0;
    uint8_t u8wr_dat = 0;
    uint8_t u8rd_dat = 0;
    uint32_t u32_data;

    uint8_t SPI_BASE_ADDR = strtoul(spi_base, NULL, 0);

    TEST_SPI_init(SPI_BASE_ADDR);
    //Device_ID Check 
    u8wb_flash_id = Test_WbFlashID(SPI_BASE_ADDR) ;
    if(u8wb_flash_id == W25Q128_ID)
    {
        dlog_info("Scan WB_Flash Pass!\n");
    }
    else 
    {
        dlog_info("Scan WB_Flash Error! \n:");
    }
 
    //Winbond_SPI_FLASH Setting 
    dlog_info("WB WRITE START\n");

    
    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done

    Test_WbFlashCommand(SPI_BASE_ADDR,0x98);         //global unlock

    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //ready to accept another command

    u8S3 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x15);       //read status reg3; [2] u8WPS;
    u8WPS = u8S3 & 0x04;
    if(!u8WPS)
    {
        dlog_info("u8WPS is zero\n");    //individual lock is disable
    }
    

    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done

    Test_WbBlockErase(SPI_BASE_ADDR,0x20,0x00);        // erase instruction 0X20:4KB;0X52: 32KB; 0XD8:64KB
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //eady to accept another command

    //========================Write data to WB flash======================================
    dlog_info("Flash write Start Check ....\n"); 
    dlog_info("\n"); 
    u8wr_dat = 0 ;
    for(i=0;i<60;i++)
    {
        u8wr_dat = (i % 256) ;

        Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable

        do{
            u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
            u8WEL = u8S1 & 0x02;
        }while(!u8WEL);               //write enable done
        Test_WbFlashWriteData(SPI_BASE_ADDR,0x02,i,u8wr_dat);         //write data i to addr i;
        do{
            u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
            u8BUSY= u8S1 & 0x01;
        }while(u8BUSY);               //write done

    }
    dlog_info("Write data done\n");        //individual lock is disable

    //========================read data from WB flash====================================
    dlog_info("Flash Read Start Check ....\n");    
    u8rd_dat = 0 ;
    for(i=0;i<60;i++)
    {
        u8rd_dat = (i % 256);
        Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
        do{
            u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
            u8WEL = u8S1 & 0x02;
        }while(!u8WEL);               //write enable done

        u8read_val = Test_WbFlashReadData(SPI_BASE_ADDR,0x03,i);  //read data in addr i;        
        do{
            u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
            u8BUSY= u8S1 & 0x01;
        }while(u8BUSY);               //write done
        dlog_info("%x %x \n",u8read_val,u8rd_dat);
    }
    Test_WbFlashCommand(SPI_BASE_ADDR,0x04); //write disable

}

uint8_t u8_wrData[260];
uint8_t u8_rdData[260];

void command_TestWbFlashWrite(char *spi_base, char *addr, char *len)
{
    uint8_t u8WEL = 0;
    uint8_t u8BUSY = 0;
    uint8_t u8WPS = 0;
    uint8_t u8S1 = 0;
    uint8_t u8S3 = 0;
    uint8_t u8read_val = 0;
    uint16_t i = 0;
    uint16_t  u8wb_flash_id = 0;
    uint8_t u8wr_dat = 0;
    uint8_t u8rd_dat = 0;
    uint32_t u32_data;
    
    uint32_t u32_lenUnAlign;
    uint32_t u32_lenAlign;
    uint8_t SPI_BASE_ADDR = strtoul(spi_base, NULL, 0);
    uint32_t u32_addr = strtoul(addr, NULL, 0);
    uint32_t u32_len = strtoul(len, NULL, 0);

    dlog_info("CLKRATE_MHZ:%d",CLKRATE_MHZ);

    TEST_SPI_init(SPI_BASE_ADDR);
    //Device_ID Check 
    u8wb_flash_id = Test_WbFlashID(SPI_BASE_ADDR) ;
    if(u8wb_flash_id == W25Q128_ID)
    {
        dlog_info("Scan WB_Flash Pass!\n");
    }
    else 
    {
        dlog_info("Scan WB_Flash Error! \n:");
    }
 
    //Winbond_SPI_FLASH Setting 
    dlog_info("WB WRITE START\n");
    
    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
    
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done

    Test_WbFlashCommand(SPI_BASE_ADDR,0x98);         //global unlock

    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //ready to accept another command

    u8S3 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x15);       //read status reg3; [2] u8WPS;
    u8WPS = u8S3 & 0x04;
    if(!u8WPS)
    {
        dlog_info("u8WPS is zero\n");    //individual lock is disable
    }
    

    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done

    Test_WbBlockErase(SPI_BASE_ADDR,0x20,0x00);        // erase instruction 0X20:4KB;0X52: 32KB; 0XD8:64KB
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //eady to accept another command

    //========================Write data to WB flash======================================
    dlog_info("Flash write Start Check ....\n"); 

    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable

    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done

    memset(u8_wrData, 0x00, u32_len+4);
    u8_wrData[0] = 0x02;
    u8_wrData[1] = (u32_addr >> 16) & 0xFF;
    u8_wrData[2] = (u32_addr >> 8) & 0xFF;
    u8_wrData[3] = (u32_addr >> 0) & 0xFF;
    for(i=4;i<(u32_len+4);i++)
    {
        u8_wrData[i] = i-3;
    }

    dlog_info("start write data.\n");  
    
    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR, u8_wrData, u32_len+4, NULL, 0, MAX_TIEM_MS*4);
    
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //write done
     

    dlog_info("Write data done\n");        //individual lock is disable

    //========================read data from WB flash====================================
#if 1
    dlog_info("Flash Read Start Check ....\n");    

    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done
    
    memset(u8_rdData, 0x00, u32_len);
    u8_wrData[0] = 0x03;
    u8_wrData[1] = (u32_addr >> 16) & 0xFF;
    u8_wrData[2] = (u32_addr >> 8) & 0xFF;
    u8_wrData[3] = (u32_addr >> 0) & 0xFF;

    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8_wrData, 4,u8_rdData, u32_len, MAX_TIEM_MS*4);

    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //write done

    u32_lenUnAlign = u32_len%10;
    u32_lenAlign = u32_len/10;

    if (0 != u32_lenAlign)
    {
        for(i=0;i<u32_lenAlign*10;i)
        {
            dlog_info("%d %d %d %d %d %d %d %d %d %d",u8_rdData[i+0],u8_rdData[i+1],u8_rdData[i+2],u8_rdData[i+3],
                      u8_rdData[i+4],u8_rdData[i+5],u8_rdData[i+6],u8_rdData[i+7],u8_rdData[i+8],u8_rdData[i+9]);
            i += 10;
           
        }
    }
    
    for(i=0;i<u32_lenUnAlign;i++)
    {
        dlog_info("%d",u8_rdData[u32_lenAlign*10+i]);
    }

    for(i=0;i<u32_len;i++)
    {
        if (u8_rdData[i] != u8_wrData[i+4])
        {
            dlog_info("data[%d] error, write:%d, read:%d", i, u8_wrData[i+4], u8_rdData[i]);
            dlog_output(1000);
        }
    }
#endif

    Test_WbFlashCommand(SPI_BASE_ADDR,0x04); //write disable

}

void command_TestWbFlashRead(char *spi_base, char *addr, char *len)
{
    uint8_t u8WEL = 0;
    uint8_t u8BUSY = 0;
    uint8_t u8WPS = 0;
    uint8_t u8S1 = 0;
    uint8_t u8S3 = 0;
    uint8_t u8read_val = 0;
    uint16_t i = 0;
    uint16_t  u8wb_flash_id = 0;
    uint8_t u8wr_dat = 0;
    uint8_t u8rd_dat = 0;
    uint32_t u32_data;
    
    uint32_t u32_lenUnAlign;
    uint32_t u32_lenAlign;
    uint8_t SPI_BASE_ADDR = strtoul(spi_base, NULL, 0);
    uint32_t u32_addr = strtoul(addr, NULL, 0);
    uint32_t u32_len = strtoul(len, NULL, 0);

    dlog_info("CLKRATE_MHZ:%d",CLKRATE_MHZ);

    TEST_SPI_init(SPI_BASE_ADDR);
    //Device_ID Check 
    u8wb_flash_id = Test_WbFlashID(SPI_BASE_ADDR) ;
    if(u8wb_flash_id == W25Q128_ID)
    {
        dlog_info("Scan WB_Flash Pass!\n");
    }
    else 
    {
        dlog_info("Scan WB_Flash Error! \n:");
    }
 
    //Winbond_SPI_FLASH Setting 
    dlog_info("WB WRITE START\n");
    
    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
    
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done

    Test_WbFlashCommand(SPI_BASE_ADDR,0x98);         //global unlock

    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //ready to accept another command

    u8S3 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x15);       //read status reg3; [2] u8WPS;
    u8WPS = u8S3 & 0x04;
    if(!u8WPS)
    {
        dlog_info("u8WPS is zero\n");    //individual lock is disable
    }
    
    //========================read data from WB flash====================================
    dlog_info("Flash Read Start Check ....\n");    

    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done
    
    memset(u8_rdData, 0x00, u32_len);
    u8_wrData[0] = 0x03;
    u8_wrData[1] = (u32_addr >> 16) & 0xFF;
    u8_wrData[2] = (u32_addr >> 8) & 0xFF;
    u8_wrData[3] = (u32_addr >> 0) & 0xFF;

    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8_wrData, 4,u8_rdData, u32_len, MAX_TIEM_MS*4);
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //write done

    u32_lenUnAlign = u32_len%10;
    u32_lenAlign = u32_len/10;

    if (0 != u32_lenAlign)
    {
        for(i=0;i<u32_lenAlign*10;i)
        {
            dlog_info("%d %d %d %d %d %d %d %d %d %d",u8_rdData[i+0],u8_rdData[i+1],u8_rdData[i+2],u8_rdData[i+3],
                      u8_rdData[i+4],u8_rdData[i+5],u8_rdData[i+6],u8_rdData[i+7],u8_rdData[i+8],u8_rdData[i+9]);
            i += 10;
           
        }
    }
    
    for(i=0;i<u32_lenUnAlign;i++)
    {
        dlog_info("%d",u8_rdData[u32_lenAlign*10+i]);
    }

    Test_WbFlashCommand(SPI_BASE_ADDR,0x04); //write disable
}

void command_TestSetWbFlashClk(char *clk)
{
    CLKRATE_MHZ = strtoul(clk, NULL, 0);

    dlog_info("CLKRATE_MHZ:%d",CLKRATE_MHZ);
}

void command_TestWbFlashWrite_loop(char *spi_base, char *addr, char *len)
{
    uint8_t u8WEL = 0;
    uint8_t u8BUSY = 0;
    uint8_t u8WPS = 0;
    uint8_t u8S1 = 0;
    uint8_t u8S3 = 0;
    uint8_t u8read_val = 0;
    uint16_t i = 0;
    uint16_t  u8wb_flash_id = 0;
    uint8_t u8wr_dat = 0;
    uint8_t u8rd_dat = 0;
    uint32_t u32_data;
    
    uint32_t u32_lenUnAlign;
    uint32_t u32_lenAlign;
    uint8_t SPI_BASE_ADDR = strtoul(spi_base, NULL, 0);
    uint32_t u32_addr = strtoul(addr, NULL, 0);
    uint32_t u32_len = strtoul(len, NULL, 0);

    dlog_info("CLKRATE_MHZ:%d",CLKRATE_MHZ);

    TEST_SPI_init(SPI_BASE_ADDR);
    //Device_ID Check 
    u8wb_flash_id = Test_WbFlashID(SPI_BASE_ADDR) ;
    if(u8wb_flash_id == W25Q128_ID)
    {
        dlog_info("Scan WB_Flash Pass!\n");
    }
    else 
    {
        dlog_info("Scan WB_Flash Error! \n:");
    }
 
    //Winbond_SPI_FLASH Setting 
    dlog_info("WB WRITE START\n");
    
    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
    
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done

    Test_WbFlashCommand(SPI_BASE_ADDR,0x98);         //global unlock

    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //ready to accept another command

    u8S3 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x15);       //read status reg3; [2] u8WPS;
    u8WPS = u8S3 & 0x04;
    if(!u8WPS)
    {
        dlog_info("u8WPS is zero\n");    //individual lock is disable
    }
    

    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done

    Test_WbBlockErase(SPI_BASE_ADDR,0x20,0x00);        // erase instruction 0X20:4KB;0X52: 32KB; 0XD8:64KB
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //eady to accept another command

    //========================Write data to WB flash======================================
    dlog_info("Flash write Start Check ....\n"); 

    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable

    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done

    memset(u8_wrData, 0x00, u32_len+4);
    u8_wrData[0] = 0x02;
    u8_wrData[1] = (u32_addr >> 16) & 0xFF;
    u8_wrData[2] = (u32_addr >> 8) & 0xFF;
    u8_wrData[3] = (u32_addr >> 0) & 0xFF;
    for(i=4;i<(u32_len+4);i++)
    {
        u8_wrData[i] = i-3;
    }

    dlog_info("start write data.\n");  
    
    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR, u8_wrData, u32_len+4, NULL, 0, MAX_TIEM_MS*4);
    
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //write done
     

    dlog_info("Write data done\n");        //individual lock is disable

    //========================read data from WB flash====================================
#if 1
    dlog_info("Flash Read Start Check ....\n");    

    Test_WbFlashCommand(SPI_BASE_ADDR,0x06);         //write enable
    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8WEL = u8S1 & 0x02;
    }while(!u8WEL);               //write enable done
    
    memset(u8_rdData, 0x00, u32_len);
    u8_wrData[0] = 0x03;
    u8_wrData[1] = (u32_addr >> 16) & 0xFF;
    u8_wrData[2] = (u32_addr >> 8) & 0xFF;
    u8_wrData[3] = (u32_addr >> 0) & 0xFF;

    uint8_t tmp = 0x11;
    uint8_t tmp_read;
   
    uint64_t u32_start = SysTicks_GetUsTickCount();
    for (i=0;i<=256;i++)
    {    
        HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,&tmp, 1,&tmp_read, 1, MAX_TIEM_MS*4);
    }
    dlog_info("used %d us", (uint32_t)(SysTicks_GetUsDiff(u32_start, SysTicks_GetUsTickCount())));    
    dlog_output(1000);
    HAL_Delay(100);

    HAL_SPI_MasterWriteRead(SPI_BASE_ADDR,u8_wrData, 4,u8_rdData, u32_len, MAX_TIEM_MS*4);

    do{
        u8S1 = Test_WbFlashReadReg(SPI_BASE_ADDR,0x05);   //read status reg1; [0] u8BUSY; [1] u8WEL;
        u8BUSY= u8S1 & 0x01;
    }while(u8BUSY);               //write done

    u32_lenUnAlign = u32_len%10;
    u32_lenAlign = u32_len/10;

    if (0 != u32_lenAlign)
    {
        for(i=0;i<u32_lenAlign*10;i)
        {
            dlog_info("%d %d %d %d %d %d %d %d %d %d",u8_rdData[i+0],u8_rdData[i+1],u8_rdData[i+2],u8_rdData[i+3],
                      u8_rdData[i+4],u8_rdData[i+5],u8_rdData[i+6],u8_rdData[i+7],u8_rdData[i+8],u8_rdData[i+9]);
            i += 10;
           
        }
    }
    
    for(i=0;i<u32_lenUnAlign;i++)
    {
        dlog_info("%d",u8_rdData[u32_lenAlign*10+i]);
    }

    for(i=0;i<u32_len;i++)
    {
        if (u8_rdData[i] != u8_wrData[i+4])
        {
            dlog_info("data[%d] error, write:%d, read:%d", i, u8_wrData[i+4], u8_rdData[i]);
            dlog_output(1000);
        }
    }
#endif

    Test_WbFlashCommand(SPI_BASE_ADDR,0x04); //write disable

}
