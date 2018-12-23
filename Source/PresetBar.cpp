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
		int result;

		showCategoryChooser(result, category);

		if (result == 0)
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

void PresetBar::saveAsNewPreset(String category)
{
	// Get file
	auto userFolder = getUserPresetsFolder();
	auto wantedName = comboBox.getText();

	auto f = userFolder.getNonexistentChildFile(wantedName, extension);
	std::unique_ptr<XmlElement> xml;
	xml.reset(getCurrentPresetAsXml(f.getFileNameWithoutExtension()));

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

	if (previousChoice > 0)
		comboBox.setSelectedId(previousChoice);

	comboBox.setText(previousText);
}

void PresetBar::comboBoxChanged()
{
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

#include "Parameters.h"

void PresetBar::restoreFactoryPresets()
{
	struct PresetValues
	{
		String name;
		String category;
		double drive;
		double driveType;
		double bendAmount;
		double bendDuration;
		double lpFreq;
		double lpModAmount;
		double lpModDuration;
		double attack;
		double decay;
		double sustain;
		double release;
		double glide;
		double master;
		double lpReso;
	};

	Array<PresetValues> presetValues;

	presetValues.add(PresetValues({ "PAWG",			"01. Bass",		0.5,	0.0,	7.0,	0.2,	1000.0,	0.0,	0.0,	0.01,	0.1,	0.8,	0.1,	0.08,	5.7,	0.707 }));
	presetValues.add(PresetValues({ "Stomper",		"01. Bass",		0.7,	1.0,	13.0,	0.14,	2663.0,	0.0,	0.0,	0.01,	0.1,	0.43,	0.1,	0.08,	4.8,	0.707 }));
	presetValues.add(PresetValues({ "Dark times",	"01. Bass",		0.56,	2.0,	-5.8,	0.07,	266.0,	0.0,	0.0,	0.047,	0.99,	0.0,	0.99,	0.0,	5.0,	0.707 }));
	presetValues.add(PresetValues({ "WOAW",			"01. Bass",		0.83,	2.0,	-16.0,	0.18,	712.0,	0.0,	0.0,	0.45,	0.55,	0.3,	0.39,	0.09,	6.45,	0.707 }));
	presetValues.add(PresetValues({ "Hit & hold",	"01. Bass",		0.4,	1.0,	24.0,	0.047,	920.0,	0.0,	0.0,	0.0035,	0.1,	0.3,	0.1,	0.0,	2.45,	0.707 }));
	presetValues.add(PresetValues({ "Hollow",		"01. Bass",		0.56,	2.0,	-24.0,	0.083,	188.0,	0.0,	0.0,	0.0031,	0.49,	0.12,	0.1,	0.1,	6.0,	0.707 }));
	presetValues.add(PresetValues({ "Dark thoughts","01. Bass",		0.73,	2.0,	4.22,	0.248,	325.0,	0.0,	0.0,	1.2,	1.57,	0.0,	0.074,	0.1,	5.17,	0.707 }));
	presetValues.add(PresetValues({ "Squarish",		"01. Bass",		1.0,	1.0,	0.08,	0.2,	10000.,	0.0,	0.0,	0.6,	0.001,	1.0,	0.1,	0.27,	6.57,	0.707 }));
	presetValues.add(PresetValues({ "BBW",			"01. Bass",		0.656,	2.0,	0.28,	0.15,	5392.,	0.0,	0.0,	0.008,	0.51,	0.1,	0.56,	0.0,	4.81,	0.707 }));
	presetValues.add(PresetValues({ "Futurism",		"01. Bass",		0.53,	0.0,	8.9,	0.043,	460.,	0.0,	0.0,	0.01,	1.02,	0.0,	0.21,	0.0,	5.69,	0.707 }));

	presetValues.add(PresetValues({ "Kick",			"03. Kick",		0.0,	0.0,	24.0,	0.138,	2781.0,	0.0,	0.0,	0.005,	0.209,	0.0,	0.1,	0.0,	7.6,	0.707 }));
	presetValues.add(PresetValues({ "Kick harder",	"03. Kick",		0.4,	1.0,	24.0,	0.138,	712.0,	0.0,	0.0,	0.002,	0.3,	0.0,	0.18,	0.0,	2.03,	0.707 }));
	presetValues.add(PresetValues({ "Kick longer",	"03. Kick",		0.0,	0.0,	24.0,	0.178,	2781.0,	0.0,	0.0,	0.005,	1.4,	0.0,	1.16,	0.0,	7.46,	0.707 }));
	presetValues.add(PresetValues({ "Kick tighter",	"03. Kick",		0.39,	2.0,	24.0,	0.059,	10000.,	0.0,	0.0,	0.005,	0.258,	0.0,	0.1,	0.0,	7.25,	0.707 }));
	presetValues.add(PresetValues({ "Kick tek",		"03. Kick",		0.56,	1.0,	18.8,	0.056,	590.,	0.0,	0.0,	0.001,	0.290,	0.02,	0.1,	0.0,	2.02,	3.62 }));

	presetValues.add(PresetValues({ "Mystic dive",	"04. Drop",		0.9,	2.0,	2.5,	1.45,	147.0,	0.0,	0.0,	0.64,	2.58,	0.0,	0.77,	0.0,	5.41,	3.157 }));
	presetValues.add(PresetValues({ "Sub drop",		"04. Drop",		0.77,	0.0,	12.0,	2.0,	60.0,	0.0,	0.0,	0.01,	0.94,	0.95,	0.52,	0.0,	7.77,	0.707 }));
	presetValues.add(PresetValues({ "SciFi drop",	"04. Drop",		1.0,	2.0,	14.0,	2.0,	186.0,	0.0,	0.0,	0.64,	2.58,	0.0,	0.77,	0.0,	9.95,	0.707 }));
	presetValues.add(PresetValues({ "This is war",	"04. Drop",		0.89,	1.0,	10.0,	2.0,	578.0,	0.0,	0.0,	2.0,	4.17,	0.0,	0.77,	0.0,	5.41,	2.02 }));

	presetValues.add(PresetValues({ "Glide sin",	"02. Sub",		0.0,	0.0,	0.0,	0.0,	10000.,	0.0,	0.0,	0.01,	0.1,	0.43,	0.1,	0.08,	6.79,	0.707 }));
	presetValues.add(PresetValues({ "Attack sin",	"02. Sub",		0.0,	0.0,	7.5,	0.06,	10000.,	0.0,	0.0,	0.002,	0.1,	0.43,	0.1,	0.0,	6.79,	0.707 }));
	presetValues.add(PresetValues({ "Deep sin",		"02. Sub",		0.3,	1.0,	1.9,	0.022,	511.,	0.0,	0.0,	0.14,	1.0,	0.15,	0.1,	0.0,	3.34,	0.707 }));

	presetValues.add(PresetValues({ "Simple sinus",	"05. Other",	0.0,	0.0,	0.0,	0.0,	1000.,	0.0,	0.0,	0.005,	0.001,	1.0,	0.005,	0.0,	0.0,	0.707 }));
	presetValues.add(PresetValues({ "Sin to square","05. Other",	0.5,	0.0,	0.0,	0.0,	1000.,	0.0,	0.0,	0.005,	0.001,	1.0,	0.005,	0.0,	0.0,	0.707 }));
	presetValues.add(PresetValues({ "Freaky",		"05. Other",	0.98,	2.0,	3.07,	0.08,	312.,	0.0,	0.0,	0.001,	0.044,	0.5,	0.005,	0.17,	0.0,	2.7 }));
	presetValues.add(PresetValues({ "Chunky toy",	"05. Other",	0.75,	1.0,	0.00,	0.001,	546.,	0.0,	0.0,	0.897,	0.013,	0.029,	0.075,	0.002,	0.0,	3.07 }));

	auto userFolder = getUserPresetsFolder();
	
	auto addParamChild = [](XmlElement* xml, String name, float value) {
		auto child = new XmlElement("PARAM");
		child->setAttribute("id", name);
		child->setAttribute("value", value);
		xml->addChildElement(child);
	};

	for (auto pv : presetValues)
	{
		const auto category = pv.category;
		const auto presetName = pv.name;
		const auto fileName = presetName + extension;

		auto f = userFolder.getChildFile(fileName);
		f.deleteFile();

		std::unique_ptr<XmlElement> xml(new XmlElement("BassGenParameters"));

		xml->setAttribute("PluginName",				JucePlugin_Name);
		xml->setAttribute("PluginVersion",			JucePlugin_VersionString);
		xml->setAttribute("PluginManufacturer",		JucePlugin_Manufacturer);

		xml->setAttribute("PresetName",				f.getFileNameWithoutExtension());
		xml->setAttribute("PresetCategory",			category);
		xml->setAttribute("PresetCreation",			Time::getCurrentTime().toString(true, true));

		addParamChild(xml.get(), ParameterIDs::drive,			pv.drive);
		addParamChild(xml.get(), ParameterIDs::driveType,		pv.driveType);
		addParamChild(xml.get(), ParameterIDs::bendAmount,		pv.bendAmount);
		addParamChild(xml.get(), ParameterIDs::bendDuration,	pv.bendDuration);
		addParamChild(xml.get(), ParameterIDs::lpFreq,			pv.lpFreq);
#if PAWG_ALLOW_LPF_MODULATION
		addParamChild(xml.get(), ParameterIDs::lpModAmount,		pv.lpModAmount);
		addParamChild(xml.get(), ParameterIDs::lpModDuration,	pv.lpModDuration);
#endif
		addParamChild(xml.get(), ParameterIDs::attack,			pv.attack);
		addParamChild(xml.get(), ParameterIDs::decay,			pv.decay);
		addParamChild(xml.get(), ParameterIDs::sustain,			pv.sustain);
		addParamChild(xml.get(), ParameterIDs::release,			pv.release);
		addParamChild(xml.get(), ParameterIDs::glide,			pv.glide);
		addParamChild(xml.get(), ParameterIDs::master,			pv.master);
		addParamChild(xml.get(), ParameterIDs::lpReso,			pv.lpReso);

		FileOutputStream fos(f);

		if (fos.openedOk())
			fos.writeText(xml->createDocument(""), false, false, nullptr);

		presetList.add(new Preset({ presetName, category, f, *xml, false }));
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
