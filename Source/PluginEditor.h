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

	Slider driveSlider;
	ReversedSlider driveTypeSlider;
	Slider attackSlider;
	Slider decaySlider;
	Slider sustainSlider;
	Slider releaseSlider;
	Slider bendAmountSlider;
	Slider bendDurationSlider;
	Slider brightnessSlider;
	Slider glideSlider;
	Slider masterSlider;

	OwnedArray<Label> labels;

	typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

	std::unique_ptr<SliderAttachment> driveAttachment;
	std::unique_ptr<SliderAttachment> driveTypeAttachment;
	std::unique_ptr<SliderAttachment> attackAttachment;
	std::unique_ptr<SliderAttachment> decayAttachment;
	std::unique_ptr<SliderAttachment> sustainAttachment;
	std::unique_ptr<SliderAttachment> releaseAttachment;
	std::unique_ptr<SliderAttachment> bendAmountAttachment;
	std::unique_ptr<SliderAttachment> bendDurationAttachment;
	std::unique_ptr<SliderAttachment> brightnessAttachment;
	std::unique_ptr<SliderAttachment> glideAttachment;
	std::unique_ptr<SliderAttachment> masterAttachment;

	PresetBar presetBar;

	std::unique_ptr<Image> backgroundImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassGeneratorAudioProcessorEditor)
};
