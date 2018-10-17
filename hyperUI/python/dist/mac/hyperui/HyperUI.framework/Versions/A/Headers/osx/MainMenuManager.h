#pragma once

namespace HyperUI
{
/*****************************************************************************/
struct SNativeMenuCallbackInfo
{
	string myUiMenuId;
	int myEntryIndex;

	string myOptDirectAction;
};
typedef std::map < int, SNativeMenuCallbackInfo > TIntNativeCallbackInfoMap;
/*****************************************************************************/
class MainMenuManager
{
public:
	static MainMenuManager* getInstance();
	~MainMenuManager();

	void refreshMenuStatus(UIMenuElement* pMenuElem);
	void refreshMenuShortcuts(UIMenuElement* pMenuElem);
	void rebuildMenu(UIMenuElement* pMenuElem);

	void onNativeMenuWillOpen(NSMenu* menu);
	void onNativeCustomMenuItem(NSMenuItem* pMenuItem);
	void onNativeRecentFilesMenuItem(NSMenuItem* pMenuItem);

	void generateMainMenu(const char* pcsElemId, NSObject* pDelegate, IMenuContentsProvider* pRecentFilesProvider);

    NSMenuItem* findMenuItemByTagRecursive(int iTag, NSMenu* pMenu);
	void generateAndSetShortcut(const SMenuEntry* pEntry, NSMenuItem* pNativeItem);
	void generateAndSetShortcut(const char* pcsKeyOut, bool bControl, bool bAlt, bool bShift, NSMenuItem* pNativeItem);

	void createDefaultMenuItem(NSObject* pDelegate, const char* pcsAppTitle);

private:
	MainMenuManager();

	NSMenu* getNativeMenuFromUIMenu(UIMenuElement* pMenuElem);
	void removeCustomMenuItemsRecursive(NSMenu* pMenu);


	UIPlane* getUIPlane();

private:

	static MainMenuManager* theInstance;

	TIntNativeCallbackInfoMap myMainMenuCallbackInfos;
	NSObject* myDelegate;
	IMenuContentsProvider* myRecentFilesProvider;
};
/*****************************************************************************/
};
