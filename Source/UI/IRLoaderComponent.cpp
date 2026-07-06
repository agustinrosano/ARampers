#include "IRLoaderComponent.h"
#include "GoldLookAndFeel.h"

IRLoaderComponent::IRLoaderComponent(juce::AudioProcessorValueTreeState& state, const juce::String& bypassParamId)
{
    titleLabel.setText("IR (Impulse Response)", juce::dontSendNotification);
    titleLabel.setColour(juce::Label::textColourId, Theme::textPrimary);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::Font(15.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    fileLabel.setText("IR: No IR loaded", juce::dontSendNotification);
    fileLabel.setJustificationType(juce::Justification::centredLeft);
    fileLabel.setFont(juce::Font(13.0f, juce::Font::plain));
    fileLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    addAndMakeVisible(fileLabel);

    addAndMakeVisible(loadButton);
    addAndMakeVisible(clearButton);

    loadButton.onClick = [this]()
    {
        if (onLoadClicked)
            onLoadClicked();
    };

    clearButton.onClick = [this]()
    {
        if (onClearClicked)
            onClearClicked();
    };

    bypassButton.setButtonText("ON");
    addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, bypassParamId, bypassButton);
}

void IRLoaderComponent::setFileName(const juce::String& name)
{
    if (name.isEmpty() || name == "No IR loaded")
        fileLabel.setText("IR: No IR loaded", juce::dontSendNotification);
    else
        fileLabel.setText("IR: " + name, juce::dontSendNotification);
}

void IRLoaderComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    g.setGradientFill(juce::ColourGradient(Theme::panelRaised, bounds.getX(), bounds.getY(),
                                           Theme::panel, bounds.getRight(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, 12.0f);
    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(bounds, 12.0f, 1.2f);

    // Draw Speaker Cabinet Icon on the left
    auto iconRect = juce::Rectangle<float>(14.0f, (getHeight() - 32.0f) / 2.0f, 32.0f, 32.0f);
    g.setColour(Theme::textSecondary);
    // Outer cabinet box
    g.drawRoundedRectangle(iconRect, 3.0f, 1.5f);
    // Inner grille area
    auto grill = iconRect.reduced(3.0f);
    g.setColour(Theme::panelOutline);
    g.fillRoundedRectangle(grill, 1.0f);
    g.setColour(Theme::textSecondary);
    // Speaker circle (cone)
    g.drawEllipse(grill.getCentreX() - 8.0f, grill.getCentreY() - 8.0f, 16.0f, 16.0f, 1.2f);
    g.fillEllipse(grill.getCentreX() - 3.0f, grill.getCentreY() - 3.0f, 6.0f, 6.0f);

    // Draw LEDs (small vertical indicators on the right, next to buttons)
    auto ledArea = juce::Rectangle<float>(static_cast<float>(getWidth()) - 190.0f, 14.0f, 10.0f, 22.0f);
    const juce::Colour ledColours[] = { Theme::success, Theme::presetBlue, Theme::presetPurple };
    for (int i = 0; i < 3; ++i)
    {
        auto led = juce::Rectangle<float>(ledArea.getX(), ledArea.getY() + i * 8.0f, 6.0f, 6.0f);
        g.setColour(ledColours[i].withAlpha(0.18f));
        g.fillEllipse(led.expanded(3.0f));
        g.setColour(ledColours[i]);
        g.fillEllipse(led);
    }
}

void IRLoaderComponent::resized()
{
    auto area = getLocalBounds().reduced(12);

    area.removeFromLeft(46);

    auto rightArea = area.removeFromRight(168);
    auto toggleArea = rightArea.removeFromRight(50).reduced(0, 4);
    bypassButton.setBounds(toggleArea);

    auto clearArea = rightArea.removeFromRight(54).reduced(0, 6);
    clearButton.setBounds(clearArea);

    auto loadArea = rightArea.removeFromRight(55).reduced(0, 6);
    loadButton.setBounds(loadArea);

    area.removeFromRight(20);

    auto labelHeight = area.getHeight() / 2;
    titleLabel.setBounds(area.removeFromTop(labelHeight).translated(0, 2));
    fileLabel.setBounds(area.translated(0, -2));
}
