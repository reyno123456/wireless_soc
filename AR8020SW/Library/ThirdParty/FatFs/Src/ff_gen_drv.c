/**
  ******************************************************************************
  * @file    ff_gen_drv.c
  * @author  Artosyn Software Team
  * @version follow open source version fatfs 0.13
  * @date    15-Jun-2017
  * @brief   FatFs generic low level driver.
  ******************************************************************************
  */

#include "ff_gen_drv.h"
#include "debuglog.h"
#include <string.h>
#include <stdlib.h>

Disk_drvTypeDef disk = {{0}, {0}, {0}, 0};

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**TODO
  * @brief  Links a compatible diskio driver/lun id and increments the number of active
  *         linked drivers.
  * @note   The number of linked drivers (volumes) is up to 10 due to FatFs limits.
  * @param  drv: pointer to the disk IO Driver structure
  * @param  path: pointer to the logical drive path
  * @param  lun : only used for USB Key Disk to add multi-lun management
            else the paramter must be equal to 0
  * @retval Returns 0 in case of success, otherwise 1.
  */
uint8_t FATFS_LinkDriverEx(Diskio_drvTypeDef *drv, char *path, uint8_t lun)
{
    int vol;
    char tmp_path[FF_MAX_LFN];              // force to use 4 Byte alined
    char *p_tmp_path = tmp_path;

#if 1
    memcpy(tmp_path, path, strlen(path));
#endif

/*     strcpy(tmp_path,path); */
// test
#if 0
    strcpy(tmp_path,"test");
    strcpy(tmp_path,(const char *)(path));
    strcpy(tmp_path,path);
#endif
// test end
    tmp_path[strlen(path)] = 0;
    dlog_info("path = %s", path);
    vol = get_ldnumber((TCHAR const**)(&p_tmp_path));
    if (vol < 0)
    {
        return FR_INVALID_DRIVE;
    }

    if (FR_OK != f_chdrive(path))
    {
        dlog_info("%d chdrive to %s// error", __LINE__, path);
    }
    
    disk.nbr = vol;

    dlog_info("disk.nbr = %d", disk.nbr);
    disk.is_initialized[disk.nbr] = 0;
    disk.drv[disk.nbr] = drv;
    disk.lun[disk.nbr] = lun;

    return 0;
}

/**
  * @brief  Links a compatible diskio driver and increments the number of active
  *         linked drivers.
  * @note   The number of linked drivers (volumes) is up to 10 due to FatFs limits
  * @param  drv: pointer to the disk IO Driver structure
  * @param  path: pointer to the logical drive path
  * @retval Returns 0 in case of success, otherwise 1.
  */
uint8_t FATFS_LinkDriver(Diskio_drvTypeDef *drv, char *path)
{
  return FATFS_LinkDriverEx(drv, path, 0);
}

/**
  * @brief  Unlinks a diskio driver and decrements the number of active linked
  *         drivers.
  * @param  path: pointer to the logical drive path
  * @param  lun : not used
  * @retval Returns 0 in case of success, otherwise 1.
  */
uint8_t FATFS_UnLinkDriverEx(char *path, uint8_t lun)
{
  uint8_t DiskNum = 0;
  uint8_t ret = 1;

#if 0
  if (disk.nbr >= 1)
  {
    DiskNum = path[0] - '0';
    if (disk.drv[DiskNum] != 0)
    {
      disk.drv[DiskNum] = 0;
      disk.lun[DiskNum] = 0;
      disk.nbr--;
      ret = 0;
    }
  }
  return ret;
#endif

    if (disk.nbr >= 1)
    {
        DiskNum = disk.nbr - 1;
        if (disk.drv[DiskNum] != 0)
        {
            disk.drv[DiskNum] = 0;
            disk.lun[DiskNum] = 0;
            disk.nbr--;
            ret = 0;
        }
    }
    return ret;
}

/**
  * @brief  Unlinks a diskio driver and decrements the number of active linked
  *         drivers.
  * @param  path: pointer to the logical drive path
  * @retval Returns 0 in case of success, otherwise 1.
  */
uint8_t FATFS_UnLinkDriver(char *path)
{
  return FATFS_UnLinkDriverEx(path, 0);
}

/**
  * @brief  Gets number of linked drivers to the FatFs module.
  * @param  None
  * @retval Number of attached drivers.
  */
uint8_t FATFS_GetAttachedDriversNbr(void)
{
  return disk.nbr;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

