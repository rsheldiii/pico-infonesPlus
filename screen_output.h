#ifndef SCREEN_OUTPUT_H
#define SCREEN_OUTPUT_H

#include <vector>
#include <cstdint>
#include <cstddef> // For size_t
#include "pico_lib/util/ring_buffer.h" // Use the lib version

class ScreenOutput {
public:
    using LineBuffer = std::vector<uint16_t>;
    struct BlankSettings { int top = 0; int bottom = 0; };
    struct AudioSample {
        int16_t l, r;
    };

    virtual ~ScreenOutput() = default; // Virtual destructor is important!

    // Core Methods
    virtual LineBuffer* getLineBuffer() = 0;
    virtual void setLineBuffer(int line, LineBuffer *p) = 0;
    virtual void waitForValidLine() = 0;
    virtual uint32_t getFrameCounter() const = 0;

    // Control Methods
    virtual void registerIRQThisCore() = 0;
    virtual void unregisterIRQThisCore() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

    // Configuration Methods
    virtual BlankSettings& getBlankSettings() = 0;
    virtual void setScanLine(bool enable) = 0;

    // Audio Methods (May be stubs in some implementations)
    virtual void setAudioFreq(int freq, int CTS, int N) = 0;
    virtual void allocateAudioBuffer(size_t size) = 0;
    virtual util::RingBuffer<ScreenOutput::AudioSample>& getAudioRingBuffer() = 0;

    // Conversion/Processing Methods (Called by Core 1 loop in DVI version)
    // These might have empty implementations if the logic moves elsewhere (e.g., IRQ handler)
    virtual void convertScanBuffer12bpp() = 0;
    virtual void convertScanBuffer12bppScaled16_7(int srcPixelOfs, int dstPixelOfs, int dstPixels) = 0;
    virtual void convertScanBuffer12bpp(uint16_t line, uint16_t *buffer, size_t size) = 0;
    virtual void convertScanBuffer12bppScaled16_7(int srcPixelOfs, int dstPixelOfs, int dstPixels, uint16_t line, uint16_t *buffer, size_t size) = 0;
};

#endif // SCREEN_OUTPUT_H 