/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PunkDistComponent::PunkDistComponent (PunkDistAudioProcessor& p) : audioProcessor (p)
{
    // ================= PARAMETERS ====================
    setSliderComponent(driveKnob, driveKnobAttachment, "DRIVE", "Rot");
    setSliderComponent(levelKnob, levelKnobAttachment, "LEVEL", "Rot");

    setSliderComponent(tone1Knob, tone1KnobAttachment, "TONE1", "Rot");
    setSliderComponent(tone2Knob, tone2KnobAttachment, "TONE2", "Rot");
    
    setSliderComponent(modeSwitch, modeSwitchAttachment, "MODE", "Lin");

    setToggleComponent(onToggle, onToggleAttachment, "ONOFF");

    // ================= ASSETS =======================
    backgroundOn = ImageCache::getFromMemory(BinaryData::backgroundOn_png, BinaryData::backgroundOn_pngSize);
    backgroundOff = ImageCache::getFromMemory(BinaryData::backgroundOff_png, BinaryData::backgroundOff_pngSize);
    //
    switchTop = ImageCache::getFromMemory(BinaryData::switchTop_png, BinaryData::switchTop_pngSize);
    //
    knobImage = ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
    
    // Size is set in the Wrapper
    // setSize (250, 450);
}

PunkDistComponent::~PunkDistComponent()
{
}

//==============================================================================
void PunkDistComponent::paint (Graphics& g)
{
    // =========== On/Off state ====================
    if (onToggle.getToggleState())
    {
        g.drawImageWithin(backgroundOn, 0, 0, getWidth(), getHeight(), RectanglePlacement::stretchToFit);
    } else
    {
        g.drawImageWithin(backgroundOff, 0, 0, getWidth(), getHeight(), RectanglePlacement::stretchToFit);
    }
    
    //  =========== Switch state ====================
    if(modeSwitch.getValue())
        g.drawImageAt(switchTop, 24, 240);
    else
        g.drawImageAt(switchTop, 24, 265);
    
    // ========== Parameter knobs angle in radians ==================
    // Drive knob mapping function: y = (x-A)/(B-A) * (D-C) + C
    // x = {A, B} = {0.0, 45.0}
    // y = {C, D} = {-150, 150} * PI / 180
    float driveRadians = (driveKnob.getValue() / 45.0f * 300.0f - 150.0f) * DEG2RADS;
    
    // Output knob mapping function: y = (x-A)/(B-A) * (D-C) + C
    // x = {A, B} = {-30.0, 30.0}
    // y = {C, D} = {-150, 150} * PI / 180
    float levelRadians = ((levelKnob.getValue() + 30.0f) / (60.0f) * 300.0f - 150.0f) * DEG2RADS;
    
    // Tone1 knob mapping function: y = (x-A)/(B-A) * (D-C) + C
    // x = {A, B} = {200.0, 2500.0}
    // y = {C, D} = {-150, 150} * PI / 180
    float tone1Radians = ((tone1Knob.getValue() - 200) / 2300.0f * 300.0f - 150.0f) * DEG2RADS;
    
    // Tone2 knob mapping function: y = (x-A)/(B-A) * (D-C) + C
    // x = {A, B} = {0.0, 10.0}
    // y = {C, D} = {-150, 150} * PI / 180
    float tone2Radians = (tone2Knob.getValue() / 10.0f * 300.0f - 150.0f) * DEG2RADS;
    
    // ========== Draw parameter knobs ==================
    g.drawImageTransformed(knobImage, knobRotation(driveRadians, 36, 23));
    g.drawImageTransformed(knobImage, knobRotation(levelRadians, 145, 23));
    g.drawImageTransformed(knobImage, knobRotation(tone1Radians, 36, 130));
    g.drawImageTransformed(knobImage, knobRotation(tone2Radians, 145, 130));
}

void PunkDistComponent::resized()
{
    // Upper row
    driveKnob.setBounds(36, 23, 80, 80);
    levelKnob.setBounds(145, 23, 80, 80);
    
    // Bottom row
    tone1Knob.setBounds(36, 130, 80, 80);
    tone2Knob.setBounds(145, 130, 80, 80);

    // Switch
    modeSwitch.setBounds(23, 240, 40, 40);
    
    // OnOff
    onToggle.setBounds(112, 340, 80, 80);
}

void PunkDistComponent::setSliderComponent(Slider &slider, std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> &sliderAttachment, String paramName, String style){
    sliderAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, paramName, slider);
    if (style == "Lin")
    {
        slider.setSliderStyle(Slider::SliderStyle::LinearVertical);
    } else
    {
        slider.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    }
    slider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(slider);
    slider.setAlpha(0);
}

void PunkDistComponent::setToggleComponent(ToggleButton& button, std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment>& buttonAttachment, String paramName){
    buttonAttachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.state, paramName, button);
    addAndMakeVisible(button);
    button.setAlpha(0);
}

AffineTransform PunkDistComponent::knobRotation(float radians, float posX, float posY){
    AffineTransform t;
    t = t.rotated(radians, 34.0f, 33.5f);
    t = t.translated(posX, posY);
    return t;
}

// ============= ROOT WRAPPER ===========================
WrappedRasterAudioProcessorEditor::WrappedRasterAudioProcessorEditor(PunkDistAudioProcessor& p) :
    AudioProcessorEditor(p),
    rasterComponent(p)
{
    addAndMakeVisible(&rasterComponent);
    
    if (auto* constrainer = getConstrainer())
    {
        constrainer -> setFixedAspectRatio(static_cast<double>(originalWidth) / static_cast<double>(originalHeigh));
        constrainer -> setSizeLimits(originalWidth / 2, originalHeigh / 2, originalWidth, originalHeigh);
    }
    
    setResizable(true, true);
    setSize(originalWidth, originalHeigh);
}

void WrappedRasterAudioProcessorEditor::resized()
{
    const auto scaleFactor = static_cast<float>(getWidth()) / originalWidth;
    rasterComponent.setTransform(AffineTransform::scale(scaleFactor));
    rasterComponent.setBounds(0, 0, originalWidth, originalHeigh);
}
