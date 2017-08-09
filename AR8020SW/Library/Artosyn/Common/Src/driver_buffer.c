#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "driver_buffer.h"
#include "debuglog.h"

int8_t COMMON_getNewBuffer(uint8_t **ppu8_drvBuf, uint8_t *pu8_usrBuf, uint32_t *pu32_txLenLast, uint32_t u32_size)
{
    if (NULL == *ppu8_drvBuf)
    {
        *ppu8_drvBuf = malloc(u32_size);
        if (NULL == *ppu8_drvBuf)
        {
            *pu32_txLenLast = 0;
            dlog_error("malloc error");
            return -1;
        }
        else
        {
            *pu32_txLenLast = u32_size;
        }
    }
    else
    {
        if (0 != *pu32_txLenLast)
        {
            //dlog_info("line = %d, u32_size = %d, *txLenLast = %d", __LINE__, u32_size, *txLenLast);
            if ((u32_size > *pu32_txLenLast) || (u32_size < *pu32_txLenLast/10))
            {
                *ppu8_drvBuf = realloc(*ppu8_drvBuf, u32_size);
                if (NULL == *ppu8_drvBuf)
                {
                    *pu32_txLenLast = 0;
                    dlog_error("realloc error, size = %d", u32_size);
                    return -1;
                }
                else
                {
                    *pu32_txLenLast = u32_size;
                    //dlog_info("line = %d", __LINE__);
                }
            }
            else
            {
                //dlog_warning("keep current length");
            }
        }        
    }

    memcpy(*ppu8_drvBuf, pu8_usrBuf, u32_size);

    return 0;
}
