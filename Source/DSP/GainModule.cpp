#include "GainModule.h"

void GainModule::prepare(double sampleRate, int blockSize, int numChannels)
{
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(blockSize),
                                 static_cast<juce::uint32>(numChannels) };
    gain.prepare(spec);
    gain.setRampDurationSeconds(0.02);
}

void GainModule::process(juce::AudioBuffer<float>& buffer)
{
    if (isBypassed())
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    gain.process(context);
}

void GainModule::reset()
{
    gain.reset();
}

void GainModule::setGainDecibels(float gainDb)
{
    gain.setGainDecibels(gainDb);
}
