/*
  ==============================================================================

    Generator.cpp
    Created: 20 Dec 2018 12:09:21am
    Author:  Pierre

  ==============================================================================
*/

#include "Generator.h"

#include "Parameters.h"
SineWaveVoice::SineWaveVoice(ADSREnvelope& envelope, ParameterValues& v, Array<int>& layout) :
	currentAngle(0.0f),
	values(v),
	adsr(envelope),
	sampleRate(-1),
	legato(false),
	currentRouting(layout)
{
	currentRouting =
	{
		envelopeIndex,
		distortionIndex,
		filterIndex
	};
}

void SineWaveVoice::prepare(double sr)
{
	jassert(sr >= 0);

	dsp::ProcessSpec spec{ sr, (uint32)sr, 2 };
	fxChain.prepare(spec);
	fxChain.template get<filterIndex>().reset();
	fxChain.template get<masterGainIndex>().setRampDurationSeconds(0.01);

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
		noteRamp.setValue((float)midiNoteNumber);
		bendRamp.setValue(0.0f);
	}
	else
	{
		// Else just set the note (force)
		noteRamp.setValue((float)midiNoteNumber, true);

		// .. prepare bend ramp
		bendRamp.reset(sampleRate, *values.bendDuration);
		bendRamp.setValue(*values.bendAmount, true);
		bendRamp.setValue(0.0f);

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
	else if (!adsr.isCurrentlySounding())	// Sound should end
	{
		clearCurrentNote();
		angleDelta = 0.0;
	}
	else
	{
		// Generate sound
		auto end = numSamples;
		auto start = startSample;

		while (--end >= 0)
		{
			auto currentSample = float(std::sin(currentAngle));

			for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
				outputBuffer.addSample(i, start, currentSample);

			currentAngle += angleDelta;
			++start;

			updateAngleDelta();
		}
	}

	// FX processing
	for (auto processorIndex : currentRouting)
		processFX(processorIndex, outputBuffer, startSample, numSamples);

	// Master gain
	auto block = juce::dsp::AudioBlock<float>(outputBuffer).getSubBlock(startSample, numSamples);
	auto context = juce::dsp::ProcessContextReplacing<float>(block);

	// process
	fxChain.template get<masterGainIndex>().process(context);
}

void SineWaveVoice::processFX(int index, AudioSampleBuffer & outputBuffer, int startSample, int numSamples)
{
	switch (index)
	{
		case envelopeIndex :
		{
			auto end = numSamples;
			auto start = startSample;

			while (--end >= 0)
			{
				for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
					outputBuffer.setSample(i, start, outputBuffer.getSample(i, start) * getNextEnvelopeValue());

				++start;
			}

			break;
		}

		case distortionIndex:
		{
			// get context
			auto block = juce::dsp::AudioBlock<float>(outputBuffer).getSubBlock(startSample, numSamples);
			auto context = juce::dsp::ProcessContextReplacing<float>(block);

			// process
			fxChain.template get<distortionIndex>().process(context);

			break;
		}

		case filterIndex:
		{
			auto block = juce::dsp::AudioBlock<float>(outputBuffer).getSubBlock(startSample, numSamples);

			// We won't adjust frequency each sample
			const int freqUpdateRate = 10;

			for (int pos = 0; pos < numSamples; ++pos)
			{
				if (pos % freqUpdateRate == 0)
				{
					// adjust cutoff value
					auto maxSize = jmin((numSamples - pos), freqUpdateRate);

					auto cutOff = float(std::pow(2, filterRamp.getNextValue())) * *values.lpFreq;
					cutOff = jlimit(20.0f, 20000.0f, cutOff);
					filterRamp.skip(maxSize - 1);

					cutOffRamp.setValue(cutOff);
				}

				auto& filter = fxChain.template get<filterIndex>();
				filter.state->setCutOffFrequency(getSampleRate(), cutOffRamp.getNextValue(), *values.lpReso);

				// get sub-block context
				auto sub = block.getSubBlock(pos, 1);
				auto context = juce::dsp::ProcessContextReplacing<float>(sub);

				// process
				filter.process(context);
			}
			break;
		}

		default:
			break;
	}
}

void SineWaveVoice::setDrive(float newValue)
{
	auto& distortion = fxChain.template get<distortionIndex>();
	distortion.setDrive(newValue);
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

SynthAudioSource::SynthAudioSource(ADSREnvelope& env, MidiKeyboardState& keyState, ParameterValues& values, Array<int>& fxLayout) : keyboardState(keyState), adsr(env)
{
	int numVoices = 1;

	for (auto i = 0; i < numVoices; ++i)
	{
		auto v = new SineWaveVoice(adsr, values, fxLayout);
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
	synth.clearPendingNotes();

	adsr.prepare(sampleRate);

	for (auto v : voices)
		v->prepare(sampleRate);
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
