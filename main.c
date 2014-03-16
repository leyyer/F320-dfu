#include <stdio.h>
#include "sysconf.h"

void main(void)
{
	char c;
	system_init();
	usb_init();
	printf("device starting\n");
	EA = 1;
	while (1) {
		c = getchar();
		putchar( c == '\r' ? '\n' : c);
	}
}

