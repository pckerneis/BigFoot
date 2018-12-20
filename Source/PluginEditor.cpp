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
	
	const float textBoxWidth = 60;
	const float adsrTextBoxWidth = 50;
	const float textBoxHeight = 18;

	const auto textBoxStyle = Slider::NoTextBox; // Slider::TextBoxBelow;

	// Keyboard comp
#if PAWG_USE_MIDI_KEYBOARD
	addAndMakeVisible (keyboardComponent);
	keyboardComponent.setLowestVisibleKey(12);
#endif

	// Get default param values
	auto defaultValues = p.getDefaultParameterValues();

	// Drive sliders
	addAndMakeVisible(driveSlider);
	driveSlider.setRange(Range<double>(0.0f, 1.0f), 0.001f);
	driveSlider.onValueChange = [this]() 
	{
		processor.setDrive(driveSlider.getValue());
	};
	driveSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	driveSlider.setTextBoxStyle(textBoxStyle, false, textBoxWidth, textBoxHeight);
	driveSlider.setColour(Slider::rotarySliderFillColourId, Colours::orangered);
	driveAttachment.reset(new SliderAttachment(valueTreeState, "drive", driveSlider));

	addAndMakeVisible(driveTypeSlider);
	driveTypeSlider.setRange(Range<double>(0.0f, 2.0f), 1.0f);
	driveTypeSlider.onValueChange = [this]() 
	{ 
		int v = (int)driveTypeSlider.getValue();

		if (v == 0)			processor.setDriveType(Distortion<float>::TransferFunction::softType);
		else if (v == 1)	processor.setDriveType(Distortion<float>::TransferFunction::hardType);
		else				processor.setDriveType(Distortion<float>::TransferFunction::sinType);
	};
	driveTypeSlider.setSliderStyle(Slider::SliderStyle::LinearVertical);
	driveTypeSlider.setColour(Slider::backgroundColourId, Colours::grey);
	driveTypeSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
	driveTypeAttachment.reset(new SliderAttachment(valueTreeState, "driveType", driveTypeSlider));

	// Bend sliders
	addAndMakeVisible(bendAmountSlider);
	bendAmountSlider.setRange(Range<double>(defaultValues.minBendAmount, defaultValues.maxBendAmount), 0.001f);
	bendAmountSlider.onValueChange = [this]() 
	{ 
		processor.setBendAmount (bendAmountSlider.getValue());
	};
	bendAmountSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	bendAmountSlider.setTextBoxStyle(textBoxStyle, false, textBoxWidth, textBoxHeight);
	bendAmountSlider.setColour(Slider::rotarySliderFillColourId, Colours::orange);
	bendAmountAttachment.reset(new SliderAttachment(valueTreeState, "bendAmount", bendAmountSlider));

	addAndMakeVisible(bendDurationSlider);
	bendDurationSlider.setRange(Range<double>(defaultValues.minBendDuration, defaultValues.maxBendDuration), 0.001f);
	bendDurationSlider.onValueChange = [this]() 
	{ 
		processor.setBendDuration (bendDurationSlider.getValue());
	};
	bendDurationSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	bendDurationSlider.setTextBoxStyle(textBoxStyle, false, textBoxWidth, textBoxHeight);
	bendDurationSlider.setColour(Slider::rotarySliderFillColourId, Colours::orange);
	bendDurationAttachment.reset(new SliderAttachment(valueTreeState, "bendDuration", bendDurationSlider));

	// Brightness slider
	addAndMakeVisible(brightnessSlider);
	brightnessSlider.setRange(Range<double>(defaultValues.minBrightness, defaultValues.maxBrightness), 0.01f);
	brightnessSlider.onValueChange = [this]() 
	{ 
		processor.setBrightness(brightnessSlider.getValue());
	};
	brightnessSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	brightnessSlider.setTextBoxStyle(textBoxStyle, false, textBoxWidth, textBoxHeight);
	brightnessSlider.setColour(Slider::rotarySliderFillColourId, Colours::red.withMultipliedSaturation(0.9f));
	brightnessAttachment.reset(new SliderAttachment(valueTreeState, "brightness", brightnessSlider));
	
	// ADSR sliders
	addAndMakeVisible(attackSlider);
	attackSlider.setRange(Range<double>(defaultValues.minAttack, defaultValues.maxAttack), 0.001f);
	attackSlider.onValueChange = [this]() 
	{ 
		processor.setAttack(attackSlider.getValue());
	};
	attackSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	attackSlider.setTextBoxStyle(textBoxStyle, false, adsrTextBoxWidth, textBoxHeight);
	attackSlider.setColour(Slider::rotarySliderFillColourId, Colours::beige);
	attackAttachment.reset(new SliderAttachment(valueTreeState, "attack", attackSlider));

	addAndMakeVisible(decaySlider);
	decaySlider.setRange(Range<double>(defaultValues.minDecay, defaultValues.maxDecay), 0.001f);
	decaySlider.onValueChange = [this]() 
	{
		processor.setDecay(decaySlider.getValue());
	};
	decaySlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	decaySlider.setTextBoxStyle(textBoxStyle, false, adsrTextBoxWidth, textBoxHeight);
	decaySlider.setColour(Slider::rotarySliderFillColourId, Colours::beige);
	decayAttachment.reset(new SliderAttachment(valueTreeState, "decay", decaySlider));

	addAndMakeVisible(sustainSlider);
	sustainSlider.setRange(Range<double>(0, 1), 0.001f);
	sustainSlider.onValueChange = [this]() 
	{
		processor.setSustain(sustainSlider.getValue());
	};
	sustainSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	sustainSlider.setTextBoxStyle(textBoxStyle, false, adsrTextBoxWidth, textBoxHeight);
	sustainSlider.setColour(Slider::rotarySliderFillColourId, Colours::beige);
	sustainAttachment.reset(new SliderAttachment(valueTreeState, "sustain", sustainSlider));

	addAndMakeVisible(releaseSlider);
	releaseSlider.setRange(Range<double>(defaultValues.minRelease, defaultValues.maxRelease), 0.001f);
	releaseSlider.onValueChange = [this]() 
	{ 
		processor.setRelease(releaseSlider.getValue());
	};
	releaseSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	releaseSlider.setTextBoxStyle(textBoxStyle, false, adsrTextBoxWidth, textBoxHeight);
	releaseSlider.setColour(Slider::rotarySliderFillColourId, Colours::beige);
	releaseAttachment.reset(new SliderAttachment(valueTreeState, "release", releaseSlider));

	// Glide slider
	addAndMakeVisible(glideSlider);
	glideSlider.setRange(Range<double>(0.0f, defaultValues.maxGlide), 0.01f);
	glideSlider.onValueChange = [this]()
	{ 
		processor.setGlide(glideSlider.getValue());
	};
	glideSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	glideSlider.setTextBoxStyle(textBoxStyle, false, textBoxWidth, textBoxHeight);
	glideSlider.setColour(Slider::rotarySliderFillColourId, Colours::gold);
	glideAttachment.reset(new SliderAttachment(valueTreeState, "glide", glideSlider));

	// Master slider
	addAndMakeVisible(masterSlider);
	masterSlider.onValueChange = [this]() 
	{
		processor.setOutputGain(masterSlider.getValue());
	};
	masterSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	masterSlider.setTextBoxStyle(textBoxStyle, false, textBoxWidth, textBoxHeight);
	masterSlider.setColour(Slider::rotarySliderFillColourId, Colours::beige);
	masterAttachment.reset(new SliderAttachment(valueTreeState, "master", masterSlider));

	StringArray labelTexts;
	labelTexts.add("Glide");
	labelTexts.add("Bend");
	labelTexts.add("Time");
	labelTexts.add("Drive");
	labelTexts.add("Type");
	labelTexts.add("Attack");
	labelTexts.add("Decay");
	labelTexts.add("Sustain");
	labelTexts.add("Release");
	labelTexts.add("Low-pass");
	labelTexts.add("Output");

	for (auto t : labelTexts)
	{
		auto l = new Label(t + "Label", t.toUpperCase());
		l->setJustificationType(Justification::centred);
		l->setFont(l->getFont().withHeight(12.0f));
		addAndMakeVisible(l);
		labels.add(l);
	}

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
#else
	setSize(400, 230);
#endif
	startTimer(400);
}

BassGeneratorAudioProcessorEditor::~BassGeneratorAudioProcessorEditor()
{
}

//==============================================================================
void BassGeneratorAudioProcessorEditor::paint (Graphics& g)
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

	auto header = r.removeFromTop(headerHeight);
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

	const int labelHeight = 20;
	const int marginHeight = 8;
	const int sliderHeight = 70;
	const int headerHeight = 30;

	auto r = getLocalBounds().reduced(5);
	const auto cellW = r.proportionOfWidth(0.2);
	const auto adsrW = cellW * (3.0f / 4.0f);

	auto header = r.removeFromTop(headerHeight);


	auto labelTop = r.removeFromTop(labelHeight).withTrimmedTop(10);
	for (int i = 0; i < 5; ++i)
		labels[i]->setBounds(labelTop.removeFromLeft(cellW));

	auto top = r.removeFromTop(sliderHeight);

	glideSlider.setBounds(top.removeFromLeft(cellW));
	bendAmountSlider.setBounds(top.removeFromLeft(cellW));
	bendDurationSlider.setBounds(top.removeFromLeft(cellW));
	driveSlider.setBounds(top.removeFromLeft(cellW));
	driveTypeSlider.setBounds(top.removeFromLeft(cellW * 0.5f).withSizeKeepingCentre(cellW * 0.5, top.getHeight() * 0.7f));

	r.removeFromTop(marginHeight);

	auto labelDown = r.removeFromTop(labelHeight).withTrimmedTop(10);
	for (int i = 5; i < 9; ++i)
		labels[i]->setBounds(labelDown.removeFromLeft(adsrW));

	for (int i = 9; i < 11; ++i)
		labels[i]->setBounds(labelDown.removeFromLeft(cellW));

	auto bottom = r.removeFromTop(sliderHeight);
	attackSlider.setBounds(bottom.removeFromLeft(adsrW));
	decaySlider.setBounds(bottom.removeFromLeft(adsrW));
	sustainSlider.setBounds(bottom.removeFromLeft(adsrW));
	releaseSlider.setBounds(bottom.removeFromLeft(adsrW));

	brightnessSlider.setBounds(bottom.removeFromLeft(cellW));
	masterSlider.setBounds(bottom);

#if PAWG_USE_MIDI_KEYBOARD
	r.removeFromTop(marginHeight);
	keyboardComponent.setBounds(r);
#endif
}

void BassGeneratorAudioProcessorEditor::drawDriveTypeSymbols(Graphics &g, Rectangle<float> area)
{
	const auto sliderArea = driveTypeSlider.getBounds().reduced(10);

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
