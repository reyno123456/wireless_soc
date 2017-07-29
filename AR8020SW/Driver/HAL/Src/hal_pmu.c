/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_pmu.c
Description: this module contains the helper fucntions necessary to control the general
             purpose pmu block
Author: Artosy Software Team
Version: 0.0.1
Date: 2017/06/09
History:
         0.0.1    2017/06/09    The initial version of hal_pmu.c
*****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hal_ret_type.h"
#include "hal_pmu.h"

#ifdef ARCAST
#include "pmu_rtp5903.h"
#endif

HAL_RET_T HAL_PMU_Init(void)
{

#ifdef ARCAST
    Pmu_Rtp5903Configure();
#endif

    return HAL_OK;

}
