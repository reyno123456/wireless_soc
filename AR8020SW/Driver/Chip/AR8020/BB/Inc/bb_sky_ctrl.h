#ifndef __SKY_CONTROLLER_H
#define __SKY_CONTROLLER_H

#include <stdint.h>
#include "bb_ctrl_internal.h"

enum EN_AGC_MODE
{
    FAR_AGC     = 0,
    NEAR_AGC    = 1,
    UNKOWN_AGC  = 0xff,
};


void BB_SKY_start(void);

void sky_SetSaveRCId(uint8_t *pu8_id);

#endif
