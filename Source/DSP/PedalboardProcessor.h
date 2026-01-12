#pragma once

#include "GainModule.h"
#include "NoiseGateModule.h"
#include "DriveModule.h"
#include "AmpModule.h"
#include "IRModule.h"
#include "EQModule.h"
#include <array>

class PedalboardProcessor
{
public:
    PedalboardProcessor();

    void prepare(double sampleRate, int blockSize, int numChannels);
    void reset();
    void process(juce::AudioBuffer<float>& buffer);

    GainModule inputGain;
    NoiseGateModule noiseGate;
    DriveModule drive;
    AmpModule amp;
    IRModule ir;
    EQModule eq;
    GainModule outputGain;

private:
    std::array<Module*, 7> chain;
};
