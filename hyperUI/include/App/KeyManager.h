#pragma once

#define UIA_CHANGE_TOOL			"uiaChangeTool"
/********************************************************************************************/
enum SilentKeyType
{
	SilentKeyNone		= 0x00000000,
	SilentKeyControl	= 0x00010000,
	SilentKeyAlt		= 0x00020000,
	SilentKeyShift		= 0x00040000,
	SilentKeyLArrow		= 0x00080000,
	SilentKeyMacControl, //= 0x00100000,
	SilentKeyRArrow,
	SilentKeyUpArrow,
	SilentKeyDownArrow,
	SilentKeyTab,
	SilentKeyNumpadEnter,
	SilentKeyBackspace,
	SilentKeyEscape,
	SilentKeyDelete,
	SilentKeyF1,
	SilentKeyF2,
	SilentKeyF3,
	SilentKeyF4,
	SilentKeyF5,
	SilentKeyF6,
	SilentKeyF7,
	SilentKeyF8,
	SilentKeyF9,
	SilentKeyF10,
	SilentKeyF11,
	SilentKeyF12,
	SilentKeyCarriageReturn,
	SilentKeyEnd,
	SilentKeyHome,
	SilentKeyPageUp,
	SilentKeyPageDown,
};
/********************************************************************************************/
class HYPERUI_API KeyManager;
class IKeyManagerCallback
{
public:
	virtual ~IKeyManagerCallback() { }
	virtual void setDefaultShortcuts(KeyManager* pKeyManager) = 0;
};
/********************************************************************************************/
class HYPERUI_API KeyManager : public ResourceItem
{
public:

	virtual ~KeyManager();

	void setCallback(IKeyManagerCallback* pCallback);

	KeyMeaningType mapKey(int pcsKey, bool bControl, bool bAlt, bool bShift);
	const char* getUiActionFor(int pcsKey, bool bControl, bool bAlt, bool bShift);
	const char* getUiActionValueFor(int pcsKey, bool bControl, bool bAlt, bool bShift);
	void setKey(int pcsKey, KeyMeaningType eKeyMeaning, bool bControl = false, bool bAlt = false, bool bShift = false, const char* pcsUIAction = PROPERTY_NONE, const char* pcsUIActionValue = PROPERTY_NONE);

	// This, unlike the setKey, only works for action-based shortcuts, and it removes any previously exisitng
	// shortcuts for this value.
	void assignKey(const char* pcsUiAction, const char* pcsOptUiActionValue, int iKey, bool bControl, bool bAlt, bool bShift);
	void removeKey(const char* pcsUiAction, const char* pcsOptUiActionValue);

	void resetAll();

	void setIsPressed(KeyMeaningType eKey, bool bIsPressed);
	bool getIsPressed(KeyMeaningType eKey);

	static KeyManager* getInstance();

	bool load(const char* pcsOptPath = NULL);
	void save(const char* pcsOptPath = NULL);
	void loadDefaults();

	void resetToDefaults();

	void onModifierKeyDown(bool bControl, bool bAlt, bool bShift, bool bMacControl);
	void onModifierKeyUp(bool bControl, bool bAlt, bool bShift, bool bMacControl);

	bool getIsModifierKeyDown(SilentKeyType eKeyType, bool bMarkConsumed = false);
	bool getHasConsumedKey(SilentKeyType eKeyType);

	void getShortcutLetterByAction(const char* pcsUiAction, const char* pcsOptUiActionValue, string& strShortcutTextOut, bool& bControlOut, bool& bAltOut, bool& bShiftOut);
	void getShortcutTextByUiAction(const char* pcsUiAction, const char* pcsOptUiActionValue, const char* pcsOptUiActionValue2, string& strShortcutTextOut);

	SilentKeyType mapModifierKeyStringToType(const char* pcsType);

	int combineFlagsWithKey(int iKey, bool bControl, bool bAlt, bool bShift);

	inline void setIsSpacePressed(bool bValue) { myIsSpacePressed = bValue; }
	inline bool getIsSpacePressed() const { return myIsSpacePressed; }

	bool getIsCapsLockOn();

	void beginKeyboardFocus(UIElement* pElement);
	void endKeyboardFocus();

private:
	KeyManager();
	int findShortcutEnumPos(const char* pcsUiAction, const char* pcsOptUiActionValue);

private:

	static KeyManager* theInstance;

	bool myPressedKeys[KeyMeaningLastPlaceholder];

	bool myIsShiftDown, myIsAltDown, myIsControlDown, myIsMacControlDown;
	bool myHasConsumedShift, myHasConsumedAlt, myHasConsumedControl, myHasConsumedMacControl;

	bool myIsSpacePressed;
	IKeyManagerCallback* myCallback;
};
/********************************************************************************************/