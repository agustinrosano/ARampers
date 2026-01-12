#pragma once

#include <JuceHeader.h>

class IRLoaderComponent : public juce::Component
{
public:
    IRLoaderComponent();

    std::function<void()> onLoadClicked;
    void setFileName(const juce::String& name);

    void resized() override;

private:
    juce::TextButton loadButton { "Load IR" };
    juce::Label fileLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IRLoaderComponent)
};
