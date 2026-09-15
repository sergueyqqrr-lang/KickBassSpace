#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"

class EnvelopeDisplay : public juce::Component, private juce::Timer
{
public:
    explicit EnvelopeDisplay (KickBassSpaceAudioProcessor& proc);
    ~EnvelopeDisplay() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawGrid (juce::Graphics&, juce::Rectangle<float> area);
    void drawEnvelopeTrace (juce::Graphics&, juce::Rectangle<float> area,
                             const std::array<float, KickBassSpaceAudioProcessor::historySize>& data,
                             int writePos, juce::Colour colour, float alpha, bool filled);
    void drawSidechainWarning (juce::Graphics&, juce::Rectangle<float> area);
    void drawBandBreakdown (juce::Graphics&, juce::Rectangle<float> area);
    void drawLegend (juce::Graphics&, juce::Rectangle<float> area);

    KickBassSpaceAudioProcessor& processor;
    KickBassSpaceAudioProcessor::EnvelopeSnapshot snapshot;

    juce::Rectangle<float> mainPlotArea, bandBreakdownArea;
};
