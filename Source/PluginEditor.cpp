#include "PluginEditor.h"

#if JucePlugin_Build_Standalone
 #include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

GoldPedalAudioProcessorEditor::GoldPedalAudioProcessorEditor(GoldPedalAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      apvts(p.getAPVTS()),
      modelPanel(apvts, ParamIDs::modelBypass),
      irLoader(apvts, ParamIDs::irBypass),
      presetShelf(p.getNumUserPresetSlots()),
      irModule(apvts, "CAB BLEND", ParamIDs::irBypass, { { "Blend", ParamIDs::irMix } }),
      eqModule(apvts, "EQ", ParamIDs::eqBypass,
               { { "Low", ParamIDs::eqLow }, { "Mid", ParamIDs::eqMid }, { "High", ParamIDs::eqHigh } })
{
    setLookAndFeel(&lookAndFeel);
    setSize(1240, 820);
    setResizable(true, true);
    setResizeLimits(900, 680, 1600, 1100);

    titleLabel.setText("ADR - AMPER", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::Font(29.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, Theme::textPrimary);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("Load a NAM capture, pair it with an IR, then shape the level and EQ below.",
                          juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centredLeft);
    subtitleLabel.setFont(juce::Font(13.0f, juce::Font::plain));
    subtitleLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    addAndMakeVisible(subtitleLabel);

    menuButton.setColour(juce::TextButton::textColourOffId, Theme::textPrimary);
    menuButton.onClick = [this]()
    {
        juce::PopupMenu menu;

       #if JucePlugin_Build_Standalone
        menu.addItem(1, "Audio/MIDI Settings...");
        menu.addSeparator();
       #endif

        menu.addItem(2, "Save Preset...");
        menu.addItem(3, "Load Preset...");

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&menuButton),
                           [this](int result)
        {
            switch (result)
            {
               #if JucePlugin_Build_Standalone
                case 1:
                    if (auto* holder = juce::StandalonePluginHolder::getInstance())
                        holder->showAudioSettingsDialog();
                    break;
               #endif
                case 2:
                    showSavePresetDialog();
                    break;
                case 3:
                    showLoadPresetDialog();
                    break;
                default:
                    break;
            }
        });
    };
    addAndMakeVisible(menuButton);

    aboutButton.setColour(juce::TextButton::textColourOffId, Theme::textPrimary);
    aboutButton.onClick = [this]()
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "ADR - AMPER",
            "ADR - AMPER\nNeural capture host for NAM models, IR and finishing tools.\n\nBuilt with JUCE.",
            "OK");
    };
    addAndMakeVisible(aboutButton);

    for (auto* label : { &inputMeterLabel, &masterVolumeLabel, &aboutLabel, &eqLineLabel, &nlmiiLineLabel, &eqActiveLabel })
    {
        label->setColour(juce::Label::textColourId, Theme::textSecondary);
        addAndMakeVisible(*label);
    }

    inputMeterLabel.setJustificationType(juce::Justification::centredLeft);
    inputMeterLabel.setFont(juce::Font(10.0f, juce::Font::plain));

    masterVolumeLabel.setJustificationType(juce::Justification::centredLeft);
    masterVolumeLabel.setFont(juce::Font(10.0f, juce::Font::plain));

    aboutLabel.setJustificationType(juce::Justification::centred);
    aboutLabel.setFont(juce::Font(10.0f, juce::Font::plain));

    eqLineLabel.setJustificationType(juce::Justification::centredRight);
    eqLineLabel.setFont(juce::Font(14.0f, juce::Font::bold));

    nlmiiLineLabel.setJustificationType(juce::Justification::centredLeft);
    nlmiiLineLabel.setFont(juce::Font(14.0f, juce::Font::bold));

    eqActiveLabel.setJustificationType(juce::Justification::centred);
    eqActiveLabel.setFont(juce::Font(11.0f, juce::Font::bold));

    addAndMakeVisible(modelPanel);
    addAndMakeVisible(irLoader);
    addAndMakeVisible(presetShelf);
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    addAndMakeVisible(eqBypassButton);

    eqBypassButton.setButtonText("");
    eqBypassButton.onClick = [this]() { refreshEqModeLabel(); };

    presetBar.setVisible(false);
    irModule.setVisible(false);
    eqModule.setVisible(false);
    inputBypassButton.setVisible(false);
    irBypassButton.setVisible(false);
    outputBypassButton.setVisible(false);

    configureKnob(inputGainSlider, inputGainLabel, "INPUT");
    configureKnob(gateThresholdSlider, gateThresholdLabel, "GATE");
    configureKnob(lowSlider, lowLabel, "BASS");
    configureKnob(midSlider, midLabel, "MIDDLE");
    configureKnob(highSlider, highLabel, "TREBLE");
    configureKnob(blendSlider, blendLabel, "CAB MIX");
    configureKnob(outputGainSlider, outputGainLabel, "OUTPUT");

    for (auto* slider : { &inputGainSlider, &gateThresholdSlider, &lowSlider, &midSlider, &highSlider, &outputGainSlider })
    {
        slider->setTextValueSuffix(" dB");
        slider->setNumDecimalPlacesToDisplay(1);
    }

    blendSlider.setNumDecimalPlacesToDisplay(0);
    blendSlider.textFromValueFunction = [](double value)
    {
        return juce::String(juce::roundToInt(value * 100.0)) + "%";
    };
    blendSlider.valueFromTextFunction = [](const juce::String& text)
    {
        return juce::jlimit(0.0, 1.0, text.upToFirstOccurrenceOf("%", false, false).getDoubleValue() / 100.0);
    };

    inputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamIDs::inputGain, inputGainSlider);
    inputBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, ParamIDs::inputBypass, inputBypassButton);
    gateThresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamIDs::gateThreshold, gateThresholdSlider);
    lowAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamIDs::eqLow, lowSlider);
    midAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamIDs::eqMid, midSlider);
    highAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamIDs::eqHigh, highSlider);
    eqBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, ParamIDs::eqBypass, eqBypassButton);
    blendAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamIDs::irMix, blendSlider);
    irBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, ParamIDs::irBypass, irBypassButton);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamIDs::outputGain, outputGainSlider);
    outputBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, ParamIDs::outputBypass, outputBypassButton);

    modelPanel.onLoadClicked = [this]() { showLoadNAMDialog(); };
    modelPanel.onClearClicked = [this]()
    {
        processor.clearNAMModel();
        refreshAssetLabels();
    };

    irLoader.onLoadClicked = [this]() { showLoadIRDialog(); };
    irLoader.onClearClicked = [this]()
    {
        processor.clearIRFile();
        refreshAssetLabels();
    };

    presetShelf.onStoreClicked = [this](int index)
    {
        processor.storeUserPresetSlot(index);
        refreshPresetShelf();
    };
    presetShelf.onLoadClicked = [this](int index)
    {
        processor.loadUserPresetSlot(index);
        presetShelf.setActiveSlot(index);
        refreshAssetLabels();
        refreshPresetShelf();
    };
    presetShelf.onClearClicked = [this](int index)
    {
        processor.clearUserPresetSlot(index);
        refreshPresetShelf();
    };
    presetShelf.onNameChanged = [this](int index, const juce::String& newName)
    {
        processor.renameUserPresetSlot(index, newName);
    };

    refreshAssetLabels();
    refreshPresetShelf();
    refreshEqModeLabel();
    startTimerHz(30);
}

GoldPedalAudioProcessorEditor::~GoldPedalAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void GoldPedalAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setGradientFill(juce::ColourGradient(Theme::backgroundTop, bounds.getX(), bounds.getY(),
                                           Theme::backgroundBottom, bounds.getX(), bounds.getBottom(), false));
    g.fillRect(bounds);

    auto frame = bounds.reduced(14.0f);
    g.setColour(juce::Colours::black.withAlpha(0.16f));
    g.fillRoundedRectangle(frame.translated(0.0f, 6.0f), 26.0f);

    g.setGradientFill(juce::ColourGradient(Theme::backgroundTop.brighter(0.08f), frame.getCentreX(), frame.getY(),
                                           Theme::backgroundBottom.darker(0.08f), frame.getCentreX(), frame.getBottom(), false));
    g.fillRoundedRectangle(frame, 26.0f);

    g.setColour(Theme::panelOutline.withAlpha(0.45f));
    g.drawRoundedRectangle(frame, 26.0f, 1.0f);

    g.setColour(juce::Colours::white.withAlpha(0.035f));
    g.fillRoundedRectangle(frame.reduced(2.0f).removeFromTop(88.0f), 24.0f);

    paintStagePanel(g, inputStageBounds, "INPUT");
    paintStagePanel(g, outputStageBounds, "OUTPUT");
    paintControlStrip(g, controlStripBounds);
    paintToggleStrip(g, toggleStripBounds);

    if (! toggleStripBounds.isEmpty())
    {
        const auto toggleCenterY = eqBypassButton.getBounds().getCentreY();
        g.setColour(Theme::panelOutline.withAlpha(0.45f));
        g.drawHorizontalLine(toggleCenterY,
                             static_cast<float>(toggleStripBounds.getX() + 22),
                             static_cast<float>(eqLineLabel.getRight() - 10));
        g.drawHorizontalLine(toggleCenterY,
                             static_cast<float>(nlmiiLineLabel.getX() + 10),
                             static_cast<float>(toggleStripBounds.getRight() - 22));
    }
}

void GoldPedalAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(22);

    auto header = area.removeFromTop(82);
    auto brandArea = header;
    auto utilityArea = brandArea.removeFromRight(440);

    menuButton.setBounds(brandArea.removeFromLeft(82).removeFromTop(34));
    brandArea.removeFromLeft(14);
    titleLabel.setBounds(brandArea.removeFromTop(34));
    subtitleLabel.setBounds(brandArea.removeFromTop(18));

    auto aboutArea = utilityArea.removeFromRight(44);
    aboutButton.setBounds(aboutArea.removeFromTop(30).reduced(4, 0));
    aboutLabel.setBounds(aboutArea.removeFromTop(16));

    utilityArea.removeFromRight(10);

    const int meterGap = 12;
    const int meterWidth = (utilityArea.getWidth() - meterGap) / 2;
    inputStageBounds = utilityArea.removeFromLeft(meterWidth);
    utilityArea.removeFromLeft(meterGap);
    outputStageBounds = utilityArea;

    auto layoutMeterCard = [](juce::Rectangle<int> bounds, MeterComponent& meter, juce::Label& label)
    {
        auto inner = bounds.reduced(12, 10);
        inner.removeFromTop(16);
        meter.setBounds(inner.removeFromTop(12));
        inner.removeFromTop(6);
        label.setBounds(inner);
    };

    layoutMeterCard(inputStageBounds, inputMeter, inputMeterLabel);
    layoutMeterCard(outputStageBounds, outputMeter, masterVolumeLabel);

    area.removeFromTop(14);

    const bool stackLoaders = area.getWidth() < 980;
    auto loaderRow = area.removeFromTop(stackLoaders ? 204 : 96);

    if (stackLoaders)
    {
        auto top = loaderRow.removeFromTop((loaderRow.getHeight() - 16) / 2);
        modelPanel.setBounds(top);
        loaderRow.removeFromTop(16);
        irLoader.setBounds(loaderRow);
    }
    else
    {
        const int loaderGap = 16;
        const int namWidth = (loaderRow.getWidth() - loaderGap) / 2;
        modelPanel.setBounds(loaderRow.removeFromLeft(namWidth));
        loaderRow.removeFromLeft(loaderGap);
        irLoader.setBounds(loaderRow);
    }

    area.removeFromTop(14);

    controlStripBounds = area.removeFromTop(258);
    auto knobArea = controlStripBounds.reduced(18, 16);
    knobArea.removeFromTop(32);
    knobArea.removeFromBottom(66);

    const int knobGap = 10;
    const int knobWidth = (knobArea.getWidth() - knobGap * 6) / 7;
    auto layoutKnob = [knobWidth, knobGap, &knobArea](juce::Label& label, juce::Slider& slider)
    {
        auto slot = knobArea.removeFromLeft(knobWidth);
        label.setBounds(slot.removeFromTop(20));
        slider.setBounds(slot.reduced(2, 2));
        if (knobArea.getWidth() > 0)
            knobArea.removeFromLeft(knobGap);
    };

    layoutKnob(inputGainLabel, inputGainSlider);
    layoutKnob(gateThresholdLabel, gateThresholdSlider);
    layoutKnob(lowLabel, lowSlider);
    layoutKnob(midLabel, midSlider);
    layoutKnob(highLabel, highSlider);
    layoutKnob(blendLabel, blendSlider);
    layoutKnob(outputGainLabel, outputGainSlider);

    toggleStripBounds = {};
    auto bypassRow = controlStripBounds.reduced(20, 18).removeFromBottom(54);

    const int toggleWidth = 56;
    const int leftLabelWidth = 132;
    const int rightLabelWidth = 132;
    const int toggleX = bypassRow.getCentreX() - (toggleWidth / 2);

    eqBypassButton.setBounds(toggleX, bypassRow.getY() + 1, toggleWidth, 26);
    eqLineLabel.setBounds(toggleX - leftLabelWidth, bypassRow.getY() + 3, leftLabelWidth - 14, 18);
    nlmiiLineLabel.setBounds(toggleX + toggleWidth + 14, bypassRow.getY() + 3, rightLabelWidth - 14, 18);
    eqActiveLabel.setBounds(controlStripBounds.getX() + 22, bypassRow.getBottom() + 2,
                            controlStripBounds.getWidth() - 44, 16);

    area.removeFromTop(14);
    presetShelf.setBounds(area);
}

void GoldPedalAudioProcessorEditor::timerCallback()
{
    inputMeter.setLevel(processor.getInputMeterLevel());
    outputMeter.setLevel(processor.getOutputMeterLevel());
    refreshEqModeLabel();
}

void GoldPedalAudioProcessorEditor::configureKnob(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.18f,
                               juce::MathConstants<float>::pi * 2.82f,
                               true);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 20);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, Theme::textSecondary);
    label.setFont(juce::Font(14.0f, juce::Font::bold));

    addAndMakeVisible(slider);
    addAndMakeVisible(label);
}

void GoldPedalAudioProcessorEditor::paintStagePanel(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title)
{
    if (bounds.isEmpty())
        return;

    auto panel = bounds.toFloat();
    g.setColour(juce::Colours::black.withAlpha(0.16f));
    g.fillRoundedRectangle(panel.translated(0.0f, 3.0f), 16.0f);

    g.setGradientFill(juce::ColourGradient(Theme::panelRaised.withAlpha(0.92f), panel.getCentreX(), panel.getY(),
                                           Theme::panel.withAlpha(0.88f), panel.getCentreX(), panel.getBottom(), false));
    g.fillRoundedRectangle(panel, 16.0f);

    g.setColour(Theme::panelOutline.withAlpha(0.8f));
    g.drawRoundedRectangle(panel, 16.0f, 1.0f);

    g.setColour(Theme::accent.withAlpha(0.12f));
    g.fillRoundedRectangle(panel.removeFromTop(18.0f), 16.0f);

    g.setColour(Theme::textSecondary);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText(title, bounds.reduced(14, 8).removeFromTop(14), juce::Justification::centredLeft, false);
}

void GoldPedalAudioProcessorEditor::paintControlStrip(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (bounds.isEmpty())
        return;

    auto panel = bounds.toFloat();
    g.setColour(juce::Colours::black.withAlpha(0.16f));
    g.fillRoundedRectangle(panel.translated(0.0f, 4.0f), 22.0f);

    g.setGradientFill(juce::ColourGradient(Theme::panelRaised.withAlpha(0.7f), panel.getX(), panel.getY(),
                                           Theme::panel.withAlpha(0.82f), panel.getRight(), panel.getBottom(), false));
    g.fillRoundedRectangle(panel, 22.0f);

    g.setColour(Theme::panelOutline.withAlpha(0.8f));
    g.drawRoundedRectangle(panel, 22.0f, 1.0f);

    auto headerArea = bounds.reduced(18, 10).removeFromTop(18);
    g.setColour(Theme::textSecondary);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText("CHAIN CONTROLS", headerArea, juce::Justification::centredLeft, false);

    auto voicingLineY = static_cast<float>(bounds.getBottom() - 48);
    g.setColour(Theme::panelOutline.withAlpha(0.5f));
    g.drawHorizontalLine(voicingLineY, static_cast<float>(bounds.getX() + 18), static_cast<float>(bounds.getRight() - 18));

    g.setColour(Theme::textSecondary);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("VOICING", bounds.getX() + 18, bounds.getBottom() - 42, 80, 14, juce::Justification::centredLeft, false);
}

void GoldPedalAudioProcessorEditor::paintToggleStrip(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (bounds.isEmpty())
        return;

    auto panel = bounds.toFloat();
    g.setGradientFill(juce::ColourGradient(Theme::panelRaised.withAlpha(0.58f), panel.getCentreX(), panel.getY(),
                                           Theme::panel.withAlpha(0.78f), panel.getCentreX(), panel.getBottom(), false));
    g.fillRoundedRectangle(panel, 18.0f);

    g.setColour(Theme::panelOutline.withAlpha(0.75f));
    g.drawRoundedRectangle(panel, 18.0f, 1.0f);

    auto headerArea = bounds.reduced(18, 8).removeFromTop(16);
    g.setColour(Theme::textSecondary);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("VOICING", headerArea, juce::Justification::centredLeft, false);
}

void GoldPedalAudioProcessorEditor::refreshAssetLabels()
{
    modelPanel.setModelName(processor.getLoadedNAMName());
    modelPanel.setArchitecture(processor.getLoadedNAMArchitecture());
    modelPanel.setStatusText(processor.getLoadedNAMStatus(), processor.hasNAMLoadError());
    irLoader.setFileName(processor.getLoadedIRName());
}

void GoldPedalAudioProcessorEditor::refreshPresetShelf()
{
    std::vector<PresetShelfSlotView> slots;
    slots.reserve(static_cast<size_t>(processor.getNumUserPresetSlots()));

    for (int index = 0; index < processor.getNumUserPresetSlots(); ++index)
    {
        const auto info = processor.getUserPresetSlotInfo(index);
        slots.push_back({ info.name, info.modelName, info.irName, info.occupied });
    }

    presetShelf.setSlots(slots);
}

void GoldPedalAudioProcessorEditor::refreshEqModeLabel()
{
    const bool rawNamMode = eqBypassButton.getToggleState();
    eqActiveLabel.setText(rawNamMode ? "Post EQ is off. You are hearing the raw NAM voicing."
                                     : "Post EQ is on and shaping the final output.",
                          juce::dontSendNotification);
    eqLineLabel.setColour(juce::Label::textColourId, rawNamMode ? Theme::textSecondary : Theme::success);
    nlmiiLineLabel.setColour(juce::Label::textColourId, rawNamMode ? Theme::danger : Theme::textSecondary);
}

void GoldPedalAudioProcessorEditor::showSavePresetDialog()
{
    presetChooser = std::make_unique<juce::FileChooser>(
        "Save Preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.xml");

    presetChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                               [this](const juce::FileChooser& chooser)
    {
        auto file = chooser.getResult();
        if (file != juce::File())
        {
            if (file.getFileExtension().isEmpty())
                file = file.withFileExtension(".xml");

            processor.savePresetToFile(file);
        }
    });
}

void GoldPedalAudioProcessorEditor::showLoadPresetDialog()
{
    presetChooser = std::make_unique<juce::FileChooser>(
        "Load Preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.xml");

    presetChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                               [this](const juce::FileChooser& chooser)
    {
        auto file = chooser.getResult();
        if (file.existsAsFile())
        {
            processor.loadPresetFromFile(file);
            refreshAssetLabels();
            refreshPresetShelf();
        }
    });
}

void GoldPedalAudioProcessorEditor::showLoadIRDialog()
{
    irChooser = std::make_unique<juce::FileChooser>(
        "Load IR", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.wav");

    irChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                           [this](const juce::FileChooser& chooser)
    {
        auto file = chooser.getResult();
        if (file.existsAsFile())
        {
            processor.loadIRFile(file);
            refreshAssetLabels();
        }
    });
}

void GoldPedalAudioProcessorEditor::showLoadNAMDialog()
{
    namChooser = std::make_unique<juce::FileChooser>(
        "Load NAM Model", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.nam");

    namChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                            [this](const juce::FileChooser& chooser)
    {
        auto file = chooser.getResult();
        if (file.existsAsFile())
        {
            processor.loadNAMModelFile(file);
            refreshAssetLabels();
        }
    });
}

void GoldPedalAudioProcessorEditor::applyFactoryPreset(int index)
{
    if (index < 0 || index >= factoryPresetNames.size())
        return;

    processor.applyFactoryPreset(index);
    presetBar.setSelectedIndex(index);
}
