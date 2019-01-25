/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "PresetBar.h"

//==============================================================================
class ReversedSlider : public Slider
{
public:
	ReversedSlider() {}
	ReversedSlider(const String& componentName) : Slider(componentName) {}
	~ReversedSlider() {}

	double proportionOfLengthToValue(double proportion) { return Slider::proportionOfLengthToValue(1.0f - proportion); };
	double valueToProportionOfLength(double value) { return 1.0f - (Slider::valueToProportionOfLength(value)); };
};

//==============================================================================
/**
*/

#include "MeterComponent.h"

#if BIGFOOT_DYNAMIC_ROUTING
	#include "AudioChainComponent.h"
#endif

class BigFootEditor  : public AudioProcessorEditor
{
public:
    BigFootEditor (BigFootAudioProcessor&, AudioProcessorValueTreeState&);
    ~BigFootEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
	void addLinearSlider(AudioProcessorValueTreeState& vts, String paramName, bool reversed = false);
	void addRotarySlider(AudioProcessorValueTreeState& vts, String paramName, Colour colour);

	Slider* getSlider(String param);
	Label* getLabel(String param);

	//==========================================================================
	void renderBackgroundImage(Graphics& g);
	void drawDriveTypeSymbols(Graphics& g);

	//==========================================================================
	BigFootAudioProcessor& processor;

	//==========================================================================
	typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

	struct AttachedSlider
	{
		AttachedSlider(String param, Slider* s, SliderAttachment* a, Label* l) : paramId(param)
		{
			slider.reset(s);
			attachment.reset(a);
			label.reset(l);
		}

		const String paramId;
		std::unique_ptr<Slider> slider;
		std::unique_ptr<SliderAttachment> attachment;
		std::unique_ptr<Label> label;
	};

	OwnedArray<AttachedSlider> sliders;

	std::unique_ptr<Image> backgroundImage;

	SharedResourcePointer<TooltipWindow> tooltipWindow;

	const int labelHeight = 22;
	const int marginHeight = 8;
	int sliderHeight = 68;
	const int headerHeight = 30;
#if BIGFOOT_DYNAMIC_ROUTING
	const int footerHeight = 40;
#endif
	const int audioChainWidth = 192;

	struct UIColours
	{
		const Colour backgroundColour		= Colour::fromRGB(17, 17, 19);
		const Colour textColour				= Colour::fromRGB(240, 240, 245);
		const Colour lineColour				= Colour::fromRGB(220, 205, 227).darker();
		const Colour highlightColour		= Colours::sandybrown;

		const Colour sliderColour			= Colour::fromRGB(92, 90, 91);

		const Colour bendSliderColour		= Colour::fromRGB(92, 90, 91);
		const Colour adsrSliderColour		= Colour::fromRGB(92, 90, 91);
		const Colour glideSliderColour		= Colour::fromRGB(92, 90, 91);
		const Colour driveSliderColour		= Colour::fromRGB(92, 90, 91);
		const Colour filterSliderColour		= Colour::fromRGB(92, 90, 91);
		const Colour masterSliderColour		= Colour::fromRGB(92, 90, 91);

		const Colour thumbColour = Colours::black;
	};

	UIColours colors;

	PresetBar presetBar;	// Use 'colors' at instantiation
	Rectangle<int> sliderZone;

	AudioPeakMeterComponent<float> audioMeter;
	FloatMeterComponent midiMeter;

	ResizableBorderComponent resizableBorder;

#if BIGFOOT_DYNAMIC_ROUTING
	AudioChainComponent audioChainComp;

	void audioChainChanged();
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BigFootEditor)
};
