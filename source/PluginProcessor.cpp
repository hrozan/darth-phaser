#include "PluginProcessor.h"

#include "Parameters.h"
#include "PluginEditor.h"

namespace
{
constexpr auto stateIdentifier = "DarthPhaserState";
}

DarthPhaserProcessor::DarthPhaserProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, juce::Identifier (stateIdentifier), Params::createLayout())
{
    rateParam        = parameters.getRawParameterValue (ParamID::rate);
    syncParam        = parameters.getRawParameterValue (ParamID::sync);
    divisionParam    = parameters.getRawParameterValue (ParamID::division);
    depthParam       = parameters.getRawParameterValue (ParamID::depth);
    stereoPhaseParam = parameters.getRawParameterValue (ParamID::stereoPhase);
    waveformParam    = parameters.getRawParameterValue (ParamID::waveform);
    stagesParam      = parameters.getRawParameterValue (ParamID::stages);
    centreParam      = parameters.getRawParameterValue (ParamID::centre);
    spreadParam      = parameters.getRawParameterValue (ParamID::spread);
    feedbackParam    = parameters.getRawParameterValue (ParamID::feedback);
    dampingParam     = parameters.getRawParameterValue (ParamID::damping);
    mixParam         = parameters.getRawParameterValue (ParamID::mix);
    levelParam       = parameters.getRawParameterValue (ParamID::level);
    bypassValue      = parameters.getRawParameterValue (ParamID::bypass);
    bypassParam      = parameters.getParameter (ParamID::bypass);
}

PhaserEngine::Settings DarthPhaserProcessor::gatherSettings() const
{
    PhaserEngine::Settings s;

    s.rateHz = SyncMath::lfoRateHz (syncParam->load() > 0.5f,
                                    juce::roundToInt (divisionParam->load()),
                                    rateParam->load(),
                                    hostBpm);

    // Choice parameters come back as floats; rounding avoids truncating 2.9999 to 2.
    const auto stageIndex = juce::jlimit (0, static_cast<int> (Params::stageCounts.size()) - 1,
                                          juce::roundToInt (stagesParam->load()));

    s.depth       = depthParam->load() * 0.01f;
    s.stereoPhase = stereoPhaseParam->load() / 360.0f;
    s.waveform    = static_cast<PhaserEngine::Waveform> (juce::jlimit (0, 3, juce::roundToInt (waveformParam->load())));
    s.stages      = Params::stageCounts[static_cast<size_t> (stageIndex)];
    s.centreHz    = centreParam->load();
    s.spread      = spreadParam->load() * 0.01f;
    s.feedback    = feedbackParam->load() * 0.01f;
    s.damping     = dampingParam->load() * 0.01f;
    s.mix         = mixParam->load() * 0.01f;
    s.levelGain   = juce::Decibels::decibelsToGain (levelParam->load());

    return s;
}

void DarthPhaserProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);

    engine.setSettings (gatherSettings());
    engine.prepare (sampleRate, juce::jmax (getTotalNumInputChannels(), getTotalNumOutputChannels()));

    for (auto& p : outputPeak)
        p.store (0.0f, std::memory_order_relaxed);
}

void DarthPhaserProcessor::releaseResources()
{
    engine.reset();
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

    if (auto* playHead = getPlayHead())
        if (const auto position = playHead->getPosition())
            if (const auto bpm = position->getBpm())
                hostBpm = *bpm;

    const auto bypassed = bypassValue != nullptr && bypassValue->load() > 0.5f;

    if (bypassed)
        engine.reset();
    else
    {
        const auto settings = gatherSettings();
        effectiveRate.store (settings.rateHz, std::memory_order_relaxed);

        engine.setSettings (settings);
        engine.process (buffer.getArrayOfWritePointers(),
                        juce::jmin (buffer.getNumChannels(), PhaserEngine::maxChannels),
                        buffer.getNumSamples());
    }

    for (size_t c = 0; c < outputPeak.size(); ++c)
    {
        const auto source = juce::jmin (static_cast<int> (c), buffer.getNumChannels() - 1);
        const auto peak = source >= 0 ? buffer.getMagnitude (source, 0, buffer.getNumSamples()) : 0.0f;
        outputPeak[c].store (peak, std::memory_order_relaxed);
    }
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
