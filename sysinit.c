#include "sysconf.h"

/* sdcc stdio helper */
static void __putchar(char c)
{
	while (!TI0)
		;
	TI0 = 0;
	SBUF0 = c;
}

void putchar(char c)
{
	__putchar(c);
	if (c == '\n')
		__putchar('\r');
}

char getchar(void)
{
	while (!RI0)
		;
	RI0 = 0;
	return SBUF0;
}

void delay(void)
{
	int x;
	for (x = 0; x < 500; ++x) {
		++x;
	}
}

static void sysclk_init(void)
{
	OSCICN |= 0x03;
	CLKMUL = 0x00;
	CLKMUL |= 0x80;
	CLKMUL |= 0xC0;
	delay();

	while (!(CLKMUL & 0x20))
		;
	CLKSEL = SYS_OSC;
	CLKSEL |= USB_CLOCK;
}

static void port_init(void)
{
	P0MDOUT |= 0x10;
	XBR0    = 0x01;
	XBR1    = 0x40;
}

static void uart0_init(void)
{
   SCON0 = 0x10;                       // SCON0: 8-bit variable bit rate
                                       //        level of STOP bit is ignored
                                       //        RX enabled
                                       //        ninth bits are zeros
                                       //        clear RI0 and TI0 bits
   if (SYSCLK/BAUDRATE/2/256 < 1) {
      TH1 = -(SYSCLK/BAUDRATE/2);
      CKCON &= ~0x0B;                  // T1M = 1; SCA1:0 = xx
      CKCON |=  0x08;
   } else if (SYSCLK/BAUDRATE/2/256 < 4) {
      TH1 = -(SYSCLK/BAUDRATE/2/4);
      CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 01                  
      CKCON |=  0x01;
   } else if (SYSCLK/BAUDRATE/2/256 < 12) {
      TH1 = -(SYSCLK/BAUDRATE/2/12);
      CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 00
   } else {
      TH1 = -(SYSCLK/BAUDRATE/2/48);
      CKCON &= ~0x0B;                  // T1M = 0; SCA1:0 = 10
      CKCON |=  0x02;
   }

   TL1 = TH1;                          // init Timer1
   TMOD &= ~0xf0;                      // TMOD: timer 1 in 8-bit autoreload
   TMOD |=  0x20;                       
   TR1 = 1;                            // START Timer1
   TI0 = 1;                            // Indicate TX0 ready
}

void system_init(void)
{
	/* disable wdt */
	PCA0MD &= ~ 0x40;
	sysclk_init();
	port_init();
	uart0_init();
}

void watchdog_reset()
{
	PCA0MD |= 0x40;
	while (1)
		;
}

