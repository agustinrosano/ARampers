#pragma once

#include <JuceHeader.h>

class ModelPanelComponent : public juce::Component
{
public:
    ModelPanelComponent(juce::AudioProcessorValueTreeState& state, const juce::String& bypassParamId);

    std::function<void()> onLoadClicked;
    std::function<void()> onClearClicked;

    void setModelName(const juce::String& name);
    void setArchitecture(const juce::String& architectureName);
    void setStatusText(const juce::String& status, bool isError);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label eyebrowLabel;
    juce::Label modelNameLabel;
    juce::Label architectureLabel;
    juce::Label statusLabel;
    juce::TextButton loadButton { "LOAD..." };
    juce::TextButton clearButton { "Clear" };
    juce::ToggleButton bypassButton { "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    bool hasError = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModelPanelComponent)
};
