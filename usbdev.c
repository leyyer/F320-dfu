#include <stdio.h>
#include "sysconf.h"
#include "usbdev.h"
#include "usbreq.h"

/* usb0 status */
#define USB_STATE_IDLE     0
#define USB_STATE_ASSIGNED 1
/* usb endpoints status */
#define USB_EP_IDLE  0
#define USB_EP_TX    1
#define USB_EP_RX    2
#define USB_EP_STALL 3

static struct SetupPacket  __xdata last_setup;
static unsigned char __xdata usb0_state;
static unsigned char __xdata usb0_ep[4];

unsigned char usb_read_byte(unsigned char addr)
{
	while (USB0ADR & 0x80)
		;
	USB0ADR = (0x80 | addr);
	while (USB0ADR & 0x80)
		;
	return USB0DAT;
}

int usb_fifo_read(unsigned char ep, unsigned char *bufp, int len)
{
	unsigned char cnt, xlen;

	cnt = usb_read_byte(E0CNT);
	xlen = cnt > len ? len : cnt;

	USB0ADR = (0xC0 | ep);
	for (cnt = 0; cnt < xlen; ++cnt) {
		while (USB0ADR & 0x80)
			;
		*bufp++ = USB0DAT;
	}
	USB0ADR = 0;
	return xlen;
}

void usb_write_byte(unsigned char addr, unsigned char byte)
{
	while (USB0ADR & 0x80)
		;
	USB0ADR = addr;
	USB0DAT = byte;
}

void usb_fifo_write(unsigned char ep, unsigned char *buf, int len)
{
	int x;
	while (USB0ADR & 0x80)
		;
	USB0ADR = ep;
	for (x = 0; x < len; ++x) {
		USB0DAT = buf[x];
		while (USB0ADR & 0x80)
			;
	}
}

void usb_write_ep0(void *dp, int size)
{
	unsigned char *bufp = dp;
	usb_fifo_write(FIFO_EP0, bufp, size);
}

void usb_init(void)
{
	/* force asynchronous USB reset */
	usb_write_byte(POWER, 0x08);
	/* enable all endpoints in interrupts */
	usb_write_byte(IN1IE, 0xf);
	/* enable all endpoints out interrupts */
	usb_write_byte(OUT1IE, 0xE);

	/* enable reset, resume, suspend interrupts */
	usb_write_byte(CMIE, 0x07);

	USB0XCN = 0xE0;

	/* enable clock recovery */
	usb_write_byte(CLKREC, 0x89);

	/* enable usb interrupt */
	EIE1 |= 0x02; 

	/* inhibit bit and enable suspend detection */
	usb_write_byte(POWER, 0x01);
}

static void usb_handle_cmint(void)
{
	unsigned char creg = usb_read_byte(CMINT);

	if (creg & ISRCOM_SOF) {
		printf("isr: sof\r\n");
	}

	if (creg & ISRCOM_RSTINT) {
		printf("isr: RSTINT\r\n");
		usb0_state = USB_STATE_IDLE;
		usb0_ep[0] = USB_EP_IDLE;
		usb0_ep[1] = USB_EP_STALL;
		usb0_ep[2] = USB_EP_STALL;
		usb0_ep[3] = USB_EP_STALL;
	}

	if (creg & ISRCOM_RSUINT) {
		printf("isr: RSUINT\r\n");
	}
	if (creg & ISRCOM_SUSINT) {
		printf("isr: SUSINT\r\n");
	}
}

static void ep0_std_devreq(void)
{
	unsigned char reg;
	static __xdata struct UsbStringDescriptor *string_descr;

	switch (last_setup.bRequest) {
	case USB_GET_STATUS:
		break;
	case USB_CLEAR_FEATURE:
		break;
	case USB_SET_FEATURE:
		break;
	case USB_SET_ADDRESS:
		usb_write_byte(FADDR, 0x80 | last_setup.wValue);
		usb_write_byte(E0CSR, E0CSR_DATAEND);
		do {
			reg = usb_read_byte(FADDR);
		} while (reg & 0x80);
		printf("set_address: %d\n", last_setup.wValue);
		break;
	case USB_GET_DESCRIPTOR:
		reg = last_setup.wValue >> 8;
		if (reg == USB_DESCRIPTOR_TYPE_DEVICE) {
			usb_device_descriptor();
			printf("get device descriptor\n"); 
		} else if (reg == USB_DESCRIPTOR_TYPE_CONFIGURATION) {
			usb_configuration_descriptor(last_setup.wValue & 0xff);
			printf("get config descriptor: index: %d\n", (last_setup.wValue & 0xff));
		} else if (reg == USB_DESCRIPTOR_TYPE_STRING) {
			usb_string_descriptor(last_setup.wValue & 0xff);
			printf("get string descriptor: index: %d\n", (last_setup.wValue & 0xff));
		} else {
			printf("unknown descriptor %d\n", reg);
		}
		usb_write_byte(E0CSR, E0CSR_INPRDY | E0CSR_DATAEND);
		break;
	case USB_SET_DESCRIPTOR:
		break;
	case USB_GET_CONFIGURATION:
		break;
	case USB_SET_CONFIGURATION:
		break;
	}
}

static void ep0_std_intfreq(void)
{
}

static void ep0_std_epreq(void)
{
}

static void ep0_setup(void)
{
	unsigned char type, recip;

	type = (last_setup.bmRequestType >> 5) & 0x3;
	recip = last_setup.bmRequestType & 0x1f;
	if (type == 0) {
		/* standard */
		if (recip == 0)
			ep0_std_devreq();
		else if (recip == 1)
			ep0_std_intfreq();
		else if (recip == 2)
			ep0_std_epreq();
	} else if (type == 1) { /* class */
	} else if (type == 2) { /* vendor */
	}
}

static void ep0_in(void)
{
	unsigned char csr, cnt;

	usb_write_byte(INDEX, 0x0);
	csr = usb_read_byte(E0CSR);

	printf("ep0 csr: %x\n", csr);
	if (csr & E0CSR_SUEND) {
		/* XXX: stall */
		usb_write_byte(E0CSR, E0CSR_SSUEND);
	}

	if (csr & E0CSR_OPRDY) {
		if (usb0_ep[0] == USB_EP_IDLE) {
			cnt = usb_fifo_read(FIFO_EP0, (unsigned char *)&last_setup, 8);
			printf("reading: %d bytes\n", cnt);
			printf("setup: %x, %d, %d, %d, %d\n", last_setup.bmRequestType,
					last_setup.bRequest, last_setup.wValue,
					last_setup.wIndex, last_setup.wLength);
			usb_write_byte(E0CSR, E0CSR_SOPRDY);
			ep0_setup();
		}
	}
}

static void usb_handle_in1int()
{
	unsigned char creg = usb_read_byte(IN1INT);

	if (creg & ISRIN_IN3) {
		printf("ep3 in\r\n");
	}

	if (creg & ISRIN_IN2) {
		printf("ep2 in\r\n");
	}

	if (creg & ISRIN_IN1) {
		printf("ep1 in\r\n");
	}

	if (creg & ISRIN_EP0) {
		printf("ep0 in\r\n");
		ep0_in();
	}
}

static void usb_handle_out1int(void)
{
	unsigned char creg = usb_read_byte(OUT1INT);

	if (creg & ISROUT_OUT3) {
		printf("ep3 out\r\n");
	}
	if (creg & ISROUT_OUT2) {
		printf("ep2 out\r\n");
	}
	if (creg & ISROUT_OUT1) {
		printf("ep1 out\r\n");
	}
}

void usb_isr(void) __interrupt(8) __using(2)
{
	printf("usb_isr\r\n");
	usb_handle_cmint();
	usb_handle_in1int();
	usb_handle_out1int();
}

