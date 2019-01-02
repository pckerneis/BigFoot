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
	static String getProcessorStateIdentifier() { return JucePlugin_Name + String("Parameters"); }

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
	virtual void parameterValueChanged(int parameterIndex, float newValue) override 
	{
		// It looks like the raw value pointers are not updated yet in case of host automation
		// So we need to use the normalized newValue argument...

		auto voice = synthAudioSource->getVoice(0);

		if (voice == nullptr)
			return;

		if (parameterIndex == driveParam)
		{
			voice->setDrive(newValue);
		}
		else if (parameterIndex == driveTypeParam)
		{
			auto v = (int)parameters.getParameter("driveType")->getNormalisableRange().convertFrom0to1(newValue);

			if (v == 0)			voice->setDriveType(Distortion<float>::TransferFunction::softType);
			else if (v == 1)	voice->setDriveType(Distortion<float>::TransferFunction::hardType);
			else				voice->setDriveType(Distortion<float>::TransferFunction::sinType);
		}
		else if (parameterIndex == masterParam)
		{
			auto v = parameters.getParameter("master")->getNormalisableRange().convertFrom0to1(newValue);
			voice->setOutputGain(v);
		}
	}

	virtual void parameterGestureChanged(int, bool) override 
	{
		parameters.state.setProperty("presetChanged", true, nullptr);
	}
	
private:
	//==============================================================================
	enum
	{
		driveParam,
		driveTypeParam,
		bendAmountParam,
		bendDurationParam,
		lpFreqParam,
		attackParam,
		decayParam,
		sustainParam,
		releaseParam,
		glideParam,
		masterParam,
		lpModAmountParam,
		lpModDurationParam,
		lpResoParam
	};
    //==============================================================================

	//==============================================================================
	DefaultParameterValues defaultParameterValues;
	AudioProcessorValueTreeState parameters;

	ParameterValues values;

	ADSRParameters adsrParams;
	std::unique_ptr<ADSREnvelope> adsr;

	//==============================================================================
	MidiKeyboardState keyboardState;
	std::unique_ptr<SynthAudioSource> synthAudioSource;

	// This is a way to have a reference to the l&f that will persist even if the plugin editor is destroyed
	SharedResourcePointer<CustomLookAndFeel> lf;
	SharedResourcePointer<LogoButtonLF> logoLF;
	
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassGeneratorAudioProcessor)
};
