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
    auto track = bounds.reduced(10.0f, 14.0f);

    g.setColour(Theme::panel.darker(0.22f));
    g.fillRoundedRectangle(track, 12.0f);
    g.setColour(Theme::panelOutline);
    g.drawRoundedRectangle(track, 12.0f, 1.0f);

    auto meter = track.reduced(5.0f);
    const auto filledHeight = meter.getHeight() * level;
    auto fill = meter.withY(meter.getBottom() - filledHeight).withHeight(filledHeight);

    juce::Colour fillTop = Theme::success;
    if (level > 0.82f)
        fillTop = Theme::danger.brighter(0.15f);
    else if (level > 0.58f)
        fillTop = Theme::accent;

    g.setColour(fillTop.withAlpha(0.12f));
    g.fillRoundedRectangle(meter, 8.0f);
    g.setGradientFill(juce::ColourGradient(fillTop, fill.getCentreX(), fill.getY(),
                                           fillTop.darker(0.35f), fill.getCentreX(), fill.getBottom(), false));
    g.fillRoundedRectangle(fill, 9.0f);

    g.setColour(Theme::textSecondary.withAlpha(0.35f));
    for (int step = 1; step < 5; ++step)
    {
        const auto y = meter.getY() + (meter.getHeight() * (static_cast<float>(step) / 5.0f));
        g.drawHorizontalLine(static_cast<int>(y), meter.getX(), meter.getRight());
    }
}
