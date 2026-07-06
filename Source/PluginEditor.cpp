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
      gateModule(apvts, "INPUT GATE", ParamIDs::gateBypass,
                 { { "Threshold", ParamIDs::gateThreshold }, { "Release", ParamIDs::gateRelease } }),
      irModule(apvts, "CAB BLEND", ParamIDs::irBypass, { { "Blend", ParamIDs::irMix } }),
      eqModule(apvts, "EQ", ParamIDs::eqBypass,
               { { "Low", ParamIDs::eqLow }, { "Mid", ParamIDs::eqMid }, { "High", ParamIDs::eqHigh } })
{
    setLookAndFeel(&lookAndFeel);
    setSize(1000, 640);

    titleLabel.setText("ADR - AMPER", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, Theme::textPrimary);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setVisible(false);

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

    inputMeterLabel.setJustificationType(juce::Justification::centred);
    inputMeterLabel.setFont(juce::Font(10.0f, juce::Font::plain));

    masterVolumeLabel.setJustificationType(juce::Justification::centred);
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
    addAndMakeVisible(gateModule);
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
    configureKnob(lowSlider, lowLabel, "BASS");
    configureKnob(midSlider, midLabel, "MIDDLE");
    configureKnob(highSlider, highLabel, "TREBLE");
    configureKnob(blendSlider, blendLabel, "CAB MIX");
    configureKnob(outputGainSlider, outputGainLabel, "OUTPUT");

    for (auto* slider : { &inputGainSlider, &lowSlider, &midSlider, &highSlider, &outputGainSlider })
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

    g.setColour(juce::Colours::white.withAlpha(0.03f));
    g.fillRoundedRectangle(bounds.reduced(12.0f).removeFromTop(82.0f), 18.0f);

    const auto toggleCenterY = eqBypassButton.getBounds().getCentreY();
    g.setColour(Theme::panelOutline.withAlpha(0.45f));
    g.drawHorizontalLine(toggleCenterY, 30.0f, static_cast<float>(eqLineLabel.getRight() - 10));
    g.drawHorizontalLine(toggleCenterY, static_cast<float>(nlmiiLineLabel.getX() + 10), static_cast<float>(getWidth() - 30));
}

void GoldPedalAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);

    auto header = area.removeFromTop(62);
    menuButton.setBounds(header.removeFromLeft(66).reduced(0, 10));
    header.removeFromLeft(12);
    titleLabel.setBounds(header.removeFromLeft(270));

    auto rightHeader = header.removeFromRight(320);
    auto aboutArea = rightHeader.removeFromRight(40);
    aboutButton.setBounds(aboutArea.removeFromTop(32).reduced(4, 0));
    aboutLabel.setBounds(aboutArea);

    rightHeader.removeFromRight(10);

    auto outputArea = rightHeader.removeFromRight(128);
    outputMeter.setBounds(outputArea.removeFromTop(12).reduced(0, 1));
    outputArea.removeFromTop(4);
    masterVolumeLabel.setBounds(outputArea);

    rightHeader.removeFromRight(10);

    auto inputArea = rightHeader.removeFromRight(128);
    inputMeter.setBounds(inputArea.removeFromTop(12).reduced(0, 1));
    inputArea.removeFromTop(4);
    inputMeterLabel.setBounds(inputArea);

    area.removeFromTop(12);

    auto loaderRow = area.removeFromTop(84);
    const int loaderGap = 16;
    const int namWidth = (loaderRow.getWidth() - loaderGap) / 2;
    modelPanel.setBounds(loaderRow.removeFromLeft(namWidth));
    loaderRow.removeFromLeft(loaderGap);
    irLoader.setBounds(loaderRow);

    area.removeFromTop(16);
    gateModule.setBounds(area.removeFromTop(120));

    area.removeFromTop(14);

    auto knobArea = area.removeFromTop(132);
    const int knobWidth = knobArea.getWidth() / 6;
    auto layoutKnob = [knobWidth, &knobArea](juce::Label& label, juce::Slider& slider)
    {
        auto slot = knobArea.removeFromLeft(knobWidth);
        label.setBounds(slot.removeFromTop(20));
        slider.setBounds(slot.reduced(4, 2));
    };

    layoutKnob(inputGainLabel, inputGainSlider);
    layoutKnob(lowLabel, lowSlider);
    layoutKnob(midLabel, midSlider);
    layoutKnob(highLabel, highSlider);
    layoutKnob(blendLabel, blendSlider);
    layoutKnob(outputGainLabel, outputGainSlider);

    area.removeFromTop(10);

    auto bypassRow = area.removeFromTop(48);
    const int toggleX = bypassRow.getCentreX() - 25;
    eqBypassButton.setBounds(toggleX, bypassRow.getY(), 50, 26);
    eqActiveLabel.setBounds(bypassRow.getCentreX() - 110, bypassRow.getY() + 28, 220, 16);
    eqLineLabel.setBounds(toggleX - 90, bypassRow.getY() + 2, 80, 20);
    nlmiiLineLabel.setBounds(toggleX + 60, bypassRow.getY() + 2, 80, 20);

    area.removeFromTop(12);
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
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 18);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, Theme::textSecondary);
    label.setFont(juce::Font(13.0f, juce::Font::bold));

    addAndMakeVisible(slider);
    addAndMakeVisible(label);
}

void GoldPedalAudioProcessorEditor::paintStagePanel(juce::Graphics&, juce::Rectangle<int>, const juce::String&) {}
void GoldPedalAudioProcessorEditor::paintControlStrip(juce::Graphics&, juce::Rectangle<int>) {}
void GoldPedalAudioProcessorEditor::paintToggleStrip(juce::Graphics&, juce::Rectangle<int>) {}

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
    eqActiveLabel.setText(rawNamMode ? "Post EQ bypassed | Raw NAM voicing"
                                     : "Post EQ active",
                          juce::dontSendNotification);
    eqLineLabel.setColour(juce::Label::textColourId, rawNamMode ? Theme::textSecondary : Theme::textPrimary);
    nlmiiLineLabel.setColour(juce::Label::textColourId, rawNamMode ? Theme::textPrimary : Theme::textSecondary);
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
