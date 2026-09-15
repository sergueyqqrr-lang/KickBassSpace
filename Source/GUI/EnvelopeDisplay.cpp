#include "EnvelopeDisplay.h"
#include "LookAndFeel.h"
#include "../DSP/MultibandEnergyAnalyzer.h"

EnvelopeDisplay::EnvelopeDisplay (KickBassSpaceAudioProcessor& proc) : processor (proc)
{
    startTimerHz (30);
}

EnvelopeDisplay::~EnvelopeDisplay() { stopTimer(); }

void EnvelopeDisplay::resized()
{
    auto area = getLocalBounds().toFloat().reduced (8.0f);
    bandBreakdownArea = area.removeFromBottom (70.0f);
    area.removeFromBottom (8.0f);
    mainPlotArea = area;
}

void EnvelopeDisplay::timerCallback()
{
    processor.getEnvelopeSnapshot (snapshot);
    repaint();
}

void EnvelopeDisplay::paint (juce::Graphics& g)
{
    g.fillAll (SpaceColours::background);

    auto plotArea = mainPlotArea;
    auto legendArea = plotArea.removeFromTop (20.0f);

    drawGrid (g, plotArea);

    drawEnvelopeTrace (g, plotArea, snapshot.bass, snapshot.writePos, SpaceColours::bass, 0.8f, false);
    drawEnvelopeTrace (g, plotArea, snapshot.kick, snapshot.writePos, SpaceColours::kick, 0.9f, false);
    drawEnvelopeTrace (g, plotArea, snapshot.conflict, snapshot.writePos, SpaceColours::conflict, 1.0f, true);

    drawLegend (g, legendArea);

    if (! snapshot.sidechainConnected)
        drawSidechainWarning (g, plotArea);

    drawBandBreakdown (g, bandBreakdownArea);
}

void EnvelopeDisplay::drawGrid (juce::Graphics& g, juce::Rectangle<float> area)
{
    for (int i = 0; i <= 4; ++i)
    {
        auto y = area.getY() + (float) i / 4.0f * area.getHeight();
        g.setColour (SpaceColours::gridLine);
        g.drawHorizontalLine ((int) y, area.getX(), area.getRight());
    }

    for (int i = 0; i <= 8; ++i)
    {
        auto x = area.getX() + (float) i / 8.0f * area.getWidth();
        g.setColour (SpaceColours::gridLine);
        g.drawVerticalLine ((int) x, area.getY(), area.getBottom());
    }

    g.setColour (SpaceColours::textDim);
    g.setFont (10.0f);
    g.drawText ("~4 segundos", (int) area.getX(), (int) area.getBottom() - 14, 100, 12, juce::Justification::left);
}

void EnvelopeDisplay::drawEnvelopeTrace (juce::Graphics& g, juce::Rectangle<float> area,
                                          const std::array<float, KickBassSpaceAudioProcessor::historySize>& data,
                                          int writePos, juce::Colour colour, float alpha, bool filled)
{
    constexpr int N = KickBassSpaceAudioProcessor::historySize;
    juce::Path path;

    for (int i = 0; i < N; ++i)
    {
        auto idx = (size_t) ((writePos + i) % N);
        auto value = juce::jlimit (0.0f, 1.2f, data[idx]);

        auto x = area.getX() + (float) i / (float) (N - 1) * area.getWidth();
        auto y = area.getBottom() - value * area.getHeight() * 0.85f;

        if (i == 0) path.startNewSubPath (x, y);
        else path.lineTo (x, y);
    }

    g.setColour (colour.withAlpha (alpha));
    g.strokePath (path, juce::PathStrokeType (filled ? 1.5f : 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    auto fillPath = path;
    fillPath.lineTo (area.getRight(), area.getBottom());
    fillPath.lineTo (area.getX(), area.getBottom());
    fillPath.closeSubPath();
    g.setColour (colour.withAlpha (alpha * (filled ? 0.35f : 0.15f)));
    g.fillPath (fillPath);
}

void EnvelopeDisplay::drawLegend (juce::Graphics& g, juce::Rectangle<float> legendArea)
{
    g.setFont (12.0f);

    g.setColour (SpaceColours::kick);
    g.fillEllipse (legendArea.getX(), legendArea.getY() + 4.0f, 8.0f, 8.0f);
    g.drawText ("Kick", legendArea.getX() + 14.0f, legendArea.getY(), 50.0f, 16.0f, juce::Justification::left);

    g.setColour (SpaceColours::bass);
    g.fillEllipse (legendArea.getX() + 70.0f, legendArea.getY() + 4.0f, 8.0f, 8.0f);
    g.drawText ("Bass", legendArea.getX() + 84.0f, legendArea.getY(), 50.0f, 16.0f, juce::Justification::left);

    g.setColour (SpaceColours::conflict);
    g.fillEllipse (legendArea.getX() + 140.0f, legendArea.getY() + 4.0f, 8.0f, 8.0f);
    g.drawText ("Conflicto", legendArea.getX() + 154.0f, legendArea.getY(), 80.0f, 16.0f, juce::Justification::left);
}

void EnvelopeDisplay::drawSidechainWarning (juce::Graphics& g, juce::Rectangle<float> area)
{
    auto warningBounds = area.withSizeKeepingCentre (juce::jmin (420.0f, area.getWidth() - 20.0f), 40.0f);

    g.setColour (SpaceColours::panel.withAlpha (0.92f));
    g.fillRoundedRectangle (warningBounds, 6.0f);
    g.setColour (SpaceColours::warning);
    g.drawRoundedRectangle (warningBounds, 6.0f, 1.5f);

    g.setFont (13.0f);
    g.drawText ("No se detecta senal en el Sidechain -- conecta la pista del bajo",
                warningBounds.reduced (10.0f), juce::Justification::centred);
}

void EnvelopeDisplay::drawBandBreakdown (juce::Graphics& g, juce::Rectangle<float> area)
{
    const auto& defs = MultibandEnergyAnalyzer::getBandDefs();
    constexpr int N = KickBassSpaceAudioProcessor::historySize;

    g.setColour (SpaceColours::textDim);
    g.setFont (10.0f);
    g.drawText ("Conflicto por banda (ahora):", area.removeFromTop (14.0f), juce::Justification::left);

    auto labelsArea = area.removeFromBottom (14.0f);
    auto barsArea = area;
    auto barWidth = barsArea.getWidth() / (float) MultibandEnergyAnalyzer::numBands;

    auto lastIdx = (size_t) ((snapshot.writePos - 1 + N) % N);
    const auto& currentBands = snapshot.conflictPerBand[lastIdx];

    for (int b = 0; b < MultibandEnergyAnalyzer::numBands; ++b)
    {
        auto bar = barsArea.removeFromLeft (barWidth).reduced (4.0f, 0.0f);
        auto labelSlot = labelsArea.removeFromLeft (barWidth);
        auto level = juce::jlimit (0.0f, 1.0f, currentBands[(size_t) b] * 3.0f);

        g.setColour (SpaceColours::gridLine);
        g.fillRoundedRectangle (bar, 3.0f);

        auto filledBar = bar.withTop (bar.getBottom() - level * bar.getHeight());
        g.setColour (SpaceColours::conflict.withAlpha (0.85f));
        g.fillRoundedRectangle (filledBar, 3.0f);

        g.setColour (SpaceColours::textDim);
        g.setFont (9.0f);
        auto label = juce::String ((int) defs[(size_t) b].lowHz) + "-" + juce::String ((int) defs[(size_t) b].highHz) + "Hz";
        g.drawText (label, labelSlot, juce::Justification::centred);
    }
}
