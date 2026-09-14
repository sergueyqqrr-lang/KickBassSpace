#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace SpaceColours
{
    const juce::Colour background   { 0xff14181c };
    const juce::Colour panel        { 0xff1c2126 };
    const juce::Colour gridLine     { 0xff2c3238 };
    const juce::Colour gridLineMain { 0xff3d444c };
    const juce::Colour text         { 0xffd8dade };
    const juce::Colour textDim      { 0xff8a8f99 };
    const juce::Colour kick         { 0xff4dd6c2 }; // cian/turquesa: el kick
    const juce::Colour bass         { 0xffb98cff }; // violeta: el bajo
    const juce::Colour conflict     { 0xffff5c5c }; // rojo: zona de conflicto
    const juce::Colour warning      { 0xffffb84d };
}

class SpaceLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SpaceLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle,
                            float rotaryEndAngle, juce::Slider&) override;

    juce::Font getLabelFont (juce::Label&) override;
};
