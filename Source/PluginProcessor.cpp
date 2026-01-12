#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    paramRefs.driveAmount = apvts.getRawParameterValue(ParamIDs::driveAmount);
    paramRefs.driveTone = apvts.getRawParameterValue(ParamIDs::driveTone);
    paramRefs.driveBypass = apvts.getRawParameterValue(ParamIDs::driveBypass);
    paramRefs.ampGain = apvts.getRawParameterValue(ParamIDs::ampGain);
    paramRefs.ampTone = apvts.getRawParameterValue(ParamIDs::ampTone);
    paramRefs.ampBypass = apvts.getRawParameterValue(ParamIDs::ampBypass);
    paramRefs.irMix = apvts.getRawParameterValue(ParamIDs::irMix);
    paramRefs.irBypass = apvts.getRawParameterValue(ParamIDs::irBypass);
    paramRefs.eqLow = apvts.getRawParameterValue(ParamIDs::eqLow);
    paramRefs.eqMid = apvts.getRawParameterValue(ParamIDs::eqMid);
    paramRefs.eqHigh = apvts.getRawParameterValue(ParamIDs::eqHigh);
    paramRefs.eqBypass = apvts.getRawParameterValue(ParamIDs::eqBypass);
    paramRefs.outputGain = apvts.getRawParameterValue(ParamIDs::outputGain);
    paramRefs.outputBypass = apvts.getRawParameterValue(ParamIDs::outputBypass);

    buildFactoryPresets();
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

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    updateParametersFromAPVTS();
    pedalboard.process(buffer);
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
    if (auto state = apvts.copyState(); state.isValid())
    {
        if (auto xml = state.createXml())
            copyXmlToBinary(*xml, destData);
    }
}

void GoldPedalAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        auto newState = juce::ValueTree::fromXml(*xml);
        if (newState.isValid())
            apvts.replaceState(newState);
    }
}

void GoldPedalAudioProcessor::updateParametersFromAPVTS()
{
    pedalboard.inputGain.setGainDecibels(paramRefs.inputGain->load());
    pedalboard.inputGain.setBypass(paramRefs.inputBypass->load() > 0.5f);

    pedalboard.noiseGate.setThresholdDb(paramRefs.gateThreshold->load());
    pedalboard.noiseGate.setReleaseMs(paramRefs.gateRelease->load());
    pedalboard.noiseGate.setBypass(paramRefs.gateBypass->load() > 0.5f);

    pedalboard.drive.setDrive(paramRefs.driveAmount->load());
    pedalboard.drive.setTone(paramRefs.driveTone->load());
    pedalboard.drive.setBypass(paramRefs.driveBypass->load() > 0.5f);

    pedalboard.amp.setDrive(paramRefs.ampGain->load());
    pedalboard.amp.setTone(paramRefs.ampTone->load());
    pedalboard.amp.setBypass(paramRefs.ampBypass->load() > 0.5f);

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
}

juce::String GoldPedalAudioProcessor::getLoadedIRName() const
{
    if (irFileName.isEmpty())
        return "No IR loaded";

    return irFileName;
}

void GoldPedalAudioProcessor::savePresetToFile(const juce::File& file)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        xml->writeTo(file);
}

void GoldPedalAudioProcessor::loadPresetFromFile(const juce::File& file)
{
    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr)
        return;

    auto newState = juce::ValueTree::fromXml(*xml);
    if (newState.isValid())
        apvts.replaceState(newState);
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
            { ParamIDs::driveAmount, 0.10f },
            { ParamIDs::driveTone, 0.60f },
            { ParamIDs::driveBypass, 0.0f },
            { ParamIDs::ampGain, 0.25f },
            { ParamIDs::ampTone, 0.60f },
            { ParamIDs::ampBypass, 0.0f },
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
            { ParamIDs::driveAmount, 0.45f },
            { ParamIDs::driveTone, 0.55f },
            { ParamIDs::driveBypass, 0.0f },
            { ParamIDs::ampGain, 0.55f },
            { ParamIDs::ampTone, 0.50f },
            { ParamIDs::ampBypass, 0.0f },
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
            { ParamIDs::driveAmount, 0.70f },
            { ParamIDs::driveTone, 0.50f },
            { ParamIDs::driveBypass, 0.0f },
            { ParamIDs::ampGain, 0.70f },
            { ParamIDs::ampTone, 0.55f },
            { ParamIDs::ampBypass, 0.0f },
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

juce::AudioProcessorValueTreeState::ParameterLayout GoldPedalAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::inputGain, "Input Gain",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.01f), 0.0f));
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

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::driveAmount, "Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::driveTone, "Drive Tone",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamIDs::driveBypass, "Drive Bypass", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::ampGain, "Amp Gain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.3f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamIDs::ampTone, "Amp Tone",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.6f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamIDs::ampBypass, "Amp Bypass", false));

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
        ParamIDs::outputGain, "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 0.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        ParamIDs::outputBypass, "Output Bypass", false));

    return { params.begin(), params.end() };
}
