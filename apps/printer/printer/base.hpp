#ifndef AVR_WORKSHOP_BASE_HPP
#define AVR_WORKSHOP_BASE_HPP

#include <LUFA/Drivers/USB/USB.h>

namespace printer {
    class base {
    public:
        static constexpr uint8_t string_id_language = 0;
        static constexpr uint8_t string_id_manufacturer = 1;
        static constexpr uint8_t string_id_product = 2;
        static constexpr uint8_t string_id_serial_number = 3;

        virtual ~base() = default;

        virtual void init();

        virtual uint16_t get_descriptor(void *dst, uint16_t size) = 0;

        virtual uint16_t get_configuration(void *dst, uint16_t size) = 0;

        virtual uint16_t get_product_string(void *dst, uint16_t size) = 0;

        virtual const char *name() = 0;

        virtual void process_byte(uint8_t b) = 0;

        static uint16_t fill_descriptor_string(void *dst, uint16_t dst_size, const void *src, uint16_t src_size);

        uint16_t get_language_string(void *dst, uint16_t size);

        uint16_t get_manufacturer_string(void *dst, uint16_t size);

        inline USB_ClassInfo_PRNT_Device_t *interface() {
            return &printer_interface;
        }

        inline uint32_t rx_bytes() const {
            return rx_bytes_;
        };

        inline uint32_t tx_bytes() const {
            return tx_bytes_;
        };

        size_t send_progmem_string(const char * str);

        static size_t send_progmem_uart(const char * str);

        static void *shared_memory;

        void *operator new(size_t size) noexcept;

        void operator delete(void *) noexcept;

    protected:
        uint32_t rx_bytes_{0};
        uint32_t tx_bytes_{0};
        USB_ClassInfo_PRNT_Device_t printer_interface{};
    };
}

#endif
