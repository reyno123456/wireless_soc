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

#ifndef __HAL_PMU_H__
#define __HAL_PMU_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
* @brief  The PMU init function.
* @param  none
* @retval HAL_OK            means the PMU init is well done.
* @note   None.
*/

HAL_RET_T HAL_PMU_Init(void);


#ifdef __cplusplus
}
#endif

#endif