#include <stdint.h>

extern uint32_t _copy_data_load, _copy_data_store, _copy_data_store_end;
extern uint32_t _bss_start, _bss_end;

extern int main();
extern "C" int init_high();

int init_high() {
    // Copy data from flash to ram
    uint32_t *src = &_copy_data_load;
    uint32_t *dest = &_copy_data_store;
    uint32_t *end = &_copy_data_store_end;

    while (dest <= end) {
        *dest++ = *src++;
    }

    // Clear the BSS segment
    dest = &_bss_start;
    end = &_bss_end;
    while (dest <= end) {
        *dest++ = 0;
    }

    return main();
}
