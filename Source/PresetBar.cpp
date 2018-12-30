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
    saveButton("Save"), 
	loadButton("Load"),
	processorState(ps)
{
	addAndMakeVisible(aboutButton);

	addAndMakeVisible(comboBox);
	comboBox.setColour(ComboBox::backgroundColourId, Colours::white.withAlpha(0.01f));
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

	aboutButton.setLookAndFeel(logoLF);
	aboutButton.setButtonText(JucePlugin_Name);
	aboutButton.setColour(ComboBox::outlineColourId, Colours::transparentBlack);
	aboutButton.onClick = [] { AboutScreen::showOrHideIfShowing(); };

	refreshPresetList();

	// TODO : remove these and find a better way to handle preset name change flag
	if (processorState.state.hasProperty("presetName"))
		for (auto preset : presetList)
			if (preset->file.getFileNameWithoutExtension() == processorState.state.getProperty("presetName").toString())
				currentPreset = preset;

	if (processorState.state.hasProperty("presetNameEdited"))
		comboBox.setText(processorState.state.getProperty("presetNameEdited"), dontSendNotification);
	
	if (currentPreset == nullptr)
		triggerAsyncUpdate();
}

PresetBar::~PresetBar()
{
	AboutScreen::close();
}

void PresetBar::paint (Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	g.setColour(getLookAndFeel().findColour(ComboBox::outlineColourId));
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

	comboBox.setText(presetName, dontSendNotification);

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
		String category;
		int chosenIndex;

		showCategoryChooser(chosenIndex, category);

		if (chosenIndex == 0)
			return;

		// Save as
		saveAsNewPreset(category);
	}
	else if (result == 3)
	{
		// Export
		exportPreset();
	}
}

#include "PluginProcessor.h"
bool PresetBar::checkXml(XmlElement * xml, String & errorMsg) const
{
	if (auto editor = findParentComponentOfClass<AudioProcessorEditor>())
	{
		if (auto proc = dynamic_cast<BassGeneratorAudioProcessor*>(editor->getAudioProcessor()))
			if (xml->getTagName() == proc->getProcessorStateIdentifier())
				return true;
	}

	errorMsg = "Error trying to load a preset file!";
	return false;
}

void PresetBar::saveAsNewPreset(String category)
{
	// Get file
	auto userFolder = getUserPresetsFolder();
	auto wantedName = comboBox.getText();

	auto f = userFolder.getNonexistentChildFile(wantedName, extension);
	std::unique_ptr<XmlElement> xml;
	xml.reset(getCurrentPresetAsXml(f.getFileNameWithoutExtension(), category));

	f.deleteFile();
	FileOutputStream fos(f);

	if (fos.openedOk())
		fos.writeText(xml->createDocument(""), false, false, nullptr);
	
	PresetComparator comparator;
	const auto presetName = f.getFileNameWithoutExtension();

	presetList.addSorted(comparator, new Preset({ presetName, category, f, *xml, false }));

	refreshComboBox();
	comboBox.setText(presetName);
}

void PresetBar::replaceCurrentPreset()
{
	if (canOverrideCurrentPreset())
	{
		const String category = currentPreset->category;

		currentPreset->file.deleteFile();		// Delete file
		refreshPresetList();
		saveAsNewPreset(category);						// Add new preset
	}
}

bool PresetBar::canOverrideCurrentPreset()
{
	return currentPreset != nullptr; // && !currentPreset->factoryPreset;
}

bool PresetBar::currentPresetChanged()
{
	return false;// processorState.state.getProperty("presetChanged");
}

XmlElement * PresetBar::getCurrentPresetAsXml(String presetName, String category)
{ 
	auto state = processorState.copyState();
	std::unique_ptr<XmlElement> xml(state.createXml());

	xml->setAttribute("PluginName", JucePlugin_Name);
	xml->setAttribute("PluginVersion", JucePlugin_VersionString);
	xml->setAttribute("PluginManufacturer", JucePlugin_Manufacturer);

	xml->setAttribute("PresetName", presetName);
	xml->setAttribute("PresetCreation", Time::getCurrentTime().toString(true, true));
	xml->setAttribute("PresetCategory", category);

	return xml.release();
}

void PresetBar::showCategoryChooser(int & result, String & category)
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
	alert.setLookAndFeel(&getLookAndFeel());
	alert.getComboBoxComponent("Category")->setColour(TextEditor::backgroundColourId, Colours::transparentBlack);
	alert.setColour(AlertWindow::backgroundColourId, findColour(ResizableWindow::backgroundColourId));

	result = alert.runModalLoop();
	category = alert.getComboBoxComponent("Category")->getText();
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

		String errorMsg;

		if (!checkXml(xml.get(), errorMsg))
		{
			NativeMessageBox::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "Error loading preset", errorMsg);
#if PAWG_PRESET_DESIGNER
#else
			comboBox.setText(processorState.state.getProperty("presetNameEdited"));
			return;
#endif
		}

		processorState.replaceState(ValueTree::fromXml(*xml));
		comboBox.setText(presetName);

		processorState.state.setProperty("presetChanged", true, nullptr);	// True because it's not save "internally"
		processorState.state.setProperty("presetName", presetName, nullptr);
		processorState.state.setProperty("presetNameEdited", presetName, nullptr);

		// The combo index will act as if it was on 0, so we can't go to the previous
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
		xml.reset(getCurrentPresetAsXml(destination.getFileNameWithoutExtension(), currentPreset->category));

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
				// Ignore this file
				continue;
			}

			presetList.add(new Preset({ xml->getStringAttribute("PresetName"), xml->getStringAttribute("PresetCategory"), f, *xml, false }));
		}
		else
			jassertfalse;
	}
	
	PresetComparator comparator;
	presetList.sort(comparator);

	// refresh combo box
	refreshComboBox();
}

void PresetBar::refreshComboBox()
{
	const auto otherCategory = "Other";

	// Store current state
	auto previousChoice = comboBox.getSelectedId();
	auto previousText = comboBox.getText();

	comboBox.clear();

	// Build categories
	struct PresetCategory
	{
		String name;

		struct IndexedPreset
		{
			Preset* preset;
			int index;
		};

		Array<IndexedPreset> presets;
	};

	Array<PresetCategory> categories;

	auto addToCategory = [&categories] (String cat, Preset* p, int i)
	{
		for (auto& c : categories)
		{
			if (c.name == cat)
			{
				c.presets.add({ p, i });
				return;
			}
		}

		Array<PresetCategory::IndexedPreset> arr;
		arr.add({ p, i });
		categories.add(PresetCategory({ cat, arr }));
	};

	int index = 0; // This is a "1 based" index as it really refers to the comboBox itemId (presetIndex + 1)

	for (auto p : presetList)
	{
		if (p->category.isEmpty())
			addToCategory(otherCategory, p, ++index);
		else
			addToCategory(p->category, p, ++index);
	}
	
	// Sort categories
	struct CategoryComparator
	{
		static int compareElements(const PresetCategory& first, const PresetCategory& second)
		{
			const auto firstName = first.name;
			const auto secondName = second.name;
			return firstName.compareNatural(secondName);
		}
	};

	CategoryComparator comparator;
	categories.sort(comparator);

	// Build combo box preset list
	auto rootMenu = comboBox.getRootMenu();

	comboBox.addItem("Reset to default parameters", setDefaultValuesItemId);
	comboBox.addSeparator();
	for (auto c : categories)
	{
		PopupMenu m;
		
		for (auto pi : c.presets)
			m.addItem(pi.index, pi.preset->name);

		rootMenu->addSubMenu(c.name, m);
	}

	comboBox.addSeparator();
	comboBox.addItem("Randomize parameters", randomizeItemId);
	comboBox.addSeparator();
	comboBox.addItem("Restore factory presets", restoreFactoryItemId);
	comboBox.addItem("Show presets folder", revealPresetsItemId);
#if PAWG_PRESET_DESIGNER
	comboBox.addSeparator();
	comboBox.addItem("Save preset sheet", savePresetSheetItemId);
	comboBox.addItem("Load preset sheet", loadPresetSheetItemId);
#endif

	if (previousChoice > 0)
		comboBox.setSelectedId(previousChoice);

	comboBox.setText(previousText);
}

void PresetBar::comboBoxChanged()
{
	auto text = comboBox.getText();
	auto selectedId = comboBox.getSelectedId();

	if (selectedId == randomizeItemId)
	{
		currentPreset = nullptr;
		randomizeParameters();
		comboBox.setText(nameGenerator.get());

		// Behaves as if it was on top of the list
		previousButton.setEnabled(false);
		nextButton.setEnabled(true);
		return;
	}
	else if (selectedId == setDefaultValuesItemId)
	{
		setDefaultValues();
		comboBox.setText("");

		// Behaves as if it was on top of the list
		previousButton.setEnabled(false);
		nextButton.setEnabled(true);
		return;
	}
	else if (selectedId == restoreFactoryItemId)
	{
		restoreFactoryPresets();
		comboBox.setText(processorState.state.getProperty("presetNameEdited"));
		return;
	}
	else if (selectedId == revealPresetsItemId)
	{
		getUserPresetsFolder().startAsProcess();
		comboBox.setText(processorState.state.getProperty("presetNameEdited"));
		return;
	}
#if PAWG_PRESET_DESIGNER
	else if (selectedId == savePresetSheetItemId)
	{
		savePresetSheet();
		comboBox.setText(processorState.state.getProperty("presetNameEdited"));
		return;
	}
	else if (selectedId == loadPresetSheetItemId)
	{
		loadPresetSheet();
		comboBox.setText(processorState.state.getProperty("presetNameEdited"));
		return;
	}
#endif

	auto presetIndex = -1;

	for (auto p : presetList)
		if (p->name == text)
			presetIndex = presetList.indexOf(p);

	if (presetIndex >= 0 && presetIndex != presetList.indexOf(currentPreset) && text == presetList[presetIndex]->name)
		setCurrentPreset(presetIndex);

	updateComboBoxTextColour();

	auto s = comboBox.getText();

	processorState.state.setProperty("presetNameEdited", comboBox.getText(), nullptr);
}

void PresetBar::updateComboBoxTextColour()
{
	const auto defaultColour = getLookAndFeel().findColour(Label::textColourId);
	auto presetIndex = comboBox.getSelectedId() - 1;
	auto textColour = (currentPresetChanged() || presetIndex < 0) ? defaultColour.withAlpha(0.75f) : defaultColour;
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

#include "Parameters.h"

void PresetBar::restoreFactoryPresets()
{
	auto s = String::createStringFromData(BinaryData::presets_preset, BinaryData::presets_presetSize);
	std::unique_ptr<XmlElement> xml(XmlDocument::parse(s));

	if (xml == nullptr || xml->getTagName() != "PRESET_SHEET")
		return;

	auto userFolder = getUserPresetsFolder();
	userFolder.moveToTrash();
	userFolder.createDirectory();

	forEachXmlChildElement(*xml.get(), e)
	{
		auto f = userFolder.getChildFile(e->getStringAttribute("PresetName") + extension);

		// Update attributes
		e->setAttribute("PluginName", JucePlugin_Name);
		e->setAttribute("PluginVersion", JucePlugin_VersionString);
		e->setAttribute("PluginManufacturer", JucePlugin_Manufacturer);

		FileOutputStream fos(f);

		if (fos.openedOk())
			fos.writeText(e->createDocument(""), false, false, nullptr);
	}

	refreshPresetList();
}

void PresetBar::randomizeParameters()
{
	Random random;

	auto setRandomValue = [this, &random](String paramId)
	{
		processorState.getParameter(paramId)->setValueNotifyingHost(random.nextFloat());
	};

	setRandomValue(ParameterIDs::drive);
	setRandomValue(ParameterIDs::driveType);
	setRandomValue(ParameterIDs::attack);
	setRandomValue(ParameterIDs::decay);
	setRandomValue(ParameterIDs::sustain);
	setRandomValue(ParameterIDs::release);
	setRandomValue(ParameterIDs::lpFreq);
	setRandomValue(ParameterIDs::glide);
	setRandomValue(ParameterIDs::bendAmount);
	setRandomValue(ParameterIDs::bendDuration);
#if PAWG_ALLOW_LPF_MODULATION
	setRandomValue(ParameterIDs::lpModAmount);
	setRandomValue(ParameterIDs::lpModDuration);
#endif

	// Limit current master gain
	auto master = processorState.getParameter(ParameterIDs::master);
	master->setValueNotifyingHost(jmin(0.8f, master->getValue()));
}

void PresetBar::setDefaultValues()
{
	auto setDefaultValue = [this](String paramId)
	{
		if (auto param = processorState.getParameter(paramId))
			param->setValueNotifyingHost(param->getDefaultValue());
	};

	setDefaultValue(ParameterIDs::drive);
	setDefaultValue(ParameterIDs::driveType);
	setDefaultValue(ParameterIDs::attack);
	setDefaultValue(ParameterIDs::decay);
	setDefaultValue(ParameterIDs::sustain);
	setDefaultValue(ParameterIDs::release);
	setDefaultValue(ParameterIDs::bendAmount);
	setDefaultValue(ParameterIDs::bendDuration);
	setDefaultValue(ParameterIDs::lpFreq);
	setDefaultValue(ParameterIDs::lpModAmount);
	setDefaultValue(ParameterIDs::lpModDuration);
	setDefaultValue(ParameterIDs::glide);
	setDefaultValue(ParameterIDs::master);
}

#if PAWG_PRESET_DESIGNER
void PresetBar::loadPresetSheet()
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

		if (xml == nullptr || xml->getTagName() != "PRESET_SHEET")
			return;

		auto userFolder = getUserPresetsFolder();
		userFolder.moveToTrash();
		userFolder.createDirectory();

		forEachXmlChildElement(*xml.get(), e)
		{
			auto f = userFolder.getChildFile(e->getStringAttribute("PresetName") + extension);

			FileOutputStream fos(f);

			if (fos.openedOk())
				fos.writeText(e->createDocument(""), false, false, nullptr);
		}

		refreshPresetList();
	}
}

void PresetBar::savePresetSheet()
{
	FileChooser fc("Select a preset location...",
		File::getSpecialLocation(File::userHomeDirectory).getChildFile(comboBox.getText()),
		"*.xml");

	if (fc.browseForFileToOpen())
	{
		File destination(fc.getResult().withFileExtension(extension));

		std::unique_ptr<XmlElement> xml;
		xml.reset(new XmlElement("PRESET_SHEET"));

		xml->setAttribute("PluginName",			JucePlugin_Name);
		xml->setAttribute("PluginVersion",		JucePlugin_VersionString);
		xml->setAttribute("PluginManufacturer", JucePlugin_Manufacturer);
		xml->setAttribute("Creation", Time::getCurrentTime().toString(true, true));

		for (auto p : presetList)
		{
			XmlDocument doc(p->file);

			if (auto element = doc.getDocumentElement())
			{
				element->setAttribute("PluginName",			JucePlugin_Name);
				element->setAttribute("PluginVersion",		JucePlugin_VersionString);
				element->setAttribute("PluginManufacturer", JucePlugin_Manufacturer);

				xml->addChildElement(element);
			}
		}

		destination.deleteFile();
		FileOutputStream fos(destination);

		if (fos.openedOk())
			fos.writeText(xml->createDocument(""), false, false, nullptr);
	}
}
#endif