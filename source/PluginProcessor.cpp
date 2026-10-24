#include "PluginProcessor.h"

#include "PluginEditor.h"

namespace
{
constexpr auto stateIdentifier = "DarkPhaserState";
}

DarkPhaserProcessor::DarkPhaserProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, juce::Identifier (stateIdentifier), createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout DarkPhaserProcessor::createParameterLayout()
{
    // Phaser parameters (rate, depth, centre frequency, feedback, stages, mix)
    // will be declared here once the DSP lands.
    return {};
}

void DarkPhaserProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void DarkPhaserProcessor::releaseResources()
{
}

bool DarkPhaserProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOutput = layouts.getMainOutputChannelSet();
    const auto& mainInput  = layouts.getMainInputChannelSet();

    if (mainOutput != juce::AudioChannelSet::mono() && mainOutput != juce::AudioChannelSet::stereo())
        return false;

    return mainInput == mainOutput;
}

void DarkPhaserProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;

    for (auto channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());

    // Pass-through for now: the phaser is applied here.
}

juce::AudioProcessorEditor* DarkPhaserProcessor::createEditor()
{
    return new DarkPhaserEditor (*this);
}

void DarkPhaserProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void DarkPhaserProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DarkPhaserProcessor();
}
