#include "PresetShelfComponent.h"

PresetShelfComponent::SlotCard::SlotCard(int slotIndex)
    : index(slotIndex)
{
    numberLabel.setText(juce::String::charToString(static_cast<juce::juce_wchar>('A' + slotIndex)), juce::dontSendNotification);
    numberLabel.setJustificationType(juce::Justification::centred);
    numberLabel.setColour(juce::Label::textColourId, Theme::getPresetAccent(slotIndex));
    numberLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    addAndMakeVisible(numberLabel);

    nameLabel.setEditable(true);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    nameLabel.setColour(juce::Label::textColourId, Theme::textPrimary);
    nameLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(nameLabel);

    modelLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    modelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(modelLabel);

    irLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    irLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(irLabel);

    addAndMakeVisible(loadButton);
    addAndMakeVisible(storeButton);
    addAndMakeVisible(clearButton);
}

void PresetShelfComponent::SlotCard::setSlotData(const PresetShelfSlotView& view)
{
    occupied = view.occupied;
    nameLabel.setText(view.name, juce::dontSendNotification);
    modelLabel.setText("Model: " + (view.modelName.isEmpty() ? "None" : view.modelName), juce::dontSendNotification);
    irLabel.setText("IR: " + (view.irName.isEmpty() ? "None" : view.irName), juce::dontSendNotification);
    loadButton.setEnabled(view.occupied);
    clearButton.setEnabled(view.occupied);
}

void PresetShelfComponent::SlotCard::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const auto accent = Theme::getPresetAccent(index);
    auto base = occupied ? Theme::panelRaised.brighter(0.12f) : Theme::panel;
    g.setGradientFill(juce::ColourGradient(base, bounds.getX(), bounds.getY(),
                                           base.darker(0.08f), bounds.getRight(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, 18.0f);
    g.setColour(occupied ? accent.withAlpha(0.18f) : juce::Colours::black.withAlpha(0.14f));
    g.fillRoundedRectangle(bounds.reduced(4.0f), 14.0f);
    g.setColour(occupied ? accent : Theme::panelOutline.withAlpha(0.8f));
    g.drawRoundedRectangle(bounds, 18.0f, occupied ? 1.8f : 1.0f);

    g.setColour((occupied ? accent : Theme::panelOutline).withAlpha(0.16f));
    g.fillRoundedRectangle(bounds.removeFromTop(38.0f), 18.0f);
}

void PresetShelfComponent::SlotCard::resized()
{
    auto area = getLocalBounds().reduced(14);

    auto topRow = area.removeFromTop(28);
    numberLabel.setBounds(topRow.removeFromLeft(42));
    nameLabel.setBounds(topRow);

    area.removeFromTop(8);
    modelLabel.setBounds(area.removeFromTop(20));
    irLabel.setBounds(area.removeFromTop(20));
    area.removeFromTop(10);

    auto buttonRow = area.removeFromBottom(30);
    loadButton.setBounds(buttonRow.removeFromLeft(buttonRow.getWidth() / 3).reduced(2));
    storeButton.setBounds(buttonRow.removeFromLeft(buttonRow.getWidth() / 2).reduced(2));
    clearButton.setBounds(buttonRow.reduced(2));
}

PresetShelfComponent::PresetShelfComponent(int slotCount)
{
    sectionLabel.setText("PRESETS", juce::dontSendNotification);
    sectionLabel.setColour(juce::Label::textColourId, Theme::textPrimary);
    sectionLabel.setJustificationType(juce::Justification::centredLeft);
    sectionLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    addAndMakeVisible(sectionLabel);

    for (int index = 0; index < slotCount; ++index)
    {
        auto card = std::make_unique<SlotCard>(index);
        auto* cardPtr = card.get();

        cardPtr->loadButton.onClick = [this, index]()
        {
            if (onLoadClicked)
                onLoadClicked(index);
        };

        cardPtr->storeButton.onClick = [this, index]()
        {
            if (onStoreClicked)
                onStoreClicked(index);
        };

        cardPtr->clearButton.onClick = [this, index]()
        {
            if (onClearClicked)
                onClearClicked(index);
        };

        cardPtr->nameLabel.onTextChange = [this, cardPtr, index]()
        {
            if (onNameChanged)
                onNameChanged(index, cardPtr->nameLabel.getText());
        };

        addAndMakeVisible(cardPtr);
        slotCards.push_back(std::move(card));
    }
}

void PresetShelfComponent::setSlots(const std::vector<PresetShelfSlotView>& slotViews)
{
    const auto count = juce::jmin(static_cast<int>(slotViews.size()), static_cast<int>(slotCards.size()));
    for (int index = 0; index < count; ++index)
        slotCards[static_cast<size_t>(index)]->setSlotData(slotViews[static_cast<size_t>(index)]);
}

void PresetShelfComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    g.setGradientFill(juce::ColourGradient(Theme::panel.withAlpha(0.94f), bounds.getX(), bounds.getY(),
                                           Theme::panel.darker(0.08f), bounds.getRight(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, 24.0f);
    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(bounds, 24.0f, 1.0f);

    auto titleLine = bounds.reduced(18.0f, 0.0f).removeFromTop(34.0f);
    g.setColour(Theme::textSecondary.withAlpha(0.32f));
    g.drawLine(titleLine.getX(), titleLine.getBottom(), titleLine.getRight(), titleLine.getBottom(), 1.0f);
}

void PresetShelfComponent::resized()
{
    auto area = getLocalBounds().reduced(14);
    sectionLabel.setBounds(area.removeFromTop(24));
    area.removeFromTop(12);

    const int columns = juce::jmax(1, static_cast<int>(slotCards.size()));
    const int gap = 10;
    const int cardWidth = (area.getWidth() - gap * (columns - 1)) / columns;
    const int cardHeight = area.getHeight();

    for (int column = 0; column < columns; ++column)
    {
        const int index = column;
        if (index >= static_cast<int>(slotCards.size()))
            return;

        slotCards[static_cast<size_t>(index)]->setBounds(area.getX() + column * (cardWidth + gap),
                                                         area.getY(),
                                                         cardWidth, cardHeight);
    }
}
