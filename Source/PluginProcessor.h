#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#if (MSVC)
#include "ipps.h"
#endif

#define DEFAULT_DRIVE 15.0f
#define DEFAULT_LEVEL 0.0f
#define DEFAULT_TONE1 5.0f
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
    juce::AudioProcessorValueTreeState state;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    using FilterBand = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    using Gain = juce::dsp::Gain<float>;
    using Shaper = juce::dsp::WaveShaper<float>;
    using OverSampling = juce::dsp::Oversampling<float>;
    using Limiter = juce::dsp::Compressor<float>;
    using Filter = juce::dsp::LinkwitzRileyFilter<float>;
        
    // Modifiable parameters
    Gain inputGain;
    juce::dsp::ProcessorChain<Shaper, FilterBand, Limiter> drive;
    juce::dsp::ProcessorChain<FilterBand, FilterBand, FilterBand, FilterBand, FilterBand> eq;
    Gain outputLevel;
    bool on;
    
    // Other things
    OverSampling driveOV { 2, 2, OverSampling::filterHalfBandPolyphaseIIR, true, false };

    // Updaters
    void updateOnOff();
    void updateDrive();
    void updateLevel();
    void updateTone();
    void updateState();
    
    // Drive functions
    static float arcTanClipping(float sample);
        
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PunkDistAudioProcessor)
};
