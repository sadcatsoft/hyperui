#include "stdafx.h"

namespace HyperUI
{
BezierCurveManager* BezierCurveManager::theInstance = NULL;

#define RESAMPLE_CURVE_ERROR		upToScreen(0.25)
#define MAX_RESAMPLE_POINT_SPACING	-1

/*****************************************************************************/
BezierCurveManager::BezierCurveManager()
{
	
}
/*****************************************************************************/
BezierCurveManager::~BezierCurveManager()
{

}
/*****************************************************************************/
BezierCurveManager* BezierCurveManager::getInstance()
{
	if(!theInstance)
		theInstance = new BezierCurveManager;
	return theInstance;
}
/*****************************************************************************/
TPointVector* BezierCurveManager::getLinearForm(const char* pcsCurve)
{
	SCurveInfo *pCurve;
	TStringCurveMap::iterator mi = myCurves.find(pcsCurve);
	if(mi == myCurves.end())
		pCurve = cacheCurveInfo(pcsCurve);
	else
		pCurve = &mi->second;

	if(!pCurve)
	{
		_ASSERT(0);
		return NULL;
	}

	return &pCurve->myLinearForm;
}
/*****************************************************************************/
FLOAT_TYPE BezierCurveManager::getCurveLength(const char* pcsCurve)
{
	SCurveInfo *pCurve;
	TStringCurveMap::iterator mi = myCurves.find(pcsCurve);
	if(mi == myCurves.end())
		pCurve = cacheCurveInfo(pcsCurve);
	else
		pCurve = &mi->second;

	if(!pCurve)
	{
		_ASSERT(0);
		return 0.0;
	}

	return pCurve->myTotalLength;
}
/*****************************************************************************/
TFloatVector* BezierCurveManager::getDistances(const char* pcsCurve)
{
	SCurveInfo *pCurve;
	TStringCurveMap::iterator mi = myCurves.find(pcsCurve);
	if(mi == myCurves.end())
		pCurve = cacheCurveInfo(pcsCurve);
	else
		pCurve = &mi->second;

	if(!pCurve)
	{
		_ASSERT(0);
		return NULL;
	}

	return &pCurve->myDistances;
}
/*****************************************************************************/
TPointVector* BezierCurveManager::getTangents(const char* pcsCurve)
{
	SCurveInfo *pCurve;
	TStringCurveMap::iterator mi = myCurves.find(pcsCurve);
	if(mi == myCurves.end())
		pCurve = cacheCurveInfo(pcsCurve);
	else
		pCurve = &mi->second;

	if(!pCurve)
	{
		_ASSERT(0);
		return NULL;
	}

	return &pCurve->myTangents;
}
/*****************************************************************************/
SCurveInfo* BezierCurveManager::cacheCurveInfo(const char*pcsCurve)
{
	// Create a temp bezier from it, and resample it:
	SBezierCurve rTempCurve;
	if(!rTempCurve.loadFromItem(pcsCurve))
	{
		_ASSERT(0);
		return NULL;
	}

	SCurveInfo rTempCurveInfo;

#ifdef _DEBUG
	if(strcmp(pcsCurve, "connCornerTRCurve") == 0)
	{
		int bp = 0;
	}
#endif

	rTempCurve.resampleCurves(RESAMPLE_CURVE_ERROR, MAX_RESAMPLE_POINT_SPACING, rTempCurveInfo.myLinearForm, &rTempCurveInfo.myTangents);

	// Now, find distances
	rTempCurveInfo.myTotalLength = Polyline2D::computePointDistances(rTempCurveInfo.myLinearForm, rTempCurveInfo.myDistances);

	return &(myCurves[pcsCurve] = rTempCurveInfo);

}
/*****************************************************************************/
void BezierCurveManager::reloadAll()
{
	myCurves.clear();
}
/*****************************************************************************/
};