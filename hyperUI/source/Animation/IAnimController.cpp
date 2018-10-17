#include "stdafx.h"

namespace HyperUI
{
/********************************************************************************************/
IAnimController::IAnimController()
{
	//myIsPlayerControlled = false;
}
/********************************************************************************************/
GTIME IAnimController::getStartTime()
{
	return myStartTime;
}
/********************************************************************************************/
GTIME IAnimController::getEndTime()
{
	return myEndTime;
}
/********************************************************************************************/
void IAnimController::setStartTime(GTIME rTime)
{
	myStartTime = rTime;
}
/********************************************************************************************/
void IAnimController::setEndTime(GTIME rTime)
{
	myEndTime = rTime;
}
/********************************************************************************************
void IAnimController::setIsPlayerControlled(bool bValue)
{
	myIsPlayerControlled = bValue;
}
/********************************************************************************************/
void IAnimController::getDirection(GTIME currTime, SVector2D& svDirOut)
{
	svDirOut = this->getTangent(currTime);
	svDirOut.normalize();
}
/********************************************************************************************/
FLOAT_TYPE IAnimController::getAngle(GTIME currTime)
{
	SVector2D svTemp;
	svTemp = this->getTangent(currTime);
	svTemp.normalize();
	return svTemp.getAngleFromPositiveX();
}
/********************************************************************************************/
void IAnimController::setTimes(GTIME startTime, GTIME endTime)
{
	setStartTime(startTime);
	setEndTime(endTime);
	//finalize();
}
/********************************************************************************************/
void IAnimController::baseCopyFrom(IAnimController* pController)
{
	myStartTime = pController->myStartTime;
	myEndTime = pController->myEndTime;
	//myIsPlayerControlled = pController->myIsPlayerControlled;
}
/********************************************************************************************/
};