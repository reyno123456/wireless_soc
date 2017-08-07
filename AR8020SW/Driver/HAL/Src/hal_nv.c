#include <stdint.h>
#include <string.h>
#include "hal_nv.h"
#include "data_type.h"
#include "nor_flash.h"
#include "debuglog.h"
#include "bb_ctrl.h"
#include "memory_config.h"
#include "sys_event.h"
#include "inter_core.h"


#define NV_SRAM_ADDR         (SRAM_NV_MEMORY_ST_ADDR)
#define NV_UPD_MAX_DELAY     (2000)


static int32_t NV_WriteToFlash(uint32_t u32_nvFlashAddr, STRU_NV_DATA *pst_nvDataPrc);

static int32_t NV_WriteFlashProc(void);

static int32_t NV_UpdateBbRcId(uint8_t *u8_bbRcId);

static int32_t NV_Update(void *par);

static void NV_Save(void *par);


static int32_t NV_SetDefaultValue(void);

static int32_t NV_ReadFromFlash(uint32_t u32_nvFlashAddr, STRU_NV_DATA *pst_nvDataPrc);

static int32_t NV_Get(void);

static int32_t NV_SetDefaultValueBbRcId(STRU_NV_DATA *pst_nvDataUpd);

static int32_t NV_CalChk(STRU_NV_DATA *pst_nvData, uint8_t *u8_chk);

static int32_t NV_CheckValidity(STRU_NV_DATA *pst_nvData);

static int32_t NV_GetInit(void);

/**
* @brief      
* @param    
* @retval   
* @note   
*/
static int32_t NV_WriteToFlash(uint32_t u32_nvFlashAddr, STRU_NV_DATA *pst_nvDataPrc)
{
    NV_CalChk(pst_nvDataPrc, &(pst_nvDataPrc->u8_nvChk));

    // write to flash
    NOR_FLASH_WriteByteBuffer(u32_nvFlashAddr, 
                             (uint8_t *)pst_nvDataPrc, 
                             sizeof(STRU_NV_DATA));

    return 0;
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
static int32_t NV_WriteFlashProc(void)
{
    int32_t s32_result = -1;
    STRU_NV *pst_nv = (STRU_NV *)NV_SRAM_ADDR;

    if ( (TRUE == (pst_nv->st_nvMng.u8_nvChg)) && 
         (TRUE != (pst_nv->st_nvMng.u8_nvUpd)) )
    {
        NOR_FLASH_EraseSector(NV_FLASH_ADDR1);

        pst_nv->st_nvMng.u8_nvPrc = TRUE;           
        memcpy((uint8_t *)(&(pst_nv->st_nvDataPrc)), 
               (uint8_t *)(&(pst_nv->st_nvDataUpd)), 
               sizeof(STRU_NV_DATA));
        pst_nv->st_nvMng.u8_nvPrc = FALSE;          
        
        pst_nv->st_nvMng.u8_nvChg = FALSE;
            
        NV_WriteToFlash(NV_FLASH_ADDR1, &(pst_nv->st_nvDataPrc));
        dlog_info("flash 1 write finished.");

        // write backup
        NOR_FLASH_EraseSector(NV_FLASH_ADDR2);
        NV_WriteToFlash(NV_FLASH_ADDR2, &(pst_nv->st_nvDataPrc));
        dlog_info("flash 2 write finished.");
        
        s32_result = 0; 
    }
    else
    {
        ;
    }
    
    return s32_result;
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
static int32_t NV_UpdateBbRcId(uint8_t *u8_bbRcId)
{
    STRU_NV *pst_nv = (STRU_NV *)NV_SRAM_ADDR;

    memcpy(pst_nv->st_nvDataUpd.u8_nvBbRcId, u8_bbRcId, 5);
    NV_CalChk(&(pst_nv->st_nvDataUpd), &(pst_nv->st_nvDataUpd.u8_nvChk));
    pst_nv->st_nvMng.u8_nvChg = TRUE;

    dlog_info("0x%x 0x%x 0x%x 0x%x 0x%x",
               pst_nv->st_nvDataUpd.u8_nvBbRcId[0],
               pst_nv->st_nvDataUpd.u8_nvBbRcId[1],
               pst_nv->st_nvDataUpd.u8_nvBbRcId[2],
               pst_nv->st_nvDataUpd.u8_nvBbRcId[3],
               pst_nv->st_nvDataUpd.u8_nvBbRcId[4], 
               pst_nv->st_nvDataUpd.u8_nvChk);


    return 0;
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
static int32_t NV_Update(void *par)
{
    uint32_t u32_delay = 0;
    uint8_t *u8_pdata = (uint8_t *)par;
    STRU_NV *pst_nv = (STRU_NV *)NV_SRAM_ADDR;
    STRU_SysEvent_NvMsg *pst_nvMsg = (STRU_SysEvent_NvMsg *)par;

    while ((TRUE == (pst_nv->st_nvMng.u8_nvPrc)) && (u32_delay < NV_UPD_MAX_DELAY))
    {
        u32_delay += 1;
    }
    
    if (u32_delay >= NV_UPD_MAX_DELAY)
    {
        return -1;
    }

    if( INTER_CORE_CPU0_ID == (pst_nvMsg->u8_nvDst)) // dst:cpu0
    {
        if(NV_NUM_RCID == (pst_nvMsg->e_nvNum))
        {
                NV_UpdateBbRcId(&(pst_nvMsg->u8_nvPar[1]));
        }
        else
        {
            ;
        }
    }

    return 0;
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
static void NV_Save(void *par)
{
    NV_Update(par);

    NV_WriteFlashProc();
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
static int32_t NV_SetDefaultValue(void)
{
    STRU_NV *pst_nv = (STRU_NV *)NV_SRAM_ADDR;

    pst_nv->st_nvMng.u8_nvUpd = TRUE;

    // set baseband rc default id
    NV_SetDefaultValueBbRcId(&(pst_nv->st_nvDataUpd));
 
    // update checksum.
    NV_CalChk(&(pst_nv->st_nvDataUpd), &(pst_nv->st_nvDataUpd.u8_nvChk));

    // default value is not stored in flash,only update to sram
    //pst_nv->st_nvMng.u8_nvChg = TRUE;

    pst_nv->st_nvMng.u8_nvUpd = FALSE;

    return 0;
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
static int32_t NV_ReadFromFlash(uint32_t u32_nvFlashAddr, STRU_NV_DATA *pst_nvDataPrc)
{
    NOR_FLASH_ReadByteBuffer(u32_nvFlashAddr, 
                            (uint8_t *)pst_nvDataPrc, 
                            sizeof(STRU_NV_DATA));
    
    return 0;
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
static int32_t NV_Get(void)
{
    uint8_t u8_checkSum;
    int32_t result = -1;
    STRU_NV *pst_nv = (STRU_NV *)NV_SRAM_ADDR;

    NV_ReadFromFlash(NV_FLASH_ADDR1, &(pst_nv->st_nvDataPrc));

    if (0 == NV_CheckValidity(&(pst_nv->st_nvDataPrc)))
    {
        memcpy((uint8_t *)(&(pst_nv->st_nvDataUpd)), 
               (uint8_t *)(&(pst_nv->st_nvDataPrc)), 
               sizeof(STRU_NV_DATA));
        dlog_info("flash1 chk ok");
        result = 0;
    }
    else
    {
        dlog_error("flash1 chk error");
    }
    
    if (0 != result)
    {
        NV_ReadFromFlash(NV_FLASH_ADDR2, &(pst_nv->st_nvDataPrc));

        if (0 == NV_CheckValidity(&(pst_nv->st_nvDataPrc)))
        {
            memcpy((uint8_t *)(&(pst_nv->st_nvDataUpd)), 
                   (uint8_t *)(&(pst_nv->st_nvDataPrc)), 
                   sizeof(STRU_NV_DATA));
            dlog_info("flash2 chk ok");
            result = 0;
        }
        else
        {
            dlog_error("flash2 chk error");
        }
    }

    return result;
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
static int32_t NV_SetDefaultValueBbRcId(STRU_NV_DATA *pst_nvDataUpd)
{
    uint8_t u8_id[8];

    memset(u8_id, 0x00, 8);
    NOR_FLASH_ReadProductID(u8_id);
    memcpy(pst_nvDataUpd->u8_nvBbRcId, &u8_id[3], 5);
    pst_nvDataUpd->u8_nvChk = 0; // set to check error

    return 0;
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
static int32_t NV_CalChk(STRU_NV_DATA *pst_nvData, uint8_t *u8_chk)
{
    uint32_t u32_nvDataLen;
    uint32_t u32_i;
    uint8_t u8_checkSum = 0;
    uint8_t *p_u8Addr;

    u32_nvDataLen = sizeof(STRU_NV_DATA);
    p_u8Addr = (uint8_t *)pst_nvData;
    
    //calculate checkSum
    for (u32_i = 1; u32_i < u32_nvDataLen; u32_i++)
    {
        u8_checkSum += *(p_u8Addr + u32_i);
    }

    *u8_chk = u8_checkSum;

    return 0;
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
static int32_t NV_CheckValidity(STRU_NV_DATA *pst_nvData)
{
    uint8_t u8_checkSum;
    uint32_t u32_nvDataLen;
    uint32_t u32_i = 0;
    uint8_t *p_u8Addr;

    u32_nvDataLen = sizeof(STRU_NV_DATA);
    p_u8Addr = (uint8_t *)pst_nvData;
    while (u32_i < u32_nvDataLen)
    {
        if (0xFF != p_u8Addr[u32_i])
        {
            break;
        }
        u32_i += 1;
    }

    NV_CalChk(pst_nvData, &u8_checkSum);

    if (((pst_nvData->u8_nvChk) == u8_checkSum) && (u32_i < u32_nvDataLen))
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
static int32_t NV_GetInit(void)
{
    STRU_NV *pst_nv = (STRU_NV *)NV_SRAM_ADDR;

    if (-1 == NV_Get())
    {
        NV_SetDefaultValue();
        pst_nv->st_nvMng.u8_nvVld = FALSE;
    }    
    else
    {
        pst_nv->st_nvMng.u8_nvVld = TRUE;
    }
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
HAL_RET_T HAL_NV_Init(void)
{
    STRU_NV *pst_nv = (STRU_NV *)NV_SRAM_ADDR;

    NOR_FLASH_Init();
    
    NV_GetInit();

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_NV_MSG, NV_Save);
    
    pst_nv->st_nvMng.u32_nvInitFlag = 0x23178546;
    
    return HAL_OK;
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
HAL_RET_T HAL_NV_ResetBbRcId(void)
{
    uint8_t u8_id[8];

    memset(u8_id, 0x00, 8);
    NOR_FLASH_ReadProductID(u8_id);

    NV_UpdateBbRcId(&u8_id[3]);

    NV_WriteFlashProc();

    return HAL_OK;
}

/**
* @brief      
* @param    
* @retval   
* @note   
*/
HAL_RET_T HAL_NV_SetBbRcId(uint8_t *u8_bbRcId)
{
    uint8_t u8_data[5];
    
    memcpy(u8_data, u8_bbRcId, 5);

    NV_UpdateBbRcId(u8_data);

    NV_WriteFlashProc();

    return HAL_OK;
}

