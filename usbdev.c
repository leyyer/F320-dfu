#include <stdio.h>
#include "sysconf.h"
#include "usbdev.h"
#include "usbreq.h"

__xdata struct SetupPacket ep0_setup;
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

void usb_endpoint_state(int ep, int state)
{
	usb0_ep[ep] = state;
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

static void usb_standard_request(void)
{
	unsigned char v;

	switch (ep0_setup.bRequest) {
	case USB_GET_STATUS:
#ifdef DEBUG
		printf("get_status\n");
#endif
		break;
	case USB_CLEAR_FEATURE:
		break;
	case USB_SET_FEATURE:
		break;
	case USB_SET_ADDRESS:
		usb_write_byte(FADDR, 0x80 | ep0_setup.wValue);
		usb_write_byte(E0CSR, E0CSR_DATAEND);
		do {
			v = usb_read_byte(FADDR);
		} while (v & 0x80);
#ifdef DEBUG
		printf("set_address: %d\n", ep0_setup.wValue);
#endif
		break;
	case USB_GET_DESCRIPTOR:
		v = ep0_setup.wValue >> 8;
#ifdef DEBUG
		printf("descriptor: type %d, index: %d\n", v, ep0_setup.wValue & 0xff);
#endif
		switch (v) {
		case USB_DESCRIPTOR_TYPE_DEVICE:
			usb_device_descriptor();
			break;
		case USB_DESCRIPTOR_TYPE_CONFIGURATION:
			usb_configuration_descriptor();
			break;
		case USB_DESCRIPTOR_TYPE_STRING:
			usb_string_descriptor();
			break;
		}
		break;
	case USB_SET_DESCRIPTOR:
		break;
	case USB_GET_CONFIGURATION:
#ifdef DEBUG
		printf("get_configuration\n");
#endif
		usb_get_configuration();
		break;
	case USB_SET_CONFIGURATION:
		break;
	}
}

static void usb_standard_interface(void)
{
}

static void ep0_std_epreq(void)
{
}

static void ep0_in(void)
{
	unsigned char csr, cnt, type, recip;

	usb_write_byte(INDEX, 0x0);
	csr = usb_read_byte(E0CSR);

#ifdef DEBUG
	printf("ep0 csr: %x\n", csr);
#endif

	if (csr & E0CSR_SUEND) {
		/* XXX: stall */
		usb_write_byte(E0CSR, E0CSR_SSUEND);
	}

	if (csr & E0CSR_OPRDY) {
		if (usb0_ep[0] == USB_EP_IDLE) {
			cnt = usb_fifo_read(FIFO_EP0, (unsigned char *)&ep0_setup, 8);
#ifdef DEBUG
			printf("setup: %x, %d, %d, %d, %d\n", ep0_setup.bmRequestType,
					ep0_setup.bRequest, ep0_setup.wValue,
					ep0_setup.wIndex, ep0_setup.wLength);
#endif
			usb_write_byte(E0CSR, E0CSR_SOPRDY);
			type = (ep0_setup.bmRequestType >> 5) & 0x3;
			recip = ep0_setup.bmRequestType & 0x1f;
			switch (type) {
			case 0:
				/* standard */
				if (recip == 0)
					usb_standard_request();
				else if (recip == 1)
					usb_standard_interface();
				else if (recip == 2)
					ep0_std_epreq();
				break;
			case 1:  /* class */
				usb_class_request();
				break;
			case 2: /* vendor */
				break;
			}
			csr &= ~E0CSR_OPRDY;
		}  else if (usb0_ep[0] == USB_EP_RX) {
			unsigned char reg = E0CSR_SOPRDY;
			if (usb_handle_rx(0) == 0) {
				reg |= E0CSR_DATAEND;
				usb0_ep[0] = USB_EP_IDLE;
			}
			usb_write_byte(E0CSR, reg);
			csr &= ~E0CSR_OPRDY;
		}
	}

	if (usb0_ep[0] == USB_EP_TX) {
		if (csr & E0CSR_OPRDY) {
			usb0_ep[0] = USB_EP_IDLE;
			return;
		}
		if (usb_handle_tx(0) == 0) {
			usb0_ep[0] = USB_EP_IDLE;
		}
		return;
	}
}

void usb_poll(void)
{
	unsigned char creg = usb_read_byte(CMINT);

#ifdef DEBUG
	if (creg & ISRCOM_SOF) {
		printf("isr: sof\r\n");
	}
#endif
	if (creg & ISRCOM_RSTINT) {
#ifdef DEBUG
		printf("isr: RSTINT\r\n");
#endif
		usb0_state = USB_STATE_IDLE;
		usb0_ep[0] = USB_EP_IDLE;
		usb0_ep[1] = USB_EP_STALL;
		usb0_ep[2] = USB_EP_STALL;
		usb0_ep[3] = USB_EP_STALL;
	}

#ifdef DEBUG
	if (creg & ISRCOM_RSUINT) {
		printf("isr: RSUINT\r\n");
	}
	if (creg & ISRCOM_SUSINT) {
		printf("isr: SUSINT\r\n");
	}
#endif
	creg = usb_read_byte(IN1INT);
	if (creg & ISRIN_EP0)
		ep0_in();
}

void start_app() __naked
{
	__asm
		ljmp APP_START
	__endasm;
}

