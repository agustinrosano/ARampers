#pragma once

#include <JuceHeader.h>

namespace Theme
{
static const juce::Colour backgroundTop = juce::Colour(0xff2d2d2d);
static const juce::Colour backgroundBottom = juce::Colour(0xff262626);
static const juce::Colour panel = juce::Colour(0xff333333);
static const juce::Colour panelRaised = juce::Colour(0xff3e3e3e);
static const juce::Colour panelOutline = juce::Colour(0xff4c4c4c);
static const juce::Colour accent = juce::Colour(0xffe85545);
static const juce::Colour accentSoft = juce::Colour(0x33e85545);
static const juce::Colour textPrimary = juce::Colour(0xffeaeaea);
static const juce::Colour textSecondary = juce::Colour(0xff9a9a9a);
static const juce::Colour success = juce::Colour(0xff5cd66f);
static const juce::Colour danger = juce::Colour(0xffe85545);
static const juce::Colour presetGreen = juce::Colour(0xff4cd964);
static const juce::Colour presetOrange = juce::Colour(0xffff9500);
static const juce::Colour presetBlue = juce::Colour(0xff34aadc);
static const juce::Colour presetPurple = juce::Colour(0xffaf52de);

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
