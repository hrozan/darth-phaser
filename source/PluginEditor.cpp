#include "PluginEditor.h"

DarthPhaserEditor::DarthPhaserEditor (DarthPhaserProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), controls (p)
{
    addAndMakeVisible (controls);

    setResizable (true, true);
    setResizeLimits (420, 420, 1200, 1100);
    setSize (560, 620);
}

void DarthPhaserEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff09090b));

    auto header = getLocalBounds().removeFromTop (headerHeight);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1a1116), header.toFloat().getCentre(),
                                             juce::Colour (0xff09090b), header.toFloat().getBottomLeft(),
                                             true));
    g.fillRect (header);

    g.setColour (juce::Colour (0xffe9e6e2));
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("DARTH PHASER", header.reduced (16, 0).withTrimmedBottom (20),
                juce::Justification::bottomLeft, false);

    g.setColour (juce::Colour (0xffff7a18));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("STEREO PHASE SHIFTER  \xc2\xb7  v" JucePlugin_VersionString,
                header.reduced (16, 0).withTrimmedTop (headerHeight - 26),
                juce::Justification::topLeft, false);

    g.setColour (juce::Colour (0xffff2233).withAlpha (0.55f));
    g.fillRect (header.removeFromBottom (1));
}

void DarthPhaserEditor::resized()
{
    controls.setBounds (getLocalBounds().withTrimmedTop (headerHeight));
}
