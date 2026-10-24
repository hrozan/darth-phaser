#include "Parameters.h"

namespace
{
juce::String hzString (float v, int)
{
    return v < 1000.0f ? juce::String (v, v < 10.0f ? 2 : 1) + " Hz"
                       : juce::String (v / 1000.0f, 2) + " kHz";
}

juce::String percentString (float v, int) { return juce::String (juce::roundToInt (v)) + " %"; }
juce::String degreeString  (float v, int) { return juce::String (juce::roundToInt (v)) + juce::String (juce::CharPointer_UTF8 ("\xc2\xb0")); }
juce::String dbString      (float v, int) { return juce::String (v, 1) + " dB"; }
}

juce::AudioProcessorValueTreeState::ParameterLayout Params::createLayout()
{
    using APF   = juce::AudioParameterFloat;
    using APC   = juce::AudioParameterChoice;
    using APB   = juce::AudioParameterBool;
    using Range = juce::NormalisableRange<float>;

    constexpr int v = 1; // parameter version hint

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // ---- modulation ----
    auto rateRange = Range { 0.01f, 10.0f, 0.0f, 0.3f };
    layout.add (std::make_unique<APF> (juce::ParameterID { ParamID::rate, v }, "Speed",
                                       rateRange, 1.42f,
                                       juce::AudioParameterFloatAttributes().withStringFromValueFunction (hzString)));

    layout.add (std::make_unique<APB> (juce::ParameterID { ParamID::sync, v }, "Tempo Sync", false));

    layout.add (std::make_unique<APC> (juce::ParameterID { ParamID::division, v }, "Division",
                                       juce::StringArray { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/4T", "1/8T" },
                                       3));

    layout.add (std::make_unique<APF> (juce::ParameterID { ParamID::depth, v }, "Depth",
                                       Range { 0.0f, 100.0f, 0.1f }, 78.0f,
                                       juce::AudioParameterFloatAttributes().withStringFromValueFunction (percentString)));

    layout.add (std::make_unique<APF> (juce::ParameterID { ParamID::stereoPhase, v }, "Stereo Phase",
                                       Range { 0.0f, 180.0f, 1.0f }, 120.0f,
                                       juce::AudioParameterFloatAttributes().withStringFromValueFunction (degreeString)));

    layout.add (std::make_unique<APC> (juce::ParameterID { ParamID::waveform, v }, "Waveform",
                                       juce::StringArray { "Sine", "Triangle", "Square", "Random" }, 0));

    // ---- phase engine ----
    layout.add (std::make_unique<APC> (juce::ParameterID { ParamID::stages, v }, "Stages",
                                       juce::StringArray { "2", "4", "6", "8", "12" }, 3));

    layout.add (std::make_unique<APF> (juce::ParameterID { ParamID::centre, v }, "Centre",
                                       Range { 80.0f, 5000.0f, 0.0f, 0.3f }, 620.0f,
                                       juce::AudioParameterFloatAttributes().withStringFromValueFunction (hzString)));

    layout.add (std::make_unique<APF> (juce::ParameterID { ParamID::spread, v }, "Spread",
                                       Range { 0.0f, 100.0f, 0.1f }, 42.0f,
                                       juce::AudioParameterFloatAttributes().withStringFromValueFunction (percentString)));

    layout.add (std::make_unique<APF> (juce::ParameterID { ParamID::feedback, v }, "Feedback",
                                       Range { -95.0f, 95.0f, 0.1f }, 65.0f,
                                       juce::AudioParameterFloatAttributes().withStringFromValueFunction (percentString)));

    layout.add (std::make_unique<APF> (juce::ParameterID { ParamID::damping, v }, "Damping",
                                       Range { 0.0f, 100.0f, 0.1f }, 30.0f,
                                       juce::AudioParameterFloatAttributes().withStringFromValueFunction (percentString)));

    // ---- output ----
    layout.add (std::make_unique<APF> (juce::ParameterID { ParamID::mix, v }, "Mix",
                                       Range { 0.0f, 100.0f, 0.1f }, 50.0f,
                                       juce::AudioParameterFloatAttributes().withStringFromValueFunction (percentString)));

    layout.add (std::make_unique<APF> (juce::ParameterID { ParamID::level, v }, "Level",
                                       Range { -24.0f, 12.0f, 0.1f }, -3.0f,
                                       juce::AudioParameterFloatAttributes().withStringFromValueFunction (dbString)));

    layout.add (std::make_unique<APB> (juce::ParameterID { ParamID::bypass, v }, "Bypass", false));

    return layout;
}
