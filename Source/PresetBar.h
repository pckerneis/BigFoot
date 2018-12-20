/*
  ==============================================================================

    PresetBar.h
    Created: 18 Dec 2018 2:56:58am
    Author:  Pierre

  ==============================================================================
*/

#pragma once

#include "LookAndFeel.h"

//==============================================================================
/*
*/
class PresetBar    : public Component, ValueTree::Listener, public AsyncUpdater
{
public:
    PresetBar(AudioProcessorValueTreeState& vts);
    ~PresetBar();

    void paint (Graphics&) override;
    void resized() override;

	//==============================================================================
	/** Updates combo box text colour. */
	void processorParametersMayHaveChanged();

	//==============================================================================
	/** Loads a preset. */
	void setCurrentPreset(int presetIndex);

	//==============================================================================
	/** ValueTree::Listener implementation */
	virtual void valueTreePropertyChanged(ValueTree&, const Identifier&)
	{
		processorParametersMayHaveChanged();
	}
	virtual void valueTreeChildAdded(ValueTree&, ValueTree&) override {}
	virtual void valueTreeChildRemoved(ValueTree&, ValueTree&, int) override {}
	virtual void valueTreeChildOrderChanged(ValueTree&, int, int) override {}
	virtual void valueTreeParentChanged(ValueTree& treeWhoseParentHasChanged) override {}

	//==============================================================================
	/** AsyncUpdater implementation. Try to load first preset after the factory presets were
	created for the first time */
	void handleAsyncUpdate() override
	{
		setCurrentPreset(0);
	}

private:
	//==============================================================================
	struct Preset
	{
		File file;
		XmlElement state;
		bool factoryPreset;
	};

	//==============================================================================
	/*	Creates a new preset file with the current combo box text and the current processor state. 
		If the desired name is already taken, add suffix */
	void saveAsNewPreset();

	/* Deletes current preset file and add a new one with current parameters. */
	void replaceCurrentPreset();

	/* True if a preset has been loaded. */
	bool canOverrideCurrentPreset();

	/* True if one of the parameters has been altered. */
	bool currentPresetChanged();

	/* Creates a XmlElement with the current processor state and some info about the current preset. */
	XmlElement* getCurrentPresetAsXml(String presetName);

	//==============================================================================
	/* Opens a FileChooser. If a suitable preset file is chosen, it will be loaded. */
	void loadPresetFromFile();

	/* Opens a FileChooser and export the current preset to the specified destination. */
	void exportPreset();

	//==============================================================================
	/* Reloads preset list from files. */
	void refreshPresetList();

	/* Refresh the combo box choices based on the preset list. */
	void refreshComboBox();

	/* React to a selected item or text change. */
	void comboBoxChanged();

	/* If the preset was changed (its name or the processor state), the combo box will be greyed out. */
	void updateComboBoxTextColour();

	//==============================================================================
	/* Show some save commands */
	void showSavePopupMenu();

	//==============================================================================
	/* Folder where are stored the presets (a subfolder of the user application data directory) */
	File getUserPresetsFolder();

	//==============================================================================
	/* Builds the default preset files */
	void restoreFactoryPresets();

	//==============================================================================
	OwnedArray<Preset> presetList;
	Preset* currentPreset;

	TextButton aboutButton;
	ComboBox comboBox;
	TextButton previousButton;
	TextButton nextButton;
	TextButton saveButton;
	TextButton loadButton;

	AudioProcessorValueTreeState& processorState;

	enum SpecialComboItemIds
	{
		restoreFactoryItemId = 1000,
		revealPresetsItemId = 1001
	};

	String extension = ".preset";

	SharedResourcePointer<LogoButtonLF> logoLF;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBar)
};
