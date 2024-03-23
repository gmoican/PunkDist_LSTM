/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PunkDistEditor::PunkDistEditor (PunkDistAudioProcessor& p) : AudioProcessorEditor(&p), audioProcessor (p)
{
    juce::ignoreUnused(audioProcessor);

    // ================= PARAMETERS ====================
    setSliderComponent(driveKnob, driveKnobAttachment, "DRIVE", "Rot");
    setSliderComponent(levelKnob, levelKnobAttachment, "LEVEL", "Rot");

    setSliderComponent(tone1Knob, tone1KnobAttachment, "TONE1", "Rot");
    setSliderComponent(tone2Knob, tone2KnobAttachment, "TONE2", "Rot");
    
    setToggleComponent(onToggle, onToggleAttachment, "ONOFF");

    // ================= ASSETS =======================
    background = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
    //
    lightOff = juce::ImageCache::getFromMemory(BinaryData::lightOff_png, BinaryData::lightOff_pngSize);
    //
    knobImage = juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
    
    setSize (180, 320);
}

PunkDistEditor::~PunkDistEditor()
{
}

//==============================================================================
void PunkDistEditor::paint (juce::Graphics& g)
{
    g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
    
    // =========== On/Off state ====================
    if (!onToggle.getToggleState()) {
        juce::AffineTransform t;
        t = t.scaled(0.485f);
        t = t.translated(75.5, 163.5);
        g.drawImageTransformed(lightOff, t);
    }
    
    // ========== Parameter knobs angle in radians ==================
    // Drive knob mapping function: y = (x-A)/(B-A) * (D-C) + C
    // x = {A, B} = {0.0, 30.0}
    // y = {C, D} = {-150, 150} * PI / 180
    float driveRadians = (driveKnob.getValue() / 30.0f * 300.0f - 150.0f) * DEG2RADS;
    
    // Output knob mapping function: y = (x-A)/(B-A) * (D-C) + C
    // x = {A, B} = {-30.0, 30.0}
    // y = {C, D} = {-150, 150} * PI / 180
    float levelRadians = ((levelKnob.getValue() + 30.0f) / (60.0f) * 300.0f - 150.0f) * DEG2RADS;
    
    // Tone1 knob mapping function: y = (x-A)/(B-A) * (D-C) + C
    // x = {A, B} = {0.0, 10.0}
    // y = {C, D} = {-150, 150} * PI / 180
    float tone1Radians = (tone1Knob.getValue() / 10.0f * 300.0f - 150.0f) * DEG2RADS;
    
    // Tone2 knob mapping function: y = (x-A)/(B-A) * (D-C) + C
    // x = {A, B} = {0.0, 10.0}
    // y = {C, D} = {-150, 150} * PI / 180
    float tone2Radians = (tone2Knob.getValue() / 10.0f * 300.0f - 150.0f) * DEG2RADS;
    
    // ========== Draw parameter knobs ==================
    g.drawImageTransformed(knobImage, knobRotation(driveRadians, 23.5, 23));
    g.drawImageTransformed(knobImage, knobRotation(levelRadians, 112.5, 23));
    g.drawImageTransformed(knobImage, knobRotation(tone1Radians, 23.5, 91));
    g.drawImageTransformed(knobImage, knobRotation(tone2Radians, 112.5, 91));
}

void PunkDistEditor::resized()
{
    // Upper row
    driveKnob.setBounds(24, 23, 46, 46);
    levelKnob.setBounds(113, 23, 46, 46);
    
    // Bottom row
    tone1Knob.setBounds(24, 91, 46, 46);
    tone2Knob.setBounds(113, 91, 46, 46);
    
    // OnOff
    onToggle.setBounds(65, 240, 50, 50);
}

void PunkDistEditor::setSliderComponent(juce::Slider &slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> &sliderAttachment, juce::String paramName, juce::String style){
    sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, paramName, slider);
    if (style == "Lin")
    {
        slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    } else
    {
        slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    }
    slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(slider);
    slider.setAlpha(0);
}

void PunkDistEditor::setToggleComponent(juce::ToggleButton& button, std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& buttonAttachment, juce::String paramName){
    buttonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.state, paramName, button);
    addAndMakeVisible(button);
    button.setAlpha(0);
}

juce::AffineTransform PunkDistEditor::knobRotation(float radians, float posX, float posY){
    juce::AffineTransform t;
    t = t.rotated(radians, 46.0f, 46.0f);
    t = t.scaled(0.48f);
    t = t.translated(posX, posY);
    return t;
}
