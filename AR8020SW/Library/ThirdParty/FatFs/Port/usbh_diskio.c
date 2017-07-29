/**
  ******************************************************************************
  * @file    usbh_diskio.c 
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    08-May-2015
  * @brief   USB Key Disk I/O driver.
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
#include "ff_gen_drv.h"
#include "ffconf.h"
#include "diskio.h"
#include "usbh_msc.h"
#include "debuglog.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern USBH_HandleTypeDef  hUSBHost[USBH_PORT_NUM];
#if _USE_BUFF_WO_ALIGNMENT == 0
/* Local buffer use to handle buffer not aligned 32bits*/
static DWORD scratch[FF_MAX_SS / 4];
#endif

/* Private function prototypes -----------------------------------------------*/
DSTATUS USBH_initialize (BYTE);
DSTATUS USBH_status (BYTE);
DRESULT USBH_read (BYTE, BYTE*, DWORD, UINT);

#if _USE_WRITE == 1
  DRESULT USBH_write (BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */

#if _USE_IOCTL == 1
  DRESULT USBH_ioctl (BYTE, BYTE, void*);
#endif /* _USE_IOCTL == 1 */
  
const Diskio_drvTypeDef  USBH_Driver =
{
  USBH_initialize,
  USBH_status,
  USBH_read, 
#if  _USE_WRITE == 1
  USBH_write,
#endif /* _USE_WRITE == 1 */  
#if  _USE_IOCTL == 1
  USBH_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  lun : lun id
  * @retval DSTATUS: Operation status
  */
DSTATUS USBH_initialize(BYTE lun)
{
  return RES_OK;
}

/**
  * @brief  Gets Disk Status
  * @param  lun : lun id
  * @retval DSTATUS: Operation status
  */
DSTATUS USBH_status(BYTE lun)
{
  DRESULT res = RES_ERROR;

  if(USBH_MSC_UnitIsReady(&hUSBHost[g_mscPortId&0x01], lun))
  {
    res = RES_OK;
  }
  else
  {
    dlog_error("USBH_MSC_UnitIsReady error: %d", g_mscPortId);
    res = RES_ERROR;
  }
  
  return res;
}

/**
  * @brief  Reads Sector(s) 
  * @param  lun : lun id
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USBH_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
  DRESULT res = RES_ERROR;
  MSC_LUNTypeDef info;
  USBH_StatusTypeDef  status = USBH_OK;

  if ((DWORD)buff & 3) /* DMA Alignment issue, do single up to aligned buffer */
  {
#if _USE_BUFF_WO_ALIGNMENT == 0
    while ((count--)&&(status == USBH_OK))
    {
      status = USBH_MSC_Read(&hUSBHost[g_mscPortId&0x01], lun, sector + count, (uint8_t *)scratch, 1);
      if(status == USBH_OK)
      {
        memcpy (&buff[count * FF_MAX_SS] ,scratch, FF_MAX_SS);
      }
      else
      {
        dlog_error("USBH_MSC_Read error: %d", g_mscPortId);
        break;
      }
    }
#else
    return res;
#endif
  }
  else
  {
    status = USBH_MSC_Read(&hUSBHost[g_mscPortId&0x01], lun, sector, buff, count);
  }
  
  if(status == USBH_OK)
  {
    res = RES_OK;
  }
  else
  {
    USBH_MSC_GetLUNInfo(&hUSBHost[g_mscPortId&0x01], lun, &info); 

    dlog_error("USBH_MSC_Read error: %d", g_mscPortId);
    
    switch (info.sense.asc)
    {
    case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
    case SCSI_ASC_MEDIUM_NOT_PRESENT:
    case SCSI_ASC_NOT_READY_TO_READY_CHANGE: 
      dlog_info("USB Disk is not ready!\n");
      res = RES_NOTRDY;
      break; 
      
    default:
      res = RES_ERROR;
      break;
    }
  }
  
  return res;
}

/**
  * @brief  Writes Sector(s)
  * @param  lun : lun id 
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USBH_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
  DRESULT res = RES_ERROR; 
  MSC_LUNTypeDef info;
  USBH_StatusTypeDef  status = USBH_OK;  

  if ((DWORD)buff & 3) /* DMA Alignment issue, do single up to aligned buffer */
  {
#if _USE_BUFF_WO_ALIGNMENT == 0
    while (count--)
    {
      memcpy (scratch, &buff[count * FF_MAX_SS], FF_MAX_SS);
      
      status = USBH_MSC_Write(&hUSBHost[g_mscPortId&0x01], lun, sector + count, (BYTE *)scratch, 1) ;
      if(status == USBH_FAIL)
      {
        dlog_error("USBH_MSC_Write error: %d", g_mscPortId);

        break;
      }
    }
#else
    return res;
#endif
  }
  else
  {
    status = USBH_MSC_Write(&hUSBHost[g_mscPortId&0x01], lun, sector, (BYTE *)buff, count);
  }
  
  if(status == USBH_OK)
  {
    res = RES_OK;
  }
  else
  {
    USBH_MSC_GetLUNInfo(&hUSBHost[g_mscPortId&0x01], lun, &info); 

    dlog_error("USBH_MSC_Write error: %d", g_mscPortId);

    switch (info.sense.asc)
    {
    case SCSI_ASC_WRITE_PROTECTED:
      dlog_info("USB Disk is Write protected!\n");
      res = RES_WRPRT;
      break;
      
    case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
    case SCSI_ASC_MEDIUM_NOT_PRESENT:
    case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
      dlog_info("USB Disk is not ready!");
      res = RES_NOTRDY;
      break; 
      
    default:
      res = RES_ERROR;
      break;
    }
  }
  
  return res;   
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  lun : lun id
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USBH_ioctl(BYTE lun, BYTE cmd, void *buff)
{
  DRESULT res = RES_ERROR;
  MSC_LUNTypeDef info;
  
  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC: 
    res = RES_OK;
    break;
    
  /* Get number of sectors on the disk (DWORD) */  
  case GET_SECTOR_COUNT : 
    if(USBH_MSC_GetLUNInfo(&hUSBHost[g_mscPortId&0x01], lun, &info) == USBH_OK)
    {
      *(DWORD*)buff = info.capacity.block_nbr;
      res = RES_OK;
    }
    else
    {
      dlog_error("USBH_MSC_GetLUNInfo GET_SECTOR_COUNT error: %d", g_mscPortId);
      res = RES_ERROR;
    }
    break;
    
  /* Get R/W sector size (WORD) */  
  case GET_SECTOR_SIZE :	
    if(USBH_MSC_GetLUNInfo(&hUSBHost[g_mscPortId&0x01], lun, &info) == USBH_OK)
    {
      *(DWORD*)buff = info.capacity.block_size;
      res = RES_OK;
    }
    else
    {
      dlog_error("USBH_MSC_GetLUNInfo GET_SECTOR_SIZE error: %d", g_mscPortId);
      res = RES_ERROR;
    }
    break;
    
    /* Get erase block size in unit of sector (DWORD) */ 
  case GET_BLOCK_SIZE : 
    
    if(USBH_MSC_GetLUNInfo(&hUSBHost[g_mscPortId&0x01], lun, &info) == USBH_OK)
    {
      *(DWORD*)buff = info.capacity.block_size;
      res = RES_OK;
    }
    else
    {
      dlog_error("USBH_MSC_GetLUNInfo GET_BLOCK_SIZE error: %d", g_mscPortId);
      res = RES_ERROR;
    }
    break;
    
  default:
    res = RES_PARERR;
  }
  
  return res;
}
#endif /* _USE_IOCTL == 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

