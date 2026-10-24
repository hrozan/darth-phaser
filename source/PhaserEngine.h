#pragma once

#include <array>
#include <cstdint>

/**
    Cascaded first-order all-pass phaser.

    Deliberately free of JUCE types so it can be compiled and tested on its own.
    process() works in place on raw channel pointers.
*/
class PhaserEngine
{
public:
    static constexpr int maxStages   = 12;
    static constexpr int maxChannels = 2;

    enum class Waveform { sine = 0, triangle, square, random };

    struct Settings
    {
        float    rateHz      = 1.42f;   // LFO speed
        float    depth       = 0.78f;   // 0..1, sweep width
        float    stereoPhase = 0.0f;    // cycles, 0..0.5 (0..180 degrees)
        Waveform waveform    = Waveform::sine;
        int      stages      = 8;       // even, 2..maxStages
        float    centreHz    = 620.0f;  // sweep centre
        float    spread      = 0.42f;   // 0..1, spacing of the notches
        float    feedback    = 0.0f;    // -0.95..0.95, negative inverts
        float    damping     = 0.3f;    // 0..1, low-pass in the feedback path
        float    mix         = 0.5f;    // 0..1, 0.5 gives the deepest notches
        float    levelGain   = 1.0f;    // linear output gain
    };

    void prepare (double sampleRate, int numChannels);
    void reset();

    void setSettings (const Settings& s) noexcept { target = s; }

    /** In-place. `channels` must hold at least `numChannels` valid pointers. */
    void process (float* const* channels, int numChannels, int numSamples) noexcept;

private:
    // Coefficients are refreshed every `controlInterval` samples rather than per
    // sample: a tan() per stage per sample is wasteful and inaudible at LFO rates.
    static constexpr int controlInterval = 16;

    struct Allpass
    {
        float a1 = 0.0f, z = 0.0f;

        inline float process (float x) noexcept
        {
            const auto y = a1 * x + z;
            z = x - a1 * y;
            return y;
        }

        void reset() noexcept { z = 0.0f; }
    };

    struct Smoothed
    {
        float value = 0.0f, coef = 0.2f;

        void snap (float v) noexcept { value = v; }
        inline float step (float t) noexcept { value += coef * (t - value); return value; }
    };

    struct Channel
    {
        std::array<Allpass, maxStages> ap {};
        float fbSample  = 0.0f;
        float dampState = 0.0f;
        float lastPhase = 0.0f;
        float randValue = 0.0f;
    };

    float lfoValue (Waveform, float phase, Channel&) noexcept;
    float allpassCoef (float frequencyHz) const noexcept;
    float nextRandom() noexcept;

    double sampleRate = 44100.0;
    int    activeChannels = 2;
    int    currentStages = 8;

    std::array<Channel, maxChannels> channels {};

    Settings target {};
    Smoothed smDepth, smCentre, smSpread, smFeedback, smDamping, smMix, smLevel;

    float    phase = 0.0f;          // shared LFO phase, 0..1
    uint32_t rngState = 0x9e3779b9; // deterministic across runs
};
