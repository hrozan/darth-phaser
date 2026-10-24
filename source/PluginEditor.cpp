#include "PluginEditor.h"

DarkPhaserEditor::DarkPhaserEditor (DarkPhaserProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setResizable (true, true);
    setResizeLimits (360, 240, 1600, 1000);
    setSize (560, 360);
}

void DarkPhaserEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff121014));

    g.setColour (juce::Colour (0xffe8e2ea));
    g.setFont (juce::FontOptions (28.0f, juce::Font::bold));
    g.drawText ("DARK PHASER", getLocalBounds().removeFromTop (getHeight() / 2),
                juce::Justification::centredBottom, false);

    g.setColour (juce::Colour (0xff8a7f95));
    g.setFont (juce::FontOptions (14.0f));
    g.drawText (juce::String ("v") + JucePlugin_VersionString,
                getLocalBounds().removeFromBottom (getHeight() / 2).removeFromTop (28),
                juce::Justification::centredTop, false);
}

void DarkPhaserEditor::resized()
{
}
