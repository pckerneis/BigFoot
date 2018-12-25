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
}

void AboutScreen::paint (Graphics& g)
{
    const auto bg = getLookAndFeel().findColour (ResizableWindow::backgroundColourId);
    g.fillAll (bg);

	g.drawImageWithin(backgroundImage, 0, 0, getWidth(), getHeight(), RectanglePlacement::fillDestination);
    
    g.setColour (bg);
    g.setOpacity (0.23f);
    
    const auto r = getLocalBounds().reduced (12);

    g.setFont (24.0f);
    //g.setColour (bg.contrasting());
    g.setColour (Colours::black);
    
    g.drawFittedText ("BigFoot by Bestiary", r, Justification::centred, 1, 1.0);
    
    g.setFont (12.5f);

	auto footer = r.withTrimmedTop(r.proportionOfHeight(0.84f));
    
	const String authorString(CharPointer_UTF8("Author: Pierre-Cl\xc3\xa9ment Kerne\xc3\xafs"));
	g.drawFittedText(authorString, footer, Justification::topLeft, 1, 1.0);

	const String copyright(CharPointer_UTF8("\xc2\xa9 2018 Bestiary"));
    g.drawFittedText (copyright, footer, Justification::bottomLeft, 1, 1.0);
    
    auto version = JucePlugin_VersionString;
    g.drawFittedText (version, r, Justification::topLeft, 1, 1.0);
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
