#include "PresetBarComponent.h"

PresetBarComponent::PresetBarComponent()
{
    addAndMakeVisible(presetBox);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(loadButton);
    addAndMakeVisible(prevButton);
    addAndMakeVisible(nextButton);

    presetBox.onChange = [this]()
    {
        if (onPresetSelected)
            onPresetSelected(presetBox.getSelectedItemIndex());
    };

    saveButton.onClick = [this]()
    {
        if (onSaveClicked)
            onSaveClicked();
    };

    loadButton.onClick = [this]()
    {
        if (onLoadClicked)
            onLoadClicked();
    };

    prevButton.onClick = [this]()
    {
        if (onPrevClicked)
            onPrevClicked();
    };

    nextButton.onClick = [this]()
    {
        if (onNextClicked)
            onNextClicked();
    };
}

void PresetBarComponent::setPresetNames(const juce::StringArray& names)
{
    presetBox.clear(juce::dontSendNotification);
    presetBox.addItemList(names, 1);
    if (names.size() > 0)
        presetBox.setSelectedItemIndex(0, juce::dontSendNotification);
}

int PresetBarComponent::getSelectedIndex() const
{
    return presetBox.getSelectedItemIndex();
}

void PresetBarComponent::setSelectedIndex(int index)
{
    presetBox.setSelectedItemIndex(index, juce::dontSendNotification);
}

void PresetBarComponent::resized()
{
    auto area = getLocalBounds().reduced(6);

    auto navArea = area.removeFromRight(120);
    prevButton.setBounds(navArea.removeFromLeft(60).reduced(2));
    nextButton.setBounds(navArea.reduced(2));

    auto saveLoadArea = area.removeFromRight(120);
    loadButton.setBounds(saveLoadArea.removeFromLeft(60).reduced(2));
    saveButton.setBounds(saveLoadArea.reduced(2));

    presetBox.setBounds(area.reduced(2));
}
