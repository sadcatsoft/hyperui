#include "stdafx.h"

#define MIN_TOUCH_DIST				upToScreen(10)

#define ZOOM_SPEED_DECAY_MULT			2.0

///#define ZOOM_ANIM_TIME					0.5
#define ZOOM_ANIM_TIME					0.1
#define PAN_ANIM_TIME					0.1
#define STATIC_PAN_SNAPBACK_TIME		0.15

namespace HyperUI
{
/*****************************************************************************/
UIZoomWindowElement::UIZoomWindowElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
UIZoomWindowElement::~UIZoomWindowElement() 
{

}
/*****************************************************************************/
void UIZoomWindowElement::onAllocated(IBaseObject* pData)
{
	myIgnoreSlider = false;
	UIElement::onAllocated(pData);
	myZoom.setNonAnimValue(1.0);
	getWindowScrollPadding(myPan);
	myAnimatedPan.setNonAnimValue(0, 0);

	myScrollSpeed = 0;

//	myPan.x += 150;
//	myPan.y += 150;
	
	myInitZoom = -1;

	myActiveTouches = 0;
	myHasScrolled = false;

	myMouseTrail.setFadeSpeed(0);
	myMouseTrail.setTrailFreqMult(0);
	myMouseTrail.setMaxPoints(128);

}
/*****************************************************************************/
void UIZoomWindowElement::render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
#ifdef _DEBUG
	// TODO: We need to make sure this element renders the outline/fill as the rest of them...
	//UIElement::render(svScroll, fOpacity, fScale);
#endif
	preRender(svScroll, fOpacity, fScale);

	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity;

	if(this->getParent())
		svPos *= fLocScale;
/*
	FLOAT_TYPE fSelfW = this->getNumProp(PropertyObjSizeW)*fScale*fLocScale;
	FLOAT_TYPE fSelfH = this->getNumProp(PropertyObjSizeH)*fScale*fLocScale;

	SRect2D srWindowRect;
	srWindowRect.x = svPos.x + svScroll.x - fSelfW/2.0;
	srWindowRect.y = svPos.y + svScroll.y - fSelfH/2.0;
	srWindowRect.w = fSelfW;
	srWindowRect.h = fSelfH;
*/

	SRect2D srWindowRect;
	//this->getLocalRectangle(svScroll, fScale, srWindowRect);
	this->getGlobalRectangle(srWindowRect);

	// Set the stencil mask, render children, unset it.
/*
	bool bDidBeginStencil = false;
	if(RenderUtils::isInsideStencil() == false)
	{
		RenderUtils::beginStencilRectangle(getDrawingCache(), srWindowRect.x, srWindowRect.y, srWindowRect.w, srWindowRect.h);
		bDidBeginStencil = true;
	}
*/
	RenderUtils::beginScissorRectangle(srWindowRect.x, srWindowRect.y, srWindowRect.w, srWindowRect.h, getDrawingCache());

	renderZoomedContents(srWindowRect, fFinalOpac, fScale*fLocScale);
	if(this->getRenderChildrenInStencilRegion())
		renderChildren(svScroll, fFinalOpac, fScale*fLocScale);

// 	if(bDidBeginStencil)
// 		RenderUtils::endStencil(getDrawingCache());
	RenderUtils::endScissorRectangle(getDrawingCache());

	if(!this->getRenderChildrenInStencilRegion())
		renderChildren(svScroll, fFinalOpac, fScale);

	postRender(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
FLOAT_TYPE UIZoomWindowElement::getZoom(void) const
{
	GTIME lTime = Application::getInstance()->getGlobalTime(this->getClockType());
	return myZoom.getValue();
}
/*****************************************************************************/
void UIZoomWindowElement::onPressed(TTouchVector& vecTouches)
{
	myInitTouchDistance = 0;
	myInitZoom = -1;
	myHasScrolled = false;
	myInitPan.x = FLOAT_TYPE_MAX;
	myMouseTrail.clear();
	UIElement::onPressed(vecTouches);
}
/*****************************************************************************/
void UIZoomWindowElement::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{
	UIElement::onReleased(vecTouches, bIgnoreActions);
	if(myHasScrolled)
		animateScrollIfNeeded(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y);
	endTouch(vecTouches);

	myInitZoom = -1;
	myInitTouchDistance = 0;

}
/*****************************************************************************/
void UIZoomWindowElement::onMouseLeave(TTouchVector& vecTouches)
{
	if(myHasScrolled && getIsMousePressed())
		animateScrollIfNeeded(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y);

	UIElement::onMouseLeave(vecTouches);
	endTouch(vecTouches);
}
/*****************************************************************************/
void UIZoomWindowElement::onMouseMove(TTouchVector& vecTouches)
{
	UIElement::onMouseMove(vecTouches);

	// To zoom, we do it really simple: if we are pressed and there
	// are two touches exactly in the array, we zoom.
	bool bIsPressed = getIsMousePressed();
	if(bIsPressed && vecTouches.size() == 2)
	{
		SVector2D svPos;
		
		FLOAT_TYPE fSelfW = this->getNumProp(PropertyWidth);
		FLOAT_TYPE fSelfH = this->getNumProp(PropertyHeight);
		FLOAT_TYPE fFinalOpac, fGlobScale;
		this->getGlobalPosition(svPos, &fFinalOpac, &fGlobScale);		
		svPos.x -= fSelfW/2.0*fGlobScale;
		svPos.y -= fSelfH/2.0*fGlobScale;
		
		FLOAT_TYPE fCurrTouchDist = (vecTouches[0].myPoint - vecTouches[1].myPoint).length();
		if(fCurrTouchDist < MIN_TOUCH_DIST)
			fCurrTouchDist = MIN_TOUCH_DIST;
		if(myInitZoom <= 0)
		{
			// We haven't started zooming yet.
			myInitZoom = getZoom();
			myInitTouchDistance = fCurrTouchDist;
			myInitPan = myPan;
			myAnimatedPan.setNonAnimValue(0, 0);

			// Find the midpoint - this is where we'll be zooming
			myZoomMidpoint = (vecTouches[0].myPoint + vecTouches[1].myPoint)*0.5;
			myZoomMidpoint -= svPos;
			screenToWindow(myZoomMidpoint, myInitMidpointReal);
			/*
			gLog("-------------- New Zoom ------------\nInit Dist = %f\n", fCurrTouchDist);
			gLog("T0 = %f, %f\n", vecTouches[0].x, vecTouches[0].y);
			gLog("T1 = %f, %f\n", vecTouches[1].x, vecTouches[1].y);
			gLog("Init Pan: %f, %f\n", myPan.x, myPan.y);
			gLog("Zoomed mid point: %f %f\n", myZoomMidpoint.x, myZoomMidpoint.y);
			gLog("Init mid point: %f %f\n", myInitMidpointReal.x, myInitMidpointReal.y);
			 */
		}
		else
		{
			//SVector2D svNewMidPointScreen;

			// See where the old point is now
			FLOAT_TYPE fTempZoom;			
			fTempZoom = myInitZoom*fCurrTouchDist/myInitTouchDistance;
			if(fTempZoom < getMinZoom())
				fTempZoom = getMinZoom();
			if(fTempZoom > getMaxZoom())
				fTempZoom = getMaxZoom();
			myZoom.setNonAnimValue(fTempZoom);
			onZoomSet(fTempZoom);
			
			SVector2D svSelfCenter(fSelfW/2.0, fSelfH/2.0);
			
			SVector2D svOffset;
			svOffset = (myZoomMidpoint - svSelfCenter)/fTempZoom;

			SVector2D svZoomPadding;
			getWindowScrollPadding(svZoomPadding);
			if(getAllowScrollingX())
				myPan.x = myInitMidpointReal.x + svZoomPadding.x - fSelfW/(2.0*fTempZoom) - svOffset.x;
			if(getAllowScrollingY())
				myPan.y = myInitMidpointReal.y + svZoomPadding.y - fSelfH/(2.0*fTempZoom) - svOffset.y;
			myAnimatedPan.setNonAnimValue(0, 0); 
		}
	}

	// Alternatively, if we've got one finger, see if we're panning
	if(bIsPressed && vecTouches.size() == 1 && myInitZoom < 0.0)
	{
		GTIME lTime = Application::getInstance()->getGlobalTime(ClockUiPrimary);
		myMouseTrail.addPoint(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y, lTime, true);

		if(myInitPan.x == FLOAT_TYPE_MAX)
		{
			// Just beginning to pan
			myInitPan = myPan;
			myInitPos = vecTouches[0].myPoint;
		}
		else
		{
			FLOAT_TYPE fPixelTolerance = getScrollPixelTolerance();
			FLOAT_TYPE svDistSq;
			svDistSq = (vecTouches[0].myPoint - myInitPos).lengthSquared();
			if(svDistSq > fPixelTolerance*fPixelTolerance)
			{
				myHasScrolled = true;
				SVector2D svDelta;
				svDelta = myInitPos - vecTouches[0].myPoint;
				///SVector2D svNewPan = myInitPan + svDelta/getZoom();
				SVector2D svNewPan = myInitPan + svDelta;
				setPan(svNewPan);
			}
		}
	}

}
/*****************************************************************************/
void UIZoomWindowElement::endTouch(TTouchVector& vecTouches)
{
	myInitPan.x = FLOAT_TYPE_MAX;
}
/*****************************************************************************/
void UIZoomWindowElement::windowToScreen(const SVector2D& svIn, SVector2D& svOut, FLOAT_TYPE fExtraWindowScale) const
{
	SVector2D svBorders;
	getWindowScrollPadding(svBorders);
	SVector2D svDummy(svIn);
	//svOut = (svDummy - myPan + svBorders)*getZoom()*fExtraWindowScale;
	svOut = svDummy*getZoom()*fExtraWindowScale - myPan + svBorders + myExtraZoomIndepOffset;
}
/*****************************************************************************/
void UIZoomWindowElement::windowToScreen(const SRect2D& srIn, SRect2D& srOut, FLOAT_TYPE fExtraWindowScale) const
{
	SVector2D svOrigin = srIn.origin();
	SVector2D svEnd = svOrigin + srIn.size();
	windowToScreen(svOrigin, svOrigin, fExtraWindowScale);
	windowToScreen(svEnd, svEnd, fExtraWindowScale);
	srOut.fromTwoPoints(svOrigin, svEnd);
}
/*****************************************************************************/
void UIZoomWindowElement::screenToWindow(const SVector2D& svIn, SVector2D& svOut) const
{
	SVector2D svBorders;
	getWindowScrollPadding(svBorders);
	SVector2D svDummy(svIn);
	//svOut = (svDummy)/getZoom() + myPan - svBorders;
	svOut = (svDummy + myPan - svBorders - myExtraZoomIndepOffset)/getZoom();
}
/*****************************************************************************/
void UIZoomWindowElement::scrollOnSlider(UISliderElement* pSlider)
{
	SVector2D svPan;
	getPan(svPan);
	SVector2D svMaxPan;
	getMaxPan(svMaxPan);

	FLOAT_TYPE fMaxValue;
	if(pSlider->getIsVertical())
		fMaxValue = svMaxPan.y;
	else
		fMaxValue = svMaxPan.x;

	SVector2D svBorders;
	getWindowScrollPadding(svBorders);

	if(pSlider->getIsVertical())
		svPan.y = pSlider->getValue()*(fMaxValue - svBorders.y*2.0) + svBorders.y;
	else
		svPan.x = pSlider->getValue()*(fMaxValue - svBorders.x*2.0) + svBorders.x;

	bool bPrevIgnoreSlider = myIgnoreSlider;
	myIgnoreSlider = true;
	setPan(svPan);
	myIgnoreSlider = bPrevIgnoreSlider;
}
/*****************************************************************************
void UIZoomWindowElement::refreshData(SRefreshInfo& rRefreshInfo)
{
	UIElement::refreshData(rRefreshInfo);
	updateRelatedSlider();
}
/*****************************************************************************/
void UIZoomWindowElement::animateScrollIfNeeded(FLOAT_TYPE fX, FLOAT_TYPE fY)
{
	GTIME lTime = Application::getInstance()->getGlobalTime(ClockUiPrimary);

	// Find where the mouse was a little while ago
	int iCurrPoint, iNumPoints = myMouseTrail.getNumPoints();
	for(iCurrPoint = iNumPoints - 1; iCurrPoint >= 0 ; iCurrPoint--)
	{
		if( myMouseTrail.getPointTime(iCurrPoint) <= lTime - SCROLLER_LOOKBACK_TICKS )
			break;
	}

	myScrollSpeed = 0.0;

	if( !(iCurrPoint == iNumPoints - 1) && !(iCurrPoint < 0) )
	{
		SVector2D svPoint;
		myMouseTrail.getPoint(iCurrPoint, svPoint);

		SVector2D svLast(fX, fY);
		mySpeedDir = (svLast - svPoint)*-1;
		
		GTIME lTimeDiff = (lTime - myMouseTrail.getPointTime(iCurrPoint));
		if(lTimeDiff != 0)
			myScrollSpeed = mySpeedDir.normalize()/lTimeDiff;
	}
	
	if(fabs(myScrollSpeed) < FLOAT_EPSILON && !getIsPanningLimitless())
	{
		// If we haven't swiped, see if we're simply outside of the normal pan
		SVector2D svZoomedPadding;
		getWindowScrollPadding(svZoomedPadding);
		svZoomedPadding *= getZoom();

		SVector2D svMaxPan, svRealContMax;
		getMaxPan(svMaxPan);
		svRealContMax = svMaxPan;
		svRealContMax.x -= svZoomedPadding.x;
		svRealContMax.y -= svZoomedPadding.y;

		if(myPan.x < svZoomedPadding.x || myPan.y < svZoomedPadding.y)
		{
//			SVector2D svRef(fZoomedPadding, fZoomedPadding);
//			mySpeedDir = svRef - myPan;
			//myScrollSpeed = mySpeedDir.normalize()/Application::secondsToTicks(STATIC_PAN_SNAPBACK_TIME);
			myScrollSpeed = 0.1;
			mySpeedDir.set(0, 0);
			//myScrollSpeed = mySpeedDir.normalize()*(SCROLL_SPEED_DECAY*ZOOM_SPEED_DECAY_MULT);
		}
		else if(myPan.x > svRealContMax.x || myPan.y > svRealContMax.y)
		{
//			mySpeedDir = svRealContMax - myPan;
//			myScrollSpeed = mySpeedDir.normalize()/Application::secondsToTicks(STATIC_PAN_SNAPBACK_TIME);

			myScrollSpeed = 0.1;
			mySpeedDir.set(0, 0);
		}

	}
}
/*****************************************************************************/
void UIZoomWindowElement::setPanInternal(SVector2D& svNewPan, bool bResetAnimation)
{

	// Do not uncomment this, since we rely on setting the same 
	// pan when zoom has changed.
// 	if((myPan - svNewPan).lengthSquared() < FLOAT_EPSILON)
// 		return; 

	if(getAllowScrollingX())
		myPan.x = svNewPan.x;
	if(getAllowScrollingY())
		myPan.y = svNewPan.y;
	//myPan = svNewPan;

	if(bResetAnimation)
		myAnimatedPan.setNonAnimValue(0, 0);

// 	FLOAT_TYPE fZoom = getZoom();
// 	gLog("CURRZOOM: %f\n", fZoom);
// 	gLog("SETPAN Extra Indep Offset: %f, %f\n", myExtraZoomIndepOffset.x, myExtraZoomIndepOffset.y);
// 	gLog("SETPAN New Pan: %f, %f\n", myPan.x, myPan.y);

	if(!getIsPanningLimitless())
		clampPanToValidRange(myPan, true);

	onPanSet(myPan);
//	gLog("SETPAN Actual new Pan: %f, %f\n", myPan.x, myPan.y);

	if(!myIgnoreSlider)
		updateRelatedSlider();

}
/*****************************************************************************/
void UIZoomWindowElement::setPan(SVector2D& svNewPan)
{
	setPanInternal(svNewPan, true);
}
/*****************************************************************************/
void UIZoomWindowElement::getMaxPan(SVector2D& svOut, bool bIncludePadding)
{
	FLOAT_TYPE fZoom = 0.0; 

	this->getContentSize(svOut);

// 	if(getIsAnimatingZoom())
// 		fZoom = max(myZoom.getStartValue(), myZoom.getEndValue());
// 	else
		fZoom = getZoom();
	svOut *= fZoom;

	SVector2D svZoomedPadding;
	if(bIncludePadding)
		getWindowScrollPadding(svZoomedPadding); 
	svZoomedPadding *= 2.0;

	svOut += svZoomedPadding;

	//gLog("WND SIZE: %f, %f\n", this->getNumProp(PropertyObjSizeW), this->getNumProp(PropertyObjSizeH));
	SVector2D svOwnSize;
	getBoxSize(svOwnSize);
	svOut -= svOwnSize;
// 	svOut.x -= this->getNumProp(PropertyObjSizeW);
// 	svOut.y -= this->getNumProp(PropertyObjSizeH);
	if(svOut.x < svZoomedPadding.x)
		svOut.x = svZoomedPadding.x;
	if(svOut.y < svZoomedPadding.y)
		svOut.y = svZoomedPadding.y;

	///gLog("MAXPAN Max Allowed Pan: %f, %f\n", svOut.x, svOut.y);
}
/*****************************************************************************/
void UIZoomWindowElement::onTimerTick(GTIME lGlobalTime)
{
	bool bDidZoomEndAnimationThisTick = (myZoom.getEndTime() == lGlobalTime);
	bool bDidPanEndAnimationThisTick = (myAnimatedPan.getX().getEndTime() == lGlobalTime);

	bool bIgnoreSliderLocal = false;
	if(getRelatedSlider(NULL))
	{
		// See if we're animating the slider
		if(getRelatedSlider(NULL)->getIsSliderBeingAnimated())
			bIgnoreSliderLocal = true;
	}

	//myZoom.checkTime(lGlobalTime);
	//myAnimatedPan.checkTime(lGlobalTime);
	if(myAnimatedPan.getIsAnimating(false) || bDidPanEndAnimationThisTick)
	{
		SVector2D svNewPan;
		myAnimatedPan.getValue(Application::getInstance()->getGlobalTime(ClockUiPrimary), svNewPan);
		if(bIgnoreSliderLocal)
			myIgnoreSlider = true;
		setPanInternal(svNewPan, false);
		if(bIgnoreSliderLocal)
			myIgnoreSlider = false;
	}
	else if(myZoom.getIsAnimating(false) || bDidZoomEndAnimationThisTick)
	{
		// Reset pan since it may have to be clamped against
		// the new max pan.
		if(bIgnoreSliderLocal)
			myIgnoreSlider = true;
		setPanInternal(myPan, false);
		if(bIgnoreSliderLocal)
			myIgnoreSlider = false;
	}
	UIElement::onTimerTick(lGlobalTime);

	if(bDidZoomEndAnimationThisTick)
		onZoomAnimationEnded();

	if(fabs(myScrollSpeed) > FLOAT_EPSILON)
		myScrollSpeed *= (1.0 - SCROLL_SPEED_DECAY*ZOOM_SPEED_DECAY_MULT);

	SVector2D fAdditionalScroll(0,0);

	if(!getIsMousePressed() && fabs(myScrollSpeed) > FLOAT_EPSILON && !getIsPanningLimitless())
	{
		// Need to find the distance to the content rectangle
		SVector2D svMaxPan;
		getMaxPan(svMaxPan);

		SVector2D svZoomPadding;
		getWindowScrollPadding(svZoomPadding); 
		
		if(myPan.x < svZoomPadding.x)
			fAdditionalScroll.x = ((FLOAT_TYPE)(svZoomPadding.x - myPan.x))/PADDING_SCROLL_RETURN_RATE;
		if(myPan.y < svZoomPadding.y)
			fAdditionalScroll.y = ((FLOAT_TYPE)(svZoomPadding.y - myPan.y))/PADDING_SCROLL_RETURN_RATE;
		if(myPan.x > svMaxPan.x - svZoomPadding.x)
			fAdditionalScroll.x = ((FLOAT_TYPE)(svMaxPan.x - svZoomPadding.x - myPan.x))/PADDING_SCROLL_RETURN_RATE;
		if(myPan.y > svMaxPan.y - svZoomPadding.y)
			fAdditionalScroll.y = ((FLOAT_TYPE)(svMaxPan.y - svZoomPadding.y - myPan.y))/PADDING_SCROLL_RETURN_RATE;

		SVector2D svNewPan;
		svNewPan = myPan + (mySpeedDir*myScrollSpeed)/getZoom() + fAdditionalScroll;
		if(bIgnoreSliderLocal)
			myIgnoreSlider = true;
		setPan(svNewPan);
		if(bIgnoreSliderLocal)
			myIgnoreSlider = false;

	}
	else if(fabs(myScrollSpeed) <= FLT_EPSILON)
	{
		myScrollSpeed = 0;
	}
}
/*****************************************************************************/
void UIZoomWindowElement::updateRelatedSlider()
{
	if(!this->getIsVisible())
		return;

	UISliderElement* pSlider = getRelatedSlider(NULL);
	if(!pSlider)
		return;

	//if(pSlider && pSlider->getIsEnabled())
	{
		SVector2D svMaxPan;
		getMaxPan(svMaxPan);
		SVector2D svPan;
		getPan(svPan);

		SVector2D svBorders;
		getWindowScrollPadding(svBorders);

		SVector2D svInterpSliderValues;
		if(svMaxPan.x > svBorders.x*2)
			svInterpSliderValues.x = (myPan.x - svBorders.x)/(svMaxPan.x - svBorders.x*2);
		if(svMaxPan.y > svBorders.y*2)
			svInterpSliderValues.y = (myPan.y - svBorders.y)/(svMaxPan.y - svBorders.y*2);

		svInterpSliderValues.x = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, svInterpSliderValues.x);
		svInterpSliderValues.y = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, svInterpSliderValues.y);

		bool bIsEnabled;
		if(pSlider->getIsVertical())
		{
			pSlider->setValue(svInterpSliderValues.y);
			bIsEnabled = (svMaxPan.y - svBorders.y*2.0) > 0;
		}
		else
		{
			pSlider->setValue(svInterpSliderValues.x);
			bIsEnabled = (svMaxPan.x - svBorders.x*2.0) > 0;
		}

		// Enable or disable. Or hide altogether.
		pSlider->setIsVisible(bIsEnabled);
	}
}
/*****************************************************************************/
void UIZoomWindowElement::onStartShowing(void)
{
	UIElement::onStartShowing();
//	myZoom = 1.0;
	myZoom.setNonAnimValue(1.0);
	getWindowScrollPadding(myPan);
	updateRelatedSlider();
}
/*****************************************************************************/
void UIZoomWindowElement::animateZoomTo(FLOAT_TYPE fTargetZoom)
{
	FLOAT_TYPE fCurrZoom = getZoom();
	if(fTargetZoom < getMinZoom())
		fTargetZoom = getMinZoom();
	if(fTargetZoom > getMaxZoom())
		fTargetZoom = getMaxZoom();

	if(fabs(fTargetZoom - fCurrZoom) < FLOAT_EPSILON)
		return;

	// Figure out the distance between these, then the time to animate.
	// Time to animate 1.0 zoom distance
	//FLOAT_TYPE fTime = fabs(fTargetZoom - fCurrZoom)*ZOOM_ANIM_TIME;
	// For now, just do flat time.
	FLOAT_TYPE fTime = ZOOM_ANIM_TIME;
	myZoom.setAnimation(fCurrZoom, fTargetZoom, fTime, this->getClockType());

	// Reset pan since it may have become obsolete
	setPanInternal(myPan, false);

	onZoomSet(fTargetZoom);
}
/*****************************************************************************/
void UIZoomWindowElement::setZoom(FLOAT_TYPE fZoom)
{
//	myZoom = fZoom;
	if(fZoom < getMinZoom())
		fZoom = getMinZoom();
	if(fZoom > getMaxZoom())
		fZoom = getMaxZoom();
	myZoom.setNonAnimValue(fZoom);

	// Reset pan since it may have become obsolete
	setPanInternal(myPan, false);

	onZoomSet(fZoom);
}
/*****************************************************************************/
void UIZoomWindowElement::panTo(SVector2D& svTargetPan, bool bAnimated, int iEndAction)
{
	// Optionally do it with an animation.
	if(bAnimated)
	{
// 		GTIME lTime = Application::getInstance()->getGlobalTime(this->getClockType());
// 		GTIME lEndTime = lTime + Application::secondsToTicks(PAN_ANIM_TIME);

		AnimationOverCallback* pCallback = NULL;
		if(iEndAction != AnimOverActionNone)
			pCallback = this;

		myAnimatedPan.setAnimationForX(myPan.x, svTargetPan.x, PAN_ANIM_TIME, getClockType(), iEndAction,  pCallback);
		myAnimatedPan.setAnimationForY(myPan.y, svTargetPan.y, PAN_ANIM_TIME, getClockType());
	}
	else
		setPan(svTargetPan);
}
/*****************************************************************************/
FLOAT_TYPE UIZoomWindowElement::getScrollPixelTolerance(void)
{
	// We want a much smaller tolerance for scrolling so that we do it smoothly.
	return upToScreen(4.0);
}
/*****************************************************************************/
bool UIZoomWindowElement::getIsAtLeftPanLimit()
{
	SVector2D svZoomedPadding;
	getWindowScrollPadding(svZoomedPadding);
	svZoomedPadding *= getZoom();
	return myPan.x <= svZoomedPadding.x;
}
/*****************************************************************************/
bool UIZoomWindowElement::getIsAtRightPanLimit()
{
	if(getIsPanningLimitless())
		return false;

	SVector2D svZoomedPadding;
	getWindowScrollPadding(svZoomedPadding);
	svZoomedPadding *= getZoom();
	SVector2D svRealContMax;
	getMaxPan(svRealContMax);
	svRealContMax -= svZoomedPadding;
	return myPan.x >= svRealContMax.x;
}
/*****************************************************************************/
bool UIZoomWindowElement::getIsAnimatingZoom() const
{ 
	return myZoom.getIsAnimating(false);
}
/*****************************************************************************/
void UIZoomWindowElement::computeNewPan(FLOAT_TYPE fNewZoom, const SVector2D *pOptScreenNewCenter, FLOAT_TYPE fWindowStartX, FLOAT_TYPE fWindowStartY, 
										bool bPlaceTargetPointUnderMouseCursor, SVector2D& svNewPanOut)
{
	SVector2D svOwnSize;
	this->getBoxSize(svOwnSize);

	SVector2D svWindowStart(fWindowStartX, fWindowStartY);

	SVector2D svTargetPoint;
	if(pOptScreenNewCenter)
		svTargetPoint = *pOptScreenNewCenter;
	else
		svTargetPoint = svWindowStart + svOwnSize*0.5;

	screenToTargetFinalSpaceSubclass(svTargetPoint, fNewZoom, svTargetPoint);
	//svNewPanOut = svTargetPoint*fNewZoom - svOwnSize/2.0;
//	svNewPanOut = svTargetPoint*fNewZoom - svOwnSize/2.0;


	// Now, we might need to place this point
	// under the current mouse position,
	// not in the center...
	svNewPanOut = svTargetPoint*fNewZoom - svOwnSize/2.0;
	if(pOptScreenNewCenter && bPlaceTargetPointUnderMouseCursor)
	{
		SVector2D svWindowCenter = svWindowStart + svOwnSize*0.5;
		SVector2D svMousePos;
		getParentWindow()->getLastMousePos(svMousePos);
		svNewPanOut -= (svMousePos - svWindowCenter);
	}
	else
	{
		// Clamp to valid first:
		clampPanToValidRange(svNewPanOut, false);
	}

	SVector2D svScrollPadding;
	getWindowScrollPadding(svScrollPadding);
	svNewPanOut += svScrollPadding;

//	svNewPanOut = svTargetPoint - (svOwnSize/2.0)/fNewZoom;
}
/*****************************************************************************/
void UIZoomWindowElement::clampPanToValidRange(SVector2D& svPanInOut, bool bIncludePadding)
{
	if(svPanInOut.x < 0)
		svPanInOut.x = 0;
	if(svPanInOut.y < 0)
		svPanInOut.y = 0;

	// See if we're not exceeding max scroll
	SVector2D svMaxPan;
	getMaxPan(svMaxPan, bIncludePadding);

	if(svPanInOut.x > svMaxPan.x)
		svPanInOut.x = svMaxPan.x;
	if(svPanInOut.y > svMaxPan.y)
		svPanInOut.y = svMaxPan.y;
}
/*****************************************************************************/
};