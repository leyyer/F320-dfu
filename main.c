#include <stdio.h>
#include "sysconf.h"

void main(void)
{
	system_init();
	usb_init();
	printf("device starting\n");
	EA = 1;
	while (1) {
		putchar(getchar());
		putchar('\r');
		putchar('\n');
	}
}

