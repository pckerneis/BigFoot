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
	SineWaveVoice(ADSREnvelope& envelope, ParameterValues& paramValues, Array<int>& fxLayout);

	void prepare(double sr);

	bool canPlaySound(SynthesiserSound* sound) override;

	void startNote(int midiNoteNumber, float velocity, SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
	void stopNote(float /*velocity*/, bool allowTailOff) override;

	void pitchWheelMoved(int) override {}
	void controllerMoved(int, int) override {}

	void renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
	void processFX(int index, AudioSampleBuffer& outputBuffer, int startSample, int numSamples);

	//==============================================================================
	void setDrive(float newValue);
	void setDriveType(Distortion<float>::TransferFunction func);

	//==============================================================================
	void setOutputGain(float newValue);

	//==============================================================================
	enum
	{
		distortionIndex,
		filterIndex,
		masterGainIndex,

		// These are NOT processors in fxChain (so don't do fxChain.get<envelopeIndex>)
		envelopeIndex,
		oscIndex
	};

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
	
	using Filter = dsp::StateVariableFilter::Filter<float>;
	using FilterParams = dsp::StateVariableFilter::Parameters<float>;
	juce::dsp::ProcessorChain<Distortion<float>, dsp::ProcessorDuplicator<Filter, FilterParams>, dsp::Gain<float>> fxChain;

	Array<int>& currentRouting;
};

//==============================================================================

class MonoLegatoSynth : public Synthesiser
{
public:
	MonoLegatoSynth(bool triller = true) : trillerMode(triller)
	{}

	void setTrillerMode(bool shouldBeOn)
	{
		if (trillerMode == shouldBeOn)
			return;

		trillerMode = shouldBeOn;

		currentNotes.clear();
	}

	void clearPendingNotes()
	{
		currentNotes.clear();
	}

	void noteOn(int midiChannel, int midiNoteNumber, float velocity) override
	{
		Synthesiser::noteOn(midiChannel, midiNoteNumber, velocity);

		if (trillerMode)
			currentNotes.addIfNotAlreadyThere(PendingNote(midiNoteNumber, velocity));
	}

	void noteOff(int midiChannel, int midiNoteNumber, float velocity, bool allowTailOff) override
	{
		if (!trillerMode)
		{
			Synthesiser::noteOff(midiChannel, midiNoteNumber, velocity, allowTailOff);
		}
		else
		{
			auto latestNotePressed = currentNotes.getLast().noteNumber;
			currentNotes.removeFirstMatchingValue(PendingNote(midiNoteNumber, velocity));

			if (latestNotePressed == midiNoteNumber && !currentNotes.isEmpty())
			{
				auto note = currentNotes.getLast();

				// Retrigger previous note
				if (note.noteNumber >= 0)
					noteOn(midiChannel, note.noteNumber, note.velocity);
			}
			else
				Synthesiser::noteOff(midiChannel, midiNoteNumber, velocity, allowTailOff);
		}
	}

private:
	bool trillerMode;

	struct PendingNote
	{
		PendingNote() : noteNumber(-1), velocity(0.0f) {}
		PendingNote(int n, float v) : noteNumber(n), velocity(v) {}

		const int noteNumber;
		const float velocity;

		bool operator== (const PendingNote& other) const
		{
			return (noteNumber == other.noteNumber);
		}
	};

	Array<PendingNote> currentNotes;
};

//==============================================================================

class SynthAudioSource : public AudioSource
{
public:
	SynthAudioSource(ADSREnvelope& adsr, MidiKeyboardState& keyState, ParameterValues& values, Array<int>& fxLayout);

	void setUsingSineWaveSound();

	void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) override;
	void releaseResources() override;

	void process(const AudioSourceChannelInfo& bufferToFill, MidiBuffer& midiBuffer);

	// NOT USED !! See process() which handles incoming midi
	void getNextAudioBlock(const AudioSourceChannelInfo& /*bufferToFill*/) override {}

	SineWaveVoice* getVoice(int index)
	{
		return voices[index];
	}

private:
	MidiKeyboardState & keyboardState;
	MonoLegatoSynth synth;

	ADSREnvelope& adsr;

	Array<SineWaveVoice*> voices;
};