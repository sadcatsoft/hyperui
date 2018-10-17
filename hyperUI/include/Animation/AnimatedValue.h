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

class AnimatedValue;
class BezierAnimController;
/********************************************************************************************/
enum AnimOverActionType
{
	AnimOverActionNone					= 0x00000000,
	AnimOverActionPlayReverse			= 0x00000001,
	AnimOverActionReplayForever			= 0x00000002, // from start
	AnimOverActionPlayReverseForever	= 0x00000004,

	AnimOverActionDeleteObject			= 0x00000008,
	AnimOverActionCloseDoor				= 0x00000010,
	AnimOverActionFinishedLevel			= 0x00000020,
	AnimationFadeInComplete				= 0x00000040,
	AnimOverActionSpawnPlayer			= 0x00000080,
	AnimOverActionEndGame				= 0x00000100,
	AnimOverActionLoop					= 0x00000200,
	AnimOverActionFadeOutComplete		= 0x00000400,
	AnimOverActionHangComplete			= 0x00000800,
	AnimOverActionKeepDoorOpen			= 0x00001000,
	AnimOverActionGenericCallback		= 0x00002000,
	AnimOverActionFinishedSliding		= 0x00004000,
	AnimOverActionPuzzleSolved			= 0x00008000

//	AnimOverActionPartDoneEating		= 0x00000004,
//	AnimOverActionLevelTransitionDone	= 0x00000008,
//	AnimOverActionAssignFishController	= 0x00000040,
//	AnimOverActionDie					= 0x00000080,
//	AnimOverActionShowGameEnd			= 0x00000100,

	
};
/*****************************************************************************/
enum InterpType
{
	InterpLinear = 0,
	InterpBezier,
};

const CHAR_TYPE* const g_pcsInterpTypeTokens[] = 
{
	STR_LIT("itLinear"),
	STR_LIT("itBezier"),
	0
};
/*****************************************************************************/
class HYPERUI_API AnimationOverCallback
{
public:
	virtual ~AnimationOverCallback() { }
	
	virtual void animationOver(AnimatedValue *pAValue, STRING_TYPE* pData) = 0;
};
/*****************************************************************************/
class HYPERUI_API AnimatedValue
{
public:
	AnimatedValue();
	AnimatedValue(const AnimatedValue& rSource);
	~AnimatedValue();
	
	FLOAT_TYPE getValue() const;
	void resetController(void);
	
	void setNonAnimValue(FLOAT_TYPE dValue);
	// Simpler version, in seconds.
	void setAnimation(FLOAT_TYPE fromValue, FLOAT_TYPE toValue, FLOAT_TYPE fSeconds, ClockType eClock, int eType = 0, 
						AnimationOverCallback* pCallback = NULL, FLOAT_TYPE fSecondsOffset = 0);

	// Changes animation taking into account any current values, and proportionally setting time.
	void changeAnimation(FLOAT_TYPE fromValue, FLOAT_TYPE toValue, FLOAT_TYPE fSeconds, ClockType eClock, int eType = 0, AnimationOverCallback* pCallback = NULL);
	
	void setCallbacks(AnimationOverCallback *pCallback, const CHAR_TYPE* pcsData);
	void setCallbackData(const CHAR_TYPE* pcsData);
	int getAnimOverAction(void);
	void setAnimOverAction(int iAction);
	
	void resetCache(void);
	//void setObeyScrolling(bool value);
	
	BezierAnimController* createAndAssignBezierController(GTIME startTime, GTIME endTime, ClockType eClockType, int iControllerResIdx, bool bCallBeginAnim);
	BezierAnimController* setCurveController(const CHAR_TYPE* pcsItemName, ClockType eClockType, FLOAT_TYPE fOffsetSeconds = 0);
	IAnimController* copyAndAssignControllerFrom(IAnimController* pController, int iControllerResIdx);

	FLOAT_TYPE getAngle(GTIME lOptTime = 0);
	FLOAT_TYPE getShootingAngle(bool bRaw);
	
	// Only to be used by the animated point!
	void setController(IAnimController* pController, int iControllerResIdx);
	inline IAnimController* getController() { return myController; }

	inline GTIME getStartTime() const { return myStartTime; }
	inline GTIME getEndTime() const { return myEndTime; }

	// Works only for simple anims!
	FLOAT_TYPE getStartValue() const;
	FLOAT_TYPE getEndValue() const;

	void saveToItem(ResourceItem& rItemOut, const CHAR_TYPE* pcsTagId);
	void loadFromItem(ResourceItem* pItem);

	bool getIsAnimating(bool bTreatLoopingAsEndless) const;

	void setStartInterpType(InterpType eType);
	void setEndInterpType(InterpType eType);

	void setDebugName(const CHAR_TYPE* pcsValue);
	const CHAR_TYPE* getDebugName() const;

	void ensureAnimOverReportedToActivityManager();

	void operator=(const AnimatedValue& rSource);

	void copyFrom(const AnimatedValue& rSource);

	void transformBy(const SMatrix2D& smTransform, bool bAsX);

private:
	BezierAnimController* setCurveControllerInternal(const CHAR_TYPE* pcsItemName, GTIME lStartTime, ClockType eClockType, bool bCallBeginAnim);
	// Internal now
	void checkTime(GTIME currTime);
protected:

void init();
private:
	
	IAnimController* myController;
	int myControllerResIdx;
	
	// Note that these are valid only if there is no controller
	// (the default)
	FLOAT_TYPE myStartValue, myEndValue;
	GTIME myStartTime, myEndTime;
	
	bool myDidReportAnimationOver;
	bool myDidReportAnimationOverToActivityManager;
	AnimationOverCallback *myCallback;	
	
	// int since this may be a combo of actions
	int myAnimOverAction;
	
	mutable FLOAT_TYPE myLastValue;
	mutable GTIME myLastEvalTime;
	
	bool myDoOwnController;

	InterpType myStartInterpType, myEndInterpType;
	ClockType myClockType;

	STRING_TYPE* myCallbackData;
#ifdef _DEBUG
	STRING_TYPE myDebugName;
#endif
};
/*****************************************************************************/
