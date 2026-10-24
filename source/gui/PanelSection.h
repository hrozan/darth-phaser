#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/** Bevelled panel with the amber title bar used by every section. */
class PanelSection final : public juce::Component
{
public:
    static constexpr int headerHeight = 23;

    explicit PanelSection (const juce::String& title, bool showLed = true);

    void paint (juce::Graphics&) override;

    juce::Rectangle<int> getContentBounds() const;
    juce::Rectangle<int> getHeaderBounds() const;

private:
    juce::String titleText;
    bool led;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelSection)
};
