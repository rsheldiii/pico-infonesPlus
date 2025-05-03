#ifndef NVRAM_H
#define NVRAM_H

#include "ff.h" // For FRESULT, FIL, etc.
#include "rom_selector.h" // For ROMSelector definition

// Initialize the NVRAM module with necessary dependencies
void nvram_init(ROMSelector* selector, char* current_rom_name, char* error_message_buffer);

// Save the current SRAM contents to a file
void nvram_save();

// Load SRAM contents from a file or flash
// Returns true on success, false on failure.
bool nvram_load();

#endif // NVRAM_H 