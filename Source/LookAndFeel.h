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
	static TextLayout layoutTooltipText(const String& text, Colour colour) noexcept
	{
		const float tooltipFontSize = 13.0f;
		const int maxToolTipWidth = 400;

		AttributedString s;
		s.setJustification(Justification::centred);
		s.append(text, Font(tooltipFontSize, Font::bold), colour);

		TextLayout tl;
		tl.createLayoutWithBalancedLineLengths(s, (float)maxToolTipWidth);
		return tl;
	}
	
	void drawTooltip(Graphics& g, const String& text, int width, int height) override
	{
		Rectangle<int> bounds(width, height);
		auto cornerSize = 0.0f;

		g.setColour(findColour(TooltipWindow::backgroundColourId));
		g.fillRoundedRectangle(bounds.toFloat(), cornerSize);

		g.setColour(findColour(TooltipWindow::outlineColourId));
		g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.0f);

		layoutTooltipText(text, findColour(TooltipWindow::textColourId))
			.draw(g, { static_cast<float> (width), static_cast<float> (height) });
	}

	int getAlertWindowButtonHeight() override
	{
		return 22;
	}

	void drawAlertBox(Graphics& g, AlertWindow& alert,
		const Rectangle<int>& textArea, TextLayout& textLayout) override
	{
		g.setColour(alert.findColour(AlertWindow::outlineColourId));
		g.drawRect(alert.getLocalBounds().toFloat());

		auto bounds = alert.getLocalBounds().reduced(1);
		g.reduceClipRegion(bounds);

		g.setColour(alert.findColour(AlertWindow::backgroundColourId));
		g.fillRect(bounds.toFloat());

		auto iconSpaceUsed = 0;

		auto iconWidth = 80;
		auto iconSize = jmin(iconWidth + 50, bounds.getHeight() + 20);

		if (alert.containsAnyExtraComponents() || alert.getNumButtons() > 2)
			iconSize = jmin(iconSize, textArea.getHeight() + 50);

		Rectangle<int> iconRect(iconSize / -10, iconSize / -10,
			iconSize, iconSize);

		if (alert.getAlertType() != AlertWindow::NoIcon)
		{
			Path icon;
			char character;
			uint32 colour;

			if (alert.getAlertType() == AlertWindow::WarningIcon)
			{
				character = '!';

				icon.addTriangle(iconRect.getX() + iconRect.getWidth() * 0.5f, (float)iconRect.getY(),
					static_cast<float> (iconRect.getRight()), static_cast<float> (iconRect.getBottom()),
					static_cast<float> (iconRect.getX()), static_cast<float> (iconRect.getBottom()));

				icon = icon.createPathWithRoundedCorners(5.0f);
				colour = 0x66ff2a00;
			}
			else
			{
				colour = Colour(0xff00b0b9).withAlpha(0.4f).getARGB();
				character = alert.getAlertType() == AlertWindow::InfoIcon ? 'i' : '?';

				icon.addEllipse(iconRect.toFloat());
			}

			GlyphArrangement ga;
			ga.addFittedText({ iconRect.getHeight() * 0.9f, Font::bold },
				String::charToString((juce_wchar)(uint8)character),
				static_cast<float> (iconRect.getX()), static_cast<float> (iconRect.getY()),
				static_cast<float> (iconRect.getWidth()), static_cast<float> (iconRect.getHeight()),
				Justification::centred, false);
			ga.createPath(icon);

			icon.setUsingNonZeroWinding(false);
			g.setColour(Colour(colour));
			g.fillPath(icon);

			iconSpaceUsed = iconWidth;
		}

		g.setColour(alert.findColour(AlertWindow::textColourId));

		Rectangle<int> alertBounds(bounds.getX() + iconSpaceUsed, 30,
			bounds.getWidth(), bounds.getHeight() - getAlertWindowButtonHeight() - 20);

		textLayout.draw(g, alertBounds.toFloat());
	}

	void positionComboBoxText(ComboBox& box, Label& label) override
	{
		label.setBounds(1, 1, box.getWidth() - 22, box.getHeight() - 2);
		label.setFont(getComboBoxFont(box));
	}

	void drawComboBox(Graphics& g, int width, int height, bool,
		int, int, int, int, ComboBox& box) override
	{
		auto cornerSize = 0.0f;
		Rectangle<int> boxBounds(0, 0, width, height);

		g.setColour(box.findColour(ComboBox::backgroundColourId));
		g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

		g.setColour(findColour(ComboBox::outlineColourId));
		//g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.0f);
		g.drawRect(boxBounds.toFloat(), 1.0f);

		Rectangle<int> arrowZone(width - 20, 0, 16, height);
		Path path;
		path.startNewSubPath(arrowZone.getX() + 3.0f, arrowZone.getCentreY() - 2.0f);
		path.lineTo(static_cast<float> (arrowZone.getCentreX()), arrowZone.getCentreY() + 3.0f);
		path.lineTo(arrowZone.getRight() - 3.0f, arrowZone.getCentreY() - 2.0f);

		g.setColour(box.findColour(ComboBox::arrowColourId).withAlpha((box.isEnabled() ? 0.9f : 0.2f)));
		g.strokePath(path, PathStrokeType(1.6f));
	}

	Font getComboBoxFont(ComboBox& box) override
	{
		Font f(jmin(15.0f, box.getHeight() * 0.85f));
		//f.setHorizontalScale(0.9f);

		// (REALLY) DIRTY WAY TO HANDLE TEMP VALUES
		//if (box.findColour(ComboBox::textColourId) != Colours::white)
		//	f = f.italicised();

		return f;
	}

	void drawButtonBackground(Graphics& g,
		Button& button,
		const Colour& backgroundColour,
		bool shouldDrawButtonAsHighlighted,
		bool shouldDrawButtonAsDown) override
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
		const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override
	{
		auto highlight = slider.findColour(Slider::textBoxHighlightColourId);
		auto outline = slider.findColour(ComboBox::outlineColourId);
		auto fill = slider.findColour(Slider::rotarySliderFillColourId);

		auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(10);
		auto size = jmin(bounds.getWidth(), bounds.getHeight());
		bounds = bounds.withSizeKeepingCentre(size, size);

		auto gradRadius = size / 2.0f;
		auto radius = size / 2.6f;
		auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
		auto lineW = jmin(8.0f, radius * 0.5f);

		// graduations
		int numGrad = 10;

		for (int i = 0; i < numGrad + 1; ++i)
		{
			float gradWidth;

			// These sliders have their bigger graduation at mid range (position for 0)
			if (slider.getName() == "bendAmountSlider" || slider.getName() == "masterSlider")
			{
				if (i == 5)
					gradWidth = lineW * 0.5f;
				else
					gradWidth = lineW * 0.3f;
			}
			else if (i == 0 || i == 10)	// Other sliders have a bigger graduation for min and max positions
				gradWidth = lineW * 0.5f;
			else
				gradWidth = lineW * 0.3f;

			auto angle = rotaryStartAngle + ((float)i / (float)numGrad) * (rotaryEndAngle - rotaryStartAngle);

			// Determine if the value has been reached
			bool valueReached = false;

			// Highlight colour, based on mouse interaction with slider
			auto highlightColour = slider.isMouseOverOrDragging() ? highlight : outline.withMultipliedBrightness(2.0f);

			// Special case for sliders with 0 at mid range (we want to enhance the distance from 0)
			if (slider.getName() == "bendAmountSlider" || slider.getName() == "masterSlider")
			{
				auto centerAngle = ((rotaryEndAngle - rotaryStartAngle) * 0.5f) + rotaryStartAngle;
				bool positiveValue = toAngle > centerAngle;
				valueReached = positiveValue ? (angle <= toAngle) && (angle >= centerAngle) 
											 : (angle >= toAngle) && (angle <= centerAngle);
			}
			else
				valueReached = angle <= toAngle;
			
			g.setColour(valueReached ? highlightColour : outline);

			Point<float> thumbPoint(bounds.getCentreX() + gradRadius * std::cos(angle - MathConstants<float>::halfPi),
				bounds.getCentreY() + gradRadius * std::sin(angle - MathConstants<float>::halfPi));

			g.fillEllipse(Rectangle<float>(gradWidth, gradWidth).withCentre(thumbPoint));
		}

		// knob
		auto knobBounds = bounds.reduced(size * 0.08f);
		ColourGradient grad (fill, bounds.getTopLeft(), fill.darker(0.8f), knobBounds.getBottomRight(), true);
		g.setGradientFill(grad);
		g.fillEllipse(knobBounds);
		g.setColour(slider.findColour(ResizableWindow::backgroundColourId));
		g.drawEllipse(knobBounds, 1.5f);

		// thumb
		auto thumbDist = radius * 0.77f;
		auto thumbWidth = lineW * 0.6f;
		Point<float> thumbPoint(bounds.getCentreX() + thumbDist * std::cos(toAngle - MathConstants<float>::halfPi),
			bounds.getCentreY() + thumbDist * std::sin(toAngle - MathConstants<float>::halfPi));

		auto thumbColour = slider.findColour(Slider::thumbColourId).interpolatedWith(fill, 0.3f);
		g.setColour(slider.isMouseOverOrDragging() ? highlight : thumbColour);
		g.fillEllipse(Rectangle<float>(thumbWidth, thumbWidth).withCentre(thumbPoint));
	}

	void drawLinearSlider(Graphics& g, int x, int y, int width, int height,
		float sliderPos,
		float minSliderPos,
		float maxSliderPos,
		const Slider::SliderStyle style, Slider& slider) override
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
			g.setColour(slider.findColour(Slider::backgroundColourId).darker());
			g.strokePath(backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });
			g.setColour(slider.findColour(Slider::backgroundColourId));
			g.strokePath(backgroundTrack, { trackWidth - 2.0f, PathStrokeType::curved, PathStrokeType::rounded });

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

			auto thumbWidth = getSliderThumbRadius(slider) * 0.9f;

			if (!isTwoVal)
			{
				auto highlight = slider.findColour(Slider::textBoxHighlightColourId);
				g.setColour(slider.isMouseOverOrDragging() ? highlight : slider.findColour(Slider::thumbColourId));

				//g.setColour(slider.findColour(Slider::thumbColourId));
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
	Font font;

public:
	void prepareFont()
	{
		auto monster = Typeface::createSystemTypefaceFor(BinaryData::mrsmonster_ttf, BinaryData::mrsmonster_ttfSize);
		font = Font(monster);
	}

private:
	void drawButtonText(Graphics& g, TextButton& button,
		bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/)
	{
		font.setHeight(button.getHeight() * 0.7f);
		g.setFont(font);

		auto defaultColour = button.findColour(button.getToggleState() ? TextButton::textColourOnId : TextButton::textColourOffId)
			.withMultipliedAlpha(shouldDrawButtonAsHighlighted ? 1.0f : 0.85f);

		auto highlightColour = button.findColour(TextButton::buttonOnColourId);

		g.setColour(shouldDrawButtonAsHighlighted ? highlightColour : defaultColour);

		const int yIndent = jmin(4, button.proportionOfHeight(0.3f));
		const int cornerSize = jmin(button.getHeight(), button.getWidth()) / 2;

		const int fontHeight = roundToInt(font.getHeight() * 0.6f);
		const int leftIndent = jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
		const int rightIndent = jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
		const int textWidth = button.getWidth() - leftIndent - rightIndent;

		if (textWidth > 0)
			g.drawFittedText(button.getButtonText(),
				leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
				Justification::centred, 2);
	}

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
