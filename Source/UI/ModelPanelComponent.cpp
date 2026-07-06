#include "ModelPanelComponent.h"
#include "GoldLookAndFeel.h"

ModelPanelComponent::ModelPanelComponent(juce::AudioProcessorValueTreeState& state, const juce::String& bypassParamId)
{
    eyebrowLabel.setText("NAM (Neural Amp Modeler)", juce::dontSendNotification);
    eyebrowLabel.setColour(juce::Label::textColourId, Theme::textPrimary);
    eyebrowLabel.setJustificationType(juce::Justification::centredLeft);
    eyebrowLabel.setFont(juce::Font(15.0f, juce::Font::bold));
    addAndMakeVisible(eyebrowLabel);

    modelNameLabel.setText("NAM: No model loaded", juce::dontSendNotification);
    modelNameLabel.setJustificationType(juce::Justification::centredLeft);
    modelNameLabel.setFont(juce::Font(13.0f, juce::Font::plain));
    modelNameLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    addAndMakeVisible(modelNameLabel);

    architectureLabel.setJustificationType(juce::Justification::centredLeft);
    architectureLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    architectureLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    addAndMakeVisible(architectureLabel);

    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setFont(juce::Font(11.0f, juce::Font::plain));
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

    bypassButton.setButtonText("ON");
    addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, bypassParamId, bypassButton);
}

void ModelPanelComponent::setModelName(const juce::String& name)
{
    modelNameLabel.setText("NAM: " + (name.isEmpty() ? "No model loaded" : name), juce::dontSendNotification);
}

void ModelPanelComponent::setArchitecture(const juce::String& architectureName)
{
    architectureLabel.setText(architectureName, juce::dontSendNotification);
}

void ModelPanelComponent::setStatusText(const juce::String& status, bool isError)
{
    hasError = isError;
    statusLabel.setText(status, juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, isError ? Theme::danger : Theme::textSecondary);
    repaint();
}

void ModelPanelComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    // Draw background panel
    g.setGradientFill(juce::ColourGradient(Theme::panelRaised, bounds.getX(), bounds.getY(),
                                           Theme::panel, bounds.getRight(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, 12.0f);

    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(bounds, 12.0f, 1.2f);

    // Draw Amp Head Icon on the left
    auto iconRect = juce::Rectangle<float>(14.0f, (getHeight() - 32.0f) / 2.0f, 44.0f, 32.0f);
    g.setColour(Theme::textSecondary);
    // Outer frame
    g.drawRoundedRectangle(iconRect, 3.0f, 1.5f);
    // Control panel (bottom half faceplate)
    auto faceplate = iconRect.removeFromBottom(12.0f).reduced(2.0f, 1.0f);
    g.setColour(Theme::panelOutline);
    g.fillRoundedRectangle(faceplate, 1.0f);
    g.setColour(Theme::textSecondary);
    // Knobs on faceplate
    for (float x = faceplate.getX() + 4.0f; x < faceplate.getRight() - 2.0f; x += 6.0f)
        g.fillEllipse(x, faceplate.getCentreY() - 1.5f, 3.0f, 3.0f);
    // Vent grille on top half
    auto vent = iconRect.reduced(6.0f, 4.0f);
    g.setColour(Theme::panelOutline.darker(0.5f));
    g.fillRect(vent);

    // Draw LEDs (small vertical indicators on the right, next to buttons)
    auto ledArea = juce::Rectangle<float>(static_cast<float>(getWidth()) - 190.0f, 14.0f, 10.0f, 22.0f);
    const juce::Colour ledColours[] = { Theme::success, Theme::presetBlue, Theme::presetPurple };
    for (int i = 0; i < 3; ++i)
    {
        auto led = juce::Rectangle<float>(ledArea.getX(), ledArea.getY() + i * 8.0f, 6.0f, 6.0f);
        g.setColour(ledColours[i].withAlpha(0.18f));
        g.fillEllipse(led.expanded(3.0f));
        g.setColour(ledColours[i]);
        g.fillEllipse(led);
    }
}

void ModelPanelComponent::resized()
{
    auto area = getLocalBounds().reduced(12);

    area.removeFromLeft(54);

    auto rightArea = area.removeFromRight(168);
    auto toggleArea = rightArea.removeFromRight(50).reduced(0, 4);
    bypassButton.setBounds(toggleArea);

    auto clearArea = rightArea.removeFromRight(54).reduced(0, 6);
    clearButton.setBounds(clearArea);

    auto loadArea = rightArea.removeFromRight(55).reduced(0, 6);
    loadButton.setBounds(loadArea);

    area.removeFromRight(20);

    eyebrowLabel.setBounds(area.removeFromTop(18));
    modelNameLabel.setBounds(area.removeFromTop(18));
    architectureLabel.setBounds(area.removeFromTop(16));
    statusLabel.setBounds(area);
}
