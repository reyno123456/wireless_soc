#include <stddef.h>
#include <stdint.h>
#include "debuglog.h"
#include "enc_internal.h"

void encoder_delay(unsigned int delay)
{
    volatile int i = delay;
    while(i > 0)
    {
        i--;
    }
}

void sdram_init_check(void)
{
    unsigned char sdram_init_done = 0 ;

    do
    {
        encoder_delay(500) ;
        READ_WORD(VSOC_GLOBAL2_BASE + SDRAM_INIT_DONE,sdram_init_done) ;
        sdram_init_done = sdram_init_done & 0x01 ;
    } while(sdram_init_done == 0);
}

