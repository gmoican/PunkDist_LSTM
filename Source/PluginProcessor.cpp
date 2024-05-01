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
    juce::ignoreUnused(index);
}

const juce::String PunkDistAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void PunkDistAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

// =========== PARAMETER LAYOUT ====================
juce::AudioProcessorValueTreeState::ParameterLayout PunkDistAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("ONOFF", 0), "On/Off", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("DRIVE", 0), "Drive Gain", juce::NormalisableRange<float>(0.0f, 60.0f, 0.1f), DEFAULT_DRIVE, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("LEVEL", 0), "Output Level", juce::NormalisableRange<float>(-30.0f, 30.0f, 0.1f), DEFAULT_LEVEL, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("TONE1", 0), "Tone 1", juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f), DEFAULT_TONE1, ""));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("TONE2", 0), "Tone 2", juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f), DEFAULT_TONE2, ""));
    
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
    auto IN = state.getRawParameterValue("DRIVE");
    float inputValue = IN->load();
    
    inputGain.setGainDecibels(inputValue);
    driveCompensationGain.setGainDecibels(-inputValue);
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
    float tone1freq = juce::jmap(TONE1->load(), 0.f, 10.f, 200.f, 2500.f);
    float tone1gain = juce::jmap(TONE1->load(), 0.f, 10.f, 1.f, 2.5f);
    float tone2dip = juce::Decibels::decibelsToGain(TONE2->load() * (-1.0f));
    float tone2bump = juce::jmap(TONE2->load(), 0.f, 10.f, 1.f, 1.5f);
    
    float sampleRate = getSampleRate();
    *eq.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, tone1freq, 0.7071f, tone1gain);
    *eq.get<2>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 800.0f, 1.5f, tone2dip);
    *eq.get<3>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 80.0f, 0.7071f, tone2bump);
}

void PunkDistAudioProcessor::updateState()
{
    updateOnOff();
    updateTone();
    updateDrive();
    updateLevel();
}

//==============================================================================
void PunkDistAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;
    
    inputGain.prepare(spec);
    inputGain.setRampDurationSeconds(0.05);
    
    driveCompensationGain.prepare(spec);
    driveCompensationGain.setRampDurationSeconds(0.05);
    
    // Read json model
    juce::MemoryInputStream jsonInputStream(BinaryData::ml_minidist_model_json, BinaryData::ml_minidist_model_jsonSize, false);
    nlohmann::json weights_json = nlohmann::json::parse(jsonInputStream.readEntireStreamAsString().toStdString());
    LSTM.reset();
    LSTM.load_json(weights_json);
    
    eq.prepare(spec);
    eq.reset();
    *eq.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.f, 0.7071f);
    *eq.get<4>().state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 8000.f, 0.3536f);
    
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
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    
    updateState();
    if(on)
    {
        juce::dsp::AudioBlock<float> audioBlock (buffer);
        
        // Input level
        inputGain.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        
        // Model inference
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            LSTM.process(buffer.getReadPointer(ch), buffer.getWritePointer(ch), buffer.getNumSamples());
        }
        
        driveCompensationGain.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));

        // Tone controls
        eq.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        
        // Output level
        outputLevel.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
    }
}

//==============================================================================
bool PunkDistAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PunkDistAudioProcessor::createEditor()
{
    return new PunkDistEditor (*this);
}

//==============================================================================
void PunkDistAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::ignoreUnused(destData);
}

void PunkDistAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Preset loader

    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PunkDistAudioProcessor();
}
