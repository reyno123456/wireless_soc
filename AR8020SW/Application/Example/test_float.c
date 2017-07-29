#include <stdint.h>
#include "test_float.h"
#include "debuglog.h"

void test_float_calculate_pi(void)
{
    float pi = 2.0;

    int i = 100;

    for(i = 100; i >= 1; i--)
    {
        pi = (pi * i / (2*i + 1)) + 2;
    }

    dlog_info("%d", (int)(pi*100000));
}


