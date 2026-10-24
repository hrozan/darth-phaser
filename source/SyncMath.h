#pragma once

#include <algorithm>
#include <array>
#include <cstddef>

/** Tempo-sync arithmetic, kept free of JUCE so it can be tested on its own. */
namespace SyncMath
{
/** Cycle length in beats for each entry of the division selector. */
inline constexpr std::array<double, 3> divisionBeats
{
    2.0,  // 1/2
    1.0,  // 1/4
    0.5   // 1/8
};

inline constexpr int numDivisions = static_cast<int> (divisionBeats.size());

/** The rate the LFO should actually run at: free-running, or the host tempo divided down. */
inline float lfoRateHz (bool sync, int divisionIndex, float freeRateHz, double hostBpm) noexcept
{
    if (! sync)
        return freeRateHz;

    const auto index = std::clamp (divisionIndex, 0, numDivisions - 1);
    const auto beats = divisionBeats[static_cast<std::size_t> (index)];
    const auto bpm = hostBpm > 0.0 ? hostBpm : 120.0;

    return static_cast<float> (bpm / 60.0 / beats);
}
}
