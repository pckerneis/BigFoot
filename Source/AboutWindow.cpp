/*
  ==============================================================================

    AboutWindow.cpp
    Created: 20 Dec 2018 2:37:51am
    Author:  Pierre

  ==============================================================================
*/

#include "AboutWindow.h"
AboutScreen::AboutScreen()
{
    setSize (windowWidth, windowHeight);

	backgroundImage = ImageCache::getFromMemory(BinaryData::bigfootprint_png, BinaryData::bigfootprint_pngSize);

    logoImage = ImageCache::getFromMemory(BinaryData::bestiary_logo_white_sm2_png, BinaryData::bestiary_logo_white_sm2_pngSize);
    
	auto monster = Typeface::createSystemTypefaceFor(BinaryData::mrsmonster_ttf, BinaryData::mrsmonster_ttfSize);
	monsterFont = Font(monster);
}

void AboutScreen::paint (Graphics& g)
{
    const auto bg = getLookAndFeel().findColour (ResizableWindow::backgroundColourId);

	g.setColour(bg);
	g.fillAll();

	g.setOpacity(0.2f);
	g.drawImageWithin(backgroundImage, 0, 0, getWidth(), getHeight(), RectanglePlacement::fillDestination);
        
    const auto r = getLocalBounds().reduced (12);
	auto center = r.withSizeKeepingCentre(r.getWidth(), r.proportionOfHeight(0.34f));
	auto footer = r.withTrimmedTop(r.proportionOfHeight(0.84f));

	g.setColour(getLookAndFeel().findColour(Label::textColourId));
	g.setFont(12.5f);
	const String authorString(CharPointer_UTF8("Author: Pierre-Cl\xc3\xa9ment Kerne\xc3\xafs"));
	g.drawFittedText(authorString, footer, Justification::topLeft, 1, 1.0);

	const String copyright(CharPointer_UTF8("\xc2\xa9 2019 Bestiary"));
    g.drawFittedText (copyright, footer, Justification::bottomLeft, 1, 1.0);
    
	auto compilationTime = Time::getCompilationDate().toString(true, true, false);
    auto version = JucePlugin_VersionString + String(" (") + compilationTime + ")";
    g.drawFittedText (version, r, Justification::topLeft, 1, 1.0);

	g.setFont(monsterFont.withHeight(36.0f));
	g.drawFittedText(JucePlugin_Name, center, Justification::centred, 1, 1.0);
    
    const auto logoSize = 30.0f;
    auto pluginArea = Rectangle<float>((float)getWidth() - ((float)logoSize * 1.3f), (float)getHeight() - logoSize * 1.3f, logoSize, logoSize);
    g.setOpacity(0.95f);
    g.drawImage(logoImage, pluginArea, RectanglePlacement::fillDestination);
}

void AboutScreen::mouseDown (const MouseEvent&)
{
    close();
}

ScopedPointer<AboutScreen> aboutScreen;

void AboutScreen::show()
{
    if (aboutScreen != nullptr)
    {
        aboutScreen->toFront (true);
        return;
    }
    
    aboutScreen = new AboutScreen();
    
    int flags = ComponentPeer::windowIsTemporary | ComponentPeer::windowHasDropShadow;
    
    aboutScreen->setOpaque (true);
    aboutScreen->addToDesktop (flags);
    aboutScreen->setCentreRelative (0.5f, 0.5f);
    aboutScreen->setVisible (true);
}

void AboutScreen::close()
{
    aboutScreen = nullptr;
}

void AboutScreen::showOrHideIfShowing()
{
	if (aboutScreen == nullptr)
		show();
	else
		close();
}
