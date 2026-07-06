#pragma once

#include "Module.h"

#include "NAM/get_dsp.h"
#include "json.hpp"

#include <filesystem>
#include <memory>
#include <vector>

class NAMModule : public Module
{
public:
    void prepare(double sampleRate, int blockSize, int numChannels) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    bool loadModelFile(const juce::File& file);
    void clearModel();

    bool hasModel() const;
    juce::String getLoadedFileName() const;
    juce::String getLoadedFilePath() const;
    juce::String getArchitectureName() const;
    juce::String getModelStatusText() const;
    juce::String getLastError() const;
    bool hasInputCalibration() const;
    bool hasOutputCalibration() const;
    double getInputCalibrationDbu() const;
    double getOutputCalibrationDbu() const;
    juce::String getCalibrationSummaryText() const;

private:
    struct ModelState
    {
        std::vector<std::unique_ptr<nam::DSP>> channelModels;
        juce::String fileName;
        juce::String fullPath;
        juce::String architecture;
        juce::String statusText;
        bool hasInputCalibration = false;
        bool hasOutputCalibration = false;
        double inputCalibrationDbu = 0.0;
        double outputCalibrationDbu = 0.0;
    };

    static juce::String describeArchitecture(const nlohmann::json& modelJson);
    static std::filesystem::path toFilesystemPath(const juce::File& file);
    std::shared_ptr<ModelState> buildModelState(const juce::File& file) const;
    void publishModelState(std::shared_ptr<ModelState> nextState);

    double hostSampleRate = 0.0;
    int maxBlockSize = 0;
    int preparedChannels = 0;

    juce::AudioBuffer<float> inputScratch;
    juce::AudioBuffer<float> outputScratch;

    juce::File loadedModelFile;
    juce::String lastError;
    std::shared_ptr<ModelState> activeModelState;
};
