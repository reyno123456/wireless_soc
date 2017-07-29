#include "debuglog.h"
#include "local_irq.h"
#include "test_localirq.h"

void command_TestLocalIrq(void)
{
    unsigned long flag1 = local_irq_save_flags();
    dlog_info("local_irq_save_flags: flag1 = %d", flag1);

    local_irq_disable();
    dlog_info("local_irq_disable");

    unsigned long flag2 = local_irq_save_flags();
    dlog_info("local_irq_save_flags: flag2 = %d", flag2);

    local_irq_enable();
    dlog_info("local_irq_enable");

    unsigned long flag3 = local_irq_save_flags();
    dlog_info("local_irq_save_flags: flag3 = %d", flag3);

    unsigned long flag4 = local_irq_disable_save_flags();
    dlog_info("flag4 = %d", flag4);
    dlog_info("local_irq_disable_save_flags");

    unsigned long flag5 = local_irq_save_flags();
    dlog_info("local_irq_save_flags: flag5 = %d", flag5);

    local_irq_restore(flag4);
    dlog_info("local_irq_restore: flag4 %d", flag4);

    unsigned long flag6 = local_irq_save_flags();
    dlog_info("local_irq_save_flags: flag6 = %d", flag6);        
}


