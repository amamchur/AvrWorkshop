#ifndef AVR_WORKSHOP_XLP504_HPP
#define AVR_WORKSHOP_XLP504_HPP

#include "../parser/easyplug_parser.hpp"
#include "base.hpp"

namespace printer {
    class xlp504 : public base {
    public:
        typedef struct {
            USB_Descriptor_Configuration_Header_t Config;
            USB_Descriptor_Interface_t Printer_Interface;
            USB_Descriptor_Endpoint_t Printer_DataOutEndpoint;
            USB_Descriptor_Endpoint_t Printer_DataInEndpoint;
        } usb_configuration;

        static constexpr uint16_t vendor_id = 0x2bfe;
        static constexpr uint16_t product_id = 0x0e45;

        void init() override;

        const char *name_pgmem() override;

        uint16_t get_descriptor(void *dst, uint16_t size) override;

        uint16_t get_configuration(void *dst, uint16_t size) override;

        uint16_t get_product_string(void *dst, uint16_t size) override;

        void process_byte(uint8_t b) override;

    private:
        static void command_callback(base_parser *p, int e);

        easyplug_parser<128> parser;

        void process_command(base_parser *p, easyplug_parse_event event);
    };
}

#endif
