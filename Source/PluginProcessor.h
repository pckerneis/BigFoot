/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "LookAndFeel.h"
#include "Distortion.h"
#include "Generator.h"
#include "Parameters.h"

//==============================================================================

/**
*/
class BassGeneratorAudioProcessor  : public AudioProcessor, public AudioProcessorParameter::Listener
{
public:
    //==============================================================================
    BassGeneratorAudioProcessor();
    ~BassGeneratorAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	//==============================================================================
	SynthAudioSource& getSynthAudioSource() { return *synthAudioSource;  }
	MidiKeyboardState& getKeyboardState() { return keyboardState; }

	//==============================================================================
	void setDrive (float newValue) 
	{
		*values.drive = newValue;

		auto& distortion = fxChain.template get<distortionIndex>();
		distortion.setDrive(*values.drive);
	}

	void setDriveType(Distortion<float>::TransferFunction func)
	{
		*values.driveType = func;

		auto& distortion = fxChain.template get<distortionIndex>();
		distortion.setTransferFunction(func);
	}

	//==============================================================================
	void setBendAmount(float newValue)
	{
		*values.bendAmount = newValue;
	}

	void setBendDuration(float newValue)
	{
		*values.bendDuration = newValue;
	}

	//==============================================================================
	void setBrightness(float newValue)
	{
		*values.brightness = newValue;

		if (getSampleRate() <= 0)	// If prepareToPlay() wasn't called yet, the filter state will be set there
			return;

		auto& filter = fxChain.template get<filterIndex>();
		filter.state->setCutOffFrequency(getSampleRate(), newValue);
	}

	//==============================================================================
	void setAttack(float newValue)
	{
		*values.attack = newValue;

		adsr.setAttack(newValue);
	}

	void setDecay(float newValue)
	{
		*values.decay = newValue;

		adsr.setDecay(newValue);
	}

	void setSustain(float newValue)
	{
		*values.sustain = newValue;

		adsr.setSustain(newValue);
	}

	void setRelease(float newValue)
	{
		*values.release = newValue;

		adsr.setRelease(newValue);
	}

	//==============================================================================
	void setGlide(float newValue)
	{
		*values.glide = newValue;
	}

	//==============================================================================
	void setOutputGain(float newValue)
	{
		*values.master = newValue;

		auto& gain = fxChain.template get<masterGainIndex>();
		gain.setGainDecibels(newValue);
	}
	
	//==============================================================================
	DefaultParameterValues getDefaultParameterValues()
	{
		return defaultParameterValues;
	}

	AudioProcessorValueTreeState& getValueTreeState()
	{
		return parameters;
	}

	//==============================================================================
	/* Implementation of AudioProcessorParameter::Listener. */
	virtual void parameterValueChanged(int parameterIndex, float newValue) override {}
	virtual void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override 
	{
		parameters.state.setProperty("presetChanged", true, nullptr);
	}
private:
    //==============================================================================
	enum
	{
		distortionIndex,
		filterIndex,
		masterGainIndex
	};

	using Filter = dsp::StateVariableFilter::Filter<float>;
	using FilterParams = dsp::StateVariableFilter::Parameters<float>;

	juce::dsp::ProcessorChain<Distortion<float>, dsp::ProcessorDuplicator<Filter, FilterParams>, dsp::Gain<float>> fxChain;

	//==============================================================================
	DefaultParameterValues defaultParameterValues;
	AudioProcessorValueTreeState parameters;

	ParameterValues values;

	ADSRParameters adsrParams;
	ADSREnvelope adsr;

	//==============================================================================
	MidiKeyboardState keyboardState;
	std::unique_ptr<SynthAudioSource> synthAudioSource;

	// This is a way to have a reference to the l&f that will persist even if the plugin editor is destroyed
	SharedResourcePointer<CustomLookAndFeel> lf;
	SharedResourcePointer<LogoButtonLF> logoLF;
	
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassGeneratorAudioProcessor)
};
