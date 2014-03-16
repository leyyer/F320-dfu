#include <stdio.h>
#include "sysconf.h"

void main(void)
{
	system_init();
	usb_init();
	EA = 0;
	printf("dfu mode.\n");
	while (1) {
		usb_isr();
	}
}

