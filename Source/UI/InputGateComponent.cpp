#include "InputGateComponent.h"

#include "GoldLookAndFeel.h"

InputGateComponent::InputGateComponent(juce::AudioProcessorValueTreeState& state,
                                       const juce::String& bypassParamId,
                                       const juce::String& thresholdParamId,
                                       const juce::String& releaseParamId)
{
    titleLabel.setText("INPUT GATE", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, Theme::textPrimary);
    titleLabel.setFont(juce::Font(13.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, bypassParamId, bypassButton);

    configureSlider(thresholdSlider);
    configureSlider(releaseSlider);

    thresholdLabel.setJustificationType(juce::Justification::centredLeft);
    thresholdLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    thresholdLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    addAndMakeVisible(thresholdLabel);

    releaseLabel.setJustificationType(juce::Justification::centredLeft);
    releaseLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    releaseLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    addAndMakeVisible(releaseLabel);

    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(releaseSlider);

    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, thresholdParamId, thresholdSlider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, releaseParamId, releaseSlider);
}

void InputGateComponent::paint(juce::Graphics& g)
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

void InputGateComponent::resized()
{
    auto area = getLocalBounds().reduced(14, 12);

    auto header = area.removeFromTop(26);
    titleLabel.setBounds(header.removeFromLeft(160));
    bypassButton.setBounds(header.removeFromRight(96));

    area.removeFromTop(10);

    auto layoutRow = [](juce::Rectangle<int> row, juce::Label& label, juce::Slider& slider)
    {
        auto labelArea = row.removeFromLeft(90);
        label.setBounds(labelArea);
        row.removeFromLeft(10);
        slider.setBounds(row);
    };

    const bool stacked = getWidth() < 720;

    if (stacked)
    {
        auto firstRow = area.removeFromTop(28);
        layoutRow(firstRow, thresholdLabel, thresholdSlider);
        area.removeFromTop(10);
        auto secondRow = area.removeFromTop(28);
        layoutRow(secondRow, releaseLabel, releaseSlider);
        return;
    }

    auto left = area.removeFromLeft((area.getWidth() - 20) / 2);
    area.removeFromLeft(20);
    auto right = area;
    layoutRow(left.removeFromTop(28), thresholdLabel, thresholdSlider);
    layoutRow(right.removeFromTop(28), releaseLabel, releaseSlider);
}

void InputGateComponent::configureSlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 20);
    slider.setNumDecimalPlacesToDisplay(1);
}
