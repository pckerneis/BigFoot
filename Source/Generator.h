/*
  ==============================================================================

    Generator.h
    Created: 20 Dec 2018 12:09:21am
    Author:  Pierre

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
struct SineWaveSound : public SynthesiserSound
{
	SineWaveSound() {}

	bool appliesToNote(int) override { return true; }
	bool appliesToChannel(int) override { return true; }
};

//==============================================================================
#include "ADSR.h"
#include "Distortion.h"

struct ParameterValues;

struct SineWaveVoice : public SynthesiserVoice
{
	SineWaveVoice(ADSREnvelope& envelope, ParameterValues& paramValues);

	void prepare(double sr);

	bool canPlaySound(SynthesiserSound* sound) override;

	void startNote(int midiNoteNumber, float velocity, SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
	void stopNote(float /*velocity*/, bool allowTailOff) override;

	void pitchWheelMoved(int) override {}
	void controllerMoved(int, int) override {}

	void renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;

	//==============================================================================
	void setDrive(float newValue);
	void setDriveType(Distortion<float>::TransferFunction func);

	//==============================================================================
	void setOutputGain(float newValue);

private:
	// Not using MidiMessage::getMidiNoteInHertz as it only takes integer midi note numbers
	double getMidiNoteInHertz(const float noteNumber) noexcept;

	float getNextEnvelopeValue();

	void updateAngleDelta();

	double currentAngle = 0.0, angleDelta = 0.0;

	LinearSmoothedValue<float> level;
	LinearSmoothedValue<float> noteRamp;
	LinearSmoothedValue<float> bendRamp;
	LinearSmoothedValue<float> filterRamp;
	LinearSmoothedValue<float> cutOffRamp;

	ParameterValues& values;

	ADSREnvelope& adsr;
	LinearSmoothedValue<float> gainValue;

	double sampleRate;

	bool legato;

	enum
	{
		distortionIndex,
		filterIndex,
		masterGainIndex
	};

	using Filter = dsp::StateVariableFilter::Filter<float>;
	using FilterParams = dsp::StateVariableFilter::Parameters<float>;
	juce::dsp::ProcessorChain<Distortion<float>, dsp::ProcessorDuplicator<Filter, FilterParams>, dsp::Gain<float>> fxChain;
};

//==============================================================================

class SynthAudioSource : public AudioSource
{
public:
	SynthAudioSource(ADSREnvelope& adsr, MidiKeyboardState& keyState, ParameterValues& values);

	void setUsingSineWaveSound();

	void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) override;
	void releaseResources() override;

	void process(const AudioSourceChannelInfo& bufferToFill, MidiBuffer& midiBuffer);

	// NOT USED !! See process() which handles incoming midi
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override {}

	SineWaveVoice* getVoice(int index)
	{
		return voices[index];
	}

private:
	MidiKeyboardState & keyboardState;
	Synthesiser synth;

	ADSREnvelope& adsr;

	Array<SineWaveVoice*> voices;
	MidiMessageCollector midiCollector;
};