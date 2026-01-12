#include "NoiseGateModule.h"
#include <cmath>

void NoiseGateModule::prepare(double newSampleRate, int blockSize, int numChannels)
{
    sampleRate = newSampleRate;
    envelope.assign(static_cast<size_t>(numChannels), 0.0f);
    gate.assign(static_cast<size_t>(numChannels), 1.0f);
    updateReleaseCoeff();
}

void NoiseGateModule::reset()
{
    std::fill(envelope.begin(), envelope.end(), 0.0f);
    std::fill(gate.begin(), gate.end(), 1.0f);
}

void NoiseGateModule::setThresholdDb(float newThresholdDb)
{
    thresholdDb = newThresholdDb;
    thresholdLinear = juce::Decibels::decibelsToGain(thresholdDb, -120.0f);
}

void NoiseGateModule::setReleaseMs(float newReleaseMs)
{
    releaseMs = newReleaseMs;
    updateReleaseCoeff();
}

void NoiseGateModule::updateReleaseCoeff()
{
    const float releaseSeconds = juce::jmax(0.001f, releaseMs / 1000.0f);
    releaseCoeff = std::exp(-1.0f / static_cast<float>(sampleRate * releaseSeconds));
}

void NoiseGateModule::process(juce::AudioBuffer<float>& buffer)
{
    if (isBypassed())
        return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float env = envelope[static_cast<size_t>(ch)];
        float gateGain = gate[static_cast<size_t>(ch)];
        float* samples = buffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            const float input = samples[i];
            const float absSample = std::abs(input);

            if (absSample > env)
                env = absSample;
            else
                env = absSample + releaseCoeff * (env - absSample);

            if (env >= thresholdLinear)
                gateGain = 1.0f;
            else
                gateGain *= releaseCoeff;

            samples[i] = input * gateGain;
        }

        envelope[static_cast<size_t>(ch)] = env;
        gate[static_cast<size_t>(ch)] = gateGain;
    }
}
