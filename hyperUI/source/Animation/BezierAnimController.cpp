#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
BezierAnimController::BezierAnimController()
{
	myTotalLength = -1;
	setStartTime(0);
}
/*****************************************************************************/
BezierAnimController::~BezierAnimController()
{

}
/*****************************************************************************/
void BezierAnimController::clear()
{
	myAnimBezierCurve.clear();
	myTotalLength = 0;
}
/*****************************************************************************/
void BezierAnimController::saveToItem(ResourceItem& rItemOut, const CHAR_TYPE* pcsTagId)
{
	if(myCurveName.length() == 0)
	{
		// For now, we only save controllers created from named curves.
		_ASSERT(0);
		return;
	}

	ResourceItem* pOwnItem = rItemOut.addChild(pcsTagId);
	pOwnItem->setStringProp(PropertyId, pcsTagId);
	pOwnItem->setAsLong(PropertySvBZContStartTime, getStartTime());
	pOwnItem->setStringProp(PropertySvBZContCurveName, myCurveName.c_str());			
}
/*****************************************************************************/
bool BezierAnimController::getIsAnimating(GTIME lTime)
{
	FLOAT_TYPE dInterp;
	int iFinalSeg;
	getSegmentIndexAndPos(lTime, iFinalSeg, dInterp);
	if(iFinalSeg < 0)
		return false;
	else
	{
		int iNumSegs = myAnimBezierCurve.getNumSegments();
		if(iFinalSeg == iNumSegs - 1 && dInterp >= 1.0)
			return false;
		else
			return true;
	}
}
/*****************************************************************************/
void BezierAnimController::loadFromItem(ResourceItem* pItem)
{
	// Just reset it, recomputed on the fly.
	myTotalLength = -1;

	myCurveName = pItem->getStringProp(PropertySvBZContCurveName);
	setStartTime(pItem->getAsLong(PropertySvBZContStartTime));
}
/*****************************************************************************/
void BezierAnimController::addLinear(SVector2D& svStart, SVector2D& svEnd)
{
	myAnimBezierCurve.addLinear(svStart, svEnd);
	/*
	SVector2D svTemp;
	svTemp = (svStart + svEnd)*0.5;

	AnimBezier rTemp;
	rTemp.setStart(svStart.x, svStart.y);
	rTemp.setEnd(svEnd.x, svEnd.y);
	rTemp.setControl1(svTemp.x, svTemp.y);
	rTemp.setControl2(svTemp.x, svTemp.y);
	rTemp.setIsLinear(true);

	myAnimBezierCurve.push_back(rTemp);
	*/
}
/*****************************************************************************/
void BezierAnimController::addBezier(SVector2D& svStart, SVector2D& svControl1, SVector2D& svControl2, SVector2D& svEnd, bool bIsLinear)
{
	myAnimBezierCurve.addBezier(svStart, svControl1, svControl2, svEnd, bIsLinear);
	/*
	AnimBezier rTemp;
	rTemp.setStart(svStart.x, svStart.y);
	rTemp.setEnd(svEnd.x, svEnd.y);
	rTemp.setControl1(svControl1.x, svControl1.y);
	rTemp.setControl2(svControl2.x, svControl2.y);
	rTemp.setIsLinear(bIsLinear);
	
	myAnimBezierCurve.push_back(rTemp);
	*/
}
/*****************************************************************************/
void BezierAnimController::getSegmentIndexAndPos(GTIME timeIn, int& iSegmentIndexOut, FLOAT_TYPE& dInterpOut)
{
	// See which time range we fall into. Note that there may be discontinuities
	// between segments, which means last good pos (or should we interp to next start?)
	int iCurrSeg, iNumSegs = myAnimBezierCurve.getNumSegments();
	
	iSegmentIndexOut = -1;
	if(iNumSegs <= 0)
		return;

	GTIME lOverallStartTime = getStartTime();
	
	const SAnimBezierSegment* pBez;
	for(iCurrSeg = 0; iCurrSeg < iNumSegs; iCurrSeg++)
	{
		pBez = myAnimBezierCurve.getSegment(iCurrSeg);
		if(pBez->getStartTime() + lOverallStartTime> timeIn)
			break;
	}
	
	if(iCurrSeg > 0 && iCurrSeg < iNumSegs)
	{
		pBez = myAnimBezierCurve.getSegment(iCurrSeg);
		// See if we're between segs
		if(pBez->getEndTime() + lOverallStartTime	< timeIn && 
		   pBez->getStartTime() + lOverallStartTime > timeIn)
		{
			dInterpOut = 1.0;
			iSegmentIndexOut = iCurrSeg - 1;
		}
	}
	
	if(iSegmentIndexOut < 0)
	{
		if(iCurrSeg == 0)
		{
			iSegmentIndexOut = 0;
			dInterpOut = 0.0;
		}
		else
		{
			// iCurrSeg is between 1 and iNumSegs-1, inclusive, and is not between the segments.
			iSegmentIndexOut = iCurrSeg - 1;
			pBez = myAnimBezierCurve.getSegment(iSegmentIndexOut);
			dInterpOut = (FLOAT_TYPE)(timeIn - (pBez->getStartTime() + lOverallStartTime))/(FLOAT_TYPE)( (pBez->getEndTime() + lOverallStartTime) - (pBez->getStartTime() + lOverallStartTime) );
			if(dInterpOut > 1.0)
				dInterpOut = 1.0;
			
		}
	}
	
	_ASSERT(dInterpOut >= 0.0 && dInterpOut <= 1.0);
	_ASSERT(iSegmentIndexOut >= 0 && iSegmentIndexOut < iNumSegs);	
}
/*****************************************************************************/
SVector2D BezierAnimController::evaluate(GTIME currTime)
{		
	FLOAT_TYPE dInterp;
	int iFinalSeg;
	getSegmentIndexAndPos(currTime, iFinalSeg, dInterp);
	if(iFinalSeg < 0)
		return SVector2D(0,0);
	SVector2D svRes;
	myAnimBezierCurve.getSegment(iFinalSeg)->evaluate(dInterp, svRes);
//	myAnimBezierCurve[iFinalSeg].evaluate(dInterp, svRes);
	return svRes;
}
/*****************************************************************************/
IAnimController* BezierAnimController::clone(void)
{
	BezierAnimController* pRes = new BezierAnimController();
	pRes->baseCopyFrom(this);

	pRes->myCurveName = this->myCurveName;
	pRes->myAnimBezierCurve = this->myAnimBezierCurve;
	pRes->myTotalLength = this->myTotalLength;
	
	return pRes;
}
/********************************************************************************************
void BezierAnimController::finalize(void)
{
	// NOTE that this distributes the poitns on the curve EQUALLY in time,
	// overwriting any existing time values in the curves themselves!
	if(myAnimBezierCurve.getNumSegments() <= 0)
		return;
	
	int iNumSegs = myAnimBezierCurve.getNumSegments();
	
	// TODO: Set times for each segment
	// Go over every segment
	GTIME startTime = getStartTime();
	GTIME endTime = getEndTime();	
	FLOAT_TYPE incTime = (FLOAT_TYPE)(endTime - startTime)/(FLOAT_TYPE)iNumSegs;
	
	int iCurrSeg;
	SAnimBezierSegment* pBez;
	for(iCurrSeg = 0; iCurrSeg < iNumSegs; iCurrSeg++)
	{
		pBez = myAnimBezierCurve.getSegment(iCurrSeg);
		pBez->setStartTime(iCurrSeg*incTime + startTime);
		pBez->setEndTime((iCurrSeg + 1)*incTime + startTime);		
	}
	
	// To make sure the start and end times are exact:
	myAnimBezierCurve.getSegment(0)->setStartTime(startTime);	
	myAnimBezierCurve.getSegment(iNumSegs - 1)->setEndTime(endTime);
}
/********************************************************************************************/
SVector2D BezierAnimController::getTangent(GTIME currTime)
{
	FLOAT_TYPE dInterp;
	int iFinalSeg;
	getSegmentIndexAndPos(currTime, iFinalSeg, dInterp);
	if(iFinalSeg < 0)
		return SVector2D(0,1);

	SVector2D svTangent;
	myAnimBezierCurve.getSegment(iFinalSeg)->getTangent(dInterp, svTangent);
	return svTangent;
}
/*****************************************************************************/
GTIME BezierAnimController::computeEndTime(GTIME startTime, FLOAT_TYPE dSpeed)
{
	if(fabs(dSpeed) < 1.0)
		return startTime;	
	
	// Get the length
	FLOAT_TYPE dLen = getTotalLength();	
	return startTime + Application::secondsToTicks(dLen/dSpeed);
}
/*****************************************************************************/
void BezierAnimController::offsetTimeBy(GTIME lTime)
{
	setStartTime(getStartTime() + lTime);
	setEndTime(getEndTime() + lTime);
/*
	AnimBezier* pCurrCurve;
	int iCurve, iNumCurves = myAnimBezierCurve.getNumSegments();

	if(iNumCurves <= 0)
		return;

	for(iCurve = 0; iCurve < iNumCurves; iCurve++)
	{
		pCurrCurve = myAnimBezierCurve.getSegment(iCurve);
		pCurrCurve->setStartTime(pCurrCurve->getStartTime() + lTime);
		pCurrCurve->setEndTime(pCurrCurve->getEndTime() + lTime);
	}
	*/
}
/*****************************************************************************/
void BezierAnimController::scaleTimeToMatchDuration(FLOAT_TYPE fOverallTime)
{
	SAnimBezierSegment* pCurrCurve;
	int iCurve, iNumCurves = myAnimBezierCurve.getNumSegments();

	if(iNumCurves <= 0)
		return;

	// First, we need to find the weights.
	myFloatVector.clear();
	GTIME lFirstCurveStartTime = myAnimBezierCurve.getSegment(0)->getStartTime();
	FLOAT_TYPE fCurrTime;
	for(iCurve = 0; iCurve < iNumCurves; iCurve++)
	{
		pCurrCurve = myAnimBezierCurve.getSegment(iCurve);
		fCurrTime = Application::ticksToSeconds(pCurrCurve->getEndTime() - pCurrCurve->getStartTime());
		myFloatVector.push_back(fCurrTime);
	}

	normalizeProbList(myFloatVector);

	// Now, reset the times in proportion
	GTIME fPrevStartTime = lFirstCurveStartTime;
	GTIME fEndTime;
	for(iCurve = 0; iCurve < iNumCurves; iCurve++)
	{
		pCurrCurve = myAnimBezierCurve.getSegment(iCurve);
		fEndTime = fPrevStartTime + Application::secondsToTicks(fOverallTime*myFloatVector[iCurve]);
		pCurrCurve->setStartTime(fPrevStartTime);
		pCurrCurve->setEndTime(fEndTime);

		fPrevStartTime = fEndTime;
	}

	setEndTime(getStartTime() + Application::secondsToTicks(fOverallTime));
}
/*****************************************************************************/
GTIME BezierAnimController::setFromItem(const CHAR_TYPE* pcsItemType, GTIME lStartTime)
{
	if(!myAnimBezierCurve.loadFromItem(pcsItemType))
		return 0;

	myCurveName = pcsItemType;
	setStartTime(lStartTime);

	int iKey, iNumKeys;

	GTIME lCurrStartTime = lStartTime;
	int iCurveCounter = 0;
	FLOAT_TYPE fTotalDuration = 0;
	FLOAT_TYPE fPrevTime = 0;
	FLOAT_TYPE fKeyTime;
	ResourceItem* pCurveItem = ResourceManager::getInstance()->getCollection(ResourceCurves)->getItemById(pcsItemType);
	CurveType eCurveType = mapStringToType(pCurveItem->getStringProp(PropertyBezType), g_pcsCurveTypes, CurveLinear1D);
	// This sets the times on the curve. The reason for doing this here (though not a good one) is because the 
	// bezier curves is templated, and not all of the template types have an idea of time, in particular, setStartTime() and setEndTime().
	//if(eCurveType == CurveLinear1D)
	{
		iNumKeys = pCurveItem->getNumericEnumPropCount(PropertyBezTimes);
		_ASSERT(iNumKeys > 0);
		// Initially, we get num times, which is one less than num keys
		iNumKeys++;

		fPrevTime = 0;
		for(iKey = 1 ; iKey < iNumKeys; iKey++)
		{
			fKeyTime = pCurveItem->getNumericEnumPropValue(PropertyBezTimes, iKey - 1);

			myAnimBezierCurve.getSegment(iCurveCounter)->setStartTime(lCurrStartTime);
			lCurrStartTime += Application::secondsToTicks(fKeyTime);
			myAnimBezierCurve.getSegment(iCurveCounter)->setEndTime(lCurrStartTime);
			iCurveCounter++;

			fTotalDuration += fKeyTime;

			fPrevTime = fKeyTime;
		}
	}
/*
	else
	{
		_ASSERT(0);
	}
*/
	return Application::secondsToTicks(fTotalDuration);

	/*

	TODO // Merge with the beziercurves code somehow. What about time? Or only handle 1D, timed types here?

	ResourceItem* pCurveItem = g_pMainEngine->getResourceManager()->getCollection(ResourcePrelimCurves)->findItemByType(pcsItemType);
	if(!pCurveItem)
	{
		_ASSERT(0);
		return 0;
	}

	clear();

	GTIME lCurrStartTime = lStartTime;
	int iCurveCounter = 0;

	FLOAT_TYPE fTotalDuration = 0;

	FLOAT_TYPE fKey, fKeyTime;
	FLOAT_TYPE fKeyX, fKeyY;
	int iKey, iNumKeys;
	FLOAT_TYPE fPrevKey, fPrevTime;
	FLOAT_TYPE fPrevKeyX, fPrevKeyY;
	SVector2D svDummy1, svDummy2;
	CurveType eCurveType = mapStringToType(pCurveItem->getStringProp(PropertyBezType), g_pcsCurveTypes, CurveLinear1D);
	if(eCurveType == CurveLinear1D)
	{
		iNumKeys = pCurveItem->getNumericEnumPropCount(PropertyBezKeys);

		fPrevTime = 0;
		fPrevKey = pCurveItem->getNumericEnumPropValue(PropertyBezKeys, 0);
		for(iKey = 1 ; iKey < iNumKeys; iKey++)
		{
			fKey = pCurveItem->getNumericEnumPropValue(PropertyBezKeys, iKey);
			fKeyTime = pCurveItem->getNumericEnumPropValue(PropertyBezTimes, iKey - 1);

			svDummy1.set(fPrevTime, fPrevKey);
			svDummy2.set(fKeyTime, fKey);
			this->addLinear(svDummy1, svDummy2);

			myAnimBezierCurve.getSegment(iCurveCounter)->setStartTime(lCurrStartTime);
			lCurrStartTime += Application::secondsToTicks(fKeyTime);
			myAnimBezierCurve.getSegment(iCurveCounter)->setEndTime(lCurrStartTime);
			iCurveCounter++;

			fTotalDuration += fKeyTime;

			fPrevKey = fKey;
			fPrevTime = fKeyTime;
		}
	}
	else if(eCurveType == CurveLinear2D)
	{
		// Linear curve in 2D. We could also do this using Beziers, but this is
		// just easier.
		iNumKeys = pCurveItem->getNumericEnumPropCount(PropertyBezKeysX);
		_ASSERT(iNumKeys == pCurveItem->getNumericEnumPropCount(PropertyBezKeysY));

		// What do we do about times? What if we don't need the key for every time?
		// Do we even need time here at all? 

		fPrevKeyX = pCurveItem->getNumericEnumPropValue(PropertyBezKeysX, 0);
		fPrevKeyY = pCurveItem->getNumericEnumPropValue(PropertyBezKeysY, 0);
		for(iKey = 1 ; iKey < iNumKeys; iKey++)
		{
			fKeyX = pCurveItem->getNumericEnumPropValue(PropertyBezKeysX, iKey);
			fKeyY = pCurveItem->getNumericEnumPropValue(PropertyBezKeysY, iKey);

			svDummy1.set(fPrevKeyX, fPrevKeyY);
			svDummy2.set(fKeyX, fKeyY);
			this->addLinear(svDummy1, svDummy2);

			// TODO // What about curve time?
			iCurveCounter++;

			fPrevKeyX = fKeyX;
			fPrevKeyY = fKeyY;
		}
	}
	else if(eCurveType == CurveBezier2D)
	{
		// Normal Bezier curve in 2D.
		iNumKeys = pCurveItem->getNumericEnumPropCount(PropertyBezKeysX);
		_ASSERT(iNumKeys == pCurveItem->getNumericEnumPropCount(PropertyBezKeysY));

		// Each segment must have three points, since we continue off of a previous one.
		// The first point is obviously extra, so we get 3*N + 1 where N is the number 
		// of control points.
		_ASSERT( (iNumKeys - 1) % 3 == 0);

		// What do we do about times? What if we don't need the key for every time?
		// Do we even need time here at all? 

		fPrevKeyX = pCurveItem->getNumericEnumPropValue(PropertyBezKeysX, 0);
		fPrevKeyY = pCurveItem->getNumericEnumPropValue(PropertyBezKeysY, 0);

		SVector2D svControl1, svControl2, svEndPoint;

		for(iKey = 1 ; iKey < iNumKeys; iKey += 3)
		{
			svControl1.x = pCurveItem->getNumericEnumPropValue(PropertyBezKeysX, iKey + 0);
			svControl1.y = pCurveItem->getNumericEnumPropValue(PropertyBezKeysY, iKey + 0);

			svControl2.x = pCurveItem->getNumericEnumPropValue(PropertyBezKeysX, iKey + 1);
			svControl2.y = pCurveItem->getNumericEnumPropValue(PropertyBezKeysY, iKey + 1);

			svEndPoint.x = pCurveItem->getNumericEnumPropValue(PropertyBezKeysX, iKey + 2);
			svEndPoint.y = pCurveItem->getNumericEnumPropValue(PropertyBezKeysY, iKey + 2);

			svDummy1.set(fPrevKeyX, fPrevKeyY);

			this->addBezier(svDummy1, svControl1, svControl2, svEndPoint);

			//TODO // What about curve time?
			iCurveCounter++;

			fPrevKeyX = fKeyX;
			fPrevKeyY = fKeyY;
		}
	}
	else
	{
		// Forgotten curve type
		_ASSERT(0);
	}

	// Note: this means we have to finalize it and set times after this call.
//	finalize();

	return Application::secondsToTicks(fTotalDuration);
	*/
}
/*****************************************************************************/
GTIME BezierAnimController::getLastCurveEndTime()
{
	int iNumCurves = myAnimBezierCurve.getNumSegments();
	if(iNumCurves < 1)
		return 0;

	return myAnimBezierCurve.getSegment(iNumCurves - 1)->getEndTime() + getStartTime();
}
/*****************************************************************************/
GTIME BezierAnimController::getTimeLength()
{
	int iNumCurves = myAnimBezierCurve.getNumSegments();
	if(iNumCurves < 1)
		return 0;

	return myAnimBezierCurve.getSegment(iNumCurves - 1)->getEndTime() - myAnimBezierCurve.getSegment(0)->getStartTime();
}
/*****************************************************************************/
FLOAT_TYPE BezierAnimController::getTotalLength(void)
{
	if(myTotalLength > 0)
		return myTotalLength;
	
	// We really only need a rough approximation
	FLOAT_TYPE dTolerance = 4;
	FLOAT_TYPE dMaxPointSpacing = 10;
	
	myTotalLength = 0;
	int iCurrSeg, iNumSegs = myAnimBezierCurve.getNumSegments();
	for(iCurrSeg = 0; iCurrSeg < iNumSegs; iCurrSeg++)
	{
		myTotalLength += myAnimBezierCurve.getSegment(iCurrSeg)->getLength(dTolerance, dMaxPointSpacing);
	}
	
	return myTotalLength;
}
/*****************************************************************************/
void BezierAnimController::copyCurvesFrom(const SBezierCurve* pOther)
{
	if(!pOther)
		return;
	else
	{
		const SBezierSegment* pBez;
		SVector2D svPoints[4];
		myAnimBezierCurve.clear();
		int iCurr, iNum = pOther->getNumSegments();
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			pBez = pOther->getSegment(iCurr);
			pBez->getStart(svPoints[0]);
			pBez->getControl1(svPoints[1]);
			pBez->getControl2(svPoints[2]);
			pBez->getEnd(svPoints[3]);
			myAnimBezierCurve.addBezier(svPoints[0], svPoints[1], svPoints[2], svPoints[3]);
		}
		recomputeSegmentTimes();
	}
}
/*****************************************************************************/
void BezierAnimController::recomputeSegmentTimes()
{
	FLOAT_TYPE fTotalLen = myAnimBezierCurve.getLength();
	GTIME lTime = getEndTime() - getStartTime() + 1;
	FLOAT_TYPE fTimePerPixel = lTime/fTotalLen;
	int iCurr, iNum = myAnimBezierCurve.getNumSegments();
	SAnimBezierSegment* pCurve;
	GTIME lCurveTime, lPrevStartTime = 0;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pCurve = myAnimBezierCurve.getSegment(iCurr);
		lCurveTime = pCurve->getLength(1.0, -1)*fTimePerPixel;
		pCurve->setStartTime(lPrevStartTime);
		lPrevStartTime += lCurveTime;
		pCurve->setEndTime(lPrevStartTime);
	}
}
/*****************************************************************************/
void BezierAnimController::extractCurvesTo(SBezierCurve& rCurve) const
{
	rCurve.clear();
	int iCurr, iNum = myAnimBezierCurve.getNumSegments();
	const SAnimBezierSegment* pCurve;
	SVector2D svPoints[4];
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pCurve = myAnimBezierCurve.getSegment(iCurr);

		pCurve->getStart(svPoints[0]);
		pCurve->getControl1(svPoints[1]);
		pCurve->getControl2(svPoints[2]);
		pCurve->getEnd(svPoints[3]);
		
		// We don't have explicit flags anymore
		rCurve.addBezier(svPoints[0], svPoints[1], svPoints[2], svPoints[3]);
		//rCurve.addBezier(svPoints[0], svPoints[1], svPoints[2], svPoints[3], pCurve->getIsLinear());
	}
}
/*****************************************************************************/
};