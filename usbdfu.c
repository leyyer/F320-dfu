/* 
 * AVR DFU, Please see: 
 * http://www.atmel.com/zh/cn/Images/doc7618.pdf
 */
#include <stdio.h>
#include <stdlib.h>
#include "usbreq.h"
#include "sysconf.h"

#define CLAZ_DFU_TYPE_IN  0xA1
#define CLAZ_DFU_TYPE_OUT 0x21

#define DFU_DETACH    0
#define DFU_DNLOAD    1
#define DFU_UPLOAD    2
#define DFU_GETSTATUS 3
#define DFU_CLRSTATUS 4
#define DFU_GETSTATE  5
#define DFU_ABORT     6

#define DFU_STATUS_OK          0x00
#define DFU_STATUS_ERRTARGET   0x01 
#define DFU_STATUS_ERRFILE     0x02
#define DFU_STATUS_ERRWRITE    0x03
#define DFU_STATUS_ERRERASE    0x04
#define DFU_STATUS_ERRCHECK_ERASED    0x05
#define DFU_STATUS_ERRPROG     0x06
#define DFU_STATUS_ERRVERIFY   0x07

#define DFU_STATE_APPIDLE          0
#define DFU_STATE_APPDETACH        1
#define DFU_STATE_DFUIDLE          2
#define DFU_STATE_DFUDNLOAD_SYNC   3
#define DFU_STATE_DFUDNBUSY        4
#define DFU_STATE_DFUDNLOAD_IDLE   5
#define DFU_STATE_DFUMANIFEST_SYNC 6
#define DFU_STATE_DFUMANIFEST      7
#define DFU_STATE_DFUMANIFEST_WAIT_RESET 8
#define DFU_STATE_DFUUPLOAD_IDLE   9
#define DFU_STATE_DFUERROR         10

#define EP0_MAX_BYTES    32 

struct DfuStatusDescribe {
	uint8_t bStatus;
	uint8_t bwPollTimeOut[3];
	uint8_t bState;
	uint8_t iString;
} dfu_state = {
	.bStatus       = DFU_STATUS_OK,
	.bwPollTimeOut = { 0, 0, 1 },
	.bState        = DFU_STATE_DFUIDLE,
	.iString       = 0
};

static __xdata __at(0x0400) unsigned char dfu_buffer[EP0_MAX_BYTES];
static __xdata unsigned int sz_rx, sz_tx;
static __bit (*handle_tx)(void);
static __bit (*handle_rx)(void);
static unsigned int start_addr, end_addr;

struct dfu_config {
	struct UsbConfigurationDescriptor config_descr;
	struct UsbInterfaceDescriptor intf_descr;
};

static __code struct UsbDeviceDescriptor __usb_device_descriptor = {
	.bLength = sizeof (struct UsbDeviceDescriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_DEVICE,
	.bcdUSB          = 0x0200,
	.bDeviceClass    = 0xFE,
	.bDeviceSubClass = 0x01,
	.bDeviceProtocol = 0,
	.bMaxPacketSize  = EP0_MAX_BYTES,
	.idVendor        = 0x03EB, 
	.idProduct       = 0x2FF3, /* Use ATmega16U4's IDs */
	.bcdDevice       = 0x0000,
	.iManufacturer   = 0,
	.iProduct        = 0,
	.iSerialNumber   = 0,
	.bNumConfigurations = 1
};

static __code struct dfu_config dfu = {
	.config_descr = {
		.bLength         = sizeof (struct UsbConfigurationDescriptor),
		.bDescriptorType = USB_DESCRIPTOR_TYPE_CONFIGURATION,
		.wTotalLength    = sizeof dfu,
		.bNumInterfaces  = 1,
		.bConfigurationValue = 1,
		.iConfiguration  = 1,
		.bmAttributes    = 0x80, /* bus powered */
		.bMaxPower       = 100,
	},
	.intf_descr = {
		.bLength            = sizeof (struct UsbInterfaceDescriptor),
		.bDescriptorType    = USB_DESCRIPTOR_TYPE_INTERFACE,
		.bInterfaceNumber   = 0,
		.bAlternateSetting  = 0,
		.bNumEndpoints      = 0,
		.bInterfaceClass    = 0xFE,
		.bInterfaceSubClass = 0x1,
		.bInterfaceProtocol = 0x0,
		.iInterface         = 0
	},
};

void usb_device_descriptor(void)
{
	USB_FIFO0_WRITE(&__usb_device_descriptor, __usb_device_descriptor.bLength);
}

void usb_configuration_descriptor()
{
	USB_FIFO0_WRITE(&dfu, sizeof dfu);
}

static __code unsigned char usb_string0[] = {
	0x4, USB_DESCRIPTOR_TYPE_STRING,
	0x09, 0x04,
};

static __code unsigned char usb_string1[] = {
	 8, USB_DESCRIPTOR_TYPE_STRING,
	'd', 0,
	'f', 0,
	'u', 0,
};

/* microsoft os descriptor */
const __code unsigned char usb_stringee[] = { 
	0x12,  USB_DESCRIPTOR_TYPE_STRING,  
	'M',  0, 
	'S',  0, 
	'F',  0, 
	'T',  0,
	'1',  0,
	'0',  0, 
	'0',  0,
	0x10,  0 
};

void usb_string_descriptor()
{
	int idx = ep0_setup.wValue & 0xff;
	switch (idx) {
	case 0:
		USB_FIFO0_WRITE(usb_string0, usb_string0[0]);
		break;
	case 1:
		USB_FIFO0_WRITE(usb_string1, usb_string1[0]);
		break;
	case 0xee:
		USB_FIFO0_WRITE(usb_stringee, usb_stringee[0]);
		break;
	}
}

void usb_get_configuration()
{
	unsigned char buf[1] = {1};
	USB_FIFO0_WRITE(buf, 1);
}

static char up_read_cmd(void)
{
	char tx = 1;

	if (dfu_buffer[1] == 0x0) {
		switch (dfu_buffer[2]) {
		case 0: /* read bootloader version */
			dfu_buffer[0] = 0x10;
			break;
		case 1: /* read device boot id1 */
			dfu_buffer[0] = 0x1;
			break;
		case 2: /* read device boot id2 */
			dfu_buffer[0] = 0x2;
			break;
		default:
			tx = 0;
			break;
		}
	} else if (dfu_buffer[1] == 0x01) {
		switch (dfu_buffer[2]) {
		case  0x30: /* read manufacturer code */
			dfu_buffer[0] = 'i';
			break;
		case 0x31: /* read family code */
			dfu_buffer[0] = 1;
			break;
		case 0x60: /* read product name */
			dfu_buffer[0] = 1;
			break;
		case 0x61: /* read product resvision */
			dfu_buffer[0] = 1;
			break;
		default:
			tx = 0;
			break;
		}
	}
	return tx;
}

#ifdef DEBUG
static void dump_buffer(int n) __using(2)
{
	int x;
	for (x = 0; x < n; ++x) {
		printf("%02.2x ", dfu_buffer[x]);
	}
	putchar('\n');
}
#endif

static __bit handle_read_flash()
{
	unsigned int n = end_addr - start_addr + 1;
#ifdef DEBUG
	unsigned char x;
	__bit EA_save = EA;
#endif

	if (n <= 0) {
#ifdef DEBUG
		printf("zero end\n");
#endif
		usb_write_byte(E0CSR, E0CSR_DATAEND);
		return 0;
	}

	if (n > EP0_MAX_BYTES) {
		n = EP0_MAX_BYTES;
	}
#ifdef DEBUG
	printf("flash read send %d bytes\n", n);
	EA = 0;
	for (x = 0; x < n; ++x) {
		dfu_buffer[x] = *(unsigned char __code * __code)start_addr;
		++start_addr;
	}
	EA = EA_save;
	usb_fifo_write(FIFO_EP0, dfu_buffer, n);
#else
	usb_fifo_write(FIFO_EP0, (unsigned char __code * __code)start_addr, n);
	start_addr += n;
#endif
	usb_write_byte(E0CSR, E0CSR_INPRDY);
	return 1;
}

static void __do_flash_erase(unsigned int t)
{
	__xdata char *p = (__xdata char *)t;

	__bit EA_save = EA;
	EA = 0;
	FLKEY = 0xA5;
	FLKEY = 0xF1;
	PSCTL = 0x3;
	*p = 1;
	PSCTL = 0;
	EA = EA_save;
}

static void __do_flash_byte(unsigned int t, unsigned char f)
{
	__xdata char *p = (__xdata char *)t;
	__bit EA_save = EA;

	EA = 0;
	FLKEY = 0xA5;
	FLKEY = 0xF1;
	PSCTL = 0x01;
	*p = f;
	PSCTL = 0x0;
	EA = EA_save;
}

__bit usb_handle_tx(int ep)
{
	if (ep != 0)
		return 0;

	if (sz_tx > 0) {
#ifdef DEBUG
		printf("handle_tx: %d bytes\n", sz_tx);
#endif
		usb_fifo_write(FIFO_EP0, dfu_buffer, sz_tx);
		sz_tx = 0;
	}
	if (handle_tx) {
		return handle_tx();
	} 
	usb_write_byte(E0CSR, E0CSR_INPRDY | E0CSR_DATAEND);
	return 0;
}

__bit usb_handle_rx(int ep)
{
	if (ep != 0)
		return 0;

	sz_rx = usb_fifo_read(FIFO_EP0, dfu_buffer, EP0_MAX_BYTES);
#ifdef DEBUG
	printf("usb_rx: %d bytes\n", sz_rx);
	dump_buffer(sz_rx);
#endif
	if (handle_rx)
		return handle_rx();
	return 0;
}

static __bit do_flash_rx()
{
	unsigned char x;
#ifdef DEBUG
	printf("do_flash_rx\n");
#endif
	for (x = 0; x < sz_rx && start_addr <= end_addr; ++x) {
		__do_flash_byte(start_addr, dfu_buffer[x]);
		++start_addr;
	}
	if (start_addr < end_addr)
		return 1;
	return 0;
}

static __bit do_flash_rx0()
{
	unsigned char skip = start_addr % 32;

#ifdef DEBUG
	printf("skip: %d, use: %d\n", skip, sz_rx - skip);
#endif
	__do_flash_erase(start_addr + 1);

	for (skip; skip < sz_rx && start_addr <= end_addr; ++skip) {
#ifdef DEBUG
		printf("write: %x\n", start_addr);
#endif
		__do_flash_byte(start_addr, dfu_buffer[skip]);
		++start_addr;
	}

	if (start_addr < end_addr) {
		handle_rx = do_flash_rx;
		return 1;
	}
	return 0;
}

static void do_jump_app()
{
	if (dfu_buffer[2] == 0) {
		watchdog_reset();
	}
	if (dfu_buffer[2] == 0x01) { /* LJUMP address */
		start_app();
	}
}

static __bit do_dnl_rx()
{
	if (dfu_buffer[0] == 0x01) { /* ld_prog_start */
		if (dfu_buffer[1] != 0x00) {/* only support flash programming */
			return 0;
		}
		start_addr = (dfu_buffer[2] << 8) | dfu_buffer[3];
		end_addr = (dfu_buffer[4] << 8) | dfu_buffer[5];
		if (start_addr < APP_START) {
#ifdef DEBUG
			printf("don't write boot region.\n");
#endif
			return 0;
		}
		if (end_addr > start_addr) {
			handle_rx = do_flash_rx0;
			return 1;
		}
	}
	return 0;
}

static __bit handle_upload()
{
	if (dfu_buffer[0] == 0x05) { /* ld_read_command */
#ifdef DEBUG
		printf("read_command\n");
#endif
		up_read_cmd();
		usb_fifo_write(FIFO_EP0, dfu_buffer, 1);
		usb_write_byte(E0CSR, E0CSR_INPRDY | E0CSR_DATAEND);
		return 0;
	}
	if (dfu_buffer[0] == 0x3) { /* display flash contents */
		if (dfu_buffer[1] != 0) { /* only support flash data */
			return 0;
		}
		start_addr = (dfu_buffer[2] << 8) | dfu_buffer[3];
		end_addr = (dfu_buffer[4] << 8) | dfu_buffer[5];
#ifdef DEBUG
		printf("read flash: from %x to %x\n", start_addr, end_addr);
#endif
		handle_tx = handle_read_flash;
		return handle_tx();
	}
	return 0;
}

static __bit handle_tx_status()
{
	usb_fifo_write(FIFO_EP0, (unsigned char *)&dfu_state, sizeof dfu_state);
	usb_write_byte(E0CSR, E0CSR_INPRDY | E0CSR_DATAEND);
	return 0;
}

void usb_class_request(void)
{
	if (ep0_setup.bmRequestType == CLAZ_DFU_TYPE_IN) {
		switch (ep0_setup.bRequest) {
		case DFU_UPLOAD:
			handle_tx = handle_upload;
			sz_tx = 0;
			usb_endpoint_state(0, USB_EP_TX);
			break;
		case DFU_GETSTATUS:
			sz_tx = 0;
			usb_endpoint_state(0, USB_EP_TX);
			handle_tx = handle_tx_status;
			break;
		case DFU_GETSTATE:
			dfu_buffer[0] = dfu_state.bState;
			sz_tx = 1;
			usb_endpoint_state(0, USB_EP_TX);
			handle_tx = 0;
			break;
		}
	}
	if (ep0_setup.bmRequestType == CLAZ_DFU_TYPE_OUT) {
		switch (ep0_setup.bRequest) {
		case DFU_DETACH:
			break;
		case DFU_DNLOAD:
			if (ep0_setup.wLength == 0 && dfu_buffer[0] == 0x4 && dfu_buffer[1] == 0x03) {
				do_jump_app();
			} else {
				handle_rx = do_dnl_rx;
				usb_endpoint_state(0, USB_EP_RX);
			}
			break;
		case DFU_CLRSTATUS:
			dfu_state.bStatus = DFU_STATUS_OK;
			dfu_state.bState  = DFU_STATE_DFUIDLE;
			break;
		case DFU_ABORT:
			break;
		}
	}
}

