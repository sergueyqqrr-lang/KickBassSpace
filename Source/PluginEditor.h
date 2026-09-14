#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "GUI/LookAndFeel.h"
#include "GUI/EnvelopeDisplay.h"

class KickBassSpaceAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit KickBassSpaceAudioProcessorEditor (KickBassSpaceAudioProcessor&);
    ~KickBassSpaceAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    KickBassSpaceAudioProcessor& processorRef;
    SpaceLookAndFeel spaceLookAndFeel;

    juce::Label titleLabel { {}, "KICK BASS SPACE" };
    juce::Label stageLabel { {}, "Etapa 1: Analisis (el audio no se modifica todavia)" };

    EnvelopeDisplay envelopeDisplay;

    juce::Slider attackSlider, releaseSlider;
    juce::Label attackLabel { {}, "Attack" }, releaseLabel { {}, "Release" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttach, releaseAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KickBassSpaceAudioProcessorEditor)
};
