#pragma once

#include <JuceHeader.h>
#include "DSP/PedalboardProcessor.h"

namespace ParamIDs
{
    static constexpr const char* inputGain = "input_gain";
    static constexpr const char* inputBypass = "input_bypass";
    static constexpr const char* gateThreshold = "gate_threshold";
    static constexpr const char* gateRelease = "gate_release";
    static constexpr const char* gateBypass = "gate_bypass";
    static constexpr const char* driveAmount = "drive_amount";
    static constexpr const char* driveTone = "drive_tone";
    static constexpr const char* driveBypass = "drive_bypass";
    static constexpr const char* ampGain = "amp_gain";
    static constexpr const char* ampTone = "amp_tone";
    static constexpr const char* ampBypass = "amp_bypass";
    static constexpr const char* irMix = "ir_mix";
    static constexpr const char* irBypass = "ir_bypass";
    static constexpr const char* eqLow = "eq_low";
    static constexpr const char* eqMid = "eq_mid";
    static constexpr const char* eqHigh = "eq_high";
    static constexpr const char* eqBypass = "eq_bypass";
    static constexpr const char* outputGain = "output_gain";
    static constexpr const char* outputBypass = "output_bypass";
}

class GoldPedalAudioProcessor : public juce::AudioProcessor
{
public:
    GoldPedalAudioProcessor();
    ~GoldPedalAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    void loadIRFile(const juce::File& file);
    juce::String getLoadedIRName() const;

    void savePresetToFile(const juce::File& file);
    void loadPresetFromFile(const juce::File& file);

    juce::StringArray getFactoryPresetNames() const;
    void applyFactoryPreset(int index);

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    struct ParamRefs
    {
        std::atomic<float>* inputGain = nullptr;
        std::atomic<float>* inputBypass = nullptr;
        std::atomic<float>* gateThreshold = nullptr;
        std::atomic<float>* gateRelease = nullptr;
        std::atomic<float>* gateBypass = nullptr;
        std::atomic<float>* driveAmount = nullptr;
        std::atomic<float>* driveTone = nullptr;
        std::atomic<float>* driveBypass = nullptr;
        std::atomic<float>* ampGain = nullptr;
        std::atomic<float>* ampTone = nullptr;
        std::atomic<float>* ampBypass = nullptr;
        std::atomic<float>* irMix = nullptr;
        std::atomic<float>* irBypass = nullptr;
        std::atomic<float>* eqLow = nullptr;
        std::atomic<float>* eqMid = nullptr;
        std::atomic<float>* eqHigh = nullptr;
        std::atomic<float>* eqBypass = nullptr;
        std::atomic<float>* outputGain = nullptr;
        std::atomic<float>* outputBypass = nullptr;
    };

    struct FactoryPreset
    {
        juce::String name;
        std::vector<std::pair<juce::String, float>> values;
    };

    void updateParametersFromAPVTS();
    void buildFactoryPresets();

    PedalboardProcessor pedalboard;
    juce::AudioProcessorValueTreeState apvts;
    ParamRefs paramRefs;

    juce::String irFileName;
    std::vector<FactoryPreset> factoryPresets;
    juce::StringArray factoryPresetNames;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GoldPedalAudioProcessor)
};
