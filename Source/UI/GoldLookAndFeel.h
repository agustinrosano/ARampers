#pragma once

#include <JuceHeader.h>

namespace Theme
{
static const juce::Colour backgroundTop = juce::Colour(0xff2c2c2f);
static const juce::Colour backgroundBottom = juce::Colour(0xff1a1a1d);
static const juce::Colour panel = juce::Colour(0xff343438);
static const juce::Colour panelRaised = juce::Colour(0xff3d3d42);
static const juce::Colour panelOutline = juce::Colour(0xff6a6b72);
static const juce::Colour accent = juce::Colour(0xffff6f61);
static const juce::Colour accentSoft = juce::Colour(0x66ff6f61);
static const juce::Colour textPrimary = juce::Colour(0xfff4f4f6);
static const juce::Colour textSecondary = juce::Colour(0xffb5b7bf);
static const juce::Colour success = juce::Colour(0xff7ef07a);
static const juce::Colour danger = juce::Colour(0xffff756c);
static const juce::Colour presetGreen = juce::Colour(0xff66eb78);
static const juce::Colour presetOrange = juce::Colour(0xffffb347);
static const juce::Colour presetBlue = juce::Colour(0xff56b8ff);
static const juce::Colour presetPurple = juce::Colour(0xffc472ff);

inline juce::Colour getPresetAccent(int index)
{
    switch (index % 4)
    {
        case 0: return presetGreen;
        case 1: return presetOrange;
        case 2: return presetBlue;
        default: return presetPurple;
    }
}
}

class GoldLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GoldLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;
    void drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int,
                      juce::ComboBox& box) override;
    juce::Font getLabelFont(juce::Label& label) override;
};
