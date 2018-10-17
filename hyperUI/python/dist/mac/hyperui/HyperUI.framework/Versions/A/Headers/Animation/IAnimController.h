#pragma once

/********************************************************************************************/
class HYPERUI_API IAnimController
{
public:	
	IAnimController();
	virtual ~IAnimController() { }

	GTIME getStartTime();
	GTIME getEndTime();

	void setTimes(GTIME startTime, GTIME endTime);

	void getDirection(GTIME currTime, SVector2D& svDirOut);
	virtual FLOAT_TYPE getAngle(GTIME currTime);
	virtual FLOAT_TYPE getShootingAngle(GTIME currTime, bool bRaw) { return 0; }

	virtual SVector2D evaluate(GTIME currTime) = 0;
	virtual IAnimController* clone() = 0;

	virtual GTIME computeEndTime(GTIME startTime, FLOAT_TYPE dSpeed) = 0;

	virtual void resetCache() { }

	virtual bool getIsAnimating(GTIME lTime) = 0;

protected:
	void setStartTime(GTIME rTime);
	void setEndTime(GTIME rTime);

	void baseCopyFrom(IAnimController* pController);

	virtual SVector2D getTangent(GTIME currTime) = 0;

private:
	GTIME myStartTime, myEndTime;
};
/********************************************************************************************/
