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
    DarthPhaserProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DarthPhaserEditor)
};
