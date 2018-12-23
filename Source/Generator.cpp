/*
  ==============================================================================

    Generator.cpp
    Created: 20 Dec 2018 12:09:21am
    Author:  Pierre

  ==============================================================================
*/

#include "Generator.h"

#include "Parameters.h"
SineWaveVoice::SineWaveVoice(ADSREnvelope& envelope, ParameterValues& v) :
	currentAngle(0.0f),
	values(v),
	adsr(envelope),
	sampleRate(-1),
	legato(false)
{
}

void SineWaveVoice::prepare(double sr)
{
	fxChain.template get<filterIndex>().reset();
	fxChain.template get<masterGainIndex>().setRampDurationSeconds(0.005);

	dsp::ProcessSpec spec{ sampleRate, (uint32)sr, 2 };
	fxChain.prepare(spec);

	sampleRate = sr;
	level.reset(sr, 0.01f);
	bendRamp.reset(sr, 0.01f);
	noteRamp.reset(sr, 0.01f);
	filterRamp.reset(sr, 0.01f);
	cutOffRamp.reset(sr, 0.02f);
	legato = false;

	setDrive(*values.drive);
	setDriveType(static_cast<Distortion<float>::TransferFunction>((int)*values.driveType));
	setOutputGain(*values.master);
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
	if (legato && *values.glide != 0.0f)
	{
		// Glide to the target note without bending
		noteRamp.reset(sampleRate, *values.glide);
		noteRamp.setValue(midiNoteNumber);
		bendRamp.setValue(0.0f);
	}
	else
	{
		// Else just set the note (force)
		noteRamp.setValue(midiNoteNumber, true);

		// .. prepare bend ramp
		bendRamp.reset(sampleRate, *values.bendDuration);
		bendRamp.setValue(*values.bendAmount, true);
		bendRamp.setValue(0.0f);

		// .. prepare filter ramp
		filterRamp.reset(sampleRate, *values.lpModDuration);
		filterRamp.setValue(*values.lpModAmount, true);
		filterRamp.setValue(0.0f);

		// Stop the current note to avoid legato mode
		adsr.noteOff();
	}

	// Initialise angleDelta
	updateAngleDelta();

	// Prepare AD phase
	adsr.noteOn();

	legato = false;
}

void SineWaveVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
	// Looks like allowTailOff is false only when stopNote is called just before a startNote. We can use it to determine
	// If we need to send a noteOff to the ADSR (otherwise next startNote() won't allow glide)
	if (allowTailOff || adsr.getState() == ADSREnvelope::releaseState)
	{
		// We need a valid sample rate to prepare any ramps
		if (getSampleRate() > 0)
			adsr.noteOff();

		legato = false;
	}
	else
	{
		legato = true;
		clearCurrentNote();
		angleDelta = 0.0;
	}
}

void SineWaveVoice::renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
	if (angleDelta == 0.0)
	{
		// No sound generation
	}
	else if (!adsr.isCurrentlySounding())
	{
		clearCurrentNote();
		angleDelta = 0.0;
	}
	else
	{
		auto end = numSamples;
		auto start = startSample;

		while (--end >= 0)
		{
			auto currentSample = (float)(std::sin(currentAngle) * getNextEnvelopeValue());

			for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
				outputBuffer.addSample(i, start, currentSample);

			currentAngle += angleDelta;
			++start;

			updateAngleDelta();
		}
	}

	// FX processing
	const int freqUpdateRate = 50;

	auto block = juce::dsp::AudioBlock<float>(outputBuffer).getSubBlock(startSample, numSamples);

	for (int pos = 0; pos < numSamples; ++pos)
	{
		// Maximum block length for this pass

		if (pos % freqUpdateRate == 0)
		{
			// adjust cutoff value
			auto maxSize = jmin((numSamples - pos), freqUpdateRate);
			const auto cutOff = jlimit(20.0f, 20000.0f, filterRamp.getNextValue() + *values.lpFreq);
			filterRamp.skip(maxSize - 1);

			cutOffRamp.setValue(cutOff);
		}

		auto& filter = fxChain.template get<filterIndex>();
		filter.state->setCutOffFrequency(getSampleRate(), cutOffRamp.getNextValue(), *values.lpReso);

		// get sub-block context
		auto context = juce::dsp::ProcessContextReplacing<float>(block.getSubBlock(pos, 1));

		// process
		fxChain.process(context);
	}
}

void SineWaveVoice::setDrive(float newValue)
{
	auto& distortion = fxChain.template get<distortionIndex>();
	distortion.setDrive(*values.drive);
}

void SineWaveVoice::setDriveType(Distortion<float>::TransferFunction func)
{
	auto& distortion = fxChain.template get<distortionIndex>();
	distortion.setTransferFunction(func);
}

void SineWaveVoice::setOutputGain(float newValue)
{
	auto& gain = fxChain.template get<masterGainIndex>();
	gain.setGainDecibels(newValue);
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

SynthAudioSource::SynthAudioSource(ADSREnvelope& env, MidiKeyboardState& keyState, ParameterValues& values) : keyboardState(keyState), adsr(env)
{
	int numVoices = 1;

	for (auto i = 0; i < numVoices; ++i)
	{
		auto v = new SineWaveVoice(adsr, values);
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