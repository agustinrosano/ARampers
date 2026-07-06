#include "GoldLookAndFeel.h"

GoldLookAndFeel::GoldLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, Theme::accent);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::textColourId, Theme::textPrimary);
    setColour(juce::TextButton::buttonColourId, Theme::panelRaised);
    setColour(juce::TextButton::textColourOffId, Theme::textPrimary);
    setColour(juce::ComboBox::backgroundColourId, Theme::panelRaised);
    setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ComboBox::textColourId, Theme::textPrimary);
    setColour(juce::PopupMenu::backgroundColourId, Theme::panel);
    setColour(juce::PopupMenu::textColourId, Theme::textPrimary);
}

void GoldLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                       float rotaryStartAngle, float rotaryEndAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                               static_cast<float>(width), static_cast<float>(height)).reduced(2.5f);
    const auto radius = juce::jmax(12.0f, juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f);
    const auto centre = bounds.getCentre();
    const auto angle = juce::jmap(sliderPos, 0.0f, 1.0f, rotaryStartAngle, rotaryEndAngle);

    juce::Path trackArc;
    trackArc.addCentredArc(centre.x, centre.y, radius - 5.0f, radius - 5.0f, 0.0f,
                           rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xff3a3a3a));
    g.strokePath(trackArc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc(centre.x, centre.y, radius - 5.0f, radius - 5.0f, 0.0f,
                           rotaryStartAngle, angle, true);
    g.setColour(Theme::accent);
    g.strokePath(valueArc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const auto dialRadius = radius - 10.0f;
    const auto capBounds = juce::Rectangle<float>(centre.x - dialRadius, centre.y - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f);
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillEllipse(capBounds.translated(0.0f, 2.0f));

    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff555555), centre.x, capBounds.getY(),
                                           juce::Colour(0xff2d2d2d), centre.x, capBounds.getBottom(), false));
    g.fillEllipse(capBounds);

    g.setColour(juce::Colour(0xff666666));
    g.drawEllipse(capBounds, 1.5f);

    const auto pointerLength = (radius - 10.0f) * 0.95f;
    const auto pointerThickness = 3.0f;
    juce::Path pointer;
    pointer.addRoundedRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength, 1.0f);
    g.setColour(Theme::accent);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
}

void GoldLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    auto base = Theme::panelRaised;

    if (shouldDrawButtonAsDown)
        base = base.brighter(0.16f);
    else if (shouldDrawButtonAsHighlighted)
        base = base.brighter(0.08f);

    g.setGradientFill(juce::ColourGradient(base.brighter(0.24f), bounds.getCentreX(), bounds.getY(),
                                           base.darker(0.12f), bounds.getCentreX(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, 10.0f);

    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(bounds, 10.0f, 1.0f);

    g.setColour(juce::Colours::white.withAlpha(0.06f));
    g.drawRoundedRectangle(bounds.reduced(2.0f), 8.5f, 1.0f);
}

void GoldLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool, bool)
{
    auto bounds = button.getLocalBounds().toFloat();
    const auto text = button.getButtonText().trim();
    const bool hasText = text.isNotEmpty();
    const auto indicatorBounds = hasText
        ? bounds.removeFromLeft(48.0f)
        : bounds.withSizeKeepingCentre(48.0f, bounds.getHeight());
    const auto indicator = indicatorBounds.reduced(4.0f, 7.0f);
    const auto active = button.getToggleState();

    g.setColour(Theme::panelRaised.darker(0.1f));
    g.fillRoundedRectangle(indicator, indicator.getHeight() * 0.5f);
    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(indicator, indicator.getHeight() * 0.5f, 1.0f);

    auto knob = indicator.reduced(3.0f);
    knob.setWidth(knob.getHeight());
    knob.setX(active ? indicator.getRight() - knob.getWidth() - 3.0f : indicator.getX() + 3.0f);
    g.setColour(active ? Theme::accent : Theme::textSecondary.withAlpha(0.7f));
    g.fillEllipse(knob);
    if (active)
    {
        g.setColour(Theme::accent.withAlpha(0.22f));
        g.fillEllipse(knob.expanded(4.0f));
    }

    if (hasText)
    {
        g.setColour(active ? Theme::textPrimary : Theme::textSecondary);
        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.drawText(text, bounds.reduced(8.0f, 0.0f), juce::Justification::centredLeft, false);
    }
}

void GoldLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int,
                                   juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f);
    g.setGradientFill(juce::ColourGradient(Theme::panelRaised.brighter(0.14f), bounds.getCentreX(), bounds.getY(),
                                           Theme::panelRaised.darker(0.14f), bounds.getCentreX(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, 10.0f);
    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(bounds, 10.0f, 1.0f);

    juce::Path arrow;
    const auto right = bounds.removeFromRight(26.0f);
    arrow.startNewSubPath(right.getCentreX() - 5.0f, right.getCentreY() - 2.0f);
    arrow.lineTo(right.getCentreX(), right.getCentreY() + 4.0f);
    arrow.lineTo(right.getCentreX() + 5.0f, right.getCentreY() - 2.0f);
    g.setColour(Theme::accent);
    g.strokePath(arrow, juce::PathStrokeType(2.0f));
}

juce::Font GoldLookAndFeel::getLabelFont(juce::Label& label)
{
    return juce::Font(label.getFont().getHeight(), juce::Font::bold);
}
