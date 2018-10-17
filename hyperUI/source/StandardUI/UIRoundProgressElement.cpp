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

#define VALUE_ANIM_TIME				0.35
#define VALUE_ANIM_OFFSET_TIME		0.25

namespace HyperUI
{
/*****************************************************************************/
UIRoundProgressElement::UIRoundProgressElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIRoundProgressElement::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
	myMinProgress = 0.0;
	myMaxProgress = 1.0;
	myCurrProgress = 0.0;
	myAnim.setNonAnimValue(0);
}
/*****************************************************************************/
void UIRoundProgressElement::stopCurrProgAnim()
{
	GTIME lTime = Application::getInstance()->getGlobalTime(getClockType());
	myAnim.setNonAnimValue(myAnim.getValue());
}
/*****************************************************************************/
void UIRoundProgressElement::setProgress(FLOAT_TYPE fValue, bool bAnimated, FLOAT_TYPE fOverrideTime, bool bContinueFromCurrentAnim, FLOAT_TYPE fOffsetTime)
{
	myCurrProgress = fValue;
	if(bAnimated)
	{
		if(bContinueFromCurrentAnim)
		{
			GTIME lTime = Application::getInstance()->getGlobalTime(getClockType());
			FLOAT_TYPE fCurrVal = myAnim.getValue();
			FLOAT_TYPE fFullTime = fOverrideTime > 0 ? fOverrideTime : VALUE_ANIM_TIME;
			FLOAT_TYPE fRemTime = fFullTime*(1.0 - fCurrVal);
			myAnim.setAnimation(fCurrVal, 1, fRemTime, getClockType());
			//myAnim.setAnimation(fCurrVal, 1, fRemTime, getClockType(), AnimOverActionNone, NULL, fOffsetTime < 0 ? VALUE_ANIM_OFFSET_TIME : fOffsetTime);
		}
		else
			myAnim.setAnimation(0, 1, fOverrideTime > 0 ? fOverrideTime : VALUE_ANIM_TIME, getClockType());
			//myAnim.setAnimation(0, 1, fOverrideTime > 0 ? fOverrideTime : VALUE_ANIM_TIME, getClockType(), AnimOverActionNone, NULL, fOffsetTime < 0 ? VALUE_ANIM_OFFSET_TIME : fOffsetTime);
	}
	else
	{
		myAnim.setNonAnimValue(1.0);
	}
}
/*****************************************************************************/
void UIRoundProgressElement::onPreRenderChildren(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	UIElement::onPreRenderChildren(svScroll, fOpacity, fScale);

	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	this->getLocalPosition(svPos, &fFinalOpac, &fLocScale); // , &fLocRotAngle);
	fFinalOpac *= fOpacity;
	svPos += svScroll;

	getDrawingCache()->flush();

	// Render our actual progress
	GTIME lTime = Application::getInstance()->getGlobalTime(getClockType());
	FLOAT_TYPE fAnim = myAnim.getValue();
	renderProgressAnimInternal(svPos, fAnim, fFinalOpac, fScale);

	// Now render our top cap anim, if applicable:
	if(this->doesPropertyExist(PropertyCapImage))
	{
		theCommonString = this->getStringProp(PropertyCapImage);
		getDrawingCache()->addSprite(theCommonString.c_str(), svPos.x, svPos.y, fFinalOpac, 0, fScale, 1.0, true);
	}
}
/*****************************************************************************/
void UIRoundProgressElement::renderProgressAnimInternal(SVector2D& svCenter, FLOAT_TYPE fAnimValue, FLOAT_TYPE fFinalOpacity, FLOAT_TYPE fScale)
{
	FLOAT_TYPE fProgress = (myCurrProgress - myMinProgress)/(myMaxProgress - myMinProgress);
	fProgress *= fAnimValue;
	theCommonString = this->getStringProp(PropertyObjThirdAnim);
	FLOAT_TYPE fRadius = getTextureManager()->getFileWidth(theCommonString.c_str())*fScale*0.5;
	RenderUtils::renderCircularProgress(getParentWindow(), fProgress, theCommonString.c_str(), svCenter.x, svCenter.y, fRadius, fFinalOpacity);
}
/*****************************************************************************/
};