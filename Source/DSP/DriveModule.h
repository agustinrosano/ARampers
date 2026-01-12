#pragma once

#include "Module.h"

class DriveModule : public Module
{
public:
    void prepare(double sampleRate, int blockSize, int numChannels) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    void setDrive(float amount);
    void setTone(float amount);

private:
    juce::dsp::Gain<float> preGain;
    juce::dsp::WaveShaper<float> waveShaper;
    juce::dsp::StateVariableTPTFilter<float> toneFilter;
    float drive = 0.0f;
    float tone = 0.5f;
};
