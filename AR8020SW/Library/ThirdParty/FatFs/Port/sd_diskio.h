/**
  ******************************************************************************
  * @file    sd_diskio.h
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    08-May-2015
  * @brief   Header for sd_diskio.c module
  ******************************************************************************
**/
#ifndef __SD_DISKIO_H
#define __SD_DISKIO_H
#include "ff_gen_drv.h"

extern Diskio_drvTypeDef  SD_Driver;
extern volatile uint8_t sd_mountStatus;

#endif /* __SD_DISKIO_H */
