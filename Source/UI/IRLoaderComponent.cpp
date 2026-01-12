#include "IRLoaderComponent.h"

IRLoaderComponent::IRLoaderComponent()
{
    addAndMakeVisible(loadButton);
    addAndMakeVisible(fileLabel);

    loadButton.onClick = [this]()
    {
        if (onLoadClicked)
            onLoadClicked();
    };

    fileLabel.setText("No IR loaded", juce::dontSendNotification);
    fileLabel.setJustificationType(juce::Justification::centredLeft);
}

void IRLoaderComponent::setFileName(const juce::String& name)
{
    if (name.isEmpty())
        fileLabel.setText("No IR loaded", juce::dontSendNotification);
    else
        fileLabel.setText(name, juce::dontSendNotification);
}

void IRLoaderComponent::resized()
{
    auto area = getLocalBounds().reduced(6);
    auto buttonArea = area.removeFromLeft(120);
    loadButton.setBounds(buttonArea);
    fileLabel.setBounds(area);
}
