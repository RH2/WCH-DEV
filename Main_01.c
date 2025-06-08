#ifndef __USB_DESC_H
#define __USB_DESC_H

#include "ch32v20x.h"

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define CDC_DATA_IN_PACKET_SIZE                 64
#define CDC_DATA_OUT_PACKET_SIZE                64

#define CDC_CMD_PACKET_SIZE                     8

#define CDC_DESC_SIZE                           67

#define CDC_SIZ_DEVICE_DESC                     18
#define CDC_SIZ_CONFIG_DESC                     67
#define CDC_SIZ_STRING_LANGID                   4
#define CDC_SIZ_STRING_VENDOR                   38
#define CDC_SIZ_STRING_PRODUCT                  30
#define CDC_SIZ_STRING_SERIAL                   26

extern const uint8_t CDC_DeviceDescriptor[CDC_SIZ_DEVICE_DESC];
extern const uint8_t CDC_ConfigDescriptor[CDC_SIZ_CONFIG_DESC];
extern const uint8_t CDC_StringLangID[CDC_SIZ_STRING_LANGID];
extern const uint8_t CDC_StringVendor[CDC_SIZ_STRING_VENDOR];
extern const uint8_t CDC_StringProduct[CDC_SIZ_STRING_PRODUCT];
extern uint8_t CDC_StringSerial[CDC_SIZ_STRING_SERIAL];

#endif
