#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace ParamID
{
inline constexpr auto rate        = "rate";
inline constexpr auto sync        = "sync";
inline constexpr auto division    = "division";
inline constexpr auto depth       = "depth";
inline constexpr auto stereoPhase = "stereoPhase";
inline constexpr auto waveform    = "waveform";
inline constexpr auto stages      = "stages";
inline constexpr auto centre      = "centre";
inline constexpr auto spread      = "spread";
inline constexpr auto feedback    = "feedback";
inline constexpr auto damping     = "damping";
inline constexpr auto mix         = "mix";
inline constexpr auto level       = "level";
inline constexpr auto bypass      = "bypass";
}

namespace Params
{
/** Stage counts behind the STAGES selector, in choice order. */
inline constexpr std::array<int, 5> stageCounts { 2, 4, 6, 8, 12 };

/** Cycle length in beats for each entry of the DIVISION selector. */
inline constexpr std::array<double, 8> divisionBeats
{
    4.0,        // 1/1
    2.0,        // 1/2
    1.0,        // 1/4
    0.5,        // 1/8
    0.25,       // 1/16
    0.125,      // 1/32
    1.0 / 1.5,  // 1/4T
    0.5 / 1.5   // 1/8T
};

juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
}
