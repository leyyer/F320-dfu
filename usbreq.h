#ifndef __USBREQ_H__
#define __USBREQ_H__
#include <stdint.h>

/* standard device request */
#define USB_GET_STATUS      (0x00)
#define USB_CLEAR_FEATURE   (0x01)
#define USB_SET_FEATURE     (0x03)
#define USB_SET_ADDRESS     (0x05)
#define USB_GET_DESCRIPTOR  (0x06)
#define USB_SET_DESCRIPTOR  (0x07)
#define USB_GET_CONFIGURATION (0x08)
#define USB_SET_CONFIGURATION (0x09)

/* standard interface request */
#define USB_GET_INTERFACE  0x0A
#define USB_SET_INTERFACE  0x11

/* standard interface request */
#define USB_SYNCH_FRAME 0x12

#define  EP_DIR_OUT  0x0
#define  EP_DIR_IN   0x80
struct SetupPacket {
#define  EP0_DIR_MASK (0x80)
	uint8_t   bmRequestType;
	uint8_t   bRequest;
	uint16_t  wValue;
	uint16_t  wIndex;
	uint16_t  wLength;
};

#define USB_DESCRIPTOR_TYPE_DEVICE  0x01
struct UsbDeviceDescriptor {
	uint8_t      bLength;
	uint8_t      bDescriptorType;
	uint16_t     bcdUSB;
	uint8_t      bDeviceClass;
	uint8_t      bDeviceSubClass;
	uint8_t      bDeviceProtocol;
	uint8_t      bMaxPacketSize;
	uint16_t     idVendor;
	uint16_t     idProduct;
	uint16_t     bcdDevice;
	uint8_t      iManufacturer;
	uint8_t      iProduct;
	uint8_t      iSerialNumber;
	uint8_t      bNumConfigurations;
};

#define USB_DESCRIPTOR_TYPE_CONFIGURATION  0x02
struct UsbConfigurationDescriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
};

#define USB_DESCRIPTOR_TYPE_INTERFACE 0x04
struct UsbInterfaceDescriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
};

#define USB_DESCRIPTOR_TYPE_ENDPOINT 0x05
struct UsbEndpointDescriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint8_t wMaxPacketSize;
	uint8_t bInterval;
};

#define USB_DESCRIPTOR_TYPE_STRING 0x03
struct UsbStringDescriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	unsigned char bString[1];
};

void usb_write_ep0(void *dp, int size);

/* device side api */
void usb_device_descriptor(void);
void usb_configuration_descriptor(int idx);
void usb_string_descriptor(int idx);
void usb_get_configuration();
void usb_class_request(struct SetupPacket *sp);
#endif

