#include "stdafx.h"

namespace HyperUI
{

#ifdef DIRECTX_PIPELINE
#include "D3DRenderer.h"
extern D3DRenderer* g_pDxRenderer;
#endif

UNIQUEID_TYPE ICursorManager::theCursorId = 0;
/*****************************************************************************/
Window::Window(int iScreenW, int iScreenH, bool bIsMainWindow)
{
	myIsMainWindow = bIsMainWindow;

	WindowManager::getInstance()->onWindowCreated(this, bIsMainWindow);

	myCursorManager = this->allocateCursorManager();

	myLastTabletButtonUpTime = 0;
	myIsTabletButtonDown = false;

	myLastMousePos.set(-1, -1);
	mySize.set(iScreenW, iScreenH);
	myLastKey = 0;
	myFocusElement = NULL;
	myCurrButtonDown = MouseButtonNone;
	//myTouchBeganEmpty = false;
	myDropAnim.setNonAnimValue(0.0);
	myLastDepthClearTime = 0;
	myCurrentBlendMode = BlendModeLastPlaceholder;
	myDidFinishPrelimInit = false;
	myCurrNumLights = 0;
	myTotalLights = 0;

	myTooltipManager = new TooltipManager(this);
	myDragDropManager = new DragDropManager(this);
	myUIPlane = NULL;
	myShouldClose = false;
	myBeganClosingFlag = ClosingNone;
	myHasCalledCloseAlready = false;

#ifdef USE_FREETYPE_FONTS
	myFontManager = new FontManager;
	initFonts();
#endif

	myTextureManager = new TextureManager(this);
	myTextObjectCacher = new TCachedTextObjectCacher(64);
	myDrawingCache = new DrawingCache(this);
}
/*****************************************************************************/
Window::~Window()
{
	//gLog("~Window() begin context = %x\n", wglGetCurrentContext());

 	delete myUIPlane;
 	myUIPlane = NULL;

#ifdef USE_FREETYPE_FONTS
	delete myFontManager;
	myFontManager = NULL;
#endif

	WindowManager::getInstance()->onWindowDestroyed(this, false);

	delete myTextureManager;
	myTextureManager = NULL;

	delete myTooltipManager;
	myTooltipManager = NULL;

	delete myDragDropManager;
	myDragDropManager = NULL;

	delete myDrawingCache;
	myDrawingCache = NULL;

	delete myTextObjectCacher;
	myTextObjectCacher = NULL;

	clearAndDeleteContainer(myOverrideUndoManagers);

	deallocateCursorManager(myCursorManager);

	myShouldClose = false;
//gLog("~Window() end\n");
}
/*****************************************************************************/
void Window::initFonts()
{
#ifdef USE_FREETYPE_FONTS
	myFontManager->initFromCollection(ResourceManager::getInstance()->getCollection(ResourceFonts));
#endif
}
/*****************************************************************************/
void Window::reloadAll()
{
	this->getUIPlane()->reloadAll();

	SVector2D svScreenDims;
	this->getSize(svScreenDims);
	this->onWindowSizeChanged(svScreenDims.x, svScreenDims.y);

#ifdef USE_FREETYPE_FONTS
	myFontManager->clear();
#endif
	initFonts();
	myDrawingCache->getTextCacher()->cleanAll();
	myDrawingCache->reloadAllTextures();
}
/*****************************************************************************/
void Window::nicefyScroll()
{
	myScroll.x = (int)myScroll.x;
	myScroll.y = (int)myScroll.y;
}
/*****************************************************************************/
void Window::resetScroll(const SVector2D& svPlayerPos)
{
	myScroll.x = svPlayerPos.x - mySize.x/2.0;
	myScroll.y = svPlayerPos.y - mySize.y/2.0;
	nicefyScroll();
}
/*****************************************************************************/
void Window::finishPrelimInit(ResourceType eSourceCollection, const char* pcsInitLayerToShow)
{
#if defined(WIN32) || defined(LINUX)
	TCollectionDefMap* pAnimCollections = ResourceManager::getInstance()->getCollectionsForRole(CollectionRoleGraphics);
	TCollectionDefMap::iterator mi;
	for(mi = pAnimCollections->begin(); mi != pAnimCollections->end(); mi++)
		myTextureManager->initFromCollection(ResourceManager::getInstance()->getCollection(mi->second.myType), mi->second.myTargetDataFolderPath.c_str(), NULL);
#endif
#ifdef _DEBUG
	START_TIMING();
#endif
	myUIPlane = this->allocateUIPlane(eSourceCollection);
#ifdef _DEBUG
	END_TIMING("Window::UIPlane initialization took");
#endif
	myDidFinishPrelimInit = true;

	if(IS_VALID_STRING_AND_NOT_NONE(pcsInitLayerToShow))
		myUIPlane->showUI(pcsInitLayerToShow);

	this->onInitialized();
}
/*****************************************************************************/
void Window::setFocusElement(UIElement* pActive, bool bIsCancelling)
{
	if(myFocusElement == pActive)
		return;

	_ASSERT(!pActive || pActive->getAllowKeepingFocus());

	if(myFocusElement && !myFocusElement->getAllowRemovingFocus())
		return;

	if(myFocusElement && myFocusElement != pActive)
		myFocusElement->onLostFocus(bIsCancelling);

	myFocusElement = pActive;
	if(myFocusElement && myFocusElement->getAllowKeyboardEntry())
		KeyManager::getInstance()->beginKeyboardFocus(myFocusElement);
	else
		KeyManager::getInstance()->endKeyboardFocus();

	if(myFocusElement)
		myFocusElement->onGainedFocus();
}
/*****************************************************************************/
void Window::onUiElementDeleted(UIElement* pElem)
{
	if(myUIPlane)
		myUIPlane->onUiElementDeleted(pElem);

	// It's being deleted, so we don't care about the onLostFocus() part...
	if(myFocusElement == pElem)
		myFocusElement = NULL;
}
/*****************************************************************************/
bool Window::onKeyUp(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl)
{
	myLastKey = 0;
	//gLog("OnKeyUp: Reset the last key to %d\n", myLastKey);

	KeyMeaningType eKeyMeaning = KeyManager::getInstance()->mapKey(iKey, bControl, bAlt, bShift);

	// Special case: if we have focus and the key is Tab, override any actions we may have had.
	if(iKey == SilentKeyTab && myFocusElement)
		eKeyMeaning = KeyMeaningTab;

	// Set the status of the modifier keys - but only if we're pressing them separately.
	// Bad idea. If we release a keys for a shortcut simultaneously, for instance,
	// Ctrl+N, we will not register the fact that Ctrl has been released, leading
	// to all kinds of confusion down the road.
	//if(iKey == 0)
	KeyManager::getInstance()->onModifierKeyUp(bControl, bAlt, bShift, bMacActualControl);

	KeyManager::getInstance()->setIsPressed(eKeyMeaning, false);
	if((char)iKey == ' ')
		KeyManager::getInstance()->setIsSpacePressed(false);

	if(this->onKeyUpSubclass(iKey, bControl, bAlt, bShift, bMacActualControl))
		return true;

	const char* pcsActionValue = PROPERTY_NONE;
	const char* pcsAction = PROPERTY_NONE;
	if(eKeyMeaning == KeyMeaningUiAction)
	{
		pcsAction = KeyManager::getInstance()->getUiActionFor(iKey, bControl, bAlt, bShift);
		pcsActionValue = KeyManager::getInstance()->getUiActionValueFor(iKey, bControl, bAlt, bShift);
	}

	performOneTimeKeyUpAction(eKeyMeaning, pcsAction, pcsActionValue, bAlt);

	ActivityManager::getInstance()->singleValueChanged();
	return true;
}
/*****************************************************************************/
bool Window::onKeyDown(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl)
{
	ActivityManager::getInstance()->singleValueChanged();

	// Disable repeating the key.
	KeyMeaningType eKeyMeaning = KeyManager::getInstance()->mapKey(iKey, bControl, bAlt, bShift);
	int iCombinedKey = KeyManager::getInstance()->combineFlagsWithKey(iKey, bControl, bAlt, bShift);
	//bool bIsKeyARepeat = (myLastKey == iKey && iKey != 0);
	bool bIsKeyARepeat = (myLastKey == iCombinedKey && iCombinedKey != 0);
	//gLog("onKeyDown: bIsRepeat=%d this=%d last=%d\n", bIsKeyARepeat, iCombinedKey, myLastKey);
	if(!bIsKeyARepeat)
	{
		myLastKey = iCombinedKey;
		//gLog("Set last key to %d\n", myLastKey);

		// Set the status of the modifier keys - but only if we're pressing them separately.
		if(iKey == 0)
			KeyManager::getInstance()->onModifierKeyDown(bControl, bAlt, bShift, bMacActualControl);

		KeyManager::getInstance()->setIsPressed(eKeyMeaning, true);
		if((char)iKey == ' ')
			KeyManager::getInstance()->setIsSpacePressed(true);
	}

	// If there's an active tool, tell it about the key
	// and see if it handles this. Right now used for shift/etc.
	if(this->onKeyDownBeforeUISubclass(iKey, bControl, bAlt, bShift, bMacActualControl, bIsKeyARepeat))
		return true;


// 		if(eKeyMeaning == KeyMeaningNone)
// 			return false;

/*
	const char* pcsActionValue = PROPERTY_NONE;
	const char* pcsAction = PROPERTY_NONE;
	if(eKeyMeaning == KeyMeaningUiAction)
	{
		pcsAction = KeyManager::getInstance()->getUiActionFor(iKey, bControl, bAlt, bShift);
		pcsActionValue = KeyManager::getInstance()->getUiActionValueFor(iKey, bControl, bAlt, bShift);
	}
*/


	// See if we have a focus element to pass the keys to
	// We still want shortcuts to be available even while a UI element is selected.
	// However, notice that we only want the ones where a modifier key exists, otherwise
	// you'd be trigerring all kinds of things while simply typing. Just like, say, a certain
	// other 3D app.
	bool bProcessKeyOnlyIfUIActionWithModifierKeys = false;
	if(myFocusElement && iKey != 0)
	{
		// Handle this and return
		if(myFocusElement->getAllowKeyboardEntry() && !bAlt && !bControl)
		{
			bool bDidAcceptText = myFocusElement->onTextInput(iKey);
			if(!bDidAcceptText)
				myFocusElement->onKeyUp(iKey, bControl, bAlt, bShift, bMacActualControl);
			else
				return true;
		}
		// This is to prevent ctrl and alt from triggering shortcuts
		// while we're typing into an actual keyboard focus control.
		else if(myFocusElement->getAllowKeyboardEntry())
		{
			// Well, no - we still want to call the onKeyUp(), since
			// it's not the part that does the typing. To avoid
			// actually trigerring other shortcuts, we
			// return after calling it, though.
			myFocusElement->onKeyUp(iKey, bControl, bAlt, bShift, bMacActualControl);
			return true;
		}
		else if(myFocusElement && myFocusElement->getIsEnabled() && myFocusElement->getAllowKeepingFocus())
		{
			// We have an elem that doesn't allow keyboard entry, but allows focus,
			// so it probably wants some keys anyway.
			myFocusElement->onKeyUp(iKey, bControl, bAlt, bShift, bMacActualControl);
		}

		bProcessKeyOnlyIfUIActionWithModifierKeys = true;
		//return true;
	}

	// Somehow accept enter to close elements with default action elems...
	if(iKey == SilentKeyNumpadEnter || iKey == SilentKeyCarriageReturn)
	{
		if(myFocusElement)
		{
			// We have a focus element. Kill it first.
			// Now, we don't kill the focus if we've pressed the carriage
			// return key *and* this is a multiline control.
			bool bKillFocus = true;
			if(iKey == SilentKeyCarriageReturn && myFocusElement && as<UIMultilineTextFieldElement>(myFocusElement))
				bKillFocus = false;

			if(bKillFocus)
				setFocusElement(NULL);
		}
		else
		{
			// Attempt to find an element with a default action child set.
			UIElement* pActionElem = myUIPlane->getElementWithDefaultActionChild();
			if(pActionElem)
			{
				UIButtonElement* pChild = pActionElem->getDefaultActionChild();
				if(pChild)
				{
					pChild->simulateClick();
					return true;
				}
			}
		}
	}
	else if(iKey == SilentKeyEscape)
	{
		// Escape is kinda special, since may mean different things depending
		// on the context.

		// Attempt to find an element with a default action child set.
		UIElement* pActionElem = myUIPlane->getElementWithDefaultCancelChild();
		bool bProcessed = false;
		if(pActionElem)
		{
			UIButtonElement* pChild = pActionElem->getDefaultCancelChild();
			if(pChild)
			{
				pChild->simulateClick();
				return true;
			}
		}

		// Otherwise, see if any menus are shown
		if(myUIPlane->getAreAnyShownWithTag("submenu"))
		{
			myUIPlane->hideAllWithTag("submenu", NULL, true, myUIPlane, AnimOverActionGenericCallback);
			return true;
		}

		// Otherwise, if we have a focus element, kill the focus
		if(getFocusElement())
			setFocusElement(NULL, true);
	}


	if(!myFocusElement && this->onKeyDownAfterUISubclass(iKey, bControl, bAlt, bShift, bMacActualControl, bIsKeyARepeat))
		return true;

	// Ask any element we're over if they want to handle the key
	if(!myFocusElement)
	{
		UIElement* pElement = myUIPlane->getTopmostShownElementAt(myLastMousePos.x, myLastMousePos.y, false);
		pElement = myUIPlane->getFinalChildAtCoords(myLastMousePos.x, myLastMousePos.y, pElement, false, false);
		for(; pElement != NULL; pElement = pElement->getParent<UIElement>())
		{
			if(pElement->onKeyUp(iKey, bControl, bAlt, bShift, bMacActualControl))
				return true;
		}
	}

	// We need the alt alone propagated forward
	if(eKeyMeaning == KeyMeaningNone && !bAlt)
		return bProcessKeyOnlyIfUIActionWithModifierKeys ? true : false;

	const char* pcsActionValue = PROPERTY_NONE;
	const char* pcsAction = PROPERTY_NONE;
	bool bProcess = bProcessKeyOnlyIfUIActionWithModifierKeys ? false : true;
	if(eKeyMeaning == KeyMeaningUiAction)
	{
		pcsAction = KeyManager::getInstance()->getUiActionFor(iKey, bControl, bAlt, bShift);
		pcsActionValue = KeyManager::getInstance()->getUiActionValueFor(iKey, bControl, bAlt, bShift);
		if(bProcessKeyOnlyIfUIActionWithModifierKeys && !IS_STRING_EQUAL(pcsAction, PROPERTY_NONE)
			&& (bControl || bAlt)  )
			bProcess = true;
	}
	else if(eKeyMeaning == KeyMeaningTab)
		bProcess = true;

	// Anything that goes here must account for the key being a repeat
	if(!bIsKeyARepeat && bProcess)
		performOneTimeKeyDownAction(eKeyMeaning, pcsAction, pcsActionValue, bIsKeyARepeat, bAlt);

	return true;
}
/*****************************************************************************/
void Window::performOneTimeKeyDownAction(KeyMeaningType eKeyMeaning, const char* pcsUiAction, const char* pcsUiActionValue, bool bIsARepeat, bool bIsAlt)
{
	if(eKeyMeaning == KeyMeaningUiAction && !bIsARepeat)
	{
		if(this->getIsActionEnabled(pcsUiAction, pcsUiActionValue))
			myUIPlane->performUiAction(pcsUiAction, pcsUiActionValue);
	}
}
/*****************************************************************************/
void Window::performOneTimeKeyUpAction(KeyMeaningType eKeyMeaning, const char* pcsUiAction, const char* pcsUiActionValue, bool bIsAlt)
{
	if(eKeyMeaning == KeyMeaningTab)
	{
		// See if we have the current element
		UIElement* pFocusElem = this->getFocusElement();
		if(pFocusElem && pFocusElem->getAllowKeyboardEntry())
		{
			int iCurrTabIndex = -1;
			if(pFocusElem->doesPropertyExist(PropertyTabIndex))
				iCurrTabIndex = pFocusElem->getNumProp(PropertyTabIndex);
			else if(pFocusElem->getParent())
			{
				// Sometimes, it's a text field where the parent has the property
				// if it's a compound control.
				if(pFocusElem->getParent()->doesPropertyExist(PropertyTabIndex))
					iCurrTabIndex = pFocusElem->getParent()->doesPropertyExist(PropertyTabIndex);
			}

			if(iCurrTabIndex >= 0)
			{
				// Get its topmost parent
				UIElement* pTopmost = pFocusElem->getTopmostParent<UIElement>();
				_ASSERT(pTopmost);
				UIElement* pNextTabElem = pTopmost->findNextChildByPropertyValue(PropertyTabIndex, iCurrTabIndex);
				UIElement* pFinalChild = pNextTabElem;
				if(pNextTabElem && !pNextTabElem->getAllowKeyboardEntry()) // !dynamic_cast<UITextFieldElement*>(pNextTabElem))
				{
					// We may have a parent with a tab value, but need the actual child of
					// textinput.
					pFinalChild = pNextTabElem->getChildByElemType<UIElement>(g_pcsUiElemTypes[UiElemTextField]);
				}
				setFocusElement(pFinalChild);
				// Select us:
				UITextFieldElement* pAsTextField = as<UITextFieldElement>(pFinalChild);
				if(pAsTextField)
					pAsTextField->selectAll();
			}
			else
				setFocusElement(NULL);
		}
	}
}
/*****************************************************************************/
bool Window::onTimerTickBegin()
{
	if(Application::getInstance()->getIsQuittingFinal())
		return false;

	myCurrNumLights = 0;
	myTotalLights = 0;

	if(this->getIsMainWindow())
	{
		FLOAT_TYPE fPausingUiOpacity = 0;
		if(myUIPlane)
			fPausingUiOpacity = myUIPlane->getMaxPausingUiOpacity();
		Application::getInstance()->onTimerTickBegin(fPausingUiOpacity <= 0.0);
		EventManager::getInstance()->onTimerTick();
	}

	if(!myUIPlane)
		return false;

	myUIPlane->onTimerTickBegin(Application::getInstance()->getGlobalTime(ClockUiPrimary));
	myTooltipManager->onTimerTick(Application::getInstance()->getGlobalTime(ClockUiPrimary));

	return true;
}
/*****************************************************************************/
bool Window::onTimerTickEnd()
{
	if(Application::getInstance()->getIsQuittingFinal())
		return false;

	if(myUIPlane)
        myUIPlane->onTimerTickEnd(Application::getInstance()->getGlobalTime(ClockUiPrimary));

	myDrawingCache->onTimerTick();

	if(myUIPlane)
		myUIPlane->deleteQueuedElements();

#ifdef DEBUG_SHOW_FRAMERATE
	// Push the latest one on
	///	unsigned long lTime = (unsigned long)clock();
	double dTime = DateUtils::getCurrentTime();
	mySimFrameTimes.push_back(dTime);
	int iNumToErase = mySimFrameTimes.size() - (int)GAME_FRAMERATE*4;
	if(iNumToErase > 0)
		mySimFrameTimes.eraseFront(iNumToErase);

#endif

	return false;
}
/*****************************************************************************/
void Window::onTouchDown(TTouchVector& vecTouches, MouseButtonType eButton)
{
	myLastMousePos.set(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y);

	myCurrButtonDown = eButton;
	myTooltipManager->onMouseDown();

	// Add the new touches to the map
	int iCurrTouch, iNumTouches = vecTouches.size();
	for(iCurrTouch = 0; iCurrTouch < iNumTouches; iCurrTouch++)
		myActiveTouches[vecTouches[iCurrTouch].myTouchId] = vecTouches[iCurrTouch];

	myUIPlane->onTouchDown(vecTouches);

	ActivityManager::getInstance()->singleValueChanged();
}
/*****************************************************************************/
void Window::onCursorGlide(TTouchVector& vecTouches)
{
	bool bDidMove = true;
	if(vecTouches[0].myPoint.x == myLastMousePos.x && vecTouches[0].myPoint.y == myLastMousePos.y)
		bDidMove = false;

	myLastMousePos.set(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y);
	myUIPlane->onCursorGlide(vecTouches);

	if(bDidMove)
		ActivityManager::getInstance()->singleValueChanged();
}
/*****************************************************************************/
void Window::onMouseWheel(SVector2D& svMousePos, FLOAT_TYPE fDelta)
{
	myUIPlane->onMouseWheel(svMousePos, fDelta);
	ActivityManager::getInstance()->singleValueChanged();
}
/*****************************************************************************/
void Window::onTouchLeave()
{
	ActivityManager::getInstance()->singleValueChanged();
}
/*****************************************************************************/
bool Window::onTouchMove(TTouchVector& vecTouches)
{
	// Note that on Windows, we generate the events even if we're holding
	// the mouse in one location! Which we don't want, since it'll add
	// extra points with exact coords...
	if(myLastMousePos.x == vecTouches[0].myPoint.x && myLastMousePos.y == vecTouches[0].myPoint.y)
		return false;

	myLastMousePos.set(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y);

	this->modifyTouches(vecTouches);

	int iCurrTouch, iNumTouches = vecTouches.size();
	for(iCurrTouch = 0; iCurrTouch < iNumTouches; iCurrTouch++)
		myActiveTouches[vecTouches[iCurrTouch].myTouchId] = vecTouches[iCurrTouch];

	if(this->getPassTouchesToUI())
	{
		myUIPlane->onTouchMove(vecTouches);
		myDragDropManager->onTouchMove(vecTouches);
	}

	ActivityManager::getInstance()->singleValueChanged();

	return true;	
}
/*****************************************************************************/
bool Window::onTouchUp(TTouchVector& vecTouches)
{
	// Note: It's important this is first, for we use the num
	// touches down to prevent ui actions being processed.
	int iCurrTouch, iNumTouches = vecTouches.size();
	TTouchMap::iterator tmi;
	for(iCurrTouch = 0; iCurrTouch < iNumTouches; iCurrTouch++)
	{
		tmi = myActiveTouches.find(vecTouches[iCurrTouch].myTouchId);
		if(tmi != myActiveTouches.end())
			myActiveTouches.erase(tmi);
	}

	_ASSERT(myActiveTouches.size() == 0);

	myTooltipManager->onMouseUp();


	/*
	if(myCurrMessageText.length() > 0 && myCurrMessageAutoHide == false)
	{
		FLOAT_TYPE fCurrProg =  myCurrMessageAnim.getValue(Application::getInstance()->getGlobalTime(TEXT_MESSAGE_CLOCK_TYPE));
		// Only fade out once, not multiple times if we click multiple times.
		if(fCurrProg >= 1.0 - FLOAT_EPSILON)
		{
			myCurrMessageAnim.setAnimation(1, 0, Application::getInstance()->getGlobalTime(TEXT_MESSAGE_CLOCK_TYPE), Application::getInstance()->getGlobalTime(TEXT_MESSAGE_CLOCK_TYPE) + Application::secondsToTicks(myMessageFadeOutDuration), AnimOverActionFadeOutComplete | myMessageOverAnimAction, this);
		}
	}
	*/
	///else

		bool bIgnoreFurtherActions = false;

		if(this->getPassTouchesToUI())
		{
			bIgnoreFurtherActions |= myDragDropManager->onTouchUp(vecTouches);
			bIgnoreFurtherActions |= myUIPlane->onTouchUp(vecTouches, bIgnoreFurtherActions);
		}

		myCurrButtonDown = MouseButtonLeft;
		ActivityManager::getInstance()->singleValueChanged();

		if(Application::getIsDebuggingEnabled())
		{
			if(vecTouches.size() > 0 && myReloadButtonRect.doesContain(vecTouches[0].myPoint))
				Application::reloadAll();
		}

		return bIgnoreFurtherActions;
}
/*****************************************************************************/
bool Window::getIsShowingAnyUI()
{
	if(myUIPlane && myUIPlane->getIsAnimatingAnything())
		return true;
	else
		return false;
}
/*****************************************************************************/
void Window::render()
{
	if(Application::getInstance()->getIsQuittingFinal())
		return;

#if defined(_DEBUG) || defined(_DEBUG)

#ifdef USE_OPENGL2
// 	myDrawingCache->addRectangle(0, 0, 1000, 1000, SColor(1,1,1,1));
// 	myDrawingCache->flush();
	//glClear(GL_DEPTH_BUFFER_BIT);
  	SColor scolTest(1, 1, 0, 1);
  	RenderUtils::fillRectangle(100, 100, mySize.x - 200, mySize.y - 200, scolTest, true);
// 
 	//return;
//	SColor scolTest(1, 0, 0, 1);
//	glFillRectangle(0,0, mySize.x, mySize.y, scolTest, true);
#else
	SColor scolTest(1, 0, 0, 1);
	RenderUtils::fillRectangle(0,0, mySize.x, mySize.y, scolTest, true);
#endif
#endif

#if defined(DEBUG_SHOW_FRAMERATE) && defined(DEBUG_SHOW_ONLY_FRAMERATE)
	renderFramerates(false);
	return;
#endif

	this->renderBeforeUISubclass();

    if(myUIPlane)
        myUIPlane->render();

	myTooltipManager->render();
	myDragDropManager->render();

	renderDropAnim();
	myDrawingCache->flush();

#ifdef DEBUG_SHOW_FRAMERATE
	if(Application::getInstance()->getShowFramerate())
		renderFramerates();
	myDrawingCache->resetNumFrameFlushes();	
#endif

	this->renderAfterUISubclass();

	myDrawingCache->flush();

	if(Application::getIsDebuggingEnabled())
	{
		// Render the status for activity
		bool bIsActive = ActivityManager::getInstance()->getIsActiveNoExtraDrawsCounter();
		const char* pcsActMessage = bIsActive ? "Active" : "Inactive";
		const SColor* pActColor = bIsActive ? &SColor::Red : &SColor::Green;
		myDrawingCache->addText(pcsActMessage, MAIN_FONT_MED, upToScreen(6), upToScreen(5), upToScreen(3), *pActColor, HorAlignLeft, VertAlignTop, upToScreen(0.5));

		// Render the reload button
		myDrawingCache->addRectangle(myReloadButtonRect, SColor::Green, upToScreen(0.25));
		myDrawingCache->addText("Reload", MAIN_FONT_MED, upToScreen(6), myReloadButtonRect.center().x, myReloadButtonRect.center().y, SColor::Green, HorAlignCenter, VertAlignCenter, upToScreen(0.5));

		myDrawingCache->flush();
	}

#ifndef DIRECTX_PIPELINE	
	glBindTexture(GL_TEXTURE_2D, 0);
#endif
}
/*****************************************************************************/
void Window::renderFramerates(bool bRenderSim)
{
	const int iMaxSamples = GAME_FRAMERATE/2;

	// Push the latest one on
	///	unsigned long lTime = (unsigned long)clock();
	double dTime = DateUtils::getCurrentTime();
	myFrameTimes.push_back(dTime);
	int iNumToErase = myFrameTimes.size() - (int)iMaxSamples*4;
	if(iNumToErase > 0)
		myFrameTimes.eraseFront(iNumToErase);

	// Compute the current average
	double dCurrTime, dAverage = 0.0;
	int iCurr, iNum = myFrameTimes.size();
	int iNumFrameTimesRaw = iNum;
	if(iNum > iMaxSamples)
		iNum = iMaxSamples;
	dAverage = 0.0;
	for(iCurr = iNumFrameTimesRaw - 1; iCurr > iNumFrameTimesRaw - iNum; iCurr--)
	{
		//dCurrTime = (double)(myFrameTimes[iCurr] - myFrameTimes[iCurr - 1])/CLOCKS_PER_SEC;
		dCurrTime = (double)(myFrameTimes[iCurr] - myFrameTimes[iCurr - 1]);
		dAverage += dCurrTime;
	}

	if(iNum > 1)
	{
		dAverage /= (double)(iNum);
	}

	int iSavedFlushes = myDrawingCache->getNumFrameFlushes();

	int iScreenOffset = 15;
	int iVertSpacing = 12;

	// Finally, print it:
	{
		SColor dbShadowCol(0, 0, 0,1);
		SColor dbTextCol(0, 0.8, 0,1);
		SVector2D svShadowOffset(1,1);
		char pcsTempString[32];
		sprintf(pcsTempString, "DRW: %.1f fps", 1.0/dAverage);
		myDrawingCache->addOrMeasureText(pcsTempString, MAIN_FONT_SMALL, MAIN_FONT_SMALL_DEF_SIZE, mySize.x - iScreenOffset, iScreenOffset, dbTextCol, HorAlignRight,
			VertAlignTop, 0, NULL, &dbShadowCol, &svShadowOffset, 0, true);
		myDrawingCache->flush();
	}

	// Print the sim times
	if(!bRenderSim)
		return;

	iNum = mySimFrameTimes.size();
	iNumFrameTimesRaw = iNum;
	if(iNum > iMaxSamples)
		iNum = iMaxSamples;
	dAverage = 0.0;
	//for(iCurr = 1; iCurr < iNum; iCurr++)
	for(iCurr = iNumFrameTimesRaw - 1; iCurr > iNumFrameTimesRaw - iNum; iCurr--)
	{
		//dCurrTime = (double)(mySimFrameTimes[iCurr] - mySimFrameTimes[iCurr - 1])/CLOCKS_PER_SEC;
		dCurrTime = (double)(mySimFrameTimes[iCurr] - mySimFrameTimes[iCurr - 1]);
		dAverage += dCurrTime;
	}

	if(iNum > 1)
	{
		dAverage /= (double)(iNum);
	}

	// Finally, print it:
	{
		SColor dbShadowCol(0, 0, 0,1);
		SColor dbTextCol(0, 0.8, 0,1);
		SVector2D svShadowOffset(1,1);
		char pcsTempString[32];
		if(dAverage > 0)
			sprintf(pcsTempString, "SIM: %.1f fps", 1.0/dAverage);
		else
			sprintf(pcsTempString, "SIM: --- fps");
		myDrawingCache->addOrMeasureText(pcsTempString, MAIN_FONT_SMALL, MAIN_FONT_SMALL_DEF_SIZE, mySize.x - iScreenOffset, iScreenOffset + upToScreen(iVertSpacing), dbTextCol, HorAlignRight,
			VertAlignTop, 0, NULL, &dbShadowCol, &svShadowOffset, 0, true);
		myDrawingCache->flush();
	}

	// Print flushes
	{
		SColor dbShadowCol(0, 0, 0,1);
		SColor dbTextCol(0, 0.8, 0,1);
		SVector2D svShadowOffset(1,1);
		char pcsTempString[32];
		sprintf(pcsTempString, "FLUSHES: %d", iSavedFlushes);
		myDrawingCache->addOrMeasureText(pcsTempString, MAIN_FONT_SMALL, MAIN_FONT_SMALL_DEF_SIZE, mySize.x - iScreenOffset, iScreenOffset + upToScreen(iVertSpacing)*2.0, dbTextCol, HorAlignRight,
			VertAlignTop, 0, NULL, &dbShadowCol, &svShadowOffset, 0, true);
		myDrawingCache->flush();
	}
}
/*****************************************************************************/
void Window::renderDropAnim()
{
	if(myDropAnim.getIsAnimating(false) == false)
		return;

	FLOAT_TYPE fValue = myDropAnim.getValue();
	myDrawingCache->flush();

	FLOAT_TYPE fScale = 8.0*fValue;
	FLOAT_TYPE fOpacity = 1.0 - fValue;

	myDrawingCache->addSprite("dropAnim", myDropInitPos.x, myDropInitPos.y, fOpacity, 0, fScale, fScale, 1.0, true);
}
/*****************************************************************************/
void Window::startDropAnim()
{
	myDropAnim.setAnimation(0, 1, 0.5, ClockUiPrimary);
	myDropInitPos = myLastMousePos;
}
/*****************************************************************************/
void Window::onUILayerShown(const char* pcsLayer)
{

}
/*****************************************************************************/
void Window::onWindowSizeChanged(int iNewW, int iNewH)
{
	//gLog("Window::onWindowSizeChanged from %dx%d to %dx%d context = %x\n", (int)mySize.x, (int)mySize.y, iNewW, iNewH, wglGetCurrentContext());

	mySize.set(iNewW, iNewH);

	setupGlView(iNewW, iNewH);

	if(myUIPlane)
		myUIPlane->onWindowSizeChanged(iNewW, iNewH);

	ActivityManager::getInstance()->singleValueChanged();

	// Adjust the reload rect
	myReloadButtonRect.w = upToScreen(24);
	myReloadButtonRect.h = upToScreen(10);
	myReloadButtonRect.x = iNewW - upToScreen(2.5) - myReloadButtonRect.w;
	myReloadButtonRect.y = upToScreen(2.5);
}
/*****************************************************************************/
UndoManager* Window::allocateUndoManager(UIElement* pParentElem)
{
	return new UndoManager(pParentElem);
}
/*****************************************************************************/
UIPlane* Window::allocateUIPlane(ResourceType eSourceCollection)
{
	return new UIPlane(this, eSourceCollection);
}
/*****************************************************************************/
void Window::setupGlView(int iIncomingW, int iIncomingH)
{
#ifndef DIRECTX_PIPELINE
#if defined(WIN32) || defined(MAC_BUILD)
	int iX = iIncomingW;
	int iY = iIncomingH;
#else
	int iX = iIncomingH;
	int iY = iIncomingW;
#endif

	//gLog("Window::setupGlView at %d x %d\n", iX, iY);
#ifndef USE_OPENGL2
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#if !defined(WIN32) && !defined(MAC_BUILD) && !defined(LINUX)
	glOrthof(0.0, iX, 0.0, iY, -ORTHO_DEPTH_LIMIT, ORTHO_DEPTH_LIMIT);
#else
	glOrtho(0.0, iX, 0.0, iY, -ORTHO_DEPTH_LIMIT, ORTHO_DEPTH_LIMIT);
#endif
	glViewport(0, 0, iX, iY);

	glMatrixMode(GL_MODELVIEW);
#endif

	// Clears the view with black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // 1.0f
	glClearStencil(0);


	setup2DGlView();
#ifndef USE_OPENGL2
	glLoadIdentity();

	// Sets up pointers and enables states needed for using vertex arrays and textures
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
#endif

#ifdef ALLOW_3D
#ifndef USE_OPENGL2
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glShadeModel(GL_SMOOTH);
#endif
	glDepthFunc(GL_LEQUAL);
	//glDepthFunc(GL_GEQUAL);


#endif

	setBlendMode(BlendModeNormal);
#ifndef USE_OPENGL2
	// Enable use of the texture
	glEnable(GL_TEXTURE_2D);
	// Enable blending
	glDisable(GL_FOG);
#endif

#endif
}
/*****************************************************************************/
void Window::setup2DGlView()
{
#ifndef DIRECTX_PIPELINE
	glDisable(GL_DEPTH_TEST);
	GraphicsUtils::disableAlphaTest();

	disableLighting();
	glEnable(GL_BLEND);

	glDepthMask(GL_FALSE);

#ifdef USE_OPENGL2
	RenderStateManager::getInstance()->setShaderType(ShaderSetFlatTexture);
	RenderStateManager::getInstance()->resetForOrtho(mySize.x, mySize.y, ORTHO_DEPTH_LIMIT);
#endif


#endif
}
/*****************************************************************************/
void Window::turnOnMaxAmbient()
{
#ifndef DIRECTX_PIPELINE
	GLfloat amb_col[] = { 1.0f, 1.0f, 1.0f, 1.0f };
#ifdef USE_OPENGL2
	GL2FIXME
#else
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb_col);
#endif
#endif
}
/*****************************************************************************/
void Window::restoreAmbient()
{
#ifndef DIRECTX_PIPELINE
	GLfloat amb_col[] = { 0.0f, 0.0f, 0.0f, 1.0f };
#ifdef USE_OPENGL2
	GL2FIXME
#else
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb_col);
#endif
#endif
}
/*****************************************************************************/
void Window::allowLighting(bool bForPerspective)
{
#ifndef DIRECTX_PIPELINE
//	if(bAllowLighting)
		glEnable(GL_CULL_FACE);

//	if(bAllowLighting)
	{
#ifdef USE_OPENGL2
			RenderStateManager::getInstance()->setShaderType(ShaderSetLighted);
#else
		glEnableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
#endif
	}

#ifdef ALLOW_LIGHTING
//	if(bAllowLighting)
	{
#ifdef USE_OPENGL2
		RenderStateManager::getInstance()->enableLighting();
		RenderStateManager::getInstance()->disableAllLights();

		RenderStateManager::getInstance()->enableLight(0);

		SColor scolLightColor(1, 1, 1, 1);
// 		if(myGamePlane)
// 			myGamePlane->getWeatherManager()->getLightColor(scolLightColor);
		scolLightColor.alpha = 1.0;

		RenderStateManager::getInstance()->setLightColor(0, scolLightColor);
		RenderStateManager::getInstance()->setLightAttenuation(0, 1, 0, 0);

		SColor scolWhite(1, 1, 1, 1);
		RenderStateManager::getInstance()->setLightSpecular(0, scolWhite, 20.0);

		SVector4D svLightPos;
		if(bForPerspective)
			// Negative y is toward us, z up - this is in our regular world space.
			svLightPos.set(-1700, -1700, 5900, 1);
		//svLightPos.set(-1400, -1400, 3900, 1);
		//svLightPos.set(-400, -400, 900, 1);
		else
		{
			svLightPos.set(512.0, 0.0, -900.0, 1);
			//svLightPos.set(512.0, 384.0, 900.0, 1);
		}
		RenderStateManager::getInstance()->setLightPosition(0, svLightPos);


#else
		glShadeModel(GL_SMOOTH);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		// We only do this when we render objects with scales.
		///glEnable( GL_NORMALIZE );

		//GLfloat diff_col[] = { 8.0f, 8.0f, 8.0f, 12.0f };
		//GLfloat diff_col[] = { 4.0f, 8.0f, 4.0f, 1.0f };

		GLfloat diff_col[] = { 1.0f, 1.0f, 1.0f, 1.0f };
#ifdef RENDER_FIELD_WITH_PERSPECTIVE
		if(myUIPlane)
		{
			SColor scolLightColor;
			myUIPlane->getWeatherManager()->getLightColor(scolLightColor);
			diff_col[0] = scolLightColor.r;
			diff_col[1] = scolLightColor.g;
			diff_col[2] = scolLightColor.b;
		}
#endif
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diff_col);
#endif // end if not gl 2
		restoreAmbient();

#ifdef USE_OPENGL2
		applyLightToGLState();
		RenderStateManager::getInstance()->updateLightBuffers(Application::getInstance()->getGlobalTime(ClockUniversal));
#else
		if(bForPerspective)
		{
			// In persp, -Z is to the viewer, -Y is up
			// However, for this game T6 we set this to be up,
			// since our 3D objects are mostly there
			//GLfloat lightpos[] = {0.0, -900.0, -200.0, 1.0f};
			//			GLfloat lightpos[] = {0.0, 0.0, -900.0, 1.0f};
			GLfloat lightpos[] = {0.0, 0.0, 900.0, 1.0f};

			//GLfloat lightpos[] = {0.0, 0.0, -900.0, 1.0f};
			glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
		}
		else
		{
			// In ortho, +Z is up???
			//GLfloat lightpos[] = {500.0, 0.0, 400.0, 1.0f};

			// Note that we keep flipping the Z axis depending on traingle orientation.
			GLfloat lightpos[] = {512.0, 384.0, 900.0, 1.0f};
			//GLfloat lightpos[] = {10.0, 0.0, 20.0, 1.0f};
			glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
		}

		glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);
		glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0);
		glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0);
#endif
	}

#endif
#endif
}
/*****************************************************************************/
void Window::disableLighting()
{
#ifndef DIRECTX_PIPELINE
#ifdef USE_OPENGL2
	RenderStateManager::getInstance()->disableLighting();
	RenderStateManager::getInstance()->updateLightBuffers(Application::getInstance()->getGlobalTime(ClockUniversal));

	RenderStateManager::getInstance()->setShaderType(ShaderSetFlatTexture);
#else

	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
#endif
	glDisable(GL_CULL_FACE);
#endif
}
/*****************************************************************************/
void Window::setup3DGlView(bool bAllowLighting, bool bPerspective, bool bDisableDepthClearing, bool bEnableAlpha)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE3
#else

#ifndef RENDER_FIELD_WITH_PERSPECTIVE
	_ASSERT(!bPerspective);
#endif
	if(bPerspective)
	{
		int iX = mySize.x;
		int iY = mySize.y;

#if defined(ORIENTATION_LANDSCAPE)

	#if !defined(MAC_BUILD) && !defined(WIN32)
		glViewport(0, 0, iY, iX);
	#else
		glViewport(0, 0, iX, iY);
	#endif

#else
		glViewport(0, 0, iX, iY);
#endif


#ifdef USE_OPENGL2

		RenderStateManager* pRSManager = RenderStateManager::getInstance();

		SMatrix4 rMatrix;
		rMatrix.appendFrustumPerspectiveProjection(PERSP_FOV_ANGLE, (GLfloat)iX/(GLfloat)iY, 1.0f, getPerspDepthLimit(), true);
		pRSManager->setProjectionMatrix(rMatrix);

		// View
		rMatrix.resetToIdentity();
		rMatrix.appendTranslation(0, 0, PERPS_Z_CAM_OFFSET);

#if defined(ORIENTATION_LANDSCAPE)
#if !defined(MAC_BUILD) && !defined(WIN32)
		rMatrix.appendYRotation(sanitizeDegAngle(-PERSP_CAMERA_ANGLE));
		rMatrix.appendZRotation(sanitizeDegAngle(-90));
#else
		rMatrix.appendXRotation(sanitizeDegAngle(PERSP_CAMERA_ANGLE));
#endif		
#else
		rMatrix.appendXRotation(sanitizeDegAngle(PERSP_CAMERA_ANGLE));
#endif

		pRSManager->setViewMatrix(rMatrix);
		rMatrix.toFloatArray(myModelviewMatrix, false);

#else

		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix

		glLoadIdentity();									// Reset The Projection Matrix
		//gluPerspective(PERSP_FOV_ANGLE,(GLfloat)iX/(GLfloat)iY, 1.0f,ORTHO_DEPTH_LIMIT*2.0);
		GraphicsUtils::perspectiveGl(PERSP_FOV_ANGLE,(GLfloat)iX/(GLfloat)iY, 1.0f, getPerspDepthLimit()); //561

		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
		glLoadIdentity();									// Reset The Modelview Matrix


		//  NOTE: If changing the camera setup, we may need to change the Y-axis
		// negation in RenderUtils::renderObject3D() which compensates for this.
		glTranslatef(0, 0, PERPS_Z_CAM_OFFSET);

#if defined(ORIENTATION_LANDSCAPE)
	#if !defined(MAC_BUILD) && !defined(WIN32)
		glRotatef(-PERSP_CAMERA_ANGLE, 0.0, 1, 0);

		// We need to rotate the camera by 90 around the y axis... or z?
		glRotatef(-90, 0, 0, 1);
	#else
		 glRotatef(PERSP_CAMERA_ANGLE, 1.0, 0, 0);
	#endif
#else
		glRotatef(PERSP_CAMERA_ANGLE, 1.0, 0, 0);
#endif

#endif // not gl2
	}

#ifdef USE_OPENGL2
	if(bAllowLighting)
		RenderStateManager::getInstance()->setShaderType(ShaderSetLighted);
	else
		RenderStateManager::getInstance()->setShaderType(ShaderSetFlatTexture);
#endif

//	if(bAllowLighting)
//		glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_LIGHTING);

	// Reset this here because "stencil" on mac will change these.
	gClearDepth(1.0f);

	GraphicsUtils::setAlphaTestCutoffFunc(GL_GREATER, ALPHA_CUTOFF_VALUE_3D);
	if(bEnableAlpha)
		GraphicsUtils::enableAlphaTest();

	glDepthMask(GL_TRUE);

	if(!bDisableDepthClearing && (myLastDepthClearTime == 0 || myLastDepthClearTime != Application::getInstance()->getGlobalTime(ClockUniversal)))
	{
//		DEPTH // with human isn't worknig.

		glClear(GL_DEPTH_BUFFER_BIT);
		myLastDepthClearTime = Application::getInstance()->getGlobalTime(ClockUniversal);
	}

	if(bAllowLighting)
		allowLighting(bPerspective);

#endif


	/*
//	glTranslatef(0,0.0f,-66.0f);
static FLOAT_TYPE fTemp = 0.0;
fTemp += 1.0;
if(fTemp >= 360)
	fTemp -= 360.0;
	//glRotatef(fTemp, 0, 1, 0);
	//glBegin(GL_TRIANGLES);						// Drawing Using Triangles
//	glVertex3f( 0.0f, 1.0f, 0.0f);				// Top
//	glVertex3f(-1.0f,-1.0f, 0.0f);				// Bottom Left
//	glVertex3f( 1.0f,-1.0f, 0.0f);				// Bottom Right


//	glVertex3f(-5.0f, -5.0f, 5.0f);				// Top
//	glVertex3f(5.0f,-5.0f, 5.0f);				// Bottom Left
//	glVertex3f(5.0f,5.0f, 5.0f);				// Bottom Right

//	glEnd();
*/
}
/*****************************************************************************/
void Window::renderToTexture(void)
{
	myDrawingCache->getTextCacher()->checkForCleaning();
	myDrawingCache->getTextCacher()->createQueued();

#ifdef ALLOW_OPENGL_STROKE_GENERATION
	// Now, do the same for other cachers
	BrushStrokeOffscreenCacher::getInstance()->createQueued();
#endif
}
/*****************************************************************************/
BlendModeType Window::setBlendMode(BlendModeType eNewMode)
{
#ifndef LINUX
	if(myCurrentBlendMode == eNewMode)
		return myCurrentBlendMode;
#endif
	BlendModeType eOldMode = myCurrentBlendMode;
	myCurrentBlendMode = eNewMode;

#ifdef DIRECTX_PIPELINE
	g_pDxRenderer->setBlendState(eNewMode);
#else
	switch(eNewMode)
	{
		// According to one page, for premultiplied alpha bitmaps, we need:
		// 	Premultiplied -> blend mode(ONE, ONE_MINUS_SRC_ALPHA)
		// Straight alpha-> blend mode (SRC_ALPHA, ONE_MINUS_SRC_ALPHA)
		// Textures coming from phtooshop are premultiplied, where as textures coming from PVRText are straight.
		// So for them, we have to use the text mode.
		case BlendModeNormal:glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);break;
		case BlendModeAdditive:glBlendFunc(GL_SRC_ALPHA, GL_ONE);break;
		case BlendModeText:glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);break;
		case BlendModeDiscolour:glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);break;
		//case BlendModeSpecial:glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);break;
	}
#endif

	return eOldMode;
}
/*****************************************************************************/
void Window::beginClosing(ClosingType eType)
{
	myBeganClosingFlag = eType;
}
/*****************************************************************************/
void Window::checkForClosing()
{
	if(myBeganClosingFlag != ClosingNone && getCanFinallyClose())
	{
		if(myBeganClosingFlag == ClosingWindow)
			this->close();
		else if(myBeganClosingFlag == ClosingTerminateApp)
			Application::quitApp();
		ELSE_ASSERT;
	}
}
/*****************************************************************************/
void Window::getFramerateSamples(string& strOut, int iMaxSamples)
{
	string strTemp;
	strOut = "";
	double dCurrTime;
	int iCount;
	int iCurr, iNum = myFrameTimes.size();
	for(iCount = 0, iCurr = iNum - 1; iCurr > 0; iCurr--, iCount++)
	{
		if(iCount > 0)
			strOut += ", ";
		dCurrTime = (double)(myFrameTimes[iCurr] - myFrameTimes[iCurr - 1]);
		if(dCurrTime > 0)
		{
			StringUtils::numberToNiceString(1.0/dCurrTime, 1, false, strTemp);
			strOut += strTemp + "s";
		}
		else
			strOut += "-";

		if(iMaxSamples > 0 && iCount >= iMaxSamples)
			break;
	}
}
/*****************************************************************************/
Window* Window::getWindowWithLayerShown(const char* pcsLayer)
{
	UIPlane* pUiPlane;
	WindowManager::Iterator wi;
	for(wi = WindowManager::getInstance()->windowsBegin(); !wi.isEnd(); wi++)
	{
		pUiPlane = wi.getWindow()->getUIPlane();
		if(pUiPlane && pUiPlane->getIsLayerShown(pcsLayer))
			return wi.getWindow();
	}

	return NULL;
}
/*****************************************************************************/
void Window::setNewTextToActiveElement(const char* pcsNewText)
{
	if(!myFocusElement)
		return;

	myFocusElement->setText(pcsNewText);
}
/*****************************************************************************/
void Window::applyLightToGLState()
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
#ifdef USE_OPENGL2

	SVector4D svLightPos;
	/*
	#ifdef _DEBUG
	// TEMP test
	SColor scolTemp;
	scolTemp.set(1,0,0,1);
	svLightPos.set(-200, -900, 0, 1);
	RenderStateManager::getInstance()->enableLight(1);
	RenderStateManager::getInstance()->setLightColor(1, scolTemp);
	RenderStateManager::getInstance()->setLightPosition(1, svLightPos);
	RenderStateManager::getInstance()->setLightAttenuation(1, 1, 0.002, 0);

	scolTemp.set(0,0,1,1);
	svLightPos.set(200, -900, 0, 1);
	RenderStateManager::getInstance()->enableLight(2);
	RenderStateManager::getInstance()->setLightColor(2, scolTemp);
	RenderStateManager::getInstance()->setLightPosition(2, svLightPos);
	RenderStateManager::getInstance()->setLightAttenuation(2, 1, 0, 0);

	#endif
	*/
	int iLight, iNumLights = min(MAX_CUSTOM_LIGHTS, myTotalLights);
	for(iLight = 0; iLight < iNumLights; iLight++)
	{
		RenderStateManager::getInstance()->enableLight(iLight + NUM_FIXED_LIGHTS);
		RenderStateManager::getInstance()->setLightColor(iLight + NUM_FIXED_LIGHTS, myLights[iLight].myColor);

		svLightPos = myLights[iLight].myPosition;
		svLightPos.y -= PERSP_FIXED_Y_OFFSET;
		svLightPos.z += PERSP_FIXED_Z_OFFSET;
		RenderStateManager::getInstance()->setLightPosition(iLight + NUM_FIXED_LIGHTS, svLightPos);

		RenderStateManager::getInstance()->setLightAttenuation(iLight + NUM_FIXED_LIGHTS, myLights[iLight].myAttenuation.x, myLights[iLight].myAttenuation.y,  myLights[iLight].myAttenuation.z);
	}

	RenderStateManager::getInstance()->setNumTotalLights(myTotalLights + NUM_FIXED_LIGHTS);

#else

	GLfloat diff_col[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat lightpos[] = {0.0, 0.0, 0.0, 1.0f};
	GLfloat lightdir[] = {0.0, 0.0, -1.0, -1.0f};
	int iLight, iNumLights = min(MAX_CUSTOM_LIGHTS, myTotalLights);
	for(iLight = 0; iLight < iNumLights; iLight++)
	{
		glEnable(GL_LIGHT0 + iLight + NUM_FIXED_LIGHTS);

		lightpos[0] = myLights[iLight].myPosition.x;
		lightpos[1] = myLights[iLight].myPosition.y - PERSP_FIXED_Y_OFFSET;
		lightpos[2] = myLights[iLight].myPosition.z + PERSP_FIXED_Z_OFFSET;
		glLightfv(GL_LIGHT0 + iLight + NUM_FIXED_LIGHTS, GL_POSITION, lightpos);

		diff_col[0] = myLights[iLight].myColor.r;
		diff_col[1] = myLights[iLight].myColor.g;
		diff_col[2] = myLights[iLight].myColor.b;
		glLightfv(GL_LIGHT0 + iLight + NUM_FIXED_LIGHTS, GL_DIFFUSE, diff_col);
		//glLightfv(GL_LIGHT0 + iLight + NUM_FIXED_LIGHTS, GL_EMISSION, diff_col);

		glLightf(GL_LIGHT0 + iLight + NUM_FIXED_LIGHTS, GL_CONSTANT_ATTENUATION, myLights[iLight].myAttenuation.x);
		glLightf(GL_LIGHT0 + iLight + NUM_FIXED_LIGHTS, GL_LINEAR_ATTENUATION, myLights[iLight].myAttenuation.y);
		glLightf(GL_LIGHT0 + iLight + NUM_FIXED_LIGHTS, GL_QUADRATIC_ATTENUATION, myLights[iLight].myAttenuation.z);


		// 		glLightf (GL_LIGHT0 + iLight + NUM_FIXED_LIGHTS, GL_SPOT_CUTOFF, 50.0f);
		// 		glLightf (GL_LIGHT0 + iLight + NUM_FIXED_LIGHTS, GL_SPOT_EXPONENT, 28.0);
		// 		glLightfv (GL_LIGHT0 + iLight + NUM_FIXED_LIGHTS, GL_SPOT_DIRECTION, lightdir);
	}

	// Disable the rest
	for(; iLight < MAX_CUSTOM_LIGHTS; iLight++)
		glDisable(GL_LIGHT0 + iLight + NUM_FIXED_LIGHTS);
#endif
#endif
}
/*****************************************************************************/
void Window::addFrameCustomLight(SVector2D& svCenter, FLOAT_TYPE fElevation, SColor& scol, FLOAT_TYPE fAtten1, FLOAT_TYPE fAtten2, FLOAT_TYPE fAtten3)
{
	myLights[myCurrNumLights].myColor = scol;
	myLights[myCurrNumLights].myPosition.set(svCenter.x, svCenter.y, fElevation, 1.0);
	myLights[myCurrNumLights].myAttenuation.set(fAtten1, fAtten2, fAtten3);

	// Just cycle...
	myTotalLights++;
	myCurrNumLights++;
	if(myCurrNumLights >= MAX_CUSTOM_LIGHTS)
		myCurrNumLights -= MAX_CUSTOM_LIGHTS;
}
/*****************************************************************************/
void Window::createAndPushOverrideUndoManager(UIElement* pParentElem)
{
#ifdef _DEBUG
	string strDbTemp;
	pParentElem->getFullPath(strDbTemp);
	gLog("Pushing override undo manager: %s\n", strDbTemp.c_str());
	_ASSERT(myOverrideUndoManagers.size() == 0);
#endif

	UndoManager* pOverrideUndoManager = this->allocateUndoManager(pParentElem);
	myOverrideUndoManagers.push_back(pOverrideUndoManager);
}
/*****************************************************************************/
void Window::destroyAndPopOverrideUndoManager(UIElement* pParentElem)
{
#ifdef _DEBUG
	string strDbTemp;
	pParentElem->getFullPath(strDbTemp);
	gLog("Popping override undo manager: %s\n", strDbTemp.c_str());
#endif
	_ASSERT(myOverrideUndoManagers.size() > 0);
	if(myOverrideUndoManagers.size() > 0)
	{
		UndoManager* pManager = myOverrideUndoManagers.back();
		delete pManager;
		myOverrideUndoManagers.pop_back();
	}
}
/*****************************************************************************/
UndoManager* Window::getLastOverrideUndoManager()
{
	if(myOverrideUndoManagers.size() > 0)
		return myOverrideUndoManagers.back();
	else
		return NULL;
}
/*****************************************************************************/
bool Window::processAction(UIElement* pElem, const string& strAction, const char* pcsTargetDataSource, PropertyType eActionValue1Prop, PropertyType eActionValue2Prop, PropertyType eActionValue3Prop, MouseButtonType eButton)
{
	bool bDidHandle = false;
	if(!bDidHandle)
		bDidHandle = this->processActionSubclass(pElem, strAction, pcsTargetDataSource, eActionValue1Prop, eActionValue2Prop, eActionValue3Prop, eButton);
	return bDidHandle;
}
/*****************************************************************************/
UNIQUEID_TYPE Window::pushCursor(CursorType eType)
{
	return myCursorManager->pushCursor(eType);
}
/*****************************************************************************/
void Window::popCursor(UNIQUEID_TYPE idCursor)
{
	myCursorManager->popCursor(idCursor);
}
/*****************************************************************************/
void Window::restoreLastCursor()
{
	myCursorManager->restoreLastCursor();
}
/*****************************************************************************/
void Window::setIsTabletButtonDown(bool bValue)
{
	myIsTabletButtonDown = bValue;
	if(!bValue)
		myLastTabletButtonUpTime = Application::getInstance()->getGlobalTime(ClockUniversal);
}
/*****************************************************************************/
bool Window::getIsTabletButtonDown()
{
	return myIsTabletButtonDown || (Application::getInstance()->getGlobalTime(ClockUniversal) <= myLastTabletButtonUpTime + Application::secondsToTicks(0.1));
}
/*****************************************************************************/
void Window::onTabletMousePan(SVector2D& svMousePos, FLOAT_TYPE fDelta)
{

}
/*****************************************************************************/
};
