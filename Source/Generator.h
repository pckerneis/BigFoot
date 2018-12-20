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

struct SineWaveVoice : public SynthesiserVoice
{
	SineWaveVoice(ADSREnvelope& envelope, float* bendAmt, float* bendDur, float* glid);

	void prepare(double sr);

	bool canPlaySound(SynthesiserSound* sound) override;

	void startNote(int midiNoteNumber, float velocity, SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
	void stopNote(float /*velocity*/, bool allowTailOff) override;

	void pitchWheelMoved(int) override {}
	void controllerMoved(int, int) override {}

	void renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;

private:
	// Not using MidiMessage::getMidiNoteInHertz as it only takes integer midi note numbers
	double getMidiNoteInHertz(const float noteNumber) noexcept;

	float getNextEnvelopeValue();

	void updateAngleDelta();

	double currentAngle = 0.0, angleDelta = 0.0;

	LinearSmoothedValue<float> level;
	LinearSmoothedValue<float> noteRamp;
	LinearSmoothedValue<float> bendRamp;
	float* bendAmount;
	float* bendDuration;
	float* glide;

	ADSREnvelope& adsr;
	LinearSmoothedValue<float> gainValue;

	double sampleRate;
};

//==============================================================================
struct ParameterValues;

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

private:
	MidiKeyboardState & keyboardState;
	Synthesiser synth;

	ADSREnvelope& adsr;

	Array<SineWaveVoice*> voices;
	MidiMessageCollector midiCollector;
};