#pragma once
#include <juce_dsp/juce_dsp.h>
#include "EnvelopeFollower.h"

/**
    Divide una señal en varias bandas de frecuencia relevantes para el
    conflicto kick/bass, y sigue la envolvente de energía de cada banda
    por separado.

    Esto es lo que permite responder "¿EN QUÉ FRECUENCIA están compitiendo
    ahora mismo?", no solo "¿hay energía en general?".
*/
class MultibandEnergyAnalyzer
{
public:
    static constexpr int numBands = 4;

    struct BandDef { float lowHz, highHz; };

    static const std::array<BandDef, numBands>& getBandDefs()
    {
        static const std::array<BandDef, numBands> defs {{
            { 30.0f,  70.0f },   // sub profundo
            { 70.0f,  150.0f },  // fundamental tipico de kick/bass
            { 150.0f, 300.0f },  // armonicos bajos / "boxiness"
            { 300.0f, 600.0f }   // armonicos superiores del bajo
        }};
        return defs;
    }

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        const auto& defs = getBandDefs();

        for (int b = 0; b < numBands; ++b)
        {
            highPass[(size_t) b].coefficients =
                juce::dsp::IIR::Coefficients<float>::makeHighPass (sr, defs[(size_t) b].lowHz, 0.707f);
            lowPass[(size_t) b].coefficients =
                juce::dsp::IIR::Coefficients<float>::makeLowPass (sr, defs[(size_t) b].highHz, 0.707f);

            envelopes[(size_t) b].prepare (sr);
            envelopes[(size_t) b].setAttackTimeMs (3.0f);
            envelopes[(size_t) b].setReleaseTimeMs (150.0f);
        }
    }

    void reset()
    {
        for (int b = 0; b < numBands; ++b)
        {
            highPass[(size_t) b].reset();
            lowPass[(size_t) b].reset();
            envelopes[(size_t) b].reset();
        }
    }

    void setAttackReleaseMs (float attackMs, float releaseMs)
    {
        for (auto& e : envelopes)
        {
            e.setAttackTimeMs (attackMs);
            e.setReleaseTimeMs (releaseMs);
        }
    }

    std::array<float, numBands> processSample (float inputSample)
    {
        std::array<float, numBands> result {};

        for (int b = 0; b < numBands; ++b)
        {
            auto filtered = highPass[(size_t) b].processSample (inputSample);
            filtered = lowPass[(size_t) b].processSample (filtered);
            result[(size_t) b] = envelopes[(size_t) b].processSample (filtered);
        }

        return result;
    }

private:
    std::array<juce::dsp::IIR::Filter<float>, numBands> highPass, lowPass;
    std::array<EnvelopeFollower, numBands> envelopes;
    double sr = 44100.0;
};
