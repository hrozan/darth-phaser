#include "PluginProcessor.h"

#include "PluginEditor.h"

namespace
{
constexpr auto stateIdentifier = "DarthPhaserState";
}

DarthPhaserProcessor::DarthPhaserProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, juce::Identifier (stateIdentifier), createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout DarthPhaserProcessor::createParameterLayout()
{
    // Phaser parameters (rate, depth, centre frequency, feedback, stages, mix)
    // will be declared here once the DSP lands.
    return {};
}

void DarthPhaserProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void DarthPhaserProcessor::releaseResources()
{
}

bool DarthPhaserProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOutput = layouts.getMainOutputChannelSet();
    const auto& mainInput  = layouts.getMainInputChannelSet();

    if (mainOutput != juce::AudioChannelSet::mono() && mainOutput != juce::AudioChannelSet::stereo())
        return false;

    return mainInput == mainOutput;
}

void DarthPhaserProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;

    for (auto channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());

    // Pass-through for now: the phaser is applied here.
}

juce::AudioProcessorEditor* DarthPhaserProcessor::createEditor()
{
    return new DarthPhaserEditor (*this);
}

void DarthPhaserProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void DarthPhaserProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DarthPhaserProcessor();
}
