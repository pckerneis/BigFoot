/*
  ==============================================================================

    PresetBar.cpp
    Created: 18 Dec 2018 2:56:58am
    Author:  Pierre

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "PresetBar.h"
#include "AboutWindow.h"

//==============================================================================
PresetBar::PresetBar(AudioProcessorValueTreeState& ps) :
	currentPreset(nullptr),
	aboutButton(""),
	previousButton("<"), 
	nextButton(">"), 
	loadButton("Load"), 
	saveButton("Save"), 
	processorState(ps)
{
	addAndMakeVisible(aboutButton);

	addAndMakeVisible(comboBox);
	comboBox.setColour(ComboBox::backgroundColourId, Colours::white.withAlpha(0.008f));
	comboBox.setColour(ComboBox::outlineColourId, Colours::darkgrey);
	comboBox.onChange = [this] { comboBoxChanged(); };
	comboBox.setEditableText(true);
	comboBox.setJustificationType(Justification::centred);

	addAndMakeVisible(previousButton);
	previousButton.onClick = [this] { comboBox.setSelectedId(comboBox.getSelectedId() - 1); };
	addAndMakeVisible(nextButton);
	nextButton.onClick = [this] { comboBox.setSelectedId(comboBox.getSelectedId() + 1); };

	addAndMakeVisible(loadButton);
	loadButton.onClick = [this] { loadPresetFromFile(); };
	 
	addAndMakeVisible(saveButton);
	saveButton.onClick = [this] { showSavePopupMenu(); };

	for (auto c : getChildren())
	{
		if (auto b = dynamic_cast<Button*>(c))
		{
			b->setColour(TextButton::buttonColourId, Colours::transparentBlack);
			b->setColour(ComboBox::outlineColourId, Colours::darkgrey);
		}
	}

	aboutButton.setLookAndFeel(logoLF);
	aboutButton.setButtonText(JucePlugin_Name);
	aboutButton.setColour(ComboBox::outlineColourId, Colours::transparentBlack);
	aboutButton.onClick = [] { AboutScreen::showOrHideIfShowing(); };

	refreshPresetList();

	if (processorState.state.hasProperty("presetName"))
		for (auto preset : presetList)
			if (preset->file.getFileNameWithoutExtension() == processorState.state.getProperty("presetName").toString())
				currentPreset = preset;

	if (processorState.state.hasProperty("presetNameEdited"))
		comboBox.setText(processorState.state.getProperty("presetNameEdited"), dontSendNotification);

	processorState.state.addListener(this);

	if (currentPreset == nullptr)
		triggerAsyncUpdate();
}

PresetBar::~PresetBar()
{
	AboutScreen::close();
	processorState.state.removeListener(this);
}

void PresetBar::paint (Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

    g.setColour (Colours::darkgrey);
    g.fillRect (getLocalBounds().removeFromBottom(1));
}

void PresetBar::resized()
{
	auto r = getLocalBounds().withTrimmedRight(2);
	aboutButton.setBounds(r.removeFromLeft(100));

	int marginW = 1;
	int marginH = 4;

	saveButton.setBounds(r.removeFromRight(40).reduced(marginW, marginH));
	loadButton.setBounds(r.removeFromRight(40).reduced(marginW, marginH));

	nextButton.setBounds(r.removeFromRight(24).reduced(marginW, marginH));
	previousButton.setBounds(r.removeFromRight(24).reduced(marginW, marginH));

	comboBox.setBounds(r.reduced(marginW, marginH));
}

void PresetBar::processorParametersMayHaveChanged()
{
	updateComboBoxTextColour();
}

void PresetBar::setCurrentPreset(int presetIndex)
{
	currentPreset = presetList[presetIndex];

	if (currentPreset == nullptr)
		return;

	auto state = currentPreset->state;
	processorState.replaceState(ValueTree::fromXml(state));

	nextButton.setEnabled(presetIndex < presetList.size() - 1);
	previousButton.setEnabled(presetIndex > 0);

	const String presetName = currentPreset->file.getFileNameWithoutExtension();

	comboBox.setText(presetName, false);

	processorState.state.setProperty("presetChanged", false, nullptr);
	processorState.state.setProperty("presetName", presetName, nullptr);
	processorState.state.setProperty("presetNameEdited", presetName, nullptr);
}

void PresetBar::showSavePopupMenu()
{
	PopupMenu m;
	m.addItem(1, "Replace existing", canOverrideCurrentPreset());
	m.addItem(2, "Save as new preset");
	m.addItem(3, "Save to file...");

	SharedResourcePointer<CustomLookAndFeel> lf;
	m.setLookAndFeel(lf);

	const int result = m.showAt(saveButton.getScreenBounds());

	if (result == 0)
	{
	}
	else if (result == 1)
	{
		// Replace
		replaceCurrentPreset();
	}
	else if (result == 2)
	{
		// Save as
		saveAsNewPreset();
	}
	else if (result == 3)
	{
		// Export
		exportPreset();
	}
}

void PresetBar::saveAsNewPreset()
{
	auto userFolder = getUserPresetsFolder();
	auto wantedName = comboBox.getText();

	auto f = userFolder.getNonexistentChildFile(wantedName, extension);
	std::unique_ptr<XmlElement> xml;
	xml.reset(getCurrentPresetAsXml(f.getFileNameWithoutExtension()));

	f.deleteFile();
	FileOutputStream fos(f);

	if (fos.openedOk())
		fos.writeText(xml->createDocument(""), false, false, nullptr);

	struct PresetComparator
	{
		static int compareElements(Preset* first, Preset* second)
		{
			const auto firstName = first->file.getFileNameWithoutExtension();
			const auto secondName = second->file.getFileNameWithoutExtension();
			return firstName.compareNatural(secondName);
		}
	};

	PresetComparator comparator;

	presetList.addSorted(comparator, new Preset({ f, *xml, false }));

	refreshComboBox();
	comboBox.setText(f.getFileNameWithoutExtension());
}

void PresetBar::replaceCurrentPreset()
{
	if (canOverrideCurrentPreset())
	{
		currentPreset->file.deleteFile();		// Delete file
		refreshPresetList();
		saveAsNewPreset();						// Add new preset
	}
}

bool PresetBar::canOverrideCurrentPreset()
{
	return currentPreset != nullptr; // && !currentPreset->factoryPreset;
}

bool PresetBar::currentPresetChanged()
{
	return processorState.state.getProperty("presetChanged");
}

XmlElement * PresetBar::getCurrentPresetAsXml(String presetName)
{ 
	auto state = processorState.copyState();
	std::unique_ptr<XmlElement> xml(state.createXml());

	xml->setAttribute("PluginName", JucePlugin_Name);
	xml->setAttribute("PluginVersion", JucePlugin_VersionString);
	xml->setAttribute("PluginManufacturer", JucePlugin_Manufacturer);

	xml->setAttribute("PresetName", presetName);
	xml->setAttribute("PresetCreation", Time::getCurrentTime().toString(true, true));

	return xml.release();
}

void PresetBar::loadPresetFromFile()
{
	FileChooser fc("Select a preset file...",
		File::getSpecialLocation(File::userHomeDirectory),
		"*" + extension);

	if (fc.browseForFileToOpen())
	{
		File f(fc.getResult());

		XmlDocument doc(f);

		std::unique_ptr<XmlElement> xml;
		xml.reset(doc.getDocumentElement());

		const auto presetName = f.getFileNameWithoutExtension();

		processorState.replaceState(ValueTree::fromXml(*xml));
		comboBox.setText(presetName);

		processorState.state.setProperty("presetChanged", true, nullptr);	// True because it's not save "internally"
		processorState.state.setProperty("presetName", presetName, nullptr);
		processorState.state.setProperty("presetNameEdited", presetName, nullptr);

		// Te combo index will act as if it was on 0, so we can't go to the previous
		// The next button will select the preset on top of the list
		previousButton.setEnabled(false);
		nextButton.setEnabled(true);
	}
}

void PresetBar::exportPreset()
{
	FileChooser fc("Select a preset location...",
		File::getSpecialLocation(File::userHomeDirectory).getChildFile(comboBox.getText()),
		"*" + extension);

	if (fc.browseForFileToOpen())
	{
		File destination(fc.getResult().withFileExtension(extension));

		std::unique_ptr<XmlElement> xml;
		xml.reset(getCurrentPresetAsXml(destination.getFileNameWithoutExtension()));

		destination.deleteFile();
		FileOutputStream fos(destination);

		if (fos.openedOk())
			fos.writeText(xml->createDocument(""), false, false, nullptr);
	}
}

void PresetBar::refreshPresetList()
{
	presetList.clear();

	File userFolder = getUserPresetsFolder();

	Array<File> presetFiles;
	userFolder.findChildFiles(presetFiles, File::findFiles, true, "*" + extension);

	if (presetFiles.size() == 0)
	{
		restoreFactoryPresets();
		triggerAsyncUpdate();
		return;
	}

	for (auto f : presetFiles)
	{
		// Check and add
		XmlDocument doc(f);
		std::unique_ptr<XmlElement> xml;
		xml.reset(doc.getDocumentElement());

		if (xml != nullptr)
		{
			if (xml->getStringAttribute("PluginManufacturer") != JucePlugin_Manufacturer
				|| xml->getStringAttribute("PluginName") != JucePlugin_Name)
			{
				jassertfalse;
				return;
			}

			presetList.add(new Preset({ f, *xml, false }));
		}
		else
			jassertfalse;
	}

	struct PresetComparator
	{
		static int compareElements(Preset* first, Preset* second)
		{
			const auto firstName = first->file.getFileNameWithoutExtension();
			const auto secondName = second->file.getFileNameWithoutExtension();
			return firstName.compareNatural(secondName);
		}
	};

	PresetComparator comparator;
	presetList.sort(comparator);

	// refresh combo box
	refreshComboBox();
}

void PresetBar::refreshComboBox()
{
	auto previousChoice = comboBox.getSelectedId();
	auto previousText = comboBox.getText();

	comboBox.clear();

	int i = 0;

	for (auto p : presetList)
		comboBox.addItem(p->file.getFileNameWithoutExtension(), ++i);

	comboBox.addSeparator();
	comboBox.addItem("Restore factory presets", restoreFactoryItemId);
	comboBox.addItem("Show presets folder", revealPresetsItemId);

	if (previousChoice > 0)
		comboBox.setSelectedId(previousChoice);

	comboBox.setText(previousText);
}

void PresetBar::comboBoxChanged()
{
	auto selectedId = comboBox.getSelectedId();

	if (selectedId == restoreFactoryItemId) // Restore factory presets
	{
		restoreFactoryPresets();
		comboBox.setText(processorState.state.getProperty("presetNameEdited"));
		return;
	}
	else if (selectedId == revealPresetsItemId) // Reveal presets folder
	{
		getUserPresetsFolder().startAsProcess();
		comboBox.setText(processorState.state.getProperty("presetNameEdited"));
		return;
	}

	auto presetIndex = selectedId - 1;

	if (presetIndex >= 0 && presetIndex != presetList.indexOf(currentPreset))
		setCurrentPreset(presetIndex);

	updateComboBoxTextColour();

	auto s = comboBox.getText();

	processorState.state.setProperty("presetNameEdited", comboBox.getText(), nullptr);
}

void PresetBar::updateComboBoxTextColour()
{
	auto presetIndex = comboBox.getSelectedId() - 1;
	auto textColour = (currentPresetChanged() || presetIndex < 0) ? Colours::lightgrey : Colours::white;
	comboBox.setColour(ComboBox::textColourId, textColour);
}

File PresetBar::getUserPresetsFolder()
{
	File rootFolder = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory);

#ifdef JUCE_MAC
	rootFolder = rootFolder.getChildFile("Audio").getChildFile("Presets");
#endif
	rootFolder = rootFolder.getChildFile(JucePlugin_Manufacturer).getChildFile(JucePlugin_Name);
	rootFolder.createDirectory();
	return rootFolder;
}

void PresetBar::restoreFactoryPresets()
{
	struct PresetValues
	{
		String name;
		double drive;
		double driveType;
		double bendAmount;
		double bendDuration;
		double brightness;
		double attack;
		double decay;
		double sustain;
		double release;
		double glide;
		double master;
	};

	Array<PresetValues> presetValues;

	presetValues.add(PresetValues({ "PAWG",			0.5,	0.0,	7.0,	0.2,	1000.0,	0.01,	0.1,	0.8,	0.1,	0.2,	5.7 }));
	presetValues.add(PresetValues({ "Stomper",		0.7,	1.0,	13.0,	0.14,	2663.0,	0.01,	0.1,	0.43,	0.1,	0.2,	4.8 }));
	presetValues.add(PresetValues({ "Dark times",	0.56,	2.0,	-5.8,	0.07,	266.0,	0.047,	0.99,	0.0,	0.99,	0.0,	5.0 }));
	presetValues.add(PresetValues({ "WOAW",			0.83,	2.0,	-16.0,	0.18,	712.0,	0.45,	0.55,	0.0,	0.39,	0.09,	6.45 }));
	presetValues.add(PresetValues({ "Kick",			0.0,	0.0,	24.0,	0.138,	2781.0,	0.005,	0.209,	0.0,	0.1,	0.0,	9.46 }));
	presetValues.add(PresetValues({ "Kick harder",	0.4,	1.0,	24.0,	0.138,	712.0,	0.002,	0.3,	0.0,	0.18,	0.0,	0.03 }));
	presetValues.add(PresetValues({ "Sub drop",		0.77,	0.0,	12.0,	0.93,	60.0,	0.01,	0.94,	0.95,	0.52,	0.0,	7.77 }));
	presetValues.add(PresetValues({ "Hit & hold",	0.4,	1.0,	24.0,	0.047,	920.0,	0.0035,	0.1,	0.3,	0.1,	0.0,	2.45 }));
	presetValues.add(PresetValues({ "Hollow",		0.56,	2.0,	-24.0,	0.083,	188.0,	0.0031,	0.49,	0.12,	0.1,	0.2,	6.0 }));
	presetValues.add(PresetValues({ "Dark thoughts",0.73,	2.0,	4.22,	0.248,	325.0,	1.2,	1.57,	0.0,	0.074,	0.2,	5.17 }));
	presetValues.add(PresetValues({ "Squarish",		1.0,	1.0,	0.08,	0.2,	10000.,	0.6,	0.001,	1.0,	0.1,	0.27,	6.57 }));
	presetValues.add(PresetValues({ "BBW",			0.656,	2.0,	0.28,	0.15,	5392.,	0.008,	0.51,	0.1,	0.56,	0.0,	4.81 }));
	presetValues.add(PresetValues({ "Simply sin",	0.0,	0.0,	0.0,	0.0,	10000.,	0.01,	0.1,	0.43,	0.1,	0.18,	9.79 }));

	auto userFolder = getUserPresetsFolder();
	
	auto addParamChild = [](XmlElement* xml, String name, float value) {
		auto child = new XmlElement("PARAM");
		child->setAttribute("id", name);
		child->setAttribute("value", value);
		xml->addChildElement(child);
	};

	int index = -1;

	for (auto pv : presetValues)
	{
		const auto fileName = String(++index).paddedLeft('0', 2) + " " + pv.name + extension;
		auto f = userFolder.getChildFile(fileName);
		f.deleteFile();

		std::unique_ptr<XmlElement> xml(new XmlElement("BassGenParameters"));

		xml->setAttribute("PluginName", JucePlugin_Name);
		xml->setAttribute("PluginVersion", JucePlugin_VersionString);
		xml->setAttribute("PluginManufacturer", JucePlugin_Manufacturer);

		xml->setAttribute("PresetName", f.getFileNameWithoutExtension());
		xml->setAttribute("PresetCreation", Time::getCurrentTime().toString(true, true));

		addParamChild(xml.get(), "drive",			pv.drive);
		addParamChild(xml.get(), "driveType",		pv.driveType);
		addParamChild(xml.get(), "bendAmount",		pv.bendAmount);
		addParamChild(xml.get(), "bendDuration",	pv.bendDuration);
		addParamChild(xml.get(), "brightness",		pv.brightness);
		addParamChild(xml.get(), "attack",			pv.attack);
		addParamChild(xml.get(), "decay",			pv.decay);
		addParamChild(xml.get(), "sustain",			pv.sustain);
		addParamChild(xml.get(), "release",			pv.release);
		addParamChild(xml.get(), "glide",			pv.glide);
		addParamChild(xml.get(), "master",			pv.master);

		FileOutputStream fos(f);

		if (fos.openedOk())
			fos.writeText(xml->createDocument(""), false, false, nullptr);

		presetList.add(new Preset({ f, *xml, false }));
	}

	refreshPresetList();
}
