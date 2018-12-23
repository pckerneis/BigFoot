/*
  ==============================================================================

    LookAndFeel.h
    Created: 18 Dec 2018 2:57:20am
    Author:  Pierre

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
class CustomLookAndFeel : public LookAndFeel_V4
{
	void drawComboBox(Graphics& g, int width, int height, bool,
		int, int, int, int, ComboBox& box)
	{
		auto cornerSize = 0.0f;
		Rectangle<int> boxBounds(0, 0, width, height);

		g.setColour(box.findColour(ComboBox::backgroundColourId));
		g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

		g.setColour(box.findColour(ComboBox::outlineColourId));
		g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.0f);

		Rectangle<int> arrowZone(width - 30, 0, 20, height);
		Path path;
		path.startNewSubPath(arrowZone.getX() + 3.0f, arrowZone.getCentreY() - 2.0f);
		path.lineTo(static_cast<float> (arrowZone.getCentreX()), arrowZone.getCentreY() + 3.0f);
		path.lineTo(arrowZone.getRight() - 3.0f, arrowZone.getCentreY() - 2.0f);

		g.setColour(box.findColour(ComboBox::arrowColourId).withAlpha((box.isEnabled() ? 0.9f : 0.2f)));
		g.strokePath(path, PathStrokeType(2.0f));
	}

	Font getComboBoxFont(ComboBox& box)
	{
		Font f(jmin(15.0f, box.getHeight() * 0.85f));
		f.setHorizontalScale(0.9f);

		// (REALLY) DIRTY WAY TO HANDLE TEMP VALUES
		if (box.findColour(ComboBox::textColourId) != Colours::white)
			f = f.italicised();

		return f;
	}

	void drawButtonBackground(Graphics& g,
		Button& button,
		const Colour& backgroundColour,
		bool shouldDrawButtonAsHighlighted,
		bool shouldDrawButtonAsDown)
	{
		auto cornerSize = 0.0f;
		auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

		auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
			.withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

		if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
			baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.1f : 0.05f);

		g.setColour(baseColour);

		if (button.isConnectedOnLeft() || button.isConnectedOnRight())
		{
			Path path;
			path.addRoundedRectangle(bounds.getX(), bounds.getY(),
				bounds.getWidth(), bounds.getHeight(),
				cornerSize, cornerSize,
				!button.isConnectedOnLeft(),
				!button.isConnectedOnRight(),
				!button.isConnectedOnLeft(),
				!button.isConnectedOnRight());

			g.fillPath(path);

			g.setColour(button.findColour(ComboBox::outlineColourId));
			g.strokePath(path, PathStrokeType(1.0f));
		}
		else
		{
			g.fillRoundedRectangle(bounds, cornerSize);

			g.setColour(button.findColour(ComboBox::outlineColourId));
			g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
		}
	}

	void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
		const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
	{
		auto outline = slider.findColour(Slider::rotarySliderOutlineColourId);
		auto fill = slider.findColour(Slider::rotarySliderFillColourId);

		auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(10);
		auto size = jmin(bounds.getWidth(), bounds.getHeight());
		bounds = bounds.withSizeKeepingCentre(size, size);

		auto gradRadius = size / 2.0f;
		auto radius = size / 3.0f;
		auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
		auto lineW = jmin(8.0f, radius * 0.5f);

		// graduations
		int numGrad = 10;

		for (int i = 0; i < numGrad + 1; ++i)
		{
			float gradWidth;

			if (slider.getName() == "bendAmountSlider" || slider.getName() == "masterSlider")
			{
				if (i == 5)
					gradWidth = lineW * 0.5f;
				else
					gradWidth = lineW * 0.3f;
			}
			else if (i == 0 || i == 10)
				gradWidth = lineW * 0.5f;
			else
				gradWidth = lineW * 0.3f;

			auto thumbWidth = lineW;
			auto angle = rotaryStartAngle + ((float)i / (float)numGrad) * (rotaryEndAngle - rotaryStartAngle);

			Point<float> thumbPoint(bounds.getCentreX() + gradRadius * std::cos(angle - MathConstants<float>::halfPi),
				bounds.getCentreY() + gradRadius * std::sin(angle - MathConstants<float>::halfPi));

			g.setColour(Colours::white);
			g.fillEllipse(Rectangle<float>(gradWidth, gradWidth).withCentre(thumbPoint));
		}

		// knob
		auto knobBounds = bounds.reduced(size * 0.15f);
		g.setColour(fill);
		g.fillEllipse(knobBounds);
		g.setColour(outline.interpolatedWith(fill, 0.7f));
		g.drawEllipse(knobBounds, 2.2f);

		// thumb
		auto thumbDist = radius * 0.77f;
		auto thumbWidth = lineW * 0.7f;
		Point<float> thumbPoint(bounds.getCentreX() + thumbDist * std::cos(toAngle - MathConstants<float>::halfPi),
			bounds.getCentreY() + thumbDist * std::sin(toAngle - MathConstants<float>::halfPi));

		g.setColour(slider.findColour(Slider::thumbColourId).interpolatedWith(fill, 0.3f));
		g.fillEllipse(Rectangle<float>(thumbWidth, thumbWidth).withCentre(thumbPoint));
	}

	void drawLinearSlider(Graphics& g, int x, int y, int width, int height,
		float sliderPos,
		float minSliderPos,
		float maxSliderPos,
		const Slider::SliderStyle style, Slider& slider)
	{
		if (slider.isBar())
		{
			g.setColour(slider.findColour(Slider::trackColourId));
			g.fillRect(slider.isHorizontal() ? Rectangle<float>(static_cast<float> (x), y + 0.5f, sliderPos - x, height - 1.0f)
				: Rectangle<float>(x + 0.5f, sliderPos, width - 1.0f, y + (height - sliderPos)));
		}
		else
		{
			auto isTwoVal = (style == Slider::SliderStyle::TwoValueVertical || style == Slider::SliderStyle::TwoValueHorizontal);
			auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);

			//auto trackWidth = jmin(6.0f, slider.isHorizontal() ? height * 0.25f : width * 0.25f);
			float trackWidth = getSliderThumbRadius(slider) * 1.18f;

			Point<float> startPoint(slider.isHorizontal() ? x : x + width * 0.5f,
				slider.isHorizontal() ? y + height * 0.5f : height + y);

			Point<float> endPoint(slider.isHorizontal() ? width + x : startPoint.x,
				slider.isHorizontal() ? startPoint.y : y);

			Path backgroundTrack;
			backgroundTrack.startNewSubPath(startPoint);
			backgroundTrack.lineTo(endPoint);
			g.setColour(slider.findColour(Slider::backgroundColourId));
			g.strokePath(backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

			Path valueTrack;
			Point<float> minPoint, maxPoint, thumbPoint;

			if (isTwoVal || isThreeVal)
			{
				minPoint = { slider.isHorizontal() ? minSliderPos : width * 0.5f,
					slider.isHorizontal() ? height * 0.5f : minSliderPos };

				if (isThreeVal)
					thumbPoint = { slider.isHorizontal() ? sliderPos : width * 0.5f,
					slider.isHorizontal() ? height * 0.5f : sliderPos };

				maxPoint = { slider.isHorizontal() ? maxSliderPos : width * 0.5f,
					slider.isHorizontal() ? height * 0.5f : maxSliderPos };
			}
			else
			{
				auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
				auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;

				minPoint = startPoint;
				maxPoint = { kx, ky };
			}

			auto thumbWidth = getSliderThumbRadius(slider);

			if (!isTwoVal)
			{
				g.setColour(slider.findColour(Slider::thumbColourId));
				g.fillEllipse(Rectangle<float>(static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre(isThreeVal ? thumbPoint : maxPoint));
			}

			if (isTwoVal || isThreeVal)
			{
				auto sr = jmin(trackWidth, (slider.isHorizontal() ? height : width) * 0.4f);
				auto pointerColour = slider.findColour(Slider::thumbColourId);

				if (slider.isHorizontal())
				{
					drawPointer(g, minSliderPos - sr,
						jmax(0.0f, y + height * 0.5f - trackWidth * 2.0f),
						trackWidth * 2.0f, pointerColour, 2);

					drawPointer(g, maxSliderPos - trackWidth,
						jmin(y + height - trackWidth * 2.0f, y + height * 0.5f),
						trackWidth * 2.0f, pointerColour, 4);
				}
				else
				{
					drawPointer(g, jmax(0.0f, x + width * 0.5f - trackWidth * 2.0f),
						minSliderPos - trackWidth,
						trackWidth * 2.0f, pointerColour, 1);

					drawPointer(g, jmin(x + width - trackWidth * 2.0f, x + width * 0.5f), maxSliderPos - sr,
						trackWidth * 2.0f, pointerColour, 3);
				}
			}
		}
	}
};

//==============================================================================
class LogoButtonLF : public CustomLookAndFeel
{
	void drawButtonBackground(Graphics& g, Button& button, const Colour& backgroundColour, bool, bool)
	{
		auto cornerSize = 0.0f;
		auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

		auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
			.withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

		g.setColour(baseColour);
		g.fillRoundedRectangle(bounds, cornerSize);

		g.setColour(button.findColour(ComboBox::outlineColourId));
		g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
	}
};