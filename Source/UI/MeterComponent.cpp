#include "MeterComponent.h"
#include "GoldLookAndFeel.h"

void MeterComponent::setLevel(float newLevel)
{
    level = juce::jlimit(0.0f, 1.0f, newLevel);
    repaint();
}

void MeterComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const int numSegments = 24;
    const float gap = 1.5f;
    const float segWidth = (bounds.getWidth() - (numSegments - 1) * gap) / numSegments;

    for (int i = 0; i < numSegments; ++i)
    {
        float segLimit = static_cast<float>(i) / static_cast<float>(numSegments);
        bool isLit = (level >= segLimit && level > 0.001f);

        auto segBounds = juce::Rectangle<float>(bounds.getX() + i * (segWidth + gap), bounds.getY(), segWidth, bounds.getHeight());

        if (isLit)
        {
            if (i > 20)
                g.setColour(Theme::danger);
            else if (i > 15)
                g.setColour(Theme::presetOrange);
            else
                g.setColour(Theme::success);
        }
        else
        {
            g.setColour(juce::Colour(0xff333333));
        }

        g.fillRect(segBounds);
    }
}
