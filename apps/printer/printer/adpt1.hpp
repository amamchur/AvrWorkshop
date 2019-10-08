#ifndef AVR_WORKSHOP_ADPT1_HPP
#define AVR_WORKSHOP_ADPT1_HPP

#include "../parser/mpcl_parser.hpp"
#include "base.hpp"

namespace printer {
    class adpt1 : public base {
    public:
        typedef struct {
            USB_Descriptor_Configuration_Header_t Config;
            USB_Descriptor_Interface_t Printer_Interface;
            USB_Descriptor_Endpoint_t Printer_DataOutEndpoint;
            USB_Descriptor_Endpoint_t Printer_DataInEndpoint;
        } usb_configuration;

        static constexpr uint16_t vendor_id = 0x12EE;
        static constexpr uint16_t product_id = 0x1043;

        void init() override;

        const char *name_pgmem() override;

        uint16_t get_descriptor(void *dst, uint16_t size) override;

        uint16_t get_configuration(void *dst, uint16_t size) override;

        uint16_t get_product_string(void *dst, uint16_t size) override;

        void process_byte(uint8_t b) override;

    private:
        static void command_callback(base_parser *p, int e);

        mpcl_parser<128> parser;

        void process_command(base_parser *p, mpcl_parse_event event);
    };
}

#endif
