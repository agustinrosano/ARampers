#pragma once

#include "Module.h"

class GainModule : public Module
{
public:
    void prepare(double sampleRate, int blockSize, int numChannels) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    void setGainDecibels(float gainDb);

private:
    juce::dsp::Gain<float> gain;
};
