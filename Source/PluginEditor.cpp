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
BigFootEditor::BigFootEditor (BigFootAudioProcessor& p, AudioProcessorValueTreeState& valueTreeState)
    :	AudioProcessorEditor (&p), 
		processor (p),
		presetBar(p.getValueTreeState(), colors.highlightColour),
		audioMeter(p.getAudioBufferQueue()),
		midiMeter(p.getSmoothedVelocity()),
		resizableBorder(this, nullptr)
{
	SharedResourcePointer<CustomLookAndFeel> lf;
	setLookAndFeel(lf);
	LookAndFeel::setDefaultLookAndFeel(lf);
	auto tf = Typeface::createSystemTypefaceFor(BinaryData::RobotoMedium_ttf, BinaryData::RobotoMedium_ttfSize);
	lf->setDefaultSansSerifTypeface(tf);

	lf->setColourScheme({
		colors.backgroundColour,			// windowBackground
		colors.backgroundColour,			// widgetBackground
		colors.backgroundColour,			// menuBackground
		colors.lineColour,					// outline
		colors.textColour,					// defaultText
		colors.textColour,					// defaultFill
		colors.backgroundColour,			// highlightedText
		colors.highlightColour,				// highlightedFill
		colors.textColour					// menuText
	});

	// We'll also style the logo button here even if it's used in PresetBar
	SharedResourcePointer<LogoButtonLF> logoLF;
	logoLF->setColour(TextButton::textColourOnId, colors.textColour);
	logoLF->setColour(TextButton::textColourOffId, colors.textColour);
	logoLF->setColour(TextButton::buttonColourId, Colours::transparentBlack);
	logoLF->prepareFont();
	
	// Add sliders
	addRotarySlider(valueTreeState, ParameterIDs::glide,		colors.glideSliderColour);
	addRotarySlider(valueTreeState, ParameterIDs::bendAmount,	colors.bendSliderColour);
	addRotarySlider(valueTreeState, ParameterIDs::bendDuration, colors.bendSliderColour);
	addRotarySlider(valueTreeState, ParameterIDs::drive,		colors.driveSliderColour);
	addLinearSlider(valueTreeState, ParameterIDs::driveType,	true);
	addRotarySlider(valueTreeState, ParameterIDs::attack,		colors.adsrSliderColour);
	addRotarySlider(valueTreeState, ParameterIDs::decay,		colors.adsrSliderColour);
	addRotarySlider(valueTreeState, ParameterIDs::sustain,		colors.adsrSliderColour);
	addRotarySlider(valueTreeState, ParameterIDs::release,		colors.adsrSliderColour);
	addRotarySlider(valueTreeState, ParameterIDs::lpFreq,		colors.filterSliderColour);
	addRotarySlider(valueTreeState, ParameterIDs::lpReso,		colors.filterSliderColour);
	addRotarySlider(valueTreeState, ParameterIDs::master,       colors.masterSliderColour);

	addAndMakeVisible(presetBar);

    // Set editor size
	const int width = 430;
	int height = 255;

#if BIGFOOT_DYNAMIC_ROUTING
	height += footerHeight;
#endif

	setSize(width, height);

	auto numPoints = 12;

	auto offColour = colors.backgroundColour.interpolatedWith(colors.lineColour, 0.4f);

	addAndMakeVisible(audioMeter);
	audioMeter.setOnColour(colors.highlightColour);
	audioMeter.setOffColour(offColour);
	audioMeter.setNumPoints(numPoints);

	addAndMakeVisible(midiMeter);
	midiMeter.setOnColour(colors.highlightColour);
	midiMeter.setOffColour(offColour);
	midiMeter.setNumPoints(numPoints);

	// Add resizable border
	//addAndMakeVisible(resizableBorder);

#if BIGFOOT_DYNAMIC_ROUTING
	// Add audio chain comp
	addAndMakeVisible(audioChainComp);
	audioChainComp.addBlock(SineWaveVoice::oscIndex, "osc", false);
	audioChainComp.addBlock(SineWaveVoice::envelopeIndex, "env", true);
	audioChainComp.addBlock(SineWaveVoice::distortionIndex, "drive", true);
	audioChainComp.addBlock(SineWaveVoice::filterIndex, "lpf", true);
	audioChainComp.addBlock(SineWaveVoice::masterGainIndex, "out", false);

	audioChainComp.setColour(AudioChainComponent::blockOutlineColourId, colors.lineColour.darker());
	audioChainComp.setColour(AudioChainComponent::cableColourId, colors.lineColour);
	audioChainComp.setColour(AudioChainComponent::blockTextColourId, colors.textColour);
	audioChainComp.setColour(AudioChainComponent::hoverTextColourId, colors.backgroundColour);
	audioChainComp.setColour(AudioChainComponent::blockBackgroundColourId, colors.backgroundColour.darker(0.05f));
	audioChainComp.setColour(AudioChainComponent::hoverBackgroundColourId, colors.highlightColour);

	audioChainComp.changed = [this] { audioChainChanged(); };
#endif //BIGFOOT_DYNAMIC_ROUTING
}

BigFootEditor::~BigFootEditor()
{
}

//==============================================================================
void BigFootEditor::paint(Graphics& g)
{
	g.drawImage(*backgroundImage, getLocalBounds().toFloat());
}

void BigFootEditor::renderBackgroundImage(Graphics& g)
{
	// Handy lambda for drawing a section
	auto drawSection = [this](Rectangle<float> bounds, Graphics& g)
	{
		auto corner = 4.0f;
		auto stroke = 0.3f;

		g.setColour(colors.backgroundColour);
		g.fillRoundedRectangle(bounds, corner);

		g.setColour(Colours::black);
		g.drawRoundedRectangle(bounds, corner, stroke);
	};

	g.fillAll (getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(0.8f));

	auto texture = ImageCache::getFromMemory(BinaryData::brushed_metal_texture_jpg,
											 BinaryData::brushed_metal_texture_jpgSize);

	g.setOpacity(0.16f);
	g.drawImageWithin(texture, 0, 0, getWidth(), getHeight(), RectanglePlacement::fillDestination);

	const int cellHeight = sliderHeight + labelHeight + int(marginHeight * 0.5);

	auto r = sliderZone.toFloat();
	const auto cellW = int((float)r.getWidth() / 6.0f);

	g.setColour(colors.backgroundColour);
	
	auto margin = 2;
	auto topRow = r.removeFromTop(cellHeight);

	Array<int> topCells;
	topCells.add(2);
	topCells.add(4);

	for (auto c : topCells)
	{
		auto b = topRow.removeFromLeft(c * cellW).reduced(margin, 0);
		drawSection(b, g);
	}

	Array<int> bottomCells;
	bottomCells.add(1);
	bottomCells.add(2);
	bottomCells.add(2);
	bottomCells.add(1);

	r.removeFromTop(marginHeight * 0.5);
	auto bottomRow = r.removeFromTop(cellHeight);

	for (auto c : bottomCells)
	{
		auto b = bottomRow.removeFromLeft(c * cellW).reduced(margin, 0);
		drawSection(b, g);
	}

	drawDriveTypeSymbols(g);

#if BIGFOOT_DYNAMIC_ROUTING
	// footer
	auto footer = getLocalBounds().removeFromBottom(footerHeight).withSizeKeepingCentre(sliderZone.getWidth(), 30).translated(0, -4).toFloat();
	drawSection(footer.removeFromLeft(audioChainWidth).reduced(margin, 0), g);
	//drawSection(footer.reduced(margin, 0), g);
#endif
}

void BigFootEditor::resized()
{
	// Adjust slider zone to new size
	sliderZone = getLocalBounds().withTrimmedTop(headerHeight)
#if BIGFOOT_DYNAMIC_ROUTING
								 .withTrimmedBottom(footerHeight)
#endif
								 .reduced(20, 8);

	// Deduce slider height
	sliderHeight = sliderZone.getHeight() - ((2 * labelHeight) + marginHeight);
	sliderHeight *= 0.5f;

	// Position preset bar
	presetBar.setBounds(getLocalBounds().removeFromTop(30));

	if (sliders.isEmpty() || sliderZone.isEmpty())
		return;
	
	// Layout sliders
	auto r = sliderZone.translated(0, -4);

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
				b = b.withTrimmedRight(b.proportionOfWidth(0.5f)).withSizeKeepingCentre(b.proportionOfWidth(0.35f), b.proportionOfHeight(0.62f));

			getSlider(paramId)->setBounds(b);

		}
	};

	StringArray topRow;
	topRow.add(ParameterIDs::bendAmount);
	topRow.add(ParameterIDs::bendDuration);
	topRow.add(ParameterIDs::attack);
	topRow.add(ParameterIDs::decay);
	topRow.add(ParameterIDs::sustain);
	topRow.add(ParameterIDs::release);

	layoutLabels(topRow, r.removeFromTop(labelHeight));
	layoutSliders(topRow, r.removeFromTop(sliderHeight));

	r.removeFromTop(marginHeight);

	StringArray bottomRow;
	bottomRow.add(ParameterIDs::glide);
	bottomRow.add(ParameterIDs::drive);
	bottomRow.add(ParameterIDs::driveType);
	bottomRow.add(ParameterIDs::lpFreq);
	bottomRow.add(ParameterIDs::lpReso);
	bottomRow.add(ParameterIDs::master);

	layoutLabels(bottomRow, r.removeFromTop(labelHeight));
	layoutSliders(bottomRow, r.removeFromTop(sliderHeight));

	int meterHeight = 100;
	int meterWidth = 10;

	auto topless = getLocalBounds().withTrimmedTop(headerHeight);

	audioMeter.setBounds(topless.removeFromRight(24).withSizeKeepingCentre(meterWidth, meterHeight));
	midiMeter.setBounds(topless.removeFromLeft(24).withSizeKeepingCentre(meterWidth, meterHeight));

	// Reset and draw background image
	backgroundImage.reset(new Image(Image::PixelFormat::ARGB, getWidth(), getHeight(), false));

	Graphics g(*backgroundImage);
	renderBackgroundImage(g);

	// Adjust resizable border
	resizableBorder.setBounds(getLocalBounds());


#if BIGFOOT_DYNAMIC_ROUTING
	// Position audio chain comp
	auto footer = getLocalBounds().removeFromBottom(footerHeight).withSizeKeepingCentre(sliderZone.getWidth(), 30).translated(0, -4);
	audioChainComp.setBounds(footer.removeFromLeft(audioChainWidth).reduced(4));
#endif
}

void BigFootEditor::addLinearSlider(AudioProcessorValueTreeState & vts, String paramName, bool reversed)
{
	// Create slider and attachment
	auto slider = reversed ? new ReversedSlider(paramName + "Slider") : new Slider(paramName + "Slider");
	auto attachment = new SliderAttachment(vts, paramName, *slider);

	// Styling
	auto bg = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
	auto txt = getLookAndFeel().findColour(Label::textColourId);

	auto trackColour = bg.interpolatedWith(txt, 0.3f);

	addAndMakeVisible(slider);
	slider->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
	slider->setSliderStyle(Slider::SliderStyle::LinearVertical);
	slider->setColour(Slider::backgroundColourId, trackColour);
	slider->setColour(Slider::textBoxHighlightColourId, colors.highlightColour);
	slider->onValueChange = [slider] { slider->setTooltip(slider->getTextFromValue(slider->getValue())); };

	// Create and style label
	auto l = new Label(paramName + "Label", ParameterIDs::getLabel(paramName));
	addAndMakeVisible(l);
	l->setJustificationType(Justification::centred);
	l->setFont(l->getFont().withHeight(13.0f));
	l->setColour(Label::textColourId, colors.textColour);

	// Keep pointers
	sliders.add(new AttachedSlider(paramName, slider, attachment, l));
}

void BigFootEditor::addRotarySlider(AudioProcessorValueTreeState& vts, String paramName, Colour colour)
{
	// Create slider and attachment
	auto slider = new Slider(paramName + "Slider");
	auto attachment = new SliderAttachment(vts, paramName, *slider);

	auto bg = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
	auto outline = bg.interpolatedWith(colour, 0.3f);

	// Styling
	addAndMakeVisible(slider);
	slider->setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	//slider->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
	slider->setTextBoxStyle(Slider::TextBoxBelow, false, 54, 16);
	slider->setColour(Slider::rotarySliderFillColourId, colour);
	slider->setColour(Slider::textBoxOutlineColourId, colors.lineColour.withAlpha(0.2f));
	slider->setColour(Slider::textBoxBackgroundColourId, colors.backgroundColour.withAlpha(0.0f));
	//slider->setColour(Slider::textBoxBackgroundColourId, Colours::black.withAlpha(0.2f));
	slider->setColour(Slider::textBoxHighlightColourId, colors.highlightColour);
	slider->setColour(Slider::rotarySliderOutlineColourId, outline);
	slider->onValueChange = [slider] { slider->setTooltip(slider->getTextFromValue(slider->getValue())); };

	// Create and style label
	auto l = new Label(paramName + "Label", ParameterIDs::getLabel(paramName));
	addAndMakeVisible(l);
	l->setJustificationType(Justification::centred);
	l->setFont(l->getFont().withHeight(13.0f));
	l->setColour(Label::textColourId, colors.textColour);

	// Keep pointers
	sliders.add(new AttachedSlider(paramName, slider, attachment, l));
}

Slider * BigFootEditor::getSlider(String param)
{
	for (auto s : sliders)
		if (s->paramId == param)
			return s->slider.get();

	jassertfalse;

	return nullptr;
}

Label * BigFootEditor::getLabel(String param)
{
	for (auto s : sliders)
		if (s->paramId == param)
			return s->label.get();

	jassertfalse;

	return nullptr;
}

void BigFootEditor::drawDriveTypeSymbols(Graphics &g)
{
	auto slider = getSlider("driveType");
	const auto sliderArea = slider->getBounds().reduced(10, 6);

	// Lines
	const auto lineW = 4;
	const auto lineH = 2;

	g.setColour(colors.textColour);
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

#if BIGFOOT_DYNAMIC_ROUTING
void BigFootEditor::audioChainChanged()
{
	auto layout = audioChainComp.getLayout();

	jassert(layout.size() == 5);

	layout.remove(0);
	layout.removeLast();

	processor.setFXLayout(layout);
}
#endif