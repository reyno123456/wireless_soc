#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__


#include <stdio.h>
#include <malloc.h>


/* Circular buffer object */
typedefstruct
{  
    uint32_t         u32_size;   /* maximum number of elements           */
    uint32_t         u32_start;  /* index of oldest element              */
    uint32_t         u32_end;    /* index at which to write new element  */
    uint32_t         u32_width;  /* element size  */
    uint8_t          *pu8_elems;  /* vector of elements                   */
} STRU_CIR_BUF;


#endif
