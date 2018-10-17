#include "stdafx.h"

namespace HyperUI
{
Application* Application::theInstance = NULL;

#if defined(_DEBUG) || defined(DEBUG)
bool Application::myIsDebuggingEnabled = true;
#else
bool Application::myIsDebuggingEnabled = false;
#endif

/*****************************************************************************/
Application* Application::getInstance()
{
	if(!theInstance)
		theInstance = new Application;
	return theInstance;
}
/*****************************************************************************/
Application::Application()
{
	resetAllClocks();

	myAllocator = new UIAllocator();
	registerTypeConstructors();

	myWindowFactory = new IWindowFactory();
	myRecentFilesProviderFactory = NULL;
	myDisableSound = false;
	myIsQuittingFinal = false;
#ifdef RUN_AS_FAST_AS_POSSIBLE
	myShowFramerate = true;
#else
	myShowFramerate = false;
#endif
}
/*****************************************************************************/
Application::~Application()
{
	setWindowFactory(NULL);
	setRecentFilesProviderFactory(NULL);

	delete myAllocator;
	myAllocator = NULL;

#ifdef USE_FREETYPE_FONTS
	FontManager::destroyFreelib();
#endif
}
/*****************************************************************************/
void Application::setWindowFactory(IWindowFactory* pFactory)
{
	if(myWindowFactory)
		delete myWindowFactory;
	myWindowFactory = pFactory;
}
/*****************************************************************************/
void Application::setRecentFilesProviderFactory(IMenuContentsProviderFactory* pFactory)
{
	if(myRecentFilesProviderFactory)
		delete myRecentFilesProviderFactory;
	myRecentFilesProviderFactory = pFactory;
}
/*****************************************************************************/
void Application::resetAllClocks()
{
	for(int iIndex = 0; iIndex < ClockLastPlaceholder; iIndex++)
		myGlobalTimes[iIndex] = GLOBAL_ANIM_START_TIME;
}
/*****************************************************************************/
void Application::loadClocksFrom(ResourceItem* pItem, PropertyType eCurrPropName)
{
	STRING_TYPE strTemp;
	int iCurrClock, iNumClocks = pItem->getEnumPropCount(eCurrPropName);
	if(iNumClocks > ClockLastPlaceholder)
		iNumClocks = ClockLastPlaceholder;
	for(iCurrClock = 0; iCurrClock < iNumClocks; iCurrClock++)
	{
		strTemp = pItem->getEnumPropValue(eCurrPropName, iCurrClock);
		_ASSERT(strTemp[0] == SAVE_LONG_AS_STRING_SYMBOL);
		// The first letter is a special symbol, skip it
		strTemp = strTemp.substr(1, strTemp.length() - 1);
		myGlobalTimes[iCurrClock] = atol(strTemp.c_str());
	}
}
/*****************************************************************************/
void Application::onTimerTickBegin(bool bIncrementMainGameClock)
{
	myInternalsLock.lock();

	// We always tick the UI, since now we have in-game UI, too.
	myGlobalTimes[ClockUiPrimary]++;
	// We tick the game only if there's no pausing UI pausing it.
	if(bIncrementMainGameClock)
		myGlobalTimes[ClockMainGame]++;

	// The universal clock always keeps going, even if we're in a "pause".
	myGlobalTimes[ClockUniversal]++;
	myInternalsLock.unlock();
}
/*****************************************************************************/
bool Application::showOpenFileDialog(const char* pcsInitFilePath, bool bAllowMultiple, TStringVector& vecFilesOut, FormatsManager* pFormatsManager, bool bCanOpenFolders)
{
	TStringVector vecExtensions;
	TStringVector vecDescs;
	if(pFormatsManager)
	{
		pFormatsManager->getExtensionsList(false, FormatRead, vecExtensions, false);
		pFormatsManager->getDescriptionsList(false, FormatRead, vecDescs, false);
	}
	else
	{
		vecExtensions.push_back("*.*");
		vecDescs.push_back("All Files");
	}
	return Application::showOpenFileDialog(pcsInitFilePath, bAllowMultiple, vecFilesOut, vecExtensions, vecDescs, pFormatsManager, bCanOpenFolders);
}
/*****************************************************************************/
ResultCodeType Application::showSaveFileDialog(string& strPathInOut, string& strErrorOut, FormatsManager* pFormatsManager, bool bForceNativeExtension, bool bDontListNative)
{
	TStringVector vecExtensions;
	TStringVector vecDescs;
	if(pFormatsManager)
	{
		pFormatsManager->getExtensionsList(false, FormatWrite, vecExtensions, bDontListNative);
		pFormatsManager->getDescriptionsList(false, FormatWrite, vecDescs, bDontListNative);
	}
	else
	{
		vecExtensions.push_back("*.*");
		vecDescs.push_back("All Files");
	}
	return Application::showSaveFileDialog(strPathInOut, strErrorOut, vecExtensions, vecDescs, pFormatsManager, bForceNativeExtension, bDontListNative);
}
/*****************************************************************************/
void Application::onSwitchedTo(Window* pExceptWindow)
{
	EventManager::getInstance()->sendEvent(EventClipboardChanged, NULL);

	KeyManager::getInstance()->onModifierKeyUp(true, true, true, true);

	// Set this window as active
	WindowManager::getInstance()->setLastActiveWindow(pExceptWindow);

	if(pExceptWindow)
		pExceptWindow->onSwitchedTo();
}
/*****************************************************************************/
Window* Application::allocateWindow(ResourceType eSourceCollection, int iScreenW, int iScreenH, bool bIsMainWindow)
{
	if(!myWindowFactory)
		return NULL;

	return myWindowFactory->allocateWindow(eSourceCollection, iScreenW, iScreenH, bIsMainWindow);
}
/*****************************************************************************/
IMenuContentsProvider* Application::allocateRecentFilesProvider()
{
	if(!myRecentFilesProviderFactory)
		return NULL;

	return myRecentFilesProviderFactory->allocateProvider();
}
/*****************************************************************************/
void Application::setUIAllocator(UIAllocator* pAllocator)
{
	delete myAllocator;
	myAllocator = pAllocator;

	// We must re-register default types.
	registerTypeConstructors();
}
/*****************************************************************************/
void Application::reloadAll()
{
	ResourceManager::getInstance()->resetAllContent();

	// Reload all windows
	WindowManager::Iterator wi;
	for(wi = WindowManager::getInstance()->windowsBegin(); !wi.isEnd(); wi++)
		wi.getWindow()->reloadAll();
}
/*****************************************************************************/
void Application::registerTypeConstructors()
{
	UIAllocator* pAllocator = this->getUIAllocator();

	// Standard UI element types
	REGISTER_STANDARD_UI_ELEMENT(UIElement, UiElemStatic);
	REGISTER_STANDARD_UI_ELEMENT(UIRoundSliderElement, UiElemRoundSlider);
	REGISTER_STANDARD_UI_ELEMENT(UIRoundProgressElement, UiElemRoundProgress);
	REGISTER_STANDARD_UI_ELEMENT(UIButtonElement, UiElemButton);
	REGISTER_STANDARD_UI_ELEMENT(UITableElement, UiElemTable);
	REGISTER_STANDARD_UI_ELEMENT(UIHorizontalTableElement, UiElemHorizontalTable);
	REGISTER_STANDARD_UI_ELEMENT(UIProgressElement, UiElemProgress);
	REGISTER_STANDARD_UI_ELEMENT(UISliderElement, UiElemSlider);
	REGISTER_STANDARD_UI_ELEMENT(UITextFieldElement, UiElemTextField);
	REGISTER_STANDARD_UI_ELEMENT(UIExpressionTextFieldElement, UiElemExpressionTextField);
	REGISTER_STANDARD_UI_ELEMENT(UIExpressionEditTextFieldElement, UiElemExpressionEditTextField);
	REGISTER_STANDARD_UI_ELEMENT(UIMultilineTextFieldElement, UiElemMultilineTextField);
	REGISTER_STANDARD_UI_ELEMENT(UISolidColorElement, UiElemSolidColor);
	REGISTER_STANDARD_UI_ELEMENT(UICheckboxElement, UiElemCheckbox);
	REGISTER_STANDARD_UI_ELEMENT(UITableCellElement, UiElemTableCell);
	REGISTER_STANDARD_UI_ELEMENT(UIMenuElement, UiElemMenu);
	REGISTER_STANDARD_UI_ELEMENT(UISplitterElement, UiElemSplitter);
	REGISTER_STANDARD_UI_ELEMENT(UITabWindowElement, UiElemTabWindow);
	REGISTER_STANDARD_UI_ELEMENT(UIGrid, UiElemGrid);
	REGISTER_STANDARD_UI_ELEMENT(UIFixedGrid, UiElemFixedGrid);
	REGISTER_STANDARD_UI_ELEMENT(UIColorSwatch, UiElemColorSwatch);
	REGISTER_STANDARD_UI_ELEMENT(UIImageDropdown, UiElemImageDropdown);
	REGISTER_STANDARD_UI_ELEMENT(UIDropdown, UiElemDropdown);
	REGISTER_STANDARD_UI_ELEMENT(UIDropdownTextPart, UiElemDropdownTextPart);
	REGISTER_STANDARD_UI_ELEMENT(UIDropdownMenu, UiElemDropdownMenu);
	REGISTER_STANDARD_UI_ELEMENT(UIUnitedNumeric, UiElemUnitedNumeric);
	REGISTER_STANDARD_UI_ELEMENT(UIGradientEdit, UiElemGradientEdit);
	REGISTER_STANDARD_UI_ELEMENT(UIPopupElement, UiElemPopup);
	REGISTER_STANDARD_UI_ELEMENT(UIThumbnailGrid, UiElemThumbnailGrid);
	REGISTER_STANDARD_UI_ELEMENT(UIHistogram, UiElemHistogram);
	REGISTER_STANDARD_UI_ELEMENT(UICurveEditor, UiElemCurveEditor);
	REGISTER_STANDARD_UI_ELEMENT(UISlidingStopsEdit, UiElemSlidingStopsEdit);
	REGISTER_STANDARD_UI_ELEMENT(UIColorList, UiElemColorList);
	REGISTER_STANDARD_UI_ELEMENT(UIRichTextElement, UiElemRichText);
	REGISTER_STANDARD_UI_ELEMENT(UIColorPalette, UiElemColorPalette);
	REGISTER_STANDARD_UI_ELEMENT(UIRecentColorsPalette, UiElemRecentColorsPalette);
	REGISTER_STANDARD_UI_ELEMENT(UIColorPicker, UiElemColorPicker);
}
/*****************************************************************************/

};
