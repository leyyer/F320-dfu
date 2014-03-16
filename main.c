#include <stdio.h>
#include "usbdev.h"
#include "sysconf.h"

void isr_0() __interrupt(0) __naked
{
	__asm
		ljmp (APP_START + 0x3)
	__endasm;
}

void isr_1() __interrupt(1) __naked
{
	__asm
		ljmp (APP_START + 0xB)
	__endasm;
}

void isr_2() __interrupt(2) __naked
{
	__asm
		ljmp (APP_START + 0x13)
	__endasm;
}

void isr_3() __interrupt(3) __naked
{
	__asm
		ljmp (APP_START + 0x1B)
	__endasm;
}

void isr_4() __interrupt(4) __naked
{
	__asm
		ljmp (APP_START + 0x23)
	__endasm;
}

void isr_5() __interrupt(5) __naked
{
	__asm
		ljmp (APP_START + 0x2B)
	__endasm;
}

void isr_6() __interrupt(6) __naked
{
	__asm
		ljmp (APP_START + 0x33)
	__endasm;
}

void isr_7() __interrupt(7) __naked
{
	__asm
		ljmp (APP_START + 0x3B)
	__endasm;
}

void isr_8() __interrupt(8) __naked
{
	__asm
		ljmp (APP_START + 0x43)
	__endasm;
}

void isr_9() __interrupt(9) __naked
{
	__asm
		ljmp (APP_START + 0x4B)
	__endasm;
}

void isr_10() __interrupt(10) __naked
{
	__asm
		ljmp (APP_START + 0x53)
	__endasm;
}

void isr_11() __interrupt(11) __naked
{
	__asm
		ljmp (APP_START + 0x5B)
	__endasm;
}

void isr_12() __interrupt(12) __naked
{
	__asm
		ljmp (APP_START + 0x63)
	__endasm;
}

void isr_13() __interrupt(13) __naked
{
	__asm
		ljmp (APP_START + 0x6B)
	__endasm;
}

void isr_14() __interrupt(14) __naked
{
	__asm
		ljmp (APP_START + 0x73)
	__endasm;
}

void isr_15() __interrupt(15) __naked
{
	__asm
		ljmp (APP_START + 0x7B)
	__endasm;
}

void main(void)
{
	system_init();
	usb_init();
	EA = 0;
	printf("dfu mode.\n");
	while (1) {
		usb_poll();
	}
}

