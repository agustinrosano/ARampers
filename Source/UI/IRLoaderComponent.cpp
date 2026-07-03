#include "IRLoaderComponent.h"
#include "GoldLookAndFeel.h"

IRLoaderComponent::IRLoaderComponent()
{
    titleLabel.setText("IR (Impulse Response)", juce::dontSendNotification);
    titleLabel.setColour(juce::Label::textColourId, Theme::accent);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    addAndMakeVisible(loadButton);
    addAndMakeVisible(clearButton);
    addAndMakeVisible(fileLabel);

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

    fileLabel.setText("No IR loaded", juce::dontSendNotification);
    fileLabel.setJustificationType(juce::Justification::centredLeft);
    fileLabel.setColour(juce::Label::textColourId, Theme::textPrimary);
}

void IRLoaderComponent::setFileName(const juce::String& name)
{
    if (name.isEmpty())
        fileLabel.setText("No IR loaded", juce::dontSendNotification);
    else
        fileLabel.setText(name, juce::dontSendNotification);
}

void IRLoaderComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    g.setGradientFill(juce::ColourGradient(Theme::panelRaised.brighter(0.08f), bounds.getX(), bounds.getY(),
                                           Theme::panel.darker(0.12f), bounds.getRight(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, 20.0f);
    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(bounds, 20.0f, 1.0f);

    auto ledArea = bounds.removeFromTop(28.0f).removeFromRight(68.0f).reduced(8.0f, 4.0f);
    const juce::Colour ledColours[] = { Theme::success, Theme::presetBlue, Theme::presetPurple };
    for (int i = 0; i < 3; ++i)
    {
        auto led = ledArea.removeFromLeft(14.0f).withSizeKeepingCentre(8.0f, 8.0f);
        g.setColour(ledColours[i].withAlpha(0.18f));
        g.fillEllipse(led.expanded(5.0f));
        g.setColour(ledColours[i]);
        g.fillEllipse(led);
    }
}

void IRLoaderComponent::resized()
{
    auto area = getLocalBounds().reduced(16);
    titleLabel.setBounds(area.removeFromTop(20));
    area.removeFromTop(8);
    auto buttonArea = area.removeFromRight(212);
    clearButton.setBounds(buttonArea.removeFromRight(84));
    buttonArea.removeFromRight(8);
    loadButton.setBounds(buttonArea);
    fileLabel.setBounds(area);
}
