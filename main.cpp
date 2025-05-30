#include <stdio.h>
#include <cstdarg>
#include "pico/stdlib.h"
#include "hardware/divider.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "hardware/watchdog.h"
#include "util/work_meter.h"
#include "InfoNES.h"
#include "InfoNES_System.h"
#include "InfoNES_pAPU.h"
#include "ff.h"
#include "tusb.h"
#include "gamepad.h"
#include "rom_selector.h"
#include "menu.h"
#include "nespad.h"
#include "wiipad.h"
#include "FrensHelpers.h"
#include "settings.h"
#include "FrensFonts.h"
#include "nvram.h"

bool isFatalError = false;

char *romName;

static bool fps_enabled = false;
static uint32_t start_tick_us = 0;
static uint32_t fps = 0;

constexpr uint32_t CPUFreqKHz = 252000;

namespace
{
    ROMSelector romSelector_;
}

#define INFONES_LINE_BUFFER_OFFSET 0 // was 32, but I think that's maybe wrong?

// this is to crush RGB555 into RGB444
// #define CC(x) (((x >> 1) & 15) | (((x >> 6) & 15) << 4) | (((x >> 11) & 15) << 8))
// this is to expand RGB555 into RGB565. it might not be very accurate
#define CC(x) ((((x) & 0x7C00) << 1) | (((x) & 0x03E0) << 1) | ((x) & 0x001F))
const WORD __not_in_flash_func(NesPalette)[64] = {
    CC(0x39ce), CC(0x1071), CC(0x0015), CC(0x2013), CC(0x440e), CC(0x5402), CC(0x5000), CC(0x3c20),
    CC(0x20a0), CC(0x0100), CC(0x0140), CC(0x00e2), CC(0x0ceb), CC(0x0000), CC(0x0000), CC(0x0000),
    CC(0x5ef7), CC(0x01dd), CC(0x10fd), CC(0x401e), CC(0x5c17), CC(0x700b), CC(0x6ca0), CC(0x6521),
    CC(0x45c0), CC(0x0240), CC(0x02a0), CC(0x0247), CC(0x0211), CC(0x0000), CC(0x0000), CC(0x0000),
    CC(0x7fff), CC(0x1eff), CC(0x2e5f), CC(0x223f), CC(0x79ff), CC(0x7dd6), CC(0x7dcc), CC(0x7e67),
    CC(0x7ae7), CC(0x4342), CC(0x2769), CC(0x2ff3), CC(0x03bb), CC(0x0000), CC(0x0000), CC(0x0000),
    CC(0x7fff), CC(0x579f), CC(0x635f), CC(0x6b3f), CC(0x7f1f), CC(0x7f1b), CC(0x7ef6), CC(0x7f75),
    CC(0x7f94), CC(0x73f4), CC(0x57d7), CC(0x5bf9), CC(0x4ffe), CC(0x0000), CC(0x0000), CC(0x0000)};

static DWORD prevButtons[2]{};
static int rapidFireMask[2]{};
static int rapidFireCounter = 0;
void InfoNES_PadState(DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem)
{
    static constexpr int LEFT = 1 << 6;
    static constexpr int RIGHT = 1 << 7;
    static constexpr int UP = 1 << 4;
    static constexpr int DOWN = 1 << 5;
    static constexpr int SELECT = 1 << 2;
    static constexpr int START = 1 << 3;
    static constexpr int A = 1 << 0;
    static constexpr int B = 1 << 1;

    ++rapidFireCounter;
    bool reset = false;
    bool usbConnected = false;
    for (int i = 0; i < 2; ++i)
    {
        auto &dst = i == 0 ? *pdwPad1 : *pdwPad2;
        auto &gp = io::getCurrentGamePadState(i);
        if ( i == 0 )
        {
            usbConnected = gp.isConnected();
        }
        int v = (gp.buttons & io::GamePadState::Button::LEFT ? LEFT : 0) |
                (gp.buttons & io::GamePadState::Button::RIGHT ? RIGHT : 0) |
                (gp.buttons & io::GamePadState::Button::UP ? UP : 0) |
                (gp.buttons & io::GamePadState::Button::DOWN ? DOWN : 0) |
                (gp.buttons & io::GamePadState::Button::A ? A : 0) |
                (gp.buttons & io::GamePadState::Button::B ? B : 0) |
                (gp.buttons & io::GamePadState::Button::SELECT ? SELECT : 0) |
                (gp.buttons & io::GamePadState::Button::START ? START : 0) |
                0;
#if NES_PIN_CLK != -1
        // When USB controller is connected both NES ports act as controller 2
        if (usbConnected)
        {          
            if (i == 1)
            {
                v = v | nespad_states[1] | nespad_states[0];
            }
        }
        else
        {
            v |= nespad_states[i];
        }
#endif

// When USB controller is connected  wiipad acts as controller 2 
#if WII_PIN_SDA >= 0 and WII_PIN_SCL >= 0
        if (usbConnected)
        {
            if (i == 1)
            {
                v |= wiipad_read();
            }
        }
        else // if no USB controller is connected, wiipad acts as controller 1
        {
            if (i == 0)
            {
                v |= wiipad_read();
            }
        }
#endif

        int rv = v;
        if (rapidFireCounter & 2)
        {
            // 15 fire/sec
            rv &= ~rapidFireMask[i];
        }

        dst = rv;

        auto p1 = v;

        auto pushed = v & ~prevButtons[i];

        // Toggle frame rate
        if (p1 & START)
        {
            if (pushed & A)
            {
                fps_enabled = !fps_enabled;
            }
        }
        if (p1 & SELECT)
        {
            if (pushed & START)
            {
                nvram_save();
                reset = true;
            }
            if (pushed & A)
            {
                rapidFireMask[i] ^= io::GamePadState::Button::A;
            }
            if (pushed & B)
            {
                rapidFireMask[i] ^= io::GamePadState::Button::B;
            }
            if (pushed & UP)
            {
                scaleMode8_7_ = Frens::screenMode(-1);
            }
            else if (pushed & DOWN)
            {
                scaleMode8_7_ = Frens::screenMode(+1);
            }
        }

        prevButtons[i] = v;
    }

    *pdwSystem = reset ? PAD_SYS_QUIT : 0;
}

void InfoNES_MessageBox(const char *pszMsg, ...)
{
    printf("[MSG]");
    va_list args;
    va_start(args, pszMsg);
    vprintf(pszMsg, args);
    va_end(args);
    printf("\n");
}

void InfoNES_Error(const char *pszMsg, ...)
{

    printf("[Error]");
    va_list args;
    va_start(args, pszMsg);
    vsnprintf(ErrorMessage, ERRORMESSAGESIZE, pszMsg, args);
    printf("%s", ErrorMessage);
    va_end(args);
    printf("\n");
}
bool parseROM(const uint8_t *nesFile)
{
    memcpy(&NesHeader, nesFile, sizeof(NesHeader));
    if (!checkNESMagic(NesHeader.byID))
    {
        return false;
    }

    nesFile += sizeof(NesHeader);

    if (NesHeader.byInfo1 & 4)
    {
        memcpy(&SRAM[0x1000], nesFile, 512);
        nesFile += 512;
    }

    auto romSize = NesHeader.byRomSize * 0x4000;
    ROM = (BYTE *)nesFile;
    nesFile += romSize;

    if (NesHeader.byVRomSize > 0)
    {
        auto vromSize = NesHeader.byVRomSize * 0x2000;
        VROM = (BYTE *)nesFile;
        nesFile += vromSize;
    }

    return true;
}

void InfoNES_ReleaseRom()
{
    ROM = nullptr;
    VROM = nullptr;
}

extern WORD PC;

int InfoNES_LoadFrame()
{
#if NES_PIN_CLK != -1
    nespad_read_start();
#endif
    auto count = dvi_->getFrameCounter();
    auto onOff = hw_divider_s32_quotient_inlined(count, 60) & 1;
    Frens::blinkLed(onOff);
#if NES_PIN_CLK != -1
    nespad_read_finish(); // Sets global nespad_state var
#endif
    tuh_task();
    // Frame rate calculation
    if (fps_enabled)
    {
        // calculate fps and round to nearest value (instead of truncating/floor)
        uint32_t tick_us = Frens::time_us() - start_tick_us;
        fps = (1000000 - 1) / tick_us + 1;
        start_tick_us = Frens::time_us();
    }
    return count;
}

namespace
{
    dvi::DVI::LineBuffer *currentLineBuffer_{};
}

void __not_in_flash_func(drawWorkMeterUnit)(int timing,
                                            [[maybe_unused]] int span,
                                            uint32_t tag)
{
    if (timing >= 0 && timing < 640)
    {
        auto p = currentLineBuffer_->data();
        p[timing] = tag; // tag = color
    }
}

void __not_in_flash_func(drawWorkMeter)(int line)
{
    if (!currentLineBuffer_)
    {
        return;
    }

    memset(currentLineBuffer_->data(), 0, 64);
    memset(&currentLineBuffer_->data()[320 - 32], 0, 64);
    (*currentLineBuffer_)[160] = 0;
    if (line == 4)
    {
        for (int i = 1; i < 10; ++i)
        {
            (*currentLineBuffer_)[16 * i] = 31;
        }
    }

    constexpr uint32_t clocksPerLine = 800 * 10;
    constexpr uint32_t meterScale = 160 * 65536 / (clocksPerLine * 2);
    util::WorkMeterEnum(meterScale, 1, drawWorkMeterUnit);
    //    util::WorkMeterEnum(160, clocksPerLine * 2, drawWorkMeterUnit);
}

void __not_in_flash_func(InfoNES_PreDrawLine)(int line)
{

    util::WorkMeterMark(0xaaaa);
    auto b = dvi_->getLineBuffer();
    util::WorkMeterMark(0x5555);
    // b.size --> 640
    // printf("Pre Draw%d\n", b->size());
    // WORD = 2 bytes
    // b->size = 640
    // printf("%d\n", b->size());
    InfoNES_SetLineBuffer(b->data() + INFONES_LINE_BUFFER_OFFSET, b->size());
    //    (*b)[319] = line + dvi_->getFrameCounter();

    currentLineBuffer_ = b;
}

void __not_in_flash_func(InfoNES_PostDrawLine)(int line)
{
#if !defined(NDEBUG)
    util::WorkMeterMark(0xffff);
    drawWorkMeter(line);
#endif
    // Display frame rate
    if (fps_enabled && line >= 8 && line < 16)
    {
        char fpsString[2];
        WORD *fpsBuffer = currentLineBuffer_->data() + 40;
        WORD fgc = NesPalette[48];
        WORD bgc = NesPalette[15];
        fpsString[0] = '0' + (fps / 10);
        fpsString[1] = '0' + (fps % 10);

        int rowInChar = line % 8;
        for (auto i = 0; i < 2; i++)
        {
            char firstFpsDigit = fpsString[i];
            char fontSlice = getcharslicefrom8x8font(firstFpsDigit, rowInChar);
            for (auto bit = 0; bit < 8; bit++)
            {
                if (fontSlice & 1)
                {
                    *fpsBuffer++ = fgc;
                }
                else
                {
                    *fpsBuffer++ = bgc;
                }
                fontSlice >>= 1;
            }
        }
    }

    assert(currentLineBuffer_);
    dvi_->setLineBuffer(line-4, currentLineBuffer_);
    currentLineBuffer_ = nullptr;
}

bool loadAndReset()
{
    auto rom = romSelector_.getCurrentROM();
    if (!rom)
    {
        printf("ROM does not exists.\n");
        return false;
    }

    if (!parseROM(rom))
    {
        printf("NES file parse error.\n");
        return false;
    }
    if (!nvram_load())
    {
        printf("NVRAM load failed.\n");
    }

    if (InfoNES_Reset() < 0)
    {
        printf("NES reset error.\n");
        return false;
    }

    return true;
}

int InfoNES_Menu()
{
    // InfoNES_Main() のループで最初に呼ばれる
    return loadAndReset() ? 0 : -1;
    // return 0;
}

int main()
{
    char selectedRom[FF_MAX_LFN];
    romName = selectedRom;
    ErrorMessage[0] = selectedRom[0] = 0;

    vreg_set_voltage(VREG_VOLTAGE_1_20);
    sleep_ms(10);
    set_sys_clock_khz(CPUFreqKHz, true);
    sleep_ms(100);
    // Get the processor sys_clk frequency in Hz
     uint32_t freq = clock_get_hz(clk_sys);

    // clk_peri does not have a divider, so input and output frequencies will be the same
    clock_configure(clk_peri,
                        0,
                        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                        freq,
                        freq);

    stdio_init_all();
    printf("Start program\n");
    printf("CPU freq: %d\n", clock_get_hz(clk_sys));
    printf("Starting Tinyusb subsystem\n");
    tusb_init();
#if NES_MAPPER_5_ENABLED == 1
    printf("Mapper 5 is enabled\n");
#else
    printf("Mapper 5 is disabled\n");
#endif
    isFatalError =  !Frens::initAll(selectedRom, CPUFreqKHz, 4, 4 );
    scaleMode8_7_ = Frens::applyScreenMode(settings.screenMode);
    bool showSplash = true;

    // Initialize NVRAM module after ROMSelector is likely initialized within initAll or similar
    // We pass the global romName and ErrorMessage buffers
    nvram_init(&romSelector_, romName, ErrorMessage);

#if defined(BOOT_FROM_FLASH_ROM) && defined(STATIC_ROM_IN_FLASH)
    printf("BOOT_FROM_FLASH_ROM & STATIC_ROM_IN_FLASH: Booting directly into ROM.\n");
    strcpy(selectedRom, "STATIC_FLASH_ROM"); // Placeholder name
    // ROM is already in flash, skip menu and SD card interactions.
    // Proceed directly to InfoNES_Main
    printf("Now playing: %s\n", selectedRom);
    romSelector_.init(ROM_FILE_ADDR); // ROM_FILE_ADDR should be set by FrensHelpers::initAll
    InfoNES_Main();
    // After InfoNES_Main finishes (e.g., user quits game), what should happen?
    // For now, we'll just enter an infinite loop to prevent further execution.
    // Or, perhaps, a watchdog reboot if that's preferred.
    printf("Game session ended. Halting system.\n");
    watchdog_enable(100, 1);
    while(1) { tight_loop_contents(); };
#else
    // Original program flow
    while (true)
    {
        if (strlen(selectedRom) == 0)
        {
            menu("Pico-InfoNES+", ErrorMessage, isFatalError, showSplash, ".nes"); // never returns, but reboots upon selecting a game
        }
        // romName is now set by menu() or was set previously if returning from InfoNES_Main
        // Re-initialize NVRAM in case romName changed (selected via menu)

        nvram_init(&romSelector_, romName, ErrorMessage); // Pass dependencies


        // printf("Now playing: %s\n", selectedRom);
        romSelector_.init(ROM_FILE_ADDR);
        InfoNES_Main();
        selectedRom[0] = 0;
        showSplash = false;
    }
#endif

    return 0;
}
