#pragma once

#include "Module.h"

class EQModule : public Module
{
public:
    void prepare(double sampleRate, int blockSize, int numChannels) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    void setLowGainDb(float gainDb);
    void setMidGainDb(float gainDb);
    void setHighGainDb(float gainDb);

private:
    juce::dsp::StateVariableTPTFilter<float> lowFilter;
    juce::dsp::StateVariableTPTFilter<float> midFilter;
    juce::dsp::StateVariableTPTFilter<float> highFilter;

    juce::AudioBuffer<float> lowBuffer;
    juce::AudioBuffer<float> midBuffer;
    juce::AudioBuffer<float> highBuffer;

    float lowGain = 1.0f;
    float midGain = 1.0f;
    float highGain = 1.0f;
};
