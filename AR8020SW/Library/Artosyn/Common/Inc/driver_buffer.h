#ifndef _DRIVER_BUFFER_H
#define _DRIVER_BUFFER_H

#include <stdint.h>

int8_t get_new_buffer(uint8_t **drv_buf, uint8_t *usr_buf, uint32_t *txLenLast, uint32_t u32_size);

#endif
