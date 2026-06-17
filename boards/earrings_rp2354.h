// -----------------------------------------------------------------------------
// Custom board: NES earrings — Raspberry Pi RP2354A (QFN-60, 30 GPIO)
//
// The RP2354A is an RP2350A die with 2MB of *stacked* internal flash.
// This is NOT a Pico 2 (RP2350A + 4MB external flash). Building with
// PICO_BOARD=pico2 declares 4MB, which makes the firmware believe there is
// ~3MB of ROM space (ROM_FILE_ADDR = XIP_BASE + 1MB) when only 1MB physically
// exists past that point — fine for a single <=1MB ROM, wrong for anything
// larger. This board file declares the correct 2MB so the flash math is right.
//
// Derived from the SDK's pico2.h, minus wifi/PSRAM, with 2MB flash.
// Select with: -DPICO_BOARD=earrings_rp2354 (PICO_BOARD_HEADER_DIRS is set to
// this directory by the top-level CMakeLists.txt).
// -----------------------------------------------------------------------------
#ifndef _BOARDS_EARRINGS_RP2354_H
#define _BOARDS_EARRINGS_RP2354_H

// For board detection
#define RASPBERRYPI_EARRINGS_RP2354

// RP2354A: QFN-60, 30 GPIO (same package class as RP2350A / Pico 2)
#define PICO_RP2350A 1

// --- UART (stdio) ---
#ifndef PICO_DEFAULT_UART
#define PICO_DEFAULT_UART 0
#endif
#ifndef PICO_DEFAULT_UART_TX_PIN
#define PICO_DEFAULT_UART_TX_PIN 0
#endif
#ifndef PICO_DEFAULT_UART_RX_PIN
#define PICO_DEFAULT_UART_RX_PIN 1
#endif

// --- I2C ---
#ifndef PICO_DEFAULT_I2C
#define PICO_DEFAULT_I2C 0
#endif
#ifndef PICO_DEFAULT_I2C_SDA_PIN
#define PICO_DEFAULT_I2C_SDA_PIN 4
#endif
#ifndef PICO_DEFAULT_I2C_SCL_PIN
#define PICO_DEFAULT_I2C_SCL_PIN 5
#endif

// --- SPI ---
#ifndef PICO_DEFAULT_SPI
#define PICO_DEFAULT_SPI 0
#endif
#ifndef PICO_DEFAULT_SPI_SCK_PIN
#define PICO_DEFAULT_SPI_SCK_PIN 18
#endif
#ifndef PICO_DEFAULT_SPI_TX_PIN
#define PICO_DEFAULT_SPI_TX_PIN 19
#endif
#ifndef PICO_DEFAULT_SPI_RX_PIN
#define PICO_DEFAULT_SPI_RX_PIN 16
#endif
#ifndef PICO_DEFAULT_SPI_CSN_PIN
#define PICO_DEFAULT_SPI_CSN_PIN 17
#endif

// --- LED (HW_CONFIG=6 also sets LED_GPIO_PIN=25 via CMake) ---
#ifndef PICO_DEFAULT_LED_PIN
#define PICO_DEFAULT_LED_PIN 25
#endif

// --- Flash: RP2354 has 2MB stacked internal flash ---
// THIS is the correctness fix vs pico2's 4MB. Stacked flash is accessed over
// the same XIP/QMI path, so the standard W25Q080 boot2 normally works
// (verify on real hardware).
#ifndef PICO_BOOT_STAGE2_CHOOSE_W25Q080
#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1
#endif

#ifndef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 2
#endif

#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (2 * 1024 * 1024)
#endif

// No PSRAM on this board
#ifndef PICO_SMPS_MODE_PIN
// (left undefined — board has no SMPS power-save mode pin)
#endif

// All RP2350 A2
#ifndef PICO_RP2350_A2_SUPPORTED
#define PICO_RP2350_A2_SUPPORTED 1
#endif

#endif // _BOARDS_EARRINGS_RP2354_H
