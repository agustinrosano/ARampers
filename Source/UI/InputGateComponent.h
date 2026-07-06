#pragma once

#include <JuceHeader.h>

class InputGateComponent : public juce::Component
{
public:
    InputGateComponent(juce::AudioProcessorValueTreeState& state,
                       const juce::String& bypassParamId,
                       const juce::String& thresholdParamId,
                       const juce::String& releaseParamId);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void configureSlider(juce::Slider& slider);

    juce::Label titleLabel;
    juce::ToggleButton bypassButton { "Bypass" };

    juce::Label thresholdLabel { {}, "Threshold" };
    juce::Slider thresholdSlider;

    juce::Label releaseLabel { {}, "Release" };
    juce::Slider releaseSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InputGateComponent)
};
