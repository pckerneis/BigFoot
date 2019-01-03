/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BassGeneratorAudioProcessor::BassGeneratorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
	parameters (*this, nullptr, getProcessorStateIdentifier(),
		{
			std::make_unique<AudioParameterFloat>(ParameterIDs::bendAmount, "Bend amount",
												  NormalisableRange<float>(defaultParameterValues.minBendAmount, defaultParameterValues.maxBendAmount),
												  defaultParameterValues.bendAmount, " semi"),
				
			std::make_unique<AudioParameterFloat>(ParameterIDs::bendDuration, "Bend duration",
												  NormalisableRange<float>(defaultParameterValues.minBendDuration, defaultParameterValues.maxBendDuration, 0.0001f, 0.4f),
												  defaultParameterValues.bendDuration, " s"),
												   
			std::make_unique<AudioParameterFloat>(ParameterIDs::glide, "Glide",
												  NormalisableRange<float>(0.0f, defaultParameterValues.maxGlide, 0.0001f, 0.4f),
												  defaultParameterValues.glide, " s"),

			std::make_unique<AudioParameterFloat>(ParameterIDs::attack, "Attack",
												  NormalisableRange<float>(defaultParameterValues.minAttack, defaultParameterValues.maxAttack, 0.0001f, 0.25f),
												  defaultParameterValues.attack, " s"),

			std::make_unique<AudioParameterFloat>(ParameterIDs::decay, "Decay",
												  NormalisableRange<float>(defaultParameterValues.minDecay, defaultParameterValues.maxDecay, 0.0001f, 0.25f),
												  defaultParameterValues.decay, " s"),

			std::make_unique<AudioParameterFloat>(ParameterIDs::sustain, "Sustain",
												  NormalisableRange<float>(0.0f, 1.0f),
												  defaultParameterValues.sustain, " s"),

			std::make_unique<AudioParameterFloat>(ParameterIDs::release, "Release",
												  NormalisableRange<float>(defaultParameterValues.minRelease, defaultParameterValues.maxRelease, 0.0001f, 0.25f),
												  defaultParameterValues.release, " s"),

			std::make_unique<AudioParameterFloat>(ParameterIDs::drive, "Drive", 0.0f, 1.0f, defaultParameterValues.drive),
			std::make_unique<AudioParameterChoice>(ParameterIDs::driveType, "Drive type", defaultParameterValues.driveTypes, 0),
															  
			std::make_unique<AudioParameterFloat>(ParameterIDs::lpFreq, "LPF frequency",
												  NormalisableRange<float>(defaultParameterValues.minLpFreq, defaultParameterValues.maxLpFreq, 0.0001f, 0.25f),
												  defaultParameterValues.lpFreq, " Hz"),

			std::make_unique<AudioParameterFloat>(ParameterIDs::lpReso, "LPF resonance",
												  NormalisableRange<float>(defaultParameterValues.minLpReso, defaultParameterValues.maxLpReso),
												  defaultParameterValues.lpReso),

#if PAWG_ALLOW_LPF_MODULATION
			std::make_unique<AudioParameterFloat>(ParameterIDs::lpModAmount, "LPF mod amount",
												  NormalisableRange<float>(defaultParameterValues.minFilterModAmount, defaultParameterValues.maxFilterModAmount),
												  defaultParameterValues.filterModAmount),

			std::make_unique<AudioParameterFloat>(ParameterIDs::lpModDuration, "LPF mod duration",
												  NormalisableRange<float>(defaultParameterValues.minFilterModDuration, defaultParameterValues.maxFilterModDuration, 0.0001f, 0.4f),
												  defaultParameterValues.filterModDuration),
#endif
													  
			std::make_unique<AudioParameterFloat>(ParameterIDs::master, "Output gain",
												  NormalisableRange<float>(defaultParameterValues.minOutputGain, defaultParameterValues.maxOutputGain, 0.0001f, defaultParameterValues.computeSkewForMasterSlider()),
												  defaultParameterValues.master)
		})
{
	values.drive				= parameters.getRawParameterValue(ParameterIDs::drive);
	values.driveType			= parameters.getRawParameterValue(ParameterIDs::driveType);
	values.bendAmount			= parameters.getRawParameterValue(ParameterIDs::bendAmount);
	values.bendDuration			= parameters.getRawParameterValue(ParameterIDs::bendDuration);
	values.lpFreq				= parameters.getRawParameterValue(ParameterIDs::lpFreq);
	values.attack				= parameters.getRawParameterValue(ParameterIDs::attack);
	values.decay				= parameters.getRawParameterValue(ParameterIDs::decay);
	values.sustain				= parameters.getRawParameterValue(ParameterIDs::sustain);
	values.release				= parameters.getRawParameterValue(ParameterIDs::release);
	values.glide				= parameters.getRawParameterValue(ParameterIDs::glide);
	values.master				= parameters.getRawParameterValue(ParameterIDs::master);
	values.lpReso				= parameters.getRawParameterValue(ParameterIDs::lpReso);
#if PAWG_ALLOW_LPF_MODULATION
	values.lpModAmount			= parameters.getRawParameterValue(ParameterIDs::lpModAmount);
	values.lpModDuration		= parameters.getRawParameterValue(ParameterIDs::lpModDuration);
#endif

	adsr.reset(new ADSREnvelope(values.attack, values.decay, values.sustain, values.release));
	synthAudioSource.reset(new SynthAudioSource(*adsr, keyboardState, values));
	
	parameters.getParameter(ParameterIDs::drive)->addListener(this);
	parameters.getParameter(ParameterIDs::driveType)->addListener(this);
	parameters.getParameter(ParameterIDs::lpFreq)->addListener(this);
	parameters.getParameter(ParameterIDs::master)->addListener(this);
}

BassGeneratorAudioProcessor::~BassGeneratorAudioProcessor()
{
}

//==============================================================================
const String BassGeneratorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BassGeneratorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BassGeneratorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BassGeneratorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BassGeneratorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BassGeneratorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BassGeneratorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BassGeneratorAudioProcessor::setCurrentProgram (int /*index*/)
{
}

const String BassGeneratorAudioProcessor::getProgramName (int /*index*/)
{
    return {};
}

void BassGeneratorAudioProcessor::changeProgramName (int /*index*/, const String&)
{
}

//==============================================================================
void BassGeneratorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	synthAudioSource->prepareToPlay(samplesPerBlock, sampleRate);
	adsr->prepare(sampleRate);
}

void BassGeneratorAudioProcessor::releaseResources()
{
	synthAudioSource->releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BassGeneratorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void BassGeneratorAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear excessive outputs
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

	
	// MIDI note processing and sound generation
	AudioSourceChannelInfo infos (buffer);
	synthAudioSource->process(infos, midiMessages);

	midiMessages.clear();
}

//==============================================================================
bool BassGeneratorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* BassGeneratorAudioProcessor::createEditor()
{
	return new BassGeneratorAudioProcessorEditor(*this, parameters);
}

//==============================================================================
void BassGeneratorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
	auto state = parameters.copyState();
	std::unique_ptr<XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void BassGeneratorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(parameters.state.getType()))
			parameters.replaceState(ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BassGeneratorAudioProcessor();
}
