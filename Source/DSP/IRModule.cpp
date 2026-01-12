#include "IRModule.h"

void IRModule::prepare(double sampleRate, int blockSize, int numChannels)
{
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(blockSize),
                                 static_cast<juce::uint32>(numChannels) };
    convolution.prepare(spec);
    dryBuffer.setSize(numChannels, blockSize);
    dryBuffer.clear();
}

void IRModule::reset()
{
    convolution.reset();
    dryBuffer.clear();
}

void IRModule::setMix(float newMix)
{
    mix = juce::jlimit(0.0f, 1.0f, newMix);
}

void IRModule::loadIRFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return;

    convolution.loadImpulseResponse(file,
                                    juce::dsp::Convolution::Stereo::no,
                                    juce::dsp::Convolution::Trim::yes,
                                    0,
                                    juce::dsp::Convolution::Normalise::yes);

    loadedFileName = file.getFileName();
    hasIR.store(true, std::memory_order_relaxed);
}

bool IRModule::hasImpulseResponse() const
{
    return hasIR.load(std::memory_order_relaxed);
}

juce::String IRModule::getLoadedFileName() const
{
    return loadedFileName;
}

void IRModule::process(juce::AudioBuffer<float>& buffer)
{
    if (isBypassed())
        return;

    if (!hasImpulseResponse())
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (mix < 0.999f)
    {
        for (int ch = 0; ch < numChannels; ++ch)
            dryBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    }

    juce::dsp::AudioBlock<float> block(buffer);
    convolution.process(juce::dsp::ProcessContextReplacing<float>(block));

    if (mix < 0.999f)
    {
        buffer.applyGain(mix);
        dryBuffer.applyGain(1.0f - mix);

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.addFrom(ch, 0, dryBuffer, ch, 0, numSamples);
    }
}
