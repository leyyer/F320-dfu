#include <stdlib.h>
#include "usbreq.h"

struct dfu_config {
	struct UsbConfigurationDescriptor config_descr;
	struct UsbInterfaceDescriptor intf_descr;
};

static __xdata struct UsbDeviceDescriptor __usb_device_descriptor = {
	.bLength = sizeof (struct UsbDeviceDescriptor),
	.bDescriptorType = USB_DESCRIPTOR_TYPE_DEVICE,
	.bcdUSB          = 0x0200,
	.bDeviceClass    = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize  = 64,
	.idVendor        = 0xc410,
	.idProduct       = 0xefd0,
	.bcdDevice       = 0x0101,
	.iManufacturer   = 1,
	.iProduct        = 2,
	.iSerialNumber   = 3,
	.bNumConfigurations = 1
};

void usb_device_descriptor(void)
{
	usb_write_ep0(&__usb_device_descriptor, __usb_device_descriptor.bLength);
}

static __xdata struct dfu_config dfu = {
	.config_descr = {
		.bLength = sizeof (struct UsbConfigurationDescriptor),
		.bDescriptorType = USB_DESCRIPTOR_TYPE_CONFIGURATION,
		.wTotalLength = sizeof dfu,
		.bNumInterfaces = 1,
		.bConfigurationValue = 1,
		.iConfiguration = 4,
		.bmAttributes = 0x80, /* bus powered */
		.bMaxPower = 100,
	},
	.intf_descr = {
		.bLength = sizeof (struct UsbInterfaceDescriptor),
		.bDescriptorType = USB_DESCRIPTOR_TYPE_INTERFACE,
		.bInterfaceNumber = 1,
		.bAlternateSetting = 0,
		.bNumEndpoints = 0,
		.bInterfaceClass = 0xFE,
		.bInterfaceSubClass = 0x1,
		.bInterfaceProtocol = 0x0,
		.iInterface = 2
	},
};

void usb_configuration_descriptor(int idx)
{
	usb_write_ep0(&dfu, sizeof dfu);
}

static __xdata unsigned char usb_string0[] = {
	0x4, USB_DESCRIPTOR_TYPE_STRING,
	0x09, 0x04,
};

static __xdata unsigned char usb_string1[] = {
	14, USB_DESCRIPTOR_TYPE_STRING,
	'C', 0,
	'o', 0,
	'o', 0,
	'D', 0,
	'E', 0,
	'V', 0,
};

static __xdata unsigned char usb_string3[] = {
	18, USB_DESCRIPTOR_TYPE_STRING,
	'0', 0,
	'1', 0,
	'0', 0,
	'1', 0,
	'0', 0,
	'1', 0,
	'0', 0,
	'1', 0,
};

static __xdata unsigned char usb_string2[] = {
	 8, USB_DESCRIPTOR_TYPE_STRING,
	'd', 0,
	'f', 0,
	'u', 0,
};

/* microsoft os descriptor */
const __idata unsigned char usb_stringee[] =
{ 0x12,  USB_DESCRIPTOR_TYPE_STRING,  'M',  0,  'S',  0,  'F',  0,  'T',  0,  '1',  0,  '0',  0,  '0',  0,  0x10,  0 };

void usb_string_descriptor(int idx)
{
	switch (idx) {
	case 0:
		usb_write_ep0(&usb_string0, usb_string0[0]);
		break;
	case 1:
		usb_write_ep0(&usb_string1, usb_string1[0]);
		break;
	case 2:
		usb_write_ep0(&usb_string2, usb_string2[0]);
		break;
	case 3:
		usb_write_ep0(&usb_string3, usb_string3[0]);
		break;
	case 0xee:
		usb_write_ep0(&usb_stringee, usb_stringee[0]);
	}
}

