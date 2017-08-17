#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "upgrade.h"
#include "bb_ctrl_internal.h"
#include "debuglog.h"
#include "md5.h"
#include "ff.h"
#include "hal_bb.h"
#include "hal.h"
#include "hal_norflash.h"
#include "hal_usb_host.h"


void UPGRADE_GNDInit(void);

uint8_t s_u8_upgradeStatus = 0;
//next data block number
uint16_t s_u16_dataBlockNum = 0;
//lose data block number
uint16_t s_u16_lostDataBlockArray[UPGRADE_MAX_BLOCKARRAYSIZE];
//lose data block number count
uint8_t s_u8_lostDataBlockCount = 0;
//receive data length
uint32_t s_u32_upgradeRecDataLen = 0;
//file size
uint32_t s_u32_upgradeTotalDataLen = 0;
//sky :total data block count
//ground :present send data block number
uint16_t s_u16_upgradeReturnStatus = 0;

int8_t UpgradeState_Reset(void)
{
    s_u8_upgradeStatus = 0;
}

int8_t UpgradeGndState_Clear(ENUM_UPGRADE_GROUND_STATE e_state)
{
    s_u8_upgradeStatus &= (~(1 << e_state));
}

int8_t UpgradeSkyState_Clear(ENUM_UPGRADE_SKY_STATE e_state)
{
    s_u8_upgradeStatus &= (~(1 << e_state));
}

int8_t UpgradeGndState_Set(ENUM_UPGRADE_GROUND_STATE e_state)
{
    s_u8_upgradeStatus |= 1 << e_state;
}

int8_t UpgradeSkyState_Set(ENUM_UPGRADE_SKY_STATE e_state)
{
    s_u8_upgradeStatus |= 1 << e_state;
}

int8_t UpgradeGndStatus_Get(ENUM_UPGRADE_GROUND_STATE e_state)
{
    if (1 == (s_u8_upgradeStatus >> e_state) & 1)
    {
        UpgradeGndState_Clear(UPGRADE_ACK_LOSTDATA);
        return 1;
    }
    else
    {
        return ((s_u8_upgradeStatus >> e_state) & 1);
    }
}

int8_t UpgradeSkyState_Get(ENUM_UPGRADE_SKY_STATE state)
{
    return ((s_u8_upgradeStatus >> state) & 1);
}

void UPGRADE_GetStatus(STRU_UPGRADE_STATUS * pst_status)
{
    pst_status->u32_fileSize = s_u32_upgradeTotalDataLen;
    if (s_u16_upgradeReturnStatus * UPGRADE_DATACHUNK_SIZE > s_u32_upgradeTotalDataLen)
    {
        pst_status->u32_sentFileSize = s_u32_upgradeTotalDataLen;
    }
    else
    {
        pst_status->u32_sentFileSize = s_u16_upgradeReturnStatus * UPGRADE_DATACHUNK_SIZE;
    }

    if (UpgradeGndStatus_Get(UPGRADE_ACK_END))
    {
        pst_status->e_upgradeSkyStatus = UPGRADE_OK;
    }
    else if (UpgradeGndStatus_Get(UPGRADE_ACK_FAIL))
    {
        pst_status->e_upgradeSkyStatus = UPGRADE_FAIL;
    }
    else
    {
        pst_status->e_upgradeSkyStatus = UPGRADE_UPGRADING;
    }
    

}

static void insertion_sort_u16(uint16_t *buff, uint8_t Length)
{
    uint16_t temp = 0;
    uint8_t i =0, j = 0;
    for (uint8_t i = 1; i < Length; i++)
    {
        if (buff[i - 1] > buff[i])
        {
            temp = buff[i];
            j = i;
            while (j > 0 && buff[j - 1] > temp)
            {
                buff[j] = buff[j - 1];
                j--;
            }
            buff[j] = temp;
        }
    }
}
// session_4 send data 
static uint32_t UPGRADE_SendMsg(const STRU_skyStatusMsg *pst_pack, const uint8_t *pu8_buff, uint16_t u16_dataLen)
{
    uint8_t u8_msgArray[1024];
    uint8_t  md5_value[MD5_SIZE];
    uint32_t u32_count = sizeof(STRU_skyStatusMsg);
    
    if (u16_dataLen + u32_count > 1024)
    {
        dlog_error("send data overflow, sizeof(STRU_skyStatusMsg) + u16_dataLen > 1024");
        return 1;
    }

    memcpy(u8_msgArray, pst_pack, u32_count);
 
    if ((NULL != pu8_buff) && (0 != u16_dataLen))
    {
        memcpy(&u8_msgArray[u32_count], pu8_buff, u16_dataLen);
        u32_count += u16_dataLen;
    } 
       
    return HAL_BB_UartComSendMsg(UPGRADE_SESSION, u8_msgArray, u32_count);
}
//flash erase write flash
void UPGRADE_EraseWriteFlash(uint32_t u32_addr)
{
    uint32_t i=0;
    for(i=0;i<(s_u32_upgradeRecDataLen/UPGRADE_BLOCK_SIZE);i++)
    {
        dlog_critical("upgrade  %p %d%%\n",u32_addr+UPGRADE_BLOCK_SIZE*i,(i*UPGRADE_BLOCK_SIZE)*100/s_u32_upgradeRecDataLen);
        HAL_NORFLASH_Erase(HAL_NORFLASH_Block, u32_addr+UPGRADE_BLOCK_SIZE*i);
        HAL_NORFLASH_WriteByteBuffer((u32_addr+UPGRADE_BLOCK_SIZE*i),(uint8_t *)(UPGRADE_SDRAM_DATA_ADDR+UPGRADE_BLOCK_SIZE*i),UPGRADE_BLOCK_SIZE);
    }
    if(0 != s_u32_upgradeRecDataLen%UPGRADE_BLOCK_SIZE)
    {
        HAL_NORFLASH_Erase(HAL_NORFLASH_Block, u32_addr+UPGRADE_BLOCK_SIZE*i);
        HAL_NORFLASH_WriteByteBuffer((u32_addr+UPGRADE_BLOCK_SIZE*i),(uint8_t *)(UPGRADE_SDRAM_DATA_ADDR+UPGRADE_BLOCK_SIZE*i),UPGRADE_BLOCK_SIZE);
    }
    
    dlog_critical("upgrade  finish\n");
}

static int8_t MD5SUM_Generate(uint8_t *pu8_addr, uint32_t u32_dataLen, uint8_t *pu8_md5)
{
    uint8_t  md5_value[MD5_SIZE];

    MD5_CTX md5;
    MD5Init(&md5);
    MD5Update(&md5, pu8_addr, u32_dataLen);
    MD5Final(&md5, md5_value);

    
    if (NULL != pu8_md5)
    {
        memcpy(pu8_md5, md5_value, MD5_SIZE);
    }

    return 0; 
    
}

int8_t UPGRADE_MD5SUM(uint8_t *pu8_addr, uint32_t u32_dataLen)
{
    uint32_t i=0;
    uint32_t u32_RecCountTmp=s_u32_upgradeRecDataLen-IMAGE_HEAD_SIZE;
    uint32_t u32_Count=0;
    uint8_t  md5_value[MD5_SIZE];
    uint8_t  u8Amd5Sum[MD5_SIZE];
    uint8_t  *p8_data = (pu8_addr+IMAGE_HEAD_SIZE);
    uint8_t* p8_md5Addr = (pu8_addr+IMAGE_MD5_OFFSET);
    for(i=0;i<MD5_SIZE;i++)
    {
        u8Amd5Sum[i]=*(p8_md5Addr+i);
    }
    MD5SUM_Generate(p8_data, u32_RecCountTmp, md5_value); 
    if (0 != memcmp(md5_value, u8Amd5Sum, MD5_SIZE))
    {
        for(i=0;i<MD5_SIZE;i++)
        {
            dlog_error("cmp %02x %02x\n",md5_value[i],u8Amd5Sum[i]);                
        }
        dlog_error("checksum......fail\n");
        return -1;
        
    }
    else
    {
        dlog_critical("checksum......ok\n");
        return 0;
    }    
}
int8_t UPGRADE_CheckUpgradeFile(const uint8_t *fileName)
{
    FRESULT    fileResult;
    FIL        MyFile;
    uint32_t   u32_bytesRead = UPGRADE_DATACHUNK_SIZE;
    uint8_t  u8_sendDataArray[UPGRADE_DATACHUNK_SIZE];
    uint32_t i = 0;
    MD5_CTX  md5;
    uint8_t  md5_value[MD5_SIZE];
    uint8_t  u8Amd5Sum[MD5_SIZE];


    fileResult = f_open(&MyFile, fileName, FA_READ);
    if (FR_OK != fileResult)
    {
        dlog_error("open or create file error: %d\n", fileResult);
        return -1;
    }
    
    MD5Init(&md5);
    u32_bytesRead = UPGRADE_DATACHUNK_SIZE;
    
    while(UPGRADE_DATACHUNK_SIZE == u32_bytesRead)
    {            
        fileResult = f_read(&MyFile, (void *)u8_sendDataArray, UPGRADE_DATACHUNK_SIZE, (void *)&u32_bytesRead);
        if((fileResult != FR_OK))
        {
            dlog_error("Cannot Read from the file \n");
            f_close(&MyFile);
            
        }
        if(s_u32_upgradeTotalDataLen != 0)
        {               
           MD5Update(&md5, u8_sendDataArray, u32_bytesRead);
        }
        else
        {                
            memcpy(u8Amd5Sum, &u8_sendDataArray[IMAGE_MD5_OFFSET], MD5_SIZE);
            MD5Update(&md5, &(u8_sendDataArray[IMAGE_HEAD_SIZE]), (u32_bytesRead-IMAGE_HEAD_SIZE));             
        }        
        s_u32_upgradeTotalDataLen += u32_bytesRead;         
    }
    MD5Final(&md5, md5_value);  
    if (0 != memcmp(md5_value, u8Amd5Sum, MD5_SIZE))
    {
        dlog_info("file checksum .........fail\n");
        for(i=0;i<MD5_SIZE;i++)
        {
            dlog_error(" md5 %02x %02x",md5_value[i], u8Amd5Sum[i]);
        }
        return -1;
    }
    else
    {
        dlog_critical("file checksum .........ok\n");
        return 0;
    }
    f_close(&MyFile);

}

uint32_t UPGRADE_SendDataBlock(uint8_t u8_opt, uint8_t *pu8_dataBuff, uint16_t u16_dataLen, uint16_t u16_dataBlockNum)
{
    STRU_skyStatusMsg st_upgrade;
    st_upgrade.pid = SKY_UPGRADE;
    st_upgrade.par.upgradeDataPack.u8_opt = u8_opt;
    st_upgrade.par.upgradeDataPack.u16_data[0] = u16_dataBlockNum;
    st_upgrade.par.upgradeDataPack.u16_data[1] = u16_dataLen;
    st_upgrade.par.upgradeDataPack.u16_data[2] = 0;
    MD5SUM_Generate(pu8_dataBuff, u16_dataLen, st_upgrade.par.upgradeDataPack.u8_md5);
    return UPGRADE_SendMsg(&st_upgrade, pu8_dataBuff, u16_dataLen);    
}

void UPGRADE_rcvHandleSky(void *p)
{
    uint8_t u8_recDataArray[1024];
    uint8_t  md5_value[MD5_SIZE];
    uint16_t u16_dataBlockNum = 0;
    uint16_t u16_dataLen = 0;
    uint32_t i = 0, j = 0, u32_savedSize = 0;
    STRU_skyStatusMsg st;
    
    uint32_t u32_rcvLen = BB_UARTComReceiveMsg(UPGRADE_SESSION, u8_recDataArray, 1024);
    memcpy(&st, u8_recDataArray, sizeof(STRU_skyStatusMsg));

    if (SKY_UPGRADE == u8_recDataArray[0])
    {
        switch(st.par.upgradeDataPack.u8_opt)
        {
            // receive ground init command
            case UPGRADE_DATA_INIT:
            {
                // init parameter
                UpgradeSkyState_Set(UPGRADE_DATA_INIT);
                memset(s_u16_lostDataBlockArray, 0xff, sizeof(s_u16_lostDataBlockArray));
                s_u8_lostDataBlockCount = 0;
                s_u16_dataBlockNum = 0;
                st.par.upgradeDataPack.u8_opt = UPGRADE_ACK_INIT;
                s_u16_upgradeReturnStatus = st.par.upgradeDataPack.u16_data[0];
                // reply ground init ok
                while (0 != UPGRADE_SendMsg(&st, NULL, 0))
                {
                    HAL_Delay(UPGRADE_DELAY);
                }
                dlog_critical("UPGRADE_DATA_INIT %d",s_u16_upgradeReturnStatus);
            }
                break;
            case UPGRADE_DATA_DATA:
            case UPGRADE_LOSTBLOCK_DATA:
            {
                //receive data
                u16_dataBlockNum = st.par.upgradeDataPack.u16_data[0];
                u16_dataLen = st.par.upgradeDataPack.u16_data[1];
                dlog_critical("UPGRADE_DATA_DATA %d%%\n",(s_u16_dataBlockNum*100/s_u16_upgradeReturnStatus));
                MD5SUM_Generate(&u8_recDataArray[sizeof(STRU_skyStatusMsg)], u16_dataLen, md5_value);
                //check md5
                if (0 != memcmp(md5_value, st.par.upgradeDataPack.u8_md5, MD5_SIZE))
                {
                    for(i=0;i<MD5_SIZE;i++)
                    {
                        dlog_error("cmp %02x %02x",md5_value[i],st.par.upgradeDataPack.u8_md5[i]);                
                    }
                }
                else
                {              
                    //receive data 
                    memcpy((uint8_t *)(UPGRADE_SDRAM_DATA_ADDR + UPGRADE_DATACHUNK_SIZE * u16_dataBlockNum), &u8_recDataArray[sizeof(STRU_skyStatusMsg)],u16_dataLen);
                    if (UPGRADE_DATA_DATA == st.par.upgradeDataPack.u8_opt)
                    {
                        //if receive data number lose, storage lose num.
                        if (s_u16_dataBlockNum != st.par.upgradeDataPack.u16_data[0])
                        {
                           for (i = s_u16_dataBlockNum; i < st.par.upgradeDataPack.u16_data[0]; i++)
                           {
                               if (s_u8_lostDataBlockCount < UPGRADE_MAX_BLOCKARRAYSIZE)
                               {
                                    s_u16_lostDataBlockArray[s_u8_lostDataBlockCount] = i;
                               }

                               s_u8_lostDataBlockCount++;
                               dlog_error("lose block %d",i);
                           }
                        }

                        s_u32_upgradeRecDataLen += st.par.upgradeDataPack.u16_data[1];            
                        //data block num add 1
                        s_u16_dataBlockNum = st.par.upgradeDataPack.u16_data[0] + 1;                        
                    }
                    else
                    {
                        //receive lose data
                        for (i = 0; i < UPGRADE_MAX_BLOCKARRAYSIZE; i++)
                        {
                            if (u16_dataBlockNum == s_u16_lostDataBlockArray[i])
                            {
                                s_u16_lostDataBlockArray[i] = 0xffff;
                                s_u32_upgradeRecDataLen += u16_dataLen;
                                s_u8_lostDataBlockCount--;
                                dlog_critical("rec lost data match %d", s_u32_upgradeRecDataLen);
                                continue;
                            }
                        }
                    }
                }
                break;
            }
            case UPGRADE_DATA_END:
            {
                dlog_critical("UPGRADE_DATA_END %d",st.par.upgradeDataPack.u16_data[0]);
                //check lose count
                if (s_u8_lostDataBlockCount >= UPGRADE_MAX_BLOCKARRAYSIZE)
                {
                    st.par.upgradeDataPack.u8_opt = UPGRADE_ACK_FAIL;
                    while (0 != UPGRADE_SendMsg(&st, NULL, 0))
                    {
                        HAL_Delay(UPGRADE_DELAY);
                    }
                    dlog_error("lose data count overflow %d", s_u8_lostDataBlockCount);
                    break;
                }

                //send lose data block number
                if (s_u8_lostDataBlockCount > 0)
                {
                    insertion_sort_u16(s_u16_lostDataBlockArray, UPGRADE_MAX_BLOCKARRAYSIZE);
                    
                    st.par.upgradeDataPack.u8_opt = UPGRADE_ACK_LOSTDATA;
                    st.par.upgradeDataPack.u16_data[0] = s_u8_lostDataBlockCount;
                    
                    for (i = 0; i < s_u8_lostDataBlockCount; i++)
                    {
                        dlog_info("lose data num %d", s_u16_lostDataBlockArray[i]);
                    }
                    while (0 != UPGRADE_SendMsg(&st, (uint8_t *)(&s_u16_lostDataBlockArray[0]), s_u8_lostDataBlockCount*sizeof(uint16_t)))
                    {
                        HAL_Delay(UPGRADE_DELAY);
                    }

                    dlog_info("send lose data block %d", s_u8_lostDataBlockCount);

                }
                else 
                {
                    dlog_critical("Md5 check rec dataLen %x",s_u32_upgradeRecDataLen);
                    //check recevie data 
                    if (-1 == UPGRADE_MD5SUM((uint8_t *)UPGRADE_SDRAM_DATA_ADDR, s_u32_upgradeRecDataLen))
                    {
                        st.par.upgradeDataPack.u8_opt = UPGRADE_ACK_FAIL;
                        dlog_error("Md5 check fail");
                    }
                    else
                    {
                        if (0 == UpgradeSkyState_Get(UPGRADE_UPGRADE_END))
                        {
                            UPGRADE_EraseWriteFlash(APP_ADDR_OFFSET);
                            dlog_critical("upgrading");                        
                        }
                        UpgradeSkyState_Set(UPGRADE_UPGRADE_END);        
                        dlog_critical("upgrade ok");
                        st.par.upgradeDataPack.u8_opt = UPGRADE_ACK_END;

                    }
                    while (0 != UPGRADE_SendMsg(&st, NULL, 0))
                    {
                        HAL_Delay(UPGRADE_DELAY);
                    }
                }
            }
                break;
            default :
                dlog_error("UPGRADE_DATA unkown");
                break;
        }
    }
    else
    {
        dlog_error("command error");
    }
}

static void UPGRADE_rcvHandleGnd(void *p)
{
    uint32_t i = 0;
    uint32_t u32_rcvLen = 0;
    uint8_t rec_data[1024];
    
    STRU_skyStatusMsg st_session;
    HAL_BB_UartComReceiveMsg(UPGRADE_SESSION, (uint8_t *)&rec_data, 1024, &u32_rcvLen);
    memcpy(&st_session, rec_data, sizeof(STRU_skyStatusMsg));
    switch (st_session.par.upgradeDataPack.u8_opt)
    {
        case UPGRADE_ACK_INIT: 
        {
            memset(s_u16_lostDataBlockArray, 0xff, sizeof(s_u16_lostDataBlockArray));
            s_u8_lostDataBlockCount = 0;
            
            UpgradeGndState_Set(UPGRADE_ACK_INIT); 
            dlog_critical("rec sky   UPGRADE_ACK_INIT");
            break;
        }
        case UPGRADE_ACK_LOSTDATA: 
        {
            //storage lose data block number
            uint16_t loseblockCount = st_session.par.upgradeDataPack.u16_data[0];
            memcpy(s_u16_lostDataBlockArray, &rec_data[sizeof(STRU_skyStatusMsg)], loseblockCount*sizeof(uint16_t));
            dlog_critical("rec sky   UPGRADE_ACK_LOSTDATA %d", loseblockCount);
            s_u8_lostDataBlockCount = loseblockCount;
            for (i = 0; i < loseblockCount; i++)
            {
                dlog_critical("lose data block num %d", s_u16_lostDataBlockArray[i]);
            }
            
            UpgradeGndState_Set(UPGRADE_ACK_LOSTDATA); 
            dlog_critical("enable re-send data", s_u8_upgradeStatus);

            break;
        }
        case UPGRADE_ACK_END: 
        case UPGRADE_ACK_FAIL: 
            UpgradeGndState_Set(st_session.par.upgradeDataPack.u8_opt);
            break;            
        default :
            dlog_error("rec sky ack error");
            break;
    }


}


int8_t UPGRADE_GndSendInit(void)
{
    STRU_skyStatusMsg st_upgrade;
    uint32_t u32_dataChunk = 0;
    uint32_t u32_upgradeChunk = 0;

    st_upgrade.pid = SKY_UPGRADE;
    st_upgrade.par.upgradeDataPack.u8_opt = UPGRADE_DATA_INIT;
    u32_dataChunk = s_u32_upgradeTotalDataLen%UPGRADE_DATACHUNK_SIZE;
    u32_upgradeChunk = s_u32_upgradeTotalDataLen/UPGRADE_DATACHUNK_SIZE;
    if (0 != u32_dataChunk)
    {
        st_upgrade.par.upgradeDataPack.u16_data[0] = u32_upgradeChunk + 1;
    }
    else
    {
        st_upgrade.par.upgradeDataPack.u16_data[0] = u32_upgradeChunk;
    }

    st_upgrade.par.upgradeDataPack.u16_data[1] = (APPLICATION_IMAGE_START & 0xffff0000) >> 16;
    st_upgrade.par.upgradeDataPack.u16_data[2] = (APPLICATION_IMAGE_START & 0xffff);
    UPGRADE_SendMsg(&st_upgrade, NULL, 0);
    return UpgradeGndStatus_Get(UPGRADE_ACK_INIT);
}

void UPGRADE_Start(void)
{
    UpgradeState_Reset();
    UPGRADE_GNDInit();

    if (0x01 != UPGRADE_GndAndSkyLockStatus())
    {
        dlog_error("unlock");
        return;
    }

    HAL_BB_UpgradeMode(1);
    HAL_Delay(500);
}

void UPGRADE_UseUsbUpgrade(const uint8_t *fileName)
{
    uint32_t i = 0;

    i = HAL_USB_GetMSCPort();

    if (i < HAL_USB_PORT_NUM)
    {
        dlog_info("usb port %d is valid", i);
    }
    else
    {
        dlog_error("usb port is not valid");
        return;
    }

    if (-1 == UPGRADE_CheckUpgradeFile(fileName))
    {
        return;
    } 
}


uint32_t UPGRADE_GndSendEnd(void)
{
    STRU_skyStatusMsg st_upgrade;
    uint32_t u32_dataChunk = 0;
    uint32_t u32_upgradeChunk = 0;
    
    st_upgrade.pid = SKY_UPGRADE;
    st_upgrade.par.upgradeDataPack.u8_opt = UPGRADE_DATA_END;
    return UPGRADE_SendMsg(&st_upgrade, NULL, 0);
}


void UPGRADE_SKYInit(void)
{
    HAL_BB_UartComRemoteSessionInit();      

    HAL_BB_UartComRegisterSession(UPGRADE_SESSION,
                                  BB_UART_SESSION_PRIORITY_HIGH,
                                  BB_UART_SESSION_DATA_NORMAL,
                                  UPGRADE_rcvHandleSky);
}

void UPGRADE_GNDInit(void)
{
    HAL_BB_UartComRemoteSessionInit();      
    HAL_BB_UartComRegisterSession(UPGRADE_SESSION,
                                  BB_UART_SESSION_PRIORITY_HIGH,
                                  BB_UART_SESSION_DATA_NORMAL,
                                  UPGRADE_rcvHandleGnd);
}


