/*****************************************************************************

Disclaimer: This software is supplied to you by Sad Cat Software
("Sad Cat") in consideration of your agreement to the following terms, and 
your use, installation, modification or redistribution of this Sad Cat software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Sad Cat software.

This software is provided "as is". Sad Cat Software makes no warranties, 
express or implied, including without limitation the implied warranties
of non-infringement, merchantability and fitness for a particular
purpose, regarding Sad Cat's software or its use and operation alone
or in combination with other hardware or software products.

In no event shall Sad Cat Software be liable for any special, indirect,
incidental, or consequential damages (including, but not limited to, 
procurement of substitute goods or services; loss of use, data, or profits;
or business interruption) arising in any way out of the use, reproduction,
modification and/or distribution of Sad Cat's software however caused and
whether under theory of contract, tort (including negligence), strict
liability or otherwise, even if Sad Cat Software has been advised of the
possibility of such damage.

Copyright (C) 2012, Sad Cat Software. All Rights Reserved.

*****************************************************************************/
#include "stdafx.h"

namespace HyperUI
{
string UIRoundSliderElement::theLocalSharedString;
string UIRoundSliderElement::theLocalSharedString2;

#define PROGRESS_ORIENTATION	OrientationCCW
#define START_ANGLE		0.0
#define ANGLE_SPAN		360.0
/*****************************************************************************/
UIRoundSliderElement::UIRoundSliderElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane), AnimSequenceAddon(PropertySecondaryImage)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIRoundSliderElement::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
	AnimSequenceAddon::onAllocated(PropertySecondaryImage);
	myProgress = 0.25;
	myLastScroll.set(0, 0);
	myIsInCallback = false;
	myCurrUndoBlockId = -1;
}
/*****************************************************************************/
void UIRoundSliderElement::postInit(void)
{
	UIElement::postInit();
}
/*****************************************************************************/
void UIRoundSliderElement::resetEvalCache(bool bRecursive)
{
	UIElement::resetEvalCache(bRecursive);
	AnimSequenceAddon::resetEvalCache();
}
/*****************************************************************************/
void UIRoundSliderElement::render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	preRender(svScroll, fOpacity, fScale);

	this->getFullTopAnimName(theLocalSharedString);
	this->getFullBaseAnimName(theLocalSharedString2);

	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity;	
	if(this->getParent())
		svPos *= fScale;

	SVector2D svCenter(svPos.x + svScroll.x, svPos.y + svScroll.y);
	FLOAT_TYPE fVeryFinalScale = fScale*fLocScale;

	int iRealW, iRealH;
	getTextureManager()->getTextureRealDims(theLocalSharedString.c_str(), iRealW, iRealH);
	FLOAT_TYPE fRealRad = iRealW/2.0;

	SVector2D svSize;
	getBoxSize(svSize);
	svSize *= fVeryFinalScale;

	myLastScroll = svScroll;

	// Render self here
	FLOAT_TYPE fRadius = getTextureManager()->getFileWidth(theLocalSharedString.c_str())/2.0*fVeryFinalScale;

	// Render the empty portion
	if(theLocalSharedString2.length() > 0)
		getDrawingCache()->addSprite(theLocalSharedString2.c_str(), svCenter.x, svCenter.y, fFinalOpac, 0, fVeryFinalScale, 1.0, true);
	getDrawingCache()->flush();

/*
	if(theLocalSharedString2.length() <= 0)
		RenderUtils::renderCircularProgress(1.0, theLocalSharedString.c_str(), svCenter.x, svCenter.y, fRadius, fFinalOpac*0.15, START_ANGLE, ANGLE_SPAN, true);
	else
		RenderUtils::renderCircularProgress(1.0, theLocalSharedString2.c_str(), svCenter.x, svCenter.y, fRadius, fFinalOpac, START_ANGLE, ANGLE_SPAN);
*/
	
	// Render the full portion
	RenderUtils::renderCircularProgress(getParentWindow(), myProgress, theLocalSharedString.c_str(), svCenter.x, svCenter.y, fRadius, fFinalOpac, START_ANGLE, ANGLE_SPAN, false, PROGRESS_ORIENTATION);


	// Figure out the knob position and draw it
	FLOAT_TYPE fAngle = myProgress*ANGLE_SPAN - START_ANGLE;
	fAngle = HyperCore::sanitizeDegAngle(fAngle);

	// This renders the optional knob that follows the slider on the current position
	if(this->doesPropertyExist(PropertyObjThirdAnim))
	{
		theLocalSharedString = this->getStringProp(PropertyObjThirdAnim);
		int iKnobRealW, iKnobRealH;
		getTextureManager()->getTextureRealDims(theLocalSharedString.c_str(), iKnobRealW, iKnobRealH);
		fRealRad -= iKnobRealW/2.0;

		SVector2D svKnobPos;
		svKnobPos.x = F_COS_DEG(fAngle)*fRealRad + svCenter.x;
		svKnobPos.y = F_SIN_DEG(fAngle)*fRealRad + svCenter.y;
		FLOAT_TYPE fNegAngle = fAngle;
		if(PROGRESS_ORIENTATION == OrientationCW)
			fNegAngle = HyperCore::sanitizeDegAngle(-fAngle);
		getDrawingCache()->addSprite(theLocalSharedString.c_str(), svKnobPos.x, svKnobPos.y, fFinalOpac, fNegAngle, fVeryFinalScale, 1.0, true);
	}

	// This renders the top cover. For now, it rotates.
	if(this->doesPropertyExist(PropertyCapImage))
	{
		FLOAT_TYPE fNegAngle = fAngle;
		if(PROGRESS_ORIENTATION == OrientationCW)
			fNegAngle = HyperCore::sanitizeDegAngle(-fAngle);
		theLocalSharedString = this->getStringProp(PropertyCapImage);
		getDrawingCache()->addSprite(theLocalSharedString.c_str(), svCenter.x, svCenter.y, fFinalOpac, fNegAngle, fVeryFinalScale, 1.0, true);
	}

	postRender(svScroll, fOpacity, fScale);

/*
#ifdef _DEBUG
	SColor scolDb(1,0,0,1);
	SRect2D srOwnRect;
	srOwnRect.setFromCenterAndSize(svCenter, svSize);
	getDrawingCache()->addRectangle(srOwnRect, scolDb, upToScreen(0.51));
#endif
*/

}
/*****************************************************************************/
void UIRoundSliderElement::onPressed(TTouchVector& vecTouches)
{
	UIElement::onPressed(vecTouches);
	FLOAT_TYPE fNewProg = getNewProgressFromGlobalPoint(vecTouches[0].myPoint);
	if(getIsEnabled() && vecTouches.size() > 0)
	{
		// Save undo...
		_ASSERT(myCurrUndoBlockId < 0);
		UIElement* pParmElem = getLinkedToElementWithValidTarget();
		if(pParmElem && UndoManager::canAcceptNewUndoNow())
			myCurrUndoBlockId = UndoManager::addUndoItemToCurrentManager(pParmElem->getUndoStringForSelfChange(), pParmElem->createUndoItemForSelfChange(), true, getParentWindow(), NULL);	

		getUIPlane()->lockMouseCursor(this);
		if(pParmElem)
			EventManager::getInstance()->sendEvent(EventUIParmChangeBegin, this);

		myProgress = fNewProg;
		UIElement::changeValueTo(myProgress*ANGLE_SPAN, NULL, false, false);
	}
}
/*****************************************************************************/
void UIRoundSliderElement::onMouseEnter(TTouchVector& vecTouches)
{
	UIElement::onMouseEnter(vecTouches);
	if(getIsEnabled())
	{
		if(this->getLinkedToElementWithValidTarget())
			EventManager::getInstance()->sendEvent(EventUIParmChangeBegin, this);
	}
}
/*****************************************************************************/
void UIRoundSliderElement::onMouseLeave(TTouchVector& vecTouches)
{
	UIElement::onMouseLeave(vecTouches);	

	if(getIsEnabled() && this->getLinkedToElementWithValidTarget())
		EventManager::getInstance()->sendEvent(EventUIParmChangeEnd, this);

	finishUndo();
}
/*****************************************************************************/
void UIRoundSliderElement::onMouseMove(TTouchVector& vecTouches)
{
	UIElement::onMouseMove(vecTouches);
	myProgress = getNewProgressFromGlobalPoint(vecTouches[0].myPoint);
	UIElement::changeValueTo(myProgress*ANGLE_SPAN, NULL, false, true);

	if(myCallbacks)
	{
		myIsInCallback = true;
//		myCallbacks->onSliderValueChanged(this);
		myIsInCallback = false;
	}
}
/*****************************************************************************/
void UIRoundSliderElement::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{
	UIElement::onReleased(vecTouches, bIgnoreActions);
	myProgress = getNewProgressFromGlobalPoint(vecTouches[0].myPoint);
	UIElement::changeValueTo(myProgress*ANGLE_SPAN, NULL, false, false);

	if(getIsEnabled())
	{
		getUIPlane()->unlockMouseCursor();
		if(this->getLinkedToElementWithValidTarget())
			EventManager::getInstance()->sendEvent(EventUIParmChangeEnd, this);
	}

	if(myCallbacks)
	{
		myIsInCallback = true;
		//		myCallbacks->onSliderValueChanged(this);
		myIsInCallback = false;
	}

	finishUndo();
}
/*****************************************************************************/
FLOAT_TYPE UIRoundSliderElement::getNewProgressFromGlobalPoint(const SVector2D& svPoint)
{
	SVector2D svPos;
	FLOAT_TYPE fLocScale;
	this->getLocalPosition(svPos, NULL, &fLocScale);

	SVector2D svCenter(svPos.x + myLastScroll.x, svPos.y + myLastScroll.y);

	// Find the vector and its angel
	SVector2D svToPoint;
	svToPoint = svPoint - svCenter;
	svToPoint.normalize();

	FLOAT_TYPE fAngle = svToPoint.getAngleFromPositiveXAccurate();
	FLOAT_TYPE fRes;
	FLOAT_TYPE fFromStart = (fAngle - START_ANGLE);
	if(fFromStart <= FLOAT_EPSILON && fFromStart >= -FLOAT_EPSILON)
		fFromStart = 0;
 	if(PROGRESS_ORIENTATION == OrientationCW && fFromStart > 0)
 		fFromStart -= 360.0;

	fRes = fabs(fFromStart)/ANGLE_SPAN;
	if(fRes < 0.0 || fRes > 1.0)
	{
		// See which limit it's closer to
		if(fabs(HyperCore::mantissa(fRes)) < 0.5)
			fRes = 1.0;
		else
			fRes = 0.0;
	}

	if(fRes < 0.0)
		fRes = 0.0;
	if(fRes > 1.0)
		fRes = 1.0;
	return fRes;
}
/*****************************************************************************/
void UIRoundSliderElement::setValue(FLOAT_TYPE fValue)
{
	if(myIsInCallback)
		return;

	setValueInternal(fValue, NULL, false, false);
}
/*****************************************************************************/
void UIRoundSliderElement::setValueInternal(FLOAT_TYPE fValue, UIElement* pOptSourceElem, bool bAnimate, bool bIsChangingContinuously)
{
	while(fValue < 0)
		fValue += ANGLE_SPAN;
	myProgress = fValue/ANGLE_SPAN;
	_ASSERT(myProgress >= 0 && myProgress <= 1.0);
	myProgress = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, myProgress);
	UIElement::changeValueTo(fValue, NULL, false, bIsChangingContinuously);
}
/*****************************************************************************/
void UIRoundSliderElement::changeValueTo(FLOAT_TYPE fValue, UIElement* pOptSourceElem, bool bAnimate, bool bIsChangingContinuously)
{
	if(myIsCallingChangeValue)
		return;

	// Note that will be called from the set value safe.
	//UIElement::changeValueTo(fValue);

	setValueInternal(fValue, pOptSourceElem, bAnimate, bIsChangingContinuously);
}
/*****************************************************************************/
void UIRoundSliderElement::finishUndo()
{
	if(myCurrUndoBlockId >= 0)
	{
		UndoManager::endUndoBlockInCurrentManager(myCurrUndoBlockId, getParentWindow(), NULL);
		myCurrUndoBlockId = -1;
	}
}
/*****************************************************************************/
};