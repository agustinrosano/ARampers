#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "UI/GoldLookAndFeel.h"
#include "UI/IRLoaderComponent.h"
#include "UI/MeterComponent.h"
#include "UI/ModelPanelComponent.h"
#include "UI/PedalModuleComponent.h"
#include "UI/PresetBarComponent.h"
#include "UI/PresetShelfComponent.h"

class GoldPedalAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    explicit GoldPedalAudioProcessorEditor(GoldPedalAudioProcessor&);
    ~GoldPedalAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void configureKnob(juce::Slider& slider, juce::Label& label, const juce::String& text);
    void paintStagePanel(juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title);
    void paintControlStrip(juce::Graphics& g, juce::Rectangle<int> bounds);
    void paintToggleStrip(juce::Graphics& g, juce::Rectangle<int> bounds);
    void refreshAssetLabels();
    void refreshPresetShelf();
    void showSavePresetDialog();
    void showLoadPresetDialog();
    void showLoadIRDialog();
    void showLoadNAMDialog();
    void applyFactoryPreset(int index);
    void refreshEqModeLabel();

    GoldPedalAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& apvts;
    GoldLookAndFeel lookAndFeel;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    PresetBarComponent presetBar;
    ModelPanelComponent modelPanel;
    IRLoaderComponent irLoader;
    PresetShelfComponent presetShelf;

    juce::TextButton menuButton { "MENU" };
    juce::TextButton aboutButton { "i" };
    juce::Label inputMeterLabel { {}, "Input Level" };
    juce::Label masterVolumeLabel { {}, "Output Level" };
    juce::Label aboutLabel { {}, "About" };
    juce::Label eqLineLabel { {}, "ON" };
    juce::Label nlmiiLineLabel { {}, "OFF" };
    juce::Label eqActiveLabel { {}, "Post EQ active" };

    PedalModuleComponent irModule;
    PedalModuleComponent eqModule;

    juce::Slider inputGainSlider;
    juce::Label inputGainLabel;
    MeterComponent inputMeter;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputAttachment;
    juce::ToggleButton inputBypassButton { "INPUT BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> inputBypassAttachment;

    juce::Slider lowSlider;
    juce::Label lowLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowAttachment;

    juce::Slider gateThresholdSlider;
    juce::Label gateThresholdLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gateThresholdAttachment;

    juce::Slider midSlider;
    juce::Label midLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midAttachment;

    juce::Slider highSlider;
    juce::Label highLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highAttachment;
    juce::ToggleButton eqBypassButton { "EQ BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> eqBypassAttachment;

    juce::Slider blendSlider;
    juce::Label blendLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> blendAttachment;
    juce::ToggleButton irBypassButton { "IR BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> irBypassAttachment;

    juce::Slider outputGainSlider;
    juce::Label outputGainLabel;
    MeterComponent outputMeter;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
    juce::ToggleButton outputBypassButton { "OUTPUT BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> outputBypassAttachment;

    juce::StringArray factoryPresetNames;

    std::unique_ptr<juce::FileChooser> presetChooser;
    std::unique_ptr<juce::FileChooser> irChooser;
    std::unique_ptr<juce::FileChooser> namChooser;

    juce::Rectangle<int> inputStageBounds;
    juce::Rectangle<int> outputStageBounds;
    juce::Rectangle<int> controlStripBounds;
    juce::Rectangle<int> toggleStripBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GoldPedalAudioProcessorEditor)
};
