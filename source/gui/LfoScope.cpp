#include "LfoScope.h"

#include "../Parameters.h"
#include "../PluginProcessor.h"
#include "Theme.h"

namespace
{
constexpr float minHz = 20.0f;
constexpr float maxHz = 20000.0f;
constexpr float minDb = -36.0f;
constexpr float maxDb = 6.0f;

constexpr float lfoWindowCycles = 2.0f;
constexpr float playheadAt      = 0.45f;

float readParam (const juce::AudioProcessorValueTreeState& state, const char* id)
{
    if (auto* v = state.getRawParameterValue (id))
        return v->load();

    return 0.0f;
}

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
            auto h = static_cast<uint32_t> (std::floor (phase * 1.0f)) * 2654435761u;
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

void LfoScope::paintGrid (juce::Graphics& g, juce::Rectangle<float> r)
{
    g.setColour (juce::Colours::white.withAlpha (0.05f));

    for (int i = 1; i < 8; ++i)
    {
        const auto x = r.getX() + r.getWidth() * static_cast<float> (i) / 8.0f;
        g.drawVerticalLine (static_cast<int> (x), r.getY(), r.getBottom());
    }

    for (int i = 1; i < 4; ++i)
    {
        const auto y = r.getY() + r.getHeight() * static_cast<float> (i) / 4.0f;
        g.drawHorizontalLine (static_cast<int> (y), r.getX(), r.getRight());
    }
}

void LfoScope::paintResponse (juce::Graphics& g, juce::Rectangle<float> r)
{
    const auto& state = proc.getValueTreeState();

    const auto stageIndex = juce::jlimit (0, static_cast<int> (Params::stageCounts.size()) - 1,
                                          static_cast<int> (readParam (state, ParamID::stages)));
    const auto stages = Params::stageCounts[static_cast<size_t> (stageIndex)];
    const auto spread = readParam (state, ParamID::spread) * 0.01f;
    const auto mix    = readParam (state, ParamID::mix) * 0.01f;
    const auto centre = proc.getModulatedCentreHz();

    juce::Path path;

    const auto width = static_cast<int> (r.getWidth());

    for (int px = 0; px <= width; ++px)
    {
        const auto t = static_cast<float> (px) / juce::jmax (1.0f, r.getWidth());
        const auto f = minHz * std::pow (maxHz / minHz, t);

        float phase = 0.0f;

        for (int s = 0; s < stages; ++s)
        {
            const auto frac = stages > 1 ? static_cast<float> (s) / static_cast<float> (stages - 1) - 0.5f
                                         : 0.0f;
            const auto fcs = centre * std::exp2 (spread * 2.0f * frac);
            phase += -2.0f * std::atan (f / juce::jmax (1.0f, fcs));
        }

        const auto re = (1.0f - mix) + mix * std::cos (phase);
        const auto im = mix * std::sin (phase);
        const auto db = juce::Decibels::gainToDecibels (std::sqrt (re * re + im * im), minDb);

        const auto y = juce::jmap (juce::jlimit (minDb, maxDb, db), maxDb, minDb, r.getY(), r.getBottom());
        const auto x = r.getX() + static_cast<float> (px);

        if (px == 0)
            path.startNewSubPath (x, y);
        else
            path.lineTo (x, y);
    }

    // 0 dB reference
    const auto zeroY = juce::jmap (0.0f, maxDb, minDb, r.getY(), r.getBottom());
    static constexpr float dashes[] = { 3.0f, 4.0f };
    g.setColour (Theme::amber.withAlpha (0.22f));
    g.drawDashedLine ({ r.getX(), zeroY, r.getRight(), zeroY }, dashes, 2);

    g.setColour (Theme::amber.withAlpha (0.18f));
    g.strokePath (path, juce::PathStrokeType (4.5f));
    g.setColour (Theme::amber);
    g.strokePath (path, juce::PathStrokeType (1.6f));
}

void LfoScope::paintLfo (juce::Graphics& g, juce::Rectangle<float> r)
{
    const auto& state = proc.getValueTreeState();
    const auto waveform = static_cast<int> (readParam (state, ParamID::waveform));
    const auto phase = proc.getLfoPhase();

    juce::Path path;

    const auto width = static_cast<int> (r.getWidth());

    for (int px = 0; px <= width; ++px)
    {
        const auto t = static_cast<float> (px) / juce::jmax (1.0f, r.getWidth());
        const auto ph = phase + (t - playheadAt) * lfoWindowCycles;
        const auto v = lfoShape (waveform, ph);

        const auto x = r.getX() + static_cast<float> (px);
        const auto y = juce::jmap (v, 0.0f, 1.0f, r.getBottom() - 2.0f, r.getY() + 2.0f);

        if (px == 0)
            path.startNewSubPath (x, y);
        else
            path.lineTo (x, y);
    }

    g.setColour (Theme::saber.withAlpha (0.20f));
    g.strokePath (path, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved));
    g.setColour (Theme::saber);
    g.strokePath (path, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
    g.setColour (juce::Colour (0xffffd7da).withAlpha (0.9f));
    g.strokePath (path, juce::PathStrokeType (0.9f, juce::PathStrokeType::curved));

    // playhead
    const auto px = r.getX() + r.getWidth() * playheadAt;
    g.setColour (Theme::amberBright.withAlpha (0.5f));
    g.drawVerticalLine (static_cast<int> (px), r.getY(), r.getBottom());

    const auto dotY = juce::jmap (lfoShape (waveform, phase), 0.0f, 1.0f,
                                  r.getBottom() - 2.0f, r.getY() + 2.0f);
    g.setColour (juce::Colours::white);
    g.fillEllipse (juce::Rectangle<float> (7.0f, 7.0f).withCentre ({ px, dotY }));
}

void LfoScope::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff101016), bounds.getCentre(),
                                             juce::Colour (0xff06060a), bounds.getBottomLeft(), true));
    g.fillRoundedRectangle (bounds, 4.0f);

    auto inner = bounds.reduced (2.0f);
    auto lfoArea = inner.removeFromBottom (inner.getHeight() * 0.30f);
    inner.removeFromBottom (2.0f);

    paintGrid (g, inner);
    paintResponse (g, inner);

    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.drawHorizontalLine (static_cast<int> (lfoArea.getY()), inner.getX(), inner.getRight());

    paintLfo (g, lfoArea);

    g.setColour (juce::Colours::black);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);
}
