#include "stdafx.h"

#define GRADIENT_BAR_RECT_HOR_INNER_PADDING		upToScreen(12.0)
#define BOTTOM_CONTROLS_SPACE					upToScreen(130.0)

namespace HyperUI
{
/*****************************************************************************/
UIGradientEdit::UIGradientEdit(UIPlane* pParentPlane)
	: UISlidingStopsEdit(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIGradientEdit::onAllocated(IBaseObject* pData)
{
	UISlidingStopsEdit::onAllocated(pData);


#if 0
	SColor scolSource(0, 1, 0, 1);
	SColor scolMask(1,1,1,1);
	Image rFullImage = ImageUtils::createNewImage(100, 200, scolSource);
	Image rMaskImage = ImageUtils::createNewImage(50, 100, scolMask);

	SVector2D svImageOff(-20, -1);
	SVector2D svMaskOff(0, 0);
	ImageUtils::multiplyImageByMask(rFullImage, svImageOff, rMaskImage, svMaskOff, true, false);

	rFullImage.write("db_mask_apply_test.png");

	int bp = 0;
#endif

#if 0
	myGradient.setColorStop(0.0, 1, 0, 0);
	myGradient.setColorStop(1.0, 0, 0, 1);
	myGradient.setOpacityStop(0.0, 1.0);
	myGradient.setOpacityStop(1.0, 1.0);
	string strTest;
	myGradient.saveToString(strTest);
	FileUtils::saveToFile("db_grad_test.txt", strTest);
	int bp = 0;
#endif

#if 0
#ifdef _DEBUG
	// TEMP: 
	myGradient.setColorStop(0.0, 1, 0, 0);
	myGradient.setColorStop(0.25, 0, 1, 0);
	myGradient.setColorStop(1.0, 0, 0, 1);

	myGradient.setOpacityStop(0.0, 1.0);
	myGradient.setOpacityStop(1.0, 1.0);
	//myGradient.setOpacityStop(1.0, 0.25);
	recreateAllStops();

	Image rImage = ImageUtils::createNewImage(400, 200, 0.0);
	ImageUtils::fillImageWithAngularGradient(rImage, myGradient, 90, 400, 200, false, 1.0, 0, 0);
	rImage.write("db_grad_test.png");
	int bp = 0;

// 	Image rTest = ImageUtils::generateGradientImage(400, 200, myGradient, 45, 1.0, 100, 0);
// 	rTest.write("db_grad_test.png");
// 	int bp = 0;
#endif
#endif
}
/*****************************************************************************/
void UIGradientEdit::postInit(void)
{
	UISlidingStopsEdit::postInit();
	setChildTargetIdentifiers();
}
/*****************************************************************************/
void UIGradientEdit::postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	SRect2D srOwnRect;
	getGlobalRectangle(srOwnRect);

	// Now, we need to render handles, the background, and the 
	SRect2D srGradientBarRect;
	srGradientBarRect.h = srOwnRect.h - HANDLE_VERTICAL_SPACE*2 - BOTTOM_CONTROLS_SPACE;
	srGradientBarRect.w = srOwnRect.w - GRADIENT_BAR_RECT_HOR_INNER_PADDING*2.0;
	srGradientBarRect.x = srOwnRect.x + GRADIENT_BAR_RECT_HOR_INNER_PADDING;
	srGradientBarRect.y = srOwnRect.y + HANDLE_VERTICAL_SPACE;
	srGradientBarRect.roundToInt();
	myGradientRect = srGradientBarRect;

	SColor scolOutline(94.0/255.0,94.0/255.0,94.0/255.0,fOpacity);
	getDrawingCache()->fillArea("emptyBack", srGradientBarRect, fOpacity, 1.0, fScale);
	getDrawingCache()->addGradientRectangle(srGradientBarRect, myGradient);

	UISlidingStopsEdit::postRender(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
void UIGradientEdit::recreateAllFromGradient(const SGradient& rGrad)
{
	clearAllStops();

	UIGradientSliderStop *pStop;
	SGradient::StopsIterator si;
	for(si = rGrad.colorStopsBegin(); !si.isEnd(); si++)
	{
		pStop = new UIGradientSliderStop;
		pStop->myId = si.getStop()->myId;
		pStop->myIsOpacityStop = false;
		myStops.push_back(pStop);
	}

	for(si = rGrad.opacityStopsBegin(); !si.isEnd(); si++)
	{
		pStop = new UIGradientSliderStop;
		pStop->myId = si.getStop()->myId;
		pStop->myIsOpacityStop = true;
		myStops.push_back(pStop);
	}

	refreshSelectedStopRelatedUI(false);
}
/*****************************************************************************/
void UIGradientEdit::getStopCenter(UISliderStop* pUiStop, const SVector2D& svStopSize, SVector2D& svCenterOut)
{
	svCenterOut.set(0, 0);
	const SGradientStop* pStop = myGradient.findStopById(pUiStop->myId);
	if(!pStop)
		ASSERT_RETURN;

	svCenterOut.x = myGradientRect.x + myGradientRect.w*pStop->getPos();
	if(pStop->myIsOpacityStop)
		svCenterOut.y = myGradientRect.y - svStopSize.y/2.0;
	else
		svCenterOut.y = myGradientRect.y + myGradientRect.h + svStopSize.y/2.0;
}
/*****************************************************************************/
void UIGradientEdit::refreshStopRelatedUI(UISliderStop* pStopIn, SlidingStopsSelectionType eSelTypeGeneric, bool bIsChangingContinuously)
{
	UIElement* pElem;
	GradientSelectionType eSelType = getGradientSelectionTypeFromGenericSelection(pStopIn, eSelTypeGeneric);

	const SGradientStop* pOrigStop = NULL;
	if(pStopIn)
		pOrigStop = myGradient.findStopById(pStopIn->myId);

	pElem = setIsEnabledForChild("defGradPosition", eSelType == GradientSelectionOpacity || eSelType == GradientSelectionColor);
	if(eSelType == GradientSelectionOpacity || eSelType == GradientSelectionColor)
	{
		UITextFieldElement* pTextField = as<UITextFieldElement>(pElem->getChildById("defNumBox", true, true));
		if(pOrigStop)
			pTextField->setNumericValue(pOrigStop->getPos()*100.0, UnitPercent, bIsChangingContinuously);
		pTextField->updateLinkedElement(false, bIsChangingContinuously);
	}

	pElem = setIsVisibleForChild("defGradColor", eSelType == GradientSelectionColor);
	if(eSelType == GradientSelectionColor)
	{
		// Set the color
		if(pOrigStop)
			as<UIColorSwatch>(pElem->getChildById("defColor", true, true))->setColor(pOrigStop->myColor);
	}
	pElem = setIsVisibleForChild("defGradOpacity", eSelType == GradientSelectionOpacity);
	if(eSelType == GradientSelectionOpacity)
	{
		UITextFieldElement* pTextField = as<UITextFieldElement>(pElem->getChildById("defNumBox", true, true));
        if(pOrigStop)
            pTextField->setNumericValue(pOrigStop->myColor.alpha*100.0, UnitPercent, bIsChangingContinuously);
        ELSE_ASSERT;
		pTextField->updateLinkedElement(false, bIsChangingContinuously);
	}

	pElem = setIsVisibleForChild("defGradMultSelMessage", eSelType == GradientSelectionMultiple || eSelType == GradientSelectionNone);
	if(eSelType == GradientSelectionNone)
		pElem->setText("No stops selected.");
	else
		pElem->setText("Multiple stops selected.");

	// Now, set our disabled stops
	pElem = setIsEnabledForChild("defGradSplitKey", eSelType == GradientSelectionColor);
	as<UICheckboxElement>(pElem)->setIsPushed(pOrigStop && pOrigStop->myIsSplit);

	pElem = setIsEnabledForChild("defGradSecondaryColor", eSelType == GradientSelectionColor && pOrigStop && pOrigStop->myIsSplit);
	if(pOrigStop)
		as<UIColorSwatch>(pElem->getChildById("defColor"))->setColor(pOrigStop->mySecondaryColor);
	pElem = setIsEnabledForChild("defGradSwapKeySidesButton", eSelType == GradientSelectionColor && pOrigStop && pOrigStop->myIsSplit);
}
/*****************************************************************************/
void UIGradientEdit::setChildTargetIdentifiers()
{
	getChildById("defGradColor", true, true)->getChildById("defColor", true, true)->setTargetUiElem(this);
	getChildById("defGradPosition", true, true)->getChildById("defNumBox", true, true)->setTargetUiElem(this);
	getChildById("defGradOpacity", true, true)->getChildById("defNumBox", true, true)->setTargetUiElem(this);
	getChildById("defGradSecondaryColor", true, true)->getChildById("defColor", true, true)->setTargetUiElem(this);
	getChildById("defGradSplitKey", true, true)->setTargetUiElem(this);
	getChildById("defGradSwapKeySidesButton", true, true)->setTargetUiElem(this);
}
/*****************************************************************************/
void UIGradientEdit::onRefreshStoredValueFromUICallback(UIElement* pSourceElem, bool bIsChangingContinuously)
{
	if(myIsRefreshingUI)
		return;

	int iSelIdx;
	getSelectionType(iSelIdx);

	// This happens on post-init.
	if(iSelIdx < 0)
		return;

	bool bRefreshOwnUI = false;

	// See what kind if an elem we are
	if(as<UIColorSwatch>(pSourceElem))
	{
		SColor scolTemp;
		as<UIColorSwatch>(pSourceElem)->getColor(scolTemp);

		const char* pcsParentNameElemName = pSourceElem->getParent()->getStringProp(PropertyOldId);

		// See what our selected handle is
		if(IS_STRING_EQUAL(pcsParentNameElemName, "defGradSecondaryColor"))
			myGradient.changeSecondaryStopColorTo(myStops[iSelIdx]->myId, scolTemp);
		else
			myGradient.changeStopColorTo(myStops[iSelIdx]->myId, scolTemp);
	}
	else if(as<UITextFieldElement>(pSourceElem))
	{
		// See whether it's opacity or position		
		UITextFieldElement* pCastElem = as<UITextFieldElement>(pSourceElem);
		if(pSourceElem->getParentById("defGradPosition", true))
		{
			// Position
			myGradient.moveStopTo(myStops[iSelIdx]->myId, pCastElem->getNumericValue(DEFAULT_STORAGE_UNITS)/100.0);
		}
		else
		{
			// Opacity
			myGradient.changeStopOpacityTo(myStops[iSelIdx]->myId, pCastElem->getNumericValue(DEFAULT_STORAGE_UNITS)/100.0);
		}
	}
	else if(as<UICheckboxElement>(pSourceElem))
	{
		// Must be our split keys checkbox.
		myGradient.setIsStopSplit(myStops[iSelIdx]->myId, as<UICheckboxElement>(pSourceElem)->getIsPushed());
		bRefreshOwnUI = true;
	}
	handleTargetElementUpdate(bIsChangingContinuously);

// 	if(bRefreshOwnUI)
// 		refreshSelectedStopRelatedUI(bIsChangingContinuously);
}
/*****************************************************************************/
bool UIGradientEdit::handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData)
{
	// Save undo...
	_ASSERT(myCurrUndoBlockId < 0);
	UIElement* pParmElem = getLinkedToElementWithValidTarget();
	UNIQUEID_TYPE lUndoBlock = -1;
	bool bDidHandle = false;
	bool bAddUndos = pParmElem && UndoManager::canAcceptNewUndoNow();

	if(strAction == "uiaMirrorGradient")
	{
		if(bAddUndos)
			lUndoBlock = UndoManager::addUndoItemToCurrentManager(pParmElem->getUndoStringForSelfChange(), pParmElem->createUndoItemForSelfChange(), true, getParentWindow(), NULL);	

		// Mirror the gradient
		SideType eSide = mapStringToType(pSourceElem->getStringProp(PropertyActionValue), g_pcsSideStrings, SideBottom);
		myGradient.mirror(eSide, pSourceElem->getBoolProp(PropertyActionValue2));
		recreateAllStops();
		handleTargetElementUpdate(false);
		bDidHandle = true;
	}
	else if(strAction == "uiaSwapGradStopColors")
	{
		if(bAddUndos)
			lUndoBlock = UndoManager::addUndoItemToCurrentManager(pParmElem->getUndoStringForSelfChange(), pParmElem->createUndoItemForSelfChange(), true, getParentWindow(), NULL);	

		int iSelIdx;
		SlidingStopsSelectionType eSelTypeGeneric = getSelectionType(iSelIdx);
		UISliderStop *pStopIn = iSelIdx >= 0 ? myStops[iSelIdx] : NULL;

		if(pStopIn)
		{
			myGradient.swapKeyColors(pStopIn->myId);
			handleTargetElementUpdate(false);
			refreshSelectedStopRelatedUI(false);
		}
		bDidHandle = true;
	}
	else if(strAction == "uiaChangeThumbnailSelection")
	{
		// This means our library dropdown was chosen
		const char* pcsSelImageId = pSourceElem->getParentOfType<UITableCellElement>()->getStringProp(PropertyTargetDataSource);
		// Get to the actual gradient
		const StringResourceItem* pGradItem = this->getGradientItemById(pcsSelImageId);
		if(pGradItem)
		{
			if(bAddUndos)
				lUndoBlock = UndoManager::addUndoItemToCurrentManager(pParmElem->getUndoStringForSelfChange(), pParmElem->createUndoItemForSelfChange(), true, getParentWindow(), NULL);	

			const char* const pcsAnimProp = PropertyMapper::getInstance()->getPropertyString(PropertyFile);
			const char* pcsGradString = pGradItem->getStringProp(pcsAnimProp);
			myGradient.loadFromString(pcsGradString);
			recreateAllStops();
			handleTargetElementUpdate(false);

		}
		ELSE_ASSERT;
		bDidHandle = true;
	}
	else
		bDidHandle = UISlidingStopsEdit::handleAction(strAction, pSourceElem, pData);

	if(lUndoBlock >= 0)
		UndoManager::endUndoBlockInCurrentManager(lUndoBlock, getParentWindow(), NULL);

	return bDidHandle;
}
/*****************************************************************************/
bool UIGradientEdit::getStopText(UISliderStop* pStop, string& strOut)
{
	UIGradientSliderStop* pCastStop = as<UIGradientSliderStop>(pStop);
	const SGradientStop* pOrigStop = myGradient.findStopById(pStop->myId);

	if(pCastStop->myIsOpacityStop)
	{
		StringUtils::numberToNiceString(pOrigStop->myColor.alpha*100.0, strOut);
		strOut += "%";
		return true;
	}
	return false;
}
/*****************************************************************************/
void UIGradientEdit::removeStopCallback(UISliderStop* pStop)
{
	myGradient.removeStop(pStop->myId);
}
/*****************************************************************************/
void UIGradientEdit::addNewStop(FLOAT_TYPE fPos, const SVector2D svTouchPoint)
{
	SColor scolNewCol;
	UNIQUEID_TYPE lNewId;
	UIGradientSliderStop *pStop = new UIGradientSliderStop;
	myGradient.getColorAndOpacityFor(fPos, scolNewCol);
	if(svTouchPoint.y < myGradientRect.y + myGradientRect.h/2.0)
	{
		// Opacity
		lNewId = myGradient.setOpacityStop(fPos, scolNewCol.alpha);
		pStop->myIsOpacityStop = true;
	}
	else
	{
		// Color
		lNewId = myGradient.setColorStop(fPos, scolNewCol.r, scolNewCol.g, scolNewCol.b);
		pStop->myIsOpacityStop = false;
	}

	// Make it selected
	pStop->myId = lNewId;
	pStop->setIsSelected(true);
	myStops.push_back(pStop);
}
/*****************************************************************************/
FLOAT_TYPE UIGradientEdit::getStopPos(UISliderStop* pUiStop)
{
	const SGradientStop* pStop = myGradient.findStopById(pUiStop->myId);
	return pStop->getPos();
}
/*****************************************************************************/
void UIGradientEdit::moveStopTo(UISliderStop* pUiStop, FLOAT_TYPE fNewPos)
{
	myGradient.moveStopTo(pUiStop->myId, fNewPos);
}
/*****************************************************************************/
bool UIGradientEdit::getIsPointInAdditionRegion(const SVector2D& svPoint)
{
	return svPoint.y < (myGradientRect.y + myGradientRect.h + HANDLE_VERTICAL_SPACE);
}
/*****************************************************************************/
void UIGradientEdit::drawCustomStopInnerContents(UISliderStop* pStop, const SVector2D& svStopCenter, FLOAT_TYPE fOpacity)
{
	const SGradientStop* pOrigStop = myGradient.findStopById(pStop->myId);

	SColor scolTemp;
	SRect2D srColorRect;
	srColorRect.w = srColorRect.h = upToScreen(8.0);

	srColorRect.x = svStopCenter.x - upToScreen(4.0);
	srColorRect.y = svStopCenter.y - upToScreen(2.0);
	scolTemp = pOrigStop->myColor;
	scolTemp.alpha = fOpacity;

	if(pOrigStop->myIsSplit)
	{
		SRect2D srTemp;
		srTemp = srColorRect;
		srTemp.w *= 0.5;
		getDrawingCache()->addRectangle(srTemp, scolTemp);

		scolTemp = pOrigStop->mySecondaryColor;
		scolTemp.alpha = fOpacity;
		srTemp = srColorRect;
		srTemp.w *= 0.5;
		srTemp.x += srTemp.w;
		getDrawingCache()->addRectangle(srTemp, scolTemp);
	}
	else
		getDrawingCache()->addRectangle(srColorRect, scolTemp);

}
/*****************************************************************************/
bool UIGradientEdit::nudgeStopValue(UISliderStop* pStop, FLOAT_TYPE fByNormAmount)
{
	bool bDidHandle = false;
	UIGradientSliderStop* pCastStop = as<UIGradientSliderStop>(pStop);
	const SGradientStop* pOrigStop = myGradient.findStopById(pStop->myId);
	if(pCastStop->myIsOpacityStop)
	{
		myGradient.changeStopOpacityTo(pStop->myId, pOrigStop->myColor.alpha + fByNormAmount);
		bDidHandle = true;
	}
	return bDidHandle;
}
/*****************************************************************************/
void UIGradientEdit::onStopDoubleClicked(UISliderStop* pUiStop, TTouchVector& vecTouches)
{
	const SGradientStop* pStop = myGradient.findStopById(pUiStop->myId);

	SRect2D srStopRect;
	getStopRectangle(pUiStop, srStopRect);

	// Hack: find our color element and simulate a click on that.
	UIColorSwatch *pColorSwatch;
	if(!pStop->myIsSplit || vecTouches[0].myPoint.x < srStopRect.x + srStopRect.w*0.5)
		pColorSwatch = getChildAndSubchild<UIColorSwatch>("defGradColor", "defColor");
	else
		pColorSwatch = getChildAndSubchild<UIColorSwatch>("defGradSecondaryColor", "defColor");
	pColorSwatch->simulateClick();
}
/*****************************************************************************/
bool UIGradientEdit::getIsChildIdentifierEnabled(UIElement* pIdentifierOwnerElem)
{
	const char* pcsElemName = pIdentifierOwnerElem->getStringProp(PropertyOldId);
	const char* pcsParentNameElemName = pIdentifierOwnerElem->getParent()->getStringProp(PropertyOldId);
	if(!IS_STRING_EQUAL(pcsElemName, "defGradSplitKey") 
		&& !IS_STRING_EQUAL(pcsParentNameElemName, "defGradSecondaryColor")
		&& !IS_STRING_EQUAL(pcsElemName, "defGradSwapKeySidesButton") )
		return true;

	int iSelIdx;
	SlidingStopsSelectionType eSelTypeGeneric = getSelectionType(iSelIdx);
	UISliderStop *pStopIn = iSelIdx >= 0 ? myStops[iSelIdx] : NULL;
	GradientSelectionType eSelType = getGradientSelectionTypeFromGenericSelection(pStopIn, eSelTypeGeneric);
	const SGradientStop* pOrigStop = NULL;
	if(pStopIn)
		pOrigStop = myGradient.findStopById(pStopIn->myId);

	if(IS_STRING_EQUAL(pcsElemName, "defGradSplitKey"))
		return eSelType == GradientSelectionColor;
	else if(IS_STRING_EQUAL(pcsParentNameElemName, "defGradSecondaryColor")
		|| IS_STRING_EQUAL(pcsElemName, "defGradSwapKeySidesButton"))
		return eSelType == GradientSelectionColor && pOrigStop && pOrigStop->myIsSplit;

	return true;
}
/*****************************************************************************/
GradientSelectionType UIGradientEdit::getGradientSelectionTypeFromGenericSelection(UISliderStop* pStopIn, SlidingStopsSelectionType eSelTypeGeneric)
{
	GradientSelectionType eSelType= GradientSelectionNone;
	if(eSelTypeGeneric == SlidingStopsSelectionSingle)
	{
		if(pStopIn && as<UIGradientSliderStop>(pStopIn)->myIsOpacityStop)
			eSelType = GradientSelectionOpacity;
		else
			eSelType = GradientSelectionColor;

	}
	else if(eSelTypeGeneric == SlidingStopsSelectionNone)
		eSelType = GradientSelectionNone;
	else if(eSelTypeGeneric == SlidingStopsSelectionMultiple)
		eSelType = GradientSelectionMultiple;
	ELSE_ASSERT;

	return eSelType;
}
/*****************************************************************************/
};