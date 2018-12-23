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

	// Get default param values
	auto defaultValues = p.getDefaultParameterValues();

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
	addRotarySlider(valueTreeState, ParameterIDs::master,       Colours::beige);

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
#if PAWG_USE_MIDI_KEYBOARD
    setSize (400, 300);
	startTimer(400);
#else
	setSize(400, 230);
#endif
	backgroundImage.reset (new Image (Image::PixelFormat::ARGB, getWidth(), getHeight(), false));

	Graphics g (*backgroundImage);
	renderBackgroundImage(g);
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
	const auto cellW = r.proportionOfWidth(0.2);
	const auto adsrW = cellW * (3.0f / 4.0f);

	const float cornerSize = 6.0f;
	const float lineThickness = 0.8f;
	const float margin = 2.0f;

	r.removeFromTop(headerHeight);
	auto topRow = r.removeFromTop(cellHeight);

	g.setColour(Colours::white);

	g.drawRoundedRectangle(topRow.removeFromLeft(cellW).reduced(margin, 0), cornerSize, lineThickness);
	g.drawRoundedRectangle(topRow.removeFromLeft(2 * cellW).reduced(margin, 0), cornerSize, lineThickness);
	g.drawRoundedRectangle(topRow.reduced(margin, 0), cornerSize, lineThickness);

	auto switchArea = topRow.removeFromRight(topRow.proportionOfWidth(0.5f)).toFloat();
	drawDriveTypeSymbols(g, switchArea);

	r.removeFromTop(marginHeight * 0.5);

	auto bottomRow = r.removeFromTop(cellHeight);
	g.drawRoundedRectangle(bottomRow.removeFromLeft(adsrW * 4).reduced(margin, 0), cornerSize, lineThickness);
	g.drawRoundedRectangle(bottomRow.removeFromLeft(cellW).reduced(margin, 0), cornerSize, lineThickness);
	g.drawRoundedRectangle(bottomRow.reduced(margin, 0), cornerSize, lineThickness);
}

void BassGeneratorAudioProcessorEditor::resized()
{
	presetBar.setBounds(getLocalBounds().removeFromTop(30));

	if (sliders.isEmpty())
		return;

	const int labelHeight = 20;
	const int marginHeight = 8;
	const int sliderHeight = 70;
	const int headerHeight = 30;

	auto r = getLocalBounds().reduced(5);
	const auto cellW = r.proportionOfWidth(0.2);
	const auto adsrW = cellW * (3.0f / 4.0f);

	r.removeFromTop(headerHeight);

	auto labelTop = r.removeFromTop(labelHeight).withTrimmedTop(10);
	for (int i = 0; i < 5; ++i)
		labels[i]->setBounds(labelTop.removeFromLeft(cellW));

	auto top = r.removeFromTop(sliderHeight);

	getSlider(ParameterIDs::glide)->setBounds(top.removeFromLeft(cellW));
	getSlider(ParameterIDs::bendAmount)->setBounds(top.removeFromLeft(cellW));
	getSlider(ParameterIDs::bendDuration)->setBounds(top.removeFromLeft(cellW));
	getSlider(ParameterIDs::drive)->setBounds(top.removeFromLeft(cellW));
	getSlider(ParameterIDs::driveType)->setBounds(top.removeFromLeft(cellW * 0.5f).withSizeKeepingCentre(cellW * 0.5, top.getHeight() * 0.7f));

	r.removeFromTop(marginHeight);

	auto labelDown = r.removeFromTop(labelHeight).withTrimmedTop(10);
	for (int i = 5; i < 9; ++i)
		labels[i]->setBounds(labelDown.removeFromLeft(adsrW));

	for (int i = 9; i < 11; ++i)
		labels[i]->setBounds(labelDown.removeFromLeft(cellW));

	auto bottom = r.removeFromTop(sliderHeight);
	getSlider(ParameterIDs::attack)->setBounds(bottom.removeFromLeft(adsrW));
	getSlider(ParameterIDs::decay)->setBounds(bottom.removeFromLeft(adsrW));
	getSlider(ParameterIDs::sustain)->setBounds(bottom.removeFromLeft(adsrW));
	getSlider(ParameterIDs::release)->setBounds(bottom.removeFromLeft(adsrW));

	getSlider(ParameterIDs::lpFreq)->setBounds(bottom.removeFromLeft(cellW));
	getSlider(ParameterIDs::master)->setBounds(bottom);

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

	// Create and style label
	auto l = new Label(paramName + "Label", ParameterIDs::getLabel(paramName).toUpperCase());
	addAndMakeVisible(l);
	l->setJustificationType(Justification::centred);
	l->setFont(l->getFont().withHeight(12.0f));

	// Keep pointers
	sliders.add(slider);
	sliderAttachments.add(attachment);
	labels.add(l);
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

	// Create and style label
	auto l = new Label(paramName + "Label", ParameterIDs::getLabel(paramName).toUpperCase());
	addAndMakeVisible(l);
	l->setJustificationType(Justification::centred);
	l->setFont(l->getFont().withHeight(12.0f));

	// Keep pointers
	sliders.add(slider);
	sliderAttachments.add(attachment);
	labels.add(l);
}

Slider * BassGeneratorAudioProcessorEditor::getSlider(String param)
{
	for (auto s : sliders)
		if (s->getName() == param + "Slider")
			return s;

	jassertfalse;

	return nullptr;
}

void BassGeneratorAudioProcessorEditor::drawDriveTypeSymbols(Graphics &g, Rectangle<float> area)
{
	auto slider = getSlider("driveType");
	const auto sliderArea = slider->getBounds().reduced(10);

	// Lines
	const auto lineW = 4;
	const auto lineH = 2;

	g.setColour(Colours::white);
	g.drawRect(sliderArea.getRight() + 2, sliderArea.getY(), lineW, lineH);
	g.drawRect(sliderArea.getRight() + 2, sliderArea.getCentreY() - (int)(lineH * 0.5f), lineW, lineH);
	g.drawRect(sliderArea.getRight() + 2, sliderArea.getBottom() - lineH, lineW, lineH);

	const float symbolW = 15.0f;
	const float symbolH = 8.0f;
	const int symbolLeft = 18;

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
