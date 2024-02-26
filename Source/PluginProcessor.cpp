/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PunkDistAudioProcessor::PunkDistAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), state(*this, nullptr, "parameters", createParams())
#endif
{
}

PunkDistAudioProcessor::~PunkDistAudioProcessor()
{
}

//==============================================================================
const juce::String PunkDistAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PunkDistAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PunkDistAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PunkDistAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PunkDistAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PunkDistAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PunkDistAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PunkDistAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PunkDistAudioProcessor::getProgramName (int index)
{
    return {};
}

void PunkDistAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

// =========== PARAMETER LAYOUT ====================
AudioProcessorValueTreeState::ParameterLayout PunkDistAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> params;
        
    params.push_back(std::make_unique<AudioParameterBool>("ONOFF", "On/Off", true));
    params.push_back(std::make_unique<AudioParameterFloat>("DRIVE", "Drive Gain", NormalisableRange<float>(0.0f, 45.0f, 0.1f), DEFAULT_DRIVE, "dB"));
    params.push_back(std::make_unique<AudioParameterFloat>("LEVEL", "Output Level", NormalisableRange<float>(-30.0f, 30.0f, 0.1f), DEFAULT_LEVEL, "dB"));
    params.push_back(std::make_unique<AudioParameterFloat>("TONE1", "Tone 1", NormalisableRange<float>(200.0f, 2500.0f, 0.1f), DEFAULT_TONE1, "Hz"));
    params.push_back(std::make_unique<AudioParameterFloat>("TONE2", "Tone 2", NormalisableRange<float>(0.0f, 10.0f, 0.1f), DEFAULT_TONE2, ""));
    params.push_back(std::make_unique<AudioParameterBool>("MODE", "Modern/Raw", false));
    
    return { params.begin(), params.end() };
}

// ============ VALUE UPDATERS =====================
void PunkDistAudioProcessor::updateOnOff()
{
    auto ONOFF = state.getRawParameterValue("ONOFF");
    on = ONOFF->load();
}

void PunkDistAudioProcessor::updateDrive()
{
    // float sampleRate = getSampleRate();
    auto IN = state.getRawParameterValue("DRIVE");
    float inputValue = IN->load();
    
    drive.get<0>().setGainDecibels(inputValue);
 }

void PunkDistAudioProcessor::updateLevel()
{
    auto OUT = state.getRawParameterValue("LEVEL");
    float val = OUT->load();
    outputLevel.setGainDecibels(val);
}

void PunkDistAudioProcessor::updateTone()
{
    auto TONE1 = state.getRawParameterValue("TONE1");
    auto TONE2 = state.getRawParameterValue("TONE2");
    float val1 = TONE1->load();
    float val2 = Decibels::decibelsToGain(TONE2->load() * (-2.0f));
    
    float sampleRate = getSampleRate();
    *eq.get<0>().state = *dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, val1, 0.7071f, 2.5f);
    *eq.get<1>().state = *dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 800.0f, 0.7071f, val2);
}

void PunkDistAudioProcessor::updateMode()
{
    auto MODE = state.getRawParameterValue("MODE");
    mode = MODE->load();
}

void PunkDistAudioProcessor::updateState()
{
    updateOnOff();
    updateDrive();
    updateTone();
    updateLevel();
}

//==============================================================================
void PunkDistAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;
    
    drive.prepare(spec);
    drive.reset();
    drive.get<0>().setRampDurationSeconds(0.05);
    drive.get<1>().setBias(0.3f);
    drive.get<2>().functionToUse = arcTanClipping;
    drive.get<3>().setBias(-0.3f);
    *drive.get<4>().state = *dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 30.0f, 0.7071f);
    drive.get<5>().setRatio(30.0f);
    drive.get<5>().setAttack(1.0f);
    drive.get<5>().setThreshold(-6.0f);
    drive.get<5>().setRelease(60.0f);

    eq.prepare(spec);
    eq.reset();
    
    outputLevel.prepare(spec);
    outputLevel.setRampDurationSeconds(0.05);
}

void PunkDistAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PunkDistAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PunkDistAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    updateState();
    if(on)
    {
        dsp::AudioBlock<float> audioBlock (buffer);

        // Drive the entire signal
        dsp::ProcessContextReplacing<float> driveCtx(audioBlock);
        auto& inputBlock = driveCtx.getInputBlock();
        driveOV.processSamplesUp(inputBlock);
        drive.process(driveCtx);
        auto& outputBlock = driveCtx.getOutputBlock();
        driveOV.processSamplesDown(outputBlock);
                
        // Tone controls
        eq.process(dsp::ProcessContextReplacing<float>(audioBlock));
        
        // Output level
        outputLevel.process(dsp::ProcessContextReplacing<float>(audioBlock));
    }
}

//==============================================================================
bool PunkDistAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PunkDistAudioProcessor::createEditor()
{
    return new WrappedRasterAudioProcessorEditor (*this);
}

//==============================================================================
void PunkDistAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PunkDistAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PunkDistAudioProcessor();
}

// ========== DRIVE MATH FUNCTIONS =============================================
// Hard clipper
float PunkDistAudioProcessor::asymptoticClipping(float sample)
{
    return sample / (abs(sample) + 1);
}

// Best distorsion IMO
float PunkDistAudioProcessor::arcTanClipping(float sample)
{
    return 2.f / MathConstants<float>::pi * std::atan(sample);
}

/*
// ISSUE: Not tested, looks nice on graphs
float PunkDistAudioProcessor::newTanClipping(float sample)
{
    return std::atan(sample) / std::atan(drive.get<0>().getGainLinear());
}
 */

// Aggressive
float PunkDistAudioProcessor::tanhClipping(float sample)
{
    return 2.f / MathConstants<float>::pi * dsp::FastMathApproximations::tanh(MathConstants<float>::twoPi * sample);
}

// Fuzz
float PunkDistAudioProcessor::asymetricClipping(float sample)
{
    if (sample < -0.08905f) {
        return (-3 / 4) * (1 - (std::pow((1 - (std::abs(sample) - 0.032857f)), 12)) + (1 / 3) * (std::abs(sample) - 0.032847f)) + 0.01f;
    } else if (sample >= -0.08905f && sample < 0.320018f) {
        return (-6.153f * std::pow(sample, 2)) + 3.9375f * sample;
    } else {
        return 0.630035f;
    }
}

// Sounds like shit...
float PunkDistAudioProcessor::softClipping(float sample)
{
    return (3 * sample) / 2 * (1 - (std::pow(sample, 2) / 3));
}
