#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/** Palette and type shared by every panel, taken from design/darth-phaser-layout.png. */
namespace Theme
{
inline const juce::Colour background   { 0xff09090b };
inline const juce::Colour panelTop     { 0xff15151a };
inline const juce::Colour panelBottom  { 0xff0d0d11 };
inline const juce::Colour panelEdge    { 0xff000000 };
inline const juce::Colour panelHilite  { 0x12ffffff };
inline const juce::Colour displayBg    { 0xff07070a };

inline const juce::Colour amber        { 0xffff7a18 };
inline const juce::Colour amberBright  { 0xffffae52 };
inline const juce::Colour saber        { 0xffff2233 };

inline const juce::Colour text         { 0xffeceae6 };
inline const juce::Colour textDim      { 0xff79766f };
inline const juce::Colour knobTrack    { 0xff25252c };

inline juce::Font displayFont (float height, float tracking = 0.0f)
{
    juce::Font f { juce::FontOptions ("Futura", height, juce::Font::plain) };
    return tracking > 0.0f ? f.withExtraKerningFactor (tracking) : f;
}

inline juce::Font label (float height, bool bold = true, float tracking = 0.18f)
{
    juce::Font f { juce::FontOptions ("Avenir Next Condensed", height,
                                      bold ? juce::Font::bold : juce::Font::plain) };
    return f.withExtraKerningFactor (tracking);
}
}
