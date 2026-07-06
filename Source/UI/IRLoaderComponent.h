#pragma once

#include <JuceHeader.h>

class IRLoaderComponent : public juce::Component
{
public:
    IRLoaderComponent(juce::AudioProcessorValueTreeState& state, const juce::String& bypassParamId);

    std::function<void()> onLoadClicked;
    std::function<void()> onClearClicked;
    void setFileName(const juce::String& name);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void refreshBypassState();

    juce::Label titleLabel;
    juce::Label onLabel { {}, "ON" };
    juce::Label offLabel { {}, "OFF" };
    juce::TextButton loadButton { "LOAD..." };
    juce::TextButton clearButton { "Clear" };
    juce::Label fileLabel;
    juce::ToggleButton bypassButton { {} };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IRLoaderComponent)
};
