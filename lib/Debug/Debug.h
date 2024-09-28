#pragma once

#include "PAL.h"

#define DEBUG

#define DEBUG_NOOP() (void)0 // no-op

#ifdef DEBUG
// Debug for BCP
#define DEBUG_BCP_PRINT(...) PAL_SERIAL.print("[BCP] "); PAL_SERIAL.print(__VA_ARGS__)
#define DEBUG_BCP_PRINTLN(...) PAL_SERIAL.print("[BCP] "); PAL_SERIAL.println(__VA_ARGS__)
#define DEBUG_BCP_WRITE(...) PAL_SERIAL.print("[BCP] "); PAL_SERIAL.write(__VA_ARGS__)

// Debug for encryption
#define DEBUG_ENCRYPTION_PRINT(...) PAL_SERIAL.print("[ENCRYPTION] "); PAL_SERIAL.print(__VA_ARGS__)
#define DEBUG_ENCRYPTION_PRINTLN(...) PAL_SERIAL.print("[ENCRYPTION] "); PAL_SERIAL.println(__VA_ARGS__)
#define DEBUG_ENCRYPTION_WRITE(...) PAL_SERIAL.print("[ENCRYPTION] "); PAL_SERIAL.write(__VA_ARGS__)

// Debug for packet
#define DEBUG_PACKET_PRINT(...) PAL_SERIAL.print("[PACKET] "); PAL_SERIAL.print(__VA_ARGS__)
#define DEBUG_PACKET_PRINTLN(...) PAL_SERIAL.print("[PACKET] "); PAL_SERIAL.println(__VA_ARGS__)
#define DEBUG_PACKET_WRITE(...) PAL_SERIAL.print("[PACKET] "); PAL_SERIAL.write(__VA_ARGS__)

// Debug for LoRa
#define DEBUG_LORA_PRINT(...) PAL_SERIAL.print("[LORA] "); PAL_SERIAL.print(__VA_ARGS__)
#define DEBUG_LORA_PRINTLN(...) PAL_SERIAL.print("[LORA] "); PAL_SERIAL.println(__VA_ARGS__)
#define DEBUG_LORA_WRITE(...) PAL_SERIAL.print("[LORA] "); PAL_SERIAL.write(__VA_ARGS__)
#else
// Debug for BCP
#define DEBUG_BCP_PRINT(...) DEBUG_NOOP()
#define DEBUG_BCP_PRINTLN(...) DEBUG_NOOP()
#define DEBUG_BCP_WRITE(...) DEBUG_NOOP()

// Debug for encryption
#define DEBUG_ENCRYPTION_PRINT(...) DEBUG_NOOP()
#define DEBUG_ENCRYPTION_PRINTLN(...) DEBUG_NOOP()
#define DEBUG_ENCRYPTION_WRITE(...) DEBUG_NOOP()

// Debug for packet
#define DEBUG_PACKET_PRINT(...) DEBUG_NOOP()
#define DEBUG_PACKET_PRINTLN(...) DEBUG_NOOP()
#define DEBUG_PACKET_WRITE(...) DEBUG_NOOP()

// Debug for LoRa
#define DEBUG_LORA_PRINT(...) DEBUG_NOOP()
#define DEBUG_LORA_PRINTLN(...) DEBUG_NOOP()
#define DEBUG_LORA_WRITE(...) DEBUG_NOOP()
#endif