#include "DriveModule.h"
#include <cmath>

void DriveModule::prepare(double sampleRate, int blockSize, int numChannels)
{
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(blockSize),
                                 static_cast<juce::uint32>(numChannels) };

    preGain.prepare(spec);
    preGain.setRampDurationSeconds(0.02);

    waveShaper.prepare(spec);
    waveShaper.functionToUse = [] (float x) { return std::tanh(x); };

    toneFilter.prepare(spec);
    toneFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    toneFilter.setResonance(0.7f);
}

void DriveModule::reset()
{
    preGain.reset();
    waveShaper.reset();
    toneFilter.reset();
}

void DriveModule::setDrive(float amount)
{
    drive = juce::jlimit(0.0f, 1.0f, amount);
    const float driveDb = juce::jmap(drive, 0.0f, 1.0f, 0.0f, 24.0f);
    preGain.setGainDecibels(driveDb);
}

void DriveModule::setTone(float amount)
{
    tone = juce::jlimit(0.0f, 1.0f, amount);
    const float cutoff = juce::jmap(tone, 0.0f, 1.0f, 800.0f, 8000.0f);
    toneFilter.setCutoffFrequency(cutoff);
}

void DriveModule::process(juce::AudioBuffer<float>& buffer)
{
    if (isBypassed())
        return;

    juce::dsp::AudioBlock<float> block(buffer);

    preGain.process(juce::dsp::ProcessContextReplacing<float>(block));
    waveShaper.process(juce::dsp::ProcessContextReplacing<float>(block));
    toneFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
}
