#ifndef __UPGREAD_SKY__
#define __UPGREAD_SKY__

#ifdef __cplusplus
extern "C"
{
#endif


#define UPGRADE_SDRAM_DATA_ADDR              (0x81f00000)
#define UPGRADE_MAX_BLOCKARRAYSIZE           (100)
#define APPLICATION_IMAGE_START              (0x10020000)
#define APP_ADDR_OFFSET                      (0x20000)
#define UPGRADE_ERASE_SIZE                   (0x1000)
#define UPGRADE_DELAY                        (50)
#define IMAGE_HEAD_SIZE                      (34)
#define IMAGE_MD5_OFFSET                     (18)
#define UPGRADE_BLOCK_SIZE                   (0x10000)
#define UPGRADE_DATACHUNK_SIZE               (512)

typedef enum
{
    UPGRADE_ACK_INIT = 0, //sky  init ok
    UPGRADE_ACK_END,      // sky upgrade ok
    UPGRADE_ACK_LOSTDATA, // sky send lose data block num
    UPGRADE_ACK_FAIL, // sky upgrade fail
} ENUM_UPGRADE_GROUND_STATE;

typedef enum
{
    UPGRADE_DATA_INIT = 0, //ground notify sky upgarde 
    UPGRADE_DATA_DATA,     //send data
    UPGRADE_LOSTBLOCK_DATA,// send lose data
    UPGRADE_DATA_END, //data end
    UPGRADE_UPGRADE_END,// upgrade ok
} ENUM_UPGRADE_SKY_STATE;

#define UPGRADE_SESSION            BB_UART_COM_SESSION_4

typedef enum
{
    UPGRADE_OK = 0, 
    UPGRADE_FAIL,
    UPGRADE_UPGRADING,
} ENUM_UPGRADE_STATUS;

typedef struct
{
    uint32_t u32_fileSize;
    uint32_t u32_sentFileSize;
    ENUM_UPGRADE_STATUS  e_upgradeSkyStatus;
} STRU_UPGRADE_STATUS;

extern uint16_t s_u16_dataBlockNum;
//lose data block number
extern uint16_t s_u16_lostDataBlockArray[UPGRADE_MAX_BLOCKARRAYSIZE];
//lose data block number count
extern uint8_t s_u8_lostDataBlockCount;
//receive data length
extern uint32_t s_u32_upgradeRecDataLen;
//file size
extern uint32_t s_u32_upgradeTotalDataLen;
//sky :total data block count
//ground :present send data block number
extern uint16_t s_u16_upgradeReturnStatus;


void UPGRADE_SKYInit(void);

/**
* @brief  ground send data status.
* @param  STRU_UPGRADE_STATUS.ENUM_UPGRADE_STATUS UPGRADE_OK upgrade ok.
                                                  UPGRADE_FAIL  upgrade fail.
                                                  UPGRADE_UPGRADING  upgrading.
          STRU_UPGRADE_STATUS.u32_fileSize upgrade file size
          STRU_UPGRADE_STATUS.u32_sentFileSize sent upgrade file size
* @retval none.
* @note   none.
*/
void UPGRADE_GetStatus(STRU_UPGRADE_STATUS * pst_status);
/**
* @brief  ground send init for sky.
* @param  none.
* @retval 1        sky receive init command.
* @retval 0        error.
* @note   None.
*/
int8_t UPGRADE_GndSendInit(void);
/**
* @brief  ground send end for sky.
* @param  none.
* @retval 1        sky receive init command.
* @retval 0        error.
* @note   none.
*/
uint32_t UPGRADE_GndSendEnd(void);
int8_t UPGRADE_GndAndSkyLockStatus(void);

int8_t UpgradeGndState_Clear(ENUM_UPGRADE_GROUND_STATE e_state);
int8_t UpgradeGndStatus_Get(ENUM_UPGRADE_GROUND_STATE e_state);

/**
* @brief  ground send data to sky.
* @param  u8_opt UPGRADE_DATA_DATA send data.
                 UPGRADE_LOSTBLOCK_DATA send lose data.
          pu8_dataBuff data buff.
          u16_dataLen data length.
          u16_dataBlockNum data block Number
* @retval 0     send ok.
          !=0   send fail.
* @note   ground use UPGRADE_DATA_DATA send data.if sky lose data , sky notify ground lose data block number in s_u16_lostDataBlockArray.
*/
uint32_t UPGRADE_SendDataBlock(uint8_t u8_opt, uint8_t *pu8_dataBuff, uint16_t u16_dataLen, uint16_t u16_dataBlockNum);

/**
* @brief  check mass storage and file.
* @param  none.
* @retval none.
* @note   none.
*/
void UPGRADE_UseUsbUpgrade(const uint8_t *fileName);
/**
* @brief  upgrade start.
* @param  none.
* @retval none.
* @note   none.
*/
void UPGRADE_Start(void);

#ifdef __cplusplus
}
#endif

#endif