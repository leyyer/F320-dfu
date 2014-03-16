#ifndef __SYS_CONFIG_H__
#define __SYS_CONFIG_H__

#include <C8051F320.h>

#define SYSCLK    12000000
#define BAUDRATE  115200
#define SYS_OSC   0x00
#define USB_CLOCK 0x00
#define FLASH_BLOCK_SIZE 512
#define APP_START 0x1400

void system_init(void);
void delay(void);
void watchdog_reset(void);
#endif

