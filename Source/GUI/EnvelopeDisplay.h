#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"

/**
    Dibuja las envolventes de Kick y Bass superpuestas en el tiempo.
    Esta es la validación visual de la Etapa 1: si esto se ve bien
    (el kick reacciona a los golpes, el bass reacciona al sidechain),
    la base de análisis funciona y se puede pasar a la Etapa 2
    (detectar solapamiento entre ambas curvas).
*/
class EnvelopeDisplay : public juce::Component, private juce::Timer
{
public:
    explicit EnvelopeDisplay (KickBassSpaceAudioProcessor& proc);
    ~EnvelopeDisplay() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawGrid (juce::Graphics&);
    void drawEnvelopeTrace (juce::Graphics&, const std::array<float, KickBassSpaceAudioProcessor::historySize>& data,
                             int writePos, juce::Colour colour, float alpha);
    void drawSidechainWarning (juce::Graphics&);

    KickBassSpaceAudioProcessor& processor;
    KickBassSpaceAudioProcessor::EnvelopeSnapshot snapshot;

    juce::Rectangle<float> plotArea;
};
