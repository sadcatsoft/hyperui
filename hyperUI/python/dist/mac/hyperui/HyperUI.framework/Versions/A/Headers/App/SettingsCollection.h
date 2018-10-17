#pragma once

#define START_SETTINGS_ALLOW_RETINA		"setAllowRetina"

class UIElement;
/*****************************************************************************/
class HYPERUI_API ISettingsCallback
{
public:

	virtual ~ISettingsCallback() { }
	virtual void setDefaults(ResourceItem* pItem) = 0;
};
/*****************************************************************************/
class SettingsCollection : public ResourceCollection
{
public:

	// TEMP. Move to application class
	HYPERUI_API static SettingsCollection* getInstance();

	virtual ~SettingsCollection();
	
	HYPERUI_API void reload();
	HYPERUI_API void load();
	HYPERUI_API void save();

	inline bool getIsLoaded() const { return myIsLoaded; }	
	HYPERUI_API ResourceItem* getSettingsItem();

	HYPERUI_API void resetToDefaults();

	inline void setCallback(ISettingsCallback* pCallback) { myCallback = pCallback; }

	HYPERUI_API static StringResourceItem* getStartupSettings();
	HYPERUI_API static void synchSingleStartupSettingFromUI(const char* pcsProp, UIElement* pElem);
	HYPERUI_API static void saveStartupSettings();

private:
	SettingsCollection();

	static void ensureStartupSettingsExist();

private:
	bool myIsLoaded;

	static StringResourceCollection* theStartupSettings;

	ISettingsCallback* myCallback;

	static SettingsCollection* theInstance;
};
/*****************************************************************************/
