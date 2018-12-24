/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "LookAndFeel.h"
//==============================================================================
BassGeneratorAudioProcessorEditor::BassGeneratorAudioProcessorEditor (BassGeneratorAudioProcessor& p, AudioProcessorValueTreeState& valueTreeState)
    :	AudioProcessorEditor (&p), 
		processor (p),
#if PAWG_USE_MIDI_KEYBOARD
		keyboardComponent(p.getKeyboardState(), MidiKeyboardComponent::horizontalKeyboard),
#endif
		presetBar(p.getValueTreeState())
{
	SharedResourcePointer<CustomLookAndFeel> lf;
	setLookAndFeel(lf);
	LookAndFeel::setDefaultLookAndFeel(lf);

	const auto bgColour = Colours::black.withBrightness(0.08f);
	lf->setColour(ResizableWindow::backgroundColourId, bgColour);
	lf->setColour(PopupMenu::backgroundColourId, bgColour);

	// Keyboard comp
#if PAWG_USE_MIDI_KEYBOARD
	addAndMakeVisible (keyboardComponent);
	keyboardComponent.setLowestVisibleKey(12);
#endif
	
	// Add sliders
	addRotarySlider(valueTreeState, ParameterIDs::glide,		Colours::gold);
	addRotarySlider(valueTreeState, ParameterIDs::bendAmount,	Colours::orange);
	addRotarySlider(valueTreeState, ParameterIDs::bendDuration, Colours::orange);
	addRotarySlider(valueTreeState, ParameterIDs::drive,		Colours::orangered);
	addLinearSlider(valueTreeState, ParameterIDs::driveType,	true);
	addRotarySlider(valueTreeState, ParameterIDs::attack,		Colours::beige);
	addRotarySlider(valueTreeState, ParameterIDs::decay,		Colours::beige);
	addRotarySlider(valueTreeState, ParameterIDs::sustain,		Colours::beige);
	addRotarySlider(valueTreeState, ParameterIDs::release,		Colours::beige);
	addRotarySlider(valueTreeState, ParameterIDs::lpFreq,		Colours::red.withMultipliedSaturation(0.9f));
	addRotarySlider(valueTreeState, ParameterIDs::lpReso,		Colours::red.withMultipliedSaturation(0.9f));
	addRotarySlider(valueTreeState, ParameterIDs::master,       Colours::beige);

#if PAWG_ALLOW_LPF_MODULATION
	addRotarySlider(valueTreeState, ParameterIDs::lpModAmount, Colours::red.withMultipliedSaturation(0.9f));
	addRotarySlider(valueTreeState, ParameterIDs::lpModDuration, Colours::red.withMultipliedSaturation(0.9f));
#endif


	// More slider styling
	for (auto c : getChildren())
	{
		if (auto s = dynamic_cast<Slider*>(c))
		{
			s->setColour(Slider::thumbColourId, Colours::black);
			s->setColour(Slider::rotarySliderOutlineColourId, Colours::black);
		}
	}

	addAndMakeVisible(presetBar);

    // Set editor size and start timer for the keyboard to grab focus

#if PAWG_ALLOW_LPF_MODULATION
	const int width = 440;
#else
	const int width = 400;
#endif

#if PAWG_USE_MIDI_KEYBOARD
	const int height = 300;
#else
	const int height = 230;
#endif

	setSize(width, height);

	backgroundImage.reset (new Image (Image::PixelFormat::ARGB, getWidth(), getHeight(), false));

	Graphics g (*backgroundImage);
	renderBackgroundImage(g);

#if PAWG_USE_MIDI_KEYBOARD
	startTimer(400);
#endif
}

BassGeneratorAudioProcessorEditor::~BassGeneratorAudioProcessorEditor()
{
}

//==============================================================================
void BassGeneratorAudioProcessorEditor::paint(Graphics& g)
{
	g.drawImage(*backgroundImage, getLocalBounds().toFloat());
}

void BassGeneratorAudioProcessorEditor::renderBackgroundImage(Graphics& g)
{
	g.fillAll (getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	const int labelHeight = 20;
	const int marginHeight = 8;
	const int sliderHeight = 70;
	const int headerHeight = 30;
	const int cellHeight = sliderHeight + labelHeight + (marginHeight * 0.5);

	auto r = getLocalBounds().reduced(5).toFloat();
#if PAWG_ALLOW_LPF_MODULATION
	const auto cellW = int((float)r.getWidth() / 7.0f);
#else
	const auto cellW = int((float)r.getWidth() / 6.0f);
#endif

	const float cornerSize = 6.0f;
	const float lineThickness = 0.8f;
	const float margin = 2.0f;

	g.setColour(Colours::lightgrey);
	
	r.removeFromTop(headerHeight);
	auto topRow = r.removeFromTop(cellHeight);

	Array<int> topCells;
#if PAWG_ALLOW_LPF_MODULATION
	topCells.add(1);
	topCells.add(2);
	topCells.add(4);
#else
	topCells.add(2);
	topCells.add(4);
#endif

	for (auto c : topCells)
		g.drawRoundedRectangle(topRow.removeFromLeft(c * cellW).reduced(margin, 0), cornerSize, lineThickness);

	Array<int> bottomCells;
#if PAWG_ALLOW_LPF_MODULATION
	bottomCells.add(2);
	bottomCells.add(4);
	bottomCells.add(1);
#else
	bottomCells.add(1);
	bottomCells.add(2);
	bottomCells.add(2);
	bottomCells.add(1);
#endif

	r.removeFromTop(marginHeight * 0.5);
	auto bottomRow = r.removeFromTop(cellHeight);

	for (auto c : bottomCells)
		g.drawRoundedRectangle(bottomRow.removeFromLeft(c * cellW).reduced(margin, 0), cornerSize, lineThickness);

	drawDriveTypeSymbols(g);
}

void BassGeneratorAudioProcessorEditor::resized()
{
	presetBar.setBounds(getLocalBounds().removeFromTop(30));

	if (sliders.isEmpty())
		return;

	const int marginHeight = 8;
	const int sliderHeight = 70;
	const int headerHeight = 30;
	const int labelHeight = 20;

	auto r = getLocalBounds().reduced(5);
	r.removeFromTop(headerHeight);

	auto layoutLabels = [this](StringArray ids, Rectangle<int> bounds)
	{
		bounds = bounds.withTrimmedTop(10);

		auto w = int((float)bounds.getWidth() / (float)ids.size());

		for (auto paramId : ids)
			getLabel(paramId)->setBounds(bounds.removeFromLeft(w));
	};

	auto layoutSliders = [this](StringArray ids, Rectangle<int> bounds)
	{
		auto w = int((float)bounds.getWidth() / (float)ids.size());

		for (auto paramId : ids)
		{
			auto b = bounds.removeFromLeft(w);

			if (paramId == ParameterIDs::driveType)
				b = b.withTrimmedRight(b.proportionOfWidth(0.5f)).withSizeKeepingCentre(b.proportionOfWidth(0.35f), b.proportionOfHeight(0.56f));

			getSlider(paramId)->setBounds(b);

		}
	};

	StringArray topRow;

#if PAWG_ALLOW_LPF_MODULATION
	topRow.add(ParameterIDs::glide);
	topRow.add(ParameterIDs::bendAmount);
	topRow.add(ParameterIDs::bendDuration);
	topRow.add(ParameterIDs::attack);
	topRow.add(ParameterIDs::decay);
	topRow.add(ParameterIDs::sustain);
	topRow.add(ParameterIDs::release);
#else
	topRow.add(ParameterIDs::bendAmount);
	topRow.add(ParameterIDs::bendDuration);
	topRow.add(ParameterIDs::attack);
	topRow.add(ParameterIDs::decay);
	topRow.add(ParameterIDs::sustain);
	topRow.add(ParameterIDs::release);
#endif

	layoutLabels(topRow, r.removeFromTop(labelHeight));
	layoutSliders(topRow, r.removeFromTop(sliderHeight));

	r.removeFromTop(marginHeight);

	StringArray bottomRow;

#if PAWG_ALLOW_LPF_MODULATION
	bottomRow.add(ParameterIDs::drive);
	bottomRow.add(ParameterIDs::driveType);
	bottomRow.add(ParameterIDs::lpFreq);
	bottomRow.add(ParameterIDs::lpReso);
	bottomRow.add(ParameterIDs::lpModAmount);
	bottomRow.add(ParameterIDs::lpModDuration);
	bottomRow.add(ParameterIDs::master);
#else
	bottomRow.add(ParameterIDs::glide);
	bottomRow.add(ParameterIDs::drive);
	bottomRow.add(ParameterIDs::driveType);
	bottomRow.add(ParameterIDs::lpFreq);
	bottomRow.add(ParameterIDs::lpReso);
	bottomRow.add(ParameterIDs::master);
#endif

	layoutLabels(bottomRow, r.removeFromTop(labelHeight));
	layoutSliders(bottomRow, r.removeFromTop(sliderHeight));

#if PAWG_USE_MIDI_KEYBOARD
	r.removeFromTop(marginHeight);
	keyboardComponent.setBounds(r);
#endif
}

void BassGeneratorAudioProcessorEditor::addLinearSlider(AudioProcessorValueTreeState & vts, String paramName, bool reversed)
{
	// Create slider and attachment
	auto slider = reversed ? new ReversedSlider(paramName + "Slider") : new Slider(paramName + "Slider");
	auto attachment = new SliderAttachment(vts, paramName, *slider);

	// Styling
	addAndMakeVisible(slider);
	slider->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
	slider->setSliderStyle(Slider::SliderStyle::LinearVertical);
	slider->setColour(Slider::backgroundColourId, Colours::grey);
	slider->onValueChange = [slider] { slider->setTooltip(slider->getTextFromValue(slider->getValue())); };

	// Create and style label
	auto l = new Label(paramName + "Label", ParameterIDs::getLabel(paramName).toUpperCase());
	addAndMakeVisible(l);
	l->setJustificationType(Justification::centred);
	l->setFont(l->getFont().withHeight(12.0f));

	// Keep pointers
	sliders.add(new AttachedSlider(paramName, slider, attachment, l));
}

void BassGeneratorAudioProcessorEditor::addRotarySlider(AudioProcessorValueTreeState& vts, String paramName, Colour colour)
{
	// Create slider and attachment
	auto slider = new Slider(paramName + "Slider");
	auto attachment = new SliderAttachment(vts, paramName, *slider);

	// Styling
	addAndMakeVisible(slider);
	slider->setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	slider->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
	slider->setColour(Slider::rotarySliderFillColourId, colour);
	slider->onValueChange = [slider] { slider->setTooltip(slider->getTextFromValue(slider->getValue())); };

	// Create and style label
	auto l = new Label(paramName + "Label", ParameterIDs::getLabel(paramName).toUpperCase());
	addAndMakeVisible(l);
	l->setJustificationType(Justification::centred);
	l->setFont(l->getFont().withHeight(12.0f));

	// Keep pointers
	sliders.add(new AttachedSlider(paramName, slider, attachment, l));
}

Slider * BassGeneratorAudioProcessorEditor::getSlider(String param)
{
	for (auto s : sliders)
		if (s->paramId == param)
			return s->slider.get();

	jassertfalse;

	return nullptr;
}

Label * BassGeneratorAudioProcessorEditor::getLabel(String param)
{
	for (auto s : sliders)
		if (s->paramId == param)
			return s->label.get();

	jassertfalse;

	return nullptr;
}

void BassGeneratorAudioProcessorEditor::drawDriveTypeSymbols(Graphics &g)
{
	auto slider = getSlider("driveType");
	const auto sliderArea = slider->getBounds().reduced(10, 6);

	// Lines
	const auto lineW = 4;
	const auto lineH = 2;

	g.setColour(Colours::white);
	g.drawRect(sliderArea.getRight() + 8, sliderArea.getY(), lineW, lineH);
	g.drawRect(sliderArea.getRight() + 8, sliderArea.getCentreY() - (int)(lineH * 0.5f), lineW, lineH);
	g.drawRect(sliderArea.getRight() + 8, sliderArea.getBottom() - lineH, lineW, lineH);

	const float symbolW = 13.0f;
	const float symbolH = 8.0f;
	const int symbolLeft = 16;

	// Soft clip
	Path softPath;
	softPath.startNewSubPath(0.0f, 1.0f);
	softPath.cubicTo(0.8f, 1.0f, 0.2f, 0.0f, 1.0f, 0.0f);

	Rectangle<float> softArea(sliderArea.getRight() + symbolLeft, sliderArea.getY() - symbolH * 0.5f, symbolW, symbolH);
	g.strokePath(softPath, PathStrokeType(1.0f), softPath.getTransformToScaleToFit(softArea, false));

	// Hard clip
	Path hardPath;
	hardPath.startNewSubPath(0.0f, 1.0f);
	hardPath.lineTo(0.3f, 1.0f);
	hardPath.lineTo(0.7f, 0.0f);
	hardPath.lineTo(1.0f, 0.0f);

	Rectangle<float> hardArea(sliderArea.getRight() + symbolLeft, sliderArea.getCentreY() - symbolH * 0.5f, symbolW, symbolH);
	g.strokePath(hardPath, PathStrokeType(1.0f), softPath.getTransformToScaleToFit(hardArea, false));

	// Sine fold
	Path sinePath;
	sinePath.startNewSubPath(0.0f, 0.5f);
	sinePath.cubicTo(0.25f, 1.0f, 0.25f, 1.0f, 0.5f, 0.5f);
	sinePath.cubicTo(0.75f, 0.0f, 0.75f, 0.0f, 1.0f, 0.5f);

	Rectangle<float> sineArea(sliderArea.getRight() + symbolLeft, sliderArea.getBottom() - symbolH * 0.5f, symbolW, symbolH);
	g.strokePath(sinePath, PathStrokeType(1.0f), sinePath.getTransformToScaleToFit(sineArea, false));
}
