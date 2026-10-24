#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class DarthPhaserProcessor;

/**
    Two linked displays in one window.

    Upper area: the live magnitude response of the all-pass cascade against the dry
    path, so the notches really are where the plugin is putting them. Lower strip:
    the LFO waveform scrolling under a fixed playhead.
*/
class LfoScope final : public juce::Component, private juce::Timer
{
public:
    explicit LfoScope (DarthPhaserProcessor&);

    void paint (juce::Graphics&) override;

private:
    void timerCallback() override { repaint(); }

    void paintGrid (juce::Graphics&, juce::Rectangle<float>);
    void paintResponse (juce::Graphics&, juce::Rectangle<float>);
    void paintLfo (juce::Graphics&, juce::Rectangle<float>);

    DarthPhaserProcessor& proc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfoScope)
};
