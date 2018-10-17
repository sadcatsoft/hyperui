#include "stdafx.h"

#define SAVE_BEZIER_CURVE_IS_CLOSED			"bezIsClosed"
#define SAVE_BEZIER_CURVE_IS_BACKGROUND		"bezIsBack"
#define SAVE_BEZIER_CURVE_IS_AHOLE			"bezIsHole"
#define SAVE_BEZIER_CURVE_MAT_ID			"bezMatId"
#define SAVE_BEZIER_SEGMENT					"bezSegment"

#define SAVE_BEZIER_CURVE_START_ARROW_ENABLED			"bezStartArrowEnabled"
#define SAVE_BEZIER_CURVE_START_ARROW_LENGTH_MULT		"bezStartArrowLengthMult"
#define SAVE_BEZIER_CURVE_START_ARROW_WIDTH_MULT		"bezStartArrowWidthMult"
#define SAVE_BEZIER_CURVE_END_ARROW_ENABLED				"bezEndArrowEnabled"
#define SAVE_BEZIER_CURVE_END_ARROW_LENGTH_MULT			"bezEndArrowLengthMult"
#define SAVE_BEZIER_CURVE_END_ARROW_WIDTH_MULT			"bezEndArrowWidthMult"

//template < class TYPE > UniqueIdGenerator TBezierCurve<TYPE>::theIdGenerator;
namespace HyperUI
{
/*****************************************************************************/
// TBezierCurve
/*****************************************************************************/
template < class TYPE > 
TBezierCurve<TYPE>::TBezierCurve()
{
	myParent = NULL;
	myMaterialId = 0;

	myIsClosed = false;
	myIsBackground = false;
	myIsAHole = false;

	myIsStartArrowEnabled = false;
	myStartArrowLengthMult = myStartArrowWidthMult = 1.0;
	myIsEndArrowEnabled =  false;
	myEndArrowLengthMult = myEndArrowWidthMult = 1.0;
}
/*****************************************************************************/
template < class TYPE > 
TBezierCurve<TYPE>::~TBezierCurve()
{
	clear();
}
/*****************************************************************************/
template < class TYPE > 
TYPE* TBezierCurve<TYPE>::addSegment(void)
{
	TYPE* pNewCurve = new TYPE();
	mySegments.push_back(pNewCurve);
	pNewCurve->setParent(this);
	return pNewCurve;
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurve<TYPE>::addExistingSegment(TYPE* pSeg)
{
	if(!pSeg)
		ASSERT_RETURN;
	mySegments.push_back(pSeg);
	pSeg->setParent(this);
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurve<TYPE>::clear()
{
	clearSegments();
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurve<TYPE>::clearSegments()
{
	int iCurve, iNum = mySegments.size();
	for(iCurve = 0; iCurve < iNum; iCurve++)
		delete mySegments[iCurve];
	mySegments.clear();
}
/*****************************************************************************/
template < class TYPE > 
int TBezierCurve<TYPE>::getNumSegments(void) const
{
	return mySegments.size();
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurve<TYPE>::resampleCurves(FLOAT_TYPE dError, FLOAT_TYPE dMaxPointSpacing, TPointVector &rPointsOut, TPointVector* optTangentsOut) const
{
	rPointsOut.clear();
	if(optTangentsOut)
		optTangentsOut->clear();

	int iCurrCurve, iNumCurves = mySegments.size();
	for(iCurrCurve = 0; iCurrCurve < iNumCurves; iCurrCurve++)
		mySegments[iCurrCurve]->resample(dError, dMaxPointSpacing, (iCurrCurve == 0), rPointsOut, optTangentsOut);
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurve<TYPE>::onTimerTick(GTIME lCurrentTime)
{
	int iCurrCurve, iNumCurves = mySegments.size();
	for(iCurrCurve = 0; iCurrCurve < iNumCurves; iCurrCurve++)
		mySegments[iCurrCurve]->onTimerTick(lCurrentTime);
}
/*****************************************************************************/
template < class TYPE > 
bool TBezierCurve<TYPE>::loadFromItem(const char*pcsItem)
{
	ResourceItem* pCurveItem = ResourceManager::getInstance()->getCollection(ResourceCurves)->getItemById(pcsItem);
	if(!pCurveItem)
	{
		_ASSERT(0);
		return false;
	}

	clear();

//	GTIME lCurrStartTime = lStartTime;
	int iCurveCounter = 0;

//	FLOAT_TYPE fTotalDuration = 0;

	FLOAT_TYPE fKey; //, fKeyTime;
	FLOAT_TYPE fKeyX, fKeyY;
	int iKey, iNumKeys;
	FLOAT_TYPE fPrevKey; // , fPrevTime;
	FLOAT_TYPE fPrevKeyX, fPrevKeyY;
	SVector2D svDummy1, svDummy2;
	CurveType eCurveType = mapStringToType(pCurveItem->getStringProp(PropertyBezType), g_pcsCurveTypes, CurveLinear1D);
	if(eCurveType == CurveLinear1D)
	{
		iNumKeys = pCurveItem->getNumericEnumPropCount(PropertyBezKeys);

//		fPrevTime = 0;
		fPrevKey = pCurveItem->getNumericEnumPropValue(PropertyBezKeys, 0);
		for(iKey = 1 ; iKey < iNumKeys; iKey++)
		{
			fKey = pCurveItem->getNumericEnumPropValue(PropertyBezKeys, iKey);
//			fKeyTime = pCurveItem->getNumericEnumPropValue(PropertyBezTimes, iKey - 1);

			svDummy1.set(0, fPrevKey);
			svDummy2.set(0, fKey);
			this->addLinear(svDummy1, svDummy2);
			/*
			myAnimBeziers[iCurveCounter].setStartTime(lCurrStartTime);
			lCurrStartTime += Application::secondsToTicks(fKeyTime);
			myAnimBeziers[iCurveCounter].setEndTime(lCurrStartTime);
			*/
			iCurveCounter++;

		//	fTotalDuration += fKeyTime;

			fPrevKey = fKey;
//			fPrevTime = fKeyTime;
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

			fPrevKeyX = svEndPoint.x;
			fPrevKeyY = svEndPoint.y;
		}
	}
	else
	{
		// Forgotten curve type
		_ASSERT(0);
	}

	return true;
	// return Application::secondsToTicks(fTotalDuration);
}
/*****************************************************************************/
template < class TYPE > 
TYPE* TBezierCurve<TYPE>::addLinear(const SVector2D& svStart, const SVector2D& svEnd, UNIQUEID_TYPE lOptEdgeId)
{
	TYPE* pNew = new TYPE();

	pNew->setStart(svStart.x, svStart.y);
	pNew->setEnd(svEnd.x, svEnd.y);
	pNew->makeLinear();
	pNew->setParent(this);

	if(lOptEdgeId > 0)
		pNew->setEdgeId(lOptEdgeId);
	else
	{
		_ASSERT(myParent);
		if(myParent)
			pNew->setEdgeId(myParent->getNextId());
	}

	mySegments.push_back(pNew);

	return pNew;
}
/*****************************************************************************/
template < class TYPE > 
TYPE* TBezierCurve<TYPE>::addBezier(const SVector2D& svStart, const SVector2D& svControl1, const SVector2D& svControl2, const SVector2D& svEnd, bool bIsLinear, UNIQUEID_TYPE lOptEdgeId)
{
	TYPE* pNew = new TYPE();

	pNew->setStart(svStart.x, svStart.y);
	pNew->setEnd(svEnd.x, svEnd.y);
	pNew->setControl1(svControl1.x, svControl1.y);
	pNew->setControl2(svControl2.x, svControl2.y);
	if(bIsLinear)
		pNew->makeLinear();
	//pNew->setIsLinear(bIsLinear);
	pNew->setParent(this);

	if(lOptEdgeId > 0)
		pNew->setEdgeId(lOptEdgeId);
	else
	{
		_ASSERT(myParent);
		if(myParent)
			pNew->setEdgeId(myParent->getNextId());
	}

	mySegments.push_back(pNew);

	return pNew;
}
/*****************************************************************************/
template < class TYPE > 
TBezierCurve<TYPE>* TBezierCurve<TYPE>::cloneSelf(FLOAT_TYPE fPerfScalingFactor) const
{
	TBezierCurve<TYPE>* pNew = new TBezierCurve<TYPE>();
	pNew->myIsClosed = myIsClosed;
	pNew->myIsBackground = myIsBackground;
	pNew->myIsAHole = myIsAHole;
	pNew->myMaterialId = myMaterialId;

	pNew->myIsStartArrowEnabled = myIsStartArrowEnabled;
	pNew->myStartArrowLengthMult = myStartArrowLengthMult;
	pNew->myStartArrowWidthMult = myStartArrowWidthMult;
	pNew->myIsEndArrowEnabled = myIsEndArrowEnabled;
	pNew->myEndArrowLengthMult = myEndArrowLengthMult;
	pNew->myEndArrowWidthMult = myEndArrowWidthMult;


	TYPE* pCurve;
	int iCurr, iNum = mySegments.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pCurve = dynamic_cast<TYPE*>(mySegments[iCurr]->cloneSelf(fPerfScalingFactor));
		_ASSERT(pCurve);
		pNew->mySegments.push_back(pCurve);
		pCurve->setParent(pNew);
	}

	return pNew;
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurve<TYPE>::getBBox(SRect2D& svBBoxOut) const
{
	int iCurr, iNum = mySegments.size();
	if(iNum <= 0)
		return;

	mySegments[0]->getBBox(svBBoxOut);
	SRect2D svTempRect;
	for(iCurr = 1; iCurr < iNum; iCurr++)
	{
		mySegments[iCurr]->getBBox(svTempRect);
		svBBoxOut.includeRect(svTempRect);
	}
}
/*****************************************************************************/
template < class TYPE > 
TYPE* TBezierCurve<TYPE>::findSegmentByEdgeId(const EdgeId& idEdge) const
{
	int iCurr, iNum = mySegments.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(mySegments[iCurr]->getEdgeId() == idEdge)
		{
			_ASSERT(mySegments[iCurr]->getParent() == this);
			return mySegments[iCurr];
		}
	}

	return NULL;
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurve<TYPE>::onSegmentPointChanged(TYPE* pSegment, BezierPointType ePointType, FLOAT_TYPE fX, FLOAT_TYPE fY, bool bIgnoreTiedControls)
{
	// Find the segment
	SVector2D svNew;
	int iCurr, iNum = mySegments.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(mySegments[iCurr] == pSegment)
		{
			// Got it!
			if(ePointType == BezierPointStart || ePointType == BezierPointControl1)
			{
				// Look backwards
				int iPrevIndex = iCurr - 1;
				if(iPrevIndex < 0 && myIsClosed)
					iPrevIndex += iNum;

				if(iPrevIndex >= 0 && iPrevIndex < iNum)
				{					
					if(ePointType == BezierPointStart)
					{
						pSegment->getPointByType(ePointType, svNew);
						mySegments[iPrevIndex]->setEnd(svNew.x, svNew.y);
						mySegments[iPrevIndex]->ensureIsLinear();
						mySegments[iPrevIndex]->movePointBy(BezierPointControl2, fX, fY, false, false);
					}
					else
					{
						if(pSegment->getPointCurveType(ePointType) == BezierPointCurveTied && !bIgnoreTiedControls)
						{
							SVector2D svControlCurrPos;
							pSegment->getPointByType(ePointType, svControlCurrPos);
							SVector2D svControlPrevPos = svControlCurrPos - SVector2D(fX, fY);

							SVector2D svPivot;
							pSegment->getPointByType(BezierPointStart, svPivot);
							SVector2D svDiff1 = svControlPrevPos - svPivot;
							FLOAT_TYPE fPrevLen = svDiff1.normalize();
							SVector2D svDiff2 = svControlCurrPos - svPivot;
							FLOAT_TYPE fCurrLen = svDiff2.normalize();
							FLOAT_TYPE fRotateAngle = svDiff2.getAngleFrom(svDiff1);

							mySegments[iPrevIndex]->getPointByType(BezierPointControl2, svControlCurrPos);
							svControlCurrPos -= svPivot;
							svControlCurrPos.rotateCCW(fRotateAngle);
							svDiff1 = svControlCurrPos;
							FLOAT_TYPE fNewLen = svDiff1.normalize();
							fNewLen += (fCurrLen - fPrevLen);
							svControlCurrPos = svPivot + svDiff1*fNewLen;
							//svControlCurrPos += svPivot;
							mySegments[iPrevIndex]->setControl2(svControlCurrPos.x, svControlCurrPos.y);
							mySegments[iPrevIndex]->ensureIsLinear();
						}
					}
				}
			}
			else if(ePointType == BezierPointEnd || ePointType == BezierPointControl2)
			{
				// Look forward
				int iNextIndex = iCurr + 1;
				if(iNextIndex >= iNum && myIsClosed)
					iNextIndex -= iNum;

				if(iNextIndex >= 0 && iNextIndex < iNum)
				{					
					if(ePointType == BezierPointEnd)
					{
						pSegment->getPointByType(ePointType, svNew);
						mySegments[iNextIndex]->setStart(svNew.x, svNew.y);
						mySegments[iNextIndex]->ensureIsLinear();

						mySegments[iNextIndex]->movePointBy(BezierPointControl1, fX, fY, false, false);
					}
					else
					{
						// Move the control point 2 of that prev segment so that original ratio is
						// maintained.
						if(pSegment->getPointCurveType(ePointType) == BezierPointCurveTied && !bIgnoreTiedControls)
						{
							SVector2D svControlCurrPos;
							pSegment->getPointByType(ePointType, svControlCurrPos);
							SVector2D svControlPrevPos = svControlCurrPos - SVector2D(fX, fY);

							SVector2D svPivot;
							pSegment->getPointByType(BezierPointEnd, svPivot);
							SVector2D svDiff1 = svControlPrevPos - svPivot;
							FLOAT_TYPE fPrevLen = svDiff1.normalize();
							SVector2D svDiff2 = svControlCurrPos - svPivot;
							FLOAT_TYPE fCurrLen = svDiff2.normalize();
							FLOAT_TYPE fRotateAngle = svDiff2.getAngleFrom(svDiff1);

							mySegments[iNextIndex]->getPointByType(BezierPointControl1, svControlCurrPos);
							svControlCurrPos -= svPivot;
							svControlCurrPos.rotateCCW(fRotateAngle);
							svDiff1 = svControlCurrPos;
							FLOAT_TYPE fNewLen = svDiff1.normalize();
							fNewLen += (fCurrLen - fPrevLen);
							svControlCurrPos = svPivot + svDiff1*fNewLen;
							//svControlCurrPos += svPivot;
							mySegments[iNextIndex]->setControl1(svControlCurrPos.x, svControlCurrPos.y);
							mySegments[iNextIndex]->ensureIsLinear();
						}
					}
				}

			}
			ELSE_ASSERT;

			return;

		}
	}


	// If we got here, something is screwed up because we couldn't find the
	// segment in us.
	_ASSERT(0);
}
/*****************************************************************************/
template < class TYPE > 
const TYPE* TBezierCurve<TYPE>::findVertexClosestTo(const SVector2D& svPoint, FLOAT_TYPE fOptMaxRadius, SVector2D& svResPointOut, BezierPointType& ePointTypeOut) const
{
	SVector2D svTempPoint;
	FLOAT_TYPE fDistSq = FLOAT_TYPE_MAX;
	FLOAT_TYPE fCurrDistSq;

	const TYPE* pRes = NULL;
	BezierPointType eCurrPointType;

	int iCurr, iNum = mySegments.size();
	if(iNum <= 0)
		return pRes;

	const TYPE* pSegment;
	bool bRes;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pSegment = getSegment(iCurr);
		bRes = pSegment->findVertexClosestTo(svPoint, fOptMaxRadius, svTempPoint, eCurrPointType);
		if(!bRes)
			continue;

		fCurrDistSq = (svTempPoint - svPoint).lengthSquared();
		if(fCurrDistSq < fDistSq)
		{
			fDistSq = fCurrDistSq;
			svResPointOut = svTempPoint;
			pRes = pSegment;
			ePointTypeOut = eCurrPointType;
		}
	}

	return pRes;
}
/*****************************************************************************/
template < class TYPE > 
const TYPE* TBezierCurve<TYPE>::findPointClosestTo(const SVector2D& svPoint, SVector2D& svPointOut, FLOAT_TYPE* fOptParmDistOut, int* iOptSegmentIndexOut) const
{
	SVector2D svTempPoint;
	TYPE* pRes = NULL;
	FLOAT_TYPE fDistSq = FLOAT_TYPE_MAX;
	FLOAT_TYPE fCurrDistSq;

	if(fOptParmDistOut)
		*fOptParmDistOut = 0.0;
	if(iOptSegmentIndexOut)
		*iOptSegmentIndexOut = 0;

	int iCurr, iNum = mySegments.size();
	if(iNum <= 0)
		return pRes;

	FLOAT_TYPE fParmDist;

	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		mySegments[iCurr]->findPointClosestTo(svPoint, svTempPoint, &fParmDist);

		fCurrDistSq = (svTempPoint - svPoint).lengthSquared();
		if(fCurrDistSq < fDistSq)
		{
			fDistSq = fCurrDistSq;
			svPointOut = svTempPoint;
			pRes = mySegments[iCurr];

			if(fOptParmDistOut)
				*fOptParmDistOut = fParmDist;
			if(iOptSegmentIndexOut)
				*iOptSegmentIndexOut = iCurr;
		}
	}

	return pRes;
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurve<TYPE>::deletePoint(TYPE* pSegment, BezierPointType ePointType)
{
	int iCurr, iNum = mySegments.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(mySegments[iCurr] == pSegment)
			break;
	}

	if(iCurr >= iNum)
		ASSERT_RETURN;

	// Now, if we only have one segment left and we found it,
	// that means we're deleting the whole curve.
	if(iNum == 1)
	{
		myParent->deleteCurve(this);
		return;
	}
	

	// Otherwise, we need to combine either this and previous or this and the next segments,
	// depending on whether it's a start or an end point.
	int iLeftIndex, iRightIndex;
	BezierPointType eOtherSegmentPointType;
	if(ePointType == BezierPointEnd)
	{
		iLeftIndex = iCurr;
		iRightIndex = iCurr + 1;
		eOtherSegmentPointType = BezierPointStart;
		if(iRightIndex >= iNum)
		{
			if(myIsClosed)
				iRightIndex -= iNum;
			else 
				iRightIndex = -1;
		}
	}
	else if(ePointType == BezierPointStart)
	{
		iLeftIndex = iCurr - 1;
		iRightIndex = iCurr;
		eOtherSegmentPointType = BezierPointEnd;
		if(iLeftIndex < 0)
		{
			if(myIsClosed)
				iLeftIndex += iNum;
			else
				iLeftIndex = -1;
		}
	}
	ELSE_ASSERT_RETURN;

	// Now we're set into left/right segments. Note that
	// either may be invalid...
	if(iRightIndex < 0)
	{
		// Just delete the iLeftIndex segment, and be done with it
		_ASSERT(iLeftIndex == 0 && !myIsClosed);
		delete mySegments[0];
		mySegments.erase(mySegments.begin());
	}
	else if(iLeftIndex < 0)
	{
		// Just delete the iRightIndex segment, and be done with it
		_ASSERT(iRightIndex == iNum - 1 && !myIsClosed);
		delete mySegments[iRightIndex];
		mySegments.erase(mySegments.begin() + iRightIndex);
	}
	else 
	{
		// Delete the right, but make its control2 and end point
		// the control and end point of the left one.
		_ASSERT(iRightIndex >= 0 && iLeftIndex >= 0);

		// Generate the new id
		//EdgeId rId;
		//EdgeId::generateMergeId(mySegments[iLeftIndex]->getEdgeId(), mySegments[iRightIndex]->getEdgeId(), rId);

		// For the purposes of properly deleting multiple points at once,
		// we need to:
		// 1) Not change the segment id above, otherwise next point identifiers won't be able to find
		// the subsequent points/segments.
		// 2) Delete not just the right segment, but one which was passed in. Otherwise, we also won't
		// be able to find the subsequent points.
		// 3) If I ever come back here to change something, BE VEWY, VEWY CAREFUL!

		// Copy the right's points onto the left one
		SVector2D svTemp;

		if(mySegments[iRightIndex] == pSegment)
		{
			mySegments[iRightIndex]->getEnd(svTemp);
			mySegments[iLeftIndex]->setEnd(svTemp.x, svTemp.y);
			mySegments[iRightIndex]->getControl2(svTemp);
			mySegments[iLeftIndex]->setControl2(svTemp.x, svTemp.y);

			// Set the new id onto the left one. Do we even need to change id here?
			//mySegments[iLeftIndex]->setEdgeId(rId);

			// Delete the right one
			delete mySegments[iRightIndex];
			mySegments.erase(mySegments.begin() + iRightIndex);
		}
		else
		{
			_ASSERT(mySegments[iLeftIndex] == pSegment);

			mySegments[iLeftIndex]->getStart(svTemp);
			mySegments[iRightIndex]->setStart(svTemp.x, svTemp.y);
			mySegments[iLeftIndex]->getControl1(svTemp);
			mySegments[iRightIndex]->setControl1(svTemp.x, svTemp.y);

			delete mySegments[iLeftIndex];
			mySegments.erase(mySegments.begin() + iLeftIndex);
		}
	}
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurve<TYPE>::convertPoint(TYPE* pSegment, BezierPointType ePointType, BezierPointCurveType eCornerType)
{
	if(ePointType != BezierPointStart && ePointType != BezierPointEnd)
		ASSERT_RETURN;

	int iCurr, iNum = mySegments.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(mySegments[iCurr] == pSegment)
			break;
	}

	if(iCurr >= iNum)
		ASSERT_RETURN;

	if(ePointType == BezierPointStart)
	{
		int iPrevIndex = iCurr - 1;
		if(iPrevIndex < 0 && myIsClosed)
			iPrevIndex += iNum;

		pSegment->setStartPointCurveType(eCornerType);
		pSegment->ensureIsLinear();
		if(iPrevIndex >= 0 && iPrevIndex < iNum)
		{
			mySegments[iPrevIndex]->setEndPointCurveType(eCornerType);
			mySegments[iPrevIndex]->ensureIsLinear();
		}
	}
	else
	{
		int iNextIndex = iCurr + 1;
		if(iNextIndex >= iNum && myIsClosed)
			iNextIndex -= iNum;

		pSegment->setEndPointCurveType(eCornerType);
		pSegment->ensureIsLinear();
		if(iNextIndex >= 0 && iNextIndex < iNum)
		{
			mySegments[iNextIndex]->setStartPointCurveType(eCornerType);
			mySegments[iNextIndex]->ensureIsLinear();
		}
	}
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurve<TYPE>::splitEdge(TYPE* pSegment, FLOAT_TYPE fSplitRatio)
{
	int iCurr, iNum = mySegments.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(mySegments[iCurr] == pSegment)
		{
			// Got it!
			if(fSplitRatio < 0.0) fSplitRatio = 0.0;
			if(fSplitRatio > 1.0) fSplitRatio = 1.0;

			SVector2D svTemp;
			SVector2D svNewPoint, svLeftControlPoint, svRightControlPoint;

			// Figure out the new tangents and the actual point
			pSegment->evaluate(fSplitRatio, svNewPoint);

			SVector2D svPoints[4];
			pSegment->getStart(svPoints[0]);
			pSegment->getControl1(svPoints[1]);
			pSegment->getControl2(svPoints[2]);
			pSegment->getEnd(svPoints[3]);

			SVector2D svPt1, svPt2, svPt3;
			SVector2D svA, svB;
			svPt1 = svPoints[0]*(1.0-fSplitRatio) + svPoints[1]*fSplitRatio;
			svPt2 = svPoints[1]*(1.0-fSplitRatio) + svPoints[2]*fSplitRatio;
			svPt3 = svPoints[2]*(1.0-fSplitRatio) + svPoints[3]*fSplitRatio;

			svA = svPt1*(1.0-fSplitRatio) + svPt2*fSplitRatio;
			svB = svPt2*(1.0-fSplitRatio) + svPt3*fSplitRatio;

			svLeftControlPoint = svA;
			svRightControlPoint = svB;

			// We need to insert a new bezier.
			// Note that we need to assign new ids to both curves, as if they're both new.

			EdgeId idOldCurve;
			EdgeId idLeftSegment, idRightSegment;
			idOldCurve = pSegment->getEdgeId();
			idOldCurve.generateSplitIds(idLeftSegment, idRightSegment);

			TYPE* pNewCurve = new TYPE();
			pNewCurve->setParent(this);
	
			if(myParent)
				pNewCurve->setEdgeId(idRightSegment);
			ELSE_ASSERT;

			pSegment->getEnd(svTemp);
			pNewCurve->setStart(svNewPoint.x, svNewPoint.y);
			pNewCurve->setControl1(svRightControlPoint.x, svRightControlPoint.y);
			pNewCurve->setEnd(svTemp.x, svTemp.y);
			pNewCurve->setControl2(svPt3.x, svPt3.y);
			if(pSegment->getStartPointCurveType() == BezierPointCurveSharp && pSegment->getEndPointCurveType() == BezierPointCurveSharp)
				pNewCurve->makeLinear();
			//pNewCurve->setIsLinear(pSegment->getIsLinear());

			pSegment->setEnd(svNewPoint.x, svNewPoint.y);
			pSegment->setControl2(svLeftControlPoint.x,  svLeftControlPoint.y);
			pSegment->setControl1(svPt1.x, svPt1.y);			

			// Insert the thing
			mySegments.insert(mySegments.begin() + (iCurr + 1), pNewCurve);
			
			if(myParent)
				pSegment->setEdgeId(idLeftSegment);
			ELSE_ASSERT;

			return;
		}
	}

	// Segment not found
	_ASSERT(0);
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurve<TYPE>::copyFrom(const TBezierCurve& rOther, FLOAT_TYPE fPerfScalingFactor)
{
	clear();

	myIsAHole = rOther.myIsAHole;
	myIsBackground = rOther.myIsBackground;
	myIsClosed = rOther.myIsClosed;
	myMaterialId = rOther.myMaterialId;

	myIsStartArrowEnabled = rOther.myIsStartArrowEnabled;
	myStartArrowLengthMult = rOther.myStartArrowLengthMult;
	myStartArrowWidthMult = rOther.myStartArrowWidthMult;
	myIsEndArrowEnabled = rOther.myIsEndArrowEnabled;
	myEndArrowLengthMult = rOther.myEndArrowLengthMult;
	myEndArrowWidthMult = rOther.myEndArrowWidthMult;


	TYPE* pNewCurve;
	int iCurr, iNum = rOther.mySegments.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pNewCurve = dynamic_cast<TYPE*>(rOther.mySegments[iCurr]->cloneSelf(fPerfScalingFactor));
		mySegments.push_back(pNewCurve);
		pNewCurve->setParent(this);
	}
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurve<TYPE>::onChildEdgeIdChanged(const EdgeId& idOld, const EdgeId& idNew) 
{ 
//	_ASSERT(myParent);
	if(myParent) 
		myParent->onChildSegmentEdgeIdChanged(idOld, idNew); 
}
/*****************************************************************************/
template < class TYPE > 
UNIQUEID_TYPE TBezierCurve<TYPE>::getNextId() 
{ 
	if(myParent) 
		return myParent->getNextId();  
	else 
	{ _ASSERT(0); return 0; } 
}
/*****************************************************************************/
template < class TYPE > 
UNIQUEID_TYPE TBezierCurve<TYPE>::getMaxId() const
{
	UNIQUEID_TYPE iMaxId = 0;
	int iCurr, iNum = mySegments.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(mySegments[iCurr]->getEdgeId().getNumeric() > iMaxId)
			iMaxId = mySegments[iCurr]->getEdgeId().getNumeric();
	}

	return iMaxId;
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurve<TYPE>::transformBy(const SMatrix2D& smTransform)
{
	int iCurr, iNum = mySegments.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		mySegments[iCurr]->transformBy(smTransform);
	}
}
/*****************************************************************************/
template < class TYPE >
TYPE* TBezierCurve<TYPE>::insertLinear(int iPos, SVector2D& svStart, SVector2D& svEnd, UNIQUEID_TYPE lOptEdgeId)
{
	SVector2D svTemp;
	svTemp = (svStart + svEnd)*0.5;
	return insertBezier(iPos, svStart, svTemp, svTemp, svEnd, true, lOptEdgeId);
}
/*****************************************************************************/
template < class TYPE >
TYPE* TBezierCurve<TYPE>::insertBezier(int iPos, SVector2D& svStart, SVector2D& svControl1, SVector2D& svControl2, SVector2D& svEnd, bool bIsLinear /*= false*/, UNIQUEID_TYPE lOptEdgeId /*= 0*/)
{
	TYPE* pNew = new TYPE();

	pNew->setStart(svStart.x, svStart.y);
	pNew->setEnd(svEnd.x, svEnd.y);
	pNew->setControl1(svControl1.x, svControl1.y);
	pNew->setControl2(svControl2.x, svControl2.y);
	if(bIsLinear)
		pNew->makeLinear();
	//pNew->setIsLinear(bIsLinear);
	pNew->setParent(this);

	if(lOptEdgeId > 0)
		pNew->setEdgeId(lOptEdgeId);
	else
	{
		_ASSERT(myParent);
		if(myParent)
			pNew->setEdgeId(myParent->getNextId());
	}

	mySegments.insert(mySegments.begin() + iPos, pNew);

	return pNew;

}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurve<TYPE>::operator=(const TBezierCurve& other)
{
	clear();

	TYPE *pNewCurve;
	TYPE* pCurve;
	int iCurve, iNumCurves = other.mySegments.size();
	for(iCurve = 0; iCurve < iNumCurves; iCurve++)
	{
		pCurve = other.mySegments[iCurve];
		pNewCurve = new TYPE();
		*pNewCurve = *pCurve;

		mySegments.push_back(pNewCurve);
	}
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurve<TYPE>::reverse()
{
	std::reverse(mySegments.begin(), mySegments.end());
	int iCurr, iNum = mySegments.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		mySegments[iCurr]->reverse();
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurve<TYPE>::append(const TBezierCurve& rSource)
{
	// What to do about ids?
	TYPE* pNewCopy;
	TYPE* pIdDuplicate;
	int iCurr, iNum = rSource.mySegments.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pNewCopy = dynamic_cast<TYPE*>(rSource.mySegments[iCurr]->cloneSelf());
		_ASSERT(pNewCopy);
		pIdDuplicate = this->findSegmentByEdgeId(pNewCopy->getEdgeId());
		if(pIdDuplicate)
		{
			// Ahutng! Set new id:
			_ASSERT(myParent);
			if(myParent)
				pNewCopy->setEdgeId(myParent->getNextId());
		}
		addExistingSegment(pNewCopy);
	}
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurve<TYPE>::loadFromItem(const StringResourceItem& rItem)
{
	clear();

	myIsClosed = rItem.getBoolProp(SAVE_BEZIER_CURVE_IS_CLOSED);
	myIsBackground = rItem.getBoolProp(SAVE_BEZIER_CURVE_IS_BACKGROUND);
	myIsAHole = rItem.getBoolProp(SAVE_BEZIER_CURVE_IS_AHOLE);
	myMaterialId = rItem.getAsLong(SAVE_BEZIER_CURVE_MAT_ID);

	if(rItem.doesPropertyExist(SAVE_BEZIER_CURVE_START_ARROW_ENABLED))
	{
		myIsStartArrowEnabled = rItem.getBoolProp(SAVE_BEZIER_CURVE_START_ARROW_ENABLED);
		myStartArrowLengthMult = rItem.getNumProp(SAVE_BEZIER_CURVE_START_ARROW_LENGTH_MULT);
		myStartArrowWidthMult = rItem.getNumProp(SAVE_BEZIER_CURVE_START_ARROW_WIDTH_MULT);

		myIsEndArrowEnabled = rItem.getBoolProp(SAVE_BEZIER_CURVE_END_ARROW_ENABLED);
		myEndArrowLengthMult = rItem.getNumProp(SAVE_BEZIER_CURVE_END_ARROW_LENGTH_MULT);
		myEndArrowWidthMult = rItem.getNumProp(SAVE_BEZIER_CURVE_END_ARROW_WIDTH_MULT);
	}

	TYPE* pSegment;
	const StringResourceItem* pChild;
	int iChild, iNumChildren = rItem.getNumChildren();
	for(iChild = 0; iChild < iNumChildren; iChild++)
	{
		pChild = rItem.getChild(iChild);

		pSegment = new TYPE;
		pSegment->loadFromItem(*pChild);
		addExistingSegment(pSegment);
	}
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurve<TYPE>::saveToItem(StringResourceItem& rItemOut) const
{
	rItemOut.setBoolProp(SAVE_BEZIER_CURVE_IS_CLOSED, myIsClosed);
	rItemOut.setBoolProp(SAVE_BEZIER_CURVE_IS_BACKGROUND, myIsBackground);
	rItemOut.setBoolProp(SAVE_BEZIER_CURVE_IS_AHOLE, myIsAHole);
	rItemOut.setAsLong(SAVE_BEZIER_CURVE_MAT_ID, myMaterialId);

	rItemOut.setBoolProp(SAVE_BEZIER_CURVE_START_ARROW_ENABLED, myIsStartArrowEnabled);
	rItemOut.setNumProp(SAVE_BEZIER_CURVE_START_ARROW_LENGTH_MULT, myStartArrowLengthMult);
	rItemOut.setNumProp(SAVE_BEZIER_CURVE_START_ARROW_WIDTH_MULT, myStartArrowWidthMult);

	rItemOut.setBoolProp(SAVE_BEZIER_CURVE_END_ARROW_ENABLED, myIsEndArrowEnabled);
	rItemOut.setNumProp(SAVE_BEZIER_CURVE_END_ARROW_LENGTH_MULT, myEndArrowLengthMult);
	rItemOut.setNumProp(SAVE_BEZIER_CURVE_END_ARROW_WIDTH_MULT, myEndArrowWidthMult);

	StringResourceItem* pChild;
	int iCurr, iNum = mySegments.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = rItemOut.addChildAndSetId(SAVE_BEZIER_SEGMENT);
		mySegments[iCurr]->saveToItem(*pChild);
	}
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurve<TYPE>::ensurePointsClosed(bool bMoveLast)
{
	if(!myIsClosed)
		return;

	int iNumSegments = mySegments.size();
	if(iNumSegments < 2)
		return;

	SVector2D svFirstPoint, svLastPoint;
	mySegments[0]->getStart(svFirstPoint);
	mySegments[iNumSegments - 1]->getEnd(svLastPoint);

	if((svFirstPoint - svLastPoint).lengthSquared() > 1.0)
	{
		if(bMoveLast)
			mySegments[iNumSegments - 1]->setEnd(svFirstPoint.x, svFirstPoint.y);
		else
			addLinear(svLastPoint, svFirstPoint);
	}
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurve<TYPE>::fromPolyline(const BezierPolyline2D& rPolylineIn, bool bIsClosed)
{
	clearSegments();

	TYPE *pNewBezier;
	SVector2D svControl1, svControl2;
	BezierPointCurveType eCurveTypePrev, eCurveTypeCurr;
	int iPoint, iNumPoints = rPolylineIn.size();
	for(iPoint = 1; iPoint < iNumPoints; iPoint++)
	{
		eCurveTypePrev = rPolylineIn[iPoint - 1].getPointType();
		eCurveTypeCurr = rPolylineIn[iPoint].getPointType();

		if(eCurveTypeCurr == BezierPointCurveSharp && eCurveTypePrev == BezierPointCurveSharp)
			pNewBezier = addLinear(rPolylineIn[iPoint - 1].getPoint(), rPolylineIn[iPoint].getPoint());
		else
		{
			// Add bezier
			rPolylineIn[iPoint - 1].getControlNext(svControl1);
			rPolylineIn[iPoint].getControlPrev(svControl2);
			pNewBezier = addBezier(rPolylineIn[iPoint - 1].getPoint(), svControl1, svControl2, rPolylineIn[iPoint].getPoint(), false);
			pNewBezier->setStartPointCurveType(eCurveTypePrev);
			pNewBezier->setEndPointCurveType(eCurveTypeCurr);
		}

		pNewBezier->setEdgeId(rPolylineIn[iPoint - 1].getEdgeId());
	}

	myIsAHole = rPolylineIn.getIsHole();

	myIsClosed = bIsClosed;
	ensurePointsClosed(true);
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurve<TYPE>::toPolyline(BezierPolyline2D& rPolylineOut) const
{
	// Note that right now this assumes we only have linear segments
	rPolylineOut.clear();
	SVector2D svPoint, svControlPrev, svControlNext;
	SBezierPoint rBezPoint;
	int iCurrSegment, iNumSegments = mySegments.size();
	for(iCurrSegment = 0; iCurrSegment < iNumSegments; iCurrSegment++)
	{
		mySegments[iCurrSegment]->getStart(svPoint);
		mySegments[iCurrSegment]->getControl1(svControlNext);
		if(iCurrSegment > 0)
			mySegments[iCurrSegment - 1]->getControl2(svControlPrev);


		rBezPoint.setPoint(svPoint, svControlPrev, svControlNext, mySegments[iCurrSegment]->getStartPointCurveType(), mySegments[iCurrSegment]->getEdgeId());
		rPolylineOut.push_back(rBezPoint);
		//rPolylineOut.push_back(svPoint);
	}

	if(iNumSegments > 0)
	{
		mySegments[iNumSegments - 1]->getEnd(svPoint);
		mySegments[iNumSegments - 1]->getControl2(svControlPrev);
		svControlNext.set(0, 0);
		rBezPoint.setPoint(svPoint, svControlPrev, svControlNext, mySegments[iNumSegments - 1]->getStartPointCurveType(), mySegments[iNumSegments - 1]->getEdgeId());
		rPolylineOut.push_back(rBezPoint);
		//rPolylineOut.push_back(svPoint);
	}

	rPolylineOut.setIsHole(myIsAHole);
}
/*****************************************************************************/
template < class TYPE >
FLOAT_TYPE TBezierCurve<TYPE>::getLength() const
{
	FLOAT_TYPE fRes = 0;
	int iCurr, iNum = mySegments.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		fRes += mySegments[iCurr]->getLength(1.0, -1);
	return fRes;
}
/*****************************************************************************/
template < class TYPE > template < class POINT_TYPE >
void TBezierCurve<TYPE>::fitBezierFromPoints(TPolyline2D<POINT_TYPE>& rPolylineInMofiable, FLOAT_TYPE fMaxError, bool bIsClosed)
{
	clearSegments();

	// We need to ensure no poitns are actually coincident.
	rPolylineInMofiable.removeCoincident(FLOAT_EPSILON);
	if(rPolylineInMofiable.size() < 3)
	{
		//fromPolyline(rPolylineInMofiable, bIsClosed);
		if(rPolylineInMofiable.size() == 2)
			addLinear(rPolylineInMofiable[0].getPoint(), rPolylineInMofiable[1].getPoint(), 1);
		return;
	}

	SVector2D svLeftTangent, svRightTangent;
	svLeftTangent = rPolylineInMofiable[1].getPoint() - rPolylineInMofiable[0].getPoint();
	svLeftTangent.normalize();

	svRightTangent = rPolylineInMofiable[rPolylineInMofiable.size() - 2].getPoint() - rPolylineInMofiable[rPolylineInMofiable.size() - 1].getPoint();
	svRightTangent.normalize();

	fitBezierRecursive(rPolylineInMofiable, 0, rPolylineInMofiable.size() - 1, svLeftTangent, svRightTangent, fMaxError*fMaxError);

	myIsClosed = bIsClosed;
	ensurePointsClosed(true);
}
/*****************************************************************************/
template < class TYPE > template < class POINT_TYPE >
void TBezierCurve<TYPE>::chordLengthParameterize(const TPolyline2D<POINT_TYPE>& rPoints, int iFirst, int iLast, TFloatVector &rUParams)
{
	int iCurrPoint;

	rUParams.reserve(iLast - iFirst + 1);
	rUParams.insert(rUParams.begin(), iLast - iFirst + 1, 0.0);
	//rUParams[0] = 0.0;

	for(iCurrPoint = iFirst + 1; iCurrPoint <= iLast; iCurrPoint++)
	{
		rUParams[iCurrPoint - iFirst] = rUParams[iCurrPoint - iFirst - 1] + (rPoints[iCurrPoint].getPoint() - rPoints[iCurrPoint-1].getPoint()).length();
	}

	for(iCurrPoint = iFirst + 1; iCurrPoint <= iLast; iCurrPoint++)
	{
		rUParams[iCurrPoint - iFirst] /= rUParams[iLast - iFirst];
	}
}
/*****************************************************************************/
inline FLOAT_TYPE B0(FLOAT_TYPE dU) { FLOAT_TYPE dTemp = 1.0 - dU; return (dTemp * dTemp * dTemp); }
inline FLOAT_TYPE B1(FLOAT_TYPE dU) { FLOAT_TYPE dTemp = 1.0 - dU; return (3.0 * dU * dTemp * dTemp); }
inline FLOAT_TYPE B2(FLOAT_TYPE dU) { FLOAT_TYPE dTemp = 1.0 - dU; return (3.0 * dU * dU * dTemp); }
inline FLOAT_TYPE B3(FLOAT_TYPE dU) { return (dU * dU * dU); }

template < class TYPE >  template < class POINT_TYPE >
void TBezierCurve<TYPE>::generateBezier(const TPolyline2D<POINT_TYPE>& rPoints, int iFirst, int iLast, const TFloatVector &rUParams, const SVector2D& rLeftTangent, const SVector2D& rRightTangent, TYPE& rNewBezierOut)
{
	int iCurrPoint;
	int iNumPoints = iLast - iFirst + 1;

//	TPointVector myFittingVecA1, myFittingVecA2;
	myFittingVecA1.clear();
	myFittingVecA2.clear();

	FLOAT_TYPE 	C[2][2];
	FLOAT_TYPE 	X[2];
	FLOAT_TYPE 	det_C0_C1, det_C0_X, det_X_C1;
	FLOAT_TYPE 	alpha_l = 0.0, alpha_r = 0.0;

	SVector2D svTemp1, svTemp2;
	myFittingVecA1.reserve(iNumPoints);
	myFittingVecA2.reserve(iNumPoints);

	for (iCurrPoint = 0; iCurrPoint < iNumPoints; iCurrPoint++) 
	{
		svTemp1 = rLeftTangent * B1(rUParams[iCurrPoint]);
		svTemp2 = rRightTangent * B2(rUParams[iCurrPoint]);

		myFittingVecA1.push_back(svTemp1);
		myFittingVecA2.push_back(svTemp2);

		//		A1[iCurrPoint] =  svTemp1;
		//		A2[iCurrPoint] =  svTemp2;
	}

	// Create the C and X matrices
	C[0][0] = 0.0;
	C[0][1] = 0.0;
	C[1][0] = 0.0;
	C[1][1] = 0.0;
	X[0]    = 0.0;
	X[1]    = 0.0;

	for (iCurrPoint = 0; iCurrPoint < iNumPoints; iCurrPoint++) 
	{
		C[0][0] += myFittingVecA1[iCurrPoint].dot(myFittingVecA1[iCurrPoint]);
		C[0][1] += myFittingVecA1[iCurrPoint].dot(myFittingVecA2[iCurrPoint]);

		C[1][0] = C[0][1];
		C[1][1] += myFittingVecA2[iCurrPoint].dot(myFittingVecA2[iCurrPoint]);

		svTemp1 = rPoints[iFirst + iCurrPoint].getPoint() - 
			(rPoints[iFirst].getPoint()*B0(rUParams[iCurrPoint]) + 
			(rPoints[iFirst].getPoint()*B1(rUParams[iCurrPoint]) + 
			(rPoints[iLast].getPoint()*B2(rUParams[iCurrPoint]) + rPoints[iLast].getPoint()*B3(rUParams[iCurrPoint]))
			)
			);

		X[0] += myFittingVecA1[iCurrPoint].dot(svTemp1);
		X[1] += myFittingVecA2[iCurrPoint].dot(svTemp1);

	}

	// Compute the determinants of C and X
	det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
	det_C0_X  = C[0][0] * X[1]    - C[0][1] * X[0];
	det_X_C1  = X[0]    * C[1][1] - X[1]    * C[0][1];

	// Finally, derive alpha values	
	if (det_C0_C1 == 0.0) 
		det_C0_C1 = (C[0][0] * C[1][1]) * (FLOAT_EPSILON/2.0);

	if (det_C0_C1 != 0.0) 
	{
		alpha_l = det_X_C1 / det_C0_C1;
		alpha_r = det_C0_X / det_C0_C1;
	}

	//  If alpha negative, use the Wu/Barsky heuristic (see text)
	// (if alpha is 0, you get coincident control points that lead to
	// divide by zero in any subsequent NewtonRaphsonRootFind() call.
	if (alpha_l < FLOAT_EPSILON || alpha_r < FLOAT_EPSILON) 
	{
		FLOAT_TYPE dDist = (rPoints[iLast].getPoint() - rPoints[iFirst].getPoint()).length()/3.0;

		rNewBezierOut.setStart(rPoints[iFirst].getPoint().x, rPoints[iFirst].getPoint().y);
		rNewBezierOut.setEnd(rPoints[iLast].getPoint().x, rPoints[iLast].getPoint().y);

		svTemp1 = rPoints[iFirst].getPoint() + rLeftTangent*dDist;
		rNewBezierOut.setControl1(svTemp1.x, svTemp1.y);

		svTemp1 = rPoints[iLast].getPoint() + rRightTangent*dDist;
		rNewBezierOut.setControl2(svTemp1.x, svTemp1.y);

		rNewBezierOut.setStartPointCurveType(BezierPointCurveTied);
		rNewBezierOut.setEndPointCurveType(BezierPointCurveTied);

		return;
	}

	//  First and last control points of the Bezier curve are 
	//  positioned exactly at the first and last data points 
	//  Control points 1 and 2 are positioned an alpha distance out 
	//  on the tangent vectors, left and right, respectively 
	rNewBezierOut.setStart(rPoints[iFirst].getPoint().x, rPoints[iFirst].getPoint().y);
	rNewBezierOut.setEnd(rPoints[iLast].getPoint().x, rPoints[iLast].getPoint().y);

	svTemp1 = rPoints[iFirst].getPoint() + rLeftTangent*alpha_l;
	rNewBezierOut.setControl1(svTemp1.x, svTemp1.y);

	svTemp1 = rPoints[iLast].getPoint() + rRightTangent*alpha_r;
	rNewBezierOut.setControl2(svTemp1.x, svTemp1.y);

	rNewBezierOut.setStartPointCurveType(BezierPointCurveTied);
	rNewBezierOut.setEndPointCurveType(BezierPointCurveTied);
}
/****************************************************************************************************/
void evaluteBezier(int iDegree, const SVector2D* pPoints, FLOAT_TYPE dT, SVector2D& rPointOut)
{
	SVector2D svVTemp[4];
	int iCurrDegree, iInnerDegree;
	for(iCurrDegree = 0; iCurrDegree <= iDegree; iCurrDegree++)
		svVTemp[iCurrDegree] = pPoints[iCurrDegree];

	for(iCurrDegree = 1; iCurrDegree <= iDegree; iCurrDegree++)	
	{
		for(iInnerDegree = 0; iInnerDegree <= iDegree-iCurrDegree; iInnerDegree++)
			svVTemp[iInnerDegree] = svVTemp[iInnerDegree]*(1.0 - dT) + svVTemp[iInnerDegree+1]*dT;
	}	
	rPointOut = svVTemp[0];
}

template < class TYPE >
FLOAT_TYPE TBezierCurve<TYPE>::doNewtonRaphsonRootFind(const TYPE& rNewBezier, const SVector2D& rPoint, FLOAT_TYPE dUParam)
{
	SVector2D Q1[3], Q2[2];	//  Q' and Q''
	FLOAT_TYPE 		dNumerator, dDenominator;
	SVector2D Q_u, Q1_u, Q2_u; // u evaluated at Q, Q', & Q''
	FLOAT_TYPE dUPrime;
	int i;

	SVector2D Q[4]; // Original points
	rNewBezier.getStart(Q[0]);
	rNewBezier.getControl1(Q[1]);
	rNewBezier.getControl2(Q[2]);
	rNewBezier.getEnd(Q[3]);

	// Compute Q(u)
	evaluteBezier(3, Q, dUParam, Q_u);

	// Generate control vertices for Q'
	for (i = 0; i <= 2; i++) 
		Q1[i] = (Q[i+1] - Q[i]) * 3.0;

	// Generate control vertices for Q''
	for (i = 0; i <= 1; i++) 
		Q2[i] = (Q1[i+1] - Q1[i]) * 2.0;

	// Compute Q'(u) and Q''(u)
	evaluteBezier(2, Q1, dUParam, Q1_u);
	evaluteBezier(1, Q2, dUParam, Q2_u);

	// Compute f(u)/f'(u) 
	dNumerator = (Q_u.x - rPoint.x) * (Q1_u.x) + (Q_u.y - rPoint.y) * (Q1_u.y);
	dDenominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) +
		(Q_u.x - rPoint.x) * (Q2_u.x) + (Q_u.y - rPoint.y) * (Q2_u.y);

	// u = u - f(u)/f'(u)
	dUPrime = dUParam - (dNumerator/dDenominator);
	return dUPrime;
}
/******************************************************************************************************************/
template < class TYPE >  template < class POINT_TYPE >
void TBezierCurve<TYPE>::reparameterize(const TPolyline2D<POINT_TYPE>& rPoints, int iFirst, int iLast, TFloatVector &rUParams, TYPE& rNewBezier, TFloatVector &rNewUParamsOut)
{
	rNewUParamsOut.reserve(iLast - iFirst + 1);
	rNewUParamsOut.insert(rNewUParamsOut.begin(), iLast - iFirst + 1, 0.0);

	int iCurrPoint;
	for(iCurrPoint = iFirst ; iCurrPoint <= iLast; iCurrPoint++) 
		rNewUParamsOut[iCurrPoint-iFirst] = doNewtonRaphsonRootFind(rNewBezier, rPoints[iCurrPoint].getPoint(), rUParams[iCurrPoint - iFirst]);
}
/******************************************************************************************************************/
template < class TYPE > template < class POINT_TYPE >
void TBezierCurve<TYPE>::fitBezierRecursive(const TPolyline2D<POINT_TYPE>& rPolylineIn, int iFirst, int iLast, const SVector2D& svLeftTangent, const SVector2D& svRightTangent, FLOAT_TYPE dErrorSq)
{
	const int iMaxIterations = 4;
	int iIterationError = dErrorSq*dErrorSq;
	int iNumPoints = iLast - iFirst + 1;
	FLOAT_TYPE dMaxError;
	int iMaxErrorPoint;

	SVector2D svTemp;
	if(iNumPoints == 2)
	{
		
		TYPE *pNewBezier = new TYPE();
		FLOAT_TYPE dDist = (rPolylineIn[iLast].getPoint() - rPolylineIn[iFirst].getPoint()).length()/3.0;
		SVector2D svTempPoint;
		svTempPoint = rPolylineIn[iFirst].getPoint();
		pNewBezier->setStart(svTempPoint.x, svTempPoint.y);
		svTempPoint = rPolylineIn[iLast].getPoint();
		pNewBezier->setEnd(svTempPoint.x, svTempPoint.y);

		svTemp = rPolylineIn[iFirst].getPoint() + svLeftTangent*dDist;
		pNewBezier->setControl1(svTemp.x, svTemp.y);

		svTemp = rPolylineIn[iLast].getPoint() + svRightTangent*dDist;
		pNewBezier->setControl2(svTemp.x, svTemp.y);

		pNewBezier->setStartPointCurveType(BezierPointCurveTied);
		pNewBezier->setEndPointCurveType(BezierPointCurveTied);

		addExistingSegment(pNewBezier);
		if(myParent)
			pNewBezier->setEdgeId(myParent->getNextId());

		return;
	}

	//TFloatVector myFittingUParams;
	myFittingUParams.clear();

	TYPE rTempBezier;
	chordLengthParameterize(rPolylineIn, iFirst, iLast, myFittingUParams);
	generateBezier(rPolylineIn, iFirst, iLast, myFittingUParams, svLeftTangent, svRightTangent, rTempBezier);

	//  Find max deviation of points to fitted curve 
	dMaxError = rTempBezier.computeMaxError(rPolylineIn, iFirst, iLast, myFittingUParams, iMaxErrorPoint);
	if (dMaxError < dErrorSq) 
	{
		TYPE* pNewCopy = dynamic_cast<TYPE*>(rTempBezier.cloneSelf());
		addExistingSegment(pNewCopy);
		if(myParent)
			pNewCopy->setEdgeId(myParent->getNextId());
		return;
	}

	//  If error not too large, try some reparameterization 
	//  and iteration 
	if(dMaxError < iIterationError)
	{
		int iCurrIteration;
		//TFloatVector myFittingUNewParams;
		myFittingUNewParams.clear();
		for (iCurrIteration = 0; iCurrIteration < iMaxIterations; iCurrIteration++) 
		{
			reparameterize(rPolylineIn, iFirst, iLast, myFittingUParams, rTempBezier, myFittingUNewParams);
			generateBezier(rPolylineIn, iFirst, iLast, myFittingUNewParams, svLeftTangent, svRightTangent, rTempBezier);

			dMaxError = rTempBezier.computeMaxError(rPolylineIn, iFirst, iLast, myFittingUNewParams, iMaxErrorPoint);
			if(dMaxError < dErrorSq)
			{
				// We're done
				TYPE* pNewCopy = dynamic_cast<TYPE*>(rTempBezier.cloneSelf());
				addExistingSegment(pNewCopy);
				if(myParent)
					pNewCopy->setEdgeId(myParent->getNextId());
				return;
			}
		}

	}

	// Fitting failed -- split at max error point and fit recursively 
	SVector2D svCenterTangent, svTemp2;
	svTemp = rPolylineIn[iMaxErrorPoint - 1].getPoint() - rPolylineIn[iMaxErrorPoint].getPoint();
	svTemp2 = rPolylineIn[iMaxErrorPoint].getPoint() - rPolylineIn[iMaxErrorPoint+1].getPoint();

	svCenterTangent = (svTemp + svTemp2)*0.5;
	svCenterTangent.normalize();

	fitBezierRecursive(rPolylineIn, iFirst, iMaxErrorPoint, svLeftTangent, svCenterTangent, dErrorSq);

	svCenterTangent *= -1;
	fitBezierRecursive(rPolylineIn, iMaxErrorPoint, iLast, svCenterTangent, svRightTangent, dErrorSq);
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurve<TYPE>::setEdgeSourceIds(UNIQUEID_TYPE idSourceId)
{
	int iSegment, iNumSegments = mySegments.size();
	for(iSegment = 0; iSegment < iNumSegments; iSegment++)
		mySegments[iSegment]->setEdgeIdSourceId(idSourceId);
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurve<TYPE>::resetEdgeIdsWithString(const char* pcsStringPart)
{
	char pcsBuff[256];
	int iSegment, iNumSegments = mySegments.size();
	for(iSegment = 0; iSegment < iNumSegments; iSegment++)
	{
		sprintf(pcsBuff, "%sc%d", pcsStringPart, iSegment);
		mySegments[iSegment]->setEdgeId(pcsBuff);
	}
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurve<TYPE>::makeIntoRectangle(FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fW, FLOAT_TYPE fH)
{
	clearSegments();

	SVector2D svStart, svEnd;

	svStart.set(fStartX, fStartY);
	svEnd.set(fStartX + fW, fStartY);
	this->addLinear(svStart, svEnd, EDGE_ID_TOP);

	svStart = svEnd;
	svEnd.set(fStartX + fW, fStartY + fH);
	this->addLinear(svStart, svEnd, EDGE_ID_RIGHT);

	svStart = svEnd;
	svEnd.set(fStartX, fStartY + fH);
	this->addLinear(svStart, svEnd, EDGE_ID_BOTTOM);

	svStart = svEnd;
	svEnd.set(fStartX, fStartY);
	this->addLinear(svStart, svEnd, EDGE_ID_LEFT);

}
/*****************************************************************************/
HYPERUI_EXTERN template class HYPERUI_API TBezierCurve< DeformableBezier >;
HYPERUI_EXTERN template class HYPERUI_API TBezierCurve< SAnimBezierSegment >;
HYPERUI_EXTERN template class HYPERUI_API TBezierCurve< SBezierSegment >;

template HYPERUI_API void TBezierCurve<SBezierSegment>::fitBezierFromPoints<SBrushPoint>(TPolyline2D<SBrushPoint>& rPolylineIn, FLOAT_TYPE fMaxError, bool bIsClosed);
template HYPERUI_API void TBezierCurve<SBezierSegment>::fitBezierFromPoints<SBezierPoint>(TPolyline2D<SBezierPoint>& rPolylineIn, FLOAT_TYPE fMaxError, bool bIsClosed);
};