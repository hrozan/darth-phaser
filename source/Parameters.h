#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "SyncMath.h"

namespace ParamID
{
inline constexpr auto rate        = "rate";
inline constexpr auto syncMode    = "syncMode";
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

/** Labels for the sync selector: FREE followed by each division. */
inline const juce::StringArray syncChoices { "FREE", "1/2", "1/4", "1/8" };

juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
}
