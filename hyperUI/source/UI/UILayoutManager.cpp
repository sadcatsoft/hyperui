#include "stdafx.h"

#define UI_LAST_LAYOUT_FILE			"lastUsed"
#define UI_LAYOUTS_FOLDER			"layouts"

#define UI_LAYOUT_USER_PREFIX		"__user__"
#define UI_LAYOUT_INFO_CHILD		"__layoutInfo__"

#define UI_LAYOUT_INDEX_FILE		"index.oli"

#define UI_LAYOUT_TAG				"layout"
#define UI_LAYOUT_WINDOW_TAG		"window"
#define UI_LAYOUT_ELEM_TAG			"elem"

namespace HyperUI
{
UILayoutManager* UILayoutManager::theInstance = NULL;
/*****************************************************************************/
UILayoutManager::UILayoutManager()
{
	myDidChangeLayoutThisSession = false;

	// Read layouts that come bundled with us:
	string strCommonDataPath;
	strCommonDataPath += UI_LAYOUTS_FOLDER;
	strCommonDataPath += FOLDER_SEP;
	strCommonDataPath += UI_LAYOUT_INDEX_FILE;

	ResourceCollection rColl;
	bool bRes = rColl.loadFromFile(strCommonDataPath.c_str(), FileSourcePackageDir);
	if(bRes)
		addFromCollection(rColl, true);

	// Now, do the same for user collection
	bRes = rColl.loadFromFile(strCommonDataPath.c_str(), FileSourceUserDir);
	if(bRes)
		addFromCollection(rColl, false);
}
/*****************************************************************************/
UILayoutManager::~UILayoutManager()
{

}
/*****************************************************************************/
UILayoutManager* UILayoutManager::getInstance()
{
	if(!theInstance)
		theInstance = new UILayoutManager;
	return theInstance;
}
/*****************************************************************************/
void UILayoutManager::saveLayout(const char* pcsLayoutName, const char* pcsFileNameIn, bool bAddToLayouts)
{
	string strGoodFileName(pcsFileNameIn);
	FileUtils::ensureIsGoodFileName(strGoodFileName);

	string strCommonDataPath;
	getCommonAppDataPath(strCommonDataPath);
	strCommonDataPath += FOLDER_SEP;
	FileUtils::createFolderIn(strCommonDataPath.c_str(), UI_LAYOUTS_FOLDER);

	ResourceCollection rTempColl;

	// Add info
	if(bAddToLayouts)
	{
		string strInnerId(pcsLayoutName);
		strInnerId = UI_LAYOUT_USER_PREFIX + strInnerId;
		FileUtils::ensureIsGoodFileName(strInnerId);

		ResourceCollection rColl;
		string strCommonDataPath;
		strCommonDataPath += UI_LAYOUTS_FOLDER;
		strCommonDataPath += FOLDER_SEP;
		strCommonDataPath += UI_LAYOUT_INDEX_FILE;
		bool bRes = rColl.loadFromFile(strCommonDataPath.c_str(), FileSourceUserDir);
		if(!bRes)
			rColl.clear();

		ResourceItem* pInfo = rColl.addItem(strInnerId.c_str(), UI_LAYOUT_TAG);
		pInfo->setStringProp(PropertyName, pcsLayoutName);
		pInfo->setStringProp(PropertySource, strGoodFileName.c_str());

		string strTemp;
		rColl.saveToString(strTemp);
		FileUtils::saveToFile(strCommonDataPath.c_str(), strTemp.c_str(), false);

		SLayoutInfo rInfo;
		rInfo.myIsBuiltIn = false;
		rInfo.myInternalName = strInnerId.c_str();
		rInfo.myLabel = pcsLayoutName;
		rInfo.myFileName = strGoodFileName.c_str();
		myLayouts[rInfo.myInternalName] = rInfo;
	}

	Window* pCurrWindow;
	WindowManager::Iterator wi;
	for(wi = WindowManager::getInstance()->windowsBegin(); !wi.isEnd(); wi++)
	{
		pCurrWindow = wi.getWindow();
		saveWindowLayout(pCurrWindow, rTempColl);
	}

	string strOut;
	rTempColl.saveToString(strOut);
	
	getLayoutPath(strGoodFileName.c_str(), mySharedString);
	FileUtils::saveToFile(mySharedString.c_str(), strOut.c_str());
}
/*****************************************************************************/
void UILayoutManager::saveWindowLayout(Window* pWindow, ResourceCollection& rCollOut) const
{
	// Find the main window; if none, then take the root.
	UIPlane* pUiPlane = pWindow->getUIPlane();

	UISplitterElement* pMainSplitter = getRootUiElement(pUiPlane);
	if(!pMainSplitter)
		return;

	// Get its right side, which the contents.
	StringUtils::longNumberToString(pWindow->getId(), mySharedString);
	mySharedString = "window" + mySharedString;
	ResourceItem* pOwnItem = rCollOut.addItem(mySharedString.c_str(), UI_LAYOUT_WINDOW_TAG);

	// Now, recursively go down each element and save:
	UIElement* pFirstElemToSave = pMainSplitter->getChild<UIElement>(1);
	if(pFirstElemToSave)
		saveElementRecursive(pFirstElemToSave, 1, pOwnItem);
}
/*****************************************************************************/
void UILayoutManager::saveElementRecursive(UIElement* pElem, int iChildIndex, ResourceItem* pItem) const
{
	if(!pElem || !pElem->getSaveInLayout())
		return;

	// Save self:
	ResourceItem* pOwnItem = pItem->addChildAndSetId(UI_LAYOUT_ELEM_TAG, pElem->getStringProp(PropertyId));
	pOwnItem->setStringProp(PropertyElemType, pElem->getStringProp(PropertyElemType));
	pOwnItem->setNumProp(PropertySvChildIndex, iChildIndex);

	// If we're a splitter, save more props
	UISplitterElement* pSplitter = as<UISplitterElement>(pElem);
	if(pSplitter)
		pSplitter->saveCoreInfoInto(pOwnItem);

	// If it's a tab window, see if we have any image windows in it, and if we only have them, mark it as such:
	UITabWindowElement* pTabWindow = as<UITabWindowElement>(pElem);
	int iCurrChild, iNumChildren;
	if(pTabWindow)
	{
		bool bIsPureNonSaveableElemsTab = true;
		iNumChildren = pTabWindow->getNumChildren();
		for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		{
			if(pTabWindow->getChild<UIElement>(iCurrChild)->getSaveInLayout())
			{
				bIsPureNonSaveableElemsTab = false;
				break;
			}
		}

		pOwnItem->setNumProp(PropertySvImageTarget, bIsPureNonSaveableElemsTab);
	}

	// Now, if we're not a splitter or a tab holder, stop saving. We need to stop at generic
	// UI containers so we avoid saving actual contents of every window.
	if(!pTabWindow && !pSplitter)
		return;

	UIElement* pChild;
	iNumChildren = pElem->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(pElem->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		saveElementRecursive(pChild, iCurrChild, pOwnItem);
	}
}
/*****************************************************************************/
void UILayoutManager::loadLayout(const char* pcsOptLayoutName) const
{
	FileSourceType eSourceType = FileSourcePackageDir;
	const char* pcsLayoutFile = NULL;
	if(pcsOptLayoutName)
	{
		// We could be built-in or custom.
		TStringLayoutMap::const_iterator mi = myLayouts.find(pcsOptLayoutName);
		if(mi != myLayouts.end())
		{
			pcsLayoutFile = mi->second.myFileName.c_str();

			if(!mi->second.myIsBuiltIn)
				eSourceType = FileSourceUserDir;
		}
	}

	if(!pcsLayoutFile)
	{
		pcsLayoutFile = UI_LAST_LAYOUT_FILE;
		eSourceType = FileSourceUserDir;
	}

	getLayoutPath(pcsLayoutFile, mySharedString);

	ResourceCollection rColl;
	bool bRes = rColl.loadFromFile(mySharedString.c_str(), eSourceType, NULL);
	if(!bRes || rColl.getNumItems() == 0)
		return;

	// For now, we just grab the first child
	ResourceItem* pWindowItem = rColl.getItem(0);	
	Window* pWindow = WindowManager::getInstance()->getMainWindow();
	UIPlane* pUiPlane = pWindow->getUIPlane();

	UISplitterElement* pMainSplitter = getRootUiElement(pUiPlane);
	if(!pMainSplitter)
		return;

	// We need to traverse the elems and collect everything
	TUIElementVector vecImageWindows;
	TStringUIElementMap mapKeyElems;
	UIElement* pFirstElem = pMainSplitter->getChild<UIElement>(1);
	if(pFirstElem)
		collectKeyUiElementsRecursive(pFirstElem, mapKeyElems, vecImageWindows);
	
	// Extract all of the UI elems from their parents
	TStringUIElementMap::iterator mi;
	for(mi = mapKeyElems.begin(); mi != mapKeyElems.end(); mi++)
		mi->second->getParent<UIElement>()->deleteChild(mi->second, false);

	int iImgWindow, iNumImgWindows = vecImageWindows.size();
	for(iImgWindow = 0; iImgWindow < iNumImgWindows; iImgWindow++)
		vecImageWindows[iImgWindow]->getParent<UIElement>()->deleteChild(vecImageWindows[iImgWindow], false);

	// Delete the root elem
	pFirstElem = pMainSplitter->getChild<UIElement>(1);
	pUiPlane->deleteElement(pFirstElem);
	pFirstElem = NULL;

	// Now go and recursively restore windows
	ResourceItem* pFirstChild = NULL;
	if(pWindowItem->getNumChildren() > 0)
	{
		pFirstChild = pWindowItem->getChild(0);
		loadElementRecursive(pUiPlane, pFirstChild, pMainSplitter, &mapKeyElems);
		pMainSplitter->adjustToScreen();

		// Now, for all splitters recursively, we have to compute rectangles for any
		// elements that may have their splitters snapped.
		processAllSplitters(pMainSplitter);
	}

	// Now, go and see if there are any key elements remaining. If so, we must have
	// added something new that was not saved in a layout. We do need to stick it
	// somewhere, since there's currently no way to force it to show and also because
	// it would then leak memory.
	if(mapKeyElems.size() > 0)
	{
		UITabWindowElement* pFirstTabWindow = pMainSplitter->getChildByClass<UITabWindowElement>();
		if(!pFirstTabWindow)
		{
			UISplitterElement* pFirstSplitter = pMainSplitter->getChildByClass<UISplitterElement>();
			pFirstTabWindow = as<UITabWindowElement>(pUiPlane->addNewElement("__tabWindowHolderTemplate__", pFirstSplitter, 0));
		}
		_ASSERT(pFirstTabWindow);
		if(pFirstTabWindow)
		{
			for(mi = mapKeyElems.begin(); mi != mapKeyElems.end(); mi++)
				pFirstTabWindow->parentExistingElement(mi->second);
			pFirstTabWindow->recomputeTabInfos();
			pFirstTabWindow->adjustToScreen();
		}
	}

	// Now we need to find the first splitter with an empty pane, and stick any image 
	// windows there
	if(vecImageWindows.size() > 0)
	{
		UITabWindowElement* pFirstTabWindow = as<UITabWindowElement>(pMainSplitter->findChildByPropertyValue(PropertySvImageTarget, true));
		if(!pFirstTabWindow)
			pFirstTabWindow = getFirstEmptyTabWindow(pMainSplitter);

		if(!pFirstTabWindow)
		{
			pFirstTabWindow = pMainSplitter->getChildByClass<UITabWindowElement>();
			if(!pFirstTabWindow)
			{
				UISplitterElement* pFirstSplitter = pMainSplitter->getChildByClass<UISplitterElement>();
				pFirstTabWindow = as<UITabWindowElement>(pUiPlane->addNewElement("__tabWindowHolderTemplate__", pFirstSplitter, 0));
			}
		}
		_ASSERT(pFirstTabWindow);

		// Now, add these as tabs:
		if(pFirstTabWindow)
		{
			for(iImgWindow = 0; iImgWindow < iNumImgWindows; iImgWindow++)
				pFirstTabWindow->parentExistingElement(vecImageWindows[iImgWindow]);
			pFirstTabWindow->recomputeTabInfos();
			pFirstTabWindow->adjustToScreen();
		}
	}

	myDidChangeLayoutThisSession = true;
}
/*****************************************************************************/
UISplitterElement* UILayoutManager::getRootUiElement(UIPlane* pUiPlane) const
{
	UIElement* pMainWnd = pUiPlane->getElementById("mainWnd", false);
	if(!pMainWnd)
		return NULL;

	return pMainWnd->getChildByClass<UISplitterElement>(false);
}
/*****************************************************************************/
void UILayoutManager::loadElementRecursive(UIPlane* pUiPlane, ResourceItem* pElemItem, UIElement* pParentElem, TStringUIElementMap* pSrcElemsMap) const
{
	// See if it's a tab holder or a splitter
	UIElement* pNewElem = NULL;
	const char* pcsElemType = pElemItem->getStringProp(PropertyElemType);
	const char* pcsElemTypeName = pElemItem->getStringProp(PropertyId);
	if(IS_STRING_EQUAL(pcsElemType, g_pcsUiElemTypes[UiElemSplitter]))
	{
		// It's a splitter
		// See if it's horizontal or vertical
		const char* pcsElemType;
		if(pElemItem->getBoolProp(PropertyIsVertical))
			pcsElemType = "__vertSplitterTemplate__";
		else
			pcsElemType = "__horSplitterTemplate__";

		pNewElem = pUiPlane->addNewElement(pcsElemType, pParentElem, pElemItem->getNumProp(PropertySvChildIndex), pElemItem->getStringProp(PropertyId));
		as<UISplitterElement>(pNewElem)->loadCoreInfoFrom(pElemItem);
		//as<UISplitterElement>(pNewElem)->setPercSplitterPosition(pElemItem->getNumProp(PropertySaveSplitterPercPos));
	}
	else if(IS_STRING_EQUAL(pcsElemType, g_pcsUiElemTypes[UiElemTabWindow]))
	{
		// It's a tab window
		pNewElem = pUiPlane->addNewElement("__tabWindowHolderTemplate__", pParentElem, pElemItem->getNumProp(PropertySvChildIndex), pElemItem->getStringProp(PropertyId));
	}
	else
	{
		// It's a custom element of some sort...
		TStringUIElementMap::iterator mi = pSrcElemsMap->find(pcsElemTypeName);
		if(mi != pSrcElemsMap->end())
		{
			// Move the element
			pParentElem->parentExistingElement(mi->second, pElemItem->getNumProp(PropertySvChildIndex));
			pSrcElemsMap->erase(mi);
		}
	}

	int iCurrChild, iNumChildren = pElemItem->getNumChildren();
	for(iCurrChild = 0; pNewElem && iCurrChild < iNumChildren; iCurrChild++)
		loadElementRecursive(pUiPlane, pElemItem->getChild(iCurrChild), pNewElem, pSrcElemsMap);

// 	if(as<UISplitterElement>(pNewElem))
// 		as<UISplitterElement>(pNewElem)->onSnapStateChanged(false);

	// Now, if we're a tab, force recompute infos
	if(as<UITabWindowElement>(pNewElem))
		as<UITabWindowElement>(pNewElem)->recomputeTabInfos();
}
/*****************************************************************************/
void UILayoutManager::collectKeyUiElementsRecursive(UIElement* pElem, TStringUIElementMap& mapOut, TUIElementVector& vecImageWindowsOut) const
{
	// If we're an image, don't process us at all.
	if(!pElem || !pElem->getSaveInLayout())
	{
		vecImageWindowsOut.push_back(pElem);
		return;
	}

	UISplitterElement* pSplitter = as<UISplitterElement>(pElem);
	UITabWindowElement* pTabWindow = as<UITabWindowElement>(pElem);
	if(!pTabWindow && !pSplitter)
	{
		mapOut[pElem->getStringProp(PropertyId)] = pElem;
		return;
	}

	UIElement* pChild;
	int iCurrChild, iNumChildren = pElem->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(pElem->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		collectKeyUiElementsRecursive(pChild, mapOut, vecImageWindowsOut);
	}
}
/*****************************************************************************/
void UILayoutManager::getLayoutPath(const char* pcsLayoutName, string& strOut) const
{
	if(!pcsLayoutName)
		pcsLayoutName = UI_LAST_LAYOUT_FILE;

	strOut = UI_LAYOUTS_FOLDER;
	strOut += FOLDER_SEP;
	strOut += pcsLayoutName;
	strOut += ".txt";
}
/*****************************************************************************/
void UILayoutManager::saveLastUsedLayout()
{
	saveLayout("Last Used", UI_LAST_LAYOUT_FILE, false);
}
/*****************************************************************************/
void UILayoutManager::addFromCollection(ResourceCollection& rCollIn, bool bIsBuiltIn)
{
	ResourceItem* pItem;
	int iChild, iNumChildren = rCollIn.getNumItems();
	for(iChild = 0; iChild < iNumChildren; iChild++)
	{
		SLayoutInfo rInfo;

		pItem = rCollIn.getItem(iChild);

		rInfo.myIsBuiltIn = bIsBuiltIn;
		pItem->getAsString(PropertyId, rInfo.myInternalName);
		pItem->getAsString(PropertyName, rInfo.myLabel);
		pItem->getAsString(PropertySource, rInfo.myFileName);
		
		myLayouts[rInfo.myInternalName] = rInfo;
	}
}
/*****************************************************************************/
UITabWindowElement* UILayoutManager::getFirstEmptyTabWindow(UIElement* pParent) const
{
	UIElement* pChild;
	int iCurrChild, iNumChildren = pParent->getNumChildren();
	UITabWindowElement *pCElem;
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = as<UIElement>(pParent->getChild(iCurrChild));
		pCElem = as<UITabWindowElement>(pChild);
		if(pCElem && pCElem->getNumChildren() == 0)
			return pCElem;
		else if(pChild)
		{
			UITabWindowElement* pRes = getFirstEmptyTabWindow(pChild);
			if(pRes)
				return pRes;
		}
	}
	return NULL;
}
/*****************************************************************************/
void UILayoutManager::processAllSplitters(UIElement* pParent) const
{
	// Call on children first
	UIElement* pChild;
	int iCurrChild, iNumChildren = pParent->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(pParent->getChild(iCurrChild));
		processAllSplitters(pChild);
	}

	UISplitterElement* pAsSplitter = as<UISplitterElement>(pParent);
	if(pAsSplitter && pAsSplitter->getCurrentSnapState() != SplitterSnapNone)
	{
		pAsSplitter->setIsAdjustingFromSavedNormalPosition(true);
		pAsSplitter->adjustToScreen();
		pAsSplitter->setIsAdjustingFromSavedNormalPosition(false);
		pAsSplitter->onSnapStateChanged();
	}
}
/*****************************************************************************/
bool UILayoutManager::getHaveValidLastUsedLayout() const
{
	getLayoutPath(UI_LAST_LAYOUT_FILE, mySharedString);
	return FileUtils::doesFileExist(mySharedString.c_str(), FileSourceUserDir);
}
/*****************************************************************************/
};