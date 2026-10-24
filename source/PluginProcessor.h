#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "PhaserEngine.h"
#include "PresetManager.h"

class DarthPhaserProcessor final : public juce::AudioProcessor
{
public:
    DarthPhaserProcessor();
    ~DarthPhaserProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    PresetManager& getPresetManager() { return presetManager; }

    // ---- display taps for the editor ----
    float getLfoPhase() const noexcept { return engine.getLfoPhase(); }
    float getEffectiveRateHz() const noexcept { return effectiveRate.load (std::memory_order_relaxed); }
    float getModulatedCentreHz() const noexcept { return engine.getModulatedCentreHz(); }
    float getOutputPeak (int channel) const noexcept
    {
        return outputPeak[static_cast<size_t> (juce::jlimit (0, 1, channel))].load (std::memory_order_relaxed);
    }

private:
    PhaserEngine::Settings gatherSettings() const;

    juce::AudioProcessorValueTreeState parameters;
    PresetManager presetManager { parameters };
    PhaserEngine engine;

    // Cached raw pointers: looking parameters up by string on the audio thread is wasteful.
    std::atomic<float>* rateParam        = nullptr;
    std::atomic<float>* syncParam        = nullptr;
    std::atomic<float>* divisionParam    = nullptr;
    std::atomic<float>* depthParam       = nullptr;
    std::atomic<float>* stereoPhaseParam = nullptr;
    std::atomic<float>* waveformParam    = nullptr;
    std::atomic<float>* stagesParam      = nullptr;
    std::atomic<float>* centreParam      = nullptr;
    std::atomic<float>* spreadParam      = nullptr;
    std::atomic<float>* feedbackParam    = nullptr;
    std::atomic<float>* dampingParam     = nullptr;
    std::atomic<float>* mixParam         = nullptr;
    std::atomic<float>* levelParam       = nullptr;
    std::atomic<float>* bypassValue      = nullptr;
    juce::AudioProcessorParameter* bypassParam = nullptr;

    double hostBpm = 120.0;

    std::array<std::atomic<float>, 2> outputPeak { };
    std::atomic<float> effectiveRate { 1.42f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DarthPhaserProcessor)
};
