#include "SegmentedControl.h"

#include "Theme.h"

SegmentedControl::SegmentedControl (juce::RangedAudioParameter& parameter, const juce::StringArray& options)
    : param (parameter), items (options),
      attachment (parameter,
                  [this] (float newValue)
                  {
                      selected = juce::jlimit (0, items.size() - 1, juce::roundToInt (newValue));
                      repaint();
                  },
                  nullptr)
{
    attachment.sendInitialUpdate();
}

SegmentedControl::~SegmentedControl() = default;

int SegmentedControl::indexAt (juce::Point<float> p) const
{
    if (items.isEmpty() || getWidth() <= 0)
        return 0;

    const auto w = static_cast<float> (getWidth()) / static_cast<float> (items.size());
    return juce::jlimit (0, items.size() - 1, static_cast<int> (p.x / w));
}

void SegmentedControl::mouseDown (const juce::MouseEvent& e)
{
    attachment.setValueAsCompleteGesture (static_cast<float> (indexAt (e.position)));
}

void SegmentedControl::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const auto corner = 3.5f;

    g.setColour (juce::Colour (0xff0a0a0d));
    g.fillRoundedRectangle (bounds, corner);

    const auto segWidth = bounds.getWidth() / static_cast<float> (juce::jmax (1, items.size()));

    for (int i = 0; i < items.size(); ++i)
    {
        const auto cell = juce::Rectangle<float> (bounds.getX() + segWidth * static_cast<float> (i),
                                                  bounds.getY(), segWidth, bounds.getHeight());

        if (i == selected)
        {
            juce::Path p;
            p.addRoundedRectangle (cell.getX(), cell.getY(), cell.getWidth(), cell.getHeight(),
                                   corner, corner,
                                   i == 0, i == items.size() - 1,
                                   i == 0, i == items.size() - 1);

            g.setColour (Theme::amber.withAlpha (0.30f));
            g.fillRoundedRectangle (cell.expanded (2.0f), corner + 2.0f);

            g.setGradientFill ({ Theme::amberBright, cell.getX(), cell.getY(),
                                 Theme::amber, cell.getX(), cell.getBottom(), false });
            g.fillPath (p);
        }
        else if (i > 0)
        {
            g.setColour (juce::Colours::black);
            g.drawVerticalLine (static_cast<int> (cell.getX()), cell.getY(), cell.getBottom());
        }

        g.setColour (i == selected ? juce::Colour (0xff120a03) : juce::Colour (0xff6f6c66));
        g.setFont (Theme::label (10.0f, true, 0.14f));
        g.drawText (items[i], cell.toNearestInt(), juce::Justification::centred, false);
    }

    g.setColour (juce::Colours::black);
    g.drawRoundedRectangle (bounds.reduced (0.5f), corner, 1.0f);

    g.setColour (Theme::panelHilite);
    g.drawHorizontalLine (static_cast<int> (bounds.getY() + 1.0f), bounds.getX() + 2.0f, bounds.getRight() - 2.0f);
}
