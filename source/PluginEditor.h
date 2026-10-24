#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"

class DarkPhaserEditor final : public juce::AudioProcessorEditor
{
public:
    explicit DarkPhaserEditor (DarkPhaserProcessor&);
    ~DarkPhaserEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    DarkPhaserProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DarkPhaserEditor)
};
