#include "PhaserEngine.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr float pi = 3.14159265358979323846f;

// Full depth sweeps the notches +/- 2 octaves around the centre frequency.
constexpr float sweepOctaves = 2.0f;

// Full spread fans the stages out across +/- 1 octave.
constexpr float spreadOctaves = 2.0f;

inline float wrap01 (float x) noexcept
{
    x -= std::floor (x);
    return x;
}

inline float softClip (float x) noexcept
{
    return std::tanh (x);
}
}

void PhaserEngine::prepare (double newSampleRate, int numChannels)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    activeChannels = std::clamp (numChannels, 1, maxChannels);

    // One-pole smoothers running at control rate; ~20 ms to settle.
    const auto blocksPerSecond = static_cast<float> (sampleRate) / static_cast<float> (controlInterval);
    const auto coef = 1.0f - std::exp (-1.0f / (0.02f * blocksPerSecond));

    for (auto* s : { &smDepth, &smCentre, &smSpread, &smFeedback, &smDamping, &smMix, &smLevel })
        s->coef = std::clamp (coef, 0.0f, 1.0f);

    reset();

    smDepth   .snap (target.depth);
    smCentre  .snap (target.centreHz);
    smSpread  .snap (target.spread);
    smFeedback.snap (target.feedback);
    smDamping .snap (target.damping);
    smMix     .snap (target.mix);
    smLevel   .snap (target.levelGain);
}

void PhaserEngine::reset()
{
    for (auto& ch : channels)
    {
        for (auto& stage : ch.ap)
            stage.reset();

        ch.fbSample  = 0.0f;
        ch.dampState = 0.0f;
        ch.lastPhase = 0.0f;
        ch.randValue = 0.0f;
    }

    phase = 0.0f;
}

float PhaserEngine::nextRandom() noexcept
{
    // xorshift32, then map to 0..1
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return static_cast<float> (rngState & 0xffffff) / static_cast<float> (0xffffff);
}

float PhaserEngine::lfoValue (Waveform waveform, float ph, Channel& ch) noexcept
{
    // Sample and hold refreshes when this channel's phase wraps.
    if (ph < ch.lastPhase)
        ch.randValue = nextRandom();

    ch.lastPhase = ph;

    switch (waveform)
    {
        case Waveform::sine:     return 0.5f - 0.5f * std::cos (2.0f * pi * ph);
        case Waveform::triangle: return 1.0f - std::fabs (2.0f * ph - 1.0f);
        case Waveform::square:   return ph < 0.5f ? 1.0f : 0.0f;
        case Waveform::random:   return ch.randValue;
    }

    return 0.5f;
}

float PhaserEngine::allpassCoef (float frequencyHz) const noexcept
{
    const auto nyquist = static_cast<float> (sampleRate) * 0.5f;
    const auto fc = std::clamp (frequencyHz, 20.0f, nyquist * 0.9f);
    const auto t  = std::tan (pi * fc / static_cast<float> (sampleRate));

    return (t - 1.0f) / (t + 1.0f);
}

void PhaserEngine::process (float* const* data, int numChannels, int numSamples) noexcept
{
    const auto nch = std::clamp (numChannels, 0, maxChannels);

    if (nch == 0 || numSamples <= 0)
        return;

    // Stage count is stepped rather than smoothed; clear the stages coming into use
    // so they do not dump stale state into the signal.
    const auto stages = std::clamp (target.stages, 1, maxStages);

    if (stages != currentStages)
    {
        for (int c = 0; c < maxChannels; ++c)
            for (int s = currentStages; s < stages; ++s)
                channels[static_cast<size_t> (c)].ap[static_cast<size_t> (s)].reset();

        currentStages = stages;
    }

    const auto phaseInc = static_cast<float> (std::max (0.0f, target.rateHz) / sampleRate);

    for (int start = 0; start < numSamples; start += controlInterval)
    {
        const auto n = std::min (controlInterval, numSamples - start);

        // ---- control rate ----
        const auto depth    = smDepth   .step (target.depth);
        const auto centre   = smCentre  .step (target.centreHz);
        const auto spread   = smSpread  .step (target.spread);
        const auto feedback = std::clamp (smFeedback.step (target.feedback), -0.95f, 0.95f);
        const auto damping  = smDamping .step (target.damping);
        const auto mix      = smMix     .step (target.mix);
        const auto level    = smLevel   .step (target.levelGain);

        // Damping sets a low-pass in the feedback path: 20 kHz wide open down to 200 Hz.
        const auto dampCutoff = 20000.0f * std::pow (0.01f, std::clamp (damping, 0.0f, 1.0f));
        const auto dampCoef   = std::clamp (1.0f - std::exp (-2.0f * pi * dampCutoff / static_cast<float> (sampleRate)),
                                            0.0f, 1.0f);

        for (int c = 0; c < nch; ++c)
        {
            auto& ch = channels[static_cast<size_t> (c)];

            const auto chPhase = wrap01 (phase + (c == 1 ? target.stereoPhase : 0.0f));
            const auto bipolar = lfoValue (target.waveform, chPhase, ch) * 2.0f - 1.0f;
            const auto fc      = centre * std::exp2 (bipolar * depth * sweepOctaves);

            if (c == 0)
                uiCentre.store (fc, std::memory_order_relaxed);

            for (int s = 0; s < stages; ++s)
            {
                const auto frac = stages > 1 ? static_cast<float> (s) / static_cast<float> (stages - 1) - 0.5f
                                             : 0.0f;
                const auto mul  = std::exp2 (spread * spreadOctaves * frac);

                ch.ap[static_cast<size_t> (s)].a1 = allpassCoef (fc * mul);
            }
        }

        // ---- audio rate ----
        for (int k = 0; k < n; ++k)
        {
            const auto i = start + k;

            for (int c = 0; c < nch; ++c)
            {
                auto& ch = channels[static_cast<size_t> (c)];

                const auto dry = data[c][i];
                auto v = dry + feedback * ch.fbSample;

                for (int s = 0; s < stages; ++s)
                    v = ch.ap[static_cast<size_t> (s)].process (v);

                ch.dampState += dampCoef * (v - ch.dampState);
                ch.fbSample = softClip (ch.dampState);

                data[c][i] = (dry * (1.0f - mix) + v * mix) * level;
            }

            phase = wrap01 (phase + phaseInc);
        }
    }

    uiPhase.store (phase, std::memory_order_relaxed);
}
