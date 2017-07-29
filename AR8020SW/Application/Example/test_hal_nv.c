#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debuglog.h"
#include "hal_nv.h"
#include "test_hal_nv.h"
#include "hal_bb.h"



void command_TestNvSkyAutoSearhRcId(void)
{
    HAL_BB_SetAutoSearchRcIdProxy();
}

void command_TestNvResetBbRcId(void)
{
    HAL_NV_ResetBbRcId();
}

void command_TestNvSetBbRcId(uint8_t *id1, uint8_t *id2, uint8_t *id3, uint8_t *id4, uint8_t *id5)
{
    uint8_t idArr[5];

    idArr[0] = (uint8_t)(strtoul(id1, NULL, 0));
    idArr[1] = (uint8_t)(strtoul(id2, NULL, 0));
    idArr[2] = (uint8_t)(strtoul(id3, NULL, 0));
    idArr[3] = (uint8_t)(strtoul(id4, NULL, 0));
    idArr[4] = (uint8_t)(strtoul(id5, NULL, 0));

    HAL_NV_SetBbRcId(idArr);

    dlog_info("id:0x%x 0x%x 0x%x 0x%x 0x%x",idArr[0], idArr[1], idArr[2], idArr[3], idArr[4]);
}
