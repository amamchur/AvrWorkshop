#include "xlp504.hpp"
#include "../localization.hpp"

namespace {
    constexpr uint8_t PRINTER_IN_EPADDR = (uint8_t) (ENDPOINT_DIR_IN | 2);
    constexpr uint8_t PRINTER_OUT_EPADDR = (uint8_t) (ENDPOINT_DIR_OUT | 1);
    constexpr uint16_t PRINTER_IO_EPSIZE = 64;

    char IEEE1284String[] = "";
    const char PROGMEM x0_response[] = "S0000A100M000000F065535KV7.75-SIM       ";
    const char PROGMEM pg30021_response[] = "427 m\r\n";
    const char PROGMEM pg30068_response[] = "A108023117520338\r\n";
    const char PROGMEM pg1_response[] = "#!A1\r\n"
                                        "#PC2045/50\r\n"
                                        "*PC1020/0.0\r\n"
                                        "*PC1021/0.0\r\n"
                                        "#PC1003/4.0\r\n"
                                        "#PC1004/4.0\r\n"
                                        "#PC2027/0\r\n"
                                        "#PC2066/1\r\n"
                                        "#PC2018/0\r\n"
                                        "#PC1008/0.0\r\n"
                                        "#PC1005/1\r\n"
                                        "#PC1006/77.8\r\n"
                                        "#PC1007/84.0\r\n"
                                        "#PC1022/0\r\n"
                                        "#PC1023/128\r\n"
                                        "#PC2015/0\r\n"
                                        "#PC2030/1\r\n"
                                        "#PC1038/450.0\r\n"
                                        "#PC1039/79.1\r\n"
                                        "#PC1040/33.0\r\n"
                                        "#PC1009/1\r\n"
                                        "#PC1029/0\r\n"
                                        "#PC1010/0\r\n"
                                        "#PC1011/0\r\n"
                                        "#PC1012/0\r\n"
                                        "#PC1013/1\r\n"
                                        "#PC1027/0\r\n"
                                        "#PC2063/1\r\n"
                                        "#PC2042/0\r\n"
                                        "#PC2043/0\r\n"
                                        "#PC2031/1\r\n"
                                        "#PC1019/1\r\n"
                                        "#PC1019/1\r\n"
                                        "#PC3203/0\r\n"
                                        "#PC3204/0\r\n"
                                        "#PC3205/0\r\n"
                                        "#PC3206/0\r\n"
                                        "#PC3207/0\r\n"
                                        "#PC3208/0\r\n"
                                        "#PC3209/1\r\n"
                                        "#PC3210/0\r\n"
                                        "#PC3211/0\r\n"
                                        "#PC1014/0\r\n"
                                        "#PC1015/3\r\n"
                                        "#PC1016/105\r\n"
                                        "#PC1017/0.0\r\n"
                                        "#PC1018/0.0\r\n"
                                        "#PC1041/1\r\n"
                                        "#PC1019/1\r\n"
                                        "*PC5123/32832\r\n"
                                        "#PC1017/0.0\r\n"
                                        "#PC1014/0\r\n"
                                        "#PC1017/0.0\r\n"
                                        "#PC2004/0\r\n"
                                        "*PC2005/171\r\n"
                                        "#PC2035/0\r\n"
                                        "#PC2039/0\r\n"
                                        "#PC6004/15.0\r\n"
                                        "#PC6014/0\r\n"
                                        "#PC6017/0.0\r\n"
                                        "#PC2501/0\r\n"
                                        "#PC2504/110\r\n"
                                        "#PC2505/110\r\n"
                                        "#PC2502/100\r\n"
                                        "#PC2503/250\r\n"
                                        "#PC2506/0\r\n"
                                        "#PC2507/30\r\n"
                                        "#PC2508/94\r\n"
                                        "#PC2509/50\r\n"
                                        "#PC2510/0\r\n"
                                        "#PC2511/120\r\n"
                                        "#PC1019/1\r\n"
                                        "#PC2501/0\r\n"
                                        "#PC2504/110\r\n"
                                        "#PC2505/110\r\n"
                                        "#PC2502/100\r\n"
                                        "#PC2503/250\r\n"
                                        "#PC2506/0\r\n"
                                        "#PC2507/30\r\n"
                                        "#PC2508/94\r\n"
                                        "#PC2509/50\r\n"
                                        "#PC2510/0\r\n"
                                        "#PC2511/120\r\n"
                                        "#PC3301/0\r\n"
                                        "#PC3303/10\r\n"
                                        "#PC3302/1\r\n"
                                        "#PC3304/0\r\n"
                                        "#PC3152/0\r\n"
                                        "#PC3153/190\r\n"
                                        "#PC3154/0\r\n"
                                        "#PC3155/350.0\r\n"
                                        "#PC3158/0\r\n"
                                        "#PC2051/1\r\n"
                                        "#PC2053/0\r\n"
                                        "#PC2081/0\r\n"
                                        "#PC2020/1\r\n"
                                        "*PC5001/1\r\n"
                                        "*PC5002/7\r\n"
                                        "*PC2024/660\r\n"
                                        "*PC2025/1130\r\n"
                                        "*PC2080/1650\r\n"
                                        "#PC2016/0\r\n"
                                        "#PC2048/4096\r\n"
                                        "#PC2046/512\r\n"
                                        "#PC2047/256\r\n"
                                        "#PC1104/64\r\n"
                                        "#PC5129/0\r\n"
                                        "#PC2029/2\r\n"
                                        "#PC2067/1\r\n"
                                        "#PC2068/5\r\n"
                                        "#PC2033/1\r\n"
                                        "#PC2050/0\r\n"
                                        "#PC2083/20.0\r\n"
                                        "#PC2003/36.3\r\n"
                                        "#PC2060/0\r\n"
                                        "#PC2022/1\r\n"
                                        "#PC2023/0\r\n"
                                        "#PC2026/20\r\n"
                                        "#PC2049/0\r\n"
                                        "#PC2012/0\r\n"
                                        "#PC2014/0\r\n"
                                        "#PC2013/9\r\n"
                                        "#PC2071/0\r\n"
                                        "#PC1102/0\r\n"
                                        "#PC1550/0\r\n"
                                        "#PC5310/0\r\n"
                                        "*PC1103/1\r\n"
                                        "#PC5004/0\r\n"
                                        "#PC4002/15\r\n"
                                        "#PC4006/0\r\n"
                                        "#PC4007/0\r\n"
                                        "#PC4010/0\r\n"
                                        "#PC4011/0\r\n"
                                        "#PC4009/0\r\n"
                                        "#PC4013/0\r\n"
                                        "#PC4017/0\r\n"
                                        "#PC4004/94\r\n"
                                        "#PC4003/126\r\n"
                                        "#PC4005/44\r\n"
                                        "#PC4014/1\r\n"
                                        "#PC4015/1\r\n"
                                        "#PC4016/1\r\n"
                                        "#PC4018/1\r\n"
                                        "#PC1101/7\r\n"
                                        "#PC1501/0\r\n"
                                        "*PC1502/172.018.001.008\r\n"
                                        "*PC1503/255.255.000.000\r\n"
                                        "*PC1504/000.000.000.000\r\n"
                                        "#PC1505/9100\r\n"
                                        "#PC1506/0\r\n"
                                        "#PC1513/XLP504_600dpi_3313FE#G\r\n"
                                        "#PC1509/1\r\n"
                                        "#PC1510/5\r\n"
                                        "#PC1511/admin#G\r\n"
                                        "#PC1512/supervisor#G\r\n"
                                        "#PC1532/operator#G\r\n"
                                        "#PC1507/1\r\n"
                                        "#PC1508/novexx#G\r\n"
                                        "#PC1529/0\r\n"
                                        "#PC1530/-2105212662\r\n"
                                        "#PC1531/3600\r\n"
                                        "#PC1533/0.0\r\n"
                                        "#PC1201/8\r\n"
                                        "#PC1202/8\r\n"
                                        "#PC1203/2\r\n"
                                        "#PC1204/1\r\n"
                                        "#PC1205/0\r\n"
                                        "#PC1206/0\r\n"
                                        "#PC1207/1\r\n"
                                        "#PC1302/8\r\n"
                                        "#PC1303/8\r\n"
                                        "#PC1304/2\r\n"
                                        "#PC1305/1\r\n"
                                        "#PC1306/0\r\n"
                                        "#PC1307/0\r\n"
                                        "#PC1308/1\r\n"
                                        "#PC1351/2\r\n"
                                        "#PC1353/8\r\n"
                                        "#PC1354/1\r\n"
                                        "#PC1355/2\r\n"
                                        "#PC1356/0\r\n"
                                        "#PC1357/0\r\n"
                                        "#PC1358/1\r\n"
                                        "#PC1361/2\r\n"
                                        "#PC1363/8\r\n"
                                        "#PC1364/1\r\n"
                                        "#PC1365/2\r\n"
                                        "#PC1366/0\r\n"
                                        "#PC1368/1\r\n"
                                        "#PC1600/3\r\n"
                                        "#PC1601/0\r\n"
                                        "#PC5005/0\r\n"
                                        "#PC5113/0\r\n"
                                        "#PC5125/0\r\n"
                                        "#PC5111/0\r\n"
                                        "#PC5112/0\r\n"
                                        "#PC1024/20\r\n"
                                        "#PC5101/35\r\n"
                                        "#PC5102/0.0\r\n"
                                        "#PC5105/0.0\r\n"
                                        "*PC5116/78\r\n"
                                        "*PC5117/112\r\n"
                                        "*PC5119/128\r\n"
                                        "*PC5120/128\r\n"
                                        "*PC5121/128\r\n"
                                        "*PC5122/128\r\n"
                                        "*PC5104/0.0\r\n"
                                        "#PC5127/0\r\n"
                                        "#PC5128/-1872945986\r\n"
                                        "#PC5124/0\r\n"
                                        "#PC5404/0\r\n"
                                        "#PC5400/0\r\n"
                                        "#PC5401/0\r\n"
                                        "#PC5402/0\r\n"
                                        "#PC5403/0\r\n"
                                        "#PC5407/0\r\n"
                                        "#PC5406/0\r\n"
                                        "#PC5131/1\r\n"
                                        "#PC5405/0\r\n"
                                        "#PC5409/0\r\n"
                                        "#PC2081/0\r\n"
                                        "#PC2051/1\r\n"
                                        "#PC1101/7\r\n"
                                        "#PC2082/0\r\n"
                                        "#PC1501/0\r\n"
                                        "#PC1201/8\r\n"
                                        "#PC1202/8\r\n"
                                        "#PC1203/2\r\n"
                                        "#PC1205/0\r\n"
                                        "#PC1207/1\r\n"
                                        "#PC999999/-1#G";
}

namespace printer {
    void xlp504::init() {
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

    uint16_t xlp504::get_descriptor(void *dst, uint16_t size) {
        if (size < sizeof(USB_Descriptor_Device_t)) {
            return 0;
        }

        auto *descriptor = reinterpret_cast<USB_Descriptor_Device_t *>(dst);
        descriptor->Header.Size = sizeof(USB_Descriptor_Device_t);
        descriptor->Header.Type = DTYPE_Device;
        descriptor->USBSpecification = VERSION_BCD(1, 1, 0);
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

    uint16_t xlp504::get_configuration(void *dst, uint16_t size) {
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

    uint16_t xlp504::get_product_string(void *dst, uint16_t size) {
        const wchar_t name[] = L"XLP504 Simulator";
        return fill_descriptor_string(dst, size, name, sizeof(name) - sizeof(name[0]));
    }

    void xlp504::command_callback(base_parser *p, int e) {
        auto me = reinterpret_cast<xlp504 *>(p->context());
        me->process_command(p, (easyplug_parse_event) e);
    }

    void xlp504::process_command(base_parser *p, easyplug_parse_event event) {
        const char *response = nullptr;
        switch (event) {
            case easyplug_parse_event::command_x0:
                response = x0_response;
                break;
            case easyplug_parse_event::command_pg1:
                response = pg1_response;
                break;
            case easyplug_parse_event::command_pg30068:
                response = pg30068_response;
                break;
            case easyplug_parse_event::command_pg30021:
                response = pg30021_response;
                break;
            default:
                return;
        }

        if (response != nullptr) {
            tx_bytes_ += send_progmem_string(response);
        }
    }

    void xlp504::process_byte(uint8_t b) {
        rx_bytes_++;
        parser.push(static_cast<char>(b));
    }

    const char *xlp504::name_pgmem() {
        return text_xlp504_name;
    }
}
