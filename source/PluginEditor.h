#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"

class DarthPhaserEditor final : public juce::AudioProcessorEditor
{
public:
    explicit DarthPhaserEditor (DarthPhaserProcessor&);
    ~DarthPhaserEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    static constexpr int headerHeight = 72;

    DarthPhaserProcessor& processorRef;

    // Placeholder until the designed panel in design/ is built: generic controls so
    // the DSP can be driven from a host today.
    juce::GenericAudioProcessorEditor controls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DarthPhaserEditor)
};
