#ifndef __USBREQ_H__
#define __USBREQ_H__
#include <stdint.h>
#include "usbdev.h"

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

extern __xdata struct SetupPacket ep0_setup;
void usb_endpoint_state(int ep, int state);
unsigned char usb_read_byte(unsigned char addr);
void usb_write_byte(unsigned char addr, unsigned char byte);
void usb_fifo_write(unsigned char ep, unsigned char *buf, int len);
int  usb_fifo_read(unsigned char ep, unsigned char *bufp, int len);

#define READ_END()   usb_write_byte(E0CSR, E0CSR_SOPRDY)
#define USB_FIFO0_WRITE(dp, size) \
	do { \
		int min = ep0_setup.wLength > size ? size : ep0_setup.wLength; \
		usb_fifo_write(FIFO_EP0, (unsigned char *)dp, min);\
		usb_write_byte(E0CSR, E0CSR_INPRDY | E0CSR_DATAEND); \
	} while (0)

#define USB_FIFO0_READ(dp, size) \
	do { \
		int min = ep0_setup.wLength > size ? size : ep0_setup.wLength; \
		usb_fifo_read(FIFO_EP0, (unsigned char *)dp, min);\
	} while (0)


/* device side api */
void usb_device_descriptor(void);
void usb_configuration_descriptor();
void usb_string_descriptor();
void usb_get_configuration();
void usb_class_request();
__bit usb_handle_tx(int ep);
__bit usb_handle_rx(int ep);
#endif

