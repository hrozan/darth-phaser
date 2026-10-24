#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class DarthPhaserProcessor;

/** The LFO waveform scrolling under a fixed playhead. */
class LfoScope final : public juce::Component, private juce::Timer
{
public:
    explicit LfoScope (DarthPhaserProcessor&);

    void paint (juce::Graphics&) override;

private:
    void timerCallback() override { repaint(); }

    DarthPhaserProcessor& proc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfoScope)
};
