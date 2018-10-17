#include "stdafx.h"

// This compensates for the fact that our splitter is 7 pixels wide, meaning we can't
// really ever properly center it...
#define HACKY_SHIFT_X		upToScreen(0.5)
#define HACKY_SHIFT_Y		upToScreen(0.0)

#define SNAP_BUTTONS_OFFSET_ALONG		upToScreen(35)
#define SNAP_BUTTON_SIZE_ALONG			upToScreen(9.0)
#define SNAP_BUTTONS_HACK_SHIFT			-upToScreen(4.5)

namespace HyperUI
{
/*****************************************************************************/
UISplitterElement::UISplitterElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
    onAllocated(pParentPlane);
}
/*****************************************************************************/
void UISplitterElement::onAllocated(IBaseObject* pData)
{
    UIElement::onAllocated(pData);
	myHandleAnim = "";
	myBarAnim = "";
	mySavedNormalPosition = -1;
	mySnapState = SplitterSnapNone;
	myDidPressButton = false;
	myIsAdjustingFromSavedNormalPosition = false;
}
/*****************************************************************************/
void UISplitterElement::postInit(void)
{
    UIElement::postInit();

    myBarAnim = this->getEnumPropValue(PropertySplitterParms, 0);
    myHandleAnim = this->getEnumPropValue(PropertySplitterParms, 1);
	_ASSERT(myHandleAnim.length() > 0);
	_ASSERT(myBarAnim.length() > 0);

	myFixedChildIndex = -1;
	if(this->doesPropertyExist(PropertySplitterFixedChildIndex))
		myFixedChildIndex = this->getNumProp(PropertySplitterFixedChildIndex);

	myDisableResizing = this->getBoolProp(PropertySplitterDisableResizing);

	myPosition = FLOAT_TYPE_MAX;

    myIsDraggingHandle = false;
}
/*****************************************************************************/
void UISplitterElement::postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	if(!this->getBoolProp(PropertySplitterDisabledWhenSnapped) || mySnapState == SplitterSnapNone)
		postRenderSplitter(svScroll, fOpacity, fScale);
    UIElement::postRender(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
void UISplitterElement::postRenderSplitter(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
    bool bIsVertical = this->getBoolProp(PropertyIsVertical);

    SVector2D svPos;
    FLOAT_TYPE fFinalOpac, fLocScale;
    this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
    fFinalOpac *= fOpacity;
    SVector2D svSize;
    this->getBoxSize(svSize);
    svSize *= fLocScale*fScale;
    if(this->getParent())
		svPos *= fScale;

    SRect2D srHandleRect;
    this->getHandleRectInScreenCoords(srHandleRect);
    SVector2D svHandleCenter;
    srHandleRect.getCenter(svHandleCenter);

    // Render the bar
    int iRealW, iRealH;
	getTextureManager()->getTextureRealDims(myBarAnim.c_str(), iRealW, iRealH);

    FLOAT_TYPE fExtraScaleX, fExtraScaleY;
    fExtraScaleX = svSize.x/(FLOAT_TYPE)iRealW*fScale*fLocScale;
    fExtraScaleY = svSize.y/(FLOAT_TYPE)iRealH*fScale*fLocScale;

    // Hack to avoid bug when resizing
    if(fabs(fExtraScaleX) < FLOAT_EPSILON)
		fExtraScaleX = 1.0;
    if(fabs(fExtraScaleY) < FLOAT_EPSILON)
		fExtraScaleY = 1.0;

	SVector2D svHandleShift;
    if(bIsVertical)
	{
		getDrawingCache()->addSprite(myBarAnim.c_str(), svHandleCenter.x + ((myFixedChildIndex == 0) ? HACKY_SHIFT_X : 0), svHandleCenter.y,
    			fFinalOpac, 0, 1.0, fExtraScaleY, 0, true);
		svHandleShift.x = ((myFixedChildIndex == 0) ? HACKY_SHIFT_X : 0);
	}
    else
	{
		getDrawingCache()->addSprite(myBarAnim.c_str(), svHandleCenter.x, svHandleCenter.y + ((myFixedChildIndex == 0) ? HACKY_SHIFT_Y : 0),
			fFinalOpac, 0, fExtraScaleX, 1.0, 0, true);
		svHandleShift.y = ((myFixedChildIndex == 0) ? HACKY_SHIFT_Y : 0);
	}

    // Render the handle
    if(!getIsFixed())
	{
		getDrawingCache()->addSprite(myHandleAnim.c_str(), svHandleCenter.x + svHandleShift.x, svHandleCenter.y + svHandleShift.y, fFinalOpac, 0, fScale, 1.0, true);

		SVector2D svCenter;
		SRect2D srButtonRect;

		getToFarButtonRectInScreenCoords(srButtonRect);
		srButtonRect.getCenter(svCenter);
		if(bIsVertical)
			svCenter.x -= upToScreen(0.5);
		else
			svCenter.y -= upToScreen(0.5);

		if(bIsVertical)
			theCommonString = "tinyElems:25";
		else
			theCommonString = "tinyElems:27";
		getDrawingCache()->addSprite(theCommonString.c_str(), svCenter.x, svCenter.y, fFinalOpac*0.65, 0, fScale, 1.0, true);
#ifdef _DEBUG
		//getDrawingCache()->addRectangle(srButtonRect, SColor::Red, upToScreen(0.51));
#endif
		getToNearButtonRectInScreenCoords(srButtonRect);
		srButtonRect.getCenter(svCenter);
		if(bIsVertical)
			theCommonString = "tinyElems:26";
		else
			theCommonString = "tinyElems:28";
		getDrawingCache()->addSprite(theCommonString.c_str(), svCenter.x, svCenter.y, fFinalOpac*0.65, 0, fScale, 1.0, true);
#ifdef _DEBUG
		//getDrawingCache()->addRectangle(srButtonRect, SColor::Red, upToScreen(0.51));
#endif
	}

}
/*****************************************************************************/
void UISplitterElement::getHandleRectInScreenCoords(SRect2D& srOut)
{
    SVector2D svPos;
    FLOAT_TYPE fGlobalScale;
    this->getGlobalPosition(svPos, NULL, &fGlobalScale);
    SVector2D svOwnSize;
    this->getBoxSize(svOwnSize);
    svOwnSize *= fGlobalScale;

    int iHandleW, iHandleH;
    getTextureManager()->getTextureRealDims(myHandleAnim.c_str(), iHandleW, iHandleH);

    bool bIsVertical = this->getBoolProp(PropertyIsVertical);

	FLOAT_TYPE fCurrPosition = getCurrPositionPerc();
    FLOAT_TYPE fOffset;
    if(bIsVertical)
    {
		fOffset = svOwnSize.x*fCurrPosition;
		srOut.x = svPos.x - svOwnSize.x/2.0 + fOffset - iHandleW/2.0;
		srOut.y = svPos.y - svOwnSize.y/2.0;
		srOut.w = iHandleW;
		srOut.h = svOwnSize.y;
    }
    else
    {
		fOffset = svOwnSize.y*fCurrPosition;
		srOut.x = svPos.x - svOwnSize.x/2.0;
		srOut.y = svPos.y  - svOwnSize.y/2.0 + fOffset - iHandleH/2.0;
		srOut.w = svOwnSize.x;
		srOut.h = iHandleH;
    }

	if(this->getBoolProp(PropertySplitterDisabledWhenSnapped) && mySnapState != SplitterSnapNone)
	{
		srOut.w = 0.0;
		srOut.h = 0.0;
	}
}
/*****************************************************************************/
void UISplitterElement::onPressed(TTouchVector& vecTouches)
{
    UIElement::onPressed(vecTouches);
	myDidPressButton = false;
    myIsDraggingHandle = false;

	if(this->getBoolProp(PropertySplitterDisabledWhenSnapped) && mySnapState != SplitterSnapNone)
		return;

    if(getIsEnabled() && vecTouches.size() > 0 && !getIsFixed())
    {
		SRect2D srTestRect;
		getToFarButtonRectInScreenCoords(srTestRect);
		if(srTestRect.doesContain(vecTouches[0].myPoint))
			myDidPressButton = true;
		else 
		{
			getToNearButtonRectInScreenCoords(srTestRect);
			if(srTestRect.doesContain(vecTouches[0].myPoint))
				myDidPressButton = true;
		}

		if(!myDidPressButton && mySnapState == SplitterSnapNone && !myDisableResizing)
		{
			getHandleRectInScreenCoords(srTestRect);
			if(srTestRect.doesContain(vecTouches[0].myPoint))
			{
				myIsDraggingHandle = true;
				myStartPos = vecTouches[0].myPoint;
				myStartSepPos = getCurrPositionPerc();
			}
		}
    }
}
/*****************************************************************************/
void UISplitterElement::onMouseEnter(TTouchVector& vecTouches)
{
    UIElement::onMouseEnter(vecTouches);	
}
/*****************************************************************************/
void UISplitterElement::onMouseLeave(TTouchVector& vecTouches)
{
    UIElement::onMouseLeave(vecTouches);	
    myIsDraggingHandle = false;
}
/*****************************************************************************/
void UISplitterElement::onMouseMove(TTouchVector& vecTouches)
{
    UIElement::onMouseMove(vecTouches);

	if(this->getBoolProp(PropertySplitterDisabledWhenSnapped) && mySnapState != SplitterSnapNone)
		return;

    if(myIsDraggingHandle && vecTouches.size() > 0)
    {
		SVector2D svDiff;
		svDiff = vecTouches[0].myPoint - myStartPos;

		SVector2D svPos;
		FLOAT_TYPE fGlobalScale;
		this->getGlobalPosition(svPos, NULL, &fGlobalScale);
		SVector2D svOwnSize;
		this->getBoxSize(svOwnSize);
		svOwnSize *= fGlobalScale;

		bool bIsVertical = this->getBoolProp(PropertyIsVertical);
		if(bIsVertical)
			setPositionInternal(myStartSepPos + (svDiff.x/svOwnSize.x));
		else
			setPositionInternal(myStartSepPos + (svDiff.y/svOwnSize.y));

		ensurePositionObeysMinMaxSizes(svOwnSize);
		autoLayoutChildrenRecursive();
    }
}
/*****************************************************************************/
void UISplitterElement::setLeftPaneSize(FLOAT_TYPE fPixelSize)
{
	SVector2D svPos;
	FLOAT_TYPE fGlobalScale;
	this->getGlobalPosition(svPos, NULL, &fGlobalScale);
	SVector2D svOwnSize;
	this->getBoxSize(svOwnSize);
	svOwnSize *= fGlobalScale;

	bool bIsVertical = this->getBoolProp(PropertyIsVertical);
	if(bIsVertical)
		setPositionInternal(fPixelSize/svOwnSize.x);
	else
		setPositionInternal(fPixelSize/svOwnSize.y);

	ensurePositionObeysMinMaxSizes(svOwnSize);

	// We do this because autoLayoutChildren(),
	// despite its name, is actually changing
	// the splitter pos unless we're dragging (when
	// it doesn't). Since it's virtual, we can't
	// simply pass in a flag, so instead, we emulate
	// dragging...
	bool bOldIsDragging = myIsDraggingHandle;
	myIsDraggingHandle = true;
	autoLayoutChildrenRecursive();
	myIsDraggingHandle = bOldIsDragging;
}
/*****************************************************************************/
void UISplitterElement::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{
    UIElement::onReleased(vecTouches, bIgnoreActions);

	if(this->getBoolProp(PropertySplitterDisabledWhenSnapped) && mySnapState != SplitterSnapNone)
		return;

    if(vecTouches.size() > 0)
    {
		if(myDidPressButton && !getIsFixed())
		{
			SRect2D srTestRect;
			getToFarButtonRectInScreenCoords(srTestRect);
			if(srTestRect.doesContain(vecTouches[0].myPoint))
				snapTowardFarEnd();
			else 
			{
				getToNearButtonRectInScreenCoords(srTestRect);
				if(srTestRect.doesContain(vecTouches[0].myPoint))
					snapTowardNearEnd();
			}
		}
		
    }
    myIsDraggingHandle = false;
	myDidPressButton = false;
}
/*****************************************************************************/
void UISplitterElement::autoLayoutChildren()
{
#ifdef _DEBUG
	if(IS_OF_TYPE("toolSplitter"))
	{
		int bp = 0;
	}
#endif
    SVector2D svPos;
    FLOAT_TYPE fGlobalScale;
    this->getGlobalPosition(svPos, NULL, &fGlobalScale);
    SVector2D svOwnSize;
    this->getBoxSize(svOwnSize);
    svOwnSize *= fGlobalScale;

    bool bIsVertical = this->getBoolProp(PropertyIsVertical);

    // Resize the children (first two only).
    SRect2D srChildRect;
    UIElement *pChildren[2] = { NULL, NULL };
    if(this->getNumChildren() > 0)
		pChildren[0] = FAST_CAST<UIElement*>(this->getChild(0));
    if(this->getNumChildren() > 1)
		pChildren[1] = FAST_CAST<UIElement*>(this->getChild(1));

	// Set to null if they're hidden
	if(pChildren[0] && !pChildren[0]->getIsVisible() && !myIsAdjustingFromSavedNormalPosition)
		pChildren[0] = NULL;
	if(pChildren[1] && !pChildren[1]->getIsVisible() && !myIsAdjustingFromSavedNormalPosition)
		pChildren[1] = NULL;

    FLOAT_TYPE fOffset;
    SRect2D srHandleRect;
    this->getHandleRectInScreenCoords(srHandleRect);

	// We only keep the panel fixed if the user isn't explicitly
	// dragging the handle.
	bool bKeepFixedPanel = !myIsDraggingHandle && !myIsAdjustingFromSavedNormalPosition;

#ifdef _DEBUG
	if(IS_OF_TYPE("mainViewerMidContainer"))
	{
		int bp = 0;
	}
#endif

	SVector2D svChildSizes[2];
	if(myFixedChildIndex >= 0 && bKeepFixedPanel)
	{
		if(pChildren[0])
			pChildren[0]->getBoxSize(svChildSizes[0]);
		if(pChildren[1])
			pChildren[1]->getBoxSize(svChildSizes[1]);
	}

    if(bIsVertical)
    {
		if(myFixedChildIndex == 0 && bKeepFixedPanel && svChildSizes[0].x > 0)
		{
			fOffset = svChildSizes[0].x;
			// Now add half the handle to compensate for its subtraction below
			fOffset += srHandleRect.w/2.0;
			setPositionInternal(fOffset/svOwnSize.x);
		}
		else if(myFixedChildIndex == 1 && bKeepFixedPanel && svChildSizes[1].x > 0)
		{
			fOffset = svOwnSize.x - svChildSizes[1].x - srHandleRect.w/2.0;
			setPositionInternal(fOffset/svOwnSize.x);
			//fOffset -= srHandleRect.w/2.0;
		}
// 		else
// 			fOffset = svOwnSize.x*getCurrPositionPerc();

		ensurePositionObeysMinMaxSizes(svOwnSize);
		fOffset = svOwnSize.x*getCurrPositionPerc();


		srChildRect.x = srChildRect.y = 0;
		srChildRect.w = fOffset - srHandleRect.w/2.0;
		srChildRect.h = svOwnSize.y;
		if(pChildren[0])
			pChildren[0]->setWindowRect(srChildRect);

		srChildRect.x = fOffset + srHandleRect.w/2.0;
		srChildRect.y = 0;
		srChildRect.w = svOwnSize.x - srChildRect.x;
		srChildRect.h = svOwnSize.y;
		if(pChildren[1])
			pChildren[1]->setWindowRect(srChildRect);
    }
    else
    {
		if(myFixedChildIndex == 0 && bKeepFixedPanel && svChildSizes[0].y > 0)
		{
			fOffset = svChildSizes[0].y;
			// Now add half the handle to compensate for its subtraction below
			fOffset += srHandleRect.h/2.0;
			setPositionInternal(fOffset/svOwnSize.y);
		}
		else if(myFixedChildIndex == 1 && bKeepFixedPanel && svChildSizes[1].y > 0)
		{
			fOffset = svOwnSize.y - svChildSizes[1].y - srHandleRect.h/2.0;
			setPositionInternal(fOffset/svOwnSize.y);
			fOffset -= srHandleRect.h/2.0;
		}
// 		else
// 			fOffset = svOwnSize.y*getCurrPositionPerc();
		
		ensurePositionObeysMinMaxSizes(svOwnSize);
		fOffset = svOwnSize.y*getCurrPositionPerc();

		srChildRect.x = srChildRect.y = 0;
		srChildRect.h = fOffset - srHandleRect.h/2.0;
		srChildRect.w = svOwnSize.x;
		if(pChildren[0])
			pChildren[0]->setWindowRect(srChildRect);

		srChildRect.y = fOffset + srHandleRect.h/2.0;
		srChildRect.x = 0;
		srChildRect.h = svOwnSize.y - srChildRect.y;
		srChildRect.w = svOwnSize.x;
		if(pChildren[1])
			pChildren[1]->setWindowRect(srChildRect);
    }



/*
    // No - this is done in autoLayoutChildrenRecursive() now.
    // Now, make sure any splitters inside the children
    // also get this call.
    UIElement* pChild;
    int iCurrChild, iNumChildren = getNumChildren();
    for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
    {
	pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
	if(!pChild)
	{
	    _ASSERT(0);
	    continue;
	}
	
	pChild->autoLayoutChildren();
    }
    */
}
/*****************************************************************************/
void UISplitterElement::adjustToScreen(void)
{
    UIElement::adjustToScreen();
    autoLayoutChildrenRecursive();
}
/*****************************************************************************/
UIElement* UISplitterElement::getChildAtRecursiveInternal(const SVector2D& svPoint, const SVector2D& svScroll, bool bFirstNonPassthrough, bool bIgnoreChildren, bool bGlidingOnly)
{
    UIElement* pRes = UIElement::getChildAtRecursiveInternal(svPoint, svScroll, bFirstNonPassthrough, bIgnoreChildren, bGlidingOnly);

    // The reason we call the above is that we  need to make sure we ourselves are in the
    // right area. In that case, it would return non-NULL.
    if(myIsDraggingHandle && pRes)
		return this;
    else
		return pRes;
}
/*****************************************************************************/
bool UISplitterElement::getIsFixed()
{
    return false;
}
/*****************************************************************************/
FLOAT_TYPE UISplitterElement::getCurrPositionPerc()
{
	if(myIsAdjustingFromSavedNormalPosition && mySnapState != SplitterSnapNone)
	{
		return mySavedNormalPosition;
	}

	if(myPosition == FLOAT_TYPE_MAX)
	{
		setPositionInternal(0.5);
		FLOAT_TYPE fPresetPos = this->getNumProp(PropertySplitterPos);

		if(fPresetPos > 0)
		{
			SVector2D svSize;
			bool bIsVertical = this->getBoolProp(PropertyIsVertical);
			this->getBoxSize(svSize);
			if(fPresetPos > 1.0)
			{
				_ASSERT(svSize.x > 0 && svSize.y > 0);
				// Assume the position is in pixels
				if(bIsVertical)
					setPositionInternal(fPresetPos/svSize.x);
				else
					setPositionInternal(fPresetPos/svSize.y);
			}
			else if(fPresetPos > 0)
			{
				// Assume the position is a ratio
				setPositionInternal(fPresetPos);
			}
			ensurePositionObeysMinMaxSizes(svSize);
		}
	}

	return myPosition;
}
/*****************************************************************************/
CursorType UISplitterElement::getOwnCursorType() const
{
	// Fixed splitters do not change cursors...
 	if(myDisableResizing)
 		return CursorArrow;

	// See if we're in either button...
	if(getParentWindow()->getDidFinishPrelimInit() && myHandleAnim.length() > 0)
	{
		SVector2D svMousePos;
		getParentWindow()->getLastMousePos(svMousePos);
		SRect2D srTestRect;
		const_cast<UISplitterElement*>(this)->getToNearButtonRectInScreenCoords(srTestRect);
		if(srTestRect.doesContain(svMousePos))
			return CursorArrow;
		const_cast<UISplitterElement*>(this)->getToFarButtonRectInScreenCoords(srTestRect);
		if(srTestRect.doesContain(svMousePos))
			return CursorArrow;
	}

	bool bIsVertical = this->getBoolProp(PropertyIsVertical);
	if(bIsVertical)
		return CursorHorSize;
	else
		return CursorVertSize;
}
/*****************************************************************************/
void UISplitterElement::getMinDims(SVector2D& svDimsOut) const
{
	SVector2D svTemp;

	svDimsOut.set(0, 0);

	// Go through all our tabs
	int iCurr, iNum = getNumChildren();
	const UIElement* pChild;

	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = dynamic_cast<const UIElement*>(getChild(iCurr));
		if(!pChild)
			ASSERT_CONTINUE;

		pChild->getMinDims(svTemp);
		if(svTemp.x > 0 && svDimsOut.x < svTemp.x)
			svDimsOut.x = svTemp.x;
		if(svTemp.y > 0 && svDimsOut.y < svTemp.y)
			svDimsOut.y = svTemp.y;
	}
}
/*****************************************************************************/
void UISplitterElement::getMaxDims(SVector2D& svDimsOut) const
{
	SVector2D svTemp;

	svDimsOut.set(0, 0);

	// Go through all our tabs
	int iCurr, iNum = getNumChildren();
	const UIElement* pChild;

	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = dynamic_cast<const UIElement*>(getChild(iCurr));
		if(!pChild)
			ASSERT_CONTINUE;

		pChild->getMaxDims(svTemp);
		if(svTemp.x > 0 && (svDimsOut.x > svTemp.x || svDimsOut.x == 0))
			svDimsOut.x = svTemp.x;
		if(svTemp.y > 0 && (svDimsOut.y > svTemp.y || svDimsOut.y == 0))
			svDimsOut.y = svTemp.y;
	}
}
/*****************************************************************************/
void UISplitterElement::ensurePositionObeysMinMaxSizes(const SVector2D& svOwnSize)
{
	// Not if we're not in our normal snap state
	if(mySnapState != SplitterSnapNone)
		return;

	ensurePositionObeysMinSizes(svOwnSize);
	ensurePositionObeysMaxSizes(svOwnSize);
}
/*****************************************************************************/
void UISplitterElement::ensurePositionObeysMinSizes(const SVector2D& svOwnSize)
{
	bool bIsVertical = this->getBoolProp(PropertyIsVertical);
	SVector2D svChild1Min, svChild2Min;
	UIElement *pChildren[2] = { NULL, NULL };
	if(this->getNumChildren() > 0)
	{		
		pChildren[0] = FAST_CAST<UIElement*>(this->getChild(0));
		pChildren[0]->getMinDims(svChild1Min);
	}
	if(this->getNumChildren() > 1)
	{
		pChildren[1] = FAST_CAST<UIElement*>(this->getChild(1));
		pChildren[1]->getMinDims(svChild2Min);
	}

	// Make sure our position is ok with these
	FLOAT_TYPE fAdjustedPositions[2] = { -1, -1 };
	if(bIsVertical)
	{
		if(svChild1Min.x > 0)
		{
			fAdjustedPositions[0] = svChild1Min.x/svOwnSize.x;
			if(myPosition*svOwnSize.x < svChild1Min.x)
				setPositionInternal(fAdjustedPositions[0]);
		}

		if(svChild2Min.x > 0)
		{
			fAdjustedPositions[1] = 1.0 - svChild2Min.x/svOwnSize.x;
			if((1.0 - myPosition)*svOwnSize.x < svChild2Min.x)
				setPositionInternal(fAdjustedPositions[1]);
		}

		// Check if neither can get the space it wants:
		if(svChild1Min.x > 0 && svChild2Min.x > 0 && svChild1Min.x + svChild2Min.x > svOwnSize.x)
			setPositionInternal((fAdjustedPositions[0] + fAdjustedPositions[1])*0.5);
	}
	else
	{
		if(svChild1Min.y > 0)
		{
			fAdjustedPositions[0] = svChild1Min.y/svOwnSize.y;
			if(myPosition*svOwnSize.y < svChild1Min.y)
				setPositionInternal(fAdjustedPositions[0]);
		}

		if(svChild2Min.y > 0)
		{
			fAdjustedPositions[1] = 1.0 - svChild2Min.y/svOwnSize.y;
			if((1.0 - myPosition)*svOwnSize.y < svChild2Min.y)
				setPositionInternal(fAdjustedPositions[1]);
		}

		// Check if neither can get the space it wants:
		if(svChild1Min.y > 0 && svChild2Min.y > 0 && svChild1Min.y + svChild2Min.y > svOwnSize.y)
			setPositionInternal((fAdjustedPositions[0] + fAdjustedPositions[1])*0.5);
	}
}
/*****************************************************************************/
void UISplitterElement::ensurePositionObeysMaxSizes(const SVector2D& svOwnSize)
{
	bool bIsVertical = this->getBoolProp(PropertyIsVertical);
	SVector2D svChild1Max, svChild2Max;
	UIElement *pChildren[2] = { NULL, NULL };
	if(this->getNumChildren() > 0)
	{		
		pChildren[0] = FAST_CAST<UIElement*>(this->getChild(0));
		pChildren[0]->getMaxDims(svChild1Max);
	}
	if(this->getNumChildren() > 1)
	{
		pChildren[1] = FAST_CAST<UIElement*>(this->getChild(1));
		pChildren[1]->getMaxDims(svChild2Max);
	}

	// Make sure our position is ok with these
	FLOAT_TYPE fAdjustedPositions[2] = { -1, -1 };
	if(bIsVertical)
	{
		if(svChild1Max.x > 0)
		{
			fAdjustedPositions[0] = svChild1Max.x/svOwnSize.x;
			if(myPosition*svOwnSize.x > svChild1Max.x)
				setPositionInternal(fAdjustedPositions[0]);
		}

		if(svChild2Max.x > 0)
		{
			fAdjustedPositions[1] = 1.0 - svChild2Max.x/svOwnSize.x;
			if((1.0 - myPosition)*svOwnSize.x > svChild2Max.x)
				setPositionInternal(fAdjustedPositions[1]);
		}

		// Check if neither can get the space it wants:
		if(svChild1Max.x > 0 && svChild2Max.x > 0 && svChild1Max.x + svChild2Max.x > svOwnSize.x)
			setPositionInternal((fAdjustedPositions[0] + fAdjustedPositions[1])*0.5);
	}
	else
	{
		if(svChild1Max.y > 0)
		{
			fAdjustedPositions[0] = svChild1Max.y/svOwnSize.y;
			if(myPosition*svOwnSize.y > svChild1Max.y)
				setPositionInternal(fAdjustedPositions[0]);
		}

		if(svChild2Max.y > 0)
		{
			fAdjustedPositions[1] = 1.0 - svChild2Max.y/svOwnSize.y;
			if((1.0 - myPosition)*svOwnSize.y > svChild2Max.y)
				setPositionInternal(fAdjustedPositions[1]);
		}

		// Check if neither can get the space it wants:
		if(svChild1Max.y > 0 && svChild2Max.y > 0 && svChild1Max.y + svChild2Max.y > svOwnSize.y)
			setPositionInternal((fAdjustedPositions[0] + fAdjustedPositions[1])*0.5);
	}
}
/*****************************************************************************/
void UISplitterElement::snapTowardFarEnd()
{
	if(getIsFixed())
		return;

	if(mySnapState == SplitterSnapFar)
		return;

	if(mySnapState == SplitterSnapNone)
		mySavedNormalPosition = myPosition;

	mySnapState = (SplitterSnapType)((int)mySnapState + 1);
	onSnapStateChanged();
}
/*****************************************************************************/
void UISplitterElement::snapTowardNearEnd()
{
	if(getIsFixed())
		return;

	if(mySnapState == SplitterSnapNear)
		return;

	if(mySnapState == SplitterSnapNone)
		mySavedNormalPosition = myPosition;

	mySnapState = (SplitterSnapType)((int)mySnapState - 1);
	onSnapStateChanged();
}
/*****************************************************************************/
void UISplitterElement::onSnapStateChanged()
{
	UIElement *pChildren[2] = { NULL, NULL };
	if(this->getNumChildren() > 0)
		pChildren[0] = FAST_CAST<UIElement*>(this->getChild(0));
	if(this->getNumChildren() > 1)
		pChildren[1] = FAST_CAST<UIElement*>(this->getChild(1));

	bool bWasChildVisible[2] = { false, false };
	if(pChildren[0])
		bWasChildVisible[0] = pChildren[0]->getIsVisible();
	if(pChildren[1])
		bWasChildVisible[1] = pChildren[1]->getIsVisible();

	// Now, hide or show the right child
	if(pChildren[0])
		pChildren[0]->setIsVisible(mySnapState != SplitterSnapNear);
	if(pChildren[1])
		pChildren[1]->setIsVisible(mySnapState != SplitterSnapFar);

	FLOAT_TYPE fExtraPaddingForSize = 0.0;

	if(mySnapState != SplitterSnapNone)
	{
		SVector2D svPos;
		FLOAT_TYPE fGlobalScale;
		this->getGlobalPosition(svPos, NULL, &fGlobalScale);
		SVector2D svOwnSize;
		this->getBoxSize(svOwnSize);
		svOwnSize *= fGlobalScale;


		bool bIsVertical = this->getBoolProp(PropertyIsVertical);
		if(bIsVertical)
			fExtraPaddingForSize = upToScreen(1.5)/svOwnSize.x;
		else
			fExtraPaddingForSize = upToScreen(1.5)/svOwnSize.y;

		if(this->getBoolProp(PropertySplitterDisabledWhenSnapped))
			fExtraPaddingForSize = 0.0;
	}

	// If we're not in a normal snap pos, change the position
	if(mySnapState == SplitterSnapNear)
		setPositionInternal(fExtraPaddingForSize);
	else if(mySnapState == SplitterSnapFar)
		setPositionInternal(1.0 - fExtraPaddingForSize);
	else if(mySavedNormalPosition >= 0.0)
		setPositionInternal(mySavedNormalPosition);

	// Our new vis status
	bool bIsChildVisible[2] = { false, false };
	if(pChildren[0])
		bIsChildVisible[0] = pChildren[0]->getIsVisible();
	if(pChildren[1])
		bIsChildVisible[1] = pChildren[1]->getIsVisible();

	// See above in setLeftPos for why this hack exists...
	bool bOldIsDragging = myIsDraggingHandle;
	myIsDraggingHandle = true;
	autoLayoutChildrenRecursive();
	myIsDraggingHandle = bOldIsDragging;
}
/*****************************************************************************/
void UISplitterElement::getToFarButtonRectInScreenCoords(SRect2D& srOut)
{
	bool bIsVertical = this->getBoolProp(PropertyIsVertical);
	getHandleRectInScreenCoords(srOut);
	if(bIsVertical)
	{
		srOut.y = srOut.y + srOut.h/2.0;
		srOut.y -= SNAP_BUTTONS_OFFSET_ALONG + SNAP_BUTTON_SIZE_ALONG/2.0;
		srOut.h = SNAP_BUTTON_SIZE_ALONG;

		srOut.y += SNAP_BUTTONS_HACK_SHIFT;
	}
	else
	{
		srOut.x = srOut.x + srOut.w/2.0;
		srOut.x -= SNAP_BUTTONS_OFFSET_ALONG + SNAP_BUTTON_SIZE_ALONG/2.0;
		srOut.w = SNAP_BUTTON_SIZE_ALONG;

		srOut.x += SNAP_BUTTONS_HACK_SHIFT;
	}
}
/*****************************************************************************/
void UISplitterElement::getToNearButtonRectInScreenCoords(SRect2D& srOut)
{
	bool bIsVertical = this->getBoolProp(PropertyIsVertical);
	getHandleRectInScreenCoords(srOut);
	if(bIsVertical)
	{
		srOut.y = srOut.y + srOut.h/2.0;
		srOut.y += SNAP_BUTTONS_OFFSET_ALONG + SNAP_BUTTON_SIZE_ALONG/2.0;
		srOut.h = SNAP_BUTTON_SIZE_ALONG;

		srOut.y += SNAP_BUTTONS_HACK_SHIFT;
	}
	else
	{
		srOut.x = srOut.x + srOut.w/2.0;
		srOut.x += SNAP_BUTTONS_OFFSET_ALONG + SNAP_BUTTON_SIZE_ALONG/2.0;
		srOut.w = SNAP_BUTTON_SIZE_ALONG;

		srOut.x += SNAP_BUTTONS_HACK_SHIFT;
	}

}
/*****************************************************************************/
void UISplitterElement::onCursorGlide(TTouchVector& vecTouches)
{
	invalidateAndUpdateCursor();
}
/*****************************************************************************/
void UISplitterElement::setNewSnapState(SplitterSnapType eState, bool bPerformUpdate, bool bManageDisabledCompletelyFlag)
{
	if(getIsFixed())
		return;

	if(mySnapState == eState)
		return;

	if(mySnapState == SplitterSnapNone)
		mySavedNormalPosition = myPosition;

	mySnapState = eState;

	if(bManageDisabledCompletelyFlag)
		this->setBoolProp(PropertySplitterDisabledWhenSnapped, eState != SplitterSnapNone);

	if(bPerformUpdate)
		onSnapStateChanged();
}
/*****************************************************************************/
bool UISplitterElement::getIsVertical() const
{
	return this->getBoolProp(PropertyIsVertical);
}
/*****************************************************************************
void UISplitterElement::setPercSplitterPosition(FLOAT_TYPE fValue)
{
	myPosition = fValue;
}
/*****************************************************************************/
void UISplitterElement::saveCoreInfoInto(ResourceItem* pOwnItem) const
{
	pOwnItem->setNumProp(PropertySvSplitterPercPos, myPosition);
	pOwnItem->setBoolProp(PropertyIsVertical, getIsVertical());
	pOwnItem->setNumProp(PropertySvSplitterSnapState, (int)mySnapState);
	pOwnItem->setNumProp(PropertySvSplitterSavedNormalPos, mySavedNormalPosition);
	pOwnItem->setNumProp(PropertySvSplitterFixedChildIndex, myFixedChildIndex);
}
/*****************************************************************************/
void UISplitterElement::loadCoreInfoFrom(const ResourceItem* pOwnItem)
{
	setPositionInternal(pOwnItem->getNumProp(PropertySvSplitterPercPos));
	mySnapState = (SplitterSnapType)(int)pOwnItem->getNumProp(PropertySvSplitterSnapState);
	mySavedNormalPosition = pOwnItem->getNumProp(PropertySvSplitterSavedNormalPos);
	myFixedChildIndex = pOwnItem->getNumProp(PropertySvSplitterFixedChildIndex);
}
/*****************************************************************************/
bool UISplitterElement::getIsChildPaneVisible(const UIElement* pChild) const
{
	if(mySnapState == SplitterSnapNone)
		return true;

	const UIElement *pChildren[2] = { NULL, NULL };
	if(this->getNumChildren() > 0)
		pChildren[0] = CONST_FAST_CAST<UIElement*>(this->getChild(0));
	if(this->getNumChildren() > 1)
		pChildren[1] = CONST_FAST_CAST<UIElement*>(this->getChild(1));

	if(mySnapState == SplitterSnapNear && pChildren[0] == pChild)
		return false;

	if(mySnapState == SplitterSnapFar && pChildren[1] == pChild)
		return false;

	return true;
}
/*****************************************************************************/
};