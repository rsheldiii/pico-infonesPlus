#include "nvram.h"
#include <stdio.h>
#include <string.h>
#include "InfoNES.h"          // Include the main InfoNES header
#include "InfoNES_System.h" // For SRAM, SRAM_SIZE, SRAMwritten, ERRORMESSAGESIZE
#include "FrensHelpers.h" // For GetfileNameFromFullPath, stripextensionfromfilename, GAMESAVEDIR
#include "rom_selector.h"

// --- Module-level variables to hold dependencies ---
static ROMSelector* rom_selector_instance = nullptr;
static char* current_rom_full_path = nullptr;
static char* error_message = nullptr;
// ---

// Helper function moved from main.cpp
static uint32_t get_current_nvram_addr()
{
    if (!rom_selector_instance || !rom_selector_instance->getCurrentROM())
    {
        return 0; // Use 0 to indicate invalid address or no ROM
    }
    int slot = rom_selector_instance->getCurrentNVRAMSlot();
    if (slot < 0)
    {
        return 0; // Use 0 to indicate invalid slot
    }
    printf("SRAM slot %d\n", slot);
    // Assuming ROM_FILE_ADDR and SRAM_SIZE are accessible (likely from InfoNES_System.h or similar)
    return ROM_FILE_ADDR - SRAM_SIZE * (slot + 1);
}


void nvram_init(ROMSelector* selector, char* current_rom_name, char* error_message_buffer)
{
    rom_selector_instance = selector;
    current_rom_full_path = current_rom_name;
    error_message = error_message_buffer;
}

void nvram_save()
{
    if (!current_rom_full_path || current_rom_full_path[0] == '\0') {
        printf("NVRAM Save Error: ROM name not set.\n");
        return;
    }
    if (!error_message) {
        printf("NVRAM Save Error: Error message buffer not set.\n");
        return;
    }

    char file_base_name[FF_MAX_LFN];
    char full_save_path[FF_MAX_LFN];

    strcpy(file_base_name, Frens::GetfileNameFromFullPath(current_rom_full_path));
    Frens::stripextensionfromfilename(file_base_name);

    if (!SRAMwritten)
    {
        printf("SRAM not written to, skipping save.\n");
        return;
    }

    snprintf(full_save_path, FF_MAX_LFN, "%s/%s.SAV", GAMESAVEDIR, file_base_name);
    printf("Save SRAM to %s\n", full_save_path);

    FIL fil;
    FRESULT fr;
    fr = f_open(&fil, full_save_path, FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FR_OK)
    {
        snprintf(error_message, ERRORMESSAGESIZE, "Cannot open save file %s: %d", full_save_path, fr);
        printf("%s\n", error_message);
        return;
    }

    size_t bytesWritten;
    fr = f_write(&fil, SRAM, SRAM_SIZE, &bytesWritten);
    if (fr != FR_OK || bytesWritten < SRAM_SIZE)
    {
        snprintf(error_message, ERRORMESSAGESIZE, "Error writing save %s: %d (%u/%u written)", full_save_path, fr, bytesWritten, SRAM_SIZE);
        printf("%s\n", error_message);
        // Note: We might still want to close the file even on error.
    } else {
        printf("SRAM saved successfully.\n");
        SRAMwritten = false; // Reset flag only on successful write
    }

    f_close(&fil);
}


bool nvram_load()
{
    if (!current_rom_full_path || current_rom_full_path[0] == '\0') {
        printf("NVRAM Load Error: ROM name not set.\n");
        return false; // Indicate failure
    }
     if (!error_message) {
        printf("NVRAM Load Error: Error message buffer not set.\n");
        return false; // Indicate failure
    }

    char file_base_name[FF_MAX_LFN];
    char full_save_path[FF_MAX_LFN];
    FILINFO fno;
    bool ok = false;

    strcpy(file_base_name, Frens::GetfileNameFromFullPath(current_rom_full_path));
    Frens::stripextensionfromfilename(file_base_name);
    snprintf(full_save_path, FF_MAX_LFN, "%s/%s.SAV", GAMESAVEDIR, file_base_name);

    FIL fil;
    FRESULT fr;
    size_t bytesRead = 0; // Initialize bytesRead

    uint32_t flash_addr = get_current_nvram_addr();

    if (flash_addr != 0) // Check if there's a valid flash slot
    {
        printf("Attempting to load SRAM from %s or flash addr 0x%08lX\n", full_save_path, flash_addr);
        fr = f_stat(full_save_path, &fno);

        if (fr == FR_NO_FILE) // Save file doesn't exist
        {
            printf("Save file not found, attempting load from flash 0x%08lX\n", flash_addr);
            // Assuming SRAM and SRAM_SIZE are globally accessible via InfoNES_System.h
            memcpy(SRAM, reinterpret_cast<void *>(flash_addr), SRAM_SIZE);
            ok = true;
            printf("SRAM loaded from flash.\n");
        }
        else if (fr == FR_OK) // Save file exists
        {
            printf("Loading save file %s\n", full_save_path);
            fr = f_open(&fil, full_save_path, FA_READ);
            if (fr == FR_OK)
            {
                fr = f_read(&fil, SRAM, SRAM_SIZE, &bytesRead);
                if (fr == FR_OK && bytesRead == SRAM_SIZE)
                {
                    printf("SRAM loaded successfully from %s (%u bytes).\n", full_save_path, bytesRead);
                    ok = true;
                }
                else
                {
                    snprintf(error_message, ERRORMESSAGESIZE, "Read fail %s: %d (%u/%u read)", full_save_path, fr, bytesRead, SRAM_SIZE);
                    printf("%s\n", error_message);
                    // ok remains false
                }
                f_close(&fil); // Close file regardless of read success
            }
            else
            {
                snprintf(error_message, ERRORMESSAGESIZE, "Cannot open %s: %d", full_save_path, fr);
                printf("%s\n", error_message);
                // ok remains false
            }
        }
        else // Error stating the file
        {
            snprintf(error_message, ERRORMESSAGESIZE, "f_stat() fail %s: %d", full_save_path, fr);
            printf("%s\n", error_message);
             // ok remains false
        }
    }
    else // No valid flash address (e.g., ROM doesn't support saving, or no slot assigned)
    {
         printf("No valid NVRAM flash slot for this ROM. Skipping NVRAM load.\n");
         // Check if a .SAV file *still* exists (maybe from a previous version or manual placement)
         fr = f_stat(full_save_path, &fno);
         if (fr == FR_OK) {
             printf("Found existing save file %s even without flash slot. Attempting load.\n", full_save_path);
             fr = f_open(&fil, full_save_path, FA_READ);
             if (fr == FR_OK) {
                 fr = f_read(&fil, SRAM, SRAM_SIZE, &bytesRead);
                 if (fr == FR_OK && bytesRead == SRAM_SIZE) {
                     printf("SRAM loaded successfully from %s (%u bytes).\n", full_save_path, bytesRead);
                     ok = true;
                 } else {
                    snprintf(error_message, ERRORMESSAGESIZE, "Read fail %s: %d (%u/%u read)", full_save_path, fr, bytesRead, SRAM_SIZE);
                    printf("%s\n", error_message);
                 }
                 f_close(&fil);
             } else {
                snprintf(error_message, ERRORMESSAGESIZE, "Cannot open %s: %d", full_save_path, fr);
                printf("%s\n", error_message);
             }
         } else {
            // No flash slot and no save file, nothing to load.
            // Consider this a success in terms of loading (as there's nothing *to* load).
            // Ensure SRAM is in a known state (e.g., zeroed) if needed, though InfoNES_Reset might handle this.
            memset(SRAM, 0, SRAM_SIZE); // Optional: Explicitly clear SRAM if no save exists
            ok = true;
         }
    }

    SRAMwritten = false; // Reset write flag after any load attempt (successful or not)
    return ok;
} 