#ifndef _DRIVER_MUTEX_H
#define _DRIVER_MUTEX_H

typedef struct
{
    uint32_t timer0to7;
    uint32_t timer8to15;
    uint32_t timer16to23;
}TIMER_MUTEX_DATA;

typedef struct
{
    uint32_t uart;
    uint32_t spi;
    uint32_t can;
    uint32_t i2c;
    uint32_t pwm;
    TIMER_MUTEX_DATA s_timer;
    uint32_t nor_flash;
}DRIVER_MUTEX_DATA;

typedef enum _emu_driver_mutex
{
    mutex_uart = 0,
    mutex_spi,
    mutex_can,
    mutex_i2c,
    mutex_pwm,
    mutex_timer,
    mutex_nor_flash
}emu_driver_mutex;

extern DRIVER_MUTEX_DATA *g_s_periMutex;

void driver_mutex_free(emu_driver_mutex driver, uint32_t channel);
void driver_mutex_set(emu_driver_mutex driver, uint32_t channel);
int8_t driver_mutex_get(emu_driver_mutex driver, uint32_t channel);

#endif
