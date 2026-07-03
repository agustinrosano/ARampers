#pragma once

#include <JuceHeader.h>

class MeterComponent : public juce::Component
{
public:
    MeterComponent() = default;

    void setLevel(float newLevel);

    void paint(juce::Graphics& g) override;

private:
    float level = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterComponent)
};
