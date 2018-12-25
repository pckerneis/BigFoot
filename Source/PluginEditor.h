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
class BassGeneratorAudioProcessorEditor  : public AudioProcessorEditor, private Timer
{
public:
    BassGeneratorAudioProcessorEditor (BassGeneratorAudioProcessor&, AudioProcessorValueTreeState&);
    ~BassGeneratorAudioProcessorEditor();

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
	void timerCallback() override
	{
#if PAWG_USE_MIDI_KEYBOARD
		keyboardComponent.grabKeyboardFocus();
#endif
		stopTimer();
	}

	//==========================================================================
	BassGeneratorAudioProcessor& processor;

	//==========================================================================
#if PAWG_USE_MIDI_KEYBOARD
	MidiKeyboardComponent keyboardComponent;
#endif

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

	PresetBar presetBar;

	std::unique_ptr<Image> backgroundImage;

	SharedResourcePointer<TooltipWindow> tooltipWindow;

	const int labelHeight = 20;
	const int marginHeight = 8;
	const int sliderHeight = 64;
	const int headerHeight = 30;

	struct UIColours
	{
		const Colour backgroundColour		= Colours::darkslategrey.withBrightness(0.08f);
		const Colour textColour				= Colours::white;
		const Colour bendSliderColour		= Colours::orange;
		const Colour adsrSliderColour		= Colours::beige;
		const Colour glideSliderColour		= Colours::gold;
		const Colour driveSliderColour		= Colours::orangered;
		const Colour filterSliderColour		= Colours::Colours::red.withMultipliedSaturation(0.9f);
		const Colour masterSliderColour		= Colours::beige;
		const Colour lineColour				= Colours::grey;
		const Colour thumbColour			= Colours::black;
	};

	UIColours colors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassGeneratorAudioProcessorEditor)
};
