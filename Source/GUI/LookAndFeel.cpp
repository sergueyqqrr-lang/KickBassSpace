#include "LookAndFeel.h"

SpaceLookAndFeel::SpaceLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, SpaceColours::background);
    setColour (juce::Slider::thumbColourId, SpaceColours::kick);
    setColour (juce::Slider::trackColourId, SpaceColours::kick);
    setColour (juce::Slider::backgroundColourId, SpaceColours::gridLine);
    setColour (juce::Label::textColourId, SpaceColours::text);
    setColour (juce::TextButton::buttonColourId, SpaceColours::panel);
    setColour (juce::TextButton::textColourOffId, SpaceColours::textDim);
    setColour (juce::TextButton::textColourOnId, SpaceColours::kick);
    setColour (juce::ToggleButton::textColourId, SpaceColours::text);
}

void SpaceLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (4.0f);
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centre = bounds.getCentre();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = juce::jmax (2.0f, radius * 0.11f);
    auto arcRadius = radius - lineW * 0.5f;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                  rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (SpaceColours::gridLine);
    g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (slider.isEnabled() ? SpaceColours::kick : SpaceColours::textDim);
    g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    auto knobRadius = radius * 0.62f;
    g.setColour (SpaceColours::panel);
    g.fillEllipse (centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
    g.setColour (SpaceColours::gridLineMain);
    g.drawEllipse (centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);

    juce::Path pointer;
    auto pointerLength = knobRadius * 0.75f;
    pointer.addRectangle (-1.5f, -knobRadius, 3.0f, pointerLength);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre));
    g.setColour (SpaceColours::kick);
    g.fillPath (pointer);
}

juce::Font SpaceLookAndFeel::getLabelFont (juce::Label&) { return juce::Font (13.0f); }
