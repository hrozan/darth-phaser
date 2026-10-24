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

/** Entries in the sync selector: index 0 is FREE, 1..numDivisions pick a division. */
inline constexpr int numSyncModes = numDivisions + 1;

inline constexpr bool isSynced (int syncMode) noexcept { return syncMode > 0; }

/**
    The rate the LFO should actually run at.

    One index covers the whole control, so a click is a single parameter change:
    hosts never see a half-applied state where sync is on but the division has
    not landed yet.
*/
inline float lfoRateHz (int syncMode, float freeRateHz, double hostBpm) noexcept
{
    if (! isSynced (syncMode))
        return freeRateHz;

    const auto index = std::clamp (syncMode - 1, 0, numDivisions - 1);
    const auto beats = divisionBeats[static_cast<std::size_t> (index)];
    const auto bpm = hostBpm > 0.0 ? hostBpm : 120.0;

    return static_cast<float> (bpm / 60.0 / beats);
}
}
