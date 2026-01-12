#include "EQModule.h"

void EQModule::prepare(double sampleRate, int blockSize, int numChannels)
{
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(blockSize),
                                 static_cast<juce::uint32>(numChannels) };

    lowFilter.prepare(spec);
    midFilter.prepare(spec);
    highFilter.prepare(spec);

    lowFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    midFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    highFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);

    lowFilter.setCutoffFrequency(200.0f);
    lowFilter.setResonance(0.7f);

    midFilter.setCutoffFrequency(1000.0f);
    midFilter.setResonance(0.8f);

    highFilter.setCutoffFrequency(3000.0f);
    highFilter.setResonance(0.7f);

    lowBuffer.setSize(numChannels, blockSize);
    midBuffer.setSize(numChannels, blockSize);
    highBuffer.setSize(numChannels, blockSize);
}

void EQModule::reset()
{
    lowFilter.reset();
    midFilter.reset();
    highFilter.reset();
}

void EQModule::setLowGainDb(float gainDb)
{
    lowGain = juce::Decibels::decibelsToGain(gainDb);
}

void EQModule::setMidGainDb(float gainDb)
{
    midGain = juce::Decibels::decibelsToGain(gainDb);
}

void EQModule::setHighGainDb(float gainDb)
{
    highGain = juce::Decibels::decibelsToGain(gainDb);
}

void EQModule::process(juce::AudioBuffer<float>& buffer)
{
    if (isBypassed())
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        lowBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        midBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        highBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    }

    {
        juce::dsp::AudioBlock<float> block(lowBuffer);
        lowFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
    }
    {
        juce::dsp::AudioBlock<float> block(midBuffer);
        midFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
    }
    {
        juce::dsp::AudioBlock<float> block(highBuffer);
        highFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
    }

    lowBuffer.applyGain(lowGain);
    midBuffer.applyGain(midGain);
    highBuffer.applyGain(highGain);

    buffer.clear();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        buffer.addFrom(ch, 0, lowBuffer, ch, 0, numSamples);
        buffer.addFrom(ch, 0, midBuffer, ch, 0, numSamples);
        buffer.addFrom(ch, 0, highBuffer, ch, 0, numSamples);
    }
}
