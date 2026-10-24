#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"
#include "gui/DarthLookAndFeel.h"

class DarthPhaserEditor final : public juce::AudioProcessorEditor
{
public:
    /** Logical panel size; the editor scales this to fit whatever the host gives it. */
    static constexpr int designWidth  = 960;
    static constexpr int designHeight = 600;

    explicit DarthPhaserEditor (DarthPhaserProcessor&);
    ~DarthPhaserEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class Content;

    DarthPhaserProcessor& processorRef;
    DarthLookAndFeel lookAndFeel;

    std::unique_ptr<Content> content;
    juce::ComponentBoundsConstrainer constrainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DarthPhaserEditor)
};
