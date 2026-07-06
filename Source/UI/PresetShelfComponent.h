#pragma once

#include <JuceHeader.h>

#include "GoldLookAndFeel.h"

struct PresetShelfSlotView
{
    juce::String name;
    juce::String modelName;
    juce::String irName;
    bool occupied = false;
};

class PresetShelfComponent : public juce::Component
{
public:
    explicit PresetShelfComponent(int slotCount);

    std::function<void(int)> onLoadClicked;
    std::function<void(int)> onStoreClicked;
    std::function<void(int)> onClearClicked;
    std::function<void(int, const juce::String&)> onNameChanged;

    void setSlots(const std::vector<PresetShelfSlotView>& slotViews);
    void setActiveSlot(int index);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    enum class PresetActionIcon
    {
        edit,
        save,
        remove
    };

    class IconButton : public juce::Button
    {
    public:
        explicit IconButton(PresetActionIcon iconType);

        void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;

    private:
        PresetActionIcon icon;
    };

    struct SlotCard : public juce::Component
    {
        explicit SlotCard(int slotIndex);

        void setSlotData(const PresetShelfSlotView& view);
        void paint(juce::Graphics& g) override;
        void resized() override;
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDoubleClick(const juce::MouseEvent& event) override;

        int index = 0;
        bool occupied = false;
        bool editing = false;
        juce::Label numberLabel;
        juce::Label nameLabel;
        juce::Label modelLabel;
        juce::Label irLabel;
        IconButton editButton { PresetActionIcon::edit };
        IconButton saveButton { PresetActionIcon::save };
        IconButton clearButton { PresetActionIcon::remove };

    private:
        void beginEditing();
        void finishEditing(bool keepChanges);
    };

    juce::Label sectionLabel;
    std::vector<std::unique_ptr<SlotCard>> slotCards;
    int activeSlotIndex = 0; // Default active slot is A (index 0)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetShelfComponent)
};
