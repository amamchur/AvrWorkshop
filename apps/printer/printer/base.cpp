#include "base.hpp"

#include <LUFA/Drivers/USB/USB.h>

namespace printer {
    uint16_t base::get_language_string(void *dst, uint16_t size) {
        const wchar_t name[] = {LANGUAGE_ID_ENG};
        return fill_descriptor_string(dst, size, name, sizeof(name));
    }

    uint16_t base::get_manufacturer_string(void *dst, uint16_t size) {
        const wchar_t name[] = L"Andrii Mamchur";
        return fill_descriptor_string(dst, size, name, sizeof(name) - sizeof(name[0]));
    }

    uint16_t base::fill_descriptor_string(void *dst, uint16_t dst_size, const void *src, uint16_t src_size) {
        auto total_size = sizeof(USB_Descriptor_Header_t) + src_size;
        if (dst_size < total_size) {
            return 0;
        }

        auto *cfg = reinterpret_cast<USB_Descriptor_String_t *>(dst);
        cfg->Header.Size = total_size;
        cfg->Header.Type = DTYPE_String;
        memcpy(&cfg->UnicodeString, src, src_size);

        return total_size;
    }

    void base::init() {
        rx_bytes_ = 0;
        tx_bytes_ = 0;
    }

    void *base::shared_memory = nullptr;

    void *base::operator new(size_t size) noexcept {
        return shared_memory;
    }

    void base::operator delete(void *) noexcept {
    }
}
