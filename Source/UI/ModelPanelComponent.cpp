#include "ModelPanelComponent.h"
#include "GoldLookAndFeel.h"

ModelPanelComponent::ModelPanelComponent(juce::AudioProcessorValueTreeState& state, const juce::String& bypassParamId)
{
    eyebrowLabel.setText("NAM (Neural Amp Modeler)", juce::dontSendNotification);
    eyebrowLabel.setColour(juce::Label::textColourId, Theme::accent);
    eyebrowLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(eyebrowLabel);

    modelNameLabel.setText("No model loaded", juce::dontSendNotification);
    modelNameLabel.setJustificationType(juce::Justification::centredLeft);
    modelNameLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    addAndMakeVisible(modelNameLabel);

    architectureLabel.setText("Primary amp capture", juce::dontSendNotification);
    architectureLabel.setJustificationType(juce::Justification::centredLeft);
    architectureLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    addAndMakeVisible(architectureLabel);

    statusLabel.setText("Load a .nam file to set the amp voice", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    addAndMakeVisible(statusLabel);

    loadButton.onClick = [this]()
    {
        if (onLoadClicked)
            onLoadClicked();
    };
    addAndMakeVisible(loadButton);

    clearButton.onClick = [this]()
    {
        if (onClearClicked)
            onClearClicked();
    };
    addAndMakeVisible(clearButton);

    addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, bypassParamId, bypassButton);
}

void ModelPanelComponent::setModelName(const juce::String& name)
{
    modelNameLabel.setText(name.isEmpty() ? "No model loaded" : name, juce::dontSendNotification);
}

void ModelPanelComponent::setArchitecture(const juce::String& architectureName)
{
    architectureLabel.setText(architectureName.isEmpty() ? "Primary amp capture" : architectureName,
                              juce::dontSendNotification);
}

void ModelPanelComponent::setStatusText(const juce::String& status, bool isError)
{
    hasError = isError;
    statusLabel.setText(status.isEmpty() ? "Ready" : status, juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, hasError ? Theme::danger.brighter(0.25f)
                                                              : Theme::textSecondary);
    repaint();
}

void ModelPanelComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    g.setGradientFill(juce::ColourGradient(Theme::panelRaised.brighter(0.12f), bounds.getX(), bounds.getY(),
                                           Theme::panel.darker(0.12f), bounds.getRight(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, 24.0f);

    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(bounds, 24.0f, 1.2f);

    auto glow = bounds.reduced(16.0f);
    const auto glowColour = hasError ? Theme::danger.withAlpha(0.16f) : Theme::accent.withAlpha(0.16f);
    g.setGradientFill(juce::ColourGradient(glowColour, glow.getX(), glow.getY(),
                                           juce::Colours::transparentBlack, glow.getRight(), glow.getBottom(), false));
    g.fillRoundedRectangle(glow.removeFromTop(90.0f), 18.0f);

    auto ledArea = bounds.removeFromTop(28.0f).removeFromRight(68.0f).reduced(8.0f, 4.0f);
    const juce::Colour ledColours[] = { Theme::success, Theme::presetBlue, Theme::presetPurple };
    for (int i = 0; i < 3; ++i)
    {
        auto led = ledArea.removeFromLeft(14.0f).withSizeKeepingCentre(8.0f, 8.0f);
        g.setColour(ledColours[i].withAlpha(0.18f));
        g.fillEllipse(led.expanded(5.0f));
        g.setColour(ledColours[i]);
        g.fillEllipse(led);
    }
}

void ModelPanelComponent::resized()
{
    auto area = getLocalBounds().reduced(20);
    auto header = area.removeFromTop(30);
    eyebrowLabel.setBounds(header.removeFromLeft(240));

    auto buttonRow = area.removeFromBottom(46);
    bypassButton.setBounds(buttonRow.removeFromLeft(180));
    clearButton.setBounds(buttonRow.removeFromRight(92));
    buttonRow.removeFromRight(8);
    loadButton.setBounds(buttonRow.removeFromRight(132));

    modelNameLabel.setBounds(area.removeFromTop(54));
    architectureLabel.setBounds(area.removeFromTop(26));
    area.removeFromTop(12);
    statusLabel.setBounds(area.removeFromTop(48));
}
