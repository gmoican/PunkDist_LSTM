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
class PunkDistComponent : public Component
{
public:
    PunkDistComponent (PunkDistAudioProcessor&);
    ~PunkDistComponent() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    //=================== PARAMETER MANIPULATION ===================================
    void setSliderComponent(Slider& slider, std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>& sliderAttachment, String paramName, String style);
    void setToggleComponent(ToggleButton& slider, std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment>& sliderAttachment, String paramName);
    AffineTransform knobRotation(float radians, float posX, float posY);

private:
    // Parameters
    Slider driveKnob;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> driveKnobAttachment;
    
    Slider levelKnob;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> levelKnobAttachment;
    
    Slider tone1Knob;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> tone1KnobAttachment;
    
    Slider tone2Knob;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> tone2KnobAttachment;
    
    Slider modeSwitch;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> modeSwitchAttachment;
    
    ToggleButton onToggle;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> onToggleAttachment;
    
    // Assets - Background, knobs and switch
    Image backgroundOn;
    Image backgroundOff;
    
    Image switchTop;
    
    Image knobDriveImage;
    Image knobLevelImage;
    Image knobTone1Image;
    Image knobTone2Image;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PunkDistAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PunkDistComponent)
};

// ============= ROOT WRAPPER ===========================
class WrappedRasterAudioProcessorEditor : public AudioProcessorEditor
{
public:
    WrappedRasterAudioProcessorEditor(PunkDistAudioProcessor&);
    void resized() override;
    
private:
    static constexpr int originalWidth{250};
    static constexpr int originalHeigh{450};
    PunkDistComponent rasterComponent;
};
