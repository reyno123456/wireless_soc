#include <string.h>
#include "bb_ctrl_internal.h"

/* 
 *default_sys_param
*/
const PARAM default_sys_param =
{
    .it_skip_freq_mode = AUTO,
    .rc_skip_freq_mode = AUTO,
    .qam_skip_mode     = AUTO,
    .qam_change_threshold = {{0, 0x6A}, {0x54, 0x129}, {0xec, 0x388}, {0x2ce, 0xa3e}, {0x823, 0x1d94}, {0x12aa, 0xffff}},
};


PARAM * BB_get_sys_param(void)
{
    return (PARAM *)&default_sys_param;
}
