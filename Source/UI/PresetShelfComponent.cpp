#include "PresetShelfComponent.h"

PresetShelfComponent::SlotCard::SlotCard(int slotIndex)
    : index(slotIndex)
{
    numberLabel.setText(juce::String::charToString(static_cast<juce::juce_wchar>('A' + slotIndex)), juce::dontSendNotification);
    numberLabel.setJustificationType(juce::Justification::centred);
    numberLabel.setColour(juce::Label::textColourId, Theme::getPresetAccent(slotIndex));
    numberLabel.setFont(juce::Font(48.0f, juce::Font::bold));
    addAndMakeVisible(numberLabel);

    nameLabel.setEditable(true);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    nameLabel.setColour(juce::Label::textColourId, Theme::getPresetAccent(slotIndex));
    nameLabel.setFont(juce::Font(13.0f, juce::Font::plain));
    addAndMakeVisible(nameLabel);

    // Hide details not in mockup
    modelLabel.setVisible(false);
    irLabel.setVisible(false);
    loadButton.setVisible(false);
    storeButton.setVisible(false);
    clearButton.setVisible(false);
}

void PresetShelfComponent::SlotCard::setSlotData(const PresetShelfSlotView& view)
{
    occupied = view.occupied;
    nameLabel.setText(juce::String::charToString(static_cast<juce::juce_wchar>('A' + index)) + ": " + (view.name.isEmpty() ? "CLEAN" : view.name.toUpperCase()), juce::dontSendNotification);
}

void PresetShelfComponent::SlotCard::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const auto accent = Theme::getPresetAccent(index);

    auto* shelf = findParentComponentOfClass<PresetShelfComponent>();
    const bool isActive = (shelf != nullptr && shelf->activeSlotIndex == index);

    // Leave space at bottom for indicator dot
    auto stompbox = bounds;
    stompbox.removeFromBottom(16.0f);

    // Draw stompbox body
    g.setColour(juce::Colour(0xff222222));
    g.fillRoundedRectangle(stompbox, 10.0f);

    // Draw border
    g.setColour(isActive ? accent : accent.withAlpha(0.35f));
    g.drawRoundedRectangle(stompbox, 10.0f, isActive ? 2.2f : 1.2f);

    // Adjust child label colours based on active state
    numberLabel.setColour(juce::Label::textColourId, isActive ? accent : accent.withAlpha(0.4f));
    nameLabel.setColour(juce::Label::textColourId, isActive ? accent : accent.withAlpha(0.4f));

    // Draw dot underneath
    auto dotRect = juce::Rectangle<float>(bounds.getCentreX() - 4.0f, bounds.getBottom() - 10.0f, 8.0f, 8.0f);
    if (isActive)
    {
        g.setColour(accent.withAlpha(0.3f));
        g.fillEllipse(dotRect.expanded(4.0f));
        g.setColour(accent);
        g.fillEllipse(dotRect);
    }
    else
    {
        g.setColour(juce::Colour(0xff555555));
        g.fillEllipse(dotRect);
    }
}

void PresetShelfComponent::SlotCard::resized()
{
    auto area = getLocalBounds();
    area.removeFromBottom(16); // space for dot

    auto h = area.getHeight();
    numberLabel.setBounds(area.removeFromTop(h * 0.65f));
    nameLabel.setBounds(area.reduced(6, 2));
}

void PresetShelfComponent::SlotCard::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown())
    {
        if (auto* shelf = findParentComponentOfClass<PresetShelfComponent>())
        {
            if (shelf->onStoreClicked)
                shelf->onStoreClicked(index);
        }
    }
    else
    {
        if (auto* shelf = findParentComponentOfClass<PresetShelfComponent>())
        {
            if (shelf->onLoadClicked)
                shelf->onLoadClicked(index);
        }
    }
}

PresetShelfComponent::PresetShelfComponent(int slotCount)
{
    sectionLabel.setText("PRESETS", juce::dontSendNotification);
    sectionLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    sectionLabel.setJustificationType(juce::Justification::centred);
    sectionLabel.setFont(juce::Font(13.0f, juce::Font::bold));
    addAndMakeVisible(sectionLabel);

    // We only show 4 stompboxes (A, B, C, D)
    const int count = juce::jmin(slotCount, 4);
    for (int index = 0; index < count; ++index)
    {
        auto card = std::make_unique<SlotCard>(index);
        auto* cardPtr = card.get();

        cardPtr->nameLabel.onTextChange = [this, cardPtr, index]()
        {
            if (onNameChanged)
            {
                // Remove the "A: " prefix before saving
                auto rawName = cardPtr->nameLabel.getText();
                if (rawName.startsWith(juce::String::charToString(static_cast<juce::juce_wchar>('A' + index)) + ": "))
                    rawName = rawName.substring(3);
                onNameChanged(index, rawName);
            }
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

void PresetShelfComponent::setActiveSlot(int index)
{
    if (index >= 0 && index < static_cast<int>(slotCards.size()))
    {
        activeSlotIndex = index;
        repaint();
    }
}

void PresetShelfComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    
    // Draw thin frame border around the presets section
    g.setColour(Theme::panelOutline.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 12.0f, 1.0f);

    // Draw the "PRESETS" title in the middle of the frame top
    auto titleArea = bounds.removeFromTop(20.0f);
    g.setColour(juce::Colour(0xff2d2d2d)); // matching background to mask frame line
    g.fillRect(titleArea.getCentreX() - 50.0f, titleArea.getY(), 100.0f, 20.0f);
}

void PresetShelfComponent::resized()
{
    auto area = getLocalBounds().reduced(14);
    
    // Title at the top center
    sectionLabel.setBounds(area.getX() + (area.getWidth() - 120) / 2, area.getY() - 24, 120, 20);
    
    area.removeFromTop(12);

    const int columns = juce::jmax(1, static_cast<int>(slotCards.size()));
    const int gap = 16;
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
