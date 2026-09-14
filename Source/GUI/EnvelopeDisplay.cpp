#include "EnvelopeDisplay.h"
#include "LookAndFeel.h"

EnvelopeDisplay::EnvelopeDisplay (KickBassSpaceAudioProcessor& proc) : processor (proc)
{
    startTimerHz (30);
}

EnvelopeDisplay::~EnvelopeDisplay() { stopTimer(); }

void EnvelopeDisplay::resized()
{
    plotArea = getLocalBounds().toFloat().reduced (8.0f);
}

void EnvelopeDisplay::timerCallback()
{
    processor.getEnvelopeSnapshot (snapshot);
    repaint();
}

void EnvelopeDisplay::paint (juce::Graphics& g)
{
    g.fillAll (SpaceColours::background);
    drawGrid (g);

    // Bass primero (fondo), Kick encima (para que el golpe destaque)
    drawEnvelopeTrace (g, snapshot.bass, snapshot.writePos, SpaceColours::bass, 0.85f);
    drawEnvelopeTrace (g, snapshot.kick, snapshot.writePos, SpaceColours::kick, 0.95f);

    // Leyenda
    auto legendArea = plotArea.removeFromTop (20.0f);
    g.setFont (12.0f);

    g.setColour (SpaceColours::kick);
    g.fillEllipse (legendArea.getX(), legendArea.getY() + 4.0f, 8.0f, 8.0f);
    g.drawText ("Kick", legendArea.getX() + 14.0f, legendArea.getY(), 60.0f, 16.0f, juce::Justification::left);

    g.setColour (SpaceColours::bass);
    g.fillEllipse (legendArea.getX() + 80.0f, legendArea.getY() + 4.0f, 8.0f, 8.0f);
    g.drawText ("Bass (Sidechain)", legendArea.getX() + 94.0f, legendArea.getY(), 140.0f, 16.0f, juce::Justification::left);

    if (! snapshot.sidechainConnected)
        drawSidechainWarning (g);
}

void EnvelopeDisplay::drawGrid (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat().reduced (8.0f);

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

void EnvelopeDisplay::drawEnvelopeTrace (juce::Graphics& g,
                                          const std::array<float, KickBassSpaceAudioProcessor::historySize>& data,
                                          int writePos, juce::Colour colour, float alpha)
{
    auto area = getLocalBounds().toFloat().reduced (8.0f);
    area.removeFromTop (20.0f); // deja espacio para la leyenda

    constexpr int N = KickBassSpaceAudioProcessor::historySize;
    juce::Path path;

    for (int i = 0; i < N; ++i)
    {
        // El punto mas antiguo es writePos, el mas reciente es writePos-1 (circular)
        auto idx = (size_t) ((writePos + i) % N);
        auto value = juce::jlimit (0.0f, 1.2f, data[idx]);

        auto x = area.getX() + (float) i / (float) (N - 1) * area.getWidth();
        auto y = area.getBottom() - value * area.getHeight() * 0.85f;

        if (i == 0) path.startNewSubPath (x, y);
        else path.lineTo (x, y);
    }

    g.setColour (colour.withAlpha (alpha));
    g.strokePath (path, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    auto fillPath = path;
    fillPath.lineTo (area.getRight(), area.getBottom());
    fillPath.lineTo (area.getX(), area.getBottom());
    fillPath.closeSubPath();
    g.setColour (colour.withAlpha (alpha * 0.15f));
    g.fillPath (fillPath);
}

void EnvelopeDisplay::drawSidechainWarning (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat().reduced (8.0f);
    auto warningBounds = area.withSizeKeepingCentre (juce::jmin (420.0f, area.getWidth() - 20.0f), 40.0f);

    g.setColour (SpaceColours::panel.withAlpha (0.92f));
    g.fillRoundedRectangle (warningBounds, 6.0f);
    g.setColour (SpaceColours::warning);
    g.drawRoundedRectangle (warningBounds, 6.0f, 1.5f);

    g.setFont (13.0f);
    g.drawText ("No se detecta señal en el Sidechain -- conecta la pista del bajo",
                warningBounds.reduced (10.0f), juce::Justification::centred);
}
