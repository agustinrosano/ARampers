#pragma once

#include <JuceHeader.h>
#include "DSP/PedalboardProcessor.h"
#include <array>

namespace ParamIDs
{
    static constexpr const char* inputGain = "input_gain";
    static constexpr const char* inputBypass = "input_bypass";
    static constexpr const char* gateThreshold = "gate_threshold";
    static constexpr const char* gateRelease = "gate_release";
    static constexpr const char* gateBypass = "gate_bypass";
    static constexpr const char* modelBypass = "model_bypass";
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
    struct UserPresetSlotInfo
    {
        juce::String name;
        juce::String modelName;
        juce::String irName;
        bool occupied = false;
    };

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
    void clearIRFile();
    juce::String getLoadedIRName() const;
    juce::String getLoadedIRPath() const;

    bool loadNAMModelFile(const juce::File& file);
    void clearNAMModel();
    juce::String getLoadedNAMName() const;
    juce::String getLoadedNAMArchitecture() const;
    juce::String getLoadedNAMStatus() const;
    bool hasNAMLoadError() const;

    float getInputMeterLevel() const;
    float getOutputMeterLevel() const;

    void savePresetToFile(const juce::File& file);
    void loadPresetFromFile(const juce::File& file);

    juce::StringArray getFactoryPresetNames() const;
    void applyFactoryPreset(int index);
    int getNumUserPresetSlots() const;
    UserPresetSlotInfo getUserPresetSlotInfo(int index) const;
    void storeUserPresetSlot(int index);
    void loadUserPresetSlot(int index);
    void clearUserPresetSlot(int index);
    void renameUserPresetSlot(int index, const juce::String& newName);

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    struct ParamRefs
    {
        std::atomic<float>* inputGain = nullptr;
        std::atomic<float>* inputBypass = nullptr;
        std::atomic<float>* gateThreshold = nullptr;
        std::atomic<float>* gateRelease = nullptr;
        std::atomic<float>* gateBypass = nullptr;
        std::atomic<float>* modelBypass = nullptr;
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

    struct UserPresetSlot
    {
        UserPresetSlotInfo info;
        juce::String serializedState;
    };

    void updateParametersFromAPVTS();
    void buildFactoryPresets();
    std::unique_ptr<juce::XmlElement> createStateXmlWithAssets();
    void applyStateXml(const juce::XmlElement& xml);
    static juce::File getUserPresetBankFile();
    void loadUserPresetBankFromDisk();
    void saveUserPresetBankToDisk() const;
    static float measureBufferPeak(const juce::AudioBuffer<float>& buffer);

    PedalboardProcessor pedalboard;
    juce::AudioProcessorValueTreeState apvts;
    ParamRefs paramRefs;

    juce::String irFileName;
    juce::String irFilePath;
    std::atomic<float> inputMeterLevel { 0.0f };
    std::atomic<float> outputMeterLevel { 0.0f };
    std::vector<FactoryPreset> factoryPresets;
    juce::StringArray factoryPresetNames;
    std::array<UserPresetSlot, 6> userPresetSlots;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GoldPedalAudioProcessor)
};
