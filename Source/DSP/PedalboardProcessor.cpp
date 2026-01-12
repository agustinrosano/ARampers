#include "PedalboardProcessor.h"

PedalboardProcessor::PedalboardProcessor()
{
    chain = { &inputGain, &noiseGate, &drive, &amp, &ir, &eq, &outputGain };
}

void PedalboardProcessor::prepare(double sampleRate, int blockSize, int numChannels)
{
    for (auto* module : chain)
        module->prepare(sampleRate, blockSize, numChannels);
}

void PedalboardProcessor::reset()
{
    for (auto* module : chain)
        module->reset();
}

void PedalboardProcessor::process(juce::AudioBuffer<float>& buffer)
{
    for (auto* module : chain)
        module->process(buffer);
}
