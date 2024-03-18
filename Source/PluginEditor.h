/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#define DEG2RADS 0.0174533f

//==============================================================================
/**
*/
class PunkDistEditor : public juce::AudioProcessorEditor
{
public:
    PunkDistEditor (PunkDistAudioProcessor&);
    ~PunkDistEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    //=================== PARAMETER MANIPULATION ===================================
    void setSliderComponent(juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& sliderAttachment, juce::String paramName, juce::String style);
    void setToggleComponent(juce::ToggleButton& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& sliderAttachment, juce::String paramName);
    juce::AffineTransform knobRotation(float radians, float posX, float posY);

private:
    // Parameters
    juce::Slider driveKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveKnobAttachment;
    
    juce::Slider levelKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> levelKnobAttachment;
    
    juce::Slider tone1Knob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tone1KnobAttachment;
    
    juce::Slider tone2Knob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tone2KnobAttachment;
    
    juce::ToggleButton onToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onToggleAttachment;
    
    // Assets - Background, knobs and switch
    juce::Image background;
    
    juce::Image lightOff;
        
    juce::Image knobImage;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PunkDistAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PunkDistEditor)
};
