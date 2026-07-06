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
    modelNameLabel.setMinimumHorizontalScale(0.8f);
    addAndMakeVisible(modelNameLabel);

    architectureLabel.setJustificationType(juce::Justification::centredLeft);
    architectureLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    architectureLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    architectureLabel.setMinimumHorizontalScale(0.85f);
    addAndMakeVisible(architectureLabel);

    statusLabel.setJustificationType(juce::Justification::centredLeft);
    statusLabel.setFont(juce::Font(11.0f, juce::Font::plain));
    statusLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    statusLabel.setMinimumHorizontalScale(0.85f);
    addAndMakeVisible(statusLabel);

    for (auto* label : { &onLabel, &offLabel })
    {
        label->setJustificationType(juce::Justification::centred);
        label->setFont(juce::Font(11.0f, juce::Font::bold));
        addAndMakeVisible(*label);
    }

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

    bypassButton.onStateChange = [this]() { refreshBypassState(); };
    addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, bypassParamId, bypassButton);
    refreshBypassState();
}

void ModelPanelComponent::setModelName(const juce::String& name)
{
    const auto displayName = name.isEmpty() ? "No model loaded" : name;
    modelNameLabel.setText("NAM: " + displayName, juce::dontSendNotification);
    modelNameLabel.setTooltip(displayName);
}

void ModelPanelComponent::setArchitecture(const juce::String& architectureName)
{
    architectureLabel.setText(architectureName, juce::dontSendNotification);
    architectureLabel.setTooltip(architectureName);
}

void ModelPanelComponent::setStatusText(const juce::String& status, bool isError)
{
    hasError = isError;
    statusLabel.setText(status, juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, isError ? Theme::danger : Theme::textSecondary);
    statusLabel.setTooltip(status);
    repaint();
}

void ModelPanelComponent::refreshBypassState()
{
    const bool bypassed = bypassButton.getToggleState();
    onLabel.setColour(juce::Label::textColourId, bypassed ? Theme::textSecondary : Theme::success);
    offLabel.setColour(juce::Label::textColourId, bypassed ? Theme::danger : Theme::textSecondary);
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

    const int iconWidth = juce::jmin(60, juce::jmax(48, getWidth() / 10));
    area.removeFromLeft(iconWidth);

    const bool compact = getWidth() < 520;
    const int controlsWidth = compact ? juce::jmin(170, juce::jmax(136, getWidth() / 3))
                                      : juce::jmin(240, juce::jmax(188, getWidth() / 3));
    auto rightArea = area.removeFromRight(controlsWidth);
    auto topControls = rightArea.removeFromTop(28);

    if (compact)
    {
        auto loadArea = topControls.removeFromLeft(topControls.getWidth() / 2);
        loadButton.setBounds(loadArea.reduced(0, 2));
        clearButton.setBounds(topControls.reduced(0, 2));
        rightArea.removeFromTop(8);
        auto toggleRow = rightArea.removeFromTop(24);
        offLabel.setBounds(toggleRow.removeFromRight(28));
        bypassButton.setBounds(toggleRow.removeFromRight(48));
        onLabel.setBounds(toggleRow.removeFromRight(28));
    }
    else
    {
        auto loadArea = topControls.removeFromLeft(juce::jmax(72, topControls.getWidth() / 2));
        loadButton.setBounds(loadArea.reduced(0, 2));

        topControls.removeFromLeft(8);

        auto clearArea = topControls;
        clearButton.setBounds(clearArea.reduced(0, 2));

        rightArea.removeFromTop(10);
        auto toggleRow = rightArea.removeFromTop(26).removeFromRight(116);
        offLabel.setBounds(toggleRow.removeFromRight(32));
        bypassButton.setBounds(toggleRow.removeFromRight(52));
        onLabel.setBounds(toggleRow.removeFromRight(32));
    }

    area.removeFromRight(juce::jmax(10, getWidth() / 40));

    eyebrowLabel.setBounds(area.removeFromTop(18));
    modelNameLabel.setBounds(area.removeFromTop(18));
    architectureLabel.setBounds(area.removeFromTop(16));
    statusLabel.setBounds(area);
}
