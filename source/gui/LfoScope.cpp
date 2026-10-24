#include "LfoScope.h"

#include "../Parameters.h"
#include "../PluginProcessor.h"
#include "Theme.h"

namespace
{
constexpr float windowCycles = 2.0f;
constexpr float playheadAt   = 0.45f;
constexpr float swing        = 0.40f;

float lfoShape (int waveform, float phase)
{
    phase -= std::floor (phase);

    switch (waveform)
    {
        case 1:  return 1.0f - std::abs (2.0f * phase - 1.0f);
        case 2:  return phase < 0.5f ? 1.0f : 0.0f;
        case 3:
        {
            // stepped values for display; the engine's own sequence is not published
            auto h = static_cast<uint32_t> (std::floor (phase)) * 2654435761u;
            h ^= h >> 15;
            return static_cast<float> (h & 0xffff) / 65535.0f;
        }
        default: return 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * phase);
    }
}
}

LfoScope::LfoScope (DarthPhaserProcessor& processor) : proc (processor)
{
    startTimerHz (30);
}

void LfoScope::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff101016), bounds.getCentre(),
                                             juce::Colour (0xff06060a), bounds.getBottomLeft(), true));
    g.fillRoundedRectangle (bounds, 4.0f);

    const auto r = bounds.reduced (2.0f);

    // sparse grid
    g.setColour (juce::Colours::white.withAlpha (0.04f));
    for (int i = 1; i < 4; ++i)
        g.drawVerticalLine (static_cast<int> (r.getX() + r.getWidth() * static_cast<float> (i) / 4.0f),
                            r.getY(), r.getBottom());

    static constexpr float dashes[] = { 3.0f, 4.0f };
    g.setColour (Theme::amber.withAlpha (0.18f));
    g.drawDashedLine ({ r.getX(), r.getCentreY(), r.getRight(), r.getCentreY() }, dashes, 2);

    const auto waveform = [this]
    {
        if (auto* v = proc.getValueTreeState().getRawParameterValue (ParamID::waveform))
            return juce::roundToInt (v->load());

        return 0;
    }();

    const auto phase = proc.getLfoPhase();

    juce::Path path;
    const auto width = static_cast<int> (r.getWidth());

    for (int px = 0; px <= width; ++px)
    {
        const auto t = static_cast<float> (px) / juce::jmax (1.0f, r.getWidth());
        const auto v = lfoShape (waveform, phase + (t - playheadAt) * windowCycles);

        const auto x = r.getX() + static_cast<float> (px);
        const auto y = r.getCentreY() - (v * 2.0f - 1.0f) * r.getHeight() * 0.5f * swing;

        if (px == 0)
            path.startNewSubPath (x, y);
        else
            path.lineTo (x, y);
    }

    g.setColour (Theme::saber.withAlpha (0.20f));
    g.strokePath (path, juce::PathStrokeType (9.0f, juce::PathStrokeType::curved));
    g.setColour (Theme::saber);
    g.strokePath (path, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
    g.setColour (juce::Colour (0xffffd7da).withAlpha (0.9f));
    g.strokePath (path, juce::PathStrokeType (1.0f, juce::PathStrokeType::curved));

    const auto px = r.getX() + r.getWidth() * playheadAt;
    g.setColour (Theme::amberBright.withAlpha (0.5f));
    g.drawVerticalLine (static_cast<int> (px), r.getY(), r.getBottom());

    const auto dotY = r.getCentreY() - (lfoShape (waveform, phase) * 2.0f - 1.0f)
                                           * r.getHeight() * 0.5f * swing;
    g.setColour (juce::Colours::white);
    g.fillEllipse (juce::Rectangle<float> (7.0f, 7.0f).withCentre ({ px, dotY }));

    g.setColour (juce::Colours::black);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);
}
