#include "stdafx.h"

#define SETTINGS_ITEM_NAME	"mainSettings"

#define STARTUP_SETTINGS_FILE		"startup_settings.txt"
#define SETTINGS_FILE				"settings.txt"

namespace HyperUI
{
StringResourceCollection* SettingsCollection::theStartupSettings = NULL;
SettingsCollection* SettingsCollection::theInstance = NULL;
/*****************************************************************************/
SettingsCollection::SettingsCollection()
{
	myIsLoaded = false;	
	myCallback = NULL;
}
/*****************************************************************************/
SettingsCollection* SettingsCollection::getInstance()
{
	if(!theInstance)
		theInstance = new SettingsCollection;
	return theInstance;
}
/*****************************************************************************/
SettingsCollection::~SettingsCollection()
{
	delete myCallback;
	myCallback = NULL;
}
/*****************************************************************************/
void SettingsCollection::resetToDefaults()
{
	ResourceItem* pItem = getSettingsItem();

	if(myCallback)
		myCallback->setDefaults(pItem);
}
/*****************************************************************************/
ResourceItem* SettingsCollection::getSettingsItem()
{
	ResourceItem* pItem = this->getItemById(SETTINGS_ITEM_NAME);
	if(!pItem)
	{
		// Create one
		pItem = this->addItem(SETTINGS_ITEM_NAME, SETTINGS_ITEM_NAME);
		resetToDefaults();
	}
	
	return pItem;
}
/*****************************************************************************/
void SettingsCollection::reload()
{
	if(!this->loadFromFile(SETTINGS_FILE, FileSourceUserDir, NULL))
	{
		resetToDefaults();
		save();
	}

	myIsLoaded = true;
}
/*****************************************************************************/
void SettingsCollection::load()
{
	if(myIsLoaded)
		return;

	reload();
}
/*****************************************************************************/
void SettingsCollection::save()
{
	ResourceItem* pItem = this->getSettingsItem();
/*
	// This is slightly different in that any settings with UI are actually
	// stored in the UI. So we have to read them from there, and set them
	// again before storing them to file (since this code is also what gets
	// called when any of those are changed).
	pItem->setNumProp(PropertySettingDifficulty, pParentEngine->getGameDiff(false));
	pItem->setBoolProp(PropertySettingSoundOn, pParentEngine->getIsSoundOn());
	pItem->setBoolProp(PropertySettingMusicOn, pParentEngine->getIsMusicOn());
*/
	string strTemp;
	this->saveToString(strTemp);
	FileUtils::saveToFile(SETTINGS_FILE, strTemp.c_str());
}
/*****************************************************************************/
void SettingsCollection::ensureStartupSettingsExist()
{
	if(FileUtils::doesFileExist(STARTUP_SETTINGS_FILE, FileSourceUserDir))
		return;

	// Write a default one
	StringResourceCollection rColl;
	StringResourceItem* pResItem = rColl.addItem("set", "set");

	string strTemp;
	rColl.saveToString(strTemp);
	FileUtils::saveToFile(STARTUP_SETTINGS_FILE, strTemp.c_str());
}
/*****************************************************************************/
StringResourceItem* SettingsCollection::getStartupSettings()
{
	if(!theStartupSettings)
	{
		ensureStartupSettingsExist();

		theStartupSettings = new StringResourceCollection;
		bool bRes = theStartupSettings->loadFromFile(STARTUP_SETTINGS_FILE, FileSourceUserDir);
		_ASSERT(bRes);

		if(theStartupSettings->getNumItems() == 0)
		{
			// Add a dummy one to avoid crashing
			theStartupSettings->addItem("set", "set");
		}
	}

	_ASSERT(theStartupSettings->getNumItems() > 0);
	return theStartupSettings->getItem(0);
}
/*****************************************************************************/
void SettingsCollection::synchSingleStartupSettingFromUI(const char* pcsProp, UIElement* pElem)
{
	bool bDidHandle = false;
	if(IS_STRING_EQUAL(pcsProp, START_SETTINGS_ALLOW_RETINA))
	{
		getStartupSettings()->setBoolProp(pcsProp, as<UICheckboxElement>(pElem)->getIsPushed());		
		bDidHandle = true;
	}
// 	else if(myCallback)
// 		bDidHandle = myCallback->synchSingleStartupSettingFromUI(pcsProp, pElem);

	_ASSERT(bDidHandle);
}
/*****************************************************************************/
void SettingsCollection::saveStartupSettings()
{
	ensureStartupSettingsExist();
	string strTemp;
	theStartupSettings->saveToString(strTemp);
	FileUtils::saveToFile(STARTUP_SETTINGS_FILE, strTemp.c_str());
}
/*****************************************************************************/
};