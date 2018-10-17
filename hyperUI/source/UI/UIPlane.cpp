#include "stdafx.h"

#if !defined(WIN32) && !defined(LINUX)
//#include "SoundManager.h"
//extern SoundManager *g_pSoundManager;
#else
//#include "WinSoundManager.h"
//extern WinSoundManager *g_pSoundManager;
#endif

#define UI_LOG_FILE				"ui_log.txt"
#define DEMO_TAG				"demoonly"

namespace HyperUI
{
//TElementAlphaMapMemCacher UIPlane::theElementsMapMemCacher(16);
/*****************************************************************************/
UIPlane::UIPlane(Window* pParentWindow, ResourceType eSourceCollection)
 : IPlane(pParentWindow), myElementsMapMemCacher(16), myElementsMap(&myElementsMapMemCacher) // , myTempGeneratedUniqueNames(&myElementsMapMemCacher)
{
	myIsCachedShowingAnythingValid = false;
	myLastMouseElement = NULL;
	myLastGlideElement = NULL;
	myIsFullyObscuring = false;
	myMouseLockedElem = NULL;
	myIsCachedMaxPausingOpacityValid = false;
	myIsCachedMaxNonPausingOpacityValid = false;
	myCurrRenderingPass = RenderingNormal;


#ifdef LOG_THY_UI
	FileUtils::deleteFile(UI_LOG_FILE);
#endif

	myIsRenderableFlagDirty = true;
	myInitCollectionType = -1;

	// Here, we create UIElement instances out of every item we have
	initFromCollection(eSourceCollection);
}
/*****************************************************************************/
UIPlane::~UIPlane()
{
	// Note: right now this only happens when we quit the game, so we
	// don't need to clear the vectors. We'll do so anyway.
	// These are deallocated in the appropriate destructor
	int iCurrLayer;
	int iNumRenderStages = myElementsInStages.getCurrentCapacity();
	for(iCurrLayer = 0; iCurrLayer < iNumRenderStages; iCurrLayer++)
	{
		if(!myElementsInStages[iCurrLayer])
			continue;

		int iElem, iNumElems = myElementsInStages[iCurrLayer]->size();
		for(iElem = 0; iElem < iNumElems; iElem++)
			delete (*myElementsInStages[iCurrLayer])[iElem];
		myElementsInStages[iCurrLayer]->clear();
	}

	// Note that here we clear but not deallocate the myUIElements
	// vector. This is because it holds a copy of the objects in myElementsInStages
	// which have already been deallocated, and if we don't deallocate them here,
	// we'll crash in GamePlane when we try to deallocate them a second time.
	myUIElements.clear();

//	TElementAlphaMapMemCacher *pMemCacherPtr = &theElementsMapMemCacher;
	myElementsMap.clear();
	//myTempGeneratedUniqueNames.clear();
}
/*****************************************************************************/
UIElement* UIPlane::createAndAddUIElement(const char* pcsId)
{
	UIElement* pRes = NULL;
	if(!pRes)
	{
		ResourceCollection* pUIPrelimElemsColl = ResourceManager::getInstance()->getCollection(myInitCollectionType);
		pRes = pUIPrelimElemsColl->createItemOfType<UIElement>(pcsId, this);
	}
/*
	if(!pRes)
	{
		ResourceCollection* pUIPrelimElemsColl = ResourceManager::getInstance()->getCollection(ResourceSepWindowUIElements);
		pRes = pUIPrelimElemsColl->createItemOfType<UIElement>(pcsId, this);
	}
*/

	myUIElements.push_back(pRes);
	return pRes;
}
/*****************************************************************************/
UIElement*  UIPlane::getElementInLayerAt(FLOAT_TYPE fX, FLOAT_TYPE fY, const char* pcsLayer, bool bGlidingOnly)
{
	int iCurrStage;
	int iElem, iNumElems;
	UIElement* pElement;

	// In reverse since we want the topmost one.
	int iNumRenderStages = myElementsInStages.getCurrentCapacity();
	for(iCurrStage = iNumRenderStages - 1; iCurrStage >= 0; iCurrStage--)
	{
		if(!myElementsInStages[iCurrStage])
			continue;

		iNumElems = myElementsInStages[iCurrStage]->size();
		for(iElem = 0; iElem < iNumElems; iElem++)
		{
			// Check if it is visible at all - i.e. its state is right.
			pElement = (*myElementsInStages[iCurrStage])[iElem];



			if(pElement->getIsVisible() == false)
				continue;

			if(pElement->getStateAnimProgress() < FLOAT_EPSILON)
				continue;

			if(strcmp(pElement->getStringProp(PropertyLayer), pcsLayer) != 0)
				continue;

			// We wanted earlier to ignore these so they don't obscure things;
			// however, this disables cases where a parent window with solid
			// color can have clickable buttons. So we enable it, but should
			// use layers to prevent obscuring by the full-screen color
			// elements.
//			if(dynamic_cast<UISolidColorElement*>(pElement))
//				continue;

			if(pElement->isInsideLocal(fX, fY, false))
			{

				// Here, we check if we're bypass, and if so, we try to find our child.
				// If we can't, pretend we don't exist.
				if(pElement->getBoolProp(PropertyIsPassthrough))
				{
					UIElement* pRes = pElement->getChildAtRecursive(fX, fY, true, bGlidingOnly);
					if(pRes)
						return pRes;
				}
				else
					return pElement;
			}


		}
	}

	return NULL;
}
/*****************************************************************************/
UIElement* UIPlane::getTopmostShownElementAt(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bGlidingOnly)
{
	UIElement* pCurrRes = NULL;
	int iTopmostRenderStageUILayer = -1;

	UIElement* pParentElem;
	int iMaxElemStage;

	int iCurrUIStage;
	UIElement* pElem;
	int iLayer, iNumLayers = myCurrentlyShownLayers.size();
	for(iLayer = iNumLayers - 1; iLayer >= 0; iLayer--)
	{
		pElem = this->getElementInLayerAt(fX, fY, myCurrentlyShownLayers[iLayer].c_str(), bGlidingOnly);
		if(pElem)
		{
			// The below is to see if there's an element in the earlier shown
			// layer but with larger UI stage than us. Visually, those appear on top,
			// and so should be selected first.
			iMaxElemStage = -1;
			for(pParentElem = pElem; pParentElem != NULL; pParentElem = pParentElem->getParent<UIElement>())
			{
				iCurrUIStage = pParentElem->getNumProp(PropertyZIndex);
				if(iCurrUIStage > iMaxElemStage)
					iMaxElemStage = iCurrUIStage;
			}

			if(iMaxElemStage > iTopmostRenderStageUILayer)
			{
				iTopmostRenderStageUILayer = iMaxElemStage;
				pCurrRes = pElem;
			}
		}
			//return pElem;
	}

	return pCurrRes;
	//return NULL;

}
/*****************************************************************************/
UIElement*  UIPlane::getTopLayerElementAt(FLOAT_TYPE fX, FLOAT_TYPE fY)
{
	int iCurrElement, iNumElements;
	UIElement* pElement;

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		if(myLastShownLayerName != pElement->getStringProp(PropertyLayer))
			continue;

		// Ignore solid colours so they don't obscure things.
		// We wanted earlier to ignore these so they don't obscure things;
		// however, this disables cases where a parent window with solid
		// color can have clickable buttons. So we enable it, but should
		// use layers to prevent obscuring by the full-screen color
		// elements.
//			if(dynamic_cast<UISolidColorElement*>(pElement))
//				continue;

//			if(findShownLayer(pElement->getStringProp(PropertyUiObjLayerName)) < 0)
//				continue;

		if(pElement->isInsideLocal(fX, fY, false))
			return pElement;
//				pElement->onPressed();
	}

	return NULL;
}
/*****************************************************************************/
UIElement* UIPlane::getFirstElementAt(FLOAT_TYPE fX, FLOAT_TYPE fY)
{
	int iCurrElement, iNumElements;
	UIElement* pElement;

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;
		if(pElement->isInsideLocal(fX, fY, false))
			return pElement;
	}

	return NULL;
}
/*****************************************************************************/
void UIPlane::onTouchDown(TTouchVector& vecTouches)
{
	FLOAT_TYPE fX = vecTouches[0].myPoint.x;
	FLOAT_TYPE fY = vecTouches[0].myPoint.y;

	UIElement* pElement = getTopmostShownElementAt(fX, fY, false);
	pElement = getFinalChildAtCoords(fX, fY, pElement, false, false);
#ifdef LOG_THY_UI
	printTouches("Touch down:", &vecTouches, pElement);
#endif
	if(pElement)
	{
		pElement->onMouseEnter(vecTouches);
		pElement->onPressed(vecTouches);
	}
	myLastMouseElement = pElement;
}
/*****************************************************************************/
const char* UIPlane::getLastShownLayerName()
{
	return myLastShownLayerName.c_str();
}
/*****************************************************************************/
void UIPlane::onMouseWheel(SVector2D& svMousePos, FLOAT_TYPE fDelta)
{
	// Note that last two guys - they force us to ignore contents of tables.
	UIElement* pElement = getTopmostShownElementAt(svMousePos.x, svMousePos.y, false);
	pElement = getFinalChildAtCoords(svMousePos.x, svMousePos.y, pElement, false, false);

	if(pElement)
		pElement->onMouseWheelRecursive(fDelta);
}
/*****************************************************************************/
void UIPlane::onCursorGlide(TTouchVector& vecTouches)
{
    FLOAT_TYPE fX = vecTouches[0].myPoint.x;
    FLOAT_TYPE fY = vecTouches[0].myPoint.y;

    UIElement* pElement = getTopmostShownElementAt(fX, fY, true);
	if(pElement)
		pElement = getFinalChildAtCoords(fX, fY, pElement, false, true);

	// See if our glide element is not the same as before, and call glide one
	// last time. This lets us do things essentially on mouse leave.
	bool bCalledEnter = false;
	if(myLastGlideElement != pElement)
	{
		if(myLastGlideElement)
			myLastGlideElement->onCursorGlideLeave(vecTouches);
		if(pElement)
		{
			bCalledEnter = true;
			pElement->onCursorGlideEnter(vecTouches);
		}
	}

    if(pElement && !bCalledEnter)
		pElement->onCursorGlide(vecTouches);

	myLastGlideElement = pElement;
}
/*****************************************************************************/
void UIPlane::onTouchMove(TTouchVector& vecTouches)
{
	FLOAT_TYPE fX = vecTouches[0].myPoint.x;
	FLOAT_TYPE fY = vecTouches[0].myPoint.y;

	UIElement* pElement;
	if(myMouseLockedElem)
		pElement = myMouseLockedElem;
	else
	{
		pElement = getTopmostShownElementAt(fX, fY, false);
#ifdef _DEBUG
		UIElement* pOldElem = pElement;
#endif
		pElement = getFinalChildAtCoords(fX, fY, pElement, false, false);
	}

#ifdef _DEBUG
	if(pElement)
	{
	    if(strcmp(pElement->getStringProp(PropertyId), "panel1") == 0 ||
		strcmp(pElement->getStringProp(PropertyId), "panel2") == 0)
	    {
		//pElement = getFinalChildAtCoords(fX, fY, pOldElem);
	    }
	}
#endif

#ifdef LOG_THY_UI
	printTouches("Touch move:", &vecTouches, pElement);
#endif
	if(pElement != myLastMouseElement && (!myLastMouseElement || myLastMouseElement->canReleaseFocus()))
	{
		if(myLastMouseElement)
			myLastMouseElement->onMouseLeave(vecTouches);
		if(pElement)
		{
			pElement->onMouseEnter(vecTouches);
		}
		myLastMouseElement = pElement;
	}

	if(myLastMouseElement)
		myLastMouseElement->onMouseMove(vecTouches);
}
/*****************************************************************************/
bool UIPlane::onTouchUp(TTouchVector& vecTouches, bool bIgnoreActions)
{
	FLOAT_TYPE fX = vecTouches[0].myPoint.x;
	FLOAT_TYPE fY = vecTouches[0].myPoint.y;

	myLastTouchUpPos = vecTouches[0].myPoint;

	getParentWindow()->getDragDropManager()->setIsDragBeginAllowed(false);
	myHideAllSubmenus = true;
	onTouchMove(vecTouches);
	getParentWindow()->getDragDropManager()->setIsDragBeginAllowed(true);

	UIElement* pElement;
	if(myMouseLockedElem)
		pElement = myMouseLockedElem;
	else
	{
		pElement = getTopmostShownElementAt(fX, fY, false);
#ifdef LOG_THY_UI
		printTouches("Touch up init:", &vecTouches, pElement);
#endif
		pElement = getFinalChildAtCoords(fX, fY, pElement, false, false);
	}

#ifdef LOG_THY_UI
	printTouches("Touch up:", &vecTouches, pElement);
#endif
	if(pElement != myLastMouseElement && myLastMouseElement
		&& !myLastMouseElement->canReleaseFocus())
		pElement = myLastMouseElement;
#ifdef LOG_THY_UI
	printTouches("Touch up final elem:", &vecTouches, pElement);
#endif
	if(pElement)
	{
		pElement->onReleased(vecTouches, bIgnoreActions);

		MouseButtonType eMouseButton = getParentWindow()->getCurrMouseButtonDown();

		// Now, we need to have generalized rmb action mechanism, not just
		// for buttons. For now, we exclue the elemenet as button because
		// it already does it in UIButtonElement::onReleased(). But for every
		// other elem, if it has an RMB prop, we do the action thingie:
		if(eMouseButton == MouseButtonRight && pElement->getIsEnabled() && as<UIButtonElement>(pElement) == NULL && pElement->doesPropertyExist(PropertyRmbAction))
			onButtonClicked(pElement, eMouseButton, true);

		if(pElement->getIsEnabled() && pElement->getAllowKeepingFocus() && eMouseButton == MouseButtonLeft)
			getParentWindow()->setFocusElement(pElement);
		pElement->onMouseLeave(vecTouches);
	}
	else
	{
		// Kill focus.
		getParentWindow()->setFocusElement(NULL);
	}
	myLastMouseElement = NULL;
#ifdef LOG_THY_UI
	printTouches("DONE", NULL, NULL);
#endif

	// Do not hide for anything in the submenu that was clicked.
	// This is used for not hiding RMB menus when controls are adjusted
	// on them.
	// The exception is a special case where we need to hide Recent List item
	// menu when an item is clicked. So we say if this clicked element
	// is a submenu itself.
	if(myHideAllSubmenus && pElement && pElement->getParentTagged<UIElement>("submenu") && !as<UIMenuElement>(pElement) )
		myHideAllSubmenus = false;
	if(pElement && pElement->getBoolProp(PropertyDontHideSubmenusOnClick))
		myHideAllSubmenus = false;

	// Hide all submenus just in case:
	if(myHideAllSubmenus)
	    this->hideAllWithTag("submenu", NULL, true, this, AnimOverActionGenericCallback);

	bool bHidePopup = true;
	if(bHidePopup && pElement && pElement->getBoolProp(PropertyDontHidePopupOnClick))
		bHidePopup = false;
	if(bHidePopup)
		this->hideLayersWithTag("popup");

	unlockMouseCursor();

	return pElement != NULL;
}
/*****************************************************************************/
UIElement* UIPlane::getElementByIdInternal(const char* pcsId, bool bRecurse, bool bIncludeOldDefType)
{
	UIElement** pAlphaRes = myElementsMap.findSimple(pcsId);
	UIElement* pFinalRes = pAlphaRes ? *pAlphaRes : NULL;

	if(!pFinalRes && bIncludeOldDefType)
		pFinalRes = getElementByIdInternalSLOW(pcsId, true, bIncludeOldDefType);

#ifdef _DEBUG
	// We skip the tables since they don't treat their cells as children,
	// and so would return NULL on the debug path.
	bool bSkipCheck = false;
	if(pFinalRes && pFinalRes->getParentOfType<UITableElement>())
		bSkipCheck = true;

	if(!bSkipCheck)
	{
		// First, we must make sure that if we have a result from the elem map,
		// the elem is found without the old name:
		// The old assert could trigger if we had a child added twice to the same elem from the
		// same id, because it would find the new child from the first time we created it
		// in one of the children of the elem itself by old name.
		UIElement* pDbCheckElementNoOldName = getElementByIdInternalSLOW(pcsId, true, false);
		_ASSERT(!pAlphaRes || *pAlphaRes == pDbCheckElementNoOldName);

		if(!pAlphaRes && bIncludeOldDefType)
		{
			UIElement* pDbCheckElement = getElementByIdInternalSLOW(pcsId, true, bIncludeOldDefType);
			_ASSERT(pFinalRes == pDbCheckElement);
		}
	}
#endif
    return pFinalRes;
}
/*****************************************************************************/
UIElement* UIPlane::getElementByIdInternalSLOW(const char* pcsId, bool bRecurse, bool bIncludeOldDefId)
{
	int iCurrElement, iNumElements;
	UIElement* pElement, *pCElem;

	string strType = pcsId;
	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;
		if(pElement->getStringProp(PropertyId) == strType)
			return pElement;
		if(bIncludeOldDefId && pElement->doesPropertyExist(PropertyOldId) && pElement->getStringProp(PropertyOldId) == strType)
			return pElement;
		else if(bRecurse)
		{
			pCElem = pElement->getChildById(pcsId, true, bIncludeOldDefId);
			if(pCElem)
				return pCElem;
		}
	}

	return NULL;
}
/*****************************************************************************
void UIPlane::hideSingleElement(const char* pcsType)
{
	myIsRenderableFlagDirty = true;
	UIElement *pElem = getElementByType<UIElement>(pcsType, true);
	if(!pElem)
		return;
	pElem->hide();
}
/*****************************************************************************
void UIPlane::showSingleElement(const char* pcsType)
{
	myIsRenderableFlagDirty = true;
	myIsCachedShowingAnythingValid = false;
	UIElement *pElem = getElementByType<UIElement>(pcsType, true);
	if(!pElem)
		return;
	pElem->show();
}
/*****************************************************************************
FLOAT_TYPE UIPlane::getMaxUiOpacity(const char* pcsExludeLayer)
{
	if(myIsCachedMaxOpacityValid && !pcsExludeLayer)
		return myCachedMaxOpacity;

	myCachedMaxOpacity = 0.0;

	FLOAT_TYPE fCurrOpacity;
	// Otherwise, we may still be in the process of animating something.
	int iCurrClass;
	int iCurrElement, iNumElements;
	bool bHaveAnimating = false;
	UIElement* pElement;
	SVector2D svDummy;

	bool bFoundExcludedLayer = false;
	bool bHaveExcludedLayersThatAreAnimating = false;

	for(iCurrClass = 0; iCurrClass < GameObjectClassLastPlaceholder; iCurrClass++)
	{
		iNumElements = myUIElements.size();
		for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
		{
			pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
			if(!pElement)
				continue;

			fCurrOpacity = pElement->getStateAnimProgress();

			if(pcsExludeLayer && strcmp(pElement->getStringProp(PropertyUiObjLayerName), pcsExludeLayer) == 0)
			{
				bFoundExcludedLayer = true;
				if(fCurrOpacity < 1.0)
					bHaveExcludedLayersThatAreAnimating = true;
				if(!pElement->getIsAnimating())
					continue;
			}

			if(pElement->getIsFullyHidden())
				continue;

			if(!bHaveAnimating && pElement->getIsAnimating())
				bHaveAnimating = true;

			//pElement->getLocalPosition(svDummy, &fCurrOpacity, NULL);
			//fCurrOpacity = pElement->getCachedOpacity();
			if(fCurrOpacity > myCachedMaxOpacity)
				myCachedMaxOpacity = fCurrOpacity;
		}
	}

	// Little hack: if we have any layers in the shown department,
	// make sure we return a non-zero opacity, otherwise things
	// are never going to show.
	if(myCachedMaxOpacity == 0 && myCurrentlyShownLayers.size() > 0)
	{
		if(!bFoundExcludedLayer || bHaveExcludedLayersThatAreAnimating)
			myCachedMaxOpacity = 0.0001;
	}


	myIsCachedMaxOpacityValid = !bHaveAnimating && !pcsExludeLayer;
	return myCachedMaxOpacity;
}
/*****************************************************************************/
FLOAT_TYPE UIPlane::getMaxPausingUiOpacity()
{
	if(myIsCachedMaxPausingOpacityValid)
		return myCachedMaxPausingOpacity;

	myCachedMaxPausingOpacity = 0.0;

	FLOAT_TYPE fCurrOpacity;
	// Otherwise, we may still be in the process of animating something.
	int iCurrElement, iNumElements;
	bool bHaveAnimating = false;
	UIElement* pElement;
	SVector2D svDummy;

	int iNumPausingLayersInShownList = 0;
	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		// Check if this is a non pausing element
		if(pElement->getBoolProp(PropertyUioInGame))
			continue;

		if(pElement->getIsFullyHidden())
			continue;

		if(!bHaveAnimating && pElement->getIsAnimating())
			bHaveAnimating = true;

		if(getIsLayerShown(pElement->getStringProp(PropertyLayer)))
			iNumPausingLayersInShownList++;

		fCurrOpacity = pElement->getStateAnimProgress();
#ifdef _DEBUG
		if(fCurrOpacity > 0.9)
		{
			int bp = 0;
		}
#endif
		if(fCurrOpacity > myCachedMaxPausingOpacity)
			myCachedMaxPausingOpacity = fCurrOpacity;
	}


	// Little hack: if we have any layers in the shown department,
	// make sure we return a non-zero opacity, otherwise things
	// are never going to show.
	if(myCachedMaxPausingOpacity == 0 && iNumPausingLayersInShownList > 0) // && myCurrentlyShownLayers.size() > 0)
		myCachedMaxPausingOpacity = 0.0001;

	myIsCachedMaxPausingOpacityValid = !bHaveAnimating;
	return myCachedMaxPausingOpacity;
}
/*****************************************************************************/
FLOAT_TYPE UIPlane::getMaxNonPausingUiOpacity()
{
	if(myIsCachedMaxNonPausingOpacityValid)
		return myCachedMaxNonPausingOpacity;

	myCachedMaxNonPausingOpacity = 0.0;

	FLOAT_TYPE fCurrOpacity;
	// Otherwise, we may still be in the process of animating something.
	int iCurrElement, iNumElements;
	bool bHaveAnimating = false;
	UIElement* pElement;
	SVector2D svDummy;

	int iNumNonPausingLayersInShownList = 0;
	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		// Check if this is a pausing element
		if(pElement->getBoolProp(PropertyUioInGame) == false)
			continue;

		if(pElement->getIsFullyHidden())
			continue;

		if(!bHaveAnimating && pElement->getIsAnimating())
			bHaveAnimating = true;

		if(getIsLayerShown(pElement->getStringProp(PropertyLayer)))
			iNumNonPausingLayersInShownList++;

		fCurrOpacity = pElement->getStateAnimProgress();
		if(fCurrOpacity > myCachedMaxNonPausingOpacity)
			myCachedMaxNonPausingOpacity = fCurrOpacity;
	}

	// Little hack: if we have any layers in the shown department,
	// make sure we return a non-zero opacity, otherwise things
	// are never going to show.
	if(myCachedMaxNonPausingOpacity == 0 && iNumNonPausingLayersInShownList > 0) // && myCurrentlyShownLayers.size() > 0)
		myCachedMaxNonPausingOpacity = 0.0001;

	myIsCachedMaxNonPausingOpacityValid = !bHaveAnimating;
	return myCachedMaxNonPausingOpacity;
}
/*****************************************************************************/
FLOAT_TYPE UIPlane::getLayerProgress(const char* pcsLayerName)
{
	int iCurrElement, iNumElements;
	UIElement* pElement;

	FLOAT_TYPE fCurrProg = 0, fMaxProg = 0;

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		if(strcmp(pElement->getStringProp(PropertyLayer), pcsLayerName) != 0)
			continue;

		fCurrProg = pElement->getStateAnimProgress();
		if(fCurrProg > fMaxProg)
			fMaxProg = fCurrProg;
	}

	return fCurrProg;
}
/*****************************************************************************/
bool UIPlane::getIsShowingAnything()
{
	int iCurrElement, iNumElements;
	UIElement* pElement;

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		if(pElement->getIsBeingShown() || pElement->getIsFullyShown())
			return true;
	}

	return false;
}
/*****************************************************************************/
bool UIPlane::getIsHidingAnything()
{
	int iCurrElement, iNumElements;
	UIElement* pElement;

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		if(pElement->getIsBeingHidden())
			return true;
	}

	return false;
}
/*****************************************************************************/
bool UIPlane::getIsAnimatingAnything()
{
	if(myIsCachedShowingAnythingValid)
		return myCachedIsShowingAnything;

	myCachedIsShowingAnything = true;
	if(myCurrentlyShownLayers.size() > 0)
		return true;

	// Otherwise, we may still be in the process of animating something.
	int iCurrElement, iNumElements;
	UIElement* pElement;

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		if(pElement->getIsAnimating())
			return true;
	}

	myIsCachedShowingAnythingValid = true;
	myCachedIsShowingAnything = false;
	return false;
}
/*****************************************************************************/
int UIPlane::findShownLayer(const char* pcsName)
{
	int iCurr, iNum = myCurrentlyShownLayers.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(myCurrentlyShownLayers[iCurr] == pcsName)
			return iCurr;
	}

	return -1;
}
/*****************************************************************************/
void UIPlane::showUI(const char* pcsLayer, bool bImmediate, const char* pcsWithTargetDataSource, StringResourceItem* pOptSrcItem, PropertyType eParmPropertyName)
{
	if(!pcsLayer || strlen(pcsLayer) == 0)
		return;

	Logger::log("Show layer = %s data source = %s \n", pcsLayer, pcsWithTargetDataSource ? pcsWithTargetDataSource : "");

	myIsRenderableFlagDirty = true;
	myIsCachedShowingAnythingValid = false;
	myIsCachedMaxPausingOpacityValid = false;
	myIsCachedMaxNonPausingOpacityValid = false;

	int iCurrElement, iNumElements;
	UIElement* pElement;
	string strLayer(pcsLayer);

	myCommonStringSet.clear();

	bool bHasDemoTag, bHasNonDemoTag;
#ifdef DEMO_MODE
	bool bIsInDemo = !GameEngine::isDemoUnlocked();
#else
	bool bIsInDemo = false;
#endif

	// Add to the list first, since the show() call below
	// will refresh items, which may show next ones, which may then
	// be in the wrong order for clicks.
	if(findShownLayer(pcsLayer) < 0)
		myCurrentlyShownLayers.push_back(pcsLayer);

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

#ifdef _DEBUG
		if(IS_OBJ_OF_TYPE(pElement, "createCustomBrushDialog"))
		{
			int bp = 0;
		}
#endif
		if(pElement->getStringProp(PropertyLayer) == strLayer)
		{
			// See if this element has an exclusive group name.
			if(pElement->doesPropertyExist(PropertyExclusiveGroup))
				myCommonStringSet.insert(pElement->getStringProp(PropertyExclusiveGroup));
/*
			bHasDemoTag = pElement->hasTag("demoonly");
			bHasNonDemoTag = pElement->hasTag("notindemo");
			if( (!bHasDemoTag && !bHasNonDemoTag)
				|| (bHasDemoTag && bIsInDemo)
				|| (bHasNonDemoTag && !bIsInDemo) )
			{
				pElement->setGenericDataSource(pcsWithTargetDataSource);
				if(pOptSrcItem)
					pElement->copyParmValuesFrom(pOptSrcItem, eParmPropertyName);
				pElement->show(bImmediate);
			}
			*/

			bool bAllowShowing = true;
#ifdef OSX_BUILD
			if(pElement->hasTag("toplevelmenu"))
				bAllowShowing = false;
#endif
			if(bAllowShowing)
			{
				pElement->setGenericDataSource(pcsWithTargetDataSource);
				if(pOptSrcItem)
					pElement->copyParmValuesFrom(pOptSrcItem, eParmPropertyName);
				pElement->show(bImmediate);
			}
		}
	}

	// Hide any layers that are in the exclusive group
	if(myCommonStringSet.size() > 0)
	{
		myCommonStringSet2.clear();

		TStringSet::iterator si;
		iNumElements = myUIElements.size();
		for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
		{
			pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
			if(!pElement)
				continue;

			if(pElement->getStringProp(PropertyLayer) != strLayer &&
				pElement->doesPropertyExist(PropertyExclusiveGroup))
			{
				si = myCommonStringSet.find(pElement->getStringProp(PropertyExclusiveGroup));
				if(si != myCommonStringSet.end())
				{
					// Hide the layer it's in. We must add to set.
					myCommonStringSet2.insert(pElement->getStringProp(PropertyLayer));
				}
			}
		}

		// Now go over and hide found layers.
		for(si = myCommonStringSet2.begin(); si != myCommonStringSet2.end(); si++)
		{
			this->hideUI(si->c_str());
		}

	}


	myLastShownLayerName = strLayer;
}
/*****************************************************************************/
void UIPlane::hideAllImmediate()
{
	myIsRenderableFlagDirty = true;
	int iLayer, iNumLayers = myCurrentlyShownLayers.size();
	for(iLayer = 0; iLayer < iNumLayers; iLayer++)
	{
		hideUI(myCurrentlyShownLayers[iLayer].c_str(), true);
	}
	myLastShownLayerName = "";
}
/*****************************************************************************/
void UIPlane::hideAll(const char* pcsExceptLayer)
{
	myIsRenderableFlagDirty = true;
	int iLayer, iNumLayers = myCurrentlyShownLayers.size();
	for(iLayer = iNumLayers - 1; iLayer >= 0 ; iLayer--)
	{
		if(!pcsExceptLayer || myCurrentlyShownLayers[iLayer] != pcsExceptLayer)
			hideUI(myCurrentlyShownLayers[iLayer].c_str(), false);
	}
	myLastShownLayerName = "";
}
/*****************************************************************************/
void UIPlane::hideLayersWithTag(const char* pcsTag)
{
	if(!pcsTag || strlen(pcsTag) == 0)
		return;

	int iCurrElement, iNumElements;
	UIElement* pElement;

	myCommonStringSet.clear();
	myCommonStringSet2.clear();
	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		if(pElement->hasTag(pcsTag) && pElement->doesPropertyExist(PropertyLayer))
		{
			myCommonStringSet.insert(pElement->getStringProp(PropertyLayer));
			if(pElement->doesPropertyExist(PropertyRadioGroup))
				myCommonStringSet2.insert(pElement->getRadioGroupName());
		}
	}

	TStringSet::iterator si;
	for(si = myCommonStringSet.begin(); si != myCommonStringSet.end(); si++)
	{
		if(this->getIsLayerShown((*si).c_str()))
			this->hideUI((*si).c_str());
	}

	// Now, turn off all the radio groups we have
	for(si = myCommonStringSet2.begin(); si != myCommonStringSet2.end(); si++)
		setPushedForRadioGroup((*si).c_str(), NULL, false);
}
/*****************************************************************************/
void UIPlane::hideAllWithTagExcept(const char* pcsTag, const char* pcsExceptionId, bool bSetMenusAsUnshowableOnGlide, AnimationOverCallback* pCallBack, AnimOverActionType eAnimOverAction, bool bInstant)
{
	TUIElementVector* pExceptionsVec = NULL;
	if(pcsExceptionId)
	{
		UIElement *pElem = this->getElementById(pcsExceptionId);
		if(pElem)
		{
			myCommonUiVElems.clear();
			myCommonUiVElems.push_back(pElem);
			pExceptionsVec = &myCommonUiVElems;
		}
	}
	this->hideAllWithTag(pcsTag, pExceptionsVec, bSetMenusAsUnshowableOnGlide, pCallBack, eAnimOverAction, bInstant);
}
/*****************************************************************************/
void UIPlane::hideAllWithTag(const char* pcsTag, TUIElementVector* pExceptions, bool bSetMenusAsUnshowableOnGlide, AnimationOverCallback* pCallBack, AnimOverActionType eAnimOverAction, bool bInstant)
{
    if(!pcsTag || strlen(pcsTag) == 0)
		return;

    int iCurrElement, iNumElements;
    UIElement* pElement;

	myCommonStringSet.clear();

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		hideAllWithTagRecursive(pElement, pcsTag, pExceptions, bSetMenusAsUnshowableOnGlide, pCallBack, eAnimOverAction, bInstant, myCommonStringSet);
	}

	// Now, we must go and ensure that for all layer names collected, if we have no more shown elements,
	// remove that layer from the currently shown layers list...
	TStringSet::iterator si;
	int iShownIndex;
	for(si = myCommonStringSet.begin(); si != myCommonStringSet.end(); si++)
	{
		if(*si == PROPERTY_NONE)
			continue;

		if(getAreAnyElementsInLayerShown(si->c_str()))
			continue;

		// Otherwise, erase all instances from our layers list:
		for(iShownIndex = findShownLayer(si->c_str()); iShownIndex >= 0; iShownIndex = findShownLayer(si->c_str()))
			myCurrentlyShownLayers.erase(myCurrentlyShownLayers.begin() + iShownIndex);
	}

	if(myCurrentlyShownLayers.size() > 0)
		myLastShownLayerName = myCurrentlyShownLayers[myCurrentlyShownLayers.size() - 1];
	else
		myLastShownLayerName = "";

	myIsRenderableFlagDirty = true;
	myIsCachedMaxPausingOpacityValid = false;
	myIsCachedMaxNonPausingOpacityValid = false;
}
/*****************************************************************************/
void UIPlane::hideAllWithTagRecursive(UIElement* pElement, const char* pcsTag, TUIElementVector* pExceptions, bool bSetMenusAsUnshowableOnGlide, AnimationOverCallback* pCallBack, AnimOverActionType eAnimOverAction, bool bInstant, TStringSet& setHiddenLayers)
{
	if(pElement->hasTag(pcsTag) &&
		(!pExceptions || find(pExceptions->begin(), pExceptions->end(), pElement) == pExceptions->end())	)
	{
		UIMenuElement* pMenuElem = dynamic_cast<UIMenuElement*>(pElement);
		if(pMenuElem && bSetMenusAsUnshowableOnGlide)
			pMenuElem->getTopLevelMenuParent()->setDoShowChildrenOnGlide(false);

		pElement->hide(bInstant, pCallBack, eAnimOverAction);

		const char* pcsLayerName = pElement->getStringProp(PropertyLayer);
		if(IS_VALID_STRING_AND_NOT_NONE(pcsLayerName) && setHiddenLayers.find(pcsLayerName) == setHiddenLayers.end())
		{
			if(getIsLayerShown(pcsLayerName))
				setHiddenLayers.insert(pcsLayerName);
		}
	}

	UIElement* pChild;
	int iCurrChild, iNumChildren = pElement->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(pElement->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		hideAllWithTagRecursive(pChild, pcsTag, pExceptions, bSetMenusAsUnshowableOnGlide, pCallBack, eAnimOverAction, bInstant, setHiddenLayers);
	}
}
/*****************************************************************************/
void UIPlane::hideUI(const char* pcsLayer, bool bImmediate, FLOAT_TYPE fExtraOffset)
{
	if(!pcsLayer || strlen(pcsLayer) == 0)
		return;

	Logger::log("Hide layer = %s \n", pcsLayer);

	if(!getIsLayerShown(pcsLayer))
	{
		Logger::log(STR_LIT("Layer %s not shown, not hiding.\n"), pcsLayer);
		return;
	}

	myIsRenderableFlagDirty = true;
	myIsCachedMaxPausingOpacityValid = false;
	myIsCachedMaxNonPausingOpacityValid = false;

	int iCurrElement, iNumElements;
	UIElement* pElement;
	string strLayer(pcsLayer);

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		if(strLayer == pElement->getStringProp(PropertyLayer))
			pElement->hide(bImmediate, NULL, AnimOverActionNone, fExtraOffset);
	}

	int iShownIndex = findShownLayer(pcsLayer);
	if(iShownIndex >= 0)
	{
		myCurrentlyShownLayers.erase(myCurrentlyShownLayers.begin() + iShownIndex);
		if(myCurrentlyShownLayers.size() > 0)
			myLastShownLayerName = myCurrentlyShownLayers[myCurrentlyShownLayers.size() - 1];
		else
			myLastShownLayerName = "";
	}
}
/*****************************************************************************/
void UIPlane::onButtonClicked(UIElement *pElem, MouseButtonType eButton, bool bSetSelfAsLastResourceDataSource)
{
	// Use the ui element object ID to determine action.
#ifdef ENABLE_SOUNDS_NEW
	// See if the button has a custom sound; if yes, play that,
	// otherwise, play the standard click sound.
	if(pElem->doesPropertyExist(PropertyWiFiringSound))
		playSound(pElem->getStringProp(PropertyWiFiringSound));
	else
		playSound(CLICK_SOUND_TYPE);
#endif

	//MouseButtonType eButton = getParentWindow()->getCurrMouseButtonDown();

	// First, though, see if it's an enum and if so,
	// increment it
	// Done in button now!
//	if(pElem->getIsEnum())
//S		pElem->incrementEnumValue();

	PropertyType eActionProp = PropertyAction;
	PropertyType eActionModKeyActionProp = PropertyActionModKeyAction;
	PropertyType eActionValue1Prop = PropertyActionValue;
	PropertyType eActionValue2Prop = PropertyActionValue2;
	PropertyType eActionValue3Prop = PropertyActionValue3;

	if(eButton == MouseButtonRight)
	{
		eActionProp = PropertyRmbAction;
		eActionValue1Prop = PropertyRmbActionValue;
		eActionValue2Prop = PropertyRmbActionValue2;
		eActionValue3Prop = PropertyRmbActionValue3;
	}

	// Look at the ui elem object action

	string strTempDataSourceStorage;
	const char *pcsTargetDataSource = NULL;
	if(pElem->doesPropertyExist(PropertyTargetDataSource))
	{
		pElem->getAsString(PropertyTargetDataSource, strTempDataSourceStorage);
		if(strTempDataSourceStorage.length() > 0)
			pcsTargetDataSource = strTempDataSourceStorage.c_str();
		//pcsTargetDataSource = pElem->getStringProp(PropertyUiObjTargetDataSource);
	}
	if(!pcsTargetDataSource && pElem->getHaveGenericDataSource())
		pcsTargetDataSource = pElem->getGenericDataSource();

	if(!pcsTargetDataSource && bSetSelfAsLastResourceDataSource)
		pcsTargetDataSource = pElem->getStringProp(PropertyId);

	string strLogValue;
	if(pElem->doesPropertyExist(eActionValue1Prop))
		pElem->getAsString(eActionValue1Prop, strLogValue);
	string strLogValue2;
	if(pElem->doesPropertyExist(eActionValue2Prop))
		pElem->getAsString(eActionValue2Prop, strLogValue2);
	string strLogValue3;
	if(pElem->doesPropertyExist(eActionValue3Prop))
		pElem->getAsString(eActionValue3Prop, strLogValue3);
	const char* pcsElemId = PROPERTY_NONE;
	if(pElem->doesPropertyExist(PropertyId))
		pcsElemId = pElem->getStringProp(PropertyId);
	string strLogDataSource;
	if(pcsTargetDataSource)
		strLogDataSource = pcsTargetDataSource;

	string strAction;
	if(pElem->doesPropertyExist(eActionProp))
		strAction = pElem->getStringProp(eActionProp);

	// See if we have any required modifier keys
	bool bAllowAction = true;
	if(bAllowAction && eButton == MouseButtonLeft && pElem->doesPropertyExist(PropertyActionModifierKeys))
	{
		const string strDelims(" +");

		pElem->getAsStringVector(PropertyActionModifierKeys, myCommonStringVector2);
		// See if we recognize them all as pressed:
		SilentKeyType eModKey;
		int iSubKeyIndex, iNumSubkeys;
		int iCurrKeyCombo, iNumKeyCombos = myCommonStringVector2.size();
		bool bHaveAllKeysPressed;
		for(iCurrKeyCombo = 0; iCurrKeyCombo < iNumKeyCombos; iCurrKeyCombo++)
		{
			// We may have several modifiers separated by + and spaces. Parse them.
			TokenizeUtils::tokenizeStringToCharPtrsInPlace(myCommonStringVector2[iCurrKeyCombo], strDelims, myCommonCharPtrVector);

			iNumSubkeys = myCommonCharPtrVector.size();
			bHaveAllKeysPressed = true;
			for(iSubKeyIndex = 0; iSubKeyIndex < iNumSubkeys; iSubKeyIndex++)
			{
				eModKey = KeyManager::getInstance()->mapModifierKeyStringToType(myCommonCharPtrVector[iSubKeyIndex]);
				if(!KeyManager::getInstance()->getIsModifierKeyDown(eModKey))
				{
					bHaveAllKeysPressed = false;
					break;
				}
			}

			if(bHaveAllKeysPressed && iNumSubkeys > 0)
			{
				// Get the action string from the enum props
				strAction = pElem->getEnumPropValue(eActionModKeyActionProp, iCurrKeyCombo);
				break;
			}
		}

		// We allow the action because presumably
		// we have the actual uiobjAction on it.
		// Or even if not, that string is going to be empty, which is ok.
		bAllowAction = true;
		/*
		if(!bHaveAllKeysPressed)
			bAllowAction = false;

		if(bAllowAction)
		{
			if(pElem->doesPropertyExist(eActionModKeyActionProp))
			{
				// Remap our action
				eActionValue1Prop = eActionModKeyActionProp;
				strAction = pElem->getStringProp(eActionModKeyActionProp);
			}
			// Need to add PropertyUiObjActionModKeyAction for mode keys...
			ELSE_ASSERT;
		}
		// We allow the action because presumably
		// we have the actual uiobjAction on it.
		// Or even if not, that string is going to be empty, which is ok.
		else
			bAllowAction = true;
			*/
	}

	if(getParentWindow()->getHaveActiveTouches())
		bAllowAction = false;

	if(strAction.length() == 0 || strAction == PROPERTY_NONE)
		bAllowAction = false;

	Logger::log("Button pressed id = %s action = %s actionValue = %s actionValue2 = %s actionValue3 = %s dataSource = %s\n", pcsElemId, strAction.c_str(), strLogValue.c_str(), strLogValue2.c_str(), strLogValue3.c_str(), strLogDataSource.c_str());

	if(bAllowAction)
	{
		// ...But first, see if one of our parents can handle this action. Or us, for that matter.
		if(pElem->handleActionUpParentChain(strAction, eButton == MouseButtonRight))
			return;
/*
		UIElement* pHandleElem = pElem;
		while(pHandleElem)
		{
			bool bResult = false;
			if(eButton == MouseButtonRight)
				bResult = pHandleElem->handleRMBAction(strAction, pElem);
			else
				bResult = pHandleElem->handleAction(strAction, pElem);
			if(bResult)
				return;
			pHandleElem = pHandleElem->getParent<UIElement>();
		}
*/

		processAction(pElem, strAction, pcsTargetDataSource, eActionValue1Prop, eActionValue2Prop, eActionValue3Prop, eButton);
	}
}
/*****************************************************************************/
void UIPlane::deleteElement(UIElement* pElem)
{
	if(!pElem)
		return;

	// Note that we have to recursively delete all child elements of us.
	int iCurrChild, iNumChildren = pElem->getNumChildren();
	UIElement* pChild;
	for(iCurrChild = iNumChildren - 1; iCurrChild >= 0; iCurrChild--)
	{
#ifdef _DEBUG
		ResourceItem* pChildDB = pElem->getChild(iCurrChild);
#endif
        pChild = FAST_CAST<UIElement*>(pElem->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		this->deleteElement(pChild);
	}

	// First, remove from global objects...
	int iCurr, iNum = myUIElements.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(myUIElements[iCurr] == pElem)
		{
			//delete myUIElements[iCurr];
			myUIElements.erase(myUIElements.begin() + iCurr);
			break;
		}
	}

	// Then, remove from stages
	int iStage;
	int iNumRenderStages = myElementsInStages.getCurrentCapacity();
	for(iStage = 0; iStage < iNumRenderStages; iStage++)
	{
		if(!myElementsInStages[iStage])
			continue;

	    iNum = myElementsInStages[iStage]->size();
	    for(iCurr = 0; iCurr < iNum; iCurr++)
	    {
			if((*myElementsInStages[iStage])[iCurr] == pElem)
			{
				myElementsInStages[iStage]->erase(myElementsInStages[iStage]->begin() + iCurr);
				iStage = iNumRenderStages + 1;
				break;
			}
	    }
	}

	// Then, remove from the map
	myElementsMap.remove(pElem->getStringProp(PropertyId));

	pElem->clearTargetUpdateIdentifier();

	// Then, remove us from the parent.
	if(pElem->getParent())
	{
		as<UIElement>(pElem->getParent())->invalidateSizingInfos();
	    // Note that this also performs the actual deallocation
	    // of the memory.
	    pElem->getParent()->deleteChild(pElem);
	}
	else
	{
	    // If we're not a child, we have to delete the element explicitly.
	    delete pElem;
	}
}
/*****************************************************************************/
UIElement* UIPlane::addNewElementFromText(const char* pcsElemSpec, UIElement* pOptParent)
{
	ResourceCollection rDummyColl;

	if(!pcsElemSpec)
		ASSERT_RETURN_NULL;

	// Note that we may have defaults for this thing. Find its id.
	const char* pcsIdPtr = strstr(pcsElemSpec, "<");
	if(!pcsIdPtr)
		ASSERT_RETURN_NULL;
	pcsIdPtr++;
	const char* pcsIdPtr2 = strstr(pcsIdPtr, ">");
	if(!pcsIdPtr2)
		ASSERT_RETURN_NULL;
	string strTag(pcsIdPtr);
	strTag = strTag.substr(0, pcsIdPtr2  - pcsIdPtr);

	ResourceItem* pDefaultsItem = getDefaultsForTag(strTag.c_str());

	// Now, save it to file as default
	string strTemp;
	if(pDefaultsItem)
		pDefaultsItem->saveToString(strTemp, FormatNative, NULL, true);

	strTemp += pcsElemSpec;
	if(!rDummyColl.loadFromString(strTemp))
		ASSERT_RETURN_NULL;

	ResourceItem* pTemplate = rDummyColl.getItem(0);
	if(!pTemplate)
		ASSERT_RETURN_NULL;

	UIElement* pRes = rDummyColl.createItemOfType<UIElement>(pTemplate->getId(), this, NULL, this->getAllocator());
	pRes->setHasNoDefinition(true);
	finishAddingCreatedElementInternal(pRes, pOptParent, -1, NULL);
	return pRes;
}
/*****************************************************************************/
UIElement* UIPlane::addNewElementFromTemplate(ResourceItem* pTemplate, UIElement* pOptParent)
{
	// Dummy collection
	ResourceCollection rDummyColl;
	ResourceItem* pItem = rDummyColl.addItem(pTemplate->getId(), pTemplate->getTag());
	pItem->copyResourcesFrom(*pTemplate, true);

    UIElement* pRes = rDummyColl.createItemOfType<UIElement>(pTemplate->getId(), this, NULL, this->getAllocator());
	finishAddingCreatedElementInternal(pRes, pOptParent, -1, NULL);
	return pRes;

	// Collection will take care of deleting the copy
}
/*****************************************************************************/
UIElement* UIPlane::addNewElement(const char* pcsType, UIElement* pOptParent, int iOptPosition, const char* pcsOptId)
{
    ResourceCollection* pUIElemsColl = ResourceManager::getInstance()->getCollection(myInitCollectionType);
	//ResourceCollection* pUIElemsColl3 = ResourceManager::getInstance()->getCollection(ResourceSepWindowUIElements);

    ResourceCollection* pResColl = NULL;
    if(pUIElemsColl->getItemById(pcsType))
		pResColl = pUIElemsColl;
//     else if(pUIElemsColl2->findItemByType(pcsType))
// 		pResColl = pUIElemsColl2;
// 	else if(pUIElemsColl3->getItemById(pcsType))
// 		pResColl = pUIElemsColl3;
    else
    {
		// Creating an unknown type
		_ASSERT(0);
		return NULL;
    }

    UIElement* pRes = pResColl->createItemOfType<UIElement>(pcsType, this, NULL, this->getAllocator());
	finishAddingCreatedElementInternal(pRes, pOptParent, iOptPosition, pcsOptId);
	return pRes;
}
/*****************************************************************************/
void UIPlane::finishAddingCreatedElementInternal(UIElement* pNewElem, UIElement* pOptParent, int iOptPosition, const char* pcsOptId)
{
	// Make sure we set a unique name BEFORE WE DO ANYTHING
	//     string strUniqueName;
	//     this->generateUniqueName(pNewElem, strUniqueName);
	// 	pNewElem->setName(strUniqueName.c_str());

	// Because
	//pNewElem->setUniqueName(this, false);
	//myTempGeneratedUniqueNames.clear();
	if(pcsOptId)
	{
		// Save the old id
		if(!pNewElem->doesPropertyExist(PropertyOldId))
			pNewElem->setStringProp(PropertyOldId, pNewElem->getStringProp(PropertyId));
		pNewElem->setId(pcsOptId);
		pNewElem->onIdChanged();

		// Just to make sure the name is truly unique
		pNewElem->setUniqueName(this, true);
	}
	else
		pNewElem->setUniqueName(this, true);

    // Parent first since the name generation depends on this.
    if(pOptParent)
		pOptParent->parentExistingNode(pNewElem, iOptPosition);

    // Reset the init state in case we changed from a parentless elem to
    // a parented one.
    pNewElem->resetInitialStateAnim();

    if(pOptParent)
    {
		// We only define layers on children, since top-level elements should have it set already.
		// We need to fake-set level to something >0 since it's used to test whether to set
		// the elements own property or not. We want our own layer set, since we're guaranteed
		// to be a child, hence this.
		pNewElem->defineLayersOnChildren(pOptParent->getStringProp(PropertyLayer), 1);
    }
    else
    {
		// Only keep track of the top-level ui elements.
		int iRenderStage = pNewElem->getNumProp(PropertyZIndex);
		this->ensureElementStageExists(iRenderStage);
		myElementsInStages[iRenderStage]->push_back(pNewElem);
		pNewElem->resetSliderChildrenVisibility();

		myUIElements.push_back(pNewElem);
    }

    pNewElem->insertIntoMap(myElementsMap, NULL);
	pNewElem->updateTitleBarVars();

	if(pOptParent)
	{
		pOptParent->invalidateSizingInfos();
		pOptParent->autoLayoutChildren();
	}

    pNewElem->adjustToScreen();
}
/*****************************************************************************/
void UIPlane::initFromCollection(ResourceType eCollectionType)
{
	Application::lockGlobal();

	// Copy all elements from the collection. We do this somewhere already.
	ResourceCollection* pUIElemsColl = ResourceManager::getInstance()->getCollection(eCollectionType);
	if(!pUIElemsColl)
	{
		Application::unlockGlobal();
		return;
	}

	myInitCollectionType = eCollectionType;

	//ResourceItem* pCurrItem = pUIElemsColl->traverseBegin();
	ResourceItem* pCurrItem = NULL;
	ResourceCollection::Iterator ci;
#ifdef DEMO_MODE
	bool bIsInDemo = !GameEngine::isDemoUnlocked();
#else
	bool bIsInDemo = false;
#endif

	string strTemp;
//	UiElemType eElemType;
	UIElement* pRes;
	int iRenderStage;
	Application::unlockGlobal();
	//while(pCurrItem)
	TUIElementVector vecNewElems;
	for(ci = pUIElemsColl->itemsBegin(); !ci.isEnd(); ci++)
	{
		Application::lockGlobal();
		pCurrItem = ci.getItem();
		// But first, check if this item is for demo only
		if(!bIsInDemo && pCurrItem->hasTag(DEMO_TAG))
		{ }
		else
		{
			strTemp = pCurrItem->getStringProp(PropertyElemType);

			// Allocate user our uber-cool allocator now. This way we get the
			// children, too.
#ifdef _DEBUG
			if(strcmp(pCurrItem->getStringProp(PropertyId), "mainWndTest") == 0)
			{
			    int bp = 0;
			}
#endif
			pRes = pUIElemsColl->createItemOfType<UIElement>(pCurrItem->getStringProp(PropertyId), this, NULL, this->getAllocator());
			pRes->defineLayersOnChildren(pRes->getStringProp(PropertyLayer));

			iRenderStage = pRes->getNumProp(PropertyZIndex);
			this->ensureElementStageExists(iRenderStage);
			myElementsInStages[iRenderStage]->push_back(pRes);
			pRes->insertIntoMap(myElementsMap, &myUniqueCounts);

			pRes->resetSliderChildrenVisibility();


			myUIElements.push_back(pRes);
			vecNewElems.push_back(pRes);
		}

		//pCurrItem = pUIElemsColl->traverseNext();
		Application::unlockGlobal();
	}

	Application::lockGlobal();
	// We need to do screen adjustment separately, since tables may create cells in it,
	// but if we run that code before the actual source elem gets processed, we'll get name
	// conflicts, since table cell will check for duplicates, but won't find any, and the
	// the source elem won't check for elems and will create crap.
	int iCurrElem, iNumElems = vecNewElems.size();
	for(iCurrElem = 0; iCurrElem < iNumElems; iCurrElem++)
		vecNewElems[iCurrElem]->adjustToScreen();
	Application::unlockGlobal();
}
/*****************************************************************************/
bool UIPlane::getIsLayerShown(const char* pcsLayer)
{
	int iCurr, iNum = myCurrentlyShownLayers.size();
	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
	{
		if(myCurrentlyShownLayers[iCurr] == pcsLayer)
			return true;
	}

	return false;
}
/*****************************************************************************/
void UIPlane::setPushedForRadioGroup(const char* pcsGroup, UIButtonElement* pException, bool bInstant)
{
	int iCurrElement, iNumElements;
	UIElement* pElement;
	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		if(pElement == (UIElement*)pException)
			continue;

		// Call this recursively
		pElement->setPushedForRadioGroupRecursive(pcsGroup, pException, bInstant);
/*
		if(pElement->doesPropertyExist(PropertyUiObjRadioGroup) &&
			strcmp(pElement->getRadioGroupName(), pcsGroup) == 0)
		{
			// Got it!
			pElement->setPushed(false);

			// Special case - if the button's action was to show a layer, we hide it.
			if(strcmp(pElement->getStringProp(PropertyUiObjAction), UIA_SHOW_LAYER) == 0)
			{
				const char* pcsButtonLayer = pElement->getStringProp(PropertyUiObjActionValue);
				if(this->getIsLayerShown(pcsButtonLayer))
					this->hideUI(pcsButtonLayer);
			}
		}


*/
	}


}
/*****************************************************************************/
UIAllocator* UIPlane::getAllocator()
{
	return Application::getInstance()->getUIAllocator();
}
/*****************************************************************************/
UIElement* UIPlane::getFinalChildAtCoords(FLOAT_TYPE fX, FLOAT_TYPE fY, UIElement* pStartElem, bool bFirstNonPassthrough, bool bGlidingOnly)
{
	if(!pStartElem)
		return NULL;

	UIElement* pChild = pStartElem->getChildAtRecursive(fX, fY, bFirstNonPassthrough, bGlidingOnly);
	if(pChild)
		return pChild;
	else
		return pStartElem;
}
/*****************************************************************************/
void UIPlane::render()
{
	SVector2D svScroll;
	getParentWindow()->getSize(svScroll);
	svScroll *= 0.5;

	// We need to go and recompute which elements are visible, and which
	// aren't if one of them obscures others.
	this->recomputeRenderableFlag();

	getDrawingCache()->flush();

	FLOAT_TYPE fOpacity = 1.0;
	FLOAT_TYPE fScale = 1.0;

	// We need to render the objects, but do so based on the render order.
	int iCurrStage;
	int iElem, iNumElems;
	int iNumRenderedStageElems;

	UIElement* pCurrElem;
	bool bIsShown;
	int iNumRenderStages = myElementsInStages.getCurrentCapacity();
	for(iCurrStage = 0; iCurrStage < iNumRenderStages; iCurrStage++)
	{
		if(!myElementsInStages[iCurrStage])
			continue;

		myCurrRenderingPass = RenderingNormal;

		iNumRenderedStageElems = 0;
		iNumElems = myElementsInStages[iCurrStage]->size();
		for(iElem = 0; iElem < iNumElems; iElem++)
		{
			pCurrElem = (*myElementsInStages[iCurrStage])[iElem];
#ifdef _DEBUG
			if(strcmp(pCurrElem->getStringProp(PropertyId), "mainLayerRowSelBack") == 0)
			{
				int bp = 0;
			}
#endif
			bIsShown = pCurrElem->getIsAnimating() || std::find(myCurrentlyShownLayers.begin(), myCurrentlyShownLayers.end(), pCurrElem->getStringProp(PropertyLayer)) != myCurrentlyShownLayers.end();

			// Check if it is visible at all - i.e. its state is right.

			if(bIsShown && pCurrElem->allowRendering())
			{
				pCurrElem->render(svScroll, fOpacity, fScale);
				iNumRenderedStageElems++;
			}
		}
		if(iNumRenderedStageElems > 0)
			getDrawingCache()->flush();

		// Now, se if we have any postponed infos:
		myCurrRenderingPass = RenderingPostopnedElements;
		if(myPostponedElemInfos.size() > 0)
		{
			renderPostponedElems();
			myPostponedElemInfos.clear();
		}
		myCurrRenderingPass = RenderingNormal;
	}
}
/*****************************************************************************/
void UIPlane::renderPostponedElems()
{
	SPostponedElemInfo* pInfo;
	int iCurr, iNum = myPostponedElemInfos.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pInfo = &myPostponedElemInfos[iCurr];
		pInfo->myElem->render(pInfo->myScroll, pInfo->myOpacity, pInfo->myScale);
	}
}
/*****************************************************************************/
void UIPlane::slideParentElement(UIElement* pElem, int iDir)
{
	// Get the parent element of this guy, and tell him to slide.
	ResourceItem* pParent = pElem;
	while(pParent->getParent())
			pParent = pParent->getParent();

	// See if we're a cell. If so, get the position of a cell
	// we're in.
	ResourceItem* pUIParent = pElem->getParent();
	UITableCellElement* pCell;
	int iTableRow = -1;
	while(pUIParent)
	{
		pCell = dynamic_cast<UITableCellElement*>(pUIParent);
		if(pCell)
		{
			iTableRow = pCell->getRow();
			break;
		}
		pUIParent = pUIParent->getParent();
	}

	// Now, tell it to slide. It'll do its own children.
	UIElement* pElemParent = FAST_CAST<UIElement*>(pParent);
	if(pElemParent)
	{
		if(pElemParent->getHasChildSliders())
		{
			// We need to find the actual element to slide.
			if(pElem->doesPropertyExist(PropertyActionValue))
				pElemParent = this->getElementById<UIElement>(pElem->getStringProp(PropertyActionValue), true);
		}

		if(pElemParent && pElemParent->allowSliding(iDir))
			pElemParent->slide(iDir, iTableRow, pElemParent->getBoolProp(PropertyIsSliderVertical));
	}
}
/*****************************************************************************/
void UIPlane::resetToInitState(const char* pcsExceptLayer)
{
	int iCurrStage;
	int iElem, iNumElems;
	UIElement *pElem;
	int iNumRenderStages = myElementsInStages.getCurrentCapacity();
	for(iCurrStage = 0; iCurrStage < iNumRenderStages; iCurrStage++)
	{
		if(!myElementsInStages[iCurrStage])
			continue;

		iNumElems = myElementsInStages[iCurrStage]->size();
		for(iElem = 0; iElem < iNumElems; iElem++)
		{
			pElem = (*myElementsInStages[iCurrStage])[iElem];

			// Skip any layer we may want to skip
			if(pcsExceptLayer && pElem->doesPropertyExist(PropertyLayer) &&
				strcmp(pElem->getStringProp(PropertyLayer), pcsExceptLayer) == 0)
				continue;

			// Check if it is visible at all - i.e. its state is right.
			if(pElem->getIsVisible())
				pElem->resetToInitState();
		}
	}
}
/*****************************************************************************/
void UIPlane::resetAllCachesForAllElements()
{
	int iCurrStage;
	int iElem, iNumElems;
	UIElement *pElem;
	int iNumRenderStages = myElementsInStages.getCurrentCapacity();
	for(iCurrStage = 0; iCurrStage < iNumRenderStages; iCurrStage++)
	{
		if(!myElementsInStages[iCurrStage])
			continue;

		iNumElems = myElementsInStages[iCurrStage]->size();
		for(iElem = 0; iElem < iNumElems; iElem++)
		{
			pElem = (*myElementsInStages[iCurrStage])[iElem];
			pElem->resetAllCaches(true);
		}
	}

	for(iCurrStage = 0; iCurrStage < iNumRenderStages; iCurrStage++)
	{
		if(!myElementsInStages[iCurrStage])
			continue;

		iNumElems = myElementsInStages[iCurrStage]->size();
		for(iElem = 0; iElem < iNumElems; iElem++)
		{
			pElem = (*myElementsInStages[iCurrStage])[iElem];
			pElem->resetUiTablesRecursive();
		}
	}
}
/*****************************************************************************/
void UIPlane::recomputeRenderableFlag()
{
	// We still have a problem. We also need to reset the flag when a layer
	// has done showing or hiding, since otherwise this will have no effect.
	if(!myIsRenderableFlagDirty)
		return;

	// Go through all our shown layers from the end, see if there's a blocker
	// layer which is fully shown.
	UIElement* pElem;
	int iObj, iNumObj = myUIElements.size();
	int iCurr, iNum = myCurrentlyShownLayers.size();

	// Set all elements to be renderable
	for(iObj = 0; iObj < iNumObj; iObj++)
	{
		pElem = FAST_CAST<UIElement*>(myUIElements[iObj]);
		pElem->setIsRenderable(true);
	}

	myIsFullyObscuring = false;
	bool bResetDirtyFlag = true;
	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
	{
		// Darn. We need to get all elements of the given layer, at least the
		// top ones.
		for(iObj = 0; iObj < iNumObj; iObj++)
		{
			// See if it has a blocker property on it
			pElem = myUIElements[iObj];
			if(pElem->getBoolProp(PropertyUioObscuresAllBelow) &&
				pElem->getStringProp(PropertyLayer) == myCurrentlyShownLayers[iCurr] )
			{
				if(pElem->getIsFullyShown())
				{
					// We need to set every element of all layers below this one
					// to be non-renderable.
					iCurr--;
					for(; iCurr >= 0; iCurr--)
					{
						for(iObj = 0; iObj < iNumObj; iObj++)
						{
							pElem = FAST_CAST<UIElement*>(myUIElements[iObj]);
							if(pElem->getStringProp(PropertyLayer) == myCurrentlyShownLayers[iCurr])
								pElem->setIsRenderable(false);
						}
					}
					iCurr = -1;
					myIsFullyObscuring = true;
					break;
				}
				else if(pElem->getIsFullyHidden() == false)
				{
					// The element is neither hidden nor shown,
					// meaning it is being animated. If we find this,
					// we don't set the dirty flag to false, since
					// we only want it as false when the animation is
					// finished.
					bResetDirtyFlag = false;
				}
			}
		}
	}

	if(bResetDirtyFlag)
		myIsRenderableFlagDirty = false;
}
/*****************************************************************************/
void UIPlane::startItemAnim(SVector2D& svInitPos, const char* pcsAnimString, const char *pcsElemId)
{
	// Get the gear element
	UIElement* pGearButton = this->getElementById<UIElement>(pcsElemId);
	_ASSERT(pGearButton);
	if(!pGearButton)
		return;

	SVector2D svTargetPos;
	pGearButton->getGlobalPosition(svTargetPos);

	UIElement* pSpecialElem = this->getElementById<UIElement>("_itemBought");
	if(!pSpecialElem)
		return;

	// Now, set all the right props on the ui element. Don't forget about
	// caches, etc.
	pSpecialElem->setTopAnim(pcsAnimString);

	pSpecialElem->setNumProp(PropertyOffPosX, svInitPos.x);
	pSpecialElem->setNumProp(PropertyOffPosY, svInitPos.y);
	pSpecialElem->setNumProp(PropertyOffOpacity, 1.0);

	pSpecialElem->setNumProp(PropertyX, svTargetPos.x);
	pSpecialElem->setNumProp(PropertyY, svTargetPos.y);
	pSpecialElem->setNumProp(PropertyOpacity, 0.15);

	// Resets the cache
	pSpecialElem->setIsEnabled(true);

	// Note: this does not go on the stack of layers shown.
	pSpecialElem->show(false, this, AnimOverActionFadeOutComplete, false);
}
/*****************************************************************************/
void UIPlane::animationOver(AnimatedValue *pAValue, string* pData)
{
	if(pAValue->getAnimOverAction() == AnimOverActionFadeOutComplete)
	{
		// The icon anim is over. Hide it.
		UIElement* pSpecialElem = this->getElementById<UIElement>("_itemBought");
		pSpecialElem->hide(true);
	}
	else
	{
		// Attempt to find out
		if(pData)
		{
			UIDropdown* pDropdown = this->getElementById<UIDropdown>(pData->c_str());
			if(pDropdown)
				pDropdown->onDropdownHidden();
		}
	}
}
/*****************************************************************************/
#ifdef LOG_THY_UI
void UIPlane::printTouches(const char* pcsText, TTouchVector* pTouches, UIElement* pElem)
{
	char pcsBuff[128];
	string strTemp(pcsText);

	if(pElem)
	{
		sprintf(pcsBuff, " E[%x %s]", pElem, pElem->getStringProp(PropertyId));
		strTemp += pcsBuff;
	}

	if(myLastMouseElement)
	{
		sprintf(pcsBuff, " L[%x %s]", myLastMouseElement, myLastMouseElement->getStringProp(PropertyId));
		strTemp += pcsBuff;
	}

	int iCurr, iNum = 0;
	if(pTouches)
		iNum = pTouches->size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		sprintf(pcsBuff, " (%.0f, %.0f)", (*pTouches)[iCurr].myPoint.x, (*pTouches)[iCurr].myPoint.y);
		strTemp += pcsBuff;
	}

	string strExisting;
	if(FileUtils::doesFileExist(UI_LOG_FILE, true))
		FileUtils::loadFromFile(UI_LOG_FILE, strExisting, FileSourceFullPath, false);

	if(strExisting.length() > 0)
		strExisting += "\n";
	strExisting += strTemp;
	FileUtils::saveToFile(UI_LOG_FILE, strExisting.c_str());
}
#endif
/*****************************************************************************/
void UIPlane::getAllShownElements(TUIElementVector& rVectorOut)
{
	int iCurrElement, iNumElements;
	UIElement* pElement;
	int iCurrChild, iNumChildren;

	rVectorOut.clear();

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		if(findShownLayer(pElement->getStringProp(PropertyLayer)) < 0)
			continue;

		rVectorOut.push_back(pElement);

		// Get its children, too.
		iNumChildren = pElement->getNumChildren();
		for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
			getElemChildrenRec(pElement->getChild(iCurrChild), rVectorOut);
	}
}
/*****************************************************************************/
void UIPlane::getElemChildrenRec(ResourceItem* pElem, TUIElementVector& rVectorOut)
{
	int iCurrChild, iNumChildren;

	rVectorOut.push_back(FAST_CAST<UIElement*>(pElem));

	iNumChildren = pElem->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		getElemChildrenRec(pElem->getChild(iCurrChild), rVectorOut);
}
/*****************************************************************************/
void UIPlane::onTimerTickBegin(GTIME lGlobalTime)
{
	// NOTE that we deliberately do not call the base GamePlane::onTimerTick*(),
	// because it does things totally unrelated to what we want now.
	// Other than calling timer events on *all* elements, even the invisible ones...


	// Refresh the ones need for refresh
	myRefreshQueueLock.lock();
	UIElement* pFocusElem;
	Window* pWindow = getParentWindow();
	TStringSet::iterator si;
	for(si = myFocusQueue.begin(); si != myFocusQueue.end(); si++)
	{
		pFocusElem = getElementById<UIElement>((*si).c_str());
		pWindow->setFocusElement(pFocusElem, false);
	}
	myFocusQueue.clear();
	myRefreshQueueLock.unlock();


// 	if(myIsCachedMaxPausingOpacityValid && myCachedMaxPausingOpacity > 0)
// 		myIsCachedMaxPausingOpacityValid = false;
//
// 	if(myIsCachedMaxNonPausingOpacityValid && myCachedMaxNonPausingOpacity > 0)
// 		myIsCachedMaxNonPausingOpacityValid = false;

	//SVector2D svScroll(0,0);

	// We need to go and recompute which elements are visible, and which
	// aren't if one of them obscures others.
	this->recomputeRenderableFlag();

	int iCurrStage;
	int iElem, iNumElems;
	bool bIsShown;
	UIElement* pCurrElem;
	int iNumRenderStages = myElementsInStages.getCurrentCapacity();
	for(iCurrStage = 0; iCurrStage < iNumRenderStages; iCurrStage++)
	{
		if(!myElementsInStages[iCurrStage])
			continue;

		iNumElems = myElementsInStages[iCurrStage]->size();
		for(iElem = 0; iElem < iNumElems; iElem++)
		{
			pCurrElem = (*myElementsInStages[iCurrStage])[iElem];

			// Check if it is visible at all - i.e. its state is right.
			bIsShown = pCurrElem->getIsAnimating() || std::find(myCurrentlyShownLayers.begin(), myCurrentlyShownLayers.end(), pCurrElem->getStringProp(PropertyLayer)) != myCurrentlyShownLayers.end();
#ifdef _DEBUG
			if(bIsShown && IS_OBJ_OF_TYPE(pCurrElem, "fileSubmenu"))
			{
				if(pCurrElem->getStateAnimProgress() < 0.12 && pCurrElem->getStateAnimProgress() > 0)
				{
					int bp = 0;
				}

			}
#endif
			if(bIsShown && pCurrElem->allowRendering())
				pCurrElem->onTimerTick(lGlobalTime);
			else if(pCurrElem->getIsHidingInProgress())
			{
				// We were hiding it, but haven't called the finished hiding callback
				// yet. Do so now.
				pCurrElem->onFinishHidingRecursive();
			}
		}
	}
}
/*****************************************************************************/
void UIPlane::onTimerTickEnd(GTIME lGlobalTime)
{
	// NOTE that we deliberately do not call the base GamePlane::onTimerTick*(),
	// because it does things totally unrelated to what we want now.
	// Other than calling timer events on *all* elements, even the invisible ones...

}
/*****************************************************************************/
void UIPlane::onWindowSizeChanged(int iNewW, int iNewH)
{
	int iCurrStage;
	int iElem, iNumElems;
	int iNumRenderStages = myElementsInStages.getCurrentCapacity();
	for(iCurrStage = 0; iCurrStage < iNumRenderStages; iCurrStage++)
	{
		if(!myElementsInStages[iCurrStage])
			continue;

		iNumElems = myElementsInStages[iCurrStage]->size();
		for(iElem = 0; iElem < iNumElems; iElem++)
		{
			(*myElementsInStages[iCurrStage])[iElem]->adjustToScreen();
		}
	}
}
/*****************************************************************************/
void UIPlane::onSliderValueChanged(UISliderElement* pSlider)
{
	const char* pcsSliderType = pSlider->getStringProp(PropertyId);

	// if(pSlider->getParent() && FAST_CAST<UIElement*>(pSlider->getParent())->getElemType() == UiElemTable)
	UIElement *pLinkedToElem = pSlider->getLinkedToElement<UIElement>();
	UITableElement* pLinkedToTable = as<UITableElement>(pLinkedToElem);
	if(pLinkedToTable)
		pLinkedToTable->scrollOnSlider(pSlider);
	else if(as<UIRichTextElement>(pLinkedToElem))
		as<UIRichTextElement>(pLinkedToElem)->scrollOnSlider(pSlider);
	else if(as<UIZoomWindowElement>(pLinkedToElem))
		as<UIZoomWindowElement>(pLinkedToElem)->scrollOnSlider(pSlider);
	else if(pLinkedToElem && pLinkedToElem->getBoolProp(PropertySupportsAutoScrollingX))
	{
		SVector2D svExistingOffset;
		pLinkedToElem->getExtraShiftOffset(svExistingOffset);
		pLinkedToElem->setExtraScrollOffset(-pSlider->getValue(), svExistingOffset.y, false);
	}
	else if(pLinkedToElem && pLinkedToElem->getBoolProp(PropertySupportsAutoScrollingY))
	{
		SVector2D svExistingOffset;
		pLinkedToElem->getExtraShiftOffset(svExistingOffset);
		pLinkedToElem->setExtraScrollOffset(svExistingOffset.x, -pSlider->getValue(), false);
	}
	else if(as<UIMultilineTextFieldElement>(pSlider->getParent()))
		as<UITextFieldElement>(pSlider->getParent())->scrollOnSlider(pSlider);
}
/*****************************************************************************/
void UIPlane::generateUniqueName(UIElement* pElem, string& strNameOut) // TElementAlphaMap& rTempStringSet, 
{
	strNameOut = pElem->getStringProp(PropertyId);

    if(strNameOut.length() == 0)
		strNameOut = "__generic__";

#if 1
	int* pCountPtr = myUniqueCounts.find(strNameOut.c_str());
	int iActuaCount = 1;
	if(pCountPtr)
	{
		iActuaCount = (*pCountPtr) + 1;
		*pCountPtr = iActuaCount;
	}
	else
		myUniqueCounts.insert(strNameOut.c_str(), iActuaCount);
	StringUtils::numberToString(iActuaCount,  theSharedString2);
	strNameOut += theSharedString2;

#else
    StringUtils::incrementStringSuffix(strNameOut);

	// Note that we need the string set because if we're generating a bunch of
	// unique names for something that, recursively, has two children with the
	// same starting name, we will generating conflicting non-unique names -
	// because we first generate en masse, then insert. But we check the unmodified
	// dictionary while generating...

    while(this->getElementByIdInternal(strNameOut.c_str(), true, false)
		//|| rTempStringSet.find(strNameOut) != rTempStringSet.end()
		|| rTempStringSet.findSimple(strNameOut.c_str()) != NULL
		)
		StringUtils::incrementStringSuffix(strNameOut);

	// Note that we don't care about the actual pointers here; just
	// whether the elem name is inserted or not.
	rTempStringSet.insert(strNameOut.c_str(), (UIElement*)1);
	//rTempStringSet.insert(strNameOut);
#endif
}
/*****************************************************************************/
void UIPlane::onUiElementDeleted(UIElement* pElem)
{
	if(myLastMouseElement == pElem)
		myLastMouseElement = NULL;
	if(myLastGlideElement == pElem)
		myLastGlideElement = NULL;
	if(myMouseLockedElem == pElem)
		myMouseLockedElem = NULL;

	// Check that it's not in our deletion list:
	if(myElemDelayedDeletionParentSet.size() > 0)
	{
		if(myElemDelayedDeletionParentSet.find(pElem) != myElemDelayedDeletionParentSet.end())
		{
			// Get its parent and put that in instead:
			UIElement* pParent = pElem->getParent<UIElement>();
			myElemDelayedDeletionParentSet.erase(pElem);
			if(pParent)
				myElemDelayedDeletionParentSet.insert(pParent);
		}
	}
}
/*****************************************************************************/
UIElement* UIPlane::getElementWithDefaultActionChild()
{
	int iCurrElement, iNumElements;
	UIElement* pElement;
	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		if(!pElement->getIsEnabled() || !pElement->getIsFullyShown())
			continue;

		if(pElement->getHasDefaultActionChild())
			return pElement;
	}

	return NULL;
}
/*****************************************************************************/
UIElement* UIPlane::getElementWithDefaultCancelChild()
{
	int iCurrElement, iNumElements;
	UIElement* pElement;

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		if(!pElement->getIsEnabled() || !pElement->getIsFullyShown())
			continue;

		if(pElement->getHasDefaultCancelChild())
			return pElement;
	}

	return NULL;
}
/*****************************************************************************/
void UIPlane::updateElementIfLayerShown(const char* pcsElemName, const char* pcsLayerName)
{
	if(getIsLayerShown(pcsLayerName))
		updateElement(pcsElemName);
}
/*****************************************************************************/
void UIPlane::updateElement(const char* pcsElemName)
{
	UIElement* pElem = this->getElementById<UIElement>(pcsElemName);
	if(!pElem)
		ASSERT_RETURN;
	pElem->updateDataRecursive();
}
/*****************************************************************************/
bool UIPlane::getAreAnyShownWithTag(const char* pcsTag)
{
	int iCurrElement, iNumElements;
	UIElement* pElement;
	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		if(!pElement->getIsFullyShown() && !pElement->getIsBeingShown())
			continue;

		// See if it matches the tag. Note that this is not recursive yet!
		if(pElement->hasTag(pcsTag))
			return true;
	}

	return false;
}
/*****************************************************************************/
void UIPlane::showPopupMessage(const char* pcsMessage, const char* pcsOptIcon)
{
	UIElement* pElem = this->getElementById<UIElement>("messageWidow", false);
	// We may legitemately not have the popup element if we're, say,
	// a color picker dialog or some other secondary UI.
	if(!pElem)
		return;

	if(pcsOptIcon)
		mySharedString = pcsOptIcon;
	UIElement *pChild = pElem->setTopAnimForChild("messageWidowIcon", pcsOptIcon ? mySharedString.c_str() : NULL);
	pChild->setIsVisible(pcsOptIcon != NULL);

	pElem->setText(pcsMessage);
	showUI("uilMessagePopup");
}
/*****************************************************************************/
void UIPlane::getAllElements(TUIElementVector& rVecOut)
{
	rVecOut.clear();
// 	int iCurr, iNum = myCurrentlyShownLayers.size();
// 	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
// 	{
		int iCurrElement, iNumElements;
		UIElement* pElement;

		iNumElements = myUIElements.size();
		for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
		{
			pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
			if(!pElement)
				continue;

			// Otherwise, see the element
			pElement->getAllElementsRecursive(rVecOut);
		}
//	}
}
/*****************************************************************************/
void UIPlane::getSelectedElements(TUIElementVector& rVecOut)
{
	rVecOut.clear();
// 	int iCurr, iNum = myCurrentlyShownLayers.size();
// 	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
// 	{
		int iCurrElement, iNumElements;
		UIElement* pElement;

		iNumElements = myUIElements.size();
		for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
		{
			pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
			if(!pElement)
				continue;

			// Otherwise, see the element
			pElement->getSelectedElementsRecursive(rVecOut);
		}
//	}

}
/*****************************************************************************/
void UIPlane::getElementsListAtScreenPosition(int iX, int iY, TUIElementVector& rVecOut)
{
	rVecOut.clear();
	int iCurr, iNum = myCurrentlyShownLayers.size();
	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
	{
		int iCurrElement, iNumElements;
		UIElement* pElement;

		iNumElements = myUIElements.size();
		for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
		{
			pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
			if(!pElement)
				continue;

			if(myCurrentlyShownLayers[iCurr] != pElement->getStringProp(PropertyLayer))
				continue;

			// Otherwise, see the element
			pElement->getElementsListAtScreenPositionRecursive(iX, iY, rVecOut);

		}
	}

	// Reverse so deeper elems come first
	std::reverse(rVecOut.begin(), rVecOut.end());
}
/*****************************************************************************/
ResourceItem* UIPlane::getDefaultsForTag(const char* pcsElemTag)
{
	ResourceItem* pDefaultsItem = NULL;
	ResourceCollection* pDefColl = NULL;
	if(!pDefaultsItem)
	{
		pDefColl = ResourceManager::getInstance()->getCollection(myInitCollectionType);
		pDefaultsItem = pDefColl->getDefaultFor(pcsElemTag);
	}
/*
	if(!pDefaultsItem)
	{
		pDefColl = ResourceManager::getInstance()->getCollection(ResourceSepWindowUIElements);
		pDefaultsItem = pDefColl->getDefaultFor(pcsElemTag);
	}
*/

	return pDefaultsItem;
}
/*****************************************************************************/
void UIPlane::showAsSubmenu(const char* pcsElemId, const SVector2D& svPoint, SideType eSide, FLOAT_TYPE fOffset, IMenuContentsProvider* pOptContentsProvider, const IMenuEnableStatusCallback* pOptEnableCallback, const char* pcsOptTargetDataSource, bool bOptDoHideAllOtherMenus)
{

	UIElement* pTargetElem = this->getElementById<UIElement>(pcsElemId);
	if(!pTargetElem)
		ASSERT_RETURN;

	if(bOptDoHideAllOtherMenus)
	{
		myCommonUiVElems.clear();
		if(as<UIMenuElement>(pTargetElem))
			as<UIMenuElement>(pTargetElem)->getHierarchyChain(pTargetElem, myCommonUiVElems);
		else
			myCommonUiVElems.push_back(pTargetElem);
		this->hideAllWithTag("submenu", &myCommonUiVElems, false, this, AnimOverActionGenericCallback);
	}

	const char* pcsMenuLayer = pTargetElem->getStringProp(PropertyLayer);

	if(pOptContentsProvider && as<UIMenuElement>(pTargetElem))
		as<UIMenuElement>(pTargetElem)->initMenuFrom(pOptContentsProvider);
	if(pOptEnableCallback && as<UIMenuElement>(pTargetElem))
		as<UIMenuElement>(pTargetElem)->setOwnedEnableStatusCallback(pOptEnableCallback);

	// Now, it's either custom or standard. If standard, open it up and go.
	setHideAllMenus(false);
	showUI(pcsMenuLayer, false, pcsOptTargetDataSource);
	pTargetElem->positionAsPopupRelativeTo(svPoint, eSide, fOffset, false);
}
/*****************************************************************************/
void UIPlane::addPostponedElement(UIElement* pElem, const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	SPostponedElemInfo rInfo;
	rInfo.myElem = pElem;
	rInfo.myOpacity = fOpacity;
	rInfo.myScale = fScale;
	rInfo.myScroll = svScroll;
	myPostponedElemInfos.push_back(rInfo);
}
/*****************************************************************************/
void UIPlane::showMessageBox(const char* pcsMessage, const char* pcsTitle /*= NULL*/)
{
	UIElement* pElem = this->getElementById<UIElement>("messageBox", false);
	if(!pElem)
		ASSERT_RETURN;

	pElem->getChildById("defDialogWindow", true, true)->setStringProp(PropertyTitle, pcsTitle ? pcsTitle : "Message");

	pElem->setTextForChild("msgBoxDialogText", pcsMessage);
	showUI("uilMessageBox");
}
/*****************************************************************************/
UIColorPicker* UIPlane::getCurrentlyOpenColorPicker()
{
	Window* pResWindow = NULL;
	Window* pTempWindow;
	WindowManager::Iterator wi;
	for(wi = WindowManager::getInstance()->windowsBegin(); !wi.isEnd(); wi++)
	{
		pTempWindow = wi.getWindow();
		if(pTempWindow->getUIPlane()->getIsLayerShown("uilColorPickerDialog"))
		{
			pResWindow = pTempWindow;
			return pResWindow->getUIPlane()->getElementById<UIColorPicker>("colorPickerDialog");
		}
	}
	return NULL;
}
/*****************************************************************************/
void UIPlane::showColorPicker(const SColor& scolExistingColor, const char* pcsSrcElemId, UNIQUEID_TYPE idParentWindow)
{
	// See if it's already open.
	Window* pResWindow = NULL;
	Window* pTempWindow;
	WindowManager::Iterator wi;
	for(wi = WindowManager::getInstance()->windowsBegin(); !wi.isEnd(); wi++)
	{
		pTempWindow = wi.getWindow();
		if(pTempWindow->getUIPlane()->getIsLayerShown("uilColorPickerDialog"))
		{
			pResWindow = pTempWindow;
			break;
		}
	}

	Window* pWindow = WindowManager::getInstance()->findItemById(idParentWindow);
	SVector2D svLastMousePos, svScreenWindowCenter;
	pWindow->getLastMousePos(svLastMousePos);
	pWindow->convertWindowCoordsToScreenCoords(svLastMousePos, svScreenWindowCenter);

	bool bDidCreate = false;
	if(!pResWindow)
	{
		int iColorPickerW = upToScreen(380);
		int iColorPickerH = upToScreen(184);
		pResWindow = Application::openNewWindow(getParentWindow(), ResourceColorPicker, iColorPickerW, iColorPickerH, true, false, iColorPickerW, iColorPickerH, "Color Picker", svScreenWindowCenter.x, svScreenWindowCenter.y);
		bDidCreate = true;
	}
	// No - the user could have positioned it to his heart's delight.
// 	else
// 		gPositionWindowCenter(pResWindow, svScreenWindowCenter.x, svScreenWindowCenter.y);
	if(!pResWindow)
		ASSERT_RETURN;

	UIPlane* pTargetUiPlane = pResWindow->getUIPlane();
	UIColorPicker* pPicker = pTargetUiPlane->getElementById<UIColorPicker>("colorPickerDialog");
	if(!bDidCreate)
	{
		// If we're still open, assume we've accepted the change, and add the color to the latest
		// picked color
		SColor scolNewColor;
		pPicker->getCurrentColor(scolNewColor);
		getParentWindow()->onAcceptNewColorFromPicker(scolNewColor);
	}

	pPicker->setColor(scolExistingColor, true, true, false);
	pPicker->setInitialColor(scolExistingColor);
	pPicker->setTargetForColorChange(pcsSrcElemId, idParentWindow);
	pPicker->setRecentColors(getParentWindow()->getRecentColors());
	if(bDidCreate)
		pTargetUiPlane->showUI("uilColorPickerDialog", true);
	else
		pResWindow->bringWindowToFront();
}
/*****************************************************************************/
void UIPlane::insertIntoNamesMap(UIElement* pElem, bool bInsertIntoUniqueNamesMapToo)
{
	pElem->insertIntoMap(myElementsMap, bInsertIntoUniqueNamesMapToo ? &myUniqueCounts : NULL);
}
/*****************************************************************************/
void UIPlane::removeFromNamesMap(UIElement* pElem)
{
	pElem->removeFromMap(myElementsMap);
}
/*****************************************************************************/
UIElement* UIPlane::testShownDropdownsRecursive(const SVector2D& svPoint, bool bFirstNonPassthrough, bool bGlidingOnly)
{
	int iCurrStage;
	int iElem, iNumElems;
	UIElement* pElement;
	UIElement* pResult = NULL;

	// In reverse since we want the topmost one.
	int iNumRenderStages = myElementsInStages.getCurrentCapacity();
	for(iCurrStage = iNumRenderStages - 1; iCurrStage >= 0; iCurrStage--)
	{
		if(!myElementsInStages[iCurrStage])
			continue;

		iNumElems = myElementsInStages[iCurrStage]->size();
		for(iElem = 0; iElem < iNumElems; iElem++)
		{
			// Check if it is visible at all - i.e. its state is right.
			pElement = (*myElementsInStages[iCurrStage])[iElem];

			if(!pElement->getIsVisible())
				continue;

			if(pElement->getStateAnimProgress() < FLOAT_EPSILON)
				continue;

			pResult = pElement->testShownDropdownsRecursive(svPoint, bFirstNonPassthrough, bGlidingOnly);
			if(pResult)
				return pResult;
		}
	}

	return NULL;
}
/*****************************************************************************/
void UIPlane::showRenameBox(const char* pcsCurrName, const char* pcsOkActionType, UIElement* pSourceElem)
{
	UIElement* pElem = this->getElementById<UIElement>("renameDialog", false);
	if(!pElem)
		ASSERT_RETURN;

	pElem->setTextForChild("renameTextField", "defTextBox", pcsCurrName);
	pElem->getChildAndSubchild("defBottomButtonContainer", "defOkButton")->setStringProp(PropertyActionValue, pcsOkActionType);

	showUI("uilRenameBox", false, pSourceElem ? pSourceElem->getStringProp(PropertyId) : NULL);
}
/*****************************************************************************/
void UIPlane::showYesNoBox(const char* pcsMessage, const char* pcsOkActionValue, UIElement* pSourceElem, const char* pcsOkButtonText, const char* pcsTitle, bool bShowApplyToAllCheckbox, const char* pcsCancelButtonText, const char* pcsCancelButtonActionValue)
{
	UIElement* pElem = this->getElementById<UIElement>("yesNoDialog", false);
	if(!pElem)
		ASSERT_RETURN;

	hideUI("uilYesNoBox", true);

	pElem->getChildById("defDialogWindow", true, true)->setStringProp(PropertyTitle, pcsTitle ? pcsTitle : "Confirmation Dialog");
	UIElement* pTextElem = pElem->setTextForChild("yesNoBoxDialogText", pcsMessage);

	// Now, set the buttons
	pElem->getChildAndSubchild("defBottomButtonContainer", "defOkButton")->setStringProp(PropertyActionValue, pcsOkActionValue);
	pElem->getChildAndSubchild("defBottomButtonContainer", "defOkButton")->setText(pcsOkButtonText ? pcsOkButtonText : "OK");

	pElem->getChildAndSubchild("defBottomButtonContainer", "defCancelButton")->setStringProp(PropertyAction, pcsCancelButtonActionValue ? pcsCancelButtonActionValue : UIA_HIDE_TOP_PARENT);
	pElem->getChildAndSubchild("defBottomButtonContainer", "defCancelButton")->setText(pcsCancelButtonText ? pcsCancelButtonText : "Cancel");

	pElem->setIsVisibleForChild("yesNoApplyToAllCheckbox", bShowApplyToAllCheckbox);

	// Also, auto-adjust to text size:
	if(pTextElem)
	{
		SVector2D svDims;
		string strTemp(pcsMessage);
		RenderUtils::measureText(getParentWindow(), strTemp.c_str(), pTextElem->getCachedFont(), pTextElem->getCachedFontSize(), pTextElem->getTextWidth(), svDims);
		if(svDims.y < upToScreen(40.0))
			svDims.y = upToScreen(40.0);
		pTextElem->setNumProp(PropertyHeight, svDims.y);
	}

	// To remove checkbox spacing
	pElem->adjustToScreen();

	showUI("uilYesNoBox", false, pSourceElem ? pSourceElem->getStringProp(PropertyId) : NULL);

}
/*****************************************************************************/
void UIPlane::showYesNoCancelBox(const char* pcsMessage, const char* pcsYesActionValue, const char* pcsNoActionValue, const char* pcsCancelActionValue, UIElement* pSourceElemAndCallback, IHandleActionCallback* pHandleCallback, const char* pcsYesButtonText, const char* pcsNoButtonText,
					const char* pcsCancelButtonText, const char* pcsTitle, bool bShowApplyToAllCheckbox)
{
	UIElement* pElem = this->getElementById<UIElement>("yesNoCancelDialog", false);
	if(!pElem)
		ASSERT_RETURN;

	hideUI("uilYesNoCancelBox", true);

	pElem->setHandleActionCallback(pHandleCallback);

	pElem->getChildById("defDialogWindow", true, true)->setStringProp(PropertyTitle, pcsTitle ? pcsTitle : "Confirmation Dialog");
	pElem->setTextForChild("yesNoBoxCancelDialogText", pcsMessage);

	// Now, set the buttons
	pElem->getChildAndSubchild("defBottomButtonContainer", "defOkButton")->setStringProp(PropertyActionValue, pcsYesActionValue);
	pElem->getChildAndSubchild("defBottomButtonContainer", "defOkButton")->setText(pcsYesButtonText ? pcsYesButtonText : "Yes");

	pElem->getChildAndSubchild("defBottomButtonContainer", "defNoButton")->setStringProp(PropertyActionValue, pcsNoActionValue);
	pElem->getChildAndSubchild("defBottomButtonContainer", "defNoButton")->setText(pcsNoButtonText ? pcsNoButtonText : "No");

	pElem->getChildAndSubchild("defBottomButtonContainer", "defCancelButton")->setStringProp(PropertyActionValue, pcsCancelActionValue);
	pElem->getChildAndSubchild("defBottomButtonContainer", "defCancelButton")->setText(pcsCancelButtonText ? pcsCancelButtonText : "Cancel");

	pElem->setIsVisibleForChild("yesNoCancelApplyToAllCheckbox", bShowApplyToAllCheckbox);

	// To remove checkbox spacing
	pElem->adjustToScreen();

	showUI("uilYesNoCancelBox", false, pSourceElemAndCallback ? pSourceElemAndCallback->getStringProp(PropertyId) : NULL);

}
/*****************************************************************************/
bool UIPlane::loadDefaultParmsFrom(UIElement* pElem, PropertyType eProp, StringResourceItem& rDefaultsOut)
{
	bool bHaveDefaults = false;
	if(pElem->doesPropertyExist(eProp))
	{
		// This is a pair-wise list of default parm values. We need to convert it
		// to a string resource item.
		string strDelims(" |");
		mySharedString = pElem->getStringProp(eProp);
		if(IS_VALID_STRING_AND_NOT_NONE(mySharedString.c_str()))
		{
			TokenizeUtils::tokenizeString(mySharedString, strDelims, myCommonStringVector);
			bHaveDefaults = rDefaultsOut.loadFromStringList(myCommonStringVector);
		}
	}

	return bHaveDefaults;
}
/*****************************************************************************/
void UIPlane::performUiAction(const char* pcsAction, const char* pcsActionValue, const char* pcsActionValue2, const char* pcsTargetData)
{
	UIButtonElement rDummyElem(this);
	rDummyElem.onAllocated(this);
	rDummyElem.setStringProp(PropertyAction, pcsAction);
	rDummyElem.setStringProp(PropertyId, "__performUiActionDummy__");
	if(IS_VALID_STRING_AND_NOT_NONE(pcsActionValue))
		rDummyElem.setStringProp(PropertyActionValue, pcsActionValue);
	if(IS_VALID_STRING_AND_NOT_NONE(pcsActionValue2))
		rDummyElem.setStringProp(PropertyActionValue2, pcsActionValue2);
	if(IS_VALID_STRING_AND_NOT_NONE(pcsTargetData))
		rDummyElem.setStringProp(PropertyTargetDataSource, pcsTargetData);
	this->onButtonClicked(&rDummyElem, MouseButtonLeft);
}
/*****************************************************************************/
UISplitterElement* UIPlane::splitElement(UIElement* pElem, DirectionType eDir, int iOverrideChildPos)
{
	UIElement* pParent = pElem->getParent<UIElement>();
	// Valid in general, but unsupported for now
	if(!pParent)
		ASSERT_RETURN_NULL;

	// Remove it from us
	int iIndex = pParent->getChildIndex(pElem);
	pParent->deleteChild(pElem, false);

	// Add a splitter
	UISplitterElement* pNewSplitter = as<UISplitterElement>(addNewElement(eDir == DirectionVertical ? "__vertSplitterTemplate__" : "__horSplitterTemplate__", pParent, iIndex));
	if(!pNewSplitter)
		ASSERT_RETURN_NULL;

	// Now, make the old child its parent
	pNewSplitter->parentExistingNode(pElem, iOverrideChildPos);
	if(pParent)
		pParent->invalidateSizingInfos();

	pParent->adjustToScreen();
	return pNewSplitter;
}
/*****************************************************************************/
bool UIPlane::receiveDragDrop(UIElement* pDraggedElem, SVector2D& svScreenPos, UIElement* pTopmostOverElem)
{
	if(!pTopmostOverElem)
		return false;

	UITabWindowElement* pSourceTabHost = as<UITabWindowElement>(pDraggedElem);
	if(!pSourceTabHost)
		return false;

	SUIDragRegion* pOverRegion = myUiDragDropManager.getRegionAt(svScreenPos);
	if(!pOverRegion)
		return false;

	UITabWindowElement *pOverTabParent = as<UITabWindowElement>(pTopmostOverElem);
	if(!pOverTabParent)
		pOverTabParent = pTopmostOverElem->getParentOfType<UITabWindowElement>();
	if(!pOverTabParent && !(pOverRegion && pOverRegion->myIsGlobal))
		return false;

	// Otherwise, see the side:
	const char* pcsTabWindowTemplateType = "__tabWindowHolderTemplate__";
	bool bRes = false;
	UISplitterElement* pNewSplitter = NULL;
	UITabWindowElement *pFinalTabAdopter = NULL;

	UIElement*pElemToSplit = pOverTabParent;
	if(pOverRegion->myIsGlobal)
	{
		pElemToSplit = getElementById("toolSplitter");
		if(pElemToSplit)
			pElemToSplit = pElemToSplit->getChild<UIElement>(1);
	}

	if(!pElemToSplit)
		ASSERT_RETURN_FALSE;

	if(pOverRegion->mySide == SideCenter)
		pFinalTabAdopter = pOverTabParent;
	else if(pOverRegion->mySide == SideRight)
	{
		pNewSplitter = pElemToSplit->split(DirectionVertical, 0);
 		if(pNewSplitter)
			pFinalTabAdopter = as<UITabWindowElement>(addNewElement(pcsTabWindowTemplateType, pNewSplitter, 1));
	}
	else if(pOverRegion->mySide == SideLeft)
	{
		pNewSplitter = pElemToSplit->split(DirectionVertical, 1);
		if(pNewSplitter)
			pFinalTabAdopter = as<UITabWindowElement>(addNewElement(pcsTabWindowTemplateType, pNewSplitter, 0));
	}
	else if(pOverRegion->mySide == SideBottom)
	{
		pNewSplitter = pElemToSplit->split(DirectionHorizontal, 0);
		if(pNewSplitter)
			pFinalTabAdopter = as<UITabWindowElement>(addNewElement(pcsTabWindowTemplateType, pNewSplitter, 1));
	}
	else if(pOverRegion->mySide == SideTop)
	{
		pNewSplitter = pElemToSplit->split(DirectionHorizontal, 1);
		if(pNewSplitter)
			pFinalTabAdopter = as<UITabWindowElement>(addNewElement(pcsTabWindowTemplateType, pNewSplitter, 0));
	}

	if(pFinalTabAdopter)
		bRes = pFinalTabAdopter->adoptTab(pSourceTabHost, pSourceTabHost->getTabBeingDragged());

	// Now, see whether we need to kill the source UI elems.
	// Do not kill the main window since we have hard-coded refs to it, such as opening a file...
	if(pSourceTabHost->getNumChildren() == 0 && !IS_STRING_EQUAL(pSourceTabHost->getStringProp(PropertyId), "mainImageParentWindow"))
	{
		// Delete from the parent
		UISplitterElement* pSrcSplitter = pSourceTabHost->getParent<UISplitterElement>();
		// Do not delete our main splitter, either.
		if(pSrcSplitter && !IS_STRING_EQUAL(pSrcSplitter->getStringProp(PropertyId), "toolSplitter"))
			scheduleForDeletion(pSourceTabHost);
	}

	if(pNewSplitter)
		pNewSplitter->adjustToScreen();

	UILayoutManager::getInstance()->setDidChangeLayoutThisSession(true);

	return bRes;
}
/*****************************************************************************/
bool UIPlane::onDragDropHover(UIElement* pDraggedElem, SVector2D& svScreenPos, UIElement* pTopmostOverElem)
{
	return false;
}
/*****************************************************************************/
void UIPlane::fuseElement(UISplitterElement* pSplitter)
{
	if(!pSplitter)
		return;

	UIElement* pParent = pSplitter->getParent<UIElement>();
	if(!pParent)
		return;

	// Now, it must have less than two children:
	int iNumChildren = pSplitter->getNumChildren();
	if(iNumChildren >= 2)
		ASSERT_RETURN;

	UIElement* pChildToSave = NULL;
	if(iNumChildren > 0)
	{
		pChildToSave = pSplitter->getChild<UIElement>(0);
		// Delete child but not deallocate it
		pSplitter->deleteChild(pChildToSave, false);
	}

	// Now, we're an empty splitter
	_ASSERT(pSplitter->getNumChildren() == 0);

	// Kill it. With fire.
	int iIndex = pParent->getChildIndex(pSplitter);
	deleteElement(pSplitter);

	// And finally re-parent our remaining child
	pParent->parentExistingElement(pChildToSave, iIndex);
}
/*****************************************************************************/
void UIPlane::deleteQueuedElements()
{
	if(myElementsToDelete.size() == 0)
		return;

	UIElement* pElem;
	UISplitterElement* pSrcSplitter;
	TUIElementSet::iterator si;
	TUIElementSet::iterator si2;

	// We need to save the set of unique top parents of our elements that are
	// not to be deleted and call adjust to screen on them...
	myElemDelayedDeletionParentSet.clear();
	UIElement* pParent;
	bool bAllowKeeping;
	const char* pcsParentId;
	// Ideally, we need to get the first parent of the element that is not
	// within any deleted elements...
	for(si = myElementsToDelete.begin(); si != myElementsToDelete.end(); si++)
	{
		pParent = (*si)->getParent<UIElement>();

		pcsParentId = pParent->getStringProp(PropertyId);
		bAllowKeeping = true;
		// See if this parent is within any element to be deleted
		for(si2 = myElementsToDelete.begin(); si2 != myElementsToDelete.end() && bAllowKeeping; si2++)
		{
			if((*si2) == (*si))
				continue;

			if((*si2) == pParent || (*si2)->getChildById(pcsParentId) != NULL)
				bAllowKeeping = false;
		}

		if(bAllowKeeping)
			myElemDelayedDeletionParentSet.insert(pParent);
	}

	for(si = myElementsToDelete.begin(); si != myElementsToDelete.end(); si++)
	{
		pElem = (*si);
		pSrcSplitter = pElem->getParent<UISplitterElement>();
		deleteElement(pElem);
		// Now, fuse this splitter
		fuseElement(pSrcSplitter);
	}
	myElementsToDelete.clear();

	// Now, call adjust to screen on all saved parents:
	for(si = myElemDelayedDeletionParentSet.begin(); si != myElemDelayedDeletionParentSet.end(); si++)
		(*si)->adjustToScreen();

	myElemDelayedDeletionParentSet.clear();
}
/*****************************************************************************/
bool UIPlane::getAreAnyElementsInLayerShown(const char* pcsLayerName)
{
	int iCurrClass;
	int iCurrElement, iNumElements;
	UIElement* pElement;

	const char* pcsCurrLayerName;

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		pcsCurrLayerName = pElement->getStringProp(PropertyLayer);
		if(!IS_STRING_EQUAL(pcsLayerName, pcsCurrLayerName))
			continue;

		if(pElement->getIsBeingShown() || pElement->getIsFullyShown())
			return true;
	}

	return false;
}
/*****************************************************************************/
UIElement* UIPlane::getTopLevelElementByLayerName(const char* pcsName)
{
	if(!IS_VALID_STRING_AND_NOT_NONE(pcsName))
		return NULL;

	int iCurrElement, iNumElements;
	UIElement* pElement;

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;
		if(IS_STRING_EQUAL(pElement->getStringProp(PropertyLayer), pcsName))
			return pElement;
	}

	return NULL;
}
/*****************************************************************************/
void UIPlane::queueForFocus(const CHAR_TYPE* pcsElemId)
{
	SCOPED_MUTEX_LOCK(&myRefreshQueueLock);
	myFocusQueue.insert(pcsElemId ? pcsElemId : EMPTY_STRING);
}
/*****************************************************************************/
void UIPlane::ensureElementStageExists(int iRenderStage)
{
	myElementsInStages.ensureCapacity(iRenderStage + 1);
	if(!myElementsInStages[iRenderStage])
		myElementsInStages[iRenderStage] = new TUIElementVector;
}
/*****************************************************************************/
void UIPlane::reloadAll()
{
	TUIElementVector* pAllObjects = this->getUIElements();

	ResourceItem* pItem;
	UIElement* pObj;
	int iCurr, iNum = pAllObjects->size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pObj = dynamic_cast<UIElement*>((*pAllObjects)[iCurr]);
		_ASSERT(pObj);

		// Try to find it in our collections
		pItem = UIElement::getDefinitionBlindSearch(myInitCollectionType, pObj->getStringProp(PropertyId));
		if(!pItem)
			pItem = UIElement::getDefinitionBlindSearch(myInitCollectionType, pObj->getStringProp(PropertyOldId));
		_ASSERT(pItem);

		pObj->mergeResourcesFrom(*pItem, true, this->getAllocator(), false, true);
		pObj->resetAllCaches(true);
		pObj->resetOnReloadDebug();
	}

	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		// Done separately to make sure everything is reset.
		pObj = dynamic_cast<UIElement*>((*pAllObjects)[iCurr]);
		pObj->resetUiTablesRecursive();
	}
}
/*****************************************************************************/
void UIPlane::setIsVisibleForAllWithTag(const char* pcsTag, bool bIsVisible)
{
	int iCurrElement, iNumElements;
	UIElement* pElement;

	iNumElements = myUIElements.size();
	for(iCurrElement = iNumElements - 1; iCurrElement >= 0; iCurrElement--)
	{
		pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElement]);
		if(!pElement)
			continue;

		setIsVisibleForAllWithTagRecursive(pElement, pcsTag, bIsVisible);
	}

}
/*****************************************************************************/
void UIPlane::setIsVisibleForAllWithTagRecursive(UIElement* pElement, const char* pcsTag, bool bIsVisible)
{
	if(pElement->hasTag(pcsTag))
		pElement->setIsVisible(bIsVisible);

	UIElement* pChild;
	int iCurrChild, iNumChildren = pElement->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(pElement->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		setIsVisibleForAllWithTagRecursive(pChild, pcsTag, bIsVisible);
	}
}
/*****************************************************************************/
};
