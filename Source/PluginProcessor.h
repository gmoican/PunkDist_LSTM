/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define DEFAULT_DRIVE 15.0f
#define DEFAULT_LEVEL 0.0f
#define DEFAULT_TONE1 1350.0f
#define DEFAULT_TONE2 5.0f

//==============================================================================
/**
*/
class PunkDistAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PunkDistAudioProcessor();
    ~PunkDistAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //=============== MY STUFF =====================================================
    AudioProcessorValueTreeState state;

private:
    AudioProcessorValueTreeState::ParameterLayout createParams();
    using FilterBand = dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>>;
    using Gain = dsp::Gain<float>;
    using Shaper = dsp::WaveShaper<float>;
    using Bias = dsp::Bias<float>;
    using OverSampling = dsp::Oversampling<float>;
    using Limiter = dsp::Compressor<float>;
    using Filter = dsp::LinkwitzRileyFilter<float>;
        
    // Modifiable parameters
    dsp::ProcessorChain<Gain, Bias, Shaper, Bias, FilterBand, Limiter> drive;
    dsp::ProcessorChain<FilterBand, FilterBand> eq;
    Gain outputLevel;
    bool mode;
    bool on;
    
    // Other things
    OverSampling driveOV { 2, 2, OverSampling::filterHalfBandPolyphaseIIR, true, false };

    // Updaters
    void updateOnOff();
    void updateDrive();
    void updateLevel();
    void updateTone();
    void updateMode();
    void updateState();
    
    // Drive functions
    static float asymptoticClipping(float sample);
    static float arcTanClipping(float sample);
    // static float newTanClipping(float sample);
    static float tanhClipping(float sample);
    static float asymetricClipping(float sample);
    static float softClipping(float sample);
        
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PunkDistAudioProcessor)
};
