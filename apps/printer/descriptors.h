#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

/* Includes: */
#include <avr/pgmspace.h>

#include <LUFA/Drivers/USB/USB.h>

/* Type Defines: */
/** Type define for the device configuration descriptor structure. This must be defined in the
 *  application code, as the configuration descriptor contains several sub-descriptors which
 *  vary between devices, and which describe the device's usage to the host.
 */
typedef struct
{
    USB_Descriptor_Configuration_Header_t Config;

    // Printer Interface
    USB_Descriptor_Interface_t            Printer_Interface;
    USB_Descriptor_Endpoint_t             Printer_DataOutEndpoint;
    USB_Descriptor_Endpoint_t             Printer_DataInEndpoint;
} USB_Descriptor_Configuration_t;

/** Enum for the device interface descriptor IDs within the device. Each string descriptor
 *  should have a unique ID index associated with it, which can be used to refer to the
 *  interface from other descriptors.
 */
enum InterfaceDescriptors_t
{
    INTERFACE_ID_Printer = 0, /**< Printer interface descriptor ID */
};

/** Enum for the device string descriptor IDs within the device. Each string descriptor should
 *  have a unique ID index associated with it, which can be used to refer to the string from
 *  other descriptors.
 */
enum StringDescriptors_t {
    STRING_ID_Language = 0, /**< Supported Languages string descriptor ID (must be zero) */
    STRING_ID_Manufacturer = 1, /**< Manufacturer string ID */
    STRING_ID_Product = 2, /**< Product string ID */
    STRING_ID_SerialNumber = 3, /**< Product string ID */
};

#define PRINTER_IN_EPADDR         (uint8_t)(ENDPOINT_DIR_IN  | 2)

/** Endpoint address of the Printer host-to-device data OUT endpoint. */
#define PRINTER_OUT_EPADDR        (uint8_t)(ENDPOINT_DIR_OUT | 1)

/** Size in bytes of the Printer data endpoints. */
#define PRINTER_IO_EPSIZE         64

/* Function Prototypes: */
uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint16_t wIndex,
                                    const void **const DescriptorAddress)
ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif
