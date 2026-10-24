#include "StereoOffsetDisplay.h"

#include "Theme.h"

StereoOffsetDisplay::StereoOffsetDisplay (juce::RangedAudioParameter& stereoPhaseDegrees)
    : attachment (stereoPhaseDegrees,
                  [this] (float degrees) { offsetCycles = degrees / 360.0f; repaint(); },
                  nullptr)
{
    attachment.sendInitialUpdate();
}

void StereoOffsetDisplay::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff101016), bounds.getCentre(),
                                             juce::Colour (0xff06060a), bounds.getBottomLeft(), true));
    g.fillRoundedRectangle (bounds, 4.0f);

    const auto r = bounds.reduced (3.0f);

    static constexpr float dashes[] = { 3.0f, 4.0f };
    g.setColour (Theme::amber.withAlpha (0.16f));
    g.drawDashedLine ({ r.getX(), r.getCentreY(), r.getRight(), r.getCentreY() }, dashes, 2);

    const auto curve = [&r] (float shift)
    {
        juce::Path path;
        const auto width = static_cast<int> (r.getWidth());

        for (int px = 0; px <= width; ++px)
        {
            const auto t = static_cast<float> (px) / juce::jmax (1.0f, r.getWidth());
            const auto y = r.getCentreY()
                           - std::sin (juce::MathConstants<float>::twoPi * (t * 1.5f + shift))
                                 * r.getHeight() * 0.34f;
            const auto x = r.getX() + static_cast<float> (px);

            if (px == 0)
                path.startNewSubPath (x, y);
            else
                path.lineTo (x, y);
        }

        return path;
    };

    const std::pair<juce::Colour, float> traces[] { { Theme::amber, 0.0f }, { Theme::saber, offsetCycles } };

    for (const auto& [colour, shift] : traces)
    {
        const auto path = curve (shift);

        g.setColour (colour.withAlpha (0.16f));
        g.strokePath (path, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved));
        g.setColour (colour);
        g.strokePath (path, juce::PathStrokeType (1.7f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    g.setColour (juce::Colours::black);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 1.0f);
}
