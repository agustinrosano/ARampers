#include "PluginEditor.h"

GoldPedalAudioProcessorEditor::GoldPedalAudioProcessorEditor(GoldPedalAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      apvts(p.getAPVTS()),
      modelPanel(apvts, ParamIDs::modelBypass),
      presetShelf(p.getNumUserPresetSlots()),
      gateModule(apvts, "MASTER GATE", ParamIDs::gateBypass,
                 { { "Thresh", ParamIDs::gateThreshold }, { "Release", ParamIDs::gateRelease } }),
      irModule(apvts, "CAB BLEND", ParamIDs::irBypass, { { "Blend", ParamIDs::irMix } }),
      eqModule(apvts, "EQ", ParamIDs::eqBypass,
               { { "Low", ParamIDs::eqLow }, { "Mid", ParamIDs::eqMid }, { "High", ParamIDs::eqHigh } })
{
    setLookAndFeel(&lookAndFeel);
    setSize(1220, 900);

    titleLabel.setText("ADR - AMPER", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::Font(31.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, Theme::textPrimary);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("Neural amp model host with IR, tone shaping and live preset recall",
                          juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centredLeft);
    subtitleLabel.setColour(juce::Label::textColourId, Theme::textSecondary);
    addAndMakeVisible(subtitleLabel);

    addAndMakeVisible(presetBar);
    addAndMakeVisible(modelPanel);
    addAndMakeVisible(irLoader);
    addAndMakeVisible(presetShelf);
    addAndMakeVisible(gateModule);
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    addAndMakeVisible(inputBypassButton);
    addAndMakeVisible(eqBypassButton);
    addAndMakeVisible(irBypassButton);
    addAndMakeVisible(outputBypassButton);

    configureKnob(inputGainSlider, inputGainLabel, "INPUT GAIN");
    configureKnob(lowSlider, lowLabel, "BASS");
    configureKnob(midSlider, midLabel, "MIDDLE");
    configureKnob(highSlider, highLabel, "TREBLE");
    configureKnob(blendSlider, blendLabel, "BLEND");
    configureKnob(outputGainSlider, outputGainLabel, "OUTPUT GAIN");

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

    factoryPresetNames = processor.getFactoryPresetNames();
    presetBar.setPresetNames(factoryPresetNames);

    presetBar.onPresetSelected = [this](int index) { applyFactoryPreset(index); };
    presetBar.onSaveClicked = [this]() { showSavePresetDialog(); };
    presetBar.onLoadClicked = [this]() { showLoadPresetDialog(); };
    presetBar.onPrevClicked = [this]()
    {
        if (factoryPresetNames.isEmpty())
            return;

        int index = presetBar.getSelectedIndex();
        if (index < 0)
            index = 0;

        applyFactoryPreset((index - 1 + factoryPresetNames.size()) % factoryPresetNames.size());
    };
    presetBar.onNextClicked = [this]()
    {
        if (factoryPresetNames.isEmpty())
            return;

        int index = presetBar.getSelectedIndex();
        if (index < 0)
            index = 0;

        applyFactoryPreset((index + 1) % factoryPresetNames.size());
    };

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
    startTimerHz(30);
}

GoldPedalAudioProcessorEditor::~GoldPedalAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void GoldPedalAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.setGradientFill(juce::ColourGradient(Theme::backgroundTop, 0.0f, 0.0f,
                                           Theme::backgroundBottom, 0.0f, static_cast<float>(getHeight()), false));
    g.fillAll();

    auto frame = getLocalBounds().toFloat().reduced(10.0f);
    g.setColour(juce::Colours::black.withAlpha(0.22f));
    g.fillRoundedRectangle(frame.translated(0.0f, 5.0f), 28.0f);

    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3d3d42), frame.getX(), frame.getY(),
                                           juce::Colour(0xff1d1d20), frame.getRight(), frame.getBottom(), false));
    g.fillRoundedRectangle(frame, 28.0f);
    g.setColour(Theme::panelOutline.withAlpha(0.9f));
    g.drawRoundedRectangle(frame, 28.0f, 1.2f);

    g.setColour(Theme::accentSoft);
    g.fillRect(24, 26, getWidth() - 48, 2);

    g.setGradientFill(juce::ColourGradient(Theme::accent.withAlpha(0.18f), 84.0f, 62.0f,
                                           juce::Colours::transparentBlack, 340.0f, 180.0f, true));
    g.fillEllipse(20.0f, 28.0f, 340.0f, 170.0f);

    g.setGradientFill(juce::ColourGradient(Theme::presetBlue.withAlpha(0.10f), static_cast<float>(getWidth()) - 260.0f, 60.0f,
                                           juce::Colours::transparentBlack, static_cast<float>(getWidth()) - 20.0f, 220.0f, true));
    g.fillEllipse(static_cast<float>(getWidth()) - 340.0f, 24.0f, 300.0f, 160.0f);

    paintControlStrip(g, controlStripBounds);
    paintStagePanel(g, inputStageBounds, "INPUT");
    paintStagePanel(g, outputStageBounds, "OUTPUT");
    paintToggleStrip(g, toggleStripBounds);
}

void GoldPedalAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(24);

    auto header = area.removeFromTop(68);
    auto titleArea = header.removeFromLeft(420);
    titleLabel.setBounds(titleArea.removeFromTop(36));
    subtitleLabel.setBounds(titleArea.removeFromTop(20));
    presetBar.setBounds(header.removeFromRight(420).reduced(0, 8));

    area.removeFromTop(10);

    auto loaderRow = area.removeFromTop(170);
    auto loaderGap = 12;
    auto namArea = loaderRow.removeFromLeft((loaderRow.getWidth() - loaderGap) / 2);
    loaderRow.removeFromLeft(loaderGap);
    modelPanel.setBounds(namArea.reduced(2));
    irLoader.setBounds(loaderRow.reduced(2));

    area.removeFromTop(12);
    controlStripBounds = area.removeFromTop(230);

    auto knobArea = controlStripBounds.reduced(20, 40);
    auto knobColumns = 6;
    auto knobWidth = knobArea.getWidth() / knobColumns;

    auto layoutKnob = [knobWidth, &knobArea](juce::Label& label, juce::Slider& slider)
    {
        auto slot = knobArea.removeFromLeft(knobWidth);
        label.setBounds(slot.removeFromTop(24));
        slider.setBounds(slot.reduced(4));
    };

    layoutKnob(inputGainLabel, inputGainSlider);
    layoutKnob(lowLabel, lowSlider);
    layoutKnob(midLabel, midSlider);
    layoutKnob(highLabel, highSlider);
    layoutKnob(blendLabel, blendSlider);
    layoutKnob(outputGainLabel, outputGainSlider);

    area.removeFromTop(10);
    auto utilityRow = area.removeFromTop(116);
    gateModule.setBounds(utilityRow.removeFromLeft(300).reduced(2));
    utilityRow.removeFromLeft(10);
    inputStageBounds = utilityRow.removeFromLeft(110).reduced(2);
    utilityRow.removeFromLeft(10);
    outputStageBounds = utilityRow.removeFromLeft(110).reduced(2);
    utilityRow.removeFromLeft(10);
    toggleStripBounds = utilityRow.reduced(2);

    auto toggleArea = toggleStripBounds.reduced(16, 18);
    const int toggleWidth = toggleArea.getWidth() / 4;
    inputBypassButton.setBounds(toggleArea.removeFromLeft(toggleWidth).reduced(4));
    eqBypassButton.setBounds(toggleArea.removeFromLeft(toggleWidth).reduced(4));
    irBypassButton.setBounds(toggleArea.removeFromLeft(toggleWidth).reduced(4));
    outputBypassButton.setBounds(toggleArea.reduced(4));

    auto inputArea = inputStageBounds.reduced(12);
    inputMeter.setBounds(inputArea.reduced(26, 4));

    auto outputArea = outputStageBounds.reduced(12);
    outputMeter.setBounds(outputArea.reduced(26, 4));

    area.removeFromTop(10);
    presetShelf.setBounds(area);
}

void GoldPedalAudioProcessorEditor::timerCallback()
{
    inputMeter.setLevel(processor.getInputMeterLevel());
    outputMeter.setLevel(processor.getOutputMeterLevel());
}

void GoldPedalAudioProcessorEditor::configureKnob(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 18);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, Theme::textPrimary);
    label.setFont(juce::Font(15.0f, juce::Font::bold));

    addAndMakeVisible(slider);
    addAndMakeVisible(label);
}

void GoldPedalAudioProcessorEditor::paintStagePanel(juce::Graphics& g, juce::Rectangle<int> bounds,
                                                    const juce::String& title)
{
    auto panel = bounds.toFloat();
    g.setGradientFill(juce::ColourGradient(Theme::panelRaised.brighter(0.08f), panel.getX(), panel.getY(),
                                           Theme::panel.darker(0.08f), panel.getRight(), panel.getBottom(), false));
    g.fillRoundedRectangle(panel, 20.0f);
    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(panel, 20.0f, 1.0f);

    g.setColour(Theme::accent.withAlpha(0.10f));
    g.fillRoundedRectangle(panel.removeFromTop(42.0f), 20.0f);

    g.setColour(Theme::textSecondary);
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.drawText(title, bounds.removeFromTop(26), juce::Justification::centred, false);
}

void GoldPedalAudioProcessorEditor::paintControlStrip(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto panel = bounds.toFloat();
    g.setGradientFill(juce::ColourGradient(Theme::panelRaised.brighter(0.10f), panel.getX(), panel.getY(),
                                           Theme::panel.darker(0.05f), panel.getRight(), panel.getBottom(), false));
    g.fillRoundedRectangle(panel, 24.0f);
    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(panel, 24.0f, 1.0f);

    auto topBand = panel.reduced(16.0f).removeFromTop(34.0f);
    g.setColour(Theme::textSecondary);
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText("MASTER GAIN / EQ / CAB", topBand, juce::Justification::centred, false);

    g.setColour(Theme::accent.withAlpha(0.10f));
    g.fillRoundedRectangle(panel.reduced(10.0f).removeFromTop(48.0f), 18.0f);
}

void GoldPedalAudioProcessorEditor::paintToggleStrip(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto panel = bounds.toFloat();
    g.setGradientFill(juce::ColourGradient(Theme::panelRaised, panel.getX(), panel.getY(),
                                           Theme::panel.darker(0.10f), panel.getRight(), panel.getBottom(), false));
    g.fillRoundedRectangle(panel, 20.0f);
    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(panel, 20.0f, 1.0f);

    g.setColour(Theme::textSecondary);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawText("ACTIVE MODULES", bounds.removeFromTop(20), juce::Justification::centred, false);
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
