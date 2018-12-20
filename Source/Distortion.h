/*
  ==============================================================================

    Distortion.h
    Created: 20 Dec 2018 12:09:33am
    Author:  Pierre

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
template <typename Type>
class Distortion
{
public:
	enum TransferFunction
	{
		softType = 0,
		hardType,
		sinType,
	};

	//==============================================================================
	Distortion()
	{
		setTransferFunction(TransferFunction::softType);
		setDrive(0.5f);
	}

	//==============================================================================
	void setTransferFunction(TransferFunction f)
	{
		currentTransferFunction = f;

		auto& waveshaper = processorChain.template get<waveshaperIndex>();

		switch (f)
		{
		case TransferFunction::hardType:
			waveshaper.functionToUse = [](Type x) { return jlimit(Type(-1.0), Type(1.0), x); };
			break;

		case TransferFunction::softType:
			waveshaper.functionToUse = [](Type x) { return std::tanh(x); };
			break;

		case TransferFunction::sinType:
			waveshaper.functionToUse = [](Type x) { return std::sin(x); };

		default:
			break;
		}

		setDrive(currentDrive);
	}

	void setDrive(float newValue)
	{
		currentDrive = newValue;

		auto& preGain = processorChain.template get<preGainIndex>();
		preGain.setGainDecibels(newValue * maxPreGain);

		auto& postGain = processorChain.template get<postGainIndex>();
		postGain.setGainDecibels(newValue * minPostGain);
	}

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec)
	{
		processorChain.prepare(spec);
	}

	//==============================================================================
	template <typename ProcessContext>
	void process(const ProcessContext& context) noexcept
	{
		processorChain.process(context);
	}

	//==============================================================================
	void reset() noexcept
	{
		processorChain.reset();
	}

private:
	enum
	{
		preGainIndex,
		waveshaperIndex,
		postGainIndex
	};

	juce::dsp::ProcessorChain<dsp::Gain<Type>, dsp::WaveShaper<Type>, dsp::Gain<Type>> processorChain;

	//==============================================================================
	TransferFunction currentTransferFunction;
	float currentDrive;

	float maxPreGain = 40.0f;
	float minPostGain = -16.0f;
};
