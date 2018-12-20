/*
  ==============================================================================

    Parameters.h
    Created: 20 Dec 2018 12:12:00am
    Author:  Pierre

  ==============================================================================
*/

#pragma once

//==============================================================================
struct ParameterValues
{
	float* drive;
	float* driveType;
	float* attack;
	float* decay;
	float* sustain;
	float* release;
	float* bendAmount;
	float* bendDuration;
	float* brightness;
	float* glide;
	float* master;
};

//==============================================================================
struct DefaultParameterValues
{
	DefaultParameterValues()
	{
		driveTypes.add("Soft");
		driveTypes.add("Hard");
		driveTypes.add("Sin");
	}

	StringArray driveTypes;

	float drive = 0.5f;

	float bendDuration = 0.2f;
	float minBendDuration = 0.001f;
	float maxBendDuration = 1.0f;

	float bendAmount = 7.0f;
	float minBendAmount = -24.0f;
	float maxBendAmount = 24.0f;

	float brightness = 1000.0f;
	float minBrightness = 50.0f;
	float maxBrightness = 10000.0f;

	float minAttack = 0.001;
	float maxAttack = 2.0f;
	float attack = 0.01f;

	float minDecay = 0.001;
	float maxDecay = 8.0f;
	float decay = 0.1f;

	float sustain = 0.8f;

	float minRelease = 0.001;
	float maxRelease = 8.0f;
	float release = 0.1f;

	float maxGlide = 1.0f;
	float glide = 0.2f;

	float master = 0.0f;
	float minOutputGain = -60.0f;
	float maxOutputGain = 16.0f;
};
