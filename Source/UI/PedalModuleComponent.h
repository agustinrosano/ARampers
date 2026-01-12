#pragma once

#include <JuceHeader.h>

struct KnobConfig
{
    juce::String label;
    juce::String paramId;
};

class PedalModuleComponent : public juce::Component
{
public:
    PedalModuleComponent(juce::AudioProcessorValueTreeState& state,
                         const juce::String& title,
                         const juce::String& bypassParamId,
                         std::initializer_list<KnobConfig> knobs);

    void resized() override;

private:
    struct KnobControl
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    void configureSlider(juce::Slider& slider);

    juce::AudioProcessorValueTreeState& apvts;
    juce::Label titleLabel;
    juce::ToggleButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::vector<std::unique_ptr<KnobControl>> knobControls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PedalModuleComponent)
};
