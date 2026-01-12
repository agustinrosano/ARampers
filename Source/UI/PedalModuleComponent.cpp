#include "PedalModuleComponent.h"

PedalModuleComponent::PedalModuleComponent(juce::AudioProcessorValueTreeState& state,
                                           const juce::String& title,
                                           const juce::String& bypassParamId,
                                           std::initializer_list<KnobConfig> knobs)
    : apvts(state)
{
    titleLabel.setText(title, juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    bypassButton.setButtonText("Bypass");
    addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, bypassParamId, bypassButton);

    for (const auto& knob : knobs)
    {
        auto control = std::make_unique<KnobControl>();

        configureSlider(control->slider);
        control->label.setText(knob.label, juce::dontSendNotification);
        control->label.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(control->label);
        addAndMakeVisible(control->slider);

        control->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, knob.paramId, control->slider);

        knobControls.push_back(std::move(control));
    }
}

void PedalModuleComponent::configureSlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
}

void PedalModuleComponent::resized()
{
    auto area = getLocalBounds().reduced(6);
    auto header = area.removeFromTop(40);

    titleLabel.setBounds(header.removeFromTop(20));
    bypassButton.setBounds(header.removeFromTop(20));

    if (knobControls.empty())
        return;

    auto knobArea = area;
    const int knobWidth = knobArea.getWidth() / static_cast<int>(knobControls.size());

    for (auto& knob : knobControls)
    {
        auto slot = knobArea.removeFromLeft(knobWidth);
        knob->label.setBounds(slot.removeFromTop(16));
        knob->slider.setBounds(slot.reduced(4));
    }
}
