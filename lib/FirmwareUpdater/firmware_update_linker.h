#pragma once

#include <stdint.h>
#include <stddef.h>

#define __FIRMWARE __attribute((section(".firmware_update_section"))) __attribute__((used))
#define __FIRMWARE_BSS __attribute((section(".firmware_update_bss"))) __attribute__((used))
#define __FIRMWARE_DATA __attribute((section(".firmware_update_data"))) __attribute__((used))
#define __FIRMWARE_RODATA __attribute((section(".firmware_update_rodata"))) __attribute__((used))

size_t __FIRMWARE firmware_strlen(const char *str);
char* __FIRMWARE firmware_strncat(char *dest, const char *src, size_t n);
char* __FIRMWARE firmware_strncpy(char *dest, const char *src, size_t n);

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