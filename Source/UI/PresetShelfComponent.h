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
    struct SlotCard : public juce::Component
    {
        explicit SlotCard(int slotIndex);

        void setSlotData(const PresetShelfSlotView& view);
        void paint(juce::Graphics& g) override;
        void resized() override;
        void mouseDown(const juce::MouseEvent& event) override;

        int index = 0;
        bool occupied = false;
        juce::Label numberLabel;
        juce::Label nameLabel;
        juce::Label modelLabel;
        juce::Label irLabel;
        juce::TextButton loadButton { "Load" };
        juce::TextButton storeButton { "Store" };
        juce::TextButton clearButton { "Clear" };
    };

    juce::Label sectionLabel;
    std::vector<std::unique_ptr<SlotCard>> slotCards;
    int activeSlotIndex = 0; // Default active slot is A (index 0)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetShelfComponent)
};
