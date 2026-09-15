#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "DSP/EnvelopeFollower.h"
#include "DSP/MultibandEnergyAnalyzer.h"

/**
    ETAPA 2 del plan Kick/Bass Space:
    - Etapa 1 (ya lista): bus de sidechain + envolventes generales de kick/bass.
    - Etapa 2 (esta): divide ambas señales en 4 bandas de frecuencia relevantes
      y calcula, banda por banda, dónde EXISTE solapamiento real (cuando
      ambas señales tienen energía significativa al mismo tiempo).
    - El audio sigue pasando SIN MODIFICAR -- todavía es solo análisis.
*/
class KickBassSpaceAudioProcessor : public juce::AudioProcessor
{
public:
    KickBassSpaceAudioProcessor();
    ~KickBassSpaceAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Kick Bass Space"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    static constexpr int historySize = 400;
    static constexpr float historyRateHz = 100.0f;
    static constexpr int numBands = MultibandEnergyAnalyzer::numBands;

    struct EnvelopeSnapshot
    {
        std::array<float, historySize> kick {};
        std::array<float, historySize> bass {};
        std::array<float, historySize> conflict {};          // conflicto total (max entre bandas)
        std::array<std::array<float, numBands>, historySize> conflictPerBand {};
        int writePos = 0;
        bool sidechainConnected = false;
    };

    void getEnvelopeSnapshot (EnvelopeSnapshot& outSnapshot) const;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    MultibandEnergyAnalyzer kickAnalyzer, bassAnalyzer;
    EnvelopeFollower kickOverallEnvelope, bassOverallEnvelope;

    EnvelopeSnapshot snapshot;
    mutable juce::SpinLock snapshotLock;

    int samplesUntilNextHistoryPoint = 0;
    int samplesPerHistoryPoint = 441;

    double currentSampleRate = 44100.0;

    float sidechainActivitySmoothed = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KickBassSpaceAudioProcessor)
};
