/*
  ==============================================================================

    PresetBar.h
    Created: 18 Dec 2018 2:56:58am
    Author:  Pierre

  ==============================================================================
*/

#pragma once

#include "LookAndFeel.h"
#include "RandomPresetNameGenerator.h"

//==============================================================================
/*
*/
class PresetBar    : public Component,/* ValueTree::Listener,*/ public AsyncUpdater
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
	/*
	virtual void valueTreePropertyChanged(ValueTree&, const Identifier&) override
	{
		processorParametersMayHaveChanged();
	}
	virtual void valueTreeChildAdded(ValueTree&, ValueTree&) override {}
	virtual void valueTreeChildRemoved(ValueTree&, ValueTree&, int) override {}
	virtual void valueTreeChildOrderChanged(ValueTree&, int, int) override {}
	virtual void valueTreeParentChanged(ValueTree& treeWhoseParentHasChanged) override {}
	*/

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
		String name;
		String category;
		File file;
		XmlElement state;
		bool factoryPreset;
	};

	struct PresetComparator
	{
		static int compareElements(Preset* first, Preset* second)
		{
			const auto firstCategory = first->category;
			const auto secondCategory = second->category;

			if (firstCategory == secondCategory)
			{
				const auto firstName = first->file.getFileNameWithoutExtension();
				const auto secondName = second->file.getFileNameWithoutExtension();

				return firstName.compareNatural(secondName);
			}
			else
				return firstCategory.compareNatural(secondCategory);
		}
	};

	bool checkXml(XmlElement* xml, String& errorMsg) const;

	//==============================================================================
	/*	Creates a new preset file with the current combo box text and the current processor state. 
		If the desired name is already taken, add suffix */
	void saveAsNewPreset(String category);

	/* Deletes current preset file and add a new one with current parameters. */
	void replaceCurrentPreset();

	/* True if a preset has been loaded. */
	bool canOverrideCurrentPreset();

	/* True if one of the parameters has been altered. */
	bool currentPresetChanged();

	/* Creates a XmlElement with the current processor state and some info about the current preset. */
	XmlElement* getCurrentPresetAsXml(String presetName, String category);

	void showCategoryChooser(int& result, String& category)
	{
		// Category chooser
		StringArray categories;

		for (auto p : presetList)
			categories.addIfNotAlreadyThere(p->category);

		AlertWindow alert("Preset category", "Choose or create a preset category", AlertWindow::AlertIconType::QuestionIcon);
		alert.addComboBox("Category", categories);
		alert.addButton("Cancel", 0);
		alert.addButton("Apply", KeyPress::backspaceKey);
		alert.getComboBoxComponent("Category")->setEditableText(true);
		alert.setColour(AlertWindow::backgroundColourId, findColour(ResizableWindow::backgroundColourId));

		result = alert.runModalLoop();
		category = alert.getComboBoxComponent("Category")->getText();
	}

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
	/* Set the current parameters to random values */
	void randomizeParameters();

	/* Generates some random preset name */
	void setDefaultValues();

	//==============================================================================
#if PAWG_PRESET_DESIGNER
	/* Load preset sheet */
	void loadPresetSheet();

	/* Save preset sheet */
	void savePresetSheet();
#endif

	//==============================================================================
	OwnedArray<Preset> presetList;
	Preset* currentPreset;

	class CustomComboBox : public ComboBox
	{
		void showPopup() override
		{
			if (auto bar = findParentComponentOfClass<PresetBar>())
				bar->refreshPresetList();

			ComboBox::showPopup();
		}
	};

	TextButton aboutButton;
	CustomComboBox comboBox;
	TextButton previousButton;
	TextButton nextButton;
	TextButton saveButton;
	TextButton loadButton;

	AudioProcessorValueTreeState& processorState;

	enum SpecialComboItemIds
	{
		restoreFactoryItemId = 1000,
		revealPresetsItemId = 1001,
		randomizeItemId = 1002,
		setDefaultValuesItemId = 1003,
		savePresetSheetItemId = 1004,
		loadPresetSheetItemId = 1005
	};

	String extension = ".preset";

	RandomPresetNameGenerator nameGenerator;

	SharedResourcePointer<LogoButtonLF> logoLF;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBar)
};
