#ifndef __USB_DEV_H__
#define __USB_DEV_H__

/* USB Core Registers */
#define  FADDR     0x00
#define  POWER     0x01
#define  IN1INT    0x02
#define  OUT1INT   0x04
#define  CMINT     0x06
#define  IN1IE     0x07
#define  OUT1IE    0x09
#define  CMIE      0x0B
#define  FRAMEL    0x0C
#define  FRAMEH    0x0D
#define  INDEX     0x0E
#define  CLKREC    0x0F
#define  E0CSR     0x11
#define  EINCSR1   0x11
#define  EINCSR2   0x12
#define  EOUTCSR1  0x14
#define  EOUTCSR2  0x15
#define  E0CNT     0x16
#define  EOUTCNTL  0x16
#define  EOUTCNTH  0x17
#define  FIFO_EP0  0x20
#define  FIFO_EP1  0x21
#define  FIFO_EP2  0x22
#define  FIFO_EP3  0x23

/* USB0 Common Interrupt */
#define ISRCOM_SOF       0x8
#define ISRCOM_RSTINT    0x4
#define ISRCOM_RSUINT    0x2
#define ISRCOM_SUSINT    0x1

/* USB0 IN Endpoint Interrupt */
#define ISRIN_IN3   0x8
#define ISRIN_IN2   0x4
#define ISRIN_IN1   0x2
#define ISRIN_EP0   0x1

/* USB0 Out Endpoint Interrupt */
#define ISROUT_OUT3 0x8
#define ISROUT_OUT2 0x4
#define ISROUT_OUT1 0x2

/* USB0 Endpoint0 control */
#define E0CSR_OPRDY   0x01
#define E0CSR_INPRDY  0x02
#define E0CSR_STSTL   0x04
#define E0CSR_DATAEND 0x08
#define E0CSR_SUEND   0x10
#define E0CSR_SDSTL   0x20
#define E0CSR_SOPRDY  0x40
#define E0CSR_SSUEND  0x80

/* usb0 status */
#define USB_STATE_IDLE        0
#define USB_STATE_ASSIGNED    1
#define USB_STATE_CONFIGURAED 2
/* usb endpoints status */
#define USB_EP_IDLE  0
#define USB_EP_TX    1
#define USB_EP_RX    2
#define USB_EP_STALL 3

void usb_endpoint_state(int ep, int state);
#endif

