#include "stdafx.h"

#define NUDGE_LARGE_STEP		10.0
#define NUDGE_SMALL_STEP		 1.0

#define GRAD_DRAG_UP_THRESHOLD					upToScreen(20.0)
#define GRAD_CLICK_THRESHOLD					upToScreen(3.0)

namespace HyperUI
{
/*****************************************************************************/
UISlidingStopsEdit::UISlidingStopsEdit(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
UISlidingStopsEdit::~UISlidingStopsEdit()
{
	onDeallocated();
}
/*****************************************************************************/
void UISlidingStopsEdit::onDeallocated()
{
	clearAllStops();
}
/*****************************************************************************/
void UISlidingStopsEdit::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
	myDidHaveSelectionOnPress = false;
	myWillRemoveSelected = false;
	myCurrUndoBlockId = -1;
	myIsRefreshingUI = false;
	myDidSendChagneBeginMessage = false;
}
/*****************************************************************************/
void UISlidingStopsEdit::deselectAllStops()
{
	int iCurr, iNum = myStops.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		myStops[iCurr]->setIsSelected(false);
}
/*****************************************************************************/
int UISlidingStopsEdit::getStopAt(const SVector2D& svPoint)
{
	SRect2D srStopRect;
	int iCurr, iNum = myStops.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		getStopRectangle(myStops[iCurr], srStopRect);
		if(srStopRect.doesContain(svPoint))
			return iCurr;
	}
	return -1;
}
/*****************************************************************************/
bool UISlidingStopsEdit::removeSelected()
{
	if(!getAllowStopDeletion())
		return false;

	bool bDidRemove = false;
	int iCurr, iNum = myStops.size();
	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
	{
		if(!myStops[iCurr]->getIsSelected())
			continue;

		removeStopCallback(myStops[iCurr]);
		delete myStops[iCurr];
		myStops.erase(myStops.begin() + iCurr);
		bDidRemove = true;
	}

	if(bDidRemove)
		refreshSelectedStopRelatedUI(false);
	return bDidRemove;
}
/*****************************************************************************/
void UISlidingStopsEdit::ensureUndoStarted()
{
	if(myCurrUndoBlockId >= 0)
		return;

	// Save undo...
	UIElement* pParmElem = getLinkedToElementWithValidTarget();
	if(pParmElem && UndoManager::canAcceptNewUndoNow())
	{
		myCurrUndoBlockId = UndoManager::addUndoItemToCurrentManager(pParmElem->getUndoStringForSelfChange(), pParmElem->createUndoItemForSelfChange(), true, getParentWindow(), NULL);	
		onUndoBegan();
	}
}
/*****************************************************************************/
void UISlidingStopsEdit::finishUndo()
{
	if(myCurrUndoBlockId >= 0)
	{
		// When we open the color picker, we will start undo in one window
		// and attempt to finish in the next - after which we will assert.
		UndoManager::endUndoBlockInCurrentManager(myCurrUndoBlockId, getParentWindow(), NULL);
		myCurrUndoBlockId = -1;
	}
}
/*****************************************************************************/
bool UISlidingStopsEdit::onKeyUp(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl)
{
	KeyMeaningType eKeyMeaning = KeyManager::getInstance()->mapKey(iKey, false, false, false);

	_ASSERT(myCurrUndoBlockId < 0);
	if(eKeyMeaning == KeyMeaningDelete)
	{
		ensureUndoStarted();

		bool bRes = removeSelected();
		if(bRes)
			handleTargetElementUpdate(false);

		finishUndo();

		return bRes;
	}

	if(eKeyMeaning != KeyMeaningLeft && eKeyMeaning != KeyMeaningRight && eKeyMeaning != KeyMeaningUp && eKeyMeaning != KeyMeaningDown)
		return false;

	bool bDidHandle = false;
	FLOAT_TYPE fMoveDist = 0.0;
	if(eKeyMeaning == KeyMeaningLeft || eKeyMeaning == KeyMeaningDown)
		fMoveDist = -1;
	else if(eKeyMeaning == KeyMeaningRight || eKeyMeaning == KeyMeaningUp)
		fMoveDist = 1;

	if(bShift)
		fMoveDist *= NUDGE_LARGE_STEP;
	else
		fMoveDist *= NUDGE_SMALL_STEP;

	// Go through the selection
	int iCurr, iNum = myStops.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(!myStops[iCurr]->getIsSelected())
			continue;

		// Otherwise, see what the key is
		bDidHandle |= nudgeStop(myStops[iCurr], eKeyMeaning, fMoveDist);
	}

	if(bDidHandle)
	{
		refreshSelectedStopRelatedUI(false);
		handleTargetElementUpdate(false);
	}

	finishUndo();

	return bDidHandle;
}
/*****************************************************************************/
void UISlidingStopsEdit::onPressed( TTouchVector& vecTouches )
{
	UIElement::onPressed(vecTouches);

	_ASSERT(myCurrUndoBlockId < 0);
	ensureUndoStarted();

	myDidSendChagneBeginMessage = false;

	int iDummy;
	myWillRemoveSelected = false;
	myDidHaveSelectionOnPress = (getSelectionType(iDummy) != SlidingStopsSelectionNone);
	myMouseDownPos = vecTouches[0].myPoint;

	bool bIsControlDown = KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyControl);
	bool bIsAltDown = KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyAlt);

	myPressedStopId = -1;
	int iStopIndex = getStopAt(vecTouches[0].myPoint);
	if(iStopIndex >= 0)
	{
		getUIPlane()->lockMouseCursor(this);

		myPressedStopId = myStops[iStopIndex]->myId;
		bool bIsClickedStopSelected = myStops[iStopIndex]->getIsSelected();

		if(!bIsClickedStopSelected)
		{
			if(!bIsControlDown)
				deselectAllStops();
			myStops[iStopIndex]->setIsSelected(true);
		}
		else
		{
			if(bIsControlDown)
				myStops[iStopIndex]->setIsSelected(false);
		}
	}
	else if(iStopIndex < 0)
	{
		// If the control isn't down, deselect all handles
		if(!bIsControlDown && !bIsAltDown)
			deselectAllStops();
	}
	refreshSelectedStopRelatedUI(false);
}
/*****************************************************************************/
bool UISlidingStopsEdit::getWillRemoveSelected(const SVector2D& svMousePos, bool bHaveSelection)
{
	return getAllowStopDeletion() && myPressedStopId >= 0 && bHaveSelection && fabs(svMousePos.y - myMouseDownPos.y) >= GRAD_DRAG_UP_THRESHOLD;
}
/*****************************************************************************/
void UISlidingStopsEdit::onMouseEnter( TTouchVector& vecTouches )
{
	UIElement::onMouseEnter(vecTouches);
}
/*****************************************************************************/
void UISlidingStopsEdit::onMouseLeave( TTouchVector& vecTouches )
{
	onReleasedCommon(vecTouches, false);

	UIElement::onMouseLeave(vecTouches);
	finishUndo();
}
/*****************************************************************************/
void UISlidingStopsEdit::onReleased( TTouchVector& vecTouches, bool bIgnoreActions )
{
	UIElement::onReleased(vecTouches, bIgnoreActions);
	onReleasedCommon(vecTouches, bIgnoreActions);
}
/*****************************************************************************/
void UISlidingStopsEdit::onReleasedCommon(TTouchVector& vecTouches, bool bIgnoreActions )
{
	getUIPlane()->unlockMouseCursor();

	int iCurr, iNum = myStops.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		myStops[iCurr]->myDragStartPos = -1;
	}

	bool bUpdateParm = false;
	int iStopIndex = getStopAt(vecTouches[0].myPoint);

	// If we have no selection, and had no selection, create a new entry.
	int iDummy;
	bool bHaveSelection = (getSelectionType(iDummy) != SlidingStopsSelectionNone);
	if(!bHaveSelection && !myDidHaveSelectionOnPress && iStopIndex < 0 && (myMouseDownPos - vecTouches[0].myPoint).length() < GRAD_CLICK_THRESHOLD 
		//&& vecTouches[0].myPoint.y < (myGradientRect.y + myGradientRect.h + HANDLE_VERTICAL_SPACE) 
		&& getIsPointInAdditionRegion(vecTouches[0].myPoint) )
	{
		// Add a new point
		// Get the position where we clicked
		FLOAT_TYPE fPos = (vecTouches[0].myPoint.x - getGradAreaStartX())/getGradAreaWidth();
		if(fPos >= -0.10 && fPos <= 1.10)
		{
			fPos = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, fPos);

			if(getAllowStopAddition())
			{
				addNewStop(fPos, vecTouches[0].myPoint);
				bUpdateParm = true;
				refreshSelectedStopRelatedUI(false);
			}
		}
	}
	
	if(getWillRemoveSelected(vecTouches[0].myPoint, bHaveSelection))
	{
		// Remove all selected thingies.
		if(removeSelected())
			bUpdateParm = true;
	}

	if(bUpdateParm)
		handleTargetElementUpdate(false);

	myWillRemoveSelected = false;
	finishUndo();

	if(myDidSendChagneBeginMessage)
		EventManager::getInstance()->sendEvent(EventUIParmChangeEnd, this);
}
/*****************************************************************************/
SlidingStopsSelectionType UISlidingStopsEdit::getSelectionType(int& iSingleSelIdxOut)
{
	int iSelCounter = 0;
	iSingleSelIdxOut = -1;
	int iCurr, iNum = myStops.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(myStops[iCurr]->getIsSelected())
		{
			iSelCounter++;
			if(iSelCounter > 1)
			{
				iSingleSelIdxOut = -1;
				return SlidingStopsSelectionMultiple;
			}
			iSingleSelIdxOut = iCurr;
		}
	}

	if(iSelCounter == 0)
		return SlidingStopsSelectionNone;
	else
		return SlidingStopsSelectionSingle;
/*
	{
		_ASSERT(iSingleSelIdxOut >= 0);
		if(as<UIGradientSliderStop>(myStops[iSingleSelIdxOut])->myIsOpacityStop)
			return GradientSelectionOpacity;
		else
			return GradientSelectionColor;
	}
*/
}
/*****************************************************************************/
void UISlidingStopsEdit::getStopRectangle(UISliderStop* pUiStop, SRect2D& srOut)
{
	SVector2D svGradStopSize(upToScreen(20.0), HANDLE_VERTICAL_SPACE);

	// Find the corresponding thingie in our gradient
	srOut.set(0, 0, 0, 0);

	// Get the center
	SVector2D svCenter;
	getStopCenter(pUiStop, svGradStopSize, svCenter);

	// Now, make a rectangle
	srOut.setFromCenterAndSize(svCenter, svGradStopSize);
}
/*****************************************************************************/
void UISlidingStopsEdit::onMouseMove(TTouchVector& vecTouches)
{
	UIElement::onMouseMove(vecTouches);

	if(getGradAreaWidth() == 0)
		return;

	SVector2D svMousePos = vecTouches[0].myPoint;
	SVector2D svDiff = svMousePos - myMouseDownPos;

	FLOAT_TYPE fPosDiff = svDiff.x/getGradAreaWidth();

	// Go through all the selected handles and move them.
	bool bDidMove = false;
	int iCurr, iNum = myStops.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(!myStops[iCurr]->getIsSelected())
			continue;

		if(!myDidSendChagneBeginMessage && this->getLinkedToElementWithValidTarget())
		{
			myDidSendChagneBeginMessage = true;
			EventManager::getInstance()->sendEvent(EventUIParmChangeBegin, this);
		}

		if(myStops[iCurr]->myDragStartPos < 0)
			myStops[iCurr]->myDragStartPos = getStopPos(myStops[iCurr]);

		moveStopTo(myStops[iCurr], myStops[iCurr]->myDragStartPos + fPosDiff);

		bDidMove = true;
	}

	if(bDidMove)
	{
		refreshSelectedStopRelatedUI(true);
		handleTargetElementUpdate(true);

		// If we have a linked elem and a pressed stop id, send a start parm
		// change messge
		if(myDidSendChagneBeginMessage)
		{
			IBaseObject* pTarget = NULL;
			IGenericIdentifier* pIdentifier = this->getTargetIdentifier();
			if(pIdentifier)
				pTarget = pIdentifier->getIdentifierTarget();
			EventManager::getInstance()->sendEvent(EventUIParmChanging, pTarget);
		}
	}

	myWillRemoveSelected = getWillRemoveSelected(vecTouches[0].myPoint, bDidMove);
}
/*****************************************************************************/
void UISlidingStopsEdit::postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	SRect2D srOwnRect;
	getGlobalRectangle(srOwnRect);


	// Render the stops
	SRect2D srStopRect;
	SRect2D srColorRect;
	SColor scolTemp;
	SColor scolText(0.8, 0.8, 0.8, fOpacity);
	srColorRect.w = srColorRect.h = upToScreen(8.0);
	SVector2D svStopCenter;
	FLOAT_TYPE fFinalOpacity;

	SVector2D svInnerTextOffset;
	getInnerTextOffset(svInnerTextOffset);

	int iCurr, iNum = myStops.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		// Get the rectangle
		getStopRectangle(myStops[iCurr], srStopRect);
		srStopRect.getCenter(svStopCenter);

		fFinalOpacity = fOpacity;
		if(myWillRemoveSelected && myStops[iCurr]->getIsSelected())
			fFinalOpacity *= 0.45;

		// Draw the stop and its color
		if(!myStops[iCurr]->getIsSelected())
			myStops[iCurr]->getDeselectedAnim(theCommonString);
		else
			myStops[iCurr]->getSelectedAnim(theCommonString);
		getDrawingCache()->addSprite(theCommonString.c_str(), svStopCenter.x, svStopCenter.y, fFinalOpacity, 0, fScale, 1.0, true);

		// Draw the actual value
		if(getStopText(myStops[iCurr], theCommonString))
		{
			scolText.alpha = fFinalOpacity;
			getDrawingCache()->addText(theCommonString.c_str(), MAIN_FONT_TINY, MAIN_FONT_TINY_DEF_SIZE, svStopCenter.x + svInnerTextOffset.x, svStopCenter.y + svInnerTextOffset.y, scolText);
		}
		else
			drawCustomStopInnerContents(myStops[iCurr], svStopCenter, fFinalOpacity);
	}
	UIElement::postRender(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
bool UISlidingStopsEdit::nudgeStop(UISliderStop* pStop, KeyMeaningType eKeyMeaning, FLOAT_TYPE fMoveDist)
{
	bool bDidHandle = false;
	// 	UIGradientSliderStop* pCastStop = as<UIGradientSliderStop>(pStop);
	// 	const SGradientStop* pOrigStop = myGradient.findStopById(pStop->myId);
	if(eKeyMeaning == KeyMeaningLeft || eKeyMeaning == KeyMeaningRight)
	{
		// Move the stops side ways.
		ensureUndoStarted();
		moveStopTo(pStop, getStopPos(pStop) + fMoveDist/100.0);
		///myGradient.moveStopTo(pCastStop->myId, pOrigStop->getPos() + fMoveDist/100.0);
		bDidHandle = true;
	}
	else if((eKeyMeaning == KeyMeaningUp || eKeyMeaning == KeyMeaningDown)) //  && pCastStop->myIsOpacityStop)
	{
		// Change opacity
		ensureUndoStarted();
		if(nudgeStopValue(pStop, fMoveDist/100.0))
		{
			// 		myGradient.changeStopOpacityTo(pCastStop->myId, pOrigStop->myColor.alpha + fMoveDist/100.0);
			bDidHandle = true;
		}
	}

	return bDidHandle;
}
/*****************************************************************************/
void UISlidingStopsEdit::refreshSelectedStopRelatedUI(bool bIsChangingContinuously)
{
	myIsRefreshingUI = true;
	int iSelIdx;
	SlidingStopsSelectionType eSelTypeGeneric = getSelectionType(iSelIdx);

	refreshStopRelatedUI(iSelIdx >= 0 ? myStops[iSelIdx] : NULL, eSelTypeGeneric, bIsChangingContinuously);

	myIsRefreshingUI = false;
}
/*****************************************************************************/
void UISlidingStopsEdit::updateOwnData(SUpdateInfo& rRefreshInfo)
{
	// See if we have a target element, and if so, set our value from it.
	// The reason we have a second clause is because while the parm is dragged,
	// we get this call; however, we never move, since we always refresh
	// at the initial position. This is a hack, admittedly, but a needed one
	// in order not to reset the scroll bar when doing undo on these controls.
	if(getHaveValidTargetElement() && (rRefreshInfo.mySourceEventType == EventUndoPerformed || rRefreshInfo.mySourceEventType == EventRedoPerformed))
		getTargetIdentifier()->refreshUIFromStoredValue(this);

	refreshSelectedStopRelatedUI(false);
}
/*****************************************************************************/
void UISlidingStopsEdit::onDoubleClick(TTouchVector& vecTouches, bool bIgnoreActions)
{
	if(bIgnoreActions)
		return;

	int iStopIndex = getStopAt(vecTouches[0].myPoint);
	if(iStopIndex >= 0)
		onStopDoubleClicked(myStops[iStopIndex], vecTouches);
}
/*****************************************************************************/
};