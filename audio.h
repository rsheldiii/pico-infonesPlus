#ifndef AUDIO_H
#define AUDIO_H

#include "InfoNES_Types.h" // For BYTE, WORD etc.

// These are part of the InfoNES system API
// Implemented in audio.cpp

void InfoNES_SoundInit(void);
int InfoNES_SoundOpen(int samples_per_sync, int sample_rate);
void InfoNES_SoundClose(void);
int InfoNES_GetSoundBufferSize();
void InfoNES_SoundOutput(int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5);

#endif // AUDIO_H 