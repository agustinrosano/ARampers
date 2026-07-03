#pragma once

#include <JuceHeader.h>

class IRLoaderComponent : public juce::Component
{
public:
    IRLoaderComponent();

    std::function<void()> onLoadClicked;
    std::function<void()> onClearClicked;
    void setFileName(const juce::String& name);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label titleLabel;
    juce::TextButton loadButton { "LOAD..." };
    juce::TextButton clearButton { "Clear" };
    juce::Label fileLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IRLoaderComponent)
};
