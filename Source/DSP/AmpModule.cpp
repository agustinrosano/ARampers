#include "AmpModule.h"
#include <cmath>

void AmpModule::prepare(double newSampleRate, int blockSize, int numChannels)
{
    sampleRate = newSampleRate;

    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(blockSize),
                                 static_cast<juce::uint32>(numChannels) };

    preGain.prepare(spec);
    preGain.setRampDurationSeconds(0.02);

    waveShaper.prepare(spec);
    waveShaper.functionToUse = [] (float x) { return std::tanh(x); };

    highPass.prepare(spec);
    highPass.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 80.0);

    toneFilter.prepare(spec);
    toneFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    toneFilter.setResonance(0.8f);
}

void AmpModule::reset()
{
    preGain.reset();
    waveShaper.reset();
    highPass.reset();
    toneFilter.reset();
}

void AmpModule::setDrive(float amount)
{
    drive = juce::jlimit(0.0f, 1.0f, amount);
    const float driveDb = juce::jmap(drive, 0.0f, 1.0f, 6.0f, 30.0f);
    preGain.setGainDecibels(driveDb);
}

void AmpModule::setTone(float amount)
{
    tone = juce::jlimit(0.0f, 1.0f, amount);
    const float cutoff = juce::jmap(tone, 0.0f, 1.0f, 1200.0f, 9000.0f);
    toneFilter.setCutoffFrequency(cutoff);
}

void AmpModule::process(juce::AudioBuffer<float>& buffer)
{
    if (isBypassed())
        return;

    juce::dsp::AudioBlock<float> block(buffer);

    preGain.process(juce::dsp::ProcessContextReplacing<float>(block));
    waveShaper.process(juce::dsp::ProcessContextReplacing<float>(block));
    highPass.process(juce::dsp::ProcessContextReplacing<float>(block));
    toneFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
}
