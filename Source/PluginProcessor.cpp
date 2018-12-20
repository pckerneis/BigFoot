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
	parameters (*this, nullptr, "BassGenParameters",
		{
			std::make_unique<AudioParameterFloat>("drive", "Drive", 0.0f, 1.0f, defaultParameterValues.drive),
			std::make_unique<AudioParameterChoice>("driveType", "Drive type", defaultParameterValues.driveTypes, 0),

			std::make_unique<AudioParameterFloat>("bendAmount", "Bend amount",
												  NormalisableRange<float>(defaultParameterValues.minBendAmount, defaultParameterValues.maxBendAmount),
												  defaultParameterValues.bendAmount),
				
			std::make_unique<AudioParameterFloat>("bendDuration", "Bend duration",
												  NormalisableRange<float>(defaultParameterValues.minBendDuration, defaultParameterValues.maxBendDuration, 0.0001f, 0.4f),
												  defaultParameterValues.bendDuration),
												  
			std::make_unique<AudioParameterFloat>("brightness", "Brightness",
												  NormalisableRange<float>(defaultParameterValues.minBrightness, defaultParameterValues.maxBrightness, 0.0001f, 0.25f),
												  defaultParameterValues.brightness),
													  
			std::make_unique<AudioParameterFloat>("attack", "Attack",
												  NormalisableRange<float>(defaultParameterValues.minAttack, defaultParameterValues.maxAttack, 0.0001f, 0.25f),
												  defaultParameterValues.attack),

			std::make_unique<AudioParameterFloat>("decay", "Decay",
												  NormalisableRange<float>(defaultParameterValues.minDecay, defaultParameterValues.maxDecay, 0.0001f, 0.25f),
												  defaultParameterValues.decay),

			std::make_unique<AudioParameterFloat>("sustain", "Sustain",
												  0.0f, 1.0f,
												  defaultParameterValues.sustain),

			std::make_unique<AudioParameterFloat>("release", "Release",
												  NormalisableRange<float>(defaultParameterValues.minRelease, defaultParameterValues.maxRelease, 0.0001f, 0.25f),
												  defaultParameterValues.release),
												   
			std::make_unique<AudioParameterFloat>("glide", "Glide",
												  NormalisableRange<float>(0.0f, defaultParameterValues.maxGlide, 0.0001f, 0.4f),
												  defaultParameterValues.glide),

			std::make_unique<AudioParameterFloat>("master", "Output gain",
												  NormalisableRange<float>(defaultParameterValues.minOutputGain, defaultParameterValues.maxOutputGain, 0.0001f, 2.0f),
												  defaultParameterValues.master)

		}),
    adsr(adsrParams)
{
	values.drive			= parameters.getRawParameterValue("drive");
	values.driveType		= parameters.getRawParameterValue("driveType");
	values.bendAmount		= parameters.getRawParameterValue("bendAmount");
	values.bendDuration		= parameters.getRawParameterValue("bendDuration");
	values.brightness		= parameters.getRawParameterValue("brightness");
	values.attack			= parameters.getRawParameterValue("attack");
	values.decay			= parameters.getRawParameterValue("decay");
	values.sustain			= parameters.getRawParameterValue("sustain");
	values.release			= parameters.getRawParameterValue("release");
	values.glide			= parameters.getRawParameterValue("glide");
	values.master			= parameters.getRawParameterValue("master");

	synthAudioSource.reset(new SynthAudioSource(adsr, keyboardState, values));
	
	parameters.getParameter("drive")->addListener(this);
	parameters.getParameter("driveType")->addListener(this);
	parameters.getParameter("bendAmount")->addListener(this);
	parameters.getParameter("bendDuration")->addListener(this);
	parameters.getParameter("brightness")->addListener(this);
	parameters.getParameter("attack")->addListener(this);
	parameters.getParameter("decay")->addListener(this);
	parameters.getParameter("sustain")->addListener(this);
	parameters.getParameter("release")->addListener(this);
	parameters.getParameter("glide")->addListener(this);
	parameters.getParameter("master")->addListener(this);
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

void BassGeneratorAudioProcessor::setCurrentProgram (int index)
{
}

const String BassGeneratorAudioProcessor::getProgramName (int index)
{
    return {};
}

void BassGeneratorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void BassGeneratorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	synthAudioSource->prepareToPlay(samplesPerBlock, sampleRate);
	
	setGlide(*values.glide);
	setBendAmount(*values.bendAmount);
	setBendDuration(*values.bendDuration);
	setDrive(*values.drive);
	setDriveType(static_cast<Distortion<float>::TransferFunction>((int)*values.driveType));
	setBrightness(*values.brightness);
	setOutputGain(*values.master);
	adsr.prepare(sampleRate);
	adsr.setParameters(*values.attack, *values.decay, *values.sustain, *values.release);

	dsp::ProcessSpec spec { sampleRate, (uint32)samplesPerBlock, 2 };
	fxChain.prepare (spec);
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

	// FX processing
	auto block = juce::dsp::AudioBlock<float>(buffer);
	auto context = juce::dsp::ProcessContextReplacing<float>(block);
	fxChain.process(context);

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
