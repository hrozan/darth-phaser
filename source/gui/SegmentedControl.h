#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

/** Row of mutually exclusive buttons: STAGES, WAVEFORM and the sync switch. */
class SegmentedControl final : public juce::Component
{
public:
    SegmentedControl (juce::RangedAudioParameter& parameter, const juce::StringArray& options);
    ~SegmentedControl() override;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    int indexAt (juce::Point<float>) const;

    juce::RangedAudioParameter& param;
    juce::StringArray items;
    int selected = 0;

    juce::ParameterAttachment attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SegmentedControl)
};
