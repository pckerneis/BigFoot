/*
  ==============================================================================

    Parameters.h
    Created: 20 Dec 2018 12:12:00am
    Author:  Pierre

  ==============================================================================
*/

#pragma once

namespace ParameterIDs
{
#define DECLARE_ID(name) const juce::String name (#name);


	DECLARE_ID(glide)

	DECLARE_ID(bendAmount)
	DECLARE_ID(bendDuration)

	DECLARE_ID(drive)
	DECLARE_ID(driveType)

	DECLARE_ID(attack)
	DECLARE_ID(decay)
	DECLARE_ID(sustain)
	DECLARE_ID(release)

	DECLARE_ID(lpFreq)
	DECLARE_ID(lpModAmount)
	DECLARE_ID(lpModDuration)
	DECLARE_ID(lpReso)

	DECLARE_ID(master)


#undef DECLARE_ID

	static String getLabel(String paramId);
}

String ParameterIDs::getLabel(String type)
{
	String name;

	if (type == glide)						name = "Glide";
	else if (type == bendAmount)            name = "Bend";
	else if (type == bendDuration)          name = "Time";
	else if (type == drive)					name = "Drive";
	else if (type == driveType)				name = "Type";
	else if (type == attack)				name = "Attack";
	else if (type == decay)					name = "Decay";
	else if (type == sustain)				name = "Sustain";
	else if (type == release)               name = "Release";
	else if (type == lpFreq)                name = "Freq";
	else if (type == lpModAmount)			name = "Mod amt";
	else if (type == lpModDuration)			name = "mod time";
	else if (type == lpReso)				name = "Reso";
	else if (type == master)                name = "Output";

	return name;
}

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
	float* glide;
	float* master;
	float* lpFreq;
	float* lpModAmount;
	float* lpModDuration;
	float* lpReso;
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

	float drive = 0.0f;

	float bendDuration = 0.001f;
	float minBendDuration = 0.001f;
	float maxBendDuration = 2.0f;

	float bendAmount = 0.0f;
	float minBendAmount = -24.0f;
	float maxBendAmount = 24.0f;

	float minAttack = 0.001;
	float maxAttack = 2.0f;
	float attack = 0.001f;

	float minDecay = 0.001;
	float maxDecay = 8.0f;
	float decay = 0.001f;

	float sustain = 1.0f;

	float minRelease = 0.001;
	float maxRelease = 8.0f;
	float release = 0.001f;

	float maxGlide = 1.0f;
	float glide = 0.0f;

	float master = 0.0f;
	float minOutputGain = -60.0f;
	float maxOutputGain = 16.0f;

	float lpFreq = 10000.0f;
	float minLpFreq = 10.0f;
	float maxLpFreq = 10000.0f;

	float filterModAmount = 0.0f;
	float minFilterModAmount = -8.0f;
	float maxFilterModAmount = 8.0f;

	float filterModDuration = 0.1f;
	float minFilterModDuration = 0.001f;
	float maxFilterModDuration = 2.0f;

	float lpReso = 1.0f / sqrt(2.0f);
	float minLpReso = 0.1f;
	float maxLpReso = 5.0f;

	float computeSkewForMasterSlider()
	{
		return std::log(0.5f) / std::log((0.0f - minOutputGain) / (maxOutputGain - minOutputGain));
	}
};
