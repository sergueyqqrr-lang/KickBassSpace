#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "DSP/EnvelopeFollower.h"

/**
    ETAPA 1 del plan Kick/Bass Space:
    - Bus de sidechain para el bajo (entrada separada del kick).
    - Detectores de envolvente para kick y bass.
    - El audio pasa SIN MODIFICAR -- esta etapa es puramente de análisis,
      para poder validar visualmente que la detección funciona antes de
      procesar nada (evita repetir el problema de construir "a ciegas").
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

    // --- Datos para la GUI ---
    // Historial reciente de envolvente (kick y bass) para dibujar en pantalla.
    // Tamaño fijo: ~4 segundos de historial a ~100 puntos/seg.
    static constexpr int historySize = 400;
    static constexpr float historyRateHz = 100.0f;

    struct EnvelopeSnapshot
    {
        std::array<float, historySize> kick {};
        std::array<float, historySize> bass {};
        int writePos = 0;
        bool sidechainConnected = false;
    };

    // Lectura segura para la GUI (copia atomica simple protegida por flag)
    void getEnvelopeSnapshot (EnvelopeSnapshot& outSnapshot) const;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    EnvelopeFollower kickEnvelope, bassEnvelope;

    EnvelopeSnapshot snapshot;
    mutable juce::SpinLock snapshotLock;

    int samplesUntilNextHistoryPoint = 0;
    int samplesPerHistoryPoint = 441; // se recalcula en prepareToPlay

    double currentSampleRate = 44100.0;

    // Para detectar si realmente hay señal en el sidechain (y avisar al usuario
    // si no está conectado, en vez de mostrar una línea plana confusa)
    float sidechainActivitySmoothed = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KickBassSpaceAudioProcessor)
};
