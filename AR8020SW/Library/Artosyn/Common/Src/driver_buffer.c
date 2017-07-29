#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "driver_buffer.h"
#include "debuglog.h"

int8_t get_new_buffer(uint8_t **drv_buf, uint8_t *usr_buf, uint32_t *txLenLast, uint32_t u32_size)
{
    if (NULL == *drv_buf)
    {
        *drv_buf = malloc(u32_size);
        if (NULL == *drv_buf)
        {
            *txLenLast = 0;
            dlog_error("malloc error");
            return -1;
        }
        else
        {
            *txLenLast = u32_size;
        }
    }
    else
    {
        if (0 != *txLenLast)
        {
            dlog_info("line = %d, u32_size = %d, *txLenLast = %d", __LINE__, u32_size, *txLenLast);
            if ((u32_size > *txLenLast) || (u32_size < *txLenLast/10))
            {
                *drv_buf = realloc(*drv_buf, u32_size);
                if (NULL == *drv_buf)
                {
                    *txLenLast = 0;
                    dlog_error("realloc error, size = %d", u32_size);
                    return -1;
                }
                else
                {
                    *txLenLast = u32_size;
                    dlog_info("line = %d", __LINE__);
                }
            }
            else
            {
                dlog_warning("keep current length");
            }
        }        
    }

    memcpy(*drv_buf, usr_buf, u32_size);

    return 0;
}
