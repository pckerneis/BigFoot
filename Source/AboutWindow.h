/*
  ==============================================================================

    AboutWindow.h
    Created: 20 Dec 2018 2:37:51am
    Author:  Pierre

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

/** \brief The about window content and static methods to add an intance to the desktop. */
class AboutScreen    : public Component
{
public:
    AboutScreen();
    virtual ~AboutScreen() {}
    
    void paint (Graphics& g) override;
    void mouseDown (const MouseEvent&) override;
    
    static void show();
    static void close();
	static void showOrHideIfShowing();
    
private:
    const int windowWidth = 300;
    const int windowHeight = 200;

	Image backgroundImage;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AboutScreen)
};