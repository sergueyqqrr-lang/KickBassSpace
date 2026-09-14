#pragma once
#include <juce_dsp/juce_dsp.h>

/**
    Sigue la envolvente de amplitud de una señal en el tiempo, con tiempos de
    ataque y release independientes (detector clásico de un polo).

    Este es el bloque de análisis más básico del sistema Kick/Bass Space:
    convierte una señal de audio en una curva de energía en el tiempo, que
    luego se usará para detectar solapamiento entre kick y bass (Etapa 2).

    En esta Etapa 1 SOLO se analiza -- no se modifica el audio.
*/
class EnvelopeFollower
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        updateCoefficients();
    }

    void reset()
    {
        envelope = 0.0f;
    }

    void setAttackTimeMs (float ms)
    {
        attackMs = juce::jmax (0.01f, ms);
        updateCoefficients();
    }

    void setReleaseTimeMs (float ms)
    {
        releaseMs = juce::jmax (0.01f, ms);
        updateCoefficients();
    }

    // Procesa una muestra y devuelve el valor actual de la envolvente (0..~1+)
    float processSample (float inputSample)
    {
        auto rectified = std::abs (inputSample);
        auto coeff = (rectified > envelope) ? attackCoeff : releaseCoeff;
        envelope += coeff * (rectified - envelope);
        return envelope;
    }

    float getCurrentValue() const noexcept { return envelope; }

private:
    void updateCoefficients()
    {
        // Coeficientes de un filtro de un polo clasico para detectores de envolvente
        attackCoeff  = 1.0f - std::exp (-1.0f / (0.001f * attackMs  * (float) sr));
        releaseCoeff = 1.0f - std::exp (-1.0f / (0.001f * releaseMs * (float) sr));
    }

    double sr = 44100.0;
    float attackMs = 3.0f;    // rapido: para no perder el transitorio del kick
    float releaseMs = 150.0f; // moderado: suficiente para "ver" la cola
    float attackCoeff = 0.5f;
    float releaseCoeff = 0.05f;
    float envelope = 0.0f;
};
