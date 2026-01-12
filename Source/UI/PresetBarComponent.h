#pragma once

#include <JuceHeader.h>

class PresetBarComponent : public juce::Component
{
public:
    PresetBarComponent();

    void setPresetNames(const juce::StringArray& names);
    int getSelectedIndex() const;
    void setSelectedIndex(int index);

    std::function<void(int)> onPresetSelected;
    std::function<void()> onSaveClicked;
    std::function<void()> onLoadClicked;
    std::function<void()> onPrevClicked;
    std::function<void()> onNextClicked;

    void resized() override;

private:
    juce::ComboBox presetBox;
    juce::TextButton saveButton { "Save" };
    juce::TextButton loadButton { "Load" };
    juce::TextButton prevButton { "<" };
    juce::TextButton nextButton { ">" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBarComponent)
};
