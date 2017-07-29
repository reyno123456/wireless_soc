/**
  ******************************************************************************
  * @file    sd_diskio.c
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    08-May-2015
  * @brief   SD Disk I/O driver
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"
#include "debuglog.h"
#include "hal_sd.h"
#include "sd_host.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

/**
  * @brief  SD status structure definition
  */
#define MSD_OK                        0x00
#define MSD_ERROR                     0x01
#define MSD_ERROR_SD_NOT_PRESENT      0x02

/* Private function prototypes -----------------------------------------------*/
DSTATUS SD_initialize (BYTE);
DSTATUS SD_status (BYTE);
DRESULT SD_read (BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
DRESULT SD_write (BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
DRESULT SD_ioctl (BYTE, BYTE, void*);
#endif  /* _USE_IOCTL == 1 */

Diskio_drvTypeDef  SD_Driver =
{
  SD_initialize,
  SD_status,
  SD_read,
#if  _USE_WRITE == 1
  SD_write,
#endif /* _USE_WRITE == 1 */

#if  _USE_IOCTL == 1
  SD_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */  
extern SD_HandleTypeDef sdhandle;
DSTATUS SD_initialize(BYTE lun)
{
  Stat = STA_NOINIT;

  /* Configure the uSD device */
#if 0
  if (HAL_SD_Init() == MSD_OK)
  {
    Stat &= ~STA_NOINIT;
  }
#endif

  if(sdhandle.inited == 1)
  { 
    // dlog_info("SD initializa success!");
    return 0;
  }
  else
  {
      dlog_error("SD initializa error!");
      return 1;
  }
}

/**
  * @brief  Gets Disk Status
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS SD_status(BYTE lun)
{
  Stat = STA_NOINIT;
#if 0
  SD_STATUS *e_cardStatus;
  if (SD_CardStatus(e_cardStatus) == SD_TRANSFER_READY)
  {
    Stat &= ~STA_NOINIT;
  }
#endif
    
  if(sdhandle.inited == 1)
  { 
/*     dlog_info("SD initializa success!"); */
    return 0;
  }
  else
  {
      dlog_error("error");
      return 1;
  }
  // return Stat;
}

/**
  * @brief  Reads Sector(s)
  * @param  lun : not used
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
  DRESULT res = RES_OK;

  // dlog_info("buff addr = 0x%x\n", buff);
  // dlog_info("sector = %d\n", sector);
  // dlog_info("count = %d\n", count);
  
  if (HAL_SD_Read((uint32_t)buff,
              (uint32_t)sector,
              count) != MSD_OK)
  {
    res = RES_ERROR;
  }
  // dlog_info("print read");
  // for (int i = 0; i < 8; ++i)
  // {
  //   dlog_info("read byte = 0x%x\n", buff[i]);
  // }
  return res;
}

/**
  * @brief  Writes Sector(s)
  * @param  lun : not used
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
  DRESULT res = RES_OK;

  // for (int j = 0; j < 8; ++j)
  // {
  //   dlog_info("write byte = 0x%x\n", buff[j]);
  // }
  // dlog_info("buff addr = 0x%x\n", buff);
  // dlog_info("sector = %d\n", sector);
  // dlog_info("count = %d\n", count);

  if (HAL_SD_Write((uint64_t)sector,
               (uint32_t)buff,
               count) != MSD_OK)
  {
    res = RES_ERROR;
  }

  return res;
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  lun : not used
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff)
{
  DRESULT res = RES_ERROR;

  if (Stat & STA_NOINIT) return RES_NOTRDY;

  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC :
    res = RES_OK;
    break;

  /* Get number of sectors on the disk (DWORD) */
  case GET_SECTOR_COUNT :
    HAL_SD_Ioctl(HAL_SD_GET_SECTOR_COUNT, (uint32_t *)buff);
    // *(DWORD*)buff = 31275008;
    res = RES_OK;
    break;

  /* Get R/W sector size (WORD) */
  case GET_SECTOR_SIZE :
    HAL_SD_Ioctl(HAL_SD_GET_SECTOR_SIZE, (uint32_t *)buff);
    res = RES_OK;
    break;

  /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE :
    HAL_SD_Ioctl(HAL_SD_GET_SECTOR_SIZE, (uint32_t *)buff);
    break;

  default:
    res = RES_PARERR;
  }

  return res;
}
#endif /* _USE_IOCTL == 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

