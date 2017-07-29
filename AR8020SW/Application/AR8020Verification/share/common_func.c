#include "common_func.h"
#include "debuglog.h"
#include "interrupt.h"
#include "cpu_info.h"
#include "cmsis_os.h"
#include "hal.h"
#include "systicks.h"
#include <string.h>
#include <stdlib.h>
#include "ff.h"
#include "hal_sd.h"

FUNC_LogSave dlog_output_SD(char* buf, unsigned int bytes)
{
    FIL g_log_file;
    FRESULT res;
    uint32_t byteswritten, bytesread;

    if (HAL_OK == HAL_SD_GetPresent())
    {                        
        f_chdrive("SD:");
        f_open(&g_log_file, "ar_8020.txt", 
                        FA_OPEN_ALWAYS | FA_WRITE | FA_OPEN_APPEND);
        f_write(&g_log_file, buf, bytes, (void *)&byteswritten);
        f_close(&g_log_file);
    }
    return HAL_OK;
}

