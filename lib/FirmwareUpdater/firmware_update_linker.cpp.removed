#include "firmware_update_linker.h"

const char __FIRMWARE_RODATA firmware_empty[] = "";
const char __FIRMWARE_RODATA firmware_crlf[] = "\r\n";

const char __FIRMWARE_RODATA firmware_ram[] = "RAM";
const char __FIRMWARE_RODATA firmware_nand[] = "NAND";
const char __FIRMWARE_RODATA firmware_cf[] = "CF";
const char __FIRMWARE_RODATA firmware_sd1[] = "SD1";
const char __FIRMWARE_RODATA firmware_sd2[] = "SD2";
const char __FIRMWARE_RODATA firmware_usb1[] = "USB1";
const char __FIRMWARE_RODATA firmware_usb2[] = "USB2";
const char __FIRMWARE_RODATA firmware_usb3[] = "USB3";

const char __FIRMWARE_RODATA firmware_illegal_chars[] = "\"*+,:;<=>\?[]|\x7F";
const char __FIRMWARE_RODATA firmware_illegal_chars_2[] = "+,;=[]";
const char __FIRMWARE_RODATA firmware_boot_jump_code_oem_name[] = "\xEB\xFE\x90" "MSDOS5.0";
const char __FIRMWARE_RODATA firmware_volume_label_fat32_signature[] = "NO NAME    " "FAT32   ";
const char __FIRMWARE_RODATA firmware_volume_label_fat_signature[] = "NO NAME    " "FAT     ";