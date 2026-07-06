#include "PresetShelfComponent.h"

PresetShelfComponent::IconButton::IconButton(PresetActionIcon iconType)
    : juce::Button({}),
      icon(iconType)
{
}

void PresetShelfComponent::IconButton::paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    auto bounds = getLocalBounds().toFloat().reduced(0.5f);
    auto base = Theme::panelRaised.brighter(isMouseOverButton ? 0.18f : 0.10f);
    if (isButtonDown)
        base = base.brighter(0.08f);

    g.setColour(base);
    g.fillRoundedRectangle(bounds, 7.0f);
    g.setColour(Theme::panelOutline.withAlpha(0.9f));
    g.drawRoundedRectangle(bounds, 7.0f, 1.0f);

    auto iconArea = bounds.reduced(5.0f);
    juce::Path iconPath;

    switch (icon)
    {
        case PresetActionIcon::edit:
        {
            iconPath.startNewSubPath(iconArea.getX() + 2.0f, iconArea.getBottom() - 2.0f);
            iconPath.lineTo(iconArea.getX() + 5.0f, iconArea.getBottom() - 5.0f);
            iconPath.lineTo(iconArea.getRight() - 3.0f, iconArea.getY() + 3.0f);
            iconPath.lineTo(iconArea.getRight() - 1.5f, iconArea.getY() + 4.5f);
            iconPath.lineTo(iconArea.getX() + 4.0f, iconArea.getBottom() - 1.5f);
            iconPath.closeSubPath();
            break;
        }

        case PresetActionIcon::save:
        {
            iconPath.addRoundedRectangle(iconArea, 2.0f);
            iconPath.addRectangle(iconArea.getX() + 3.0f, iconArea.getY() + 2.0f, iconArea.getWidth() - 6.0f, 3.0f);
            iconPath.addRectangle(iconArea.getX() + 4.0f, iconArea.getCentreY(), iconArea.getWidth() - 8.0f, iconArea.getHeight() * 0.28f);
            break;
        }

        case PresetActionIcon::remove:
        {
            iconPath.startNewSubPath(iconArea.getX() + 3.0f, iconArea.getY() + 3.0f);
            iconPath.lineTo(iconArea.getRight() - 3.0f, iconArea.getBottom() - 3.0f);
            iconPath.startNewSubPath(iconArea.getRight() - 3.0f, iconArea.getY() + 3.0f);
            iconPath.lineTo(iconArea.getX() + 3.0f, iconArea.getBottom() - 3.0f);
            break;
        }
    }

    g.setColour(icon == PresetActionIcon::remove ? Theme::danger : Theme::textPrimary);
    g.strokePath(iconPath, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

PresetShelfComponent::SlotCard::SlotCard(int slotIndex)
    : index(slotIndex)
{
    numberLabel.setText(juce::String::charToString(static_cast<juce::juce_wchar>('A' + slotIndex)), juce::dontSendNotification);
    numberLabel.setJustificationType(juce::Justification::centred);
    numberLabel.setColour(juce::Label::textColourId, Theme::getPresetAccent(slotIndex));
    numberLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    addAndMakeVisible(numberLabel);

    nameLabel.setEditable(false, true, false);
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    nameLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    nameLabel.setColour(juce::Label::textColourId, Theme::getPresetAccent(slotIndex));
    nameLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    nameLabel.setMinimumHorizontalScale(0.8f);
    addAndMakeVisible(nameLabel);

    modelLabel.setJustificationType(juce::Justification::centredLeft);
    modelLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    modelLabel.setFont(juce::Font(10.5f, juce::Font::plain));
    modelLabel.setMinimumHorizontalScale(0.75f);
    addAndMakeVisible(modelLabel);

    irLabel.setJustificationType(juce::Justification::centredLeft);
    irLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    irLabel.setFont(juce::Font(10.5f, juce::Font::plain));
    irLabel.setMinimumHorizontalScale(0.75f);
    addAndMakeVisible(irLabel);

    editButton.onClick = [this]() { beginEditing(); };
    saveButton.onClick = [this]()
    {
        if (auto* shelf = findParentComponentOfClass<PresetShelfComponent>())
        {
            auto rawName = nameLabel.getText().trim();
            if (rawName.isEmpty())
                rawName = juce::String::charToString(static_cast<juce::juce_wchar>('A' + index)) + " Preset";

            if (shelf->onNameChanged)
                shelf->onNameChanged(index, rawName);
            if (shelf->onStoreClicked)
                shelf->onStoreClicked(index);
        }

        finishEditing(true);
    };
    clearButton.onClick = [this]()
    {
        if (auto* shelf = findParentComponentOfClass<PresetShelfComponent>())
        {
            if (shelf->onClearClicked)
                shelf->onClearClicked(index);
        }

        finishEditing(false);
    };

    addAndMakeVisible(editButton);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(clearButton);

    for (auto* button : { &editButton, &saveButton, &clearButton })
    {
        button->setClickingTogglesState(false);
    }

    editButton.setTooltip("Rename preset");
    saveButton.setTooltip("Save current chain");
    clearButton.setTooltip("Delete preset");
}

void PresetShelfComponent::SlotCard::setSlotData(const PresetShelfSlotView& view)
{
    occupied = view.occupied;
    const auto slotName = occupied
        ? (view.name.isEmpty() ? "CLEAN" : view.name.toUpperCase())
        : "EMPTY SLOT";

    nameLabel.setText(slotName, juce::dontSendNotification);
    nameLabel.setTooltip(occupied ? view.name : "Empty preset slot");
    modelLabel.setText("NAM: " + (occupied ? view.modelName : "No model"), juce::dontSendNotification);
    irLabel.setText("IR: " + (occupied ? view.irName : "No IR"), juce::dontSendNotification);
    finishEditing(false);
}

void PresetShelfComponent::SlotCard::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const auto accent = Theme::getPresetAccent(index);

    auto* shelf = findParentComponentOfClass<PresetShelfComponent>();
    const bool isActive = (shelf != nullptr && shelf->activeSlotIndex == index);

    auto stompbox = bounds;
    stompbox.removeFromBottom(16.0f);

    g.setColour(juce::Colours::black.withAlpha(0.22f));
    g.fillRoundedRectangle(stompbox.translated(0.0f, 3.0f), 12.0f);

    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff242424), stompbox.getCentreX(), stompbox.getY(),
                                           juce::Colour(0xff171717), stompbox.getCentreX(), stompbox.getBottom(), false));
    g.fillRoundedRectangle(stompbox, 12.0f);

    g.setColour(isActive ? accent : Theme::panelOutline.withAlpha(occupied ? 0.85f : 0.55f));
    g.drawRoundedRectangle(stompbox, 12.0f, isActive ? 2.0f : 1.1f);

    const auto inactiveAccent = occupied ? accent.withAlpha(0.45f) : Theme::textSecondary.withAlpha(0.35f);
    numberLabel.setColour(juce::Label::textColourId, isActive ? accent : inactiveAccent);
    nameLabel.setColour(juce::Label::textColourId, isActive ? Theme::textPrimary : Theme::textSecondary);

    auto footer = juce::Rectangle<float>(stompbox.getX() + 12.0f, stompbox.getBottom() - 28.0f, stompbox.getWidth() - 24.0f, 18.0f);
    g.setColour(Theme::panelOutline.withAlpha(0.35f));
    g.drawHorizontalLine(footer.getY(), footer.getX(), footer.getRight());

    auto bottomBadge = juce::Rectangle<float>(footer.getCentreX() - 16.0f, footer.getY() - 9.0f, 32.0f, 18.0f);
    g.setColour(Theme::backgroundBottom.withAlpha(0.96f));
    g.fillRoundedRectangle(bottomBadge, 8.0f);
    g.setColour(isActive ? accent : Theme::panelOutline.withAlpha(0.75f));
    g.drawRoundedRectangle(bottomBadge, 8.0f, 1.0f);
}

void PresetShelfComponent::SlotCard::resized()
{
    auto area = getLocalBounds();
    area.removeFromBottom(16);
    area.reduce(10, 8);

    auto topRow = area.removeFromTop(24);
    auto actions = topRow.removeFromRight(96);
    nameLabel.setBounds(topRow);

    area.removeFromTop(4);
    auto infoArea = area;
    modelLabel.setBounds(infoArea.removeFromTop(16));
    irLabel.setBounds(infoArea.removeFromTop(16));

    numberLabel.setBounds(getWidth() / 2 - 16, getHeight() - 42, 32, 18);

    const int buttonGap = 4;
    const int buttonWidth = (actions.getWidth() - buttonGap * 2) / 3;
    editButton.setBounds(actions.removeFromLeft(buttonWidth).reduced(0, 1));
    actions.removeFromLeft(buttonGap);
    saveButton.setBounds(actions.removeFromLeft(buttonWidth).reduced(0, 1));
    actions.removeFromLeft(buttonGap);
    clearButton.setBounds(actions.reduced(0, 1));
}

void PresetShelfComponent::SlotCard::mouseDown(const juce::MouseEvent& event)
{
    if (! editing)
    {
        if (auto* shelf = findParentComponentOfClass<PresetShelfComponent>())
        {
            if (shelf->onLoadClicked)
                shelf->onLoadClicked(index);
        }
    }
}

void PresetShelfComponent::SlotCard::mouseDoubleClick(const juce::MouseEvent&)
{
    beginEditing();
}

void PresetShelfComponent::SlotCard::beginEditing()
{
    editing = true;
    nameLabel.showEditor();
}

void PresetShelfComponent::SlotCard::finishEditing(bool keepChanges)
{
    juce::ignoreUnused(keepChanges);
    editing = false;
    nameLabel.hideEditor(true);
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

        addAndMakeVisible(cardPtr);
        slotCards.push_back(std::move(card));
    }
}

void PresetShelfComponent::setSlots(const std::vector<PresetShelfSlotView>& slotViews)
{
    const auto count = juce::jmin(static_cast<int>(slotViews.size()), static_cast<int>(slotCards.size()));
    for (int index = 0; index < count; ++index)
        slotCards[static_cast<size_t>(index)]->setSlotData(slotViews[static_cast<size_t>(index)]);

    repaint();
}

void PresetShelfComponent::setActiveSlot(int index)
{
    if (index >= 0 && index < static_cast<int>(slotCards.size()))
    {
        activeSlotIndex = index;
        for (auto& card : slotCards)
            card->repaint();
    }
}

void PresetShelfComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    g.setGradientFill(juce::ColourGradient(Theme::panelRaised.withAlpha(0.5f), bounds.getCentreX(), bounds.getY(),
                                           Theme::panel.withAlpha(0.7f), bounds.getCentreX(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, 16.0f);

    g.setColour(Theme::panelOutline.withAlpha(0.75f));
    g.drawRoundedRectangle(bounds, 16.0f, 1.0f);

    g.setColour(Theme::panelOutline.withAlpha(0.35f));
    g.drawHorizontalLine(42.0f, 16.0f, bounds.getRight() - 16.0f);
}

void PresetShelfComponent::resized()
{
    auto area = getLocalBounds().reduced(14);

    sectionLabel.setBounds(area.removeFromTop(18));
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
