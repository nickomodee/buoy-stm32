#pragma once

#include <stdint.h>
#include <stddef.h>

#define __FIRMWARE __attribute__((section(".firmware_update_section"), used, noinline))
#define __FIRMWARE_BSS __attribute__((section(".firmware_update_bss"), used, noinline))
#define __FIRMWARE_DATA __attribute__((section(".firmware_update_data"), used, noinline))
#define __FIRMWARE_RODATA __attribute__((section(".firmware_update_rodata"), used, noinline))

extern const char firmware_empty[];
extern const char firmware_crlf[];

extern const char firmware_ram[];
extern const char firmware_nand[];
extern const char firmware_cf[];
extern const char firmware_sd1[];
extern const char firmware_sd2[];
extern const char firmware_usb1[];
extern const char firmware_usb2[];
extern const char firmware_usb3[];

extern const char firmware_illegal_chars[];
extern const char firmware_illegal_chars_2[];
extern const char firmware_boot_jump_code_oem_name[];
extern const char firmware_volume_label_fat32_signature[];
extern const char firmware_volume_label_fat_signature[];