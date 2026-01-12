#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/PresetBarComponent.h"
#include "UI/PedalModuleComponent.h"
#include "UI/IRLoaderComponent.h"

class GoldPedalAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit GoldPedalAudioProcessorEditor(GoldPedalAudioProcessor&);
    ~GoldPedalAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    GoldPedalAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& apvts;

    PresetBarComponent presetBar;
    juce::Label titleLabel;
    IRLoaderComponent irLoader;

    PedalModuleComponent gateModule;
    PedalModuleComponent driveModule;
    PedalModuleComponent ampModule;
    PedalModuleComponent irModule;
    PedalModuleComponent eqModule;

    juce::Slider inputGainSlider;
    juce::Label inputGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputAttachment;

    juce::Slider outputGainSlider;
    juce::Label outputGainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;

    juce::StringArray factoryPresetNames;

    std::unique_ptr<juce::FileChooser> presetChooser;
    std::unique_ptr<juce::FileChooser> irChooser;

    void configureKnob(juce::Slider& slider, juce::Label& label, const juce::String& text);

    void showSavePresetDialog();
    void showLoadPresetDialog();
    void showLoadIRDialog();
    void applyFactoryPreset(int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GoldPedalAudioProcessorEditor)
};
