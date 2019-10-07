#include "adpt1.hpp"

namespace {
    constexpr uint8_t PRINTER_IN_EPADDR = (uint8_t) (ENDPOINT_DIR_IN | 2);
    constexpr uint8_t PRINTER_OUT_EPADDR = (uint8_t) (ENDPOINT_DIR_OUT | 1);
    constexpr uint16_t PRINTER_IO_EPSIZE = 64;

    char IEEE1284String[] = "";
    const char PROGMEM enq_response[] = "\x05""A@\x06";
    const char PROGMEM mm_response[] =  "M46\x06";
    const char PROGMEM mv_response[] =  "V03\x06";
    const char PROGMEM mr_response[] =  "R04\x06";
    const char PROGMEM mc_response[] =  "C00\x06";
    const char PROGMEM mi_response[] =  "I00\x06";
    const char PROGMEM mp_response[] =  "P00\x06";
    const char PROGMEM mts_response[] =  "MTS18088753\x06";
}
namespace printer {
    void adpt1::init() {
        rx_bytes_ = 0;
        tx_bytes_ = 0;

        printer_interface.Config.InterfaceNumber = 0;
        printer_interface.Config.DataINEndpoint.Address = PRINTER_IN_EPADDR;
        printer_interface.Config.DataINEndpoint.Size = PRINTER_IO_EPSIZE;
        printer_interface.Config.DataINEndpoint.Banks = 1;
        printer_interface.Config.DataOUTEndpoint.Address = PRINTER_OUT_EPADDR;
        printer_interface.Config.DataOUTEndpoint.Size = PRINTER_IO_EPSIZE;
        printer_interface.Config.DataOUTEndpoint.Banks = 1;
        printer_interface.Config.IEEE1284String = IEEE1284String;

        parser.init();
        parser.context(this);
        parser.callback(command_callback);
    }

    uint16_t adpt1::get_descriptor(void *dst, uint16_t size) {
        if (size < sizeof(USB_Descriptor_Device_t)) {
            return 0;
        }

        auto *descriptor = reinterpret_cast<USB_Descriptor_Device_t *>(dst);
        descriptor->Header.Size = sizeof(USB_Descriptor_Device_t);
        descriptor->Header.Type = DTYPE_Device;
        descriptor->USBSpecification = VERSION_BCD(2, 0, 0);
        descriptor->Class = USB_CSCP_NoDeviceClass;
        descriptor->SubClass = USB_CSCP_NoDeviceSubclass;
        descriptor->Protocol = USB_CSCP_NoDeviceProtocol;
        descriptor->Endpoint0Size = FIXED_CONTROL_ENDPOINT_SIZE;
        descriptor->VendorID = vendor_id;
        descriptor->ProductID = product_id;
        descriptor->ReleaseNumber = VERSION_BCD(1, 0, 0);
        descriptor->ManufacturerStrIndex = string_id_manufacturer;
        descriptor->ProductStrIndex = string_id_product;
        descriptor->SerialNumStrIndex = string_id_serial_number;
        descriptor->NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS;
        return sizeof(USB_Descriptor_Device_t);
    }

    uint16_t adpt1::get_configuration(void *dst, uint16_t size) {
        if (size < sizeof(usb_configuration)) {
            return 0;
        }

        auto *cfg = reinterpret_cast<usb_configuration *>(dst);
        cfg->Config.Header.Size = sizeof(USB_Descriptor_Configuration_Header_t);
        cfg->Config.Header.Type = DTYPE_Configuration;
        cfg->Config.TotalConfigurationSize = sizeof(usb_configuration);
        cfg->Config.TotalInterfaces = 1;
        cfg->Config.ConfigurationNumber = 1;
        cfg->Config.ConfigurationStrIndex = NO_DESCRIPTOR;
        cfg->Config.ConfigAttributes = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED);
        cfg->Config.MaxPowerConsumption = USB_CONFIG_POWER_MA(100);

        cfg->Printer_Interface.Header.Size = sizeof(USB_Descriptor_Interface_t);
        cfg->Printer_Interface.Header.Type = DTYPE_Interface;
        cfg->Printer_Interface.InterfaceNumber = 0;
        cfg->Printer_Interface.AlternateSetting = 0;
        cfg->Printer_Interface.TotalEndpoints = 2;
        cfg->Printer_Interface.Class = PRNT_CSCP_PrinterClass;
        cfg->Printer_Interface.SubClass = PRNT_CSCP_PrinterSubclass;
        cfg->Printer_Interface.Protocol = PRNT_CSCP_BidirectionalProtocol;
        cfg->Printer_Interface.InterfaceStrIndex = NO_DESCRIPTOR;

        cfg->Printer_DataInEndpoint.Header.Size = sizeof(USB_Descriptor_Endpoint_t);
        cfg->Printer_DataInEndpoint.Header.Type = DTYPE_Endpoint;

        cfg->Printer_DataInEndpoint.EndpointAddress = PRINTER_IN_EPADDR,
        cfg->Printer_DataInEndpoint.Attributes = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
        cfg->Printer_DataInEndpoint.EndpointSize = PRINTER_IO_EPSIZE;
        cfg->Printer_DataInEndpoint.PollingIntervalMS = 0x05;

        cfg->Printer_DataOutEndpoint.Header.Size = sizeof(USB_Descriptor_Endpoint_t);
        cfg->Printer_DataOutEndpoint.Header.Type = DTYPE_Endpoint;
        cfg->Printer_DataOutEndpoint.EndpointAddress = PRINTER_OUT_EPADDR;
        cfg->Printer_DataOutEndpoint.Attributes = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA);
        cfg->Printer_DataOutEndpoint.EndpointSize = PRINTER_IO_EPSIZE;
        cfg->Printer_DataOutEndpoint.PollingIntervalMS = 0x05;

        return sizeof(usb_configuration);
    }

    uint16_t adpt1::get_product_string(void *dst, uint16_t size) {
        const wchar_t name[] = L"ADTP1 Simulator";
        return fill_descriptor_string(dst, size, name, sizeof(name) - sizeof(name[0]));
    }

    void adpt1::command_callback(base_parser *p, int e) {
        auto me = reinterpret_cast<adpt1 *>(p->context());
        me->process_command(p, (mpcl_parse_event) e);
    }

    void adpt1::process_command(base_parser *p, mpcl_parse_event event) {
        const char *response = nullptr;
        switch (event) {
            case mpcl_parse_event::command_enq:
                response = enq_response;
                break;
            case mpcl_parse_event::command_mm:
                response = mm_response;
                break;
            case mpcl_parse_event::command_mv:
                response = mv_response;
                break;
            case mpcl_parse_event::command_mr:
                response = mr_response;
                break;
            case mpcl_parse_event::command_mc:
                response = mc_response;
                break;
            case mpcl_parse_event::command_mi:
                response = mi_response;
                break;
            case mpcl_parse_event::command_mp:
                response = mp_response;
                break;
            case mpcl_parse_event::command_mts:
                response = mts_response;
                break;
            default:
                return;
        }

        if (response != nullptr) {
            tx_bytes_ += send_progmem_string(response);
        }
    }

    void adpt1::process_byte(uint8_t b) {
        rx_bytes_++;
        parser.push(static_cast<char>(b));
    }

    const char *adpt1::name() {
        return "ADPM1";
    }
}