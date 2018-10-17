#import <Cocoa/Cocoa.h>
#include "stdafx.h"
#include "MacCommon.h"

#ifdef _DEBUG
//#define DEBUG_MENU_GENERATION
#endif

#define INIT_MENU_TAG_COUNT					400

typedef std::map < string, NSMenuItem* > TStringNSMenuItemMap;

namespace HyperUI
{
HyperUI::MainMenuManager* MainMenuManager::theInstance = NULL;
/*****************************************************************************/
class MacMenuReconstructor : public IMenuVisitor
{
public:

	MacMenuReconstructor(TIntNativeCallbackInfoMap *pMainMenuCallbackInfos, NSObject* pDelegate, IMenuContentsProvider* pRecentFilesProvider)
	{ 
		myMainMenuCallbackInfos = pMainMenuCallbackInfos; 
		myDelegate = pDelegate;
		myRecentFilesProvider = pRecentFilesProvider;
	}

	virtual ~MacMenuReconstructor() 
	{
 
	}

	virtual void visit(SMenuEntry* pEntry, UIMenuElement* pParent, int iEntryIndex)
	{
        if(pParent && (IS_STRING_EQUAL(pParent->getStringProp(PropertyId), "recentOpenedFiles") ||
                       IS_STRING_EQUAL(pParent->getStringProp(PropertyId), "recentSavedFiles")))
            return;

#ifdef DEBUG_MENU_GENERATION
			gLog("MENU_GEN: Visiting parent %s\n", pParent ? pParent->getStringProp(PropertyId) : "NULL");
#endif
        // This assumes we have no previous menu
        const int iMainMenuIndexOffset = 0;
        NSMenu* pMainMenu = [NSApp mainMenu];
        
		NSMenuItem* pParentMenuItem = nil;
		NSMenu* pMenuToAddTo = pMainMenu;
		const char* pcsParentTypename = pParent ? pParent->getStringProp(PropertyId) : "";
		if(pParent && !pParent->hasTag("toplevelmenu"))
		{
			UIMenuElement* pGrandParent = pParent->getParentMenuElem();
			int iParentEntryIndex = -1;
            const SMenuEntry* pGrandParentEntry = NULL;
            if(pGrandParent)
            {
				iParentEntryIndex = pGrandParent->findSubmenuIndex(pcsParentTypename);
                pGrandParentEntry = pGrandParent->getEntry(iParentEntryIndex);
            }
			if(pGrandParentEntry)
			{
				pParentMenuItem = MainMenuManager::getInstance()->findMenuItemByTagRecursive(pGrandParentEntry->myMacTag, pMainMenu);
				pMenuToAddTo = pParentMenuItem.submenu;
			}
		}

		NSMenuItem* pAddedItem = nil;
		
        
		if(pEntry->myId == SEPARATOR_ID)
			[pMenuToAddTo addItem:[NSMenuItem separatorItem]];
		else
		{
            if(pMenuToAddTo == pMainMenu)
            {
                pAddedItem = [pMenuToAddTo insertItemWithTitle:[NSString stringWithCString:pEntry->myLabel.c_str()] action:@selector(onCustomMenuItem:) keyEquivalent:@"" atIndex:([pMenuToAddTo.itemArray count] - iMainMenuIndexOffset)];
            }
            else
                pAddedItem = [pMenuToAddTo addItemWithTitle:[NSString stringWithCString:pEntry->myLabel.c_str()] action:@selector(onCustomMenuItem:) keyEquivalent:@""];

			SNativeMenuCallbackInfo rInfo;
			rInfo.myUiMenuId = pcsParentTypename;
			rInfo.myEntryIndex = iEntryIndex;

			// Assign our tag
			pAddedItem.tag = theMenuTagCounter;
			(*myMainMenuCallbackInfos)[theMenuTagCounter] = rInfo;
            if(pParent)
                pParent->setEntryTag(iEntryIndex, theMenuTagCounter);
#ifdef DEBUG_MENU_GENERATION
			gLog("MENU_GEN: Tag %d created for %s\n", theMenuTagCounter, pEntry->myLabel.c_str());
#endif
			theMenuTagCounter++;

			// Set the shortcut
			MainMenuManager::getInstance()->generateAndSetShortcut(pEntry, pAddedItem);
		}

		// Now, if we're a submenu, add an empty menu to it and add the item itself to a map
		NSMenu* pSubmenu = NULL;
        bool bIsEmptyTopLevelItem = (pMenuToAddTo == pMainMenu) && (pEntry->mySubmenu.length() == 0);
		if( (pEntry->mySubmenu.length() > 0 || bIsEmptyTopLevelItem) && pAddedItem)
		{
		    pSubmenu = [[NSMenu alloc] initWithTitle:[NSString stringWithCString:pEntry->myLabel.c_str()]];
            [pSubmenu setAutoenablesItems:NO];
			pSubmenu.delegate = myDelegate;
			[pAddedItem setSubmenu:pSubmenu];
		}	
		
		if(pEntry->mySubmenu == "recentFilesSubmenu" && pSubmenu)
		{
			// Special case - our recent files submenu.
			//RecentFilesMenuProvider rOpenProvider(NULL, false, "", 1, 0);
			const char* pcsItemLabel;
			const char* pcsActionValue;
			NSString* strFullPath;
			int iCurr, iNum = myRecentFilesProvider->getNumItems();
			for(iCurr = 0; iCurr < iNum; iCurr++)
			{
				pcsItemLabel = myRecentFilesProvider->getItemLabel(iCurr);
				pcsActionValue = myRecentFilesProvider->getItemActionValue(iCurr);
				strFullPath = [NSString stringWithCString: pcsActionValue];

                pAddedItem = [pSubmenu addItemWithTitle:[NSString stringWithCString:pcsItemLabel] action:@selector(onCustomRecentFilesMenuItem:) keyEquivalent:@""];
				pAddedItem.representedObject = strFullPath;

                SNativeMenuCallbackInfo rInfo;
                rInfo.myUiMenuId = pEntry->mySubmenu;
                rInfo.myEntryIndex = iCurr;
                
				// Assign our tag
				pAddedItem.tag = theMenuTagCounter;
				(*myMainMenuCallbackInfos)[theMenuTagCounter] = rInfo;
				pParent->setEntryTag(iEntryIndex, theMenuTagCounter);
#ifdef DEBUG_MENU_GENERATION
			gLog("MENU_GEN: Recent tag %d created for %s\n", theMenuTagCounter, pEntry->myLabel.c_str());
#endif
				theMenuTagCounter++;
			}
		}

		//gLog("MENU ENUM: %s -> %s\n", pcsParentTypename, pEntry->myLabel.c_str());
	}

private:

	TIntNativeCallbackInfoMap *myMainMenuCallbackInfos;
	NSObject* myDelegate;
	IMenuContentsProvider* myRecentFilesProvider;

	static int theMenuTagCounter;
};
int MacMenuReconstructor::theMenuTagCounter = INIT_MENU_TAG_COUNT;
/*****************************************************************************/
MainMenuManager::MainMenuManager()
{
	myDelegate = NULL;
	myRecentFilesProvider = NULL;
}
/*****************************************************************************/
MainMenuManager::~MainMenuManager()
{

}
/*****************************************************************************/
HyperUI::MainMenuManager* MainMenuManager::getInstance()
{
	if(!theInstance)
		theInstance = new MainMenuManager();
	return theInstance;
}
/*****************************************************************************/
void MainMenuManager::createDefaultMenuItem(NSObject* pDelegate, const char* pcsAppTitle)
{
    NSMenu* pMainMenu = [NSApp mainMenu];
    if(pMainMenu)
		return;
    
    pMainMenu = [[NSMenu alloc] initWithTitle:@"test"];
    NSApp.mainMenu = pMainMenu;
    pMainMenu = [NSApp mainMenu];
    
    // Add a menu item that acts like the main item
    NSMenuItem* pAppItem = [pMainMenu insertItemWithTitle:@"maintest\x1b" action:@selector(onCustomMenuItem:) keyEquivalent:@"" atIndex:0];
    
	// Add the submenu
	NSMenu* pSubmenu = [[NSMenu alloc] initWithTitle:@"submenu"];
    [pSubmenu setAutoenablesItems:NO];
	pSubmenu.delegate = pDelegate;
	[pAppItem setSubmenu:pSubmenu];

	// Quit item
    //NSString *appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:(id)kCFBundleNameKey];
	NSString *appName = [NSString stringWithCString:pcsAppTitle];
    NSString *pQuitShortcutName = [NSString stringWithFormat:@"Quit %@", appName];
	NSMenuItem* pQuitItem = [pSubmenu addItemWithTitle:pQuitShortcutName action:@selector(onCustomMenuItem:) keyEquivalent:@""];
	this->generateAndSetShortcut("q", true, false, false, pQuitItem);

    SNativeMenuCallbackInfo rInfo;
    rInfo.myOptDirectAction = UIA_QUIT_APP;
    rInfo.myEntryIndex = -1;
                
	// Assign our tag
	int iTagToUse = INIT_MENU_TAG_COUNT - 1;
	pQuitItem.tag = iTagToUse;
	myMainMenuCallbackInfos[iTagToUse] = rInfo;
#ifdef DEBUG_MENU_GENERATION
	gLog("MENU_GEN: Tag %d created for quit item\n", iTagToUse);
#endif

    // This is a hack to make the first item be the app name
    NSMenu *menu = [[[NSApp mainMenu] itemAtIndex:0] submenu];
    // Append some invisible character to title :)
    NSString *title = [appName stringByAppendingString:@"\x1b"];
    [menu setTitle:title];
}
/*****************************************************************************/
void MainMenuManager::refreshMenuStatus(UIMenuElement* pMenuElem)
{
    // No refresh until we've created it.
    if(myMainMenuCallbackInfos.size() == 0)
        return;
    
	NSMenu* pSubmenu = this->getNativeMenuFromUIMenu(pMenuElem);
	if(!pSubmenu)
		return;

	// Otherwise, refresh all elements in this menu. Non-recusrive.
	NSMenuItem* pFoundItem;
	const SMenuEntry* pEntry;
	int iCurr, iNum = pMenuElem->getNumEntries();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pEntry = pMenuElem->getEntry(iCurr);
        if(pEntry->myMacTag < 0)
            continue;

		pFoundItem = [pSubmenu itemWithTag:pEntry->myMacTag];
		
		//if(!pFoundItem)
		//	ASSERT_CONTINUE;

		// Otherwise, set enabled status:
		pFoundItem.enabled = pEntry->myIsEnabled;

		if(pEntry->myIsCheckmark && pEntry->myIsChecked)
			pFoundItem.state = NSOnState;
		else
			pFoundItem.state = NSOffState;
	}
}
/*****************************************************************************/
void MainMenuManager::refreshMenuShortcuts(UIMenuElement* pMenuElem)
{
    // No refresh until we've created it.
    if(myMainMenuCallbackInfos.size() == 0)
        return;
    
	NSMenu* pSubmenu = this->getNativeMenuFromUIMenu(pMenuElem);
	if(!pSubmenu)
		return;

	// Otherwise, refresh all elements in this menu. Non-recusrive.
	NSMenuItem* pFoundItem;
	const SMenuEntry* pEntry;
	int iCurr, iNum = pMenuElem->getNumEntries();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pEntry = pMenuElem->getEntry(iCurr);
        if(pEntry->myMacTag < 0)
            continue;

		pFoundItem = [pSubmenu itemWithTag:pEntry->myMacTag];
		
		if(!pFoundItem)
			ASSERT_CONTINUE;

		// Otherwise, set enabled status:
		this->generateAndSetShortcut(pEntry, pFoundItem);
	}
}
/*****************************************************************************/
void MainMenuManager::rebuildMenu(UIMenuElement* pMenuElem)
{
#ifdef DEBUG_MENU_GENERATION
	gLog("MENU_GEN: Rebuilding menu %s\n", pMenuElem ? pMenuElem->getStringProp(PropertyId) : "NULL");
#endif

	// Note that this must be recursive, by definition,
	// since we're removing items that may have entire submenus with them...
	NSMenu *pMenu = this->getNativeMenuFromUIMenu(pMenuElem);
	this->removeCustomMenuItemsRecursive(pMenu);

	// Now, generate it again:
	MacMenuReconstructor rVisitor(&myMainMenuCallbackInfos, myDelegate, myRecentFilesProvider);
	pMenuElem->iterateOverMenuItems(rVisitor);

}
/*****************************************************************************/
NSMenu* MainMenuManager::getNativeMenuFromUIMenu(UIMenuElement* pMenuElem)
{
	NSMenu* pSubmenu = nil;

	// So we're refreshing a menu. Now we need to find
	// its parent item in another menu element
	UIMenuElement* pParentMenuElem = pMenuElem->getParentMenuElem();
	if(!pParentMenuElem)
	{
		if(pMenuElem->hasTag("toplevelmenu"))
			pSubmenu = [NSApp mainMenu];
		else
            return nil;
	}
	else
	{
		int iIndex = pParentMenuElem->findSubmenuIndex(pMenuElem->getStringProp(PropertyId));
		const SMenuEntry* pParentEntry = pParentMenuElem->getEntry(iIndex);
		if(!pParentEntry)
			ASSERT_RETURN_NULL;

		// Find the NSMenu with the same tag. Note that this may not 
		// be the main menu, so we won't necessarily find it.
		int iTag = pParentEntry->myMacTag;
        if(iTag < 0)
            return nil;
        
#ifdef _DEBUG
        if(iTag == 481 || iTag == 491)
        {
            int bp = 0;
        }
#endif
        
		NSMenuItem* pMenuItem = this->findMenuItemByTagRecursive(iTag, [NSApp mainMenu]);
		if(!pMenuItem)
			ASSERT_RETURN_NULL;

		pSubmenu = [pMenuItem submenu];
	}

	return pSubmenu;
}
/*****************************************************************************/
void MainMenuManager::generateAndSetShortcut(const SMenuEntry* pEntry, NSMenuItem* pNativeItem)
{
	bool bControl, bAlt, bShift;
	string strKeyOut;
	KeyManager::getInstance()->getShortcutLetterByAction(pEntry->myAction.c_str(), pEntry->myActionValue.c_str(), strKeyOut, bControl, bAlt, bShift);
	this->generateAndSetShortcut(strKeyOut.c_str(), bControl, bAlt, bShift, pNativeItem);
}
/*****************************************************************************/
void MainMenuManager::generateAndSetShortcut(const char* pcsKeyOut, bool bControl, bool bAlt, bool bShift, NSMenuItem* pNativeItem)
{
	string strKeyOut(pcsKeyOut);
	std::transform(strKeyOut.begin(), strKeyOut.end(), strKeyOut.begin(), ::tolower);

	char pcsBuff[2] = { 0, 0 };
	if(strKeyOut.length() > 0)
	{
		char cFirst = strKeyOut[0];
		if(cFirst == 0x05)
		{
			pcsBuff[0] = NSTabCharacter;
		}
	}
        
	if(pcsBuff[0] == 0)
		pNativeItem.keyEquivalent = [NSString stringWithCString:strKeyOut.c_str()];
	else
		pNativeItem.keyEquivalent = [NSString stringWithCString:pcsBuff];

	// Key mask
	NSUInteger keyMask = 0;
	if(bControl)
		keyMask |= NSCommandKeyMask;
	if(bAlt)
		keyMask |= NSAlternateKeyMask;
	if(bShift)
		keyMask |= NSShiftKeyMask;
	pNativeItem.keyEquivalentModifierMask = keyMask;	
}
/*****************************************************************************/
void MainMenuManager::onNativeMenuWillOpen(NSMenu* menu)
{
    Application::lockGlobal();
	NSMenu* pParent = menu.supermenu;
	int iIndex = [pParent indexOfItemWithSubmenu:menu];
	NSMenuItem* pMenuItem = [pParent itemAtIndex:iIndex];

	TIntNativeCallbackInfoMap::iterator mi = myMainMenuCallbackInfos.find(pMenuItem.tag);
	if(mi == myMainMenuCallbackInfos.end())
    {
        Application::unlockGlobal();
        return;
    }

	SNativeMenuCallbackInfo *pInfo = &mi->second;
	UIMenuElement* pParentMenu = this->getUIPlane()->getElementById<UIMenuElement>(pInfo->myUiMenuId.c_str());
	const SMenuEntry* pUiMenuEntry = pParentMenu->getEntry(pInfo->myEntryIndex);

	// Now, we need to find *that* submenu and refresh it:
 
    // This is normal in some cases - Apple doesn't allow non-submenu items
    // in the top menu, so sometimes we generate fakes ourselves.
	if(pUiMenuEntry->mySubmenu.length() == 0)
    {
        // Instead, trigger the action:
        Application::unlockGlobal();
		this->onNativeCustomMenuItem(pMenuItem);
		return;
    }

	UIMenuElement* pTargetSubmenu = this->getUIPlane()->getElementById<UIMenuElement>(pUiMenuEntry->mySubmenu.c_str());
	pTargetSubmenu->refreshMenuItemEnabledStatus(true);
    Application::unlockGlobal();
}
/*****************************************************************************/
void MainMenuManager::onNativeCustomMenuItem(NSMenuItem* pMenuItem)
{
//    NSLog(@"onCustomMenuItem called, title = %@\n", pMenuItem.title);

    Application::lockGlobal();    
	TIntNativeCallbackInfoMap::iterator mi = myMainMenuCallbackInfos.find(pMenuItem.tag);
	if(mi == myMainMenuCallbackInfos.end())
	{
		Application::unlockGlobal();
		return;
	}

	// See if it's a direct execute action
	if(mi->second.myOptDirectAction.length() > 0)
	{
		this->getUIPlane()->performUiAction(mi->second.myOptDirectAction.c_str());
	}
	else
	{
		// Otherwise, find the element and its entry:
		SNativeMenuCallbackInfo *pInfo = &mi->second;
		UIMenuElement* pParentMenu = this->getUIPlane()->getElementById<UIMenuElement>(pInfo->myUiMenuId.c_str());
		const SMenuEntry* pUiMenuEntry = pParentMenu->getEntry(pInfo->myEntryIndex);
		pParentMenu->executEntry(pUiMenuEntry);
	}
    Application::unlockGlobal();
}
/*****************************************************************************/
void MainMenuManager::onNativeRecentFilesMenuItem(NSMenuItem* pMenuItem)
{
	string strFullPath = getCStringSafe(pMenuItem.representedObject);
    Application::lockGlobal();
	this->getUIPlane()->performUiAction("uiaOpenRecentFile", strFullPath.c_str());
    Application::unlockGlobal();
}
/*****************************************************************************/
void MainMenuManager::removeCustomMenuItemsRecursive(NSMenu* pMenu)
{
	TIntNativeCallbackInfoMap::iterator mi;
	NSMenuItem* pCurrItem;
	int iCurr, iNum = [pMenu.itemArray count];
	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
	{
		pCurrItem = [pMenu.itemArray objectAtIndex:iCurr];

		if([pCurrItem submenu])
			this->removeCustomMenuItemsRecursive([pCurrItem submenu]);

		//if(pCurrItem.tag >= INIT_MENU_TAG_COUNT)
		{
			mi = myMainMenuCallbackInfos.find(pCurrItem.tag);
			if(mi != myMainMenuCallbackInfos.end() && mi->second.myUiMenuId.length() > 0)
			{
#ifdef DEBUG_MENU_GENERATION
			gLog("MENU_GEN: Erasing tag %d\n", (int)pCurrItem.tag);
#endif
				myMainMenuCallbackInfos.erase(mi);
                [pMenu removeItemAtIndex:iCurr];
			}
			//ELSE_ASSERT;
			/*
        NOTES:
            - Right now, recent files disappear in the store builds
            - Also, oipening a file in the store build can't locate the other files in the folder, permissions?'
			*/
		}
	}
}
/*****************************************************************************/
void MainMenuManager::generateMainMenu(const char* pcsElemId, NSObject* pDelegate, IMenuContentsProvider* pRecentFilesProvider)
{
	MacMenuReconstructor rVisitor(&myMainMenuCallbackInfos, pDelegate, pRecentFilesProvider);
	UIMenuElement *pMainMenu = this->getUIPlane()->getElementById<UIMenuElement>(pcsElemId);
	pMainMenu->iterateOverMenuItems(rVisitor);
}
/*****************************************************************************/
// Thanks, Apple, for not building this in. Thanks for nothing.
NSMenuItem* MainMenuManager::findMenuItemByTagRecursive(int iTag, NSMenu* pMenu)
{
	NSMenuItem* pFoundItem = [pMenu itemWithTag:iTag];
	if(pFoundItem)
		return pFoundItem;

	// Otherwise, go through all items of this menu that have submenus,
	// and run on them:
	NSMenuItem* pCurrItem;
	int iCurr, iNum = [pMenu.itemArray count];
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pCurrItem = [pMenu.itemArray objectAtIndex:iCurr];
        
#ifdef DEBUG_MENU_GENERATION
     //   gLog("MENU_GEN: Looking at tag %d while searching for %d\n", (int)pCurrItem.tag, iTag);
#endif
        if(pCurrItem.tag == iTag)
            return pCurrItem;
        
		if([pCurrItem submenu])
		{
			pFoundItem = this->findMenuItemByTagRecursive(iTag, [pCurrItem submenu]);
			if(pFoundItem)
				return pFoundItem;
		}
	}

	return nil;
}
/*****************************************************************************/
UIPlane* MainMenuManager::getUIPlane()
{
	Window* pWindow = WindowManager::getInstance()->getMainWindow();
	return pWindow->getUIPlane();
}
/*****************************************************************************/
};
