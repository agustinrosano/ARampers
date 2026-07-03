#include "GoldLookAndFeel.h"

GoldLookAndFeel::GoldLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, Theme::textPrimary);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, Theme::panelRaised);
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
                                               static_cast<float>(width), static_cast<float>(height)).reduced(8.0f);
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = juce::jmap(sliderPos, 0.0f, 1.0f, rotaryStartAngle, rotaryEndAngle);

    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.fillEllipse(bounds.translated(0.0f, 4.0f));

    g.setColour(Theme::accent.withAlpha(0.10f));
    g.fillEllipse(bounds.expanded(6.0f));

    g.setGradientFill(juce::ColourGradient(Theme::panelRaised.brighter(0.28f), centre.x, bounds.getY(),
                                           Theme::panel.darker(0.35f), centre.x, bounds.getBottom(), false));
    g.fillEllipse(bounds);

    g.setColour(Theme::panelOutline);
    g.drawEllipse(bounds, 1.5f);

    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawEllipse(bounds.reduced(4.0f), 1.0f);

    juce::Path valueArc;
    valueArc.addCentredArc(centre.x, centre.y, radius - 6.0f, radius - 6.0f, 0.0f,
                           rotaryStartAngle, angle, true);
    g.setColour(Theme::accent);
    g.strokePath(valueArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path trackArc;
    trackArc.addCentredArc(centre.x, centre.y, radius - 6.0f, radius - 6.0f, 0.0f,
                           angle, rotaryEndAngle, true);
    g.setColour(Theme::textSecondary.withAlpha(0.38f));
    g.strokePath(trackArc, juce::PathStrokeType(2.0f));

    const auto pointerLength = radius * 0.58f;
    const auto pointerThickness = 3.0f;
    juce::Path pointer;
    pointer.addRoundedRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength, 1.5f);
    g.setColour(Theme::textPrimary);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));

    g.setColour(Theme::accent.brighter(0.25f));
    g.fillEllipse(centre.x - 5.0f, centre.y - 5.0f, 10.0f, 10.0f);
    g.setColour(juce::Colours::white.withAlpha(0.35f));
    g.fillEllipse(centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);
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
    const auto indicator = bounds.removeFromLeft(46.0f).reduced(4.0f, 7.0f);
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

    g.setColour(active ? Theme::textPrimary : Theme::textSecondary);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText(button.getButtonText(), bounds.reduced(8.0f, 0.0f), juce::Justification::centredLeft, false);
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
