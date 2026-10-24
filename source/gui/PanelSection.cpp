#include "PanelSection.h"

#include "Theme.h"

PanelSection::PanelSection (const juce::String& title, bool showLed)
    : titleText (title), led (showLed)
{
    setInterceptsMouseClicks (false, true);
}

juce::Rectangle<int> PanelSection::getContentBounds() const
{
    return getLocalBounds().withTrimmedTop (headerHeight);
}

juce::Rectangle<int> PanelSection::getHeaderBounds() const
{
    return getLocalBounds().removeFromTop (headerHeight);
}

void PanelSection::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const auto corner = 4.0f;

    g.setGradientFill ({ Theme::panelTop, bounds.getX(), bounds.getY(),
                         Theme::panelBottom, bounds.getX(), bounds.getBottom(), false });
    g.fillRoundedRectangle (bounds, corner);

    g.setColour (Theme::panelEdge);
    g.drawRoundedRectangle (bounds.reduced (0.5f), corner, 1.0f);

    g.setColour (Theme::panelHilite);
    g.drawHorizontalLine (static_cast<int> (bounds.getY() + 1.0f),
                          bounds.getX() + 3.0f, bounds.getRight() - 3.0f);

    auto header = getHeaderBounds().toFloat();

    g.setColour (Theme::amber);
    g.setFont (Theme::label (10.0f, true, 0.22f));
    const auto titleWidth = juce::GlyphArrangement::getStringWidth (g.getCurrentFont(), titleText) + 4.0f;
    g.drawText (titleText, header.withTrimmedLeft (11.0f).toNearestInt(),
                juce::Justification::centredLeft, false);

    // hairline running from the title to the right edge
    const auto lineY = header.getCentreY();
    const auto lineStart = header.getX() + 15.0f + titleWidth;
    const auto lineEnd = header.getRight() - (led ? 20.0f : 11.0f);

    if (lineEnd > lineStart)
    {
        g.setGradientFill ({ Theme::amber.withAlpha (0.32f), lineStart, lineY,
                             Theme::amber.withAlpha (0.0f), lineEnd, lineY, false });
        g.fillRect (juce::Rectangle<float> (lineStart, lineY - 0.5f, lineEnd - lineStart, 1.0f));
    }

    if (led)
    {
        const auto dot = juce::Rectangle<float> (5.0f, 5.0f)
                             .withCentre ({ header.getRight() - 11.0f, lineY });
        g.setColour (Theme::amber.withAlpha (0.28f));
        g.fillEllipse (dot.expanded (3.0f));
        g.setColour (Theme::amber.withAlpha (0.8f));
        g.fillEllipse (dot);
    }

    g.setColour (Theme::panelEdge);
    g.drawHorizontalLine (static_cast<int> (header.getBottom()), bounds.getX() + 1.0f, bounds.getRight() - 1.0f);
    g.setColour (Theme::panelHilite);
    g.drawHorizontalLine (static_cast<int> (header.getBottom()) + 1, bounds.getX() + 1.0f, bounds.getRight() - 1.0f);
}
