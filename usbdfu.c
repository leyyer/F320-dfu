/* 
 * AVR DFU, Please see: 
 * http://www.atmel.com/zh/cn/Images/doc7618.pdf
 */
#include <stdlib.h>
#include "usbreq.h"

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

struct DfuStatusDescribe {
	uint8_t bStatus;
	uint8_t bwPollTimeOut[3];
	uint8_t bState;
	uint8_t iString;
} dfu_state = {
	.bStatus = DFU_STATUS_OK,
	.bwPollTimeOut = {0,0,1},
	.bState = DFU_STATE_DFUIDLE,
	.iString = 0
};

struct dfu_config {
	struct UsbConfigurationDescriptor config_descr;
	struct UsbInterfaceDescriptor intf_descr;
};

static __xdata struct UsbDeviceDescriptor __usb_device_descriptor = {
	.bLength = sizeof (struct UsbDeviceDescriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_DEVICE,
	.bcdUSB          = 0x0200,
	.bDeviceClass    = 0xFE,
	.bDeviceSubClass = 0x01,
	.bDeviceProtocol = 0,
	.bMaxPacketSize  = 32,
	.idVendor        = 0x03EB, 
	.idProduct       = 0x2FF3, /* Use ATmega16U4's IDs */
	.bcdDevice       = 0x0000,
	.iManufacturer   = 0,
	.iProduct        = 0,
	.iSerialNumber   = 0,
	.bNumConfigurations = 1
};
static __xdata struct dfu_config dfu = {
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
	usb_write_ep0(&__usb_device_descriptor, __usb_device_descriptor.bLength);
}

void usb_configuration_descriptor(int idx)
{
	usb_write_ep0(&dfu, sizeof dfu);
}

static __xdata unsigned char usb_string0[] = {
	0x4, USB_DESCRIPTOR_TYPE_STRING,
	0x09, 0x04,
};

static __xdata unsigned char usb_string1[] = {
	 8, USB_DESCRIPTOR_TYPE_STRING,
	'd', 0,
	'f', 0,
	'u', 0,
};

/* microsoft os descriptor */
const __idata unsigned char usb_stringee[] =
{ 0x12,  USB_DESCRIPTOR_TYPE_STRING,  
	'M',  0, 
	'S',  0, 
	'F',  0, 
	'T',  0,
	'1',  0,
	'0',  0, 
	'0',  0,
	0x10,  0 
};

void usb_string_descriptor(int idx)
{
	switch (idx) {
	case 0:
		usb_write_ep0(&usb_string0, usb_string0[0]);
		break;
	case 1:
		usb_write_ep0(&usb_string1, usb_string1[0]);
		break;
	case 0xee:
		usb_write_ep0(&usb_stringee, usb_stringee[0]);
	}
}

void usb_get_configuration()
{
	usb_write_ep0(1, 1);
}

void usb_class_request(struct SetupPacket *sp)
{
	if (sp->bmRequestType == CLAZ_DFU_TYPE_IN || sp->bmRequestType == CLAZ_DFU_TYPE_OUT) {
		switch (sp->bRequest) {
		case DFU_DETACH:
			break;
		case DFU_DNLOAD:
			break;
		case DFU_UPLOAD:
			break;
		case DFU_GETSTATUS:
			usb_write_ep0(&dfu_state, sizeof dfu_state);
			break;
		case DFU_CLRSTATUS:
			dfu_state.bStatus = DFU_STATUS_OK;
			dfu_state.bState  = DFU_STATE_DFUIDLE;
			break;
		case DFU_GETSTATE:
			break;
		case DFU_ABORT:
			break;
		}
	}
}

