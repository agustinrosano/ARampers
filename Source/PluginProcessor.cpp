#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr auto stateModelPath = "loadedModelPath";
constexpr auto stateIRPath = "loadedIRPath";
constexpr auto presetBankRootTag = "ADR_AMPER_PRESET_BANK";
constexpr auto legacyPresetBankRootTag = "ACR_AMPER_PRESET_BANK";
constexpr auto appDataFolderName = "ADR-AMPER";
constexpr auto legacyAppDataFolderName = "ACR-AMPER";
constexpr auto presetSlotTag = "SLOT";
constexpr auto emptySlotName = "Empty slot";

juce::File getLegacyUserPresetBankFile()
{
    auto directory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                         .getChildFile(legacyAppDataFolderName);
    return directory.getChildFile("preset-bank.xml");
}

void normaliseStandaloneMonoInputToStereo(juce::AudioBuffer<float>& buffer, int totalNumInputChannels, int totalNumOutputChannels)
{
   #if JucePlugin_Build_Standalone
    if (totalNumInputChannels < 2 || totalNumOutputChannels < 2 || buffer.getNumChannels() < 2)
        return;

    constexpr float silenceThreshold = 1.0e-4f;
    constexpr float dominantChannelRatio = 6.0f;
    const auto leftMagnitude = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    const auto rightMagnitude = buffer.getMagnitude(1, 0, buffer.getNumSamples());

    if (leftMagnitude > silenceThreshold && rightMagnitude <= silenceThreshold)
    {
        buffer.copyFrom(1, 0, buffer, 0, 0, buffer.getNumSamples());
    }
    else if (rightMagnitude > silenceThreshold && leftMagnitude <= silenceThreshold)
    {
        buffer.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    }
    else if (leftMagnitude > silenceThreshold && leftMagnitude > rightMagnitude * dominantChannelRatio)
    {
        buffer.copyFrom(1, 0, buffer, 0, 0, buffer.getNumSamples());
    }
    else if (rightMagnitude > silenceThreshold && rightMagnitude > leftMagnitude * dominantChannelRatio)
    {
        buffer.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    }
   #else
    juce::ignoreUnused(buffer, totalNumInputChannels, totalNumOutputChannels);
   #endif
}
}

GoldPedalAudioProcessor::GoldPedalAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    paramRefs.inputGain = apvts.getRawParameterValue(ParamIDs::inputGain);
    paramRefs.inputBypass = apvts.getRawParameterValue(ParamIDs::inputBypass);
    paramRefs.gateThreshold = apvts.getRawParameterValue(ParamIDs::gateThreshold);
    paramRefs.gateRelease = apvts.getRawParameterValue(ParamIDs::gateRelease);
    paramRefs.gateBypass = apvts.getRawParameterValue(ParamIDs::gateBypass);
    paramRefs.modelBypass = apvts.getRawParameterValue(ParamIDs::modelBypass);
    paramRefs.irMix = apvts.getRawParameterValue(ParamIDs::irMix);
    paramRefs.irBypass = apvts.getRawParameterValue(ParamIDs::irBypass);
    paramRefs.eqLow = apvts.getRawParameterValue(ParamIDs::eqLow);
    paramRefs.eqMid = apvts.getRawParameterValue(ParamIDs::eqMid);
    paramRefs.eqHigh = apvts.getRawParameterValue(ParamIDs::eqHigh);
    paramRefs.eqBypass = apvts.getRawParameterValue(ParamIDs::eqBypass);
    paramRefs.outputGain = apvts.getRawParameterValue(ParamIDs::outputGain);
    paramRefs.outputBypass = apvts.getRawParameterValue(ParamIDs::outputBypass);

    buildFactoryPresets();

    for (size_t index = 0; index < userPresetSlots.size(); ++index)
        userPresetSlots[index].info.name = juce::String(emptySlotName) + " " + juce::String(static_cast<int>(index + 1));

    loadUserPresetBankFromDisk();
}

GoldPedalAudioProcessor::~GoldPedalAudioProcessor() = default;

const juce::String GoldPedalAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GoldPedalAudioProcessor::acceptsMidi() const
{
    return false;
}

bool GoldPedalAudioProcessor::producesMidi() const
{
    return false;
}

bool GoldPedalAudioProcessor::isMidiEffect() const
{
    return false;
}

double GoldPedalAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GoldPedalAudioProcessor::getNumPrograms()
{
    return 1;
}

int GoldPedalAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GoldPedalAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String GoldPedalAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void GoldPedalAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void GoldPedalAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    pedalboard.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    pedalboard.reset();
}

void GoldPedalAudioProcessor::releaseResources()
{
}

bool GoldPedalAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    const auto& mainIn = layouts.getMainInputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    if (mainOut != mainIn)
        return false;

    return true;
}

void GoldPedalAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    normaliseStandaloneMonoInputToStereo(buffer, totalNumInputChannels, totalNumOutputChannels);
    inputMeterLevel.store(measureBufferPeak(buffer), std::memory_order_relaxed);
    updateParametersFromAPVTS();
    pedalboard.process(buffer);
    outputMeterLevel.store(measureBufferPeak(buffer), std::memory_order_relaxed);
}

bool GoldPedalAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* GoldPedalAudioProcessor::createEditor()
{
    return new GoldPedalAudioProcessorEditor(*this);
}

void GoldPedalAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = createStateXmlWithAssets())
        copyXmlToBinary(*xml, destData);
}

void GoldPedalAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        applyStateXml(*xml);
}

void GoldPedalAudioProcessor::updateParametersFromAPVTS()
{
    pedalboard.inputGain.setGainDecibels(paramRefs.inputGain->load());
    pedalboard.inputGain.setBypass(paramRefs.inputBypass->load() > 0.5f);

    pedalboard.noiseGate.setThresholdDb(paramRefs.gateThreshold->load());
    pedalboard.noiseGate.setReleaseMs(paramRefs.gateRelease->load());
    pedalboard.noiseGate.setBypass(paramRefs.gateBypass->load() > 0.5f);

    pedalboard.nam.setBypass(paramRefs.modelBypass->load() > 0.5f);

    pedalboard.ir.setMix(paramRefs.irMix->load());
    pedalboard.ir.setBypass(paramRefs.irBypass->load() > 0.5f);

    pedalboard.eq.setLowGainDb(paramRefs.eqLow->load());
    pedalboard.eq.setMidGainDb(paramRefs.eqMid->load());
    pedalboard.eq.setHighGainDb(paramRefs.eqHigh->load());
    pedalboard.eq.setBypass(paramRefs.eqBypass->load() > 0.5f);

    pedalboard.outputGain.setGainDecibels(paramRefs.outputGain->load());
    pedalboard.outputGain.setBypass(paramRefs.outputBypass->load() > 0.5f);
}

void GoldPedalAudioProcessor::loadIRFile(const juce::File& file)
{
    pedalboard.ir.loadIRFile(file);
    irFileName = file.getFileName();
    irFilePath = file.getFullPathName();
}

void GoldPedalAudioProcessor::clearIRFile()
{
    pedalboard.ir.clearIR();
    irFileName.clear();
    irFilePath.clear();
}

juce::String GoldPedalAudioProcessor::getLoadedIRName() const
{
    return irFileName.isEmpty() ? "No IR loaded" : irFileName;
}

juce::String GoldPedalAudioProcessor::getLoadedIRPath() const
{
    return irFilePath;
}

bool GoldPedalAudioProcessor::loadNAMModelFile(const juce::File& file)
{
    return pedalboard.nam.loadModelFile(file);
}

void GoldPedalAudioProcessor::clearNAMModel()
{
    pedalboard.nam.clearModel();
}

juce::String GoldPedalAudioProcessor::getLoadedNAMName() const
{
    return pedalboard.nam.getLoadedFileName();
}

juce::String GoldPedalAudioProcessor::getLoadedNAMArchitecture() const
{
    return pedalboard.nam.getArchitectureName();
}

juce::String GoldPedalAudioProcessor::getLoadedNAMStatus() const
{
    return pedalboard.nam.getModelStatusText();
}

bool GoldPedalAudioProcessor::hasNAMLoadError() const
{
    return pedalboard.nam.getLastError().isNotEmpty();
}

float GoldPedalAudioProcessor::getInputMeterLevel() const
{
    return inputMeterLevel.load(std::memory_order_relaxed);
}

float GoldPedalAudioProcessor::getOutputMeterLevel() const
{
    return outputMeterLevel.load(std::memory_order_relaxed);
}

void GoldPedalAudioProcessor::savePresetToFile(const juce::File& file)
{
    if (auto xml = createStateXmlWithAssets())
        xml->writeTo(file);
}

void GoldPedalAudioProcessor::loadPresetFromFile(const juce::File& file)
{
    if (auto xml = juce::XmlDocument::parse(file))
        applyStateXml(*xml);
}

juce::StringArray GoldPedalAudioProcessor::getFactoryPresetNames() const
{
    return factoryPresetNames;
}

void GoldPedalAudioProcessor::applyFactoryPreset(int index)
{
    if (index < 0 || index >= static_cast<int>(factoryPresets.size()))
        return;

    const auto& preset = factoryPresets[static_cast<size_t>(index)];
    for (const auto& entry : preset.values)
    {
        if (auto* param = apvts.getParameter(entry.first))
            param->setValueNotifyingHost(param->convertTo0to1(entry.second));
    }
}

int GoldPedalAudioProcessor::getNumUserPresetSlots() const
{
    return static_cast<int>(userPresetSlots.size());
}

GoldPedalAudioProcessor::UserPresetSlotInfo GoldPedalAudioProcessor::getUserPresetSlotInfo(int index) const
{
    if (index < 0 || index >= static_cast<int>(userPresetSlots.size()))
        return {};

    return userPresetSlots[static_cast<size_t>(index)].info;
}

void GoldPedalAudioProcessor::storeUserPresetSlot(int index)
{
    if (index < 0 || index >= static_cast<int>(userPresetSlots.size()))
        return;

    auto xml = createStateXmlWithAssets();
    if (xml == nullptr)
        return;

    auto& slot = userPresetSlots[static_cast<size_t>(index)];
    if (!slot.info.occupied || slot.info.name.startsWithIgnoreCase(emptySlotName))
    {
        const auto suggestedName = pedalboard.nam.hasModel() ? pedalboard.nam.getLoadedFileName()
                                                             : juce::String("Preset ") + juce::String(index + 1);
        slot.info.name = suggestedName;
    }

    slot.info.occupied = true;
    slot.info.modelName = pedalboard.nam.hasModel() ? pedalboard.nam.getLoadedFileName() : "NAM only";
    slot.info.irName = irFileName.isNotEmpty() ? irFileName : "No IR";
    slot.serializedState = xml->toString();
    saveUserPresetBankToDisk();
}

void GoldPedalAudioProcessor::loadUserPresetSlot(int index)
{
    if (index < 0 || index >= static_cast<int>(userPresetSlots.size()))
        return;

    const auto& slot = userPresetSlots[static_cast<size_t>(index)];
    if (!slot.info.occupied || slot.serializedState.isEmpty())
        return;

    if (auto xml = juce::XmlDocument::parse(slot.serializedState))
        applyStateXml(*xml);
}

void GoldPedalAudioProcessor::clearUserPresetSlot(int index)
{
    if (index < 0 || index >= static_cast<int>(userPresetSlots.size()))
        return;

    auto& slot = userPresetSlots[static_cast<size_t>(index)];
    slot = {};
    slot.info.name = juce::String(emptySlotName) + " " + juce::String(index + 1);
    saveUserPresetBankToDisk();
}

void GoldPedalAudioProcessor::renameUserPresetSlot(int index, const juce::String& newName)
{
    if (index < 0 || index >= static_cast<int>(userPresetSlots.size()))
        return;

    auto& slot = userPresetSlots[static_cast<size_t>(index)];
    slot.info.name = newName.trim().isEmpty() ? slot.info.name : newName.trim();
    saveUserPresetBankToDisk();
}

void GoldPedalAudioProcessor::buildFactoryPresets()
{
    factoryPresets.clear();

    factoryPresets.push_back({ "Clean",
        {
            { ParamIDs::inputGain, 0.0f },
            { ParamIDs::inputBypass, 0.0f },
            { ParamIDs::gateThreshold, -55.0f },
            { ParamIDs::gateRelease, 120.0f },
            { ParamIDs::gateBypass, 0.0f },
            { ParamIDs::modelBypass, 0.0f },
            { ParamIDs::irMix, 0.70f },
            { ParamIDs::irBypass, 0.0f },
            { ParamIDs::eqLow, 1.5f },
            { ParamIDs::eqMid, 0.0f },
            { ParamIDs::eqHigh, 1.0f },
            { ParamIDs::eqBypass, 0.0f },
            { ParamIDs::outputGain, -6.0f },
            { ParamIDs::outputBypass, 0.0f }
        }
    });

    factoryPresets.push_back({ "Crunch",
        {
            { ParamIDs::inputGain, 3.0f },
            { ParamIDs::inputBypass, 0.0f },
            { ParamIDs::gateThreshold, -50.0f },
            { ParamIDs::gateRelease, 140.0f },
            { ParamIDs::gateBypass, 0.0f },
            { ParamIDs::modelBypass, 0.0f },
            { ParamIDs::irMix, 0.85f },
            { ParamIDs::irBypass, 0.0f },
            { ParamIDs::eqLow, 1.0f },
            { ParamIDs::eqMid, 2.0f },
            { ParamIDs::eqHigh, 0.5f },
            { ParamIDs::eqBypass, 0.0f },
            { ParamIDs::outputGain, -8.0f },
            { ParamIDs::outputBypass, 0.0f }
        }
    });

    factoryPresets.push_back({ "Lead",
        {
            { ParamIDs::inputGain, 4.0f },
            { ParamIDs::inputBypass, 0.0f },
            { ParamIDs::gateThreshold, -48.0f },
            { ParamIDs::gateRelease, 160.0f },
            { ParamIDs::gateBypass, 0.0f },
            { ParamIDs::modelBypass, 0.0f },
            { ParamIDs::irMix, 0.90f },
            { ParamIDs::irBypass, 0.0f },
            { ParamIDs::eqLow, 0.5f },
            { ParamIDs::eqMid, 3.0f },
            { ParamIDs::eqHigh, 1.5f },
            { ParamIDs::eqBypass, 0.0f },
            { ParamIDs::outputGain, -10.0f },
            { ParamIDs::outputBypass, 0.0f }
        }
    });

    factoryPresetNames.clear();
    for (const auto& preset : factoryPresets)
        factoryPresetNames.add(preset.name);
}

std::unique_ptr<juce::XmlElement> GoldPedalAudioProcessor::createStateXmlWithAssets()
{
    auto state = apvts.copyState();
    if (!state.isValid())
        return {};

    auto xml = state.createXml();
    if (xml == nullptr)
        return {};

    xml->setAttribute(stateModelPath, pedalboard.nam.getLoadedFilePath());
    xml->setAttribute(stateIRPath, irFilePath);
    return xml;
}

void GoldPedalAudioProcessor::applyStateXml(const juce::XmlElement& xml)
{
    auto newState = juce::ValueTree::fromXml(xml);
    if (newState.isValid())
        apvts.replaceState(newState);

    const auto modelPath = xml.getStringAttribute(stateModelPath);
    if (modelPath.isNotEmpty())
    {
        const auto file = juce::File(modelPath);
        if (file.existsAsFile())
            loadNAMModelFile(file);
        else
            clearNAMModel();
    }
    else
    {
        clearNAMModel();
    }

    const auto savedIRPath = xml.getStringAttribute(stateIRPath);
    if (savedIRPath.isNotEmpty())
    {
        const auto file = juce::File(savedIRPath);
        if (file.existsAsFile())
            loadIRFile(file);
        else
            clearIRFile();
    }
    else
    {
        clearIRFile();
    }
}

juce::File GoldPedalAudioProcessor::getUserPresetBankFile()
{
    auto directory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                         .getChildFile(appDataFolderName);
    directory.createDirectory();
    return directory.getChildFile("preset-bank.xml");
}

void GoldPedalAudioProcessor::loadUserPresetBankFromDisk()
{
    auto file = getUserPresetBankFile();
    if (!file.existsAsFile())
        file = getLegacyUserPresetBankFile();

    if (!file.existsAsFile())
        return;

    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr || (!xml->hasTagName(presetBankRootTag) && !xml->hasTagName(legacyPresetBankRootTag)))
        return;

    forEachXmlChildElementWithTagName(*xml, slotXml, presetSlotTag)
    {
        const auto index = slotXml->getIntAttribute("index", -1);
        if (index < 0 || index >= static_cast<int>(userPresetSlots.size()))
            continue;

        auto& slot = userPresetSlots[static_cast<size_t>(index)];
        slot.info.name = slotXml->getStringAttribute("name", slot.info.name);
        slot.info.modelName = slotXml->getStringAttribute("modelName");
        slot.info.irName = slotXml->getStringAttribute("irName");
        slot.info.occupied = slotXml->getBoolAttribute("occupied", false);

        if (auto* stateXml = slotXml->getFirstChildElement())
            slot.serializedState = stateXml->toString();
    }
}

void GoldPedalAudioProcessor::saveUserPresetBankToDisk() const
{
    juce::XmlElement xml(presetBankRootTag);

    for (size_t index = 0; index < userPresetSlots.size(); ++index)
    {
        const auto& slot = userPresetSlots[index];

        juce::XmlElement* slotXml = xml.createNewChildElement(presetSlotTag);
        slotXml->setAttribute("index", static_cast<int>(index));
        slotXml->setAttribute("name", slot.info.name);
        slotXml->setAttribute("modelName", slot.info.modelName);
        slotXml->setAttribute("irName", slot.info.irName);
        slotXml->setAttribute("occupied", slot.info.occupied);

        if (slot.info.occupied && slot.serializedState.isNotEmpty())
        {
            if (auto stateXml = juce::XmlDocument::parse(slot.serializedState))
                slotXml->addChildElement(new juce::XmlElement(*stateXml));
        }
    }

    xml.writeTo(getUserPresetBankFile());
}

juce::AudioProcessorValueTreeState::ParameterLayout GoldPedalAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::inputGain, "Input",
        juce::NormalisableRange<float>(-18.0f, 18.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamIDs::inputBypass, "Input Bypass", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::gateThreshold, "Gate Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -55.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::gateRelease, "Gate Release",
        juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f), 120.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamIDs::gateBypass, "Gate Bypass", false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamIDs::modelBypass, "Model Bypass", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::irMix, "IR Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamIDs::irBypass, "IR Bypass", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::eqLow, "EQ Low",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::eqMid, "EQ Mid",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::eqHigh, "EQ High",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamIDs::eqBypass, "EQ Bypass", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::outputGain, "Output",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamIDs::outputBypass, "Output Bypass", false));

    return { params.begin(), params.end() };
}

float GoldPedalAudioProcessor::measureBufferPeak(const juce::AudioBuffer<float>& buffer)
{
    float peak = 0.0f;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        peak = juce::jmax(peak, buffer.getMagnitude(channel, 0, buffer.getNumSamples()));

    return peak;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GoldPedalAudioProcessor();
}
