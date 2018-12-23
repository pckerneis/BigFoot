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

	//==========================================================================
	void renderBackgroundImage(Graphics& g);
	void drawDriveTypeSymbols(Graphics&, Rectangle<float>);

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

	OwnedArray<Slider> sliders;
	OwnedArray<SliderAttachment> sliderAttachments;
	OwnedArray<Label> labels;

	PresetBar presetBar;

	std::unique_ptr<Image> backgroundImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassGeneratorAudioProcessorEditor)
};
