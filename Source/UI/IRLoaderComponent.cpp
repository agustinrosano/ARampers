#include "IRLoaderComponent.h"
#include "GoldLookAndFeel.h"

IRLoaderComponent::IRLoaderComponent(juce::AudioProcessorValueTreeState& state, const juce::String& bypassParamId)
{
    titleLabel.setText("IR (Impulse Response)", juce::dontSendNotification);
    titleLabel.setColour(juce::Label::textColourId, Theme::textPrimary);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::Font(15.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    fileLabel.setText("IR: No IR loaded", juce::dontSendNotification);
    fileLabel.setJustificationType(juce::Justification::centredLeft);
    fileLabel.setFont(juce::Font(13.0f, juce::Font::plain));
    fileLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    fileLabel.setMinimumHorizontalScale(0.8f);
    addAndMakeVisible(fileLabel);

    for (auto* label : { &onLabel, &offLabel })
    {
        label->setJustificationType(juce::Justification::centred);
        label->setFont(juce::Font(11.0f, juce::Font::bold));
        addAndMakeVisible(*label);
    }

    addAndMakeVisible(loadButton);
    addAndMakeVisible(clearButton);

    loadButton.onClick = [this]()
    {
        if (onLoadClicked)
            onLoadClicked();
    };

    clearButton.onClick = [this]()
    {
        if (onClearClicked)
            onClearClicked();
    };

    bypassButton.onStateChange = [this]() { refreshBypassState(); };
    addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, bypassParamId, bypassButton);
    refreshBypassState();
}

void IRLoaderComponent::setFileName(const juce::String& name)
{
    if (name.isEmpty() || name == "No IR loaded")
        fileLabel.setText("IR: No IR loaded", juce::dontSendNotification);
    else
        fileLabel.setText("IR: " + name, juce::dontSendNotification);

    fileLabel.setTooltip(name);
}

void IRLoaderComponent::refreshBypassState()
{
    const bool bypassed = bypassButton.getToggleState();
    onLabel.setColour(juce::Label::textColourId, bypassed ? Theme::textSecondary : Theme::success);
    offLabel.setColour(juce::Label::textColourId, bypassed ? Theme::danger : Theme::textSecondary);
}

void IRLoaderComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    g.setGradientFill(juce::ColourGradient(Theme::panelRaised, bounds.getX(), bounds.getY(),
                                           Theme::panel, bounds.getRight(), bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, 12.0f);
    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(bounds, 12.0f, 1.2f);

    // Draw Speaker Cabinet Icon on the left
    auto iconRect = juce::Rectangle<float>(14.0f, (getHeight() - 32.0f) / 2.0f, 32.0f, 32.0f);
    g.setColour(Theme::textSecondary);
    // Outer cabinet box
    g.drawRoundedRectangle(iconRect, 3.0f, 1.5f);
    // Inner grille area
    auto grill = iconRect.reduced(3.0f);
    g.setColour(Theme::panelOutline);
    g.fillRoundedRectangle(grill, 1.0f);
    g.setColour(Theme::textSecondary);
    // Speaker circle (cone)
    g.drawEllipse(grill.getCentreX() - 8.0f, grill.getCentreY() - 8.0f, 16.0f, 16.0f, 1.2f);
    g.fillEllipse(grill.getCentreX() - 3.0f, grill.getCentreY() - 3.0f, 6.0f, 6.0f);

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

void IRLoaderComponent::resized()
{
    auto area = getLocalBounds().reduced(12);

    const int iconWidth = juce::jmin(52, juce::jmax(44, getWidth() / 11));
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

    auto labelHeight = area.getHeight() / 2;
    titleLabel.setBounds(area.removeFromTop(labelHeight).translated(0, 2));
    fileLabel.setBounds(area.translated(0, -2));
}
