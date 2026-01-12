#include "PluginEditor.h"

GoldPedalAudioProcessorEditor::GoldPedalAudioProcessorEditor(GoldPedalAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      apvts(p.getAPVTS()),
      gateModule(apvts, "Gate", ParamIDs::gateBypass,
                 { { "Thresh", ParamIDs::gateThreshold }, { "Release", ParamIDs::gateRelease } }),
      driveModule(apvts, "Drive", ParamIDs::driveBypass,
                  { { "Drive", ParamIDs::driveAmount }, { "Tone", ParamIDs::driveTone } }),
      ampModule(apvts, "Amp", ParamIDs::ampBypass,
                { { "Gain", ParamIDs::ampGain }, { "Tone", ParamIDs::ampTone } }),
      irModule(apvts, "IR", ParamIDs::irBypass, { { "Mix", ParamIDs::irMix } }),
      eqModule(apvts, "EQ", ParamIDs::eqBypass,
               { { "Low", ParamIDs::eqLow }, { "Mid", ParamIDs::eqMid }, { "High", ParamIDs::eqHigh } })
{
    setSize(900, 520);

    addAndMakeVisible(presetBar);
    addAndMakeVisible(irLoader);

    addAndMakeVisible(gateModule);
    addAndMakeVisible(driveModule);
    addAndMakeVisible(ampModule);
    addAndMakeVisible(irModule);
    addAndMakeVisible(eqModule);

    configureKnob(inputGainSlider, inputGainLabel, "Input");
    configureKnob(outputGainSlider, outputGainLabel, "Output");

    inputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamIDs::inputGain, inputGainSlider);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, ParamIDs::outputGain, outputGainSlider);

    factoryPresetNames = processor.getFactoryPresetNames();
    presetBar.setPresetNames(factoryPresetNames);

    presetBar.onPresetSelected = [this](int index)
    {
        applyFactoryPreset(index);
    };

    presetBar.onSaveClicked = [this]()
    {
        showSavePresetDialog();
    };

    presetBar.onLoadClicked = [this]()
    {
        showLoadPresetDialog();
    };

    presetBar.onPrevClicked = [this]()
    {
        if (factoryPresetNames.isEmpty())
            return;

        int index = presetBar.getSelectedIndex();
        if (index < 0)
            index = 0;
        index = (index - 1 + factoryPresetNames.size()) % factoryPresetNames.size();
        applyFactoryPreset(index);
    };

    presetBar.onNextClicked = [this]()
    {
        if (factoryPresetNames.isEmpty())
            return;

        int index = presetBar.getSelectedIndex();
        index = (index + 1) % factoryPresetNames.size();
        applyFactoryPreset(index);
    };

    irLoader.onLoadClicked = [this]()
    {
        showLoadIRDialog();
    };

    irLoader.setFileName(processor.getLoadedIRName());
}

GoldPedalAudioProcessorEditor::~GoldPedalAudioProcessorEditor() = default;

void GoldPedalAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff202124));
}

void GoldPedalAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    auto presetArea = area.removeFromTop(40);
    presetBar.setBounds(presetArea);

    auto moduleArea = area.removeFromTop(240);
    const int cardWidth = moduleArea.getWidth() / 5;
    gateModule.setBounds(moduleArea.removeFromLeft(cardWidth).reduced(4));
    driveModule.setBounds(moduleArea.removeFromLeft(cardWidth).reduced(4));
    ampModule.setBounds(moduleArea.removeFromLeft(cardWidth).reduced(4));
    irModule.setBounds(moduleArea.removeFromLeft(cardWidth).reduced(4));
    eqModule.setBounds(moduleArea.removeFromLeft(cardWidth).reduced(4));

    auto irArea = area.removeFromTop(50);
    irLoader.setBounds(irArea);

    auto ioArea = area.removeFromTop(130);
    auto inputArea = ioArea.removeFromLeft(140);
    inputGainLabel.setBounds(inputArea.removeFromTop(20));
    inputGainSlider.setBounds(inputArea);

    auto outputArea = ioArea.removeFromLeft(140);
    outputGainLabel.setBounds(outputArea.removeFromTop(20));
    outputGainSlider.setBounds(outputArea);
}

void GoldPedalAudioProcessorEditor::configureKnob(juce::Slider& slider, juce::Label& label,
                                                  const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(slider);
    addAndMakeVisible(label);
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
            processor.loadPresetFromFile(file);
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
            irLoader.setFileName(processor.getLoadedIRName());
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
