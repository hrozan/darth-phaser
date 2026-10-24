#include "Presets.h"

#include "Parameters.h"

const std::vector<Presets::Preset>& Presets::factory()
{
    // Waveform: 0 sine, 1 triangle, 2 square, 3 random.
    // Stages:   0 = 2, 1 = 4, 2 = 6, 3 = 8, 4 = 12.
    static const std::vector<Preset> presets
    {
        { "IMPERIAL SWEEP", {
            { ParamID::rate, 1.42f }, { ParamID::sync, 0.0f }, { ParamID::depth, 78.0f },
            { ParamID::stereoPhase, 120.0f }, { ParamID::waveform, 0.0f }, { ParamID::stages, 3.0f },
            { ParamID::centre, 620.0f }, { ParamID::spread, 42.0f }, { ParamID::feedback, 65.0f },
            { ParamID::damping, 30.0f }, { ParamID::mix, 50.0f }, { ParamID::level, -3.0f } } },

        { "VADER'S BREATH", {
            { ParamID::rate, 0.18f }, { ParamID::sync, 0.0f }, { ParamID::depth, 95.0f },
            { ParamID::stereoPhase, 60.0f }, { ParamID::waveform, 0.0f }, { ParamID::stages, 4.0f },
            { ParamID::centre, 240.0f }, { ParamID::spread, 70.0f }, { ParamID::feedback, 82.0f },
            { ParamID::damping, 72.0f }, { ParamID::mix, 50.0f }, { ParamID::level, -4.0f } } },

        { "TIE FIGHTER", {
            { ParamID::rate, 7.5f }, { ParamID::sync, 0.0f }, { ParamID::depth, 100.0f },
            { ParamID::stereoPhase, 180.0f }, { ParamID::waveform, 2.0f }, { ParamID::stages, 1.0f },
            { ParamID::centre, 1800.0f }, { ParamID::spread, 20.0f }, { ParamID::feedback, 90.0f },
            { ParamID::damping, 5.0f }, { ParamID::mix, 55.0f }, { ParamID::level, -6.0f } } },

        { "DEATH STAR HUM", {
            { ParamID::rate, 0.05f }, { ParamID::sync, 0.0f }, { ParamID::depth, 45.0f },
            { ParamID::stereoPhase, 20.0f }, { ParamID::waveform, 1.0f }, { ParamID::stages, 0.0f },
            { ParamID::centre, 110.0f }, { ParamID::spread, 0.0f }, { ParamID::feedback, -70.0f },
            { ParamID::damping, 88.0f }, { ParamID::mix, 50.0f }, { ParamID::level, -2.0f } } },

        { "SITH CHOIR", {
            { ParamID::rate, 0.75f }, { ParamID::sync, 1.0f }, { ParamID::division, 2.0f },
            { ParamID::depth, 70.0f }, { ParamID::stereoPhase, 150.0f }, { ParamID::waveform, 1.0f },
            { ParamID::stages, 3.0f }, { ParamID::centre, 900.0f }, { ParamID::spread, 85.0f },
            { ParamID::feedback, 55.0f }, { ParamID::damping, 40.0f }, { ParamID::mix, 50.0f },
            { ParamID::level, -3.0f } } },

        { "HYPERDRIVE", {
            { ParamID::rate, 4.2f }, { ParamID::sync, 0.0f }, { ParamID::depth, 100.0f },
            { ParamID::stereoPhase, 90.0f }, { ParamID::waveform, 3.0f }, { ParamID::stages, 2.0f },
            { ParamID::centre, 1400.0f }, { ParamID::spread, 100.0f }, { ParamID::feedback, -88.0f },
            { ParamID::damping, 15.0f }, { ParamID::mix, 50.0f }, { ParamID::level, -5.0f } } },
    };

    return presets;
}

void Presets::apply (juce::AudioProcessorValueTreeState& state, int index)
{
    const auto& all = factory();

    if (! juce::isPositiveAndBelow (index, static_cast<int> (all.size())))
        return;

    for (const auto& [id, value] : all[static_cast<size_t> (index)].values)
        if (auto* p = state.getParameter (id))
            p->setValueNotifyingHost (p->convertTo0to1 (value));
}
