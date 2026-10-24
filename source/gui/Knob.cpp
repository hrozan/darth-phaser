#include "Knob.h"

#include "Theme.h"

namespace
{
constexpr float rotaryStart = juce::MathConstants<float>::pi * 1.25f;
constexpr float rotaryEnd   = juce::MathConstants<float>::pi * 2.75f;
}

Knob::Knob (juce::RangedAudioParameter& parameter, const juce::String& caption,
            juce::Colour accent, bool hero)
    : param (parameter), captionText (caption), isHero (hero),
      attachment (parameter, slider, nullptr)
{
    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters (rotaryStart, rotaryEnd, true);
    slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
    slider.setDoubleClickReturnValue (true, parameter.convertFrom0to1 (parameter.getDefaultValue()));
    slider.setVelocityBasedMode (false);
    slider.setTooltip (parameter.getName (64));

    slider.onValueChange = [this] { refreshReadout(); repaint(); };
    addAndMakeVisible (slider);

    refreshReadout();
}

void Knob::refreshReadout()
{
    // Derive the text from the slider value rather than the parameter, so it is
    // correct regardless of listener ordering during a drag.
    const auto normalised = param.convertTo0to1 (static_cast<float> (slider.getValue()));
    readout = param.getText (normalised, 24);
}

void Knob::resized()
{
    auto area = getLocalBounds();
    area.removeFromBottom (isHero ? 34 : 30);
    slider.setBounds (area);
}

void Knob::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();
    auto footer = area.removeFromBottom (isHero ? 34 : 30);

    auto valueRow   = footer.removeFromBottom (isHero ? 18 : 15);
    auto captionRow = footer;

    g.setColour (isHero ? Theme::text.withAlpha (0.78f) : Theme::textDim);
    g.setFont (Theme::label (isHero ? 11.0f : 10.0f));
    g.drawText (captionText, captionRow, juce::Justification::centredTop, false);

    g.setColour (Theme::amberBright);
    g.setFont (Theme::label (isHero ? 15.0f : 12.0f, true, 0.06f));
    g.drawText (readout, valueRow, juce::Justification::centredTop, false);
}
