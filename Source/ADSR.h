/*
  ==============================================================================

    ADSR.h
    Created: 18 Dec 2018 3:00:04am
    Author:  Pierre

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
struct ADSRParameters
{
	float attack = 0;
	float decay = 0;
	float sustain = 1;
	float release = 0;
};

class ADSREnvelope
{
public:
	enum State
	{
		attackState,
		decayState,
		sustainState,
		releaseState,
		offState
	};

	ADSREnvelope(float* a, float* d, float* s, float* r) : attack(a), decay(d), sustain(s), release(r),
		sampleRate(0.0),
		numSamplesBeforeDecay(-1), numSamplesBeforeReleaseEnd(-1),
        rampStart(0.0f),
        rampEnd(1.0f),
        lastOutput(0.0f),
        exponentialAttack(false),
        exponentialDecay(true),
        exponentialRelease(true),
		currentlyOn(false),
        state(offState),
		envelopeLUT([](float x) { return pow(x, 2); }, 0.0f, 1.0f, 128)
	{
	}
	
	void prepare(double sr)
	{
		sampleRate = sr;
		ramp.reset(sr, 0.01f);
		ramp.setValue(0.0f, true);
		rampStart = 0.0f;
		rampEnd = 0.0f;
		lastOutput = 0;
		state = offState;
		currentlyOn = false;
		numSamplesBeforeDecay = -1;
		numSamplesBeforeReleaseEnd = -1;
	}

	float getNextValue()
	{
		jassert(sampleRate > 0);

		auto processOutput = [this](bool exponentialRamp)
		{
			auto v = exponentialRamp ? envelopeLUT(ramp.getNextValue()) : ramp.getNextValue();
			return rampStart + ((rampEnd - rampStart) * v);
		};

		float output;

		switch (state)
		{
		case offState:
			output = 0.0f;
			break;

		case attackState:
			output = processOutput(exponentialAttack);

			if (--numSamplesBeforeDecay == 0)
			{
				// Prepare decay phase
				ramp.reset(sampleRate, *decay);
				ramp.setValue(0.0f, true);
				ramp.setValue(1.0f, false);

				rampStart = 1.0;
				rampEnd = *sustain;

				state = decayState;
			}

			break;

		case decayState:
			output = processOutput(exponentialDecay);
			break;

		case sustainState:
			output = *sustain;
			break;

		case releaseState:
			output = processOutput(exponentialRelease);

			if (--numSamplesBeforeReleaseEnd == 0)
				state = offState;

			break;

		default:
			jassertfalse;
			output = 0.0f;
			break;
		}

		lastOutput = output;

		return output;
	}

	void noteOn()
	{
		jassert(sampleRate > 0);

		if (isCurrentlyOn() && skipAttackForLegato)
		{
			ramp.reset(sampleRate, *decay);
			ramp.setValue(0.0f, true);
			ramp.setValue(1.0f, false);

			numSamplesBeforeDecay = -1;
			numSamplesBeforeReleaseEnd = -1;

			rampStart = lastOutput;
			rampEnd = *sustain;

			state = decayState;

			return;
		}

		// Prepare attack phase
		ramp.reset(sampleRate, *attack);
		ramp.setValue(0.0f, true);
		ramp.setValue(1.0f, false);

		numSamplesBeforeDecay = static_cast<int>(*attack * sampleRate);
		numSamplesBeforeReleaseEnd = -1;

		rampStart = lastOutput;
		rampEnd = 1.0f;

		state = attackState;
		currentlyOn = true;
	}

	void noteOff()
	{
		jassert(sampleRate > 0);

		// Avoid undesired noteOff that would mess with the current state
		if (state == releaseState || state == offState)
			return;

		// Prepare release phase
		ramp.reset(sampleRate, *release);
		ramp.setValue(0.0f, true);
		ramp.setValue(1.0f, false);

		numSamplesBeforeDecay = -1;
		numSamplesBeforeReleaseEnd = static_cast<int>(*release * sampleRate);

		rampStart = lastOutput;
		rampEnd = 0.0f;

		state = releaseState;
		currentlyOn = false;
	}

	State getState() const { return state; }

	bool isCurrentlySounding() const
	{
		return state != offState;
	}

	bool isCurrentlyOn() const
	{
		return currentlyOn;
	}

	void releaseResources()
	{
		rampStart = 0.0f;
		rampEnd = 0.0f;
		lastOutput = 0.0f;
		state = offState;
		numSamplesBeforeDecay = -1;
		numSamplesBeforeReleaseEnd = -1;
		ramp.setValue(0.0f, true);
	}

private:
	float *attack, *decay, *sustain, *release;

	double sampleRate;

	int numSamplesBeforeDecay;
	int numSamplesBeforeReleaseEnd;

	float rampStart;
	float rampEnd;
	float lastOutput;

	bool exponentialAttack;
	bool exponentialDecay;
	bool exponentialRelease;

	bool currentlyOn;

	LinearSmoothedValue<float> ramp;
	State state;
	dsp::LookupTableTransform<float> envelopeLUT;

	bool skipAttackForLegato = true;
};
