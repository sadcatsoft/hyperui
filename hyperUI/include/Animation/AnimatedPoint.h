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
#pragma once

class AnimationOverCallback;
/*****************************************************************************/
class HYPERUI_API AnimatedPoint
{
public:
	AnimatedPoint();
	~AnimatedPoint();

	void getValue(GTIME currTime, SVector2D& resOut) const;
	FLOAT_TYPE getAngle(GTIME currTime);
	FLOAT_TYPE getShootingAngle(GTIME currTime, bool bRaw = false);
	void setLastShootingAngle(FLOAT_TYPE fValue);

	void setNonAnimValue(FLOAT_TYPE dX, FLOAT_TYPE dY);

	void setAnimationOverAction(AnimationOverCallback* pCallback, int iXAction, int iYAction);

	void setNonAnimValueForX(FLOAT_TYPE dValue);
	void setNonAnimValueForY(FLOAT_TYPE dValue);	

	void setAnimationForX(FLOAT_TYPE fromValue, FLOAT_TYPE toValue, FLOAT_TYPE fDuration, ClockType eClockType, int eType = 0, AnimationOverCallback* pCallback = NULL, FLOAT_TYPE fOffset = 0.0);
	void setAnimationForY(FLOAT_TYPE fromValue, FLOAT_TYPE toValue, FLOAT_TYPE fDuration, ClockType eClockType, int eType = 0, AnimationOverCallback* pCallback = NULL, FLOAT_TYPE fOffset = 0.0);
	void setAnimation(SVector2D& svFrom, SVector2D& svTo, FLOAT_TYPE fDuration, ClockType eClockType,int eType = 0, AnimationOverCallback* pCallback = NULL, FLOAT_TYPE fOffset = 0.0);

	IAnimController* getCommonController(void);
	void resetController(void);
	void resetCache(void);

	void setCachedValue(FLOAT_TYPE fX, FLOAT_TYPE fY, GTIME lTime);
	FLOAT_TYPE getSpeed(void);

	void getCachedValue(SVector2D& svOut) { svOut = myLastValue; }
	bool getIsAnimating(bool bTreatLoopingAsEndless);

	bool doesContain(AnimatedValue* pValue);
	void setBezierController(const SBezierCurve* pCurve, GTIME startTime, GTIME endTime, ClockType eClock);
	
	void transformBy(const SMatrix2D& smTransform);

	const AnimatedValue& getX() const { return myX; }
	const AnimatedValue& getY() const { return myY; }
private:
	AnimatedValue& x(void);
	AnimatedValue& y(void);

private:
	// Just a pointer, not an allocatoin
	IAnimController *myUnifiedControllerPtr;
	AnimatedValue myX, myY;

	mutable GTIME myLastEvalTime, myLastTangentEvalTime;
	mutable SVector2D myLastValue;
	FLOAT_TYPE myLastTangentValue;

	FLOAT_TYPE myLastShootingAngle;
	GTIME myLastShootingAngleEvalTime;

};
/*****************************************************************************/