#pragma once

#include "Module.h"
#include <atomic>

class IRModule : public Module
{
public:
    void prepare(double sampleRate, int blockSize, int numChannels) override;
    void process(juce::AudioBuffer<float>& buffer) override;
    void reset() override;

    void setMix(float newMix);
    void loadIRFile(const juce::File& file);
    bool hasImpulseResponse() const;
    juce::String getLoadedFileName() const;

private:
    juce::dsp::Convolution convolution;
    juce::AudioBuffer<float> dryBuffer;
    std::atomic<bool> hasIR { false };
    float mix = 1.0f;
    juce::String loadedFileName;
};
