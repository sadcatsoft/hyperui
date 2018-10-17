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

#ifdef _DEBUG
//#define DEBUG_ANIM_VALUE
#endif

#define BEZIER_CONTROL_OFFSET		0.9
#define BEZIER_CONTROLLER_SAVE_TAG		STR_LIT("svBezController")

namespace HyperUI
{
/*****************************************************************************/
AnimatedValue::AnimatedValue()
{
	init();
}
/*****************************************************************************/
AnimatedValue::AnimatedValue(const AnimatedValue& rSource)
{
	init();
	copyFrom(rSource);
}
/*****************************************************************************/
void AnimatedValue::init()
{
	myDidReportAnimationOverToActivityManager = true;
	myClockType = ClockLastPlaceholder;
	myController = NULL;
//	myObeyScrolling = false;
	myDidReportAnimationOver = false;
	myCallback = NULL;
	myAnimOverAction = AnimOverActionNone;
	setNonAnimValue(0.0);
	myLastEvalTime = 0;
	myLastValue = 0;
	myCallbackData = NULL;

	myStartInterpType = InterpLinear;
	myEndInterpType = InterpLinear;
}
/*****************************************************************************/
AnimatedValue::~AnimatedValue()
{
	ensureAnimOverReportedToActivityManager();

	resetController();

	delete myCallbackData;
	myCallbackData = NULL;
}
/*****************************************************************************/
void AnimatedValue::setCallbackData(const CHAR_TYPE* pcsData)
{
	if(pcsData)
	{
		if(!myCallbackData)
			myCallbackData = new STRING_TYPE;
		*myCallbackData = pcsData;
	}
	else if(myCallbackData)
	{
		delete myCallbackData;
		myCallbackData = NULL;
	}
}
/********************************************************************************************/
void AnimatedValue::setCallbacks(AnimationOverCallback *pCallback, const CHAR_TYPE* pcsData)
{
#ifdef DEBUG_ANIM_VALUE
	gLog("ANIMVALUE: %x setCallback callback = %x\n", this, pCallback);
#endif
	myCallback = pCallback;
	setCallbackData(pcsData);
}
/********************************************************************************************/
void AnimatedValue::setNonAnimValue(FLOAT_TYPE dValue)
{
	if(!myDidReportAnimationOverToActivityManager)
	{
		ActivityManager::getInstance()->endAnimation(getDebugName());
		myDidReportAnimationOverToActivityManager = true;
	}

	myStartTime = 0;
	myEndTime = 1;
	myStartValue = myEndValue = dValue;	

	myClockType = ClockLastPlaceholder;
	myLastEvalTime = 0;

	// What if we're in a point and it owns the controller?
	resetController();

	ActivityManager::getInstance()->singleValueChanged();
}
/*****************************************************************************/
void AnimatedValue::resetController(void)
{
	if(myController && myDoOwnController)
		delete myController;
	myController = NULL;
	myDoOwnController = false;
}
/*****************************************************************************/
FLOAT_TYPE AnimatedValue::getValue() const
{
	if(myStartValue == myEndValue)
	{
		myLastValue = myStartValue;
		return myStartValue;
	}

	_ASSERT(myClockType != ClockLastPlaceholder);
	GTIME currTime = Application::getInstance()->getGlobalTime(myClockType);

	if(myLastEvalTime >= currTime)
		return myLastValue;

	///if(myAnimOverAction != AnimOverActionNone && !myDidReportAnimationOver)
	bool bDontContinueLogic = myAnimOverAction == AnimOverActionNone || myDidReportAnimationOver;
	if(!(myDidReportAnimationOverToActivityManager && bDontContinueLogic))
		const_cast<AnimatedValue*>(this)->checkTime(currTime);
	
	myLastEvalTime = currTime;
	
	if(myController)
	{
		// We have a controller. Eval it instead.
		myLastValue = (myController->evaluate(currTime)[myControllerResIdx]);
		return myLastValue;
	}
	
	if(currTime <= myStartTime)
	{
		myLastValue = myStartValue;
		return myLastValue;
	}
	else if(currTime >= myEndTime)
	{
		myLastValue = myEndValue;
		return myLastValue;
	}

	FLOAT_TYPE dInterp = (FLOAT_TYPE)(currTime - myStartTime)/(FLOAT_TYPE)(myEndTime - myStartTime);
	if(myStartInterpType == InterpLinear && myEndInterpType == InterpLinear)
	{		
		myLastValue = (myEndValue - myStartValue)*dInterp + myStartValue;
	}
	else
	{
		// We have some other sort of interp, currently Bezier. Construct a curve
		// and evaluate it at the right point.
		FLOAT_TYPE fTemp;
		SVector2D svPoints[4];
		// Start
		svPoints[0].set(myStartValue, 0);
		
		// First control point
		//fTemp = (myEndValue - myStartValue)*BEZIER_CONTROL_OFFSET + myStartValue;
		fTemp = BEZIER_CONTROL_OFFSET;
		if(myStartInterpType == InterpLinear)
		{
			//svPoints[1].set(fTemp, BEZIER_CONTROL_OFFSET);
			svPoints[1].set(myStartValue + BEZIER_CONTROL_OFFSET, fTemp);
		}
		else
		{
			_ASSERT(myStartInterpType == InterpBezier);			
			//svPoints[1].set(fTemp, 0);
			svPoints[1].set(myStartValue, fTemp);
		}

		// Second control point
		//fTemp = (myEndValue - myStartValue)*(1.0 - BEZIER_CONTROL_OFFSET) + myStartValue;
		fTemp = (1.0 - BEZIER_CONTROL_OFFSET);
		if(myStartInterpType == InterpLinear)
		{
			//svPoints[2].set(fTemp, 1.0 - BEZIER_CONTROL_OFFSET);
			svPoints[2].set(myEndValue - BEZIER_CONTROL_OFFSET, fTemp);
		}
		else
		{
			_ASSERT(myStartInterpType == InterpBezier);			
			//svPoints[2].set(fTemp, 1);
			svPoints[2].set(myEndValue, fTemp);
		}

		// End point
		svPoints[3].set(myEndValue, 1);

		// Evaluate
		SVector2D svRes;
		SBezierSegment::evaluate(dInterp,svPoints, svRes);
		myLastValue = svRes.x;
	}
	return myLastValue;
}
/*****************************************************************************/
void AnimatedValue::changeAnimation(FLOAT_TYPE fromValue, FLOAT_TYPE toValue, FLOAT_TYPE fSeconds, ClockType eClock, int eType, AnimationOverCallback* pCallback)
{
	FLOAT_TYPE fFromValue = fromValue;
	FLOAT_TYPE fTimeInSeconds = fSeconds;

	GTIME lTime = Application::getInstance()->getGlobalTime(eClock);
	if(this->getIsAnimating(false))
	{
		fFromValue = this->getValue();
		// Compute the portion of time remaining
		FLOAT_TYPE fPortion = (toValue - fFromValue)/(toValue - fromValue);
		fTimeInSeconds *= fPortion;
	}
	// Do not animate if we're already at that value.
	else if(myLastValue == toValue)
	{
		// Note: We may have reset the myLastValue somewhere, but the
		// end value isn't. So we force-reset it here instantly:
		setNonAnimValue(toValue);
		return;
	}

	this->setAnimation(fFromValue, toValue, fTimeInSeconds, eClock, eType, pCallback);
}
/*****************************************************************************/
void AnimatedValue::setAnimation(FLOAT_TYPE fromValue, FLOAT_TYPE toValue, FLOAT_TYPE fSeconds, ClockType eClock, int eType, AnimationOverCallback* pCallback, FLOAT_TYPE fSecondsOffset)
{
	if(!myDidReportAnimationOverToActivityManager)
	{
		ActivityManager::getInstance()->endAnimation(getDebugName());
		myDidReportAnimationOverToActivityManager = true;
	}


#ifdef DEBUG_ANIM_VALUE
	gLog("ANIMVALUE: %x setAnimation %f to %f callback = %x\n", this, fromValue, toValue, pCallback);
	if(myCallback && !pCallback && fromValue == 0.0 && toValue == 1.0)
	{
		int bp = 0;
	}
#endif
	GTIME lTime = Application::getInstance()->getGlobalTime(eClock);
	GTIME lOffset = Application::secondsToTicks(fSecondsOffset);

	// If we have a controller, it must die!
	setController(NULL, myControllerResIdx);

	myStartTime = lTime + lOffset;
	myEndTime = myStartTime + Application::secondsToTicks(fSeconds);

	if(myEndTime > myStartTime)
	{
		ActivityManager::getInstance()->beginAnimation(getDebugName());
		myDidReportAnimationOverToActivityManager = false;
	}
	else if(myEndTime == myStartTime + 1)
		ActivityManager::getInstance()->singleValueChanged();

	myClockType = eClock;
	myLastEvalTime = 0;
	myDoOwnController = true;
	myAnimOverAction = eType;
	myDidReportAnimationOver = false;
	myStartValue = fromValue;
	myEndValue = toValue;
	myCallback = pCallback;

}
/*****************************************************************************/
void AnimatedValue::resetCache(void)
{
	if(myController)
		myController->resetCache();
	myLastEvalTime = 0;
}
/*****************************************************************************/
void AnimatedValue::checkTime(GTIME currTime)
{
	// Nothing to do.
	// Checked at caller now.
	// Wrong, to update
//	if(myAnimOverAction == AnimOverActionNone || myDidReportAnimationOver)
//		return;
	
	GTIME lEndTime;
	if(myController)
		lEndTime = myController->getEndTime();
	else
		lEndTime = myEndTime;

	if(!myDidReportAnimationOverToActivityManager && currTime >= lEndTime && lEndTime >= GLOBAL_ANIM_START_TIME)
	{
		// See if we have a non-repeating action
		if( (myAnimOverAction & AnimOverActionPlayReverseForever) == 0
			&& (myAnimOverAction & AnimOverActionReplayForever) == 0
			&& (myAnimOverAction & AnimOverActionPlayReverse) == 0)
		{
			ActivityManager::getInstance()->endAnimation(getDebugName());
			myDidReportAnimationOverToActivityManager = true;
		}
	}

#ifdef DEBUG_ANIM_VALUE
	gLog("ANIMVALUE: %x onTimerTick callback = %x currTime = %lld endTime = %lld \n", this, myCallback, currTime, lEndTime);
#endif
	
	if(currTime >= lEndTime && lEndTime >= GLOBAL_ANIM_START_TIME)
	{	
		// Check if we have to play this in reverse before reporting
		if( (myAnimOverAction & AnimOverActionPlayReverse) || 
			(myAnimOverAction & AnimOverActionPlayReverseForever) )
		{
			GTIME lSpan = myEndTime - myStartTime;
			myStartTime = currTime;
			myEndTime = myStartTime + lSpan;
			FLOAT_TYPE fOldStartValue = myStartValue;
			myStartValue = myEndValue;
			myEndValue = fOldStartValue;

			// Remove the reverse flag
			if(myAnimOverAction & AnimOverActionPlayReverse)
				myAnimOverAction = myAnimOverAction & (~AnimOverActionPlayReverse);
		}
		else if(myAnimOverAction & AnimOverActionReplayForever)
		{
			GTIME lSpan = myEndTime - myStartTime;
			myStartTime = currTime;
			myEndTime = myStartTime + lSpan;
		}
		else
		{
			myDidReportAnimationOver = true;
			if(myCallback)
				myCallback->animationOver(this, myCallbackData);
		}
	}
}
/*****************************************************************************/
void AnimatedValue::setAnimOverAction(int iAction)
{
	myAnimOverAction = iAction;
}
/*****************************************************************************/
int AnimatedValue::getAnimOverAction(void)
{
	return myAnimOverAction;
}
/*****************************************************************************/
BezierAnimController* AnimatedValue::createAndAssignBezierController(GTIME startTime, GTIME endTime, ClockType eClockType, int iControllerResIdx, bool bCallBeginAnim)
{
	myClockType = eClockType;
	BezierAnimController* pRes = new BezierAnimController();
	pRes->setTimes(startTime, endTime); // , true
	resetController();
	myController = pRes;
	myControllerResIdx = iControllerResIdx;
	myDoOwnController = true;

	if(bCallBeginAnim)
	{
		ActivityManager::getInstance()->beginAnimation(getDebugName());
		myDidReportAnimationOverToActivityManager = false;
	}

	return pRes;
}
/*****************************************************************************/
BezierAnimController* AnimatedValue::setCurveController(const CHAR_TYPE* pcsItemName, ClockType eClockType, FLOAT_TYPE fOffsetSeconds)
{
	GTIME lStartTime = Application::getInstance()->getGlobalTime(eClockType) + Application::secondsToTicks(fOffsetSeconds);
	return setCurveControllerInternal(pcsItemName, lStartTime, eClockType, true);
}
/*****************************************************************************/
BezierAnimController* AnimatedValue::setCurveControllerInternal(const CHAR_TYPE* pcsItemName, GTIME lStartTime, ClockType eClockType, bool bCallBeginAnim)
{
	myClockType = eClockType;
	BezierAnimController* pBezCont = createAndAssignBezierController(0, 1, eClockType, 0, bCallBeginAnim);
	// =1 since our values are actually in Y.
	myControllerResIdx = 1;
	GTIME lDuration = pBezCont->setFromItem(pcsItemName, lStartTime);
	pBezCont->setTimes(lStartTime, lStartTime + lDuration); // , false
	//pBezCont->finalize();

	// Just so they're not equial
	myStartValue = -2;
	myEndValue = -1;
	return pBezCont;
}
/*****************************************************************************/
IAnimController* AnimatedValue::copyAndAssignControllerFrom(IAnimController* pController, int iControllerResIdx)
{
	resetController();
	myController = pController->clone();
	myControllerResIdx = iControllerResIdx;
	myDoOwnController = true;

	ActivityManager::getInstance()->beginAnimation(getDebugName());
	myDidReportAnimationOverToActivityManager = false;

	return myController;
}
/*****************************************************************************/
FLOAT_TYPE AnimatedValue::getShootingAngle(bool bRaw)
{
	if(myController)
	{
		_ASSERT(myClockType != ClockLastPlaceholder);
		GTIME currTime = Application::getInstance()->getGlobalTime(myClockType);
		return myController->getShootingAngle(currTime, bRaw);
	}
	else
		return 0;
}
/*****************************************************************************/
FLOAT_TYPE AnimatedValue::getAngle(GTIME lOptTime)
{
	if(myController)
	{
		_ASSERT(myClockType != ClockLastPlaceholder);
		GTIME currTime = 0;
		if(lOptTime > 0)
			currTime = lOptTime;
		else
			currTime = Application::getInstance()->getGlobalTime(myClockType);
		return myController->getAngle(currTime);
	}
	else
	{
		// Linear animation
		return (myEndValue - myStartValue);
	}
}
/********************************************************************************************/
void AnimatedValue::setController(IAnimController* pController, int iControllerResIdx)
{
	myLastEvalTime = 0;
	resetController();
	myDoOwnController = false;
	myControllerResIdx = iControllerResIdx;
	myController = pController;
}
/********************************************************************************************/
FLOAT_TYPE AnimatedValue::getStartValue() const
{
	_ASSERT(!myController);
	return myStartValue;
}
/********************************************************************************************/
FLOAT_TYPE AnimatedValue::getEndValue() const
{
	_ASSERT(!myController);
	return myEndValue;
}
/*****************************************************************************/
void AnimatedValue::saveToItem(ResourceItem& rItemOut, const CHAR_TYPE* pcsTagId)
{
	// We can only save if we have no controller, and are a simple animation.
	_ASSERT(myController == NULL || dynamic_cast<BezierAnimController*>(myController));

	// Save: start and end times, start and end values
	ResourceItem* pOwnItem = rItemOut.addChild(pcsTagId);
	pOwnItem->setStringProp(PropertyId, pcsTagId);

	// Temp, we just save the bezier controller info
	if(myController)
	{
		dynamic_cast<BezierAnimController*>(myController)->saveToItem(*pOwnItem, BEZIER_CONTROLLER_SAVE_TAG);
	}

	pOwnItem->setNumProp(PropertySvAVClockType, (int)myClockType);
	pOwnItem->setNumProp(PropertySvAVStartValue, myStartValue);
	pOwnItem->setNumProp(PropertySvAVEndValue, myEndValue);
	pOwnItem->setAsLong(PropertySvAVStartTime, myStartTime);
	pOwnItem->setAsLong(PropertySvAVEndTime, myEndTime);

	pOwnItem->setNumProp(PropertySvAVStartInterp, (int)myStartInterpType);
	pOwnItem->setNumProp(PropertySvAVEndInterp, (int)myEndInterpType);

	pOwnItem->setNumProp(PropertySvAVAnimOverAction, myAnimOverAction);
}
/*****************************************************************************/
void AnimatedValue::loadFromItem(ResourceItem* pItem)
{
	myClockType = (ClockType)(int)pItem->getNumProp(PropertySvAVClockType);

	// First, load all the children, since that may create all the needed controllers.
	// For now, just the bezier
	ResourceItem* pChild;
	const CHAR_TYPE* pcsTag;
	int iCurrChild, iNumChildren = pItem->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = pItem->getChild(iCurrChild);
		pcsTag = pChild->getTag();
		if(STRCMP(pcsTag, BEZIER_CONTROLLER_SAVE_TAG) == 0)
		{
			// The last two are irrelevant and will be overridden.
			this->setCurveControllerInternal(pChild->getStringProp(PropertySvBZContCurveName), pChild->getAsLong(PropertySvBZContStartTime), myClockType, false);
			dynamic_cast<BezierAnimController*>(myController)->loadFromItem(pChild);
		}
		else
		{
			_ASSERT(0);
		}
	}

	myStartValue = pItem->getNumProp(PropertySvAVStartValue);
	myEndValue = pItem->getNumProp(PropertySvAVEndValue);
	myAnimOverAction = pItem->getNumProp(PropertySvAVAnimOverAction);

	myStartTime = pItem->getAsLong(PropertySvAVStartTime);
	myEndTime = pItem->getAsLong(PropertySvAVEndTime);

	myStartInterpType = (InterpType)(int)pItem->getNumProp(PropertySvAVStartInterp);
	myEndInterpType = (InterpType)(int)pItem->getNumProp(PropertySvAVEndInterp);

	ActivityManager::getInstance()->beginAnimation(getDebugName());
	myDidReportAnimationOverToActivityManager = false;

}
/*****************************************************************************/
bool AnimatedValue::getIsAnimating(bool bTreatLoopingAsEndless) const
{
	GTIME lTime = 0;
	if(myClockType != ClockLastPlaceholder)
		lTime = Application::getInstance()->getGlobalTime(myClockType);

	_ASSERT( (myStartValue != myEndValue && myClockType != ClockLastPlaceholder) || (myStartValue == myEndValue));

	bool bIsAnimating = (myStartValue != myEndValue) && !myDidReportAnimationOver && lTime >= myStartTime && lTime < myEndTime;
	// If we have a endless anim, we will never report animation, and will always be at a 
	// time which is either in middle of our animation span or at an end of it.
	// So we check for that, and if we're at either end of the animation, we say
	// it's over.
	if(bIsAnimating && !bTreatLoopingAsEndless
		&& myAnimOverAction & (AnimOverActionReplayForever | AnimOverActionPlayReverseForever) 
		&& (lTime == myEndTime) )
		bIsAnimating = false;

	if(myController)
	{
		_ASSERT(myClockType != ClockLastPlaceholder);
		bIsAnimating = myController->getIsAnimating(lTime);
	}

	return bIsAnimating;
	//return (myStartValue != myEndValue) && !myDidReportAnimationOver && lTime >= myStartTime && lTime <= myEndTime;
}
/*****************************************************************************/
void AnimatedValue::setStartInterpType(InterpType eType)
{
	myStartInterpType = eType;
}
/*****************************************************************************/
void AnimatedValue::setEndInterpType(InterpType eType)
{
	myEndInterpType = eType;
}
/*****************************************************************************/
void AnimatedValue::transformBy(const SMatrix2D& smTransform, bool bAsX)
{
	_ASSERT(myController);

	SVector2D svTemp;
	if(bAsX)
	{
		svTemp.set(myStartValue, 0);
		myStartValue = (smTransform*svTemp).x;

		svTemp.set(myEndValue, 0);
		myEndValue = (smTransform*svTemp).x;
	}
	else
	{
		svTemp.set(0, myStartValue);
		myStartValue = (smTransform*svTemp).y;

		svTemp.set(0, myEndValue);
		myEndValue = (smTransform*svTemp).y;
	}
	myLastEvalTime = 0;
}
/*****************************************************************************/
void AnimatedValue::setDebugName(const CHAR_TYPE* pcsValue)
{
#ifdef _DEBUG
	if(pcsValue)
	{
		CHAR_TYPE pcsBuff[128];
		SPRINTF(pcsBuff, 128, STR_LIT("%x"), this);
		myDebugName = pcsBuff;
		myDebugName += STR_LIT(" ");
		myDebugName += pcsValue;
	}
	else 
		myDebugName = EMPTY_STRING;
#endif
}
/*****************************************************************************/
const CHAR_TYPE* AnimatedValue::getDebugName() const
{
#ifdef _DEBUG
	return myDebugName.c_str();
#else
	return NULL;
#endif
}
/*****************************************************************************/
void AnimatedValue::ensureAnimOverReportedToActivityManager()
{
	if(!myDidReportAnimationOverToActivityManager)
	{
		ActivityManager::getInstance()->endAnimation(getDebugName());
		myDidReportAnimationOverToActivityManager = true;
	}
}
/*****************************************************************************/
void AnimatedValue::operator=(const AnimatedValue& rSource)
{
	copyFrom(rSource);
}
/*****************************************************************************/
void AnimatedValue::copyFrom(const AnimatedValue& rSource)
{
	resetController();
	if(rSource.myController)
	{
		myDoOwnController = rSource.myDoOwnController;
		if(rSource.myDoOwnController)
			myController = rSource.myController->clone();
		else
			myController = rSource.myController;
	}

	myControllerResIdx = rSource.myControllerResIdx;
	myClockType = rSource.myClockType;
	myStartValue = rSource.myStartValue;
	myEndValue = rSource.myEndValue;
	myStartTime = rSource.myStartTime;
	myEndTime = rSource.myEndTime;
	myAnimOverAction = rSource.myAnimOverAction;
	myLastValue = rSource.myLastValue;
	myLastEvalTime = rSource.myLastEvalTime;
	myDidReportAnimationOver = rSource.myDidReportAnimationOver;
	myDidReportAnimationOverToActivityManager = rSource.myDidReportAnimationOverToActivityManager;
	myStartInterpType = rSource.myStartInterpType;
	myEndInterpType = rSource.myEndInterpType;

	setDebugName(rSource.getDebugName());

	myCallback = rSource.myCallback;
	setCallbackData(rSource.myCallbackData ? rSource.myCallbackData->c_str() : NULL);
}
/********************************************************************************************/
};
