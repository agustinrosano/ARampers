#include "PedalModuleComponent.h"
#include "GoldLookAndFeel.h"

PedalModuleComponent::PedalModuleComponent(juce::AudioProcessorValueTreeState& state,
                                           const juce::String& title,
                                           const juce::String& bypassParamId,
                                           std::initializer_list<KnobConfig> knobs)
    : apvts(state)
{
    titleLabel.setText(title, juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, Theme::textPrimary);
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
        control->label.setColour(juce::Label::textColourId, Theme::textSecondary);

        addAndMakeVisible(control->label);
        addAndMakeVisible(control->slider);

        control->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, knob.paramId, control->slider);

        knobControls.push_back(std::move(control));
    }
}

void PedalModuleComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    g.setGradientFill(juce::ColourGradient(Theme::panelRaised.brighter(0.08f), bounds.getX(), bounds.getY(),
                                           Theme::panel.darker(0.08f), bounds.getRight(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, 20.0f);
    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(bounds, 20.0f, 1.0f);

    g.setColour(Theme::accent.withAlpha(0.08f));
    g.fillRoundedRectangle(bounds.removeFromTop(30.0f), 20.0f);
}

void PedalModuleComponent::configureSlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.18f,
                               juce::MathConstants<float>::pi * 2.82f,
                               true);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 20);
}

void PedalModuleComponent::resized()
{
    auto area = getLocalBounds().reduced(12);
    auto header = area.removeFromTop(30);

    auto titleArea = header.removeFromLeft(juce::jmax(120, header.getWidth() / 2));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setBounds(titleArea);
    bypassButton.setBounds(header.removeFromRight(96).reduced(0, 2));

    area.removeFromTop(10);

    if (knobControls.empty())
        return;

    auto knobArea = area.reduced(4, 0);

    if (knobControls.size() <= 2)
    {
        const int columns = static_cast<int>(knobControls.size());
        const int gap = 24;
        const int knobWidth = (knobArea.getWidth() - gap * (columns - 1)) / columns;

        for (auto& knob : knobControls)
        {
            auto slot = knobArea.removeFromLeft(knobWidth);
            knob->label.setBounds(slot.removeFromTop(18));
            knob->slider.setBounds(slot.reduced(2, 0));

            if (knobArea.getWidth() > 0)
                knobArea.removeFromLeft(gap);
        }

        return;
    }

    const int gap = 16;
    const int knobWidth = (knobArea.getWidth() - gap * (static_cast<int>(knobControls.size()) - 1))
                        / static_cast<int>(knobControls.size());

    for (auto& knob : knobControls)
    {
        auto slot = knobArea.removeFromLeft(knobWidth);
        knob->label.setBounds(slot.removeFromTop(16));
        knob->slider.setBounds(slot.reduced(4));

        if (knobArea.getWidth() > 0)
            knobArea.removeFromLeft(gap);
    }
}
