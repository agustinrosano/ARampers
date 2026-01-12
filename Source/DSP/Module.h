#pragma once

#include <JuceHeader.h>
#include <atomic>

class Module
{
public:
    virtual ~Module() = default;

    virtual void prepare(double sampleRate, int blockSize, int numChannels) = 0;
    virtual void process(juce::AudioBuffer<float>& buffer) = 0;
    virtual void reset() = 0;

    void setBypass(bool shouldBypass)
    {
        bypassed.store(shouldBypass, std::memory_order_relaxed);
    }

    bool isBypassed() const
    {
        return bypassed.load(std::memory_order_relaxed);
    }

private:
    std::atomic<bool> bypassed { false };
};
