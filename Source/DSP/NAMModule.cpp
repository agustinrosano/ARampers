#include "NAMModule.h"

#include "NAM/container.h"
#include "NAM/convnet.h"
#include "NAM/linear.h"
#include "NAM/lstm.h"
#include "NAM/model_config.h"
#include "NAM/wavenet/model.h"

#include <cstring>
#include <fstream>

namespace
{
constexpr double fallbackModelSampleRate = 48000.0;

juce::String formatCalibrationText(bool hasInputCalibration, double inputCalibrationDbu,
                                   bool hasOutputCalibration, double outputCalibrationDbu)
{
    if (hasInputCalibration && hasOutputCalibration)
        return juce::String::formatted("Calibrated IN %.1f dBu | OUT %.1f dBu", inputCalibrationDbu, outputCalibrationDbu);

    if (hasInputCalibration)
        return juce::String::formatted("Calibrated IN %.1f dBu", inputCalibrationDbu);

    if (hasOutputCalibration)
        return juce::String::formatted("Calibrated OUT %.1f dBu", outputCalibrationDbu);

    return "No calibration metadata in this NAM";
}

void ensureNAMArchitectureRegistration()
{
    auto& registry = nam::ConfigParserRegistry::instance();

    if (!registry.has("Linear"))
        registry.registerParser("Linear", nam::linear::create_config);

    if (!registry.has("LSTM"))
        registry.registerParser("LSTM", nam::lstm::create_config);

    if (!registry.has("ConvNet"))
        registry.registerParser("ConvNet", nam::convnet::create_config);

    if (!registry.has("WaveNet"))
        registry.registerParser("WaveNet", nam::wavenet::create_config);

    if (!registry.has("SlimmableContainer"))
        registry.registerParser("SlimmableContainer", nam::container::create_config);
}
}

void NAMModule::prepare(double sampleRate, int blockSize, int numChannels)
{
    hostSampleRate = sampleRate;
    maxBlockSize = blockSize;
    preparedChannels = juce::jmax(1, numChannels);

    inputScratch.setSize(1, blockSize);
    outputScratch.setSize(1, blockSize);
    inputScratch.clear();
    outputScratch.clear();

    if (loadedModelFile.existsAsFile())
        loadModelFile(loadedModelFile);
}

void NAMModule::process(juce::AudioBuffer<float>& buffer)
{
    if (isBypassed())
        return;

    auto currentState = std::atomic_load(&activeModelState);
    if (currentState == nullptr || currentState->channelModels.empty())
        return;

    const int numSamples = buffer.getNumSamples();
    if (numSamples > inputScratch.getNumSamples())
    {
        inputScratch.setSize(1, numSamples, false, false, true);
        outputScratch.setSize(1, numSamples, false, false, true);
    }

    auto* inputWrite = inputScratch.getWritePointer(0);
    auto* outputWrite = outputScratch.getWritePointer(0);
    NAM_SAMPLE* inputPtrs[] = { inputWrite };
    NAM_SAMPLE* outputPtrs[] = { outputWrite };

    const int channelsToProcess = juce::jmin(buffer.getNumChannels(),
                                             static_cast<int>(currentState->channelModels.size()));

    for (int channel = 0; channel < channelsToProcess; ++channel)
    {
        std::memcpy(inputWrite, buffer.getReadPointer(channel), static_cast<size_t>(numSamples) * sizeof(float));
        currentState->channelModels[static_cast<size_t>(channel)]->process(inputPtrs, outputPtrs, numSamples);
        std::memcpy(buffer.getWritePointer(channel), outputWrite, static_cast<size_t>(numSamples) * sizeof(float));
    }
}

void NAMModule::reset()
{
    auto currentState = std::atomic_load(&activeModelState);
    if (currentState == nullptr)
        return;

    for (auto& model : currentState->channelModels)
        model->Reset(hostSampleRate, maxBlockSize);
}

bool NAMModule::loadModelFile(const juce::File& file)
{
    lastError.clear();

    if (!file.existsAsFile())
    {
        lastError = "Model file not found";
        return false;
    }

    try
    {
        ensureNAMArchitectureRegistration();
        auto nextState = buildModelState(file);
        loadedModelFile = file;
        publishModelState(std::move(nextState));
        return true;
    }
    catch (const std::exception& exception)
    {
        lastError = exception.what();
        return false;
    }
}

void NAMModule::clearModel()
{
    loadedModelFile = {};
    lastError.clear();
    publishModelState(nullptr);
}

bool NAMModule::hasModel() const
{
    return std::atomic_load(&activeModelState) != nullptr;
}

juce::String NAMModule::getLoadedFileName() const
{
    auto currentState = std::atomic_load(&activeModelState);
    return currentState != nullptr ? currentState->fileName : "No model loaded";
}

juce::String NAMModule::getLoadedFilePath() const
{
    return loadedModelFile.getFullPathName();
}

juce::String NAMModule::getArchitectureName() const
{
    auto currentState = std::atomic_load(&activeModelState);
    return currentState != nullptr ? currentState->architecture : "NAM capture";
}

juce::String NAMModule::getModelStatusText() const
{
    if (lastError.isNotEmpty())
        return "Load failed: " + lastError;

    auto currentState = std::atomic_load(&activeModelState);
    if (currentState != nullptr)
        return currentState->statusText;

    return "Load a .nam model to define the main tone";
}

juce::String NAMModule::getLastError() const
{
    return lastError;
}

bool NAMModule::hasInputCalibration() const
{
    auto currentState = std::atomic_load(&activeModelState);
    return currentState != nullptr && currentState->hasInputCalibration;
}

bool NAMModule::hasOutputCalibration() const
{
    auto currentState = std::atomic_load(&activeModelState);
    return currentState != nullptr && currentState->hasOutputCalibration;
}

double NAMModule::getInputCalibrationDbu() const
{
    auto currentState = std::atomic_load(&activeModelState);
    return currentState != nullptr ? currentState->inputCalibrationDbu : 0.0;
}

double NAMModule::getOutputCalibrationDbu() const
{
    auto currentState = std::atomic_load(&activeModelState);
    return currentState != nullptr ? currentState->outputCalibrationDbu : 0.0;
}

juce::String NAMModule::getCalibrationSummaryText() const
{
    auto currentState = std::atomic_load(&activeModelState);
    if (currentState == nullptr)
        return {};

    return formatCalibrationText(currentState->hasInputCalibration, currentState->inputCalibrationDbu,
                                 currentState->hasOutputCalibration, currentState->outputCalibrationDbu);
}

juce::String NAMModule::describeArchitecture(const nlohmann::json& modelJson)
{
    const auto architecture = juce::String(modelJson.value("architecture", "Unknown"));

    if (architecture == "SlimmableContainer")
        return "NAM A2 Slimmable";

    if (architecture == "WaveNet")
        return "NAM WaveNet";

    return "NAM " + architecture;
}

std::filesystem::path NAMModule::toFilesystemPath(const juce::File& file)
{
   #if JUCE_WINDOWS
    return std::filesystem::path(file.getFullPathName().toWideCharPointer());
   #else
    return std::filesystem::path(file.getFullPathName().toStdString());
   #endif
}

std::shared_ptr<NAMModule::ModelState> NAMModule::buildModelState(const juce::File& file) const
{
    std::ifstream inputStream(toFilesystemPath(file));
    if (!inputStream.is_open())
        throw std::runtime_error("Unable to open the selected NAM file");

    nlohmann::json modelJson;
    inputStream >> modelJson;

    auto nextState = std::make_shared<ModelState>();
    nextState->fileName = file.getFileName();
    nextState->fullPath = file.getFullPathName();
    nextState->architecture = describeArchitecture(modelJson);

    const auto channelCount = juce::jmax(1, preparedChannels);
    const auto effectiveSampleRate = hostSampleRate > 0.0 ? hostSampleRate : fallbackModelSampleRate;
    const auto effectiveBlockSize = juce::jmax(32, maxBlockSize);
    const auto modelSampleRate = nam::get_sample_rate_from_nam_file(modelJson);

    nam::DspLoadOptions loadOptions;
    loadOptions.prewarm = false;

    for (int channel = 0; channel < channelCount; ++channel)
    {
        auto model = nam::get_dsp(toFilesystemPath(file), loadOptions);

        if (model->NumInputChannels() != 1 || model->NumOutputChannels() != 1)
            throw std::runtime_error("Only mono NAM captures are supported at the moment");

        if (channel == 0)
        {
            nextState->hasInputCalibration = model->HasInputLevel();
            nextState->hasOutputCalibration = model->HasOutputLevel();

            if (nextState->hasInputCalibration)
                nextState->inputCalibrationDbu = model->GetInputLevel();

            if (nextState->hasOutputCalibration)
                nextState->outputCalibrationDbu = model->GetOutputLevel();
        }

        model->Reset(effectiveSampleRate, effectiveBlockSize);
        nextState->channelModels.push_back(std::move(model));
    }

    const auto calibrationText = formatCalibrationText(nextState->hasInputCalibration,
                                                       nextState->inputCalibrationDbu,
                                                       nextState->hasOutputCalibration,
                                                       nextState->outputCalibrationDbu);

    nextState->statusText = juce::String::formatted("%s | Model %.1f kHz | Host %.1f kHz | %d channel%s",
                                                    calibrationText.toRawUTF8(),
                                                    modelSampleRate > 0.0 ? modelSampleRate / 1000.0
                                                                          : fallbackModelSampleRate / 1000.0,
                                                    effectiveSampleRate / 1000.0,
                                                    channelCount,
                                                    channelCount == 1 ? "" : "s");

    return nextState;
}

void NAMModule::publishModelState(std::shared_ptr<ModelState> nextState)
{
    std::atomic_store(&activeModelState, std::move(nextState));
}
