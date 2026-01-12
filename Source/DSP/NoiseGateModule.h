#pragma once

#include "Module.h"
#include <vector>

class NoiseGateModule : public Module
{
public:
    void prepare(double sampleRate, int blockSize, int numChannels) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    void setThresholdDb(float thresholdDb);
    void setReleaseMs(float releaseMs);

private:
    void updateReleaseCoeff();

    double sampleRate = 44100.0;
    float thresholdDb = -60.0f;
    float thresholdLinear = 0.001f;
    float releaseMs = 100.0f;
    float releaseCoeff = 0.99f;
    std::vector<float> envelope;
    std::vector<float> gate;
};
