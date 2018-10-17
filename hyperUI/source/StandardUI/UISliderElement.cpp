#include "stdafx.h"

//#define BUTTON_SPACING		upToScreen(6.0)
#define BUTTON_SPACING		upToScreen(0)
#define VALUE_ANIM_TIME		0.10

#define SLIDER_MULT_HOR_PADDING		0.5

#define VALUE_DISPLAY_FADE_SECONDS		0.10

// Note that the larger the divisor, the flatter the curve - i.e. the faster it rises at first.
#define LOG_SLIDER_DIVISOR			13.0

namespace HyperUI
{
/*****************************************************************************/
UISliderElement::UISliderElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane), AnimSequenceAddon(PropertySecondaryImage)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UISliderElement::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
	AnimSequenceAddon::onAllocated(PropertySecondaryImage);
	myCurrUndoBlockId = -1;
}
/*****************************************************************************/
void UISliderElement::postInit(void)
{
	UIElement::postInit();

	// Get this from a property.
	myIsVertical = this->getBoolProp(PropertyIsVertical);
	myStartInMiddle = this->getBoolProp(PropertyStartInMiddle);

/*
	if(myIsVertical)
		myIsSizable = this->getIsSizableInY() || this->doesPropertyExist(PropertyUiObjSpacingWidth) || this->doesPropertyExist(PropertyUiObjFillParentYLessPadding);
	else
		myIsSizable = this->getIsSizableInX() || this->doesPropertyExist(PropertyUiObjSpacingWidth) || this->doesPropertyExist(PropertyUiObjFillParentXLessPadding);
		*/
	if(myIsVertical)
		myIsSizable = this->doesPropertyExist(PropertyLayoutWidth) || (this->getFillParentYLessPadding() >= 0);
	else
		myIsSizable = this->doesPropertyExist(PropertyLayoutWidth) || (this->getFillParentXLessPadding() >= 0);

	myIsInCallback = false;
	myAllowPressing = false;
	if(myStartInMiddle)
	{
		myMinValue = -1.0;
		myMaxValue = 1.0;
	}
	else
	{
		myMinValue = 0.0;
		myMaxValue = 1.0;
	}
	myButtonValueStep = 0.05;
	myCurrValue.setNonAnimValue(0.0);
	myActiveSliderPart = SliderPartNone;
	myDragStartValue = 0;
	myValueDisplayOpacity.setNonAnimValue(0);

	// See if we have a button size
	myKnobSize.set(0, 0);
	myButtonSize.set(0,0);
	if(this->doesPropertyExist(PropertySliderButtonSizes))
	{
		myKnobSize.x = this->getNumericEnumPropValue(PropertySliderButtonSizes, 0);
		myKnobSize.y = this->getNumericEnumPropValue(PropertySliderButtonSizes, 1);
		if(this->getNumericEnumPropCount(PropertySliderButtonSizes) > 2)
		{
			myButtonSize.x = this->getNumericEnumPropValue(PropertySliderButtonSizes, 2);
			myButtonSize.y = this->getNumericEnumPropValue(PropertySliderButtonSizes, 3);
		}
	}

	// Need to specify the knob size
	_ASSERT(myKnobSize.x > 0 && myKnobSize.y > 0);

	myValueDisplayAnim = "";
	myFullSliderAnim = "";
	myLessButtonAnim = myMoreButtonAnim = "";
	if(this->doesPropertyExist(PropertySliderParms))
	{
		myLessButtonAnim = this->getEnumPropValue(PropertySliderParms, 0);
		if(strcmp(myLessButtonAnim.c_str(), PROPERTY_NONE) == 0)
			myLessButtonAnim = "";
		myMoreButtonAnim = this->getEnumPropValue(PropertySliderParms, 1);
		if(strcmp(myMoreButtonAnim.c_str(), PROPERTY_NONE) == 0)
			myMoreButtonAnim = "";

		if(this->getEnumPropCount(PropertySliderParms) >= 3)
		{
			// We have the full slider specified, too.
			myFullSliderAnim = this->getEnumPropValue(PropertySliderParms, 2);
			if(strcmp(myFullSliderAnim.c_str(), PROPERTY_NONE) == 0)
				myFullSliderAnim = "";
		}

		if(this->getEnumPropCount(PropertySliderParms) >= 4)
		{
			myValueDisplayAnim = this->getEnumPropValue(PropertySliderParms, 3);
			if(strcmp(myValueDisplayAnim.c_str(), PROPERTY_NONE) == 0)
				myValueDisplayAnim = "";
		}
	}

	// See if we have value display parms
	myValueDisplayMult = 1.0;
	myValueDisplayYOffset = 0;
	if(this->doesPropertyExist(PropertySliderValParms))
	{
		myValueDisplayYOffset = this->getNumericEnumPropValue(PropertySliderValParms, 0);
		if(this->getEnumPropCount(PropertySliderParms) >= 2)
			myValueDisplayMult = this->getNumericEnumPropValue(PropertySliderValParms, 1);
	}

	if(this->doesPropertyExist(PropertyMin))
	{
		this->getAsString(PropertyMin, theLocalSharedString);
		getParentWindow()->substituteVariables(theLocalSharedString);
		FLOAT_TYPE fVal = StringUtils::convertStringToNumber(theLocalSharedString);
		if(fVal == FLOAT_TYPE_MAX)
			setMinValue(this->getAsNumber(PropertyMin));
		else
			setMinValue(fVal);
	}

	if(this->doesPropertyExist(PropertyMax))
	{
		this->getAsString(PropertyMax, theLocalSharedString);
		getParentWindow()->substituteVariables(theLocalSharedString);
		FLOAT_TYPE fVal = StringUtils::convertStringToNumber(theLocalSharedString);
		if(fVal == FLOAT_TYPE_MAX)
			setMaxValue(this->getAsNumber(PropertyMax));
		else
			setMaxValue(fVal);
	}
}
/*****************************************************************************/
void UISliderElement::setMinValue(FLOAT_TYPE fValue)
{
	myMinValue = fValue;
}
/*****************************************************************************/
void UISliderElement::setMaxValue(FLOAT_TYPE fValue)
{
	myMaxValue = fValue;
}
/*****************************************************************************/
void UISliderElement::setButtonStep(FLOAT_TYPE fValue)
{
	myButtonValueStep = fValue;
}
/*****************************************************************************/
void UISliderElement::setValue(FLOAT_TYPE fValue)
{
	if(myIsInCallback)
		return;
	setValueSafe(fValue, false, NULL, false);
	//myCurrValue.setNonAnimValue(fValue);
}
/*****************************************************************************/
void UISliderElement::setValueSafe(FLOAT_TYPE fValue, bool bAnimated, UIElement* pOptChangeSourceElem, bool bIsChangingContinuously)
{
	if(fValue < myMinValue)
		fValue = myMinValue;
	if(fValue > myMaxValue)
		fValue = myMaxValue;

	FLOAT_TYPE fCurrValue = getValue();
	if(fabs(fValue - fCurrValue) < FLOAT_EPSILON)
		return;

	if(bAnimated)
	{
		FLOAT_TYPE fTime = fabs((fValue - fCurrValue)/(myMaxValue - myMinValue))*VALUE_ANIM_TIME;
		myCurrValue.setAnimation(fCurrValue, fValue, fTime, this->getClockType());
	}
	else
		myCurrValue.setNonAnimValue(fValue);

	UIElement::changeValueTo(fValue, pOptChangeSourceElem, bAnimated, bIsChangingContinuously);
}
/*****************************************************************************/
void UISliderElement::render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	// See if we want to postpone rendering
	// Bad. There's a copy in UIElement, and anything that overrides render() won't use this...
	if(this->getDoPostponeRendering() && getUIPlane()->getRenderingPass() != RenderingPostopnedElements)
	{
		getUIPlane()->addPostponedElement(this, svScroll, fOpacity, fScale);
		return;
	}

	preRender(svScroll, fOpacity, fScale);

	renderSliderInternal(svScroll, fOpacity, fScale);

	postRender(svScroll, fOpacity, fScale);

/*
#ifdef WIN32
	SColor scolRed(1,0,0,1);
	SRect2D srWindowRect;
	this->getElementRectangle(svScroll, fScale, srWindowRect);

	pDrawingCache->addRectangle(srWindowRect.x, srWindowRect.y, srWindowRect.w, srWindowRect.h, scolRed, 2);
#endif
	*/
}
/*****************************************************************************/
void UISliderElement::renderSliderInternal(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	DrawingCache* pDrawingCache = getDrawingCache();

	this->getFullTopAnimName(theLocalSharedString);
	this->getFullBaseAnimName(theLocalSharedString2);
	// Slider knob anim forgotten if fails
	_ASSERT(theLocalSharedString.length() > 0);
	// Empty slider anim forgotten if fails
	_ASSERT(theLocalSharedString2.length() > 0);

	// Now, see if we have full slider anim. If we do, just
	// render progress. If not, just render simple slider.
	bool bHaveButtons = false;
	if(myButtonSize.x > 0 && myButtonSize.y > 0 &&
		myLessButtonAnim.length() > 0 && myMoreButtonAnim.length() > 0)
		bHaveButtons = true;

	// Progress
	FLOAT_TYPE fPercProgress = 0.0;
	FLOAT_TYPE fNormProgress = 0.0;
	FLOAT_TYPE fCurrProg = getValue();
	if(fabs(myMaxValue - myMinValue) > FLOAT_EPSILON)
	{
		fPercProgress = valueToPercProgress(fCurrProg);
		//fPercProgress = (fCurrProg - myMinValue)/(myMaxValue - myMinValue);

		if(myStartInMiddle)
			fNormProgress = (fPercProgress - 0.5)*2.0;
		else
			fNormProgress = fPercProgress;

	}

	if(fPercProgress < 0.0)
		fPercProgress = 0;
	if(fPercProgress > 1.0)
		fPercProgress = 1.0;

	if(!myStartInMiddle)
		fNormProgress = fPercProgress;
	else
	{
		if(fNormProgress < -1.0)
			fNormProgress = -1.0;
		if(fNormProgress > 1.0)
			fNormProgress = 1.0;
	}

	// Finally, draw the slider
	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity;	

	if(this->getParent())
		svPos *= fScale;
	SVector2D svCenter(svPos.x + svScroll.x, svPos.y + svScroll.y);
	
	
	FLOAT_TYPE fVeryFinalScale = fScale*fLocScale;

	// Now, render the slider pos.
	// For this, we need to know how large our slider bitmap is...
	int iSliderW, iSliderH;
	getTextureManager()->getTextureRealDims(theLocalSharedString2.c_str(), iSliderW, iSliderH);
	SVector2D svSliderSize;
	if(myIsSizable)
	{
		getBoxSize(svSliderSize);
		if(myIsVertical)
			svSliderSize.x = iSliderW;
		else
			svSliderSize.y = iSliderH;
		if(!bHaveButtons)
		{
			// We need this because if we only have a slider and no buttons,
			// and the slider is in the 0 or 1 position, 
			// it's impossible to grab the half of the slider outside
			// of the box.
			svSliderSize.x -= myKnobSize.x*SLIDER_MULT_HOR_PADDING;
		}
		svSliderSize *= fVeryFinalScale;
	}
	else
	{
		svSliderSize.x = (FLOAT_TYPE)iSliderW*fVeryFinalScale;
		svSliderSize.y = (FLOAT_TYPE)iSliderH*fVeryFinalScale;
	}
	if(bHaveButtons)
	{
		if(myIsVertical)
			svSliderSize.y -= myButtonSize.y*2;
		else
			svSliderSize.x -= myButtonSize.x*2;
	}

	
	FLOAT_TYPE fLongPartScale = 1.0;
//	if(myFullSliderAnim.length() > 0)
	{
		ProgressBarStyleType eStyle;
		// Shouldn't this be left and top, respectivly?
		if(myIsVertical)
			eStyle = ProgressBarStyleFromTop;
		else
			eStyle = ProgressBarStyleFromLeft;

		SVector2D *pSizePtr = NULL;
		if(myIsSizable)
			pSizePtr = &svSliderSize;

		// Don't draw the progress if we're disabled (which may be colored and visible under the
		// semi-transparent knob).
		FLOAT_TYPE fProgCopy = fNormProgress;
		if(!getIsEnabled())
			fProgCopy = 0;

		RenderUtils::renderProgressBar(getParentWindow(), theLocalSharedString2, myFullSliderAnim, fProgCopy, svCenter, fFinalOpac, fVeryFinalScale, eStyle, pSizePtr, myStartInMiddle);
	}
//	else
//	{
		// Just render the empty part. 
//		pDrawingCache->addSprite(theLocalSharedString2.c_str(), svCenter.x, svCenter.y, fFinalOpac, 0, fVeryFinalScale, 1.0, true);
//	}

	// Draw the knob
	FLOAT_TYPE fButtonSpacing = BUTTON_SPACING*fVeryFinalScale;
	int iPixelOffsetHor;
	if(myIsVertical)
	{
		iPixelOffsetHor = (FLOAT_TYPE)svSliderSize.y*fPercProgress;

		// Now, draw the buttons, if any.
		if(bHaveButtons)
		{	
			pDrawingCache->addSprite(myLessButtonAnim.c_str(), svCenter.x, svCenter.y - svSliderSize.y/2.0 - fButtonSpacing - myButtonSize.y/2.0, fFinalOpac, 0, fVeryFinalScale,1.0, true);
			pDrawingCache->addSprite(myMoreButtonAnim.c_str(), svCenter.x, svCenter.y + svSliderSize.y/2.0 + fButtonSpacing + myButtonSize.y/2.0, fFinalOpac, 0, fVeryFinalScale,1.0, true);
		}

		FLOAT_TYPE fFinalKnobPos = svCenter.y - svSliderSize.y/2.0 + iPixelOffsetHor;
		if(bHaveButtons)
		{
			FLOAT_TYPE fMinKnobVisualPos, fMaxKnobVisualPos;
			fMinKnobVisualPos = svCenter.y - svSliderSize.y/2.0 + myKnobSize.y/2.0;
			if(fFinalKnobPos < fMinKnobVisualPos)
				fFinalKnobPos = fMinKnobVisualPos;
			fMaxKnobVisualPos = svCenter.y + svSliderSize.y/2.0 - myKnobSize.y/2.0;
			if(fFinalKnobPos > fMaxKnobVisualPos)
				fFinalKnobPos = fMaxKnobVisualPos;
		}

		pDrawingCache->addSprite(theLocalSharedString.c_str(), svCenter.x, fFinalKnobPos, fFinalOpac, 0, fVeryFinalScale,1.0, true);

		if(myActiveSliderPart == SliderPartKnob && myValueDisplayAnim.length() > 0)
		{
			// Ahtung!
			_ASSERT(0);
		}

	}
	else
	{	
		iPixelOffsetHor = (FLOAT_TYPE)svSliderSize.x*fPercProgress;

		// Now, draw the buttons, if any.
		if(bHaveButtons)
		{
			pDrawingCache->addSprite(myLessButtonAnim.c_str(), svCenter.x - svSliderSize.x/2.0 - fButtonSpacing - myButtonSize.x/2.0, svCenter.y, fFinalOpac, 0, fVeryFinalScale,1.0, true);
			pDrawingCache->addSprite(myMoreButtonAnim.c_str(), svCenter.x + svSliderSize.x/2.0 + fButtonSpacing + myButtonSize.x/2.0, svCenter.y, fFinalOpac, 0, fVeryFinalScale,1.0, true);
		}

		// Knob
		FLOAT_TYPE fFinalKnobPos = svCenter.x - svSliderSize.x/2.0 + iPixelOffsetHor;
		if(bHaveButtons)
		{
			FLOAT_TYPE fMinKnobVisualPos, fMaxKnobVisualPos;
			fMinKnobVisualPos = svCenter.x - svSliderSize.x/2.0 + myKnobSize.x/2.0;
			if(fFinalKnobPos < fMinKnobVisualPos)
				fFinalKnobPos = fMinKnobVisualPos;
			fMaxKnobVisualPos = svCenter.x + svSliderSize.x/2.0 - myKnobSize.x/2.0;
			if(fFinalKnobPos > fMaxKnobVisualPos)
				fFinalKnobPos = fMaxKnobVisualPos;
		}
		pDrawingCache->addSprite(theLocalSharedString.c_str(), fFinalKnobPos, svCenter.y, fFinalOpac, 0, fVeryFinalScale,1.0, true);

		// Render the slider value if we're dragged
		GTIME lTime = Application::getInstance()->getGlobalTime(this->getClockType());
		FLOAT_TYPE fValueOp = myValueDisplayOpacity.getValue();
		if(fValueOp > 0.0 && myValueDisplayAnim.length() > 0)
		{
			pDrawingCache->flush();
			pDrawingCache->addSprite(myValueDisplayAnim.c_str(), svCenter.x - svSliderSize.x/2.0 + iPixelOffsetHor, svCenter.y + myValueDisplayYOffset, fFinalOpac*fValueOp, 0, fVeryFinalScale,1.0, true);
			char pcsBuffer[256];
			theLocalSharedString = "%d";
			if(this->doesPropertyExist(PropertyTextFormat))
				theLocalSharedString = this->getStringProp(PropertyTextFormat);
			if(theLocalSharedString.find("d") != string::npos)
			{
				int iVal = fCurrProg*myValueDisplayMult;
				sprintf(pcsBuffer, theLocalSharedString.c_str(), iVal);
			}
			else
			{
				FLOAT_TYPE fVal = fCurrProg*myValueDisplayMult;
				sprintf(pcsBuffer, theLocalSharedString.c_str(), fVal);
			}
			SColor scolSelCol;
			this->getAsColor(PropertySelectedTextColor, scolSelCol);			
			scolSelCol.alpha = fFinalOpac*fValueOp;
			pDrawingCache->addText(pcsBuffer,  getCachedFont(), getCachedFontSize(), svCenter.x - svSliderSize.x/2.0 + iPixelOffsetHor + this->getNumProp(PropertyTextOffsetX), 
				svCenter.y + myValueDisplayYOffset + this->getNumProp(PropertyTextOffsetY), scolSelCol, HorAlignCenter);
		}
	}

	if(this->doesPropertyExist(PropertySliderMarkParms))
	{
		// Draw the marks
		FLOAT_TYPE fMarkSpacing = this->getNumericEnumPropValue(PropertySliderMarkParms, 0);
		int iNthLargeMark = (int)this->getNumericEnumPropValue(PropertySliderMarkParms, 1);
		SColor scolMarks;
		this->getAsColor(PropertySliderMarkColor, scolMarks);

		FLOAT_TYPE fSmallLength = upToScreen(1.0);
		FLOAT_TYPE fSmallThickness = upToScreen(0.4);
		FLOAT_TYPE fLargeLength = upToScreen(1.5);
		FLOAT_TYPE fLargeThickness = upToScreen(0.9);

		FLOAT_TYPE fMarkCenterOffset = upToScreen(5);

		if(myIsVertical)
		{
			theCommonString = "notchesAnim:3";
			theCommonString2 = "notchesAnim:4";
			if(myStartInMiddle)
			{
				int iCount = 0;
				FLOAT_TYPE fStart, fEnd = svCenter.y + svSliderSize.y/2.0;
				for(fStart = svCenter.y; fStart <= fEnd; fStart += fMarkSpacing, iCount++ )
				{
					if(iNthLargeMark > 0 && iCount % iNthLargeMark == 0)
					{
						pDrawingCache->addSprite(theCommonString2.c_str(), svCenter.x - fMarkCenterOffset, fStart, fOpacity, 0, 1.0, 1.0, true);
						pDrawingCache->addSprite(theCommonString2.c_str(), svCenter.x + fMarkCenterOffset, fStart, fOpacity, 0, 1.0, 1.0, true);
// 						pDrawingCache->addLine(svCenter.x - fMarkCenterOffset - fLargeLength, fStart, svCenter.x - fMarkCenterOffset + fLargeLength, fStart, scolMarks, fLargeThickness);
// 						pDrawingCache->addLine(svCenter.x + fMarkCenterOffset - fLargeLength, fStart, svCenter.x + fMarkCenterOffset + fLargeLength, fStart, scolMarks, fLargeThickness);
					}
					else
					{
						pDrawingCache->addSprite(theCommonString.c_str(), svCenter.x - fMarkCenterOffset, fStart, fOpacity, 0, 1.0, 1.0, true);
						pDrawingCache->addSprite(theCommonString.c_str(), svCenter.x + fMarkCenterOffset, fStart, fOpacity, 0, 1.0, 1.0, true);
// 						pDrawingCache->addLine(svCenter.x - fMarkCenterOffset - fSmallLength, fStart, svCenter.x - fMarkCenterOffset + fSmallLength, fStart, scolMarks, fSmallThickness);
// 						pDrawingCache->addLine(svCenter.x + fMarkCenterOffset - fSmallLength, fStart, svCenter.x + fMarkCenterOffset + fSmallLength, fStart, scolMarks, fSmallThickness);
					}
				}

				iCount = 0;
				fEnd = svCenter.y - svSliderSize.y/2.0;
				for(fStart = svCenter.y; fStart >= fEnd; fStart -= fMarkSpacing, iCount++ )
				{
					if(iNthLargeMark > 0 && iCount % iNthLargeMark == 0)
					{
						pDrawingCache->addSprite(theCommonString2.c_str(), svCenter.x - fMarkCenterOffset, fStart, fOpacity, 0, 1.0, 1.0, true);
						pDrawingCache->addSprite(theCommonString2.c_str(), svCenter.x + fMarkCenterOffset, fStart, fOpacity, 0, 1.0, 1.0, true);
// 						pDrawingCache->addLine(svCenter.x - fMarkCenterOffset - fLargeLength, fStart, svCenter.x - fMarkCenterOffset + fLargeLength, fStart, scolMarks, fLargeThickness);
// 						pDrawingCache->addLine(svCenter.x + fMarkCenterOffset - fLargeLength, fStart, svCenter.x + fMarkCenterOffset + fLargeLength, fStart, scolMarks, fLargeThickness);
					}
					else
					{
						pDrawingCache->addSprite(theCommonString.c_str(), svCenter.x - fMarkCenterOffset, fStart, fOpacity, 0, 1.0, 1.0, true);
						pDrawingCache->addSprite(theCommonString.c_str(), svCenter.x + fMarkCenterOffset, fStart, fOpacity, 0, 1.0, 1.0, true);
// 						pDrawingCache->addLine(svCenter.x - fMarkCenterOffset - fSmallLength, fStart, svCenter.x - fMarkCenterOffset + fSmallLength, fStart, scolMarks, fSmallThickness);
// 						pDrawingCache->addLine(svCenter.x + fMarkCenterOffset - fSmallLength, fStart, svCenter.x + fMarkCenterOffset + fSmallLength, fStart, scolMarks, fSmallThickness);
					}
				}

			}
			else
			{
				int iCount = 0;
				FLOAT_TYPE fStart, fEnd = svCenter.x + svSliderSize.x/2.0;
				for(fStart = svCenter.x - svSliderSize.x/2.0; fStart <= fEnd; fStart += fMarkSpacing, iCount++ )
				{
					if(iNthLargeMark > 0 && iCount % iNthLargeMark == 0)
					{
						pDrawingCache->addSprite(theCommonString2.c_str(), svCenter.x - fMarkCenterOffset, fStart, fOpacity, 0, 1.0, 1.0, true);
						pDrawingCache->addSprite(theCommonString2.c_str(), svCenter.x + fMarkCenterOffset, fStart, fOpacity, 0, 1.0, 1.0, true);
// 						pDrawingCache->addLine(svCenter.x - fMarkCenterOffset - fLargeLength, fStart, svCenter.x - fMarkCenterOffset + fLargeLength, fStart, scolMarks, fLargeThickness);
// 						pDrawingCache->addLine(svCenter.x + fMarkCenterOffset - fLargeLength, fStart, svCenter.x + fMarkCenterOffset + fLargeLength, fStart, scolMarks, fLargeThickness);
					}
					else
					{
						pDrawingCache->addSprite(theCommonString.c_str(), svCenter.x - fMarkCenterOffset, fStart, fOpacity, 0, 1.0, 1.0, true);
						pDrawingCache->addSprite(theCommonString.c_str(), svCenter.x + fMarkCenterOffset, fStart, fOpacity, 0, 1.0, 1.0, true);
// 						pDrawingCache->addLine(svCenter.x - fMarkCenterOffset - fSmallLength, fStart, svCenter.x - fMarkCenterOffset + fSmallLength, fStart, scolMarks, fSmallThickness);
// 						pDrawingCache->addLine(svCenter.x + fMarkCenterOffset - fSmallLength, fStart, svCenter.x + fMarkCenterOffset + fSmallLength, fStart, scolMarks, fSmallThickness);
					}
				}
			}
		}
		else
		{
			theCommonString = "notchesAnim:2";
			theCommonString2 = "notchesAnim:1";
			if(myStartInMiddle)
			{
				int iCount = 0;
				FLOAT_TYPE fStart, fEnd = svCenter.x + svSliderSize.x/2.0;
				for(fStart = svCenter.x; fStart <= fEnd; fStart += fMarkSpacing, iCount++ )
				{
					if(iNthLargeMark > 0 && iCount % iNthLargeMark == 0)
					{
// 						pDrawingCache->addLine(fStart, svCenter.y - fMarkCenterOffset - fLargeLength, fStart, svCenter.y - fMarkCenterOffset + fLargeLength, scolMarks, fLargeThickness);
// 						pDrawingCache->addLine(fStart, svCenter.y + fMarkCenterOffset - fLargeLength, fStart, svCenter.y + fMarkCenterOffset + fLargeLength, scolMarks, fLargeThickness);
						pDrawingCache->addSprite(theCommonString2.c_str(), fStart, svCenter.y - fMarkCenterOffset, fOpacity, 0, 1.0, 1.0, true);
						pDrawingCache->addSprite(theCommonString2.c_str(), fStart, svCenter.y + fMarkCenterOffset, fOpacity, 0, 1.0, 1.0, true);
					}
					else
					{
						pDrawingCache->addSprite(theCommonString.c_str(), fStart, svCenter.y - fMarkCenterOffset, fOpacity, 0, 1.0, 1.0, true);
						pDrawingCache->addSprite(theCommonString.c_str(), fStart, svCenter.y + fMarkCenterOffset, fOpacity, 0, 1.0, 1.0, true);
// 						pDrawingCache->addLine(fStart, svCenter.y - fMarkCenterOffset - fSmallLength, fStart, svCenter.y - fMarkCenterOffset + fSmallLength, scolMarks, fSmallThickness);
// 						pDrawingCache->addLine(fStart, svCenter.y + fMarkCenterOffset - fSmallLength, fStart, svCenter.y + fMarkCenterOffset + fSmallLength, scolMarks, fSmallThickness);
					}
				}

				iCount = 0;
				fEnd = svCenter.x - svSliderSize.x/2.0;
				for(fStart = svCenter.x; fStart >= fEnd; fStart -= fMarkSpacing, iCount++ )
				{
					if(iNthLargeMark > 0 && iCount % iNthLargeMark == 0)
					{
// 						pDrawingCache->addLine(fStart, svCenter.y - fMarkCenterOffset - fLargeLength, fStart, svCenter.y - fMarkCenterOffset + fLargeLength, scolMarks, fLargeThickness);
// 						pDrawingCache->addLine(fStart, svCenter.y + fMarkCenterOffset - fLargeLength, fStart, svCenter.y + fMarkCenterOffset + fLargeLength, scolMarks, fLargeThickness);
						pDrawingCache->addSprite(theCommonString2.c_str(), fStart, svCenter.y - fMarkCenterOffset, fOpacity, 0, 1.0, 1.0, true);
						pDrawingCache->addSprite(theCommonString2.c_str(), fStart, svCenter.y + fMarkCenterOffset, fOpacity, 0, 1.0, 1.0, true);
					}
					else
					{
						pDrawingCache->addSprite(theCommonString.c_str(), fStart, svCenter.y - fMarkCenterOffset, fOpacity, 0, 1.0, 1.0, true);
						pDrawingCache->addSprite(theCommonString.c_str(), fStart, svCenter.y + fMarkCenterOffset, fOpacity, 0, 1.0, 1.0, true);
// 						pDrawingCache->addLine(fStart, svCenter.y - fMarkCenterOffset - fSmallLength, fStart, svCenter.y - fMarkCenterOffset + fSmallLength, scolMarks, fSmallThickness);
// 						pDrawingCache->addLine(fStart, svCenter.y + fMarkCenterOffset - fSmallLength, fStart, svCenter.y + fMarkCenterOffset + fSmallLength, scolMarks, fSmallThickness);
					}
				}

			}
			else
			{
				int iCount = 0;
				FLOAT_TYPE fStart, fEnd = svCenter.x + svSliderSize.x/2.0;
				for(fStart = svCenter.x - svSliderSize.x/2.0; fStart <= fEnd; fStart += fMarkSpacing, iCount++ )
				{
					if(iNthLargeMark > 0 && iCount % iNthLargeMark == 0)
					{
// 						pDrawingCache->addLine(fStart, svCenter.y - fMarkCenterOffset - fLargeLength, fStart, svCenter.y - fMarkCenterOffset + fLargeLength, scolMarks, fLargeThickness);
// 						pDrawingCache->addLine(fStart, svCenter.y + fMarkCenterOffset - fLargeLength, fStart, svCenter.y + fMarkCenterOffset + fLargeLength, scolMarks, fLargeThickness);
						pDrawingCache->addSprite(theCommonString2.c_str(), fStart, svCenter.y - fMarkCenterOffset, fOpacity, 0, 1.0, 1.0, true);
						pDrawingCache->addSprite(theCommonString2.c_str(), fStart, svCenter.y + fMarkCenterOffset, fOpacity, 0, 1.0, 1.0, true);
					}
					else
					{
// 						pDrawingCache->addLine(fStart, svCenter.y - fMarkCenterOffset - fSmallLength, fStart, svCenter.y - fMarkCenterOffset + fSmallLength, scolMarks, fSmallThickness);
// 						pDrawingCache->addLine(fStart, svCenter.y + fMarkCenterOffset - fSmallLength, fStart, svCenter.y + fMarkCenterOffset + fSmallLength, scolMarks, fSmallThickness);
						pDrawingCache->addSprite(theCommonString.c_str(), fStart, svCenter.y - fMarkCenterOffset, fOpacity, 0, 1.0, 1.0, true);
						pDrawingCache->addSprite(theCommonString.c_str(), fStart, svCenter.y + fMarkCenterOffset, fOpacity, 0, 1.0, 1.0, true);
					}
				}
			}
		}
	}

}
/*****************************************************************************/
SliderPartType UISliderElement::getClickedPart(int iX, int iY, FLOAT_TYPE* fKnobValueOut)
{
	SliderPartType eRes = SliderPartNone;
	SVector2D svTestPoint(iX, iY);
	SVector2D svScroll(0,0);

	if(fKnobValueOut)
		*fKnobValueOut = myMinValue;

	this->getFullBaseAnimName(theLocalSharedString2);
	// Empty slider anim forgotten if fails
	_ASSERT(theLocalSharedString2.length() > 0);

	// Now, see if we have full slider anim. If we do, just
	// render progress. If not, just render simple slider.
	bool bHaveButtons = false;
	if(myButtonSize.x > 0 && myButtonSize.y > 0 &&
		myLessButtonAnim.length() > 0 && myMoreButtonAnim.length() > 9)
		bHaveButtons = true;

	// Finally, draw the slider
	SVector2D svPos;
	FLOAT_TYPE fGlobalScale;
	this->getGlobalPosition(svPos, NULL, &fGlobalScale);

	SVector2D svOwnSize;
	this->getBoxSize(svOwnSize);
	svOwnSize *= fGlobalScale;

	// Progress
	FLOAT_TYPE fPercProgress = 0.0;
	FLOAT_TYPE fNormProgress;
	if(fabs(myMaxValue - myMinValue) > FLOAT_EPSILON)
	{
		FLOAT_TYPE fCurrProg = getValue();
		fPercProgress = valueToPercProgress(fCurrProg);
		//fPercProgress = (fCurrProg - myMinValue)/(myMaxValue - myMinValue);

		if(myStartInMiddle)
			fNormProgress = (fPercProgress - 0.5)*2.0;
		else
			fNormProgress = fPercProgress;

	}

	if(fPercProgress < 0.0)
		fPercProgress = 0;
	if(fPercProgress > 1.0)
		fPercProgress = 1.0;

	if(!myStartInMiddle)
		fNormProgress = fPercProgress;
	else
	{
		if(fNormProgress < -1.0)
			fNormProgress = -1.0;
		if(fNormProgress > 1.0)
			fNormProgress = 1.0;
	}


	SVector2D svCenter(svPos.x + svScroll.x, svPos.y + svScroll.y);

	// Now, render the slider pos.
	// For this, we need to know how large our slider bitmap is...
	int iSliderW, iSliderH;
	getTextureManager()->getTextureRealDims(theLocalSharedString2.c_str(), iSliderW, iSliderH);
	SVector2D svSliderSize;
	if(myIsSizable)
	{
		getBoxSize(svSliderSize);
		if(!bHaveButtons)
		{
			// We need this because if we only have a slider and no buttons,
			// and the slider is in the 0 or 1 position, 
			// it's impossible to grab the half of the slider outside
			// of the box.
			svSliderSize.x -= myKnobSize.x*SLIDER_MULT_HOR_PADDING;
		}
		svSliderSize *= fGlobalScale;
	}
	else
	{
		svSliderSize.x = (FLOAT_TYPE)iSliderW*fGlobalScale;
		svSliderSize.y = (FLOAT_TYPE)iSliderH*fGlobalScale;
	}
	if(bHaveButtons)
	{
		if(myIsVertical)
			svSliderSize.y -= myButtonSize.y*2;
		else
			svSliderSize.x -= myButtonSize.x*2;
	}

    
	FLOAT_TYPE fLongPartScale = 1.0;
	SRect2D srTestRect;
	if(myIsVertical)
	{
		int iPixelOffsetHor = (FLOAT_TYPE)svSliderSize.y*fPercProgress;

		// Try the buttons, if any, first. With new allowances on the 
		// slider/knob, we may always be thinking we're over the slider,
		// even when visually we're over buttons...
		if(bHaveButtons)
		{
			FLOAT_TYPE fButtonSpacing = BUTTON_SPACING*fGlobalScale;

			// Less button
			srTestRect.x = svCenter.x - svOwnSize.x/2.0;
			srTestRect.y = svCenter.y - svSliderSize.y/2.0 - fButtonSpacing - myButtonSize.y;
			srTestRect.w = svOwnSize.x;
			srTestRect.h = myButtonSize.y;

			if(srTestRect.doesContain(svTestPoint))
			{
				// We're over the less button
				eRes = SliderPartLess;
				return eRes;
			}
			else
			{
				// More button
				srTestRect.x = svCenter.x - svOwnSize.x/2.0;
				srTestRect.y = svCenter.y + svSliderSize.y/2.0 + fButtonSpacing;
				srTestRect.w = svOwnSize.x;
				srTestRect.h = myButtonSize.y;

				if(srTestRect.doesContain(svTestPoint))
				{
					// We're over the less button
					eRes = SliderPartMore;
					if(fKnobValueOut)
						*fKnobValueOut = myMaxValue;
					return eRes;
				}
			}
		}

		srTestRect.x = svCenter.x - svOwnSize.x/2.0;
		srTestRect.y = svCenter.y - svSliderSize.y/2.0 - myKnobSize.y/2.0;
		srTestRect.w = svOwnSize.x;
		srTestRect.h = svSliderSize.y + myKnobSize.y;

		if(srTestRect.doesContain(svTestPoint))
		{
			// We're over the slider. See if we're on a knob or not.
			// If not, return percentage.
			srTestRect.x = svCenter.x - svOwnSize.x/2.0;
			srTestRect.y = svCenter.y - svSliderSize.y/2.0 + iPixelOffsetHor - myKnobSize.y/2.0;		
			srTestRect.w = svOwnSize.x;
			srTestRect.h = myKnobSize.y;

			if(srTestRect.doesContain(svTestPoint))
				eRes = SliderPartKnob;
			else
				eRes = SliderPartMain;

			// If we need the location, see where it is
			if(fKnobValueOut)
			{
				*fKnobValueOut = (svTestPoint.y - (svCenter.y - svSliderSize.y/2.0))/(FLOAT_TYPE)svSliderSize.y;
				// This is currently in [0,1]. Convert to actual values.
				*fKnobValueOut = percProgressToValue(*fKnobValueOut);
				//*fKnobValueOut = (*fKnobValueOut)*(myMaxValue - myMinValue) + myMinValue;
			}

			return eRes;
		}
		else if(fKnobValueOut)
		{
			*fKnobValueOut = (svTestPoint.y - (svCenter.y - svSliderSize.y/2.0))/(FLOAT_TYPE)svSliderSize.y;
			*fKnobValueOut = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, *fKnobValueOut);
			*fKnobValueOut = percProgressToValue(*fKnobValueOut);
/*
			if(svTestPoint.y < svCenter.y)
				*fKnobValueOut = myMinValue;
			else if(svTestPoint.y > svCenter.y)
				*fKnobValueOut = myMaxValue;
			else
				*fKnobValueOut = (myMaxValue + myMinValue)*0.5;
				*/
		}
	}
	else
	{
		int iPixelOffsetHor = (FLOAT_TYPE)svSliderSize.x*fPercProgress;

		// Try the buttons, if any, first. With new allowances on the 
		// slider/knob, we may always be thinking we're over the slider,
		// even when visually we're over buttons...
		if(bHaveButtons)
		{
			FLOAT_TYPE fButtonSpacing = BUTTON_SPACING*fGlobalScale;

			// Less button
			srTestRect.x = svCenter.x - svSliderSize.x/2.0 - fButtonSpacing - myButtonSize.x;
			srTestRect.y = svCenter.y - svOwnSize.y/2.0;
			srTestRect.w = myButtonSize.x;
			srTestRect.h = svOwnSize.y;

			if(srTestRect.doesContain(svTestPoint))
			{
				// We're over the less button
				eRes = SliderPartLess;
				return eRes;
			}
			else
			{
				// More button
				srTestRect.x = svCenter.x + svSliderSize.x/2.0 + fButtonSpacing;
				srTestRect.y = svCenter.y - svOwnSize.y/2.0;
				srTestRect.w = myButtonSize.x;
				srTestRect.h = svOwnSize.y;

				if(srTestRect.doesContain(svTestPoint))
				{
					// We're over the less button
					eRes = SliderPartMore;
					if(fKnobValueOut)
						*fKnobValueOut = myMaxValue;
					return eRes;
				}
			}
		}

		// See if we're over main slider. We'll probably need position, too.
		srTestRect.x = svCenter.x - svSliderSize.x/2.0 - myKnobSize.x/2.0;
		srTestRect.y = svCenter.y - svOwnSize.y/2.0;
		srTestRect.w = svSliderSize.x + myKnobSize.x;
		srTestRect.h = svOwnSize.y;
		if(srTestRect.doesContain(svTestPoint))
		{
			// We're over the slider. See if we're on a knob or not.
			// If not, return percentage.
			srTestRect.x = svCenter.x - svSliderSize.x/2.0 + iPixelOffsetHor - myKnobSize.x/2.0;
			srTestRect.y = svCenter.y - svOwnSize.y/2.0;		
			srTestRect.w = myKnobSize.x;
			srTestRect.h = svOwnSize.y;

			if(srTestRect.doesContain(svTestPoint))
				eRes = SliderPartKnob;
			else
				eRes = SliderPartMain;

			// If we need the location, see where it is
			if(fKnobValueOut)
			{
				*fKnobValueOut = (svTestPoint.x - (svCenter.x - svSliderSize.x/2.0))/(FLOAT_TYPE)svSliderSize.x;
				// This is currently in [0,1]. Convert to actual values.
				*fKnobValueOut = percProgressToValue(*fKnobValueOut);
				//*fKnobValueOut = (*fKnobValueOut)*(myMaxValue - myMinValue) + myMinValue;
			}

			return eRes;
		}
		else if(fKnobValueOut)
		{
			*fKnobValueOut = (svTestPoint.x - (svCenter.x - svSliderSize.x/2.0))/(FLOAT_TYPE)svSliderSize.x;
			*fKnobValueOut = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, *fKnobValueOut);
			*fKnobValueOut = percProgressToValue(*fKnobValueOut);
/*

			if(svTestPoint.x < svCenter.x)
				*fKnobValueOut = myMinValue;
			else if(svTestPoint.x > svCenter.x)
				*fKnobValueOut = myMaxValue;
			else
				*fKnobValueOut = (myMaxValue + myMinValue)*0.5;
*/
		}

		
	}
	return eRes;
}
/*****************************************************************************/
void UISliderElement::onPressed(TTouchVector& vecTouches)
{
	UIElement::onPressed(vecTouches);

	if(getIsEnabled() && vecTouches.size() > 0)
	{
		myActiveSliderPart = this->getClickedPart(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y, NULL);
		
		myDragStartValue = getValue();
		if(myActiveSliderPart == SliderPartKnob)
		{
			myValueDisplayOpacity.changeAnimation(0, 1, VALUE_DISPLAY_FADE_SECONDS, getClockType());
			getUIPlane()->lockMouseCursor(this);
			if(this->getLinkedToElementWithValidTarget())
				EventManager::getInstance()->sendEvent(EventUIParmChangeBegin, this);
		}
		myAllowPressing = true;

		// Save undo...
		_ASSERT(myCurrUndoBlockId < 0);
		UIElement* pParmElem = getLinkedToElementWithValidTarget();
		if(pParmElem && UndoManager::canAcceptNewUndoNow())
			myCurrUndoBlockId = UndoManager::addUndoItemToCurrentManager(pParmElem->getUndoStringForSelfChange(), pParmElem->createUndoItemForSelfChange(), true, getParentWindow(), NULL);	
	}
}
/*****************************************************************************/
void UISliderElement::onMouseEnter(TTouchVector& vecTouches)
{
	UIElement::onMouseEnter(vecTouches);
	if(getIsEnabled() && myActiveSliderPart == SliderPartKnob)
	{
		GTIME lTime = Application::getInstance()->getGlobalTime(this->getClockType());
		if(myValueDisplayOpacity.getValue() < 1.0)
			myValueDisplayOpacity.changeAnimation(0, 1, VALUE_DISPLAY_FADE_SECONDS, getClockType());

		if(this->getLinkedToElementWithValidTarget())
			EventManager::getInstance()->sendEvent(EventUIParmChangeBegin, this);
	}
}
/*****************************************************************************/
void UISliderElement::onMouseLeave(TTouchVector& vecTouches)
{
	UIElement::onMouseLeave(vecTouches);	

	if(getIsEnabled() && myActiveSliderPart == SliderPartKnob && this->getLinkedToElementWithValidTarget())
		EventManager::getInstance()->sendEvent(EventUIParmChangeEnd, this);

	if(getIsEnabled() && myActiveSliderPart != SliderPartKnob)
	{
		myActiveSliderPart = SliderPartNone;
		getUIPlane()->unlockMouseCursor();
		myAllowPressing = false;
	}


	GTIME lTime = Application::getInstance()->getGlobalTime(this->getClockType());
	if(myValueDisplayOpacity.getValue() > 0)
		myValueDisplayOpacity.changeAnimation(1, 0, VALUE_DISPLAY_FADE_SECONDS, getClockType());

	finishUndo();
}
/*****************************************************************************/
void UISliderElement::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{
	UIElement::onReleased(vecTouches, bIgnoreActions);

	if(getIsEnabled())
	{
		if(myActiveSliderPart == SliderPartKnob && this->getLinkedToElementWithValidTarget())
			EventManager::getInstance()->sendEvent(EventUIParmChangeEnd, this);
		getUIPlane()->unlockMouseCursor();
		if(myAllowPressing)
		{
			FLOAT_TYPE fReleasedValue;
			SliderPartType eFinalPart = this->getClickedPart(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y, &fReleasedValue);

			// If we were dragging the knob, set the new final value.
			if((myActiveSliderPart == SliderPartKnob || 
				myActiveSliderPart == SliderPartMain) && 
				(eFinalPart == SliderPartMain || eFinalPart == SliderPartKnob))
			{
				setValueSafe(fReleasedValue, myActiveSliderPart == SliderPartMain, NULL, false);				
			}
			else if(myActiveSliderPart == SliderPartLess)
			{
				// Decrease the value by a given step
				FLOAT_TYPE fNewValue = getValue() - myButtonValueStep;
				setValueSafe(fNewValue, true, NULL, false);
			}
			else if(myActiveSliderPart == SliderPartMore)
			{
				// Increase the slider value by a given step
				FLOAT_TYPE fNewValue = getValue() + myButtonValueStep;
				setValueSafe(fNewValue, true, NULL, false);
			}

			myDragStartValue = getValue();

			if(myCallbacks)
			{
				myIsInCallback = true;
				myCallbacks->onSliderValueChanged(this);
				myIsInCallback = false;
			}
		}
//		else
//			this->setPushed(false);
	}

	GTIME lTime = Application::getInstance()->getGlobalTime(this->getClockType());
	if(myValueDisplayOpacity.getValue() > 0)
		myValueDisplayOpacity.changeAnimation(1, 0, VALUE_DISPLAY_FADE_SECONDS, getClockType());

	myActiveSliderPart = SliderPartNone;
	myAllowPressing = false;
	finishUndo();
}
/*****************************************************************************/
void UISliderElement::onMouseMove(TTouchVector& vecTouches)
{
#ifdef _DEBUG
	if(vecTouches.size() > 0 && vecTouches[0].myPoint.x >= 820)
	{
		int bp = 0;
	}
#endif
	UIElement::onMouseMove(vecTouches);
	if(getIsEnabled() && vecTouches.size() > 0)
	{
		if(myAllowPressing && myActiveSliderPart == SliderPartKnob)
		{
			// Get the new position of the slider
			FLOAT_TYPE fTempVal;
			SliderPartType eSlidePart = this->getClickedPart(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y, &fTempVal);

			// If we're moving the slider, make sure we continue to do so
			if(myActiveSliderPart == SliderPartKnob)
				eSlidePart = SliderPartKnob;

			if(eSlidePart == SliderPartMain || eSlidePart == SliderPartKnob)
			{
				setValueSafe(fTempVal, false, NULL, true);
			}

			if(myCallbacks)
			{
				myIsInCallback = true;
				myCallbacks->onSliderValueChanged(this);
				myIsInCallback = false;
			}
		}

		// Otherwise, we don't really care when the mouse moves.
	}
}
/*****************************************************************************/
FLOAT_TYPE UISliderElement::getValue(void) 
{ 
	GTIME lTime = Application::getInstance()->getGlobalTime(this->getClockType());
	return myCurrValue.getValue();
}
/*****************************************************************************/
void UISliderElement::onTimerTick(GTIME lGlobalTime)
{
	UIElement::onTimerTick(lGlobalTime);
//	myCurrValue.checkTime(lGlobalTime);
	//myValueDisplayOpacity.checkTime(lGlobalTime);
	if(myCurrValue.getIsAnimating(false))
	{
		// Callback
		if(myCallbacks)
		{
			myIsInCallback = true;
			myCallbacks->onSliderValueChanged(this);
			myIsInCallback = false;
		}
	}
}
/*****************************************************************************/
bool UISliderElement::getIsSliderBeingAnimated(void)
{
	//GTIME lTime = Application::getInstance()->getGlobalTime(this->getClockType());
	return myCurrValue.getIsAnimating(false);
}
/*****************************************************************************/
void UISliderElement::changeValueTo(FLOAT_TYPE fValue, UIElement* pOptSourceElem, bool bAnimate, bool bIsChangingContinuously)
{
	if(myIsCallingChangeValue)
		return;

	// Note that will be called from the set value safe.
	//UIElement::changeValueTo(fValue);

	setValueSafe(fValue, bAnimate, pOptSourceElem, bIsChangingContinuously);
}
/*****************************************************************************/
void UISliderElement::finishUndo()
{
	if(myCurrUndoBlockId >= 0)
	{
		UndoManager::endUndoBlockInCurrentManager(myCurrUndoBlockId, getParentWindow(), NULL);
		myCurrUndoBlockId = -1;
	}
}
/*****************************************************************************/
FLOAT_TYPE UISliderElement::percProgressToValue(FLOAT_TYPE fPercProgress)
{
	if(this->getBoolProp(PropertyIsLogarithmic))
	{
		///FLOAT_TYPE fLogInterp = log10(fPercProgress*LOG_SLIDER_DIVISOR + 1)/log10(LOG_SLIDER_DIVISOR + 1);
		FLOAT_TYPE fLogInterp = (pow(10, fPercProgress*log10(LOG_SLIDER_DIVISOR + 1)) - 1)/LOG_SLIDER_DIVISOR;
		return fLogInterp*(myMaxValue - myMinValue) + myMinValue;
	}
	else
		return fPercProgress*(myMaxValue - myMinValue) + myMinValue;
}
/*****************************************************************************/
FLOAT_TYPE UISliderElement::valueToPercProgress(FLOAT_TYPE fValue)
{
	if(this->getBoolProp(PropertyIsLogarithmic))
	{
		FLOAT_TYPE fNormValue = (fValue - myMinValue)/(myMaxValue - myMinValue);
		return log10(fNormValue*LOG_SLIDER_DIVISOR + 1)/log10(LOG_SLIDER_DIVISOR + 1);
		//FLOAT_TYPE fInterp = pow(10, fNormValue*log10(LOG_SLIDER_DIVISOR + 1)) - 1;
		//return fInterp/LOG_SLIDER_DIVISOR;
	}
	else
		return (fValue - myMinValue)/(myMaxValue - myMinValue);

}
/*****************************************************************************/
bool UISliderElement::onMouseWheel(FLOAT_TYPE fDelta)
{
	// Find if we're related to anything
	// Note that if we start asserting in UIElement::applyMouseWheelDelta()
	// we can just return true here to never scroll if we're over a slider.
	if(!doesPropertyExist(PropertyLinkTo))
		return false;

	// Otherwise, get that element and call its onMouseWheel().
	UIElement* pLinkedElem = this->getElementAtPath(this->getStringProp(PropertyLinkTo));
	if(!pLinkedElem)
		ASSERT_RETURN_FALSE;

	return pLinkedElem->onMouseWheel(fDelta);
}
/*****************************************************************************/
bool UISliderElement::getAllowValuePropagation(FLOAT_TYPE fNewValue, bool bIsChangingContinuously, UIElement* pOptSourceElem, UIElement* pLinkedToElem)
{
	// We don't propagate the value if we are a table's slider.
	// Otherwise, we may start marking nodes dirty every slide...
	UITableElement* pTable = as<UITableElement>(pLinkedToElem);
	if(pTable && pTable->getRelatedTableSlider() == this)
			return false;

	return UIElement::getAllowValuePropagation(fNewValue, bIsChangingContinuously, pOptSourceElem, pLinkedToElem);
}
/*****************************************************************************/
void UISliderElement::resetEvalCache(bool bRecursive)
{
	UIElement::resetEvalCache(bRecursive);
	AnimSequenceAddon::resetEvalCache();
}
/*****************************************************************************/
};