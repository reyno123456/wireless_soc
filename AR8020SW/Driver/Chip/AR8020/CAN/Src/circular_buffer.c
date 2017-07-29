#include "circular_buffer.h"

void CIR_BUF_Init(STRU_CIR_BUF *pst_cirBuf, uint32_t u32_size, uint32_t u32_width)
{  
    pst_cirBuf->u32_size  = size +1;/* include empty elem */
    pst_cirBuf->u32_start = 0;
    pst_cirBuf->u32_end   = 0;
    pst_cirBuf->u32_width = u32_width;
    pst_cirBuf->pu8_elems =(uint8_t *)calloc(pst_cirBuf->size,u32_width);
}

void CIR_BUF_Free(CircularBuffer *pst_cirBuf){  
    free(pst_cirBuf->elems);/* OK if null */
}

int CIR_BUF_IsFull(CircularBuffer *pst_cirBuf){  
    return(pst_cirBuf->end +1)% pst_cirBuf->size == pst_cirBuf->start;
}

int CIR_BUF_IsEmpty(CircularBuffer *pst_cirBuf){  
    return pst_cirBuf->end == pst_cirBuf->start;
}

/* Write an element, overwriting oldest element if buffer is full. App can
 *    choose to avoid the overwrite by checking cbIsFull(). */
void CIR_BUF_Write(CircularBuffer *pst_cirBuf, ElemType *elem){  
    pst_cirBuf->elems[pst_cirBuf->end]=*elem;
    pst_cirBuf->end =(pst_cirBuf->end +1)% pst_cirBuf->size;
    if(pst_cirBuf->end == pst_cirBuf->start)
        pst_cirBuf->start =(pst_cirBuf->start +1)% pst_cirBuf->size;/* full, overwrite */
}

/* Read oldest element. App must ensure !pst_cirBufIsEmpty() first. */
void CIR_BUF_Read(CircularBuffer *pst_cirBuf, ElemType *elem){  
    *elem = pst_cirBuf->elems[pst_cirBuf->start];
    pst_cirBuf->start =(pst_cirBuf->start +1)% pst_cirBuf->size;
}
