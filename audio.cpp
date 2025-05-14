#include "audio.h"
#include "pico/stdlib.h" // Included for __not_in_flash_func if needed, though might be indirect
#include "FrensHelpers.h" // Provides the global dvi_ pointer
#include "screen_output.h"          // Provides the dvi::DVI class definition
#include <algorithm>    // For std::min

// --- Implementation of InfoNES Sound API ---

void InfoNES_SoundInit()
{
    // Currently empty in main.cpp
}

int InfoNES_SoundOpen(int samples_per_sync, int sample_rate)
{
    // Currently returns 0 in main.cpp
    return 0;
}

void InfoNES_SoundClose()
{
    // Currently empty in main.cpp
}

// Place in RAM because it's called often from the sound generation loop
int __not_in_flash_func(InfoNES_GetSoundBufferSize)()
{
#ifdef SPI_SCREEN
    return 0;
#endif
    // Requires dvi_
    if (!dvi_) return 0; // Safety check
    return dvi_->getAudioRingBuffer().getFullWritableSize();
}

// Place in RAM because it's called often from the sound generation loop
void __not_in_flash_func(InfoNES_SoundOutput)(int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5)
{
#ifdef SPI_SCREEN
    return;
#endif
    // Requires dvi_
    if (!dvi_) return; // Safety check

    while (samples)
    {
        auto &ring = dvi_->getAudioRingBuffer();
        auto n = std::min<int>(samples, ring.getWritableSize());
        if (!n)
        {
            // Buffer full, drop samples for now
            // Alternatively, could block, but that might stall the emulator
            return;
        }
        auto p = ring.getWritePointer();

        int ct = n;
        while (ct--)
        {
            // Mixing logic copied from main.cpp
            int w1 = *wave1++;
            int w2 = *wave2++;
            int w3 = *wave3++;
            int w4 = *wave4++;
            int w5 = *wave5++;
            // Original comment: w3 = w2 = w4 = w5 = 0;
            int l = w1 * 6 + w2 * 3 + w3 * 5 + w4 * 3 * 17 + w5 * 2 * 32;
            int r = w1 * 3 + w2 * 6 + w3 * 5 + w4 * 3 * 17 + w5 * 2 * 32;
            *p++ = {static_cast<short>(l), static_cast<short>(r)};
        }

        ring.advanceWritePointer(n);
        samples -= n;
    }
} 