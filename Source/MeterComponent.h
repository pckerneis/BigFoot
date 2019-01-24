/*
  ==============================================================================

    MeterComponent.h
    Created: 24 Jan 2019 12:05:31pm
    Author:  Pierre

  ==============================================================================
*/

#pragma once

#include "AudioDataCollector.h"


class MeterComponent : public Component, private Timer
{
public:
	//==============================================================================
	MeterComponent()
	{
		setFramesPerSecond(30);
		onColour = Colours::white;
		offColour = Colours::white.withAlpha(0.5f);
		numPoints = 10;
	}

	//==============================================================================
	void setOnColour(Colour newColour)
	{
		onColour = newColour;
	}

	void setOffColour(Colour newColour)
	{
		offColour = newColour;
	}

	void setNumPoints(int num)
	{
		numPoints = num;
	}

	//==============================================================================
	void setFramesPerSecond(int framesPerSecond)
	{
		jassert(framesPerSecond > 0 && framesPerSecond < 1000);
		startTimerHz(framesPerSecond);
	}

	//==============================================================================
	void paint(Graphics& g) override
	{
		float spacing = getHeight() / float(numPoints);
		auto r = getLocalBounds().toFloat();
		auto w = getWidth() * 0.4f;

		for (int i = 0; i < numPoints; ++i)
		{
			bool isOn = (1.0f - value0to1) < (float(i) / float(numPoints));

			g.setColour(isOn ? onColour : offColour);
			auto bounds = r.removeFromTop(spacing).withSizeKeepingCentre(w, w);
			g.fillEllipse(bounds);
		}
	}

	void resized() override {}

protected:
	virtual void computeCurrentValue() { value0to1 = 0.0; }
	double value0to1;

private:
	void timerCallback() override
	{
		computeCurrentValue();
		repaint();
	}

	Colour onColour;
	Colour offColour;
	int numPoints;
};

//==============================================================================
template <typename SampleType>
class AudioPeakMeterComponent : public MeterComponent
{
public:
	using Queue = AudioBufferQueue<SampleType>;

	AudioPeakMeterComponent(Queue& queueToUse)
		: audioBufferQueue(queueToUse)
	{
		sampleData.fill(SampleType(0));
		setFramesPerSecond(30);
	}

protected:
	void computeCurrentValue() override
	{
		audioBufferQueue.pop(sampleData.data());

		auto peak = SampleType(0);

		for (auto d : sampleData)
			if (d > peak)
				peak = d;

		value0to1 = peak;
	}

private:
	Queue& audioBufferQueue;
	std::array<SampleType, Queue::bufferSize> sampleData;
};

//==============================================================================
class FloatMeterComponent : public MeterComponent
{
public:
	FloatMeterComponent(float& v) : value(v) {}

protected:
	void computeCurrentValue() override
	{
		value0to1 = value;
	}

private:
	float& value;
};