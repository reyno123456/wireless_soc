#ifndef __LOCAL_IRQ___
#define __LOCAL_IRQ___

static inline unsigned long local_irq_disable_save_flags(void)
{
    unsigned long flags;

    __asm volatile("mrs     %0, primask        @ local_irq_disable_save_flags\n"
                   "cpsid   i"
                   : "=r" (flags) : : "memory", "cc");
    
   return flags;
}

static inline void local_irq_enable(void)
{
    __asm volatile(
                   "cpsie i                 @ local_irq_enable"
                   :
                   :
                   : "memory", "cc");
}

static inline void local_irq_disable(void)
{
    __asm volatile(
                   "cpsid i                 @ local_irq_disable"
                   :
                   :
                   : "memory", "cc");
}

#define local_fiq_enable()  ____asm__("cpsie f    @ __stf" : : : "memory", "cc")
#define local_fiq_disable() ____asm__("cpsid f    @ __clf" : : : "memory", "cc")

/*
 * Save the current interrupt enable state.
 */
static inline unsigned long local_irq_save_flags(void)
{
    unsigned long flags;
    __asm volatile(
                   "mrs     %0, primask        @ local_irq_save_flags"
                   : "=r" (flags) : : "memory", "cc");
    return flags;
}

/*
 * restore saved IRQ & FIQ state
 */
static inline void local_irq_restore(unsigned long flags)
{
    __asm volatile(
                   "msr     primask, %0      @ local_irq_restore"
                   :
                   : "r" (flags)
                   : "memory", "cc");
}

#endif

