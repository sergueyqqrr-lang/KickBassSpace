#include "PluginEditor.h"

KickBassSpaceAudioProcessorEditor::KickBassSpaceAudioProcessorEditor (KickBassSpaceAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), envelopeDisplay (p)
{
    setLookAndFeel (&spaceLookAndFeel);

    titleLabel.setFont (juce::Font (18.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, SpaceColours::kick);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel);

    stageLabel.setFont (juce::Font (11.0f));
    stageLabel.setColour (juce::Label::textColourId, SpaceColours::textDim);
    stageLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (stageLabel);

    addAndMakeVisible (envelopeDisplay);

    for (auto* s : { &attackSlider, &releaseSlider })
    {
        s->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
        addAndMakeVisible (s);
    }

    for (auto* l : { &attackLabel, &releaseLabel })
    {
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::Font (11.0f));
        l->setColour (juce::Label::textColourId, SpaceColours::textDim);
        addAndMakeVisible (l);
    }

    using APVTS = juce::AudioProcessorValueTreeState;
    attackAttach = std::make_unique<APVTS::SliderAttachment> (p.apvts, "attackMs", attackSlider);
    releaseAttach = std::make_unique<APVTS::SliderAttachment> (p.apvts, "releaseMs", releaseSlider);

    setResizable (true, true);
    setResizeLimits (700, 420, 1400, 900);
    setSize (900, 560);
}

KickBassSpaceAudioProcessorEditor::~KickBassSpaceAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void KickBassSpaceAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (SpaceColours::background);
}

void KickBassSpaceAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    auto header = area.removeFromTop (48);
    auto titleArea = header.removeFromLeft (400).reduced (12, 4);
    titleLabel.setBounds (titleArea.removeFromTop (22));
    stageLabel.setBounds (titleArea);

    auto controlsArea = header.reduced (8, 4);
    auto attackArea = controlsArea.removeFromLeft (70);
    attackLabel.setBounds (attackArea.removeFromTop (14));
    attackSlider.setBounds (attackArea);

    auto releaseArea = controlsArea.removeFromLeft (70);
    releaseLabel.setBounds (releaseArea.removeFromTop (14));
    releaseSlider.setBounds (releaseArea);

    envelopeDisplay.setBounds (area.reduced (4));
}
