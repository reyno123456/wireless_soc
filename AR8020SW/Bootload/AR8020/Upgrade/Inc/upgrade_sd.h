#ifndef UPGREADE_SD_H
#define UPGREADE_SD_H


#include "ff_gen_drv.h"

extern Diskio_drvTypeDef  SD_Driver;

void UPGRADE_UpdataFromSDToNor(void);
//void UPGRADE_BootFromSD(void);
void UPGRADE_UpdataBootloaderFromSD(void);

#endif