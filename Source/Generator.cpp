/*
  ==============================================================================

    Generator.cpp
    Created: 20 Dec 2018 12:09:21am
    Author:  Pierre

  ==============================================================================
*/

#include "Generator.h"

SineWaveVoice::SineWaveVoice(ADSREnvelope& envelope, float* bendAmt, float* bendDur, float *glid) :
	currentAngle(0.0f),
	bendAmount(bendAmt),
	bendDuration(bendDur),
	glide(glid),
	adsr(envelope),
	sampleRate(-1)
{
}

void SineWaveVoice::prepare(double sr)
{
	sampleRate = sr;
	level.reset(sr, 0.01f);
	bendRamp.reset(sr, 0.01f);
	noteRamp.reset(sr, 0.01f);
}

bool SineWaveVoice::canPlaySound(SynthesiserSound* sound)
{
	return dynamic_cast<SineWaveSound*> (sound) != nullptr;
}

void SineWaveVoice::startNote(int midiNoteNumber, float velocity, SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
	// Reset osc state
	level.setValue(velocity * 0.35f);

	// If there's already a note playing and glide param is non 0
	if (adsr.isCurrentlyOn() && *glide >= 0.01f)
	{
		// Stop the current note to avoid legato mode
		adsr.noteOff();

		// Glide to the target note without bending
		noteRamp.reset(sampleRate, *glide);
		noteRamp.setValue(midiNoteNumber);
		bendRamp.setValue(0.0f);
	}
	else
	{
		// Else just set the note (force)
		noteRamp.setValue(midiNoteNumber, true);

		// ...and prepare bend ramp
		bendRamp.reset(sampleRate, *bendDuration);
		bendRamp.setValue(*bendAmount, true);
		bendRamp.setValue(0.0f);
	}

	// Initialise angleDelta
	updateAngleDelta();

	// Prepare AD phase
	adsr.noteOn();
}

void SineWaveVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
	// Looks like allowTailOff is false only when stopNote is called just before a startNote. We can use it to determine
	// If we need to send a noteOff to the ADSR (otherwise next startNote() won't allow glide)
	if (allowTailOff)
	{
		// We need a valid sample rate to prepare ramps
		if (getSampleRate() > 0)
			adsr.noteOff();
	}
	else
	{
		clearCurrentNote();
		angleDelta = 0.0;
	}
}

void SineWaveVoice::renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
	if (angleDelta == 0.0)
		return;

	if (!adsr.isCurrentlySounding())
	{
		clearCurrentNote();
		angleDelta = 0.0;
	}
	else
	{
		while (--numSamples >= 0)
		{
			auto currentSample = (float)(std::sin(currentAngle) * getNextEnvelopeValue());

			for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
				outputBuffer.addSample(i, startSample, currentSample);

			currentAngle += angleDelta;
			++startSample;

			updateAngleDelta();
		}
	}
}

double SineWaveVoice::getMidiNoteInHertz(const float noteNumber) noexcept
{
	return 440.0f * std::pow(2.0, (noteNumber - 69.0f) / 12.0);
}

float SineWaveVoice::getNextEnvelopeValue()
{
	return adsr.getNextValue() * level.getNextValue();
}

void SineWaveVoice::updateAngleDelta()
{
	auto cyclesPerSecond = getMidiNoteInHertz(noteRamp.getNextValue() + bendRamp.getNextValue());
	auto cyclesPerSample = cyclesPerSecond / getSampleRate();
	angleDelta = cyclesPerSample * 2.0 * MathConstants<double>::pi;
}
//==============================================================================
#include "Parameters.h"

SynthAudioSource::SynthAudioSource(ADSREnvelope& env, MidiKeyboardState& keyState, ParameterValues& values) : keyboardState(keyState), adsr(env)
{
	int numVoices = 1;

	for (auto i = 0; i < numVoices; ++i)
	{
		auto v = new SineWaveVoice(adsr, values.bendAmount, values.bendDuration, values.glide);
		synth.addVoice(v);
		voices.add(v);
	}

	synth.addSound(new SineWaveSound());
}

void SynthAudioSource::setUsingSineWaveSound()
{
	synth.clearSounds();
}

void SynthAudioSource::prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate)
{
	synth.setCurrentPlaybackSampleRate(sampleRate);

	adsr.prepare(sampleRate);

	for (auto v : voices)
		v->prepare(sampleRate);

	midiCollector.reset(sampleRate);
}

void SynthAudioSource::releaseResources()
{
	adsr.releaseResources();
}

void SynthAudioSource::process(const AudioSourceChannelInfo& bufferToFill, MidiBuffer& midiBuffer)
{
	bufferToFill.clearActiveBufferRegion();

	keyboardState.processNextMidiBuffer(midiBuffer, bufferToFill.startSample,
		bufferToFill.numSamples, true);

	synth.renderNextBlock(*bufferToFill.buffer, midiBuffer,
		bufferToFill.startSample, bufferToFill.numSamples);
}