#include "stdafx.h"

#define KEY_MANAGER_SHORTCUT_FILE		"shortcuts.txt"

#define KEY_MANAGER_TAG		"srvKeyManager"

namespace HyperUI
{
KeyManager* KeyManager::theInstance = NULL;
/********************************************************************************************/
KeyManager::KeyManager()
	:ResourceItem(KEY_MANAGER_TAG)
{
	this->setStringProp(PropertyId, KEY_MANAGER_TAG);

	myCallback = NULL;
	myIsSpacePressed = false;
	memset(myPressedKeys, 0, sizeof(bool)*KeyMeaningLastPlaceholder);
	
	if(!load())
		loadDefaults();

	myHasConsumedShift = myHasConsumedAlt = myHasConsumedControl = myHasConsumedMacControl = false;
	myIsShiftDown = myIsAltDown = myIsControlDown = myIsMacControlDown = false;
}
/********************************************************************************************/
KeyManager::~KeyManager()
{
	delete myCallback;
	myCallback = NULL;
}
/********************************************************************************************/
KeyManager* KeyManager::getInstance()
{
	if(!theInstance)
		theInstance = new KeyManager();
	return theInstance;
}
/********************************************************************************************/
const char* KeyManager::getUiActionFor(int pcsKey, bool bControl, bool bAlt, bool bShift)
{
	// Handle control, alt.
	if((int)pcsKey >= 0 && (int)pcsKey < 256)
		pcsKey = tolower(pcsKey);
	int iFullCode = combineFlagsWithKey(pcsKey, bControl, bAlt, bShift);
	int iIndex = this->findNumInEnumProp(PropertyKmgrKeys, iFullCode);
	if(iIndex < 0)
		return PROPERTY_NONE;
	return this->getEnumPropValue(PropertyKmgrUiAction, iIndex);
}
/********************************************************************************************/
const char* KeyManager::getUiActionValueFor(int pcsKey, bool bControl, bool bAlt, bool bShift)
{
	// Handle control, alt.
	if((int)pcsKey >= 0 && (int)pcsKey < 256)
		pcsKey = tolower(pcsKey);
	int iFullCode = combineFlagsWithKey(pcsKey, bControl, bAlt, bShift);
	int iIndex = this->findNumInEnumProp(PropertyKmgrKeys, iFullCode);
	if(iIndex < 0)
		return PROPERTY_NONE;
	return this->getEnumPropValue(PropertyKmgrUiActionValue, iIndex);
}
/********************************************************************************************/
KeyMeaningType KeyManager::mapKey(int pcsKey, bool bControl, bool bAlt, bool bShift)
{
	// Handle control, alt.
	if((int)pcsKey >= 0 && (int)pcsKey < 256)
		pcsKey = tolower(pcsKey);
	int iFullCode = combineFlagsWithKey(pcsKey, bControl, bAlt, bShift);

	int iIndex = this->findNumInEnumProp(PropertyKmgrKeys, iFullCode);
	if(iIndex < 0)
		return KeyMeaningNone;
	return (KeyMeaningType)(int)this->getNumericEnumPropValue(PropertyKmgrMeanings, iIndex);
}
/********************************************************************************************/
void KeyManager::setKey(int pcsKey, KeyMeaningType eKeyMeaning, bool bControl, bool bAlt, bool bShift, const char* pcsUIAction, const char* pcsUIActionValue)
{
	_ASSERT(pcsKey != 0 || eKeyMeaning != 0);
	if((int)pcsKey >= 0 && (int)pcsKey < 256)
		pcsKey = tolower(pcsKey);
	if(pcsKey == 0)
		pcsKey = eKeyMeaning;
	int iFullCode = combineFlagsWithKey(pcsKey, bControl, bAlt, bShift);

	int iIndex = this->findNumInEnumProp(PropertyKmgrKeys, iFullCode);
	if(iIndex < 0)
	{
		// New entry
		// If we're zero, we're going to be reacting to cases
		// where nothing is pressed, but, say control is and we
		// move the mouse, and we will create the trasnform node!
		_ASSERT(iFullCode != 0);
		this->addToNumericEnumProp(PropertyKmgrKeys, iFullCode);
		this->addToNumericEnumProp(PropertyKmgrMeanings, eKeyMeaning);
		this->addToEnumProp(PropertyKmgrUiAction, pcsUIAction);
		this->addToEnumProp(PropertyKmgrUiActionValue, pcsUIActionValue);
	}
	else
	{
		// Overwriting an existing entry
		this->setNumericEnumPropValue(PropertyKmgrMeanings, iIndex, eKeyMeaning);
		this->setEnumProp(PropertyKmgrUiAction, iIndex, pcsUIAction);
		this->setEnumProp(PropertyKmgrUiActionValue, iIndex, pcsUIActionValue);
	}
}
/********************************************************************************************/
void KeyManager::loadDefaults()
{
	this->clearModifiedProps();

	this->setKey(SilentKeyLArrow, KeyMeaningLeft);
	this->setKey(SilentKeyRArrow, KeyMeaningRight);
	this->setKey(SilentKeyUpArrow, KeyMeaningUp);
	this->setKey(SilentKeyDownArrow, KeyMeaningDown);
	this->setKey(SilentKeyTab, KeyMeaningTab);
	this->setKey(SilentKeyNumpadEnter, KeyMeaningEnter);
	this->setKey(SilentKeyDelete, KeyMeaningDelete);
	this->setKey(SilentKeyEscape, KeyMeaningEscape);

	if(myCallback)
		myCallback->setDefaultShortcuts(this);
}
/********************************************************************************************/
void KeyManager::save(const char* pcsOptPath)
{
	string strTemp;
	this->setTag("keys");
	this->setStringProp(PropertyId, "keys");
	this->saveToString(strTemp);
	if(pcsOptPath)
		FileUtils::saveToFile(pcsOptPath, strTemp.c_str(), true);
	else
		FileUtils::saveToFile(KEY_MANAGER_SHORTCUT_FILE, strTemp.c_str());
}
/********************************************************************************************/
bool KeyManager::load(const char* pcsOptPath)
{
	ResourceCollection rFakeColl;
	bool bRes;
	if(pcsOptPath)
		bRes = rFakeColl.loadFromFile(pcsOptPath, FileSourceFullPath, NULL);
	else
		bRes = rFakeColl.loadFromFile(KEY_MANAGER_SHORTCUT_FILE, FileSourceUserDir, NULL);
	if(!bRes || rFakeColl.getNumItems() <= 0)
		return false;
	this->mergeResourcesFrom(*rFakeColl.getItem(0), false);	

	return bRes;
}
/********************************************************************************************/
void KeyManager::setIsPressed(KeyMeaningType eKey, bool bIsPressed)
{
	if(eKey == KeyMeaningLastPlaceholder || eKey == KeyMeaningNone)
		return;
	myPressedKeys[eKey] = bIsPressed;
}
/********************************************************************************************/
bool KeyManager::getIsPressed(KeyMeaningType eKey)
{
	if(eKey != KeyMeaningLastPlaceholder)
		return myPressedKeys[eKey];
	else
		return false;
}
/********************************************************************************************/
void KeyManager::resetAll()
{
	int iCurr;
	for(iCurr = 0; iCurr < KeyMeaningLastPlaceholder; iCurr++)
		myPressedKeys[iCurr] = false;
}
/********************************************************************************************/
void KeyManager::onModifierKeyDown(bool bControl, bool bAlt, bool bShift, bool bMacControl)
{
	myIsShiftDown = bShift;	
	myIsAltDown = bAlt;
	myIsControlDown = bControl;
	myIsMacControlDown = bMacControl;

    if(bShift)
		myHasConsumedShift = false;
	if(bAlt)
		myHasConsumedAlt = false; 
	if(bControl)
		myHasConsumedControl = false;
	if(bMacControl)
		myHasConsumedMacControl = false;

	//gLog("MOD_KEY_DOWN: %s\n", myIsShiftDown ? "YES" : "NO");
}
/********************************************************************************************/
void KeyManager::onModifierKeyUp(bool bControl, bool bAlt, bool bShift, bool bMacControl)
{
	if(bControl)
		myIsControlDown = false;
	if(bAlt)
		myIsAltDown = false;
	if(bShift)
		myIsShiftDown = false;
	if(bMacControl)
		myIsMacControlDown = false;

	//gLog("MOD_KEY_UP: %s\n", myIsShiftDown ? "YES" : "NO");
}
/********************************************************************************************/
bool KeyManager::getIsModifierKeyDown(SilentKeyType eKeyType, bool bMarkConsumed)
{
	if(eKeyType == SilentKeyShift)
	{
		if(bMarkConsumed)
			myHasConsumedShift = true;
		return myIsShiftDown;
	}
	else if(eKeyType == SilentKeyAlt)
	{
		if(bMarkConsumed)
			myHasConsumedAlt = true;
		return myIsAltDown;
	}
	else if(eKeyType == SilentKeyControl)
	{
		if(bMarkConsumed)
			myHasConsumedControl = true;
		return myIsControlDown;
	}
	else if(eKeyType == SilentKeyMacControl)
	{
		if(bMarkConsumed)
			myHasConsumedMacControl = true;
		return myIsMacControlDown;
	}
	
	_ASSERT(0);
	return false;
}
/********************************************************************************************/
void KeyManager::getShortcutTextByUiAction(const char* pcsUiAction, const char* pcsOptUiActionValue, const char* pcsOptUiActionValue2, string& strShortcutTextOut)
{
	strShortcutTextOut = "";
	int iPos = findShortcutEnumPos(pcsUiAction, pcsOptUiActionValue);
	// We currently don't support secondary action values...
	if(iPos < 0 || (pcsOptUiActionValue2 && strlen(pcsOptUiActionValue2) > 0) )
		return;

	// See which keys are pressed
	int iFullCode = this->getNumericEnumPropValue(PropertyKmgrKeys, iPos);
	
	if(iFullCode & SilentKeyControl)
	{
#if defined(WIN32) || defined(LINUX)
		strShortcutTextOut += "Ctrl+";
#else
		strShortcutTextOut += "Cmd+";
#endif
	}
	if(iFullCode & SilentKeyAlt)
		strShortcutTextOut += "Alt+";
	if(iFullCode & SilentKeyShift)
		strShortcutTextOut += "Shift+";

	int iNonModCode = iFullCode & (~(SilentKeyControl | SilentKeyAlt | SilentKeyShift));

	// Now, get the actual letter
	bool bHaveSet = false;
	if(iNonModCode >= SilentKeyF1 && iNonModCode <= SilentKeyF12)
	{
		int iFIndex = (iNonModCode - (int)SilentKeyF1) + 1;
		char pcsBuff[32];
		sprintf(pcsBuff, "F%d", iFIndex);
		strShortcutTextOut += pcsBuff;
		bHaveSet = true;
	}
	else if(iNonModCode == KeyMeaningTab || iNonModCode == SilentKeyTab)
	{
		strShortcutTextOut += "Tab";
		bHaveSet = true;
	}

	if(!bHaveSet)
	{
		// The modifiers are stored in the high bits. Just get
		// the lower ones.
		char cLetter[2] = { (char)iFullCode, 0 };
		_ASSERT( (int)cLetter[0] >= 0 && (int)cLetter[0] <= 255);

		cLetter[0] = toupper(cLetter[0]);
		strShortcutTextOut += cLetter;
	}
}
/*****************************************************************************/
SilentKeyType KeyManager::mapModifierKeyStringToType(const char* pcsType)
{
	if(IS_STRING_EQUAL(pcsType, "Ctrl"))
		return SilentKeyControl;
	else if(IS_STRING_EQUAL(pcsType, "Shift"))
		return SilentKeyShift;
	else if(IS_STRING_EQUAL(pcsType, "Alt"))
		return SilentKeyAlt;
	else
		return SilentKeyNone;
}
/*****************************************************************************/
bool KeyManager::getHasConsumedKey(SilentKeyType eKeyType)
{
	if(eKeyType == SilentKeyControl)
		return myHasConsumedControl;
	else if(eKeyType == SilentKeyAlt)
		return myHasConsumedAlt;
	else if(eKeyType == SilentKeyShift)
		return myHasConsumedShift;
	else if(eKeyType == SilentKeyMacControl)
		return myHasConsumedMacControl;
	ELSE_ASSERT;

	return false;
}
/*****************************************************************************/
int KeyManager::combineFlagsWithKey(int iKey, bool bControl, bool bAlt, bool bShift)
{
	int iRes = iKey;
	if(bControl)
		iRes |= SilentKeyControl;
	if(bAlt)
		iRes |= SilentKeyAlt;
	if(bShift)
		iRes |= SilentKeyShift;

	return iRes;
}
/*****************************************************************************/
void KeyManager::getShortcutLetterByAction(const char* pcsUiAction, const char* pcsOptUiActionValue, string& strShortcutTextOut, bool& bControlOut, bool& bAltOut, bool& bShiftOut)
{
	bControlOut = bAltOut = bShiftOut = false;
	strShortcutTextOut = "";
	int iPos = findShortcutEnumPos(pcsUiAction, pcsOptUiActionValue);
	if(iPos < 0)
		return;

	// See which keys are pressed
	int iFullCode = this->getNumericEnumPropValue(PropertyKmgrKeys, iPos);

	bControlOut = (iFullCode & SilentKeyControl);
	bAltOut = (iFullCode & SilentKeyAlt);
	bShiftOut = (iFullCode & SilentKeyShift);

	int iNonModCode = iFullCode & (~(SilentKeyControl | SilentKeyAlt | SilentKeyShift));

	// Now, get the actual letter
	bool bHaveSet = false;
	if(iNonModCode >= SilentKeyF1 && iNonModCode <= SilentKeyF12)
	{
		int iFIndex = (iNonModCode - (int)SilentKeyF1) + 1;
		char pcsBuff[32];
		sprintf(pcsBuff, "F%d", iFIndex);
		strShortcutTextOut += pcsBuff;
		bHaveSet = true;
	}

	if(!bHaveSet)
	{
		// The modifiers are stored in the high bits. Just get
		// the lower ones.
		char cLetter[2] = { (char)iFullCode, 0 };
		_ASSERT( (int)cLetter[0] >= 0 && (int)cLetter[0] <= 255);

		cLetter[0] = toupper(cLetter[0]);
		strShortcutTextOut += cLetter;
	}
}
/*****************************************************************************/
int KeyManager::findShortcutEnumPos(const char* pcsUiAction, const char* pcsOptUiActionValue)
{
	if(strlen(pcsUiAction) == 0 || strcmp(pcsUiAction, PROPERTY_NONE) == 0)
		return -1;

	int iPos = -1;
	if(pcsOptUiActionValue && !IS_STRING_EQUAL(pcsOptUiActionValue, PROPERTY_NONE) && strlen(pcsOptUiActionValue) > 0)
	{
		// Go through all matching ui actions, look for also matching
		// action value. Needed for general uiaShow/hide items...
		// I.e. actions with the same action but different values.
		int iCurrEntry, iNumEntries = this->getEnumPropCount(PropertyKmgrUiAction);
		for(iCurrEntry = 0; iCurrEntry < iNumEntries; iCurrEntry++)
		{
			if(IS_STRING_EQUAL(this->getEnumPropValue(PropertyKmgrUiAction, iCurrEntry), pcsUiAction))
			{
				// See if our action values are the same
				if(IS_STRING_EQUAL(this->getEnumPropValue(PropertyKmgrUiActionValue, iCurrEntry), pcsOptUiActionValue))
				{
					iPos = iCurrEntry;
					break;
				}
			}
		}
	}
	else
		iPos = this->findStringInEnumProp(PropertyKmgrUiAction, pcsUiAction);
	
	return iPos;
}
/*****************************************************************************/
void KeyManager::removeKey(const char* pcsUiAction, const char* pcsOptUiActionValue)
{
	int iIndex = this->findShortcutEnumPos(pcsUiAction, pcsOptUiActionValue);
	if(iIndex < 0)
		return;

	this->removeEnumValue(PropertyKmgrKeys, iIndex);
	this->removeEnumValue(PropertyKmgrMeanings, iIndex);
	this->removeEnumValue(PropertyKmgrUiAction, iIndex);
	this->removeEnumValue(PropertyKmgrUiActionValue, iIndex);
}
/*****************************************************************************/
void KeyManager::assignKey(const char* pcsUiAction, const char* pcsOptUiActionValue, int iKey, bool bControl, bool bAlt, bool bShift)
{
	// Remove any shortcuts for this action/value combo.
	char pcsKey = tolower(iKey);
	if(pcsKey == 0)
		return;
	KeyManager::getInstance()->removeKey(pcsUiAction, pcsOptUiActionValue);

	// Now, we may have the same key combo assigned somewhere else. Make sure we
	// remove those, too:
	int iIndex;
	int iFullCode = combineFlagsWithKey(pcsKey, bControl, bAlt, bShift);
	for(iIndex = this->findNumInEnumProp(PropertyKmgrKeys, iFullCode); iIndex >= 0; iIndex = this->findNumInEnumProp(PropertyKmgrKeys, iFullCode))
	{
		this->removeEnumValue(PropertyKmgrKeys, iIndex);
		this->removeEnumValue(PropertyKmgrMeanings, iIndex);
		this->removeEnumValue(PropertyKmgrUiAction, iIndex);
		this->removeEnumValue(PropertyKmgrUiActionValue, iIndex);
	}

	// Now, assign the new key
	KeyManager::getInstance()->setKey(iKey, KeyMeaningUiAction, bControl, bAlt, bShift, pcsUiAction, pcsOptUiActionValue);
}
/*****************************************************************************/
void KeyManager::resetToDefaults()
{
	loadDefaults();

	// Delete the saved file, if it exists:
	if(FileUtils::doesFileExist(KEY_MANAGER_SHORTCUT_FILE, FileSourceUserDir))
		FileUtils::deleteFile(KEY_MANAGER_SHORTCUT_FILE);
}

/*****************************************************************************/
void KeyManager::setCallback(IKeyManagerCallback* pCallback)
{
	if(myCallback)
		delete myCallback;
	myCallback = pCallback;

	// We need to reload our shortcuts in case the defaults have changed:
	this->clearModifiedProps();
	if(!load())
		loadDefaults();
}
/*****************************************************************************/
};