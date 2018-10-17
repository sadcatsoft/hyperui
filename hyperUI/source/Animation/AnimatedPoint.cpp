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
/*****************************************************************************/
AnimatedPoint::AnimatedPoint()
{
	myLastEvalTime = 0;
	myLastTangentEvalTime = 0;
	myLastShootingAngleEvalTime = 0;
	myUnifiedControllerPtr = NULL;	
}
/*****************************************************************************/
AnimatedPoint::~AnimatedPoint()
{
	myUnifiedControllerPtr = NULL;
}
/*****************************************************************************/
AnimatedValue& AnimatedPoint::x(void)
{
	return myX;
}
/*****************************************************************************/
AnimatedValue& AnimatedPoint::y(void)
{
	return myY;
}
/*****************************************************************************/
void AnimatedPoint::resetController(void)
{
	myX.resetController();
	myY.resetController();
	myUnifiedControllerPtr = NULL;

}
/********************************************************************************************/
IAnimController* AnimatedPoint::getCommonController(void)
{
	IAnimController* pContr = myX.getController();
	if(pContr && pContr == myY.getController())
		return pContr;
	else
		return NULL;
}
/*****************************************************************************/
void AnimatedPoint::setLastShootingAngle(FLOAT_TYPE fValue)
{
	myLastShootingAngle = fValue;
	myLastShootingAngleEvalTime = Application::getInstance()->getGlobalTime(ClockMainGame);
}
/*****************************************************************************/
FLOAT_TYPE AnimatedPoint::getShootingAngle(GTIME currTime, bool bRaw)
{	
	if(bRaw)
	{
		if(myX.getController())
			return myX.getShootingAngle(bRaw);
		else
			return 0;
	}

	if(myLastShootingAngleEvalTime >= currTime)
		return myLastShootingAngle;

	if(myX.getController() && myY.getController())
	{
		myLastShootingAngle = myX.getShootingAngle(bRaw);
		myLastShootingAngleEvalTime = currTime;

		return myLastShootingAngle;
	}

	return 0;
}
/*****************************************************************************/
FLOAT_TYPE AnimatedPoint::getAngle(GTIME currTime)
{	
	if(myLastTangentEvalTime >= currTime)
		return myLastTangentValue;

	if(myX.getController() && myY.getController())
	{
		myLastTangentValue = myX.getAngle(currTime);
		myLastTangentEvalTime = currTime;

		return myLastTangentValue;
	}
	else if(!myX.getController() && !myY.getController())
	{
		// The values returned are actual slopes in this case
		SVector2D svTemp;
		svTemp.x = myX.getAngle();
		svTemp.y = myY.getAngle();
		if(svTemp.normalize() > FLOAT_EPSILON)
			myLastTangentValue = svTemp.getAngleFromPositiveX();
		else
			myLastTangentValue = 0;
		return myLastTangentValue;
	}
	else
	{
		// Undefined now where one value has a controller, 
		// and the other one doesn't
		_ASSERT(0);

		return 0;
	}
}
/*****************************************************************************/
void AnimatedPoint::getValue(GTIME currTime, SVector2D& resOut) const
{	
	if(myLastEvalTime >= currTime)
	{
		resOut = myLastValue;
		return;
	} 
	if(myUnifiedControllerPtr)
	{
		resOut = myUnifiedControllerPtr->evaluate(currTime);
	}
	else
	{
		resOut.x = myX.getValue();
		resOut.y = myY.getValue();
	}
	myLastValue = resOut;
	myLastEvalTime = currTime;
}
/*****************************************************************************/
void AnimatedPoint::setNonAnimValue(FLOAT_TYPE dX, FLOAT_TYPE dY)
{
	resetCache();

	myX.setNonAnimValue(dX);
	myY.setNonAnimValue(dY);
	myUnifiedControllerPtr = NULL;
}
/*****************************************************************************/
void AnimatedPoint::setAnimationOverAction(AnimationOverCallback* pCallback, int iXAction, int iYAction)
{
	myX.setCallbacks(pCallback, NULL);
	myX.setAnimOverAction(iXAction);
	myY.setCallbacks(pCallback, NULL);
	myY.setAnimOverAction(iYAction);	
}
/*****************************************************************************/
void AnimatedPoint::setNonAnimValueForX(FLOAT_TYPE dValue)
{
	resetCache();
	myUnifiedControllerPtr = NULL;
	myX.setNonAnimValue(dValue);
}
/*****************************************************************************/
void AnimatedPoint::setNonAnimValueForY(FLOAT_TYPE dValue)
{
	resetCache();
	myUnifiedControllerPtr = NULL;
	myY.setNonAnimValue(dValue);
}
/*****************************************************************************/
void AnimatedPoint::setAnimationForX(FLOAT_TYPE fromValue, FLOAT_TYPE toValue, FLOAT_TYPE fDuration, ClockType eClockType, int eType, AnimationOverCallback* pCallback, FLOAT_TYPE fOffset)
{
	resetCache();
	myUnifiedControllerPtr = NULL;
	myX.setAnimation(fromValue, toValue,  fDuration, eClockType, eType, pCallback, fOffset);
}
/*****************************************************************************/
void AnimatedPoint::setAnimationForY(FLOAT_TYPE fromValue, FLOAT_TYPE toValue, FLOAT_TYPE fDuration, ClockType eClockType, int eType, AnimationOverCallback* pCallback, FLOAT_TYPE fOffset)
{
	resetCache();
	myUnifiedControllerPtr = NULL;
	myY.setAnimation(fromValue, toValue,  fDuration, eClockType, eType, pCallback, fOffset);	
}
/*****************************************************************************/
void AnimatedPoint::setAnimation(SVector2D& svFrom, SVector2D& svTo, FLOAT_TYPE fDuration, ClockType eClockType, int eType, AnimationOverCallback* pCallback, FLOAT_TYPE fOffset)
{
	resetCache();
	myUnifiedControllerPtr = NULL;
	myX.setAnimation(svFrom.x, svTo.x,  fDuration, eClockType, eType, pCallback, fOffset);
	myY.setAnimation(svFrom.y, svTo.y,  fDuration, eClockType, AnimOverActionNone, NULL, fOffset);
}
/*****************************************************************************/
void AnimatedPoint::setCachedValue(FLOAT_TYPE fX, FLOAT_TYPE fY, GTIME lTime)
{
	myLastEvalTime = lTime;
	myLastValue.set(fX, fY);
}
/*****************************************************************************/
void AnimatedPoint::resetCache(void)
{
	myLastShootingAngleEvalTime = 0;
	myLastEvalTime = 0;
	myLastTangentEvalTime = 0;
	x().resetCache();
	y().resetCache();
}
/*****************************************************************************/
FLOAT_TYPE AnimatedPoint::getSpeed(void)
{
	FLOAT_TYPE fRes = 0.0;
	if(x().getEndTime() <= Application::getInstance()->getGlobalTime(ClockMainGame) && 
		x().getEndTime() != x().getStartTime())
	{
		fRes = (x().getEndValue() - x().getStartValue())/(x().getEndTime() - x().getStartTime());
	}
	return fRes;
}
/*****************************************************************************/
bool AnimatedPoint::getIsAnimating(bool bTreatLoopingAsEndless)
{
	return x().getIsAnimating(bTreatLoopingAsEndless) || y().getIsAnimating(bTreatLoopingAsEndless);
}
/*****************************************************************************/
bool AnimatedPoint::doesContain(AnimatedValue* pValue)
{
	if(&myX == pValue || &myY == pValue)
		return true;
	else
		return false;
}
/*****************************************************************************/
void AnimatedPoint::setBezierController(const SBezierCurve* pCurve, GTIME startTime, GTIME endTime, ClockType eClock)
{
	resetCache();
	BezierAnimController* pTempCont;
	pTempCont = myX.createAndAssignBezierController(startTime, endTime, eClock, 0, true);
	pTempCont->setTimes(startTime, endTime);
	myUnifiedControllerPtr = pTempCont;
	myY.setController(pTempCont, 1);

	pTempCont->copyCurvesFrom(pCurve);
	/*
TODO: OLd:
	resetCache();
	IAnimController* pTempCont;
	pTempCont = myX.copyAndAssignControllerFrom(g_pMainEngine->getAnimControllerPreset(pcsControllerPreset), 0);
	pTempCont->setTimes(startTime, endTime);

	myUnifiedControllerPtr = pTempCont;
	myY.setController(pTempCont, 1);
	*/
}
/*****************************************************************************/
void AnimatedPoint::transformBy(const SMatrix2D& smTransform)
{
	myX.transformBy(smTransform, true);
	myY.transformBy(smTransform, false);
}
/********************************************************************************************/
};