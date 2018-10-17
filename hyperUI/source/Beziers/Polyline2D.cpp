#include "stdafx.h"

#ifdef HAVE_POLYBOOL
#include "polybool.h"

using namespace POLYBOOLEAN; 
#endif

namespace HyperUI
{
/********************************************************************************************/
template < class POINT_TYPE >
TPolyline2D<POINT_TYPE>::TPolyline2D()
{
	myIsHole = false;
}
/********************************************************************************************/
template < class POINT_TYPE >
TPolyline2D<POINT_TYPE>::~TPolyline2D()
{

}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::translate(FLOAT_TYPE fX, FLOAT_TYPE fY)
{
	TPolyline2D<POINT_TYPE>::translate(*this, fX, fY);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::scale(FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY)
{
	TPolyline2D<POINT_TYPE>::scale(*this, fScaleX, fScaleY);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::subdivide(FLOAT_TYPE fMaxDistance)
{
	TPolyline2D<POINT_TYPE>::subdivide(*this, fMaxDistance);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::transform(const SMatrix2D& smMatrix)
{
	TPolyline2D<POINT_TYPE>::transform(*this, smMatrix);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::transform(const CombinedTransform& smCombTransform)
{
	TPolyline2D<POINT_TYPE>::transform(*this, smCombTransform);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::bend(FLOAT_TYPE fCenter, FLOAT_TYPE fAngle)
{
	TPolyline2D<POINT_TYPE>::bend(*this, fCenter, fAngle);
}
/********************************************************************************************/
template < class POINT_TYPE >
SVertexInfo* TPolyline2D<POINT_TYPE>::triangulateAsPolygon(int &iNumVertsOut) const
{
	return TPolyline2D<POINT_TYPE>::triangulateAsPolygon(*this, iNumVertsOut);
}
/*****************************************************************************/
template < class POINT_TYPE >
HyperCore::OrientationType TPolyline2D<POINT_TYPE>::getPolygonOrientation() const
{
	return getPolygonOrientation(*this);
}
/*****************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::rotate(FLOAT_TYPE fAngle, FLOAT_TYPE fPivotX, FLOAT_TYPE fPivotY, int iStartIndex, int iEndIndex)
{
	TPolyline2D<POINT_TYPE>::rotate(*this, fAngle, fPivotX, fPivotY, iStartIndex, iEndIndex);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::generateRectangle(const SRect2D& srRect)
{
	TPolyline2D<POINT_TYPE>::generateRectangle(*this, srRect);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::generateAtOrigin(FLOAT_TYPE fLength, FLOAT_TYPE fPointSpacing)
{
	TPolyline2D<POINT_TYPE>::generateAtOrigin(*this, fLength, fPointSpacing);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::generateNoiseLineAtOrigin(FLOAT_TYPE fLength, FLOAT_TYPE fMinPointSpacing, PerlinNoiseGenerator& rNoiseGenerator, FLOAT_TYPE fNoiseTime)
{
	TPolyline2D<POINT_TYPE>::generateNoiseLineAtOrigin(*this, fLength, fMinPointSpacing, rNoiseGenerator, fNoiseTime);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::generateSpiralAtOrigin(FLOAT_TYPE fStartRadius, FLOAT_TYPE fRollProgress, FLOAT_TYPE fLength, FLOAT_TYPE fPointSpacing)
{
	TPolyline2D<POINT_TYPE>::generateSpiralAtOrigin(*this, fStartRadius, fRollProgress, fLength, fPointSpacing);
}
/********************************************************************************************/
template < class POINT_TYPE >
FLOAT_TYPE TPolyline2D<POINT_TYPE>::computeLength(void)
{
	return TPolyline2D<POINT_TYPE>::computeLength(*this);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::blend(TemplatePointVector& vecOther, FLOAT_TYPE fAmount)
{
	TPolyline2D<POINT_TYPE>::blend(*this, vecOther, fAmount);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::deform(const SBezierCurve* pSourceCurve, const SBezierCurve* pTargetCurve, TemplatePointVector& rDeformedOut) const
{
	TPolyline2D<POINT_TYPE>::deform(*this, pSourceCurve, pTargetCurve, rDeformedOut);
}
/*****************************************************************************/
template < class POINT_TYPE >
FLOAT_TYPE TPolyline2D<POINT_TYPE>::computePointDistances(TFloatVector& vecDistOut)
{
	return TPolyline2D<POINT_TYPE>::computePointDistances(*this, vecDistOut);
}
/********************************************************************************************/
template < class POINT_TYPE >
FLOAT_TYPE TPolyline2D<POINT_TYPE>::computeAngleAtPoint(int iIndex, SVector2D* pOptNormVecOut)
{
	return TPolyline2D<POINT_TYPE>::computeAngleAtPoint(*this, iIndex, pOptNormVecOut);
}
/********************************************************************************************/
template < class POINT_TYPE >
FLOAT_TYPE TPolyline2D<POINT_TYPE>::findPointOn(SVector2D& svPoint, int& iPointPosOut, FLOAT_TYPE fTolerance, SVector2D* svClosestPointOut)
{
	return TPolyline2D<POINT_TYPE>::findPointOn(*this, svPoint, iPointPosOut, fTolerance, svClosestPointOut);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::computeSpacedPointsOnPath(FLOAT_TYPE fInitOffset, FLOAT_TYPE fSpacing, TemplatePointVector& vecResOut) const
{
	TPolyline2D<POINT_TYPE>::computeSpacedPointsOnPath(*this, fInitOffset, fSpacing, vecResOut);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::computeSpacedPointsOnPath(FLOAT_TYPE fInitOffset, TFloatVector& vecSpacings, TemplatePointVector& vecResOut, TPointVector* optTangentsOut, bool bSpacingArePerPoint) const
{
	TPolyline2D<POINT_TYPE>::computeSpacedPointsOnPath(*this, false, fInitOffset, vecSpacings, NULL, 1.0, false, vecResOut, optTangentsOut, false, bSpacingArePerPoint);	
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::removeCoincident(FLOAT_TYPE fDistance)
{
	TPolyline2D<POINT_TYPE>::removeCoincident(*this, fDistance);	
}
/*****************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::removeCollinear(FLOAT_TYPE fDegAngleTolerance, TemplatePointVector& vecResOut) const
{
	TPolyline2D<POINT_TYPE>::removeCollinear(*this, fDegAngleTolerance, vecResOut);	
}
/*****************************************************************************/
#ifdef HAVE_POLYBOOL
template < class POINT_TYPE >
PLINE2* TPolyline2D<POINT_TYPE>::convertToPBLine() const
{
	return TPolyline2D<POINT_TYPE>::convertToPBLine(*this, myIsHole);
}
#endif
/*****************************************************************************/
template < class POINT_TYPE >
bool TPolyline2D<POINT_TYPE>::doesIntersect(const TemplatePointVector& rOther, SVector2D& svPointOut) const
{
	return TPolyline2D<POINT_TYPE>::doIntersect(*this, rOther, false, svPointOut);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::trimFromFront(FLOAT_TYPE fDistance)
{
	TPolyline2D<POINT_TYPE>::trimFromFront(*this, fDistance);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::trimFromEnd(FLOAT_TYPE fDistance)
{
	TPolyline2D<POINT_TYPE>::trimFromEnd(*this, fDistance);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::computeBBox(SRect2D& srOut) const
{
	TPolyline2D<POINT_TYPE>::computeBBox(*this, srOut);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::getPointByDistanceFromStart(FLOAT_TYPE fLength, SVector2D& svPointOut)
{
	TPolyline2D<POINT_TYPE>::getPointByDistanceFromStart(*this, fLength, svPointOut);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::translate(TemplatePointVector& vecPoints, FLOAT_TYPE fX, FLOAT_TYPE fY)
{
	SVector2D svDiff(fX, fY);
	int iCurr, iNum = vecPoints.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		vecPoints[iCurr].getPoint() += svDiff;
	}

}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::scale(TemplatePointVector& vecPoints, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY)
{
	int iCurr, iNum = vecPoints.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		vecPoints[iCurr].getPoint().x *= fScaleX;
		vecPoints[iCurr].getPoint().y *= fScaleY;
	}

}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::subdivide(TemplatePointVector& vecPoints, FLOAT_TYPE fMaxDistance)
{
	if(fMaxDistance <= 0)
		return;

	SVector2D svDir;
	POINT_TYPE svNewPoint, svStartPoint, svEndPoint;
	FLOAT_TYPE fSegmentLen, fMarchDist;
	int iCurr, iNum = vecPoints.size();
	int iNext, iNumInserted;
	for(iCurr = 0; iCurr < iNum - 1; iCurr++)
	{
		iNext = (iCurr + 1)%iNum;
		svDir =(vecPoints[iNext].getPoint() - vecPoints[iCurr].getPoint());
		fSegmentLen = svDir.normalize();

		if(fSegmentLen <= fMaxDistance + FLOAT_EPSILON)
			continue;

		svStartPoint = vecPoints[iCurr];
		svEndPoint = vecPoints[iNext];

		// Otherwise, march along until we subidive
		iNumInserted = 0;
		for(fMarchDist = fMaxDistance; fMarchDist < fSegmentLen - FLOAT_EPSILON; fMarchDist += fMaxDistance)
		{
			/// svNewPoint = svStartPoint + svDir*fMarchDist;
			svNewPoint.interpolateFrom(svStartPoint, svEndPoint, fMarchDist);
			
			// Now comes the funny part...
			vecPoints.insert(vecPoints.begin() + 1 + iNumInserted, svNewPoint);
			iNumInserted++;
		}
		iCurr += iNumInserted;
		iNum = vecPoints.size();
	}
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::bend(TemplatePointVector& vecPoints, FLOAT_TYPE fCenter, FLOAT_TYPE fAngle)
{
	// First, we must find the center
	FLOAT_TYPE fCurrLen, fCumulLen = 0;
	int iCurr, iNum = vecPoints.size();
	SVector2D svDir;
	int iPrev;
	FLOAT_TYPE fTotalLen = 0;

	if(iNum < 2)
		return;

	// Get the total length.
	for(iCurr = 1; iCurr < iNum; iCurr++)
	{
		iPrev = iCurr - 1;
		svDir = vecPoints[iCurr].getPoint() - vecPoints[iPrev].getPoint();
		fCurrLen = svDir.normalize();
		fTotalLen  += fCurrLen;
	}

	// See where the actual center is.
	FLOAT_TYPE fActualCenterDist = fCenter * fTotalLen;
	FLOAT_TYPE fDist;
	POINT_TYPE svNewPoint;

	int iCenterPointIndex = -1;

	for(iCurr = 1; iCurr < iNum; iCurr++)
	{
		iPrev = iCurr - 1;
		svDir = vecPoints[iCurr].getPoint() - vecPoints[iPrev].getPoint();
		fCurrLen = svDir.normalize();

		if(fActualCenterDist >= fCumulLen && fActualCenterDist <= fCumulLen + fCurrLen)
		{
			// We have it. See if we need to insert a new point.
			fDist = fActualCenterDist - fCumulLen;
			if(fDist <= FLOAT_EPSILON)
				iCenterPointIndex = iPrev;
			else if(fDist >= fCurrLen - FLOAT_EPSILON)
				iCenterPointIndex = iCurr;
			else
			{
				svNewPoint.getPoint() = vecPoints[iPrev].getPoint() + svDir*fDist;
				vecPoints.insert(vecPoints.begin() + iCurr, svNewPoint);
				iCenterPointIndex = iCurr;
			}

			break;
		}
		else
			fCumulLen += fCurrLen;
	}

	if(iCenterPointIndex < 0)
	{
		_ASSERT(0);
		return;
	}

	// Finally, bend in both directions. 
	FLOAT_TYPE fAngleDelta;

	fAngleDelta = fAngle/(FLOAT_TYPE)(iNum - iCenterPointIndex - 1);
	for(iCurr = iCenterPointIndex + 1; iCurr < iNum; iCurr++)
	{
		TPolyline2D<POINT_TYPE>::rotate(vecPoints, fAngleDelta, vecPoints[iCurr - 1].getPoint().x, vecPoints[iCurr - 1].getPoint().y, iCurr);
	}

	fAngleDelta = -fAngle/(FLOAT_TYPE)(iCenterPointIndex);
	for(iCurr = iCenterPointIndex - 1; iCurr >= 0; iCurr--)
	{
		TPolyline2D<POINT_TYPE>::rotate(vecPoints, fAngleDelta, vecPoints[iCurr + 1].getPoint().x, vecPoints[iCurr + 1].getPoint().y, 0, iCurr);
	}
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::rotate(TemplatePointVector& vecPoints, FLOAT_TYPE fAngle, FLOAT_TYPE fPivotX, FLOAT_TYPE fPivotY, int iStartIndex, int iEndIndex)
{
	// Translate to origin
	TPolyline2D<POINT_TYPE>::translate(vecPoints, -fPivotX, -fPivotY);

	if(iEndIndex < 0)
		iEndIndex = vecPoints.size() - 1;

	int iCurr;
	for(iCurr = iStartIndex; iCurr <= iEndIndex; iCurr++)
	{
		vecPoints[iCurr].getPoint().rotateCCW(fAngle);
	}

	// Translate forward
	TPolyline2D<POINT_TYPE>::translate(vecPoints, fPivotX, fPivotY);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::generateRectangle(TemplatePointVector& vecPoints, const SRect2D& srRect)
{
	vecPoints.clear();
	POINT_TYPE svTemp;

	svTemp.getPoint().set(srRect.x, srRect.y);
	vecPoints.push_back(svTemp);
	svTemp.getPoint().set(srRect.x + srRect.w, srRect.y);
	vecPoints.push_back(svTemp);
	svTemp.getPoint().set(srRect.x + srRect.w, srRect.y + srRect.h);
	vecPoints.push_back(svTemp);
	svTemp.getPoint().set(srRect.x, srRect.y + srRect.h);
	vecPoints.push_back(svTemp);
	svTemp.getPoint().set(srRect.x, srRect.y);
	vecPoints.push_back(svTemp);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::generateAtOrigin(TemplatePointVector& vecPoints, FLOAT_TYPE fLength, FLOAT_TYPE fPointSpacing)
{
	POINT_TYPE svTemp;
	vecPoints.clear();
	
	svTemp.getPoint().set(0,0);
	vecPoints.push_back(svTemp);
	svTemp.getPoint().set(fLength, 0);
	vecPoints.push_back(svTemp);

	if(fPointSpacing > 0)
		TPolyline2D<POINT_TYPE>::subdivide(vecPoints, fPointSpacing);
}
/********************************************************************************************/
template < class POINT_TYPE >
FLOAT_TYPE TPolyline2D<POINT_TYPE>::computeLength(TemplatePointVector& vecPoints)
{
	int iCurrPoint, iNumPoints = vecPoints.size();
	FLOAT_TYPE fRes = 0;
	for(iCurrPoint = 1; iCurrPoint < iNumPoints; iCurrPoint++)
	{
		fRes += (vecPoints[iCurrPoint].getPoint() - vecPoints[iCurrPoint - 1].getPoint()).length();
	}

	return fRes;
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::generateSpiralAtOrigin(TemplatePointVector& vecPoints, FLOAT_TYPE fStartRadius, FLOAT_TYPE fRollProgress, FLOAT_TYPE fLength, FLOAT_TYPE fPointSpacing)
{
	vecPoints.clear();

	// The progress blend the radius between its original value and zero.
	FLOAT_TYPE fActualRadius = fStartRadius*fRollProgress;

	// The progress also controls where we start curling.
	int iNumPoints;
	if(fPointSpacing > 0.0)
		iNumPoints = fLength/fPointSpacing + 1;
	else
		iNumPoints = 20;

	FLOAT_TYPE fActualSegmentLength = fLength/(iNumPoints - 1);

	FLOAT_TYPE fCurrRadius;
	FLOAT_TYPE fCurrProg, fPrevProg = 0;
	FLOAT_TYPE fCurrLen = fActualSegmentLength;
	FLOAT_TYPE fCurrRollProg, fCurrAngle;
	int iCurrPoint;
	POINT_TYPE svCurrPoint, svPrevPoint, svLastFlatPoint;

	//vecPoints.push_back(svCurrPoint);

	for(iCurrPoint = 0; iCurrPoint < iNumPoints; iCurrPoint++)
	{
		fCurrProg = (FLOAT_TYPE)iCurrPoint/(FLOAT_TYPE)(iNumPoints - 1);

		if(fCurrProg >= 1.0 - fRollProgress)
		{
			fCurrRollProg = (fRollProgress - (1.0 - fCurrProg))/ (fRollProgress);
			_ASSERT(fCurrRollProg >= 0 && fCurrRollProg <= 1.0);

			// We need to generate a "rolled" point
			fCurrRadius = fActualRadius*(1.0 - fCurrRollProg);
			//fCurrRadius = fActualRadius;
			fCurrAngle = fCurrRollProg*360.0 - 90;
			svCurrPoint.getPoint().x = F_COS_DEG(fCurrAngle)*fCurrRadius;
			svCurrPoint.getPoint().y = (-F_SIN_DEG(fCurrAngle)*fCurrRadius) - fActualRadius;

			svCurrPoint.getPoint() += svLastFlatPoint.getPoint();
		}
		else
		{
			// Otherwise, generate an straight point
			svCurrPoint.getPoint().set(fActualSegmentLength, 0);
			svCurrPoint.getPoint() += svPrevPoint.getPoint();

			svLastFlatPoint = svCurrPoint;
		}

		vecPoints.push_back(svCurrPoint);

		fPrevProg = fCurrProg;
		svPrevPoint = svCurrPoint;
	}

}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::blend(TemplatePointVector& vecPoints, TemplatePointVector& vecOther, FLOAT_TYPE fAmount)
{
	_ASSERT(vecPoints.size() == vecOther.size());
	if(vecPoints.size() != vecOther.size())
		return;
	
	int iNumPoints = vecPoints.size();
	if(iNumPoints == 0)
		return;

	int iCurr;
	for(iCurr = 0; iCurr < iNumPoints; iCurr++)
	{
		vecPoints[iCurr].interpolateFrom(vecPoints[iCurr], vecOther[iCurr], fAmount);
		//vecPoints[iCurr] = vecPoints[iCurr].getPoint()*(1.0 - fAmount) + vecOther[iCurr].getPoint()*fAmount;
	}
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::generateNoiseLineAtOrigin(TemplatePointVector& vecPoints, FLOAT_TYPE fLength, FLOAT_TYPE fMinPointSpacing, 
										   PerlinNoiseGenerator& rNoiseGenerator, FLOAT_TYPE fNoiseTime)
{
	// Generate recursively.
	POINT_TYPE svStart, svEnd;
	svStart.getPoint().set(0, 0);
	svEnd.getPoint().set(fLength, 0);
	vecPoints.clear();
	vecPoints.push_back(svStart);
	generateNoiseRecursive(vecPoints, fLength, svStart.getPoint(), svEnd.getPoint(), fMinPointSpacing, rNoiseGenerator, fNoiseTime);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::generateNoiseRecursive(TemplatePointVector& vecPoints, FLOAT_TYPE fLength, SVector2D& svStart, SVector2D& svEnd, 
										FLOAT_TYPE fMinPointSpacing, PerlinNoiseGenerator& rNoiseGenerator, FLOAT_TYPE fNoiseTime)
{
	// See if need to do the middle
	SVector2D svDiff;
	svDiff.x = fabs(svEnd.x - svStart.x);
//	if(svDiff.lengthSquared() > fMinPointSpacing*fMinPointSpacing)
	if(svDiff.x > fMinPointSpacing)
	{
		// We can split ourselves
		SVector2D svMid;
		FLOAT_TYPE fNoise;
		svMid = (svStart + svEnd)*0.5;

		// Noise move the point
		FLOAT_TYPE fXInterp = svMid.x/fLength;
		_ASSERT(fXInterp >= 0 && fXInterp <= 1.0);
		fNoise = rNoiseGenerator.getNoise(fXInterp, fNoiseTime); //  - rNoiseGenerator.getAmplitude()/2.0;
		svMid.y += fNoise;

		generateNoiseRecursive(vecPoints, fLength, svStart, svMid, fMinPointSpacing, rNoiseGenerator, fNoiseTime);
		generateNoiseRecursive(vecPoints, fLength, svMid, svEnd, fMinPointSpacing, rNoiseGenerator, fNoiseTime);
	}
	else
	{
		// Push the end back
		POINT_TYPE rNewPoint;
		rNewPoint.getPoint() = svEnd;
		vecPoints.push_back(rNewPoint);
	}
}
/********************************************************************************************/
template < class POINT_TYPE >
FLOAT_TYPE TPolyline2D<POINT_TYPE>::computePointDistances(TemplatePointVector& vecPoints, TFloatVector& vecDistOut)
{
	vecDistOut.clear();
	if(vecPoints.size() == 0)
		return 0;
	// The first point's distance is 0.
	vecDistOut.push_back(0);

	FLOAT_TYPE fTotalLen = 0, fCurrDist;
	int iCurr, iNum = vecPoints.size();
	for(iCurr = 1; iCurr < iNum; iCurr++)
	{
		fCurrDist = (vecPoints[iCurr].getPoint() - vecPoints[iCurr - 1].getPoint()).length();
		// If this fails, we have overlapping points and should weld them
		// before this call.
		_ASSERT(fCurrDist > FLOAT_EPSILON);
		fTotalLen += fCurrDist;
		vecDistOut.push_back(fTotalLen);
	}

	_ASSERT(vecDistOut.size() == vecPoints.size());
	return fTotalLen;
}
/********************************************************************************************/
template < class POINT_TYPE >
bool TPolyline2D<POINT_TYPE>::advanceBezierCurvePos(TemplatePointVector& vecPoints, TFloatVector& vecDistances, FLOAT_TYPE fByDistance, int& iCurvePointPosInOut, 
									FLOAT_TYPE& fDistTraveledInOut, SVector2D& svNewPosOut)
{
	int iNumPoints = vecDistances.size();
	if(fByDistance > 0 && iCurvePointPosInOut == iNumPoints - 1)
	{
		svNewPosOut = vecPoints[iNumPoints - 1].getPoint();
		fDistTraveledInOut = vecDistances[iNumPoints - 1];
		return true;
	}
	else if(fByDistance < 0 && iCurvePointPosInOut == 0)
	{
		svNewPosOut = vecPoints[0].getPoint();
		fDistTraveledInOut = 0;
		return true;
	}

	_ASSERT(iCurvePointPosInOut < iNumPoints);
	_ASSERT( (fByDistance > 0 && fDistTraveledInOut >= vecDistances[iCurvePointPosInOut])
		|| (fByDistance < 0 && fDistTraveledInOut <= vecDistances[iCurvePointPosInOut]));

	// Get the distance at the next point
	FLOAT_TYPE fFinalDist = fDistTraveledInOut + fByDistance;
	FLOAT_TYPE fCurrDist = vecDistances[iCurvePointPosInOut];
	FLOAT_TYPE fDelta;
	int iNewPoint;

	if(fByDistance > 0)
	{
		for(iNewPoint = iCurvePointPosInOut; iNewPoint < iNumPoints - 1; iNewPoint++)
		{
			fDelta = vecDistances[iNewPoint + 1] - vecDistances[iNewPoint];
			if(fCurrDist + fDelta > fFinalDist)
				break;

			fCurrDist = vecDistances[iNewPoint + 1];
		}

		if(iNewPoint >= iNumPoints - 1)
		{
			svNewPosOut = vecPoints[iNumPoints - 1].getPoint();
			fDistTraveledInOut = vecDistances[iNumPoints - 1];
			iCurvePointPosInOut = iNewPoint;
			return true;
		}

		// Otherwise, we're at the point where the next one is over the needed distance.
		// I.e. the needed point is somewhere between iNewPoint + 1 and iNewPoint.
		fDelta = vecDistances[iNewPoint + 1] - vecDistances[iNewPoint];
		FLOAT_TYPE fDeltaFinal = fFinalDist - vecDistances[iNewPoint];
		_ASSERT(fDeltaFinal < fDelta);
		_ASSERT(fDelta > FLOAT_EPSILON);
		FLOAT_TYPE fInterp = fDeltaFinal/fDelta;
		_ASSERT(fInterp >= 0.0 && fInterp <= 1.0);

		svNewPosOut = vecPoints[iNewPoint].getPoint()*(1.0 - fInterp) + vecPoints[iNewPoint + 1].getPoint()*fInterp;
		fDistTraveledInOut = fFinalDist;
		iCurvePointPosInOut = iNewPoint;

	}
	else
	{
		for(iNewPoint = iCurvePointPosInOut; iNewPoint > 0; iNewPoint--)
		{
			fDelta = vecDistances[iNewPoint - 1] - vecDistances[iNewPoint];
			if(fCurrDist + fDelta < fFinalDist)
				break;

			fCurrDist = vecDistances[iNewPoint - 1];
		}

		if(iNewPoint <= 0)
		{
			svNewPosOut = vecPoints[0].getPoint();
			fDistTraveledInOut = 0;
			return true;
		}

		// Otherwise, we're at the point where the prev one is over the needed distance.
		// I.e. the needed point is somewhere between iNewPoint - 1 and iNewPoint.
		fDelta = vecDistances[iNewPoint - 1] - vecDistances[iNewPoint];
		FLOAT_TYPE fDeltaFinal = fFinalDist - vecDistances[iNewPoint];
		_ASSERT(fDeltaFinal > fDelta);
		_ASSERT(fDelta < -FLOAT_EPSILON);
		FLOAT_TYPE fInterp = fDeltaFinal/fDelta;
		_ASSERT(fInterp >= 0.0 && fInterp <= 1.0);

		svNewPosOut = vecPoints[iNewPoint].getPoint()*(1.0 - fInterp) + vecPoints[iNewPoint - 1].getPoint()*fInterp;
		fDistTraveledInOut = fFinalDist;
		iCurvePointPosInOut = iNewPoint;

	}

	return false;
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::computeSpacedPointsOnPath(const TemplatePointVector& vecPoints, FLOAT_TYPE fInitOffset, FLOAT_TYPE fSpacing, TemplatePointVector& vecResOut)
{
	TFloatVector vecSpacingsVector;
	vecSpacingsVector.push_back(fSpacing);
	TPolyline2D<POINT_TYPE>::computeSpacedPointsOnPath(vecPoints, false, fInitOffset, vecSpacingsVector, NULL, 1.0, false, vecResOut, NULL, false, false);
}
/*****************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::computeSpacedPointsOnPath(const TemplatePointVector& vecPoints, FLOAT_TYPE fInitOffset, TFloatVector& vecSpacings, TemplatePointVector& vecResOut, TPointVector* optTangentsOut, bool bSpacingArePerPoint)
{
	TPolyline2D<POINT_TYPE>::computeSpacedPointsOnPath(vecPoints, false, fInitOffset, vecSpacings, NULL, 1.0, false, vecResOut, optTangentsOut, false, bSpacingArePerPoint);	
}
/********************************************************************************************/
#define CONV_ANGLE_MULT		0.55

template < class POINT_TYPE >
FLOAT_TYPE TPolyline2D<POINT_TYPE>::computeSpacedPointsOnPath(const TemplatePointVector& vecPoints, bool bFlipTangent, FLOAT_TYPE fInitOffset, 
										TFloatVector& vecSpacings, TFloatVector* pTangentsAsAngles, FLOAT_TYPE fTangentSpaceMult, bool bStopAtVectorEnd, 
										TemplatePointVector& vecResOut, TPointVector* optTangentsOut, bool bForceAppendLastPoint, bool bSpacingArePerPoint)
{
	_ASSERT(&vecResOut != &vecPoints);
	vecResOut.clear();
	if(optTangentsOut)
		optTangentsOut->clear();

	// Now, special case: if we only have a single point,
	// add it on
	if(vecPoints.size() == 1)
	{
		vecResOut.push_back(vecPoints[0]);
		if(optTangentsOut)
		{
			SVector2D svTempDir(1, 0);
			optTangentsOut->push_back(svTempDir);
		}
		return 0;
	}

	FLOAT_TYPE fLastSpacing = 0;
	int iSpacingIdx = 0, iNumSpacings = vecSpacings.size();
	
	// Now walk along, measuring each point.
	FLOAT_TYPE fPointDist, fNumerator, fInterp;

	SVector2D svDir;
	POINT_TYPE svPoint;
	FLOAT_TYPE fNextDist = fInitOffset;
	int iCurrPoint = 0;
	FLOAT_TYPE fCurrDist = 0;
	int iNewPoint;
	FLOAT_TYPE fDelta;
	FLOAT_TYPE fLastTangentCos = 1.0;
	int iNumPoints = vecPoints.size();
	bool bQuitNextIter = false;
	int iPrevStartPointUsed = -1;
	while(1)
	{
		// Find the initial point			
		for(iNewPoint = iCurrPoint; iNewPoint < iNumPoints - 1; iNewPoint++)
		{
			fDelta = (vecPoints[iNewPoint + 1].getPoint() - vecPoints[iNewPoint].getPoint()).length();
			if(fCurrDist + fDelta >= fNextDist)
				break;

			fCurrDist += fDelta;
		}

		// This means our offset is longer than the actual path
		// The below is a hack to add the last point even if we're shorter than the req dist
		//if(iNewPoint >= iNumPoints - 1)
		if(iNewPoint >= iNumPoints - 1 && (!bForceAppendLastPoint || !bQuitNextIter))
			break;

		iCurrPoint = iNewPoint;
		if(iCurrPoint >= iNumPoints - 1)
		{
			iCurrPoint = iNumPoints - 2;
			fInterp = 1.0;
		}
		else
		{
			fPointDist = (vecPoints[iCurrPoint + 1].getPoint() - vecPoints[iCurrPoint].getPoint()).length();
			fNumerator = fNextDist - fCurrDist;

			fInterp = fNumerator/fPointDist;
            _ASSERT(fInterp >= 0.0 && fInterp <= (1.0 + FLOAT_EPSILON));
		}

		svPoint.interpolateFrom(vecPoints[iCurrPoint], vecPoints[iCurrPoint + 1], fInterp);
		vecResOut.push_back(svPoint);

		if(optTangentsOut || fTangentSpaceMult != 1.0)
		{
			SVector2D svTan1, svTan2, svTanComm;

			int iTangPont = iCurrPoint;

			svDir = vecPoints[iTangPont + 1].getPoint() - vecPoints[iTangPont].getPoint();
			svDir.normalize();

			if(optTangentsOut)
				optTangentsOut->push_back(svDir);

			FLOAT_TYPE fFinalAngleVal = svDir.getAngleFromPositiveX();
			if(pTangentsAsAngles)
			{
				FLOAT_TYPE fExistWeight = CONV_ANGLE_MULT;
				int iCurrAngIdx = optTangentsOut->size() - 1;
				FLOAT_TYPE fTempAngle;

				if(bFlipTangent)
					fTempAngle = (svDir*-1.0).getAngleFromPositiveX();
				else
					fTempAngle = svDir.getAngleFromPositiveX();
				fFinalAngleVal = fTempAngle;

				int iNumExisting = pTangentsAsAngles->size();
					
				if((int)pTangentsAsAngles->size() <= iCurrAngIdx)
					pTangentsAsAngles->push_back(fTempAngle);
				else
				{
					// Weights here cause the carraiges to flip when reversing.
					if(fabs((*pTangentsAsAngles)[iCurrAngIdx] - fTempAngle) >= 90.0)
						fExistWeight = 0.0;

/*
					static bool bFlip = bFlipTangent;
					if(bFlip != bFlipTangent)
						fExistWeight = 0;
					bFlip = bFlipTangent;
*/

					(*pTangentsAsAngles)[iCurrAngIdx] = (*pTangentsAsAngles)[iCurrAngIdx]*fExistWeight + fTempAngle*(1.0 - fExistWeight);
					fFinalAngleVal = (*pTangentsAsAngles)[iCurrAngIdx];
				}
			}

			if(fTangentSpaceMult != 1.0)
			{
				fLastTangentCos = fabs(F_COS_DEG(fFinalAngleVal));
				_ASSERT(fLastTangentCos >= 0 && fLastTangentCos <= 1.0);
			}
		}

		if(bQuitNextIter)
			break;
		
		// Now, update our req vars
		if(fTangentSpaceMult != 1.0)
			fLastSpacing = vecSpacings[iSpacingIdx]*fTangentSpaceMult*(1.0 - fLastTangentCos) + vecSpacings[iSpacingIdx]*fLastTangentCos;
		else
			fLastSpacing = vecSpacings[iSpacingIdx];

		fNextDist += fLastSpacing;

		if(!bSpacingArePerPoint || (bSpacingArePerPoint && iPrevStartPointUsed != iCurrPoint ) )
			iSpacingIdx = (iSpacingIdx + 1);
		if(iSpacingIdx >= iNumSpacings && bStopAtVectorEnd)
			bQuitNextIter = true;
		iSpacingIdx %= iNumSpacings;	

		iPrevStartPointUsed = iCurrPoint;
	}
	// fl/2.0 for the ending overhang.
	return fNextDist + fLastSpacing/2.0;
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::interpolatePointAttribute(TFloatVector &vecDistances, int iCurvePointPos, FLOAT_TYPE fCurveDistPos, 
							FLOAT_TYPE fMotionDir, TemplatePointVector& vecAttrsToInterp, SVector2D& svResOut)
{
	int iNumDistances = vecDistances.size();
	int iOtherPontIdx;
	if(fMotionDir > 0)
	{
		iOtherPontIdx = iCurvePointPos + 1;
		if(iOtherPontIdx >= iNumDistances)
		{
			iCurvePointPos = iNumDistances - 2;
			iOtherPontIdx = iNumDistances - 1;
		}
	}
	else
	{
		iOtherPontIdx = iCurvePointPos - 1;
		if(iOtherPontIdx < 0)
		{
			iCurvePointPos = 1;
			iOtherPontIdx = 0;
		}
	}

	if(iOtherPontIdx == iCurvePointPos)
	{
		if(iOtherPontIdx == 0)
			iOtherPontIdx = 1;
		else if(iOtherPontIdx == vecAttrsToInterp.size() - 1)
			iOtherPontIdx = vecAttrsToInterp.size() - 2;
	}

	FLOAT_TYPE fInterp;
	fInterp = (fCurveDistPos - vecDistances[iCurvePointPos])/(vecDistances[iOtherPontIdx] - vecDistances[iCurvePointPos]);

	svResOut = vecAttrsToInterp[iCurvePointPos].getPoint()*(1.0 - fInterp) + vecAttrsToInterp[iOtherPontIdx].getPoint()*fInterp;
}
/********************************************************************************************/
template < class POINT_TYPE >
FLOAT_TYPE TPolyline2D<POINT_TYPE>::findPointOn(TemplatePointVector& vecPoints, SVector2D& svPoint, int& iPointPosOut, FLOAT_TYPE fTolerance, SVector2D* svClosestPointOut)
{
	if(vecPoints.size() < 2)
		return -1;

	SVector2D svDir, svDiff, svNorm;
	FLOAT_TYPE fTotalDist = 0;
	FLOAT_TYPE fCurrDist;
	FLOAT_TYPE fDot;
	int iCurr, iNum = vecPoints.size();
	for(iCurr = 1; iCurr < iNum; iCurr++)
	{
		svDir = vecPoints[iCurr].getPoint() - vecPoints[iCurr - 1].getPoint();
		fCurrDist = svDir.normalize();

		svDiff = svPoint - vecPoints[iCurr - 1].getPoint();
		svNorm.set(svDir.y, -svDir.x);
		if(fabs(svDiff.dot(svNorm)) < fTolerance)
		{
			// It's on the curve in terms of perp dist, check the segment.
			fDot = svDiff.dot(svDir);
			if(fDot >= 0 && fDot <= fCurrDist)
			{
				// It's on us!
				// fDot is the distance from the last point
				iPointPosOut = iCurr - 1;
				if(svClosestPointOut)
					*svClosestPointOut = vecPoints[iCurr - 1].getPoint() + svDir*fDot;
				return fTotalDist + fDot;
			}
		}

		fTotalDist += fCurrDist;
	}

	return -1;
}
/********************************************************************************************/
template < class POINT_TYPE >
bool TPolyline2D<POINT_TYPE>::doIntersect(const TemplatePointVector& rFirst, const TemplatePointVector& rSecond, bool bExcludeExactMatches, SVector2D& svPointOut)
{
	const FLOAT_TYPE fGeomTolerance = upToScreen(1.0);

	FLOAT_TYPE fRes;
	int iCurr, iNum = rFirst.size();
	int iCurr2, iNum2 = rSecond.size();

	for(iCurr = 0; iCurr < iNum - 1; iCurr++)
	{
		for(iCurr2 = 0; iCurr2 < iNum2 - 1; iCurr2++)
		{
			if(bExcludeExactMatches)
			{
				if(rFirst[iCurr].getPoint() == rSecond[iCurr2].getPoint() 
					|| rFirst[iCurr].getPoint() == rSecond[iCurr2 + 1].getPoint()
					|| rFirst[iCurr + 1].getPoint() == rSecond[iCurr2].getPoint() 
					|| rFirst[iCurr + 1].getPoint() == rSecond[iCurr2 + 1].getPoint())
					continue;
			}

			// Test for intersection
			fRes = GeometryUtils::segmentSegmentIntersection(rFirst[iCurr].getPoint(), rFirst[iCurr + 1].getPoint(), 
				rSecond[iCurr2].getPoint(), rSecond[iCurr2 + 1].getPoint(), &svPointOut, fGeomTolerance);
			if(fRes != FLOAT_TYPE_MAX)
			{
				// They intersect - add a point to each one						
				return true;
			}

			// Now we need to test for overlap
			if(GeometryUtils::doSegmentsOverlap(rFirst[iCurr].getPoint(), rFirst[iCurr + 1].getPoint(), 
				rSecond[iCurr2].getPoint(), rSecond[iCurr2 + 1].getPoint(), fGeomTolerance))
			{
				// Cheat - middle between start points.
				svPointOut = ((rFirst[iCurr].getPoint() + rFirst[iCurr + 1].getPoint())*0.5 + (rSecond[iCurr2].getPoint() + rSecond[iCurr2 + 1].getPoint())*0.5)*0.5;
				return true;
			}
		}
	}

	return false;
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::trimFromEnd(TemplatePointVector& vecPoints, FLOAT_TYPE fDistance)
{
	// Cheap way out...
	if(vecPoints.size() > 0)
		std::reverse(vecPoints.begin(), vecPoints.end());
	TPolyline2D<POINT_TYPE>::trimFromFront(vecPoints, fDistance);
	if(vecPoints.size() > 0)
		std::reverse(vecPoints.begin(), vecPoints.end());
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::trimFromFront(TemplatePointVector& vecPoints, FLOAT_TYPE fDistance)
{

	int iCurr, iNum = vecPoints.size();

	// We just need to find one point
	FLOAT_TYPE fCumulDist = 0, fCurrDist;
	SVector2D svDir;
	for(iCurr = 0; iCurr < iNum - 1; iCurr++ )
	{
		svDir = vecPoints[iCurr + 1].getPoint() - vecPoints[iCurr].getPoint();
		fCurrDist = svDir.normalize();

		if(fCumulDist + fCurrDist > fDistance)
			break;

		fCumulDist += fCurrDist;
	}

	// The distance is larger than the actual vector
	if(iCurr >= iNum)
	{
		_ASSERT(fCumulDist >= fDistance);
		return;
	}

	// Figure out the interp point.
	POINT_TYPE svNewPoint;
	svNewPoint.getPoint() = vecPoints[iCurr].getPoint() + svDir*(fDistance - fCumulDist);

	// Erase the front N points
	if(iCurr < iNum + 1)
	{
		vecPoints.erase(vecPoints.begin() + iCurr + 1, vecPoints.end());
		vecPoints.push_back(svNewPoint);
	}
	else
	{
		_ASSERT(iCurr == iNum - 1);
		vecPoints[iCurr] = svNewPoint;
	}

#ifdef _DEBUG
	FLOAT_TYPE dbTemp = TPolyline2D<POINT_TYPE>::computeLength(vecPoints);
	int bp = 0;
	_ASSERT(fabs(dbTemp - fDistance) < 3.0);
#endif

}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::getPointByDistanceFromStart(TemplatePointVector& vecPoints, FLOAT_TYPE fLength, SVector2D& svPointOut)
{
	FLOAT_TYPE fDistPos = 0;
	int iCurvePointPos = 0;

	TFloatVector vecDistancesVector;
	computePointDistances(vecPoints, vecDistancesVector);
	advanceBezierCurvePos(vecPoints, vecDistancesVector, fLength, iCurvePointPos, fDistPos, svPointOut);
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::computeBBox(const TemplatePointVector& vecPoints, SRect2D& srOut)
{	
	srOut.x = srOut.y = 0;
	srOut.w = srOut.h = 0;
	int iCurrPoint, iNumPoints = vecPoints.size();
	if(iNumPoints == 0)
		return;

	srOut.x = vecPoints[0].getPoint().x;
	srOut.y = vecPoints[0].getPoint().y;

	for(iCurrPoint = 1; iCurrPoint < iNumPoints; iCurrPoint++)
		srOut.includePoint(vecPoints[iCurrPoint].getPoint());
}
/********************************************************************************************/
template < class POINT_TYPE >
FLOAT_TYPE TPolyline2D<POINT_TYPE>::computeSpacedPointsOnMultiplePaths(vector< TPolyline2D<POINT_TYPE> >& vecPaths, bool bFlipTangent, FLOAT_TYPE fInitOffset, TFloatVector& vecSpacings, TFloatVector* pTangentsAsAngles, FLOAT_TYPE fTangentSpaceMult, bool bStopAtVectorEnd, TemplatePointVector& vecResOut, TPointVector* optTangentsOut, bool bForceAppendLastPoint)
{
	vecResOut.clear();
	if(optTangentsOut)
		optTangentsOut->clear();

	FLOAT_TYPE fLastSpacing = 0;
	int iSpacingIdx = 0, iNumSpacings = vecSpacings.size();
	
	// Now walk along, measuring each point.
	FLOAT_TYPE fPointDist, fNumerator, fInterp;

	SVector2D svDir;
	POINT_TYPE svPoint;
	FLOAT_TYPE fNextDist = fInitOffset;
	int iCurrPoint = 0;
	FLOAT_TYPE fCurrDist = 0;
	int iNewPoint;
	FLOAT_TYPE fDelta;
	FLOAT_TYPE fLastTangentCos = 1.0;
	int iNumPoints;
	bool bQuitNextIter = false;

	TemplatePointVector* pPoints;
	int iCurrPath, iNumPaths = vecPaths.size();
	for(iCurrPath = 0; iCurrPath < iNumPaths; iCurrPath++)
	{
		pPoints = &vecPaths[iCurrPath];
		iCurrPoint = 0;
		iNumPoints = pPoints->size();
	while(1)
	{
		// Find the initial point			
		for(iNewPoint = iCurrPoint; iNewPoint < iNumPoints - 1; iNewPoint++)
		{
			fDelta = ((*pPoints)[iNewPoint + 1].getPoint() - (*pPoints)[iNewPoint].getPoint()).length();
			if(fCurrDist + fDelta >= fNextDist)
				break;

			fCurrDist += fDelta;
		}

		// This means our offset is longer than the actual path
		// The below is a hack to add the last point even if we're shorter than the req dist
		//if(iNewPoint >= iNumPoints - 1)
		if(iNewPoint >= iNumPoints - 1 &&  (!bForceAppendLastPoint || !bQuitNextIter || iCurrPath + 1 < iNumPaths))
			break;

		iCurrPoint = iNewPoint;
		if(iCurrPoint >= iNumPoints - 1)
		{
			iCurrPoint = iNumPoints - 2;
			fInterp = 1.0;
		}
		else
		{
			fPointDist = ((*pPoints)[iCurrPoint + 1].getPoint() - (*pPoints)[iCurrPoint].getPoint()).length();
			fNumerator = fNextDist - fCurrDist;

			fInterp = fNumerator/fPointDist;
			_ASSERT(fInterp >= 0.0 && fInterp <= 1.0);
		}

		//svPoint = (*pPoints)[iCurrPoint].getPoint()*(1.0 - fInterp) + (*pPoints)[iCurrPoint + 1].getPoint()*fInterp;
		svPoint.interpolateFrom((*pPoints)[iCurrPoint], (*pPoints)[iCurrPoint + 1], fInterp);
		vecResOut.push_back(svPoint);

		if(optTangentsOut || fTangentSpaceMult != 1.0)
		{
			SVector2D svTan1, svTan2, svTanComm;
//			int iTempPoint;

			int iTangPont = iCurrPoint;

			svDir = (*pPoints)[iTangPont + 1].getPoint() - (*pPoints)[iTangPont].getPoint();
			svDir.normalize();

			if(optTangentsOut)
				optTangentsOut->push_back(svDir);

			FLOAT_TYPE fFinalAngleVal = svDir.getAngleFromPositiveX();
			if(pTangentsAsAngles)
			{
				FLOAT_TYPE fExistWeight = CONV_ANGLE_MULT;
				int iCurrAngIdx = optTangentsOut->size() - 1;
				FLOAT_TYPE fTempAngle;

				if(bFlipTangent)
					fTempAngle = (svDir*-1.0).getAngleFromPositiveX();
				else
					fTempAngle = svDir.getAngleFromPositiveX();
				fFinalAngleVal = fTempAngle;

				int iNumExisting = pTangentsAsAngles->size();
					
				if((int)pTangentsAsAngles->size() <= iCurrAngIdx)
					pTangentsAsAngles->push_back(fTempAngle);
				else
				{
					// Weights here cause the carraiges to flip when reversing.
					if(fabs((*pTangentsAsAngles)[iCurrAngIdx] - fTempAngle) >= 90.0)
						fExistWeight = 0.0;

					static bool bFlip = bFlipTangent;
					if(bFlip != bFlipTangent)
						fExistWeight = 0;
					bFlip = bFlipTangent;

					(*pTangentsAsAngles)[iCurrAngIdx] = (*pTangentsAsAngles)[iCurrAngIdx]*fExistWeight + fTempAngle*(1.0 - fExistWeight);
					fFinalAngleVal = (*pTangentsAsAngles)[iCurrAngIdx];
				}
			}

			if(fTangentSpaceMult != 1.0)
			{
				fLastTangentCos = fabs(F_COS_DEG(fFinalAngleVal));
				_ASSERT(fLastTangentCos >= 0 && fLastTangentCos <= 1.0);
			}
		}

		if(bQuitNextIter)
			break;
		
		// Now, update our req vars
		if(fTangentSpaceMult != 1.0)
			fLastSpacing = vecSpacings[iSpacingIdx]*fTangentSpaceMult*(1.0 - fLastTangentCos) + vecSpacings[iSpacingIdx]*fLastTangentCos;
		else
			fLastSpacing = vecSpacings[iSpacingIdx];

		fNextDist += fLastSpacing;

		iSpacingIdx = (iSpacingIdx + 1);
		if(iSpacingIdx >= iNumSpacings && bStopAtVectorEnd)
			bQuitNextIter = true;
		iSpacingIdx %= iNumSpacings;	
	} // end while(1)
	if(bQuitNextIter)
		break;
	} // end over all paths

	/*
#ifdef _DEBUG
	
	{
		char pcsBuff[512];
		int iDbTmp = bFlipTangent;
		sprintf(pcsBuff, " FLIP = %d  LEN = %f OFF = %f\n", iDbTmp, fNextDist - fInitOffset, fInitOffset);
		OutputDebugString(pcsBuff);
	}

#endif
	*/
	// fl/2.0 for the ending overhang.
	return fNextDist + fLastSpacing/2.0;
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::transform(TemplatePointVector& vecPoints, const CombinedTransform& smCombTransform)
{
	SVector2D svTemp;
	int iPoint, iNumPoints = vecPoints.size();
	for(iPoint = 0; iPoint < iNumPoints; iPoint++)
	{
		svTemp = vecPoints[iPoint].getPoint();
		vecPoints[iPoint].getPoint() = smCombTransform*svTemp;
	}
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::transform(TemplatePointVector& vecPoints, const SMatrix2D& smMatrix)
{
	SVector2D svTemp;
	int iPoint, iNumPoints = vecPoints.size();
	for(iPoint = 0; iPoint < iNumPoints; iPoint++)
	{
		svTemp = vecPoints[iPoint].getPoint();
		vecPoints[iPoint].getPoint() = smMatrix*svTemp;
	}
}
/*****************************************************************************/
template < class POINT_TYPE >
HyperCore::OrientationType TPolyline2D<POINT_TYPE>::getPolygonOrientation(const TemplatePointVector& vecPoints)
{
	int i,j;
	FLOAT_TYPE dTotalArea=0.0;

	int iNumPoints = vecPoints.size();
	for (i = 0; i < iNumPoints; i++) 
	{
		j = (i+1)%iNumPoints;
		dTotalArea += vecPoints[i].getPoint().x * vecPoints[j].getPoint().y;
		dTotalArea -= vecPoints[i].getPoint().y * vecPoints[j].getPoint().x;
	}
	if(dTotalArea>=0)
		return OrientationCW;
	else
		return OrientationCCW;
}
/*****************************************************************************/

// Anti-infinete loop protectoin multiplier. If the number of iterations
// exceeds this multiplied by the number of vertices, the loop exits, regardless
// of how many triangles were created.
#define GP_AILP_MULTIPLIER	4.0

struct GP_FastTestPointType
{
	SVector2D m_vecPoint;
	int m_iIndex;

	void operator =(const GP_FastTestPointType& pSrc)
	{
		m_iIndex = pSrc.m_iIndex;
		m_vecPoint = pSrc.m_vecPoint;
	}
};


GP_FastTestPointType *g_nvaVertexCacheArray = NULL;
int g_iVertexCacheArraySize = 0;

unsigned char *m_sVertexHelperArray = NULL;
int m_iVertexHelperArraySize = 0;

int GP_SVector2DSortByX( const void *arg1, const void *arg2 )
{
	GP_FastTestPointType *v1, *v2;
	v1 = (GP_FastTestPointType *)arg1;
	v2 = (GP_FastTestPointType *)arg2;

	if(v1->m_vecPoint.x < v2->m_vecPoint.x)
		return -1;
	else if(v1->m_vecPoint.x > v2->m_vecPoint.x)
		return 1;
	else
		return 0;
}

bool IsEpsilonEqual(SVector2D* v1, SVector2D* v2)
{
	double dx = v1->x - v2->x;
	double dy = v1->y - v2->y;
	if(dx <= FLOAT_EPSILON && dx>=-FLOAT_EPSILON &&
		dy <= FLOAT_EPSILON && dy>=-FLOAT_EPSILON)
		return true;
	else
		return false;
}


bool FastIsVertexInside(GP_FastTestPointType *pSortedArray, int iNumArrayPoints, SVector2D& pTriVert1,SVector2D& pTriVert2,SVector2D& pTriVert3)
{
	// Find the minimum x-value
	GP_FastTestPointType minXVector, maxXVector;
	GP_FastTestPointType *pFoundMinVec, *pFoundMaxVec, *pPrev;
	minXVector.m_vecPoint.set(min(min(pTriVert1.x, pTriVert2.x),pTriVert3.x),0);
	maxXVector.m_vecPoint.set(max(max(pTriVert1.x, pTriVert2.x),pTriVert3.x),0);
	pFoundMinVec = (GP_FastTestPointType *)bsearch(&minXVector, pSortedArray, iNumArrayPoints, sizeof(GP_FastTestPointType), GP_SVector2DSortByX);
	pFoundMaxVec = (GP_FastTestPointType *)bsearch(&maxXVector, pSortedArray, iNumArrayPoints, sizeof(GP_FastTestPointType), GP_SVector2DSortByX);

	// Scan backwards to see if there's more with the same
	pPrev = pFoundMinVec;
	while(pFoundMinVec != (&pSortedArray[0]) && pFoundMinVec->m_vecPoint.x == minXVector.m_vecPoint.x)
	{
		pPrev = pFoundMinVec;
		pFoundMinVec--;
	}
	pFoundMinVec = pPrev;

	pPrev = pFoundMaxVec;
	while(pFoundMaxVec != (&pSortedArray[iNumArrayPoints-1]) && pFoundMaxVec->m_vecPoint.x == maxXVector.m_vecPoint.x)
	{
		pPrev = pFoundMaxVec;
		pFoundMaxVec++;
	}
	pFoundMaxVec = pPrev;

	// Now test between the two
	GP_FastTestPointType *pCurr;
	pCurr = pFoundMinVec;
	do
	{
		if(m_sVertexHelperArray[pCurr->m_iIndex] == 0 &&
			!IsEpsilonEqual(&pCurr->m_vecPoint, &pTriVert1) &&
			!IsEpsilonEqual(&pCurr->m_vecPoint, &pTriVert2) &&
			!IsEpsilonEqual(&pCurr->m_vecPoint, &pTriVert3))
		{
			if(GeometryUtils::isPointInTriangle(pTriVert1, pTriVert2, pTriVert3, pCurr->m_vecPoint))
				return true;
		}
		pCurr++;
	} while(pCurr!=pFoundMaxVec);

	return false;
}

template < class POINT_TYPE >
SVertexInfo* TPolyline2D<POINT_TYPE>::triangulateAsPolygon(const TemplatePointVector& vecPoints, int &iNumVertsOut)
{
	_ASSERT(0);
	// Note that this method produces the wrong triangultaion sometimes, where triangles overlap and are outside
	// of the actual poly! Do not use!

	int numVerts = (int)vecPoints.size();
	if(numVerts <= 2)
		return NULL;

	int iMaxNumTriangles = numVerts - 2;
	SVertexInfo* pRes = new SVertexInfo[iMaxNumTriangles*3];
	int iOutArrayCounter = 0;

	FLOAT_TYPE dCrossProductMultiplier;

	HyperCore::OrientationType eOrientation = getPolygonOrientation(vecPoints);

	if(eOrientation == OrientationCW)
		dCrossProductMultiplier = 1.0;
	else // if(eOrientation == OrientationCW)
		dCrossProductMultiplier = -1.0;

	int iCurrentVertex = 0;
	int iNumCutoffTriangles = 0;
	POINT_TYPE triPoints[3];
	SVector2D vec1,vec2;
	int vIndices[3];
	SVector2D svDummy;
	
	TemplatePointVector rThreeSides[3];
	rThreeSides[0].resize(2);
	rThreeSides[1].resize(2);
	rThreeSides[2].resize(2);

	// anti-infinite loop protection
	int iAILPCntr = 0, iAILPCutoff;
	iAILPCutoff = numVerts*GP_AILP_MULTIPLIER;

	if(numVerts>m_iVertexHelperArraySize)
	{
		if(m_sVertexHelperArray)
			delete[] m_sVertexHelperArray;
		m_iVertexHelperArraySize = numVerts;
		m_sVertexHelperArray = new unsigned char[numVerts];
	}

	memset(m_sVertexHelperArray, 0, sizeof(unsigned char)*m_iVertexHelperArraySize);

	if(numVerts > g_iVertexCacheArraySize)
	{
		if(g_nvaVertexCacheArray)
			delete[] g_nvaVertexCacheArray;
		g_iVertexCacheArraySize = numVerts;
		g_nvaVertexCacheArray = new GP_FastTestPointType[numVerts];				
	}
	// Copy all vertices into an array, sort it by x coordinate, only check those points later.
	for(iCurrentVertex = 0; iCurrentVertex<numVerts;iCurrentVertex++)
	{
		g_nvaVertexCacheArray[iCurrentVertex].m_vecPoint = vecPoints[iCurrentVertex].getPoint();
		g_nvaVertexCacheArray[iCurrentVertex].m_iIndex = iCurrentVertex;
	}
	qsort(g_nvaVertexCacheArray, numVerts, sizeof(GP_FastTestPointType), GP_SVector2DSortByX);

	iCurrentVertex = 0;
	while(iNumCutoffTriangles < iMaxNumTriangles)
	{
		// Get the three vertices, check that the angle between them is less than 180.
		while(m_sVertexHelperArray[iCurrentVertex]) { iCurrentVertex++; if(iCurrentVertex>=numVerts) iCurrentVertex-=numVerts; }
		vIndices[0] = iCurrentVertex;
		iCurrentVertex++;
		if(iCurrentVertex>=numVerts) iCurrentVertex-=numVerts;

		while(m_sVertexHelperArray[iCurrentVertex]) { iCurrentVertex++; if(iCurrentVertex>=numVerts) iCurrentVertex-=numVerts; }
		vIndices[1] = iCurrentVertex;
		iCurrentVertex++;
		if(iCurrentVertex>=numVerts) iCurrentVertex-=numVerts;


		while(m_sVertexHelperArray[iCurrentVertex]) { iCurrentVertex++; if(iCurrentVertex>=numVerts) iCurrentVertex-=numVerts; }
		vIndices[2] = iCurrentVertex;
		iCurrentVertex++;
		if(iCurrentVertex>=numVerts) iCurrentVertex-=numVerts;

		// Now, check the angle between them.
		vec1 = vecPoints[vIndices[1]].getPoint() - vecPoints[vIndices[0]].getPoint();
		vec2 = vecPoints[vIndices[2]].getPoint() - vecPoints[vIndices[1]].getPoint();
		vec1.normalize();
		vec2.normalize();

		iAILPCntr++;
		if(iAILPCntr > iAILPCutoff)
			break;

		if(vec1.cross(vec2)*dCrossProductMultiplier < 0.0 || iNumCutoffTriangles+1 == numVerts-2)
		{
			// an ear, cut it off			
			triPoints[0] = vecPoints[vIndices[0]];
			triPoints[1] = vecPoints[vIndices[1]];
			triPoints[2] = vecPoints[vIndices[2]];

			rThreeSides[0][0] = triPoints[0];
			rThreeSides[0][1] = triPoints[1];

			rThreeSides[1][0] = triPoints[1];
			rThreeSides[1][1] = triPoints[2];

			rThreeSides[2][0] = triPoints[2];
			rThreeSides[2][1] = triPoints[0];

			if(!FastIsVertexInside(g_nvaVertexCacheArray,numVerts,triPoints[0].getPoint(),triPoints[1].getPoint(),triPoints[2].getPoint())
/*
				&& !doIntersect(vecPoints, rThreeSides[0], true, svDummy)
				&& !doIntersect(vecPoints, rThreeSides[1], true, svDummy)
				&& !doIntersect(vecPoints, rThreeSides[2], true, svDummy)
*/
				)
			{
				// an ear, cut it off
				////pCallbackClass->TriangulateConcavePolygonCallback(&triPoints[0],&triPoints[1],&triPoints[2], NULL);

				pRes[iOutArrayCounter].copyFrom(triPoints[0].getPoint());
				pRes[iOutArrayCounter + 1].copyFrom(triPoints[1].getPoint());
				pRes[iOutArrayCounter + 2].copyFrom(triPoints[2].getPoint());
				iOutArrayCounter += 3;

				m_sVertexHelperArray[vIndices[1]] = 1;
				iNumCutoffTriangles++;
				iCurrentVertex = vIndices[2];
				iAILPCntr = 0;
			}
			else
			{
				iCurrentVertex = vIndices[0]+1;
				if(iCurrentVertex>=numVerts) iCurrentVertex-=numVerts; 
			}
		}
		else
		{
			// concavity
			iCurrentVertex = vIndices[0]+1;
			if(iCurrentVertex >= numVerts) 
				iCurrentVertex -= numVerts; 
		} // end if cross product
	} // end while triangulating

	iNumVertsOut = iNumCutoffTriangles*3;
	return pRes;
}
/*****************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::removeCoincident(TemplatePointVector& vecPoints, FLOAT_TYPE fDistance)
{
	if(fDistance <= 0)
		return;

	int iNext, iCurr, iNum = vecPoints.size();
	if(iNum <= 2)
		return;

	for(iCurr = iNum - 2; iCurr >= 0; iCurr--)
	{
		iNext = iCurr + 1;
		if((vecPoints[iCurr].getPoint() - vecPoints[iNext].getPoint()).lengthSquared() <= fDistance*fDistance)
		{
			if(iCurr > 0)
				vecPoints.erase(vecPoints.begin() + iCurr);
			else if(iNext < vecPoints.size())
				vecPoints.erase(vecPoints.begin() + iNext);
		}
	}
/*
	iNum = vecPoints.size();
	if(iNum > 2)
	{
		if((vecPoints[iNum - 1] - vecPoints[0]).lengthSquared() <= fDistance*fDistance)
			bIsClosed = true;
	}*/

}
/*****************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::removeCollinear(const TemplatePointVector& vecPoints, FLOAT_TYPE fDegAngleTolerance, TemplatePointVector& vecResOut)
{
	SVector2D svTemp1, svTemp2;
	if(fDegAngleTolerance <= 0)
		return;

	int iNext, iNext2, iCurr;
	int iStop = 1;
	int iNum = vecPoints.size();
	vecResOut.clear();
	vecResOut.push_back(vecPoints[iNum - 1]);

	bool bAllowPoint;
	FLOAT_TYPE dTolerance = cos((fDegAngleTolerance)/180.0*M_PI);
	for(iCurr = iNum - 1; iCurr >= iStop; iCurr--)
	{
		iNext = iCurr - 1;
		if(iNext < 0)
			iNext += vecPoints.size();
		iNext2 = iCurr - 2;
		if(iNext2 < 0)
			iNext2 = vecPoints.size() - 1;

		bAllowPoint = true;
		if(vecPoints[iCurr].getPointType() == BezierPointCurveSharp 
			&& vecPoints[iNext].getPointType() == BezierPointCurveSharp 
			&& vecPoints[iNext2].getPointType() == BezierPointCurveSharp)
		{
			svTemp1 = vecPoints[iCurr].getPoint() - vecPoints[iNext].getPoint();
			svTemp1.normalize();
			svTemp2 = vecPoints[iCurr].getPoint() - vecPoints[iNext2].getPoint();
			svTemp2.normalize();

			if(fabs(svTemp1.dot(svTemp2)) >= dTolerance)
				bAllowPoint = false;
		}

		if(bAllowPoint)
			vecResOut.push_back(vecPoints[iNext]);
	}

	vecResOut.push_back(vecPoints[0]);
	
	// Now we need to reverse the vector to preserve order!
	std::reverse(vecResOut.begin(), vecResOut.end());
}
/*****************************************************************************/
template < class POINT_TYPE >
FLOAT_TYPE TPolyline2D<POINT_TYPE>::computeAngleAtPoint(TemplatePointVector& vecPoints, int iIndex, SVector2D* pOptNormVecOut)
{
	int iPrev, iNext, iNum = vecPoints.size();
	if(iNum < 2)
		return 0;

	iPrev = iIndex - 1;
	iNext = iIndex + 1;

	SVector2D svSrcVector;
	if(iIndex == 0)
		svSrcVector = vecPoints[1].getPoint() - vecPoints[0].getPoint();
	else if(iIndex == iNum - 1)
		svSrcVector = vecPoints[iIndex].getPoint() - vecPoints[iIndex - 1].getPoint();
	else
		svSrcVector =  vecPoints[iNext].getPoint() - vecPoints[iPrev].getPoint();
	
	svSrcVector.normalize();
	if(pOptNormVecOut)
		*pOptNormVecOut = svSrcVector;

	return svSrcVector.getAngleFromPositiveX();
}
/*****************************************************************************/
template < class POINT_TYPE >
bool TPolyline2D<POINT_TYPE>::doesContain(const TemplatePointVector& rFirst, const TemplatePointVector& rSecond, bool bAllowOverlap)
{
	// Conditions for containment: 
	// 1) Every point of rSecond has to be inside rFirst
	// 2) No segment of rSecond intersects rFirst

	int iCurr, iNum = rSecond.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(!doesContain(rFirst, rSecond[iCurr].getPoint(), bAllowOverlap))
			return false;
	}

	SVector2D svPoint;
	if(doIntersect(rFirst, rSecond, true, svPoint))
		return false;

	return true;
}
/*****************************************************************************/
template < class POINT_TYPE >
int countRayPolygonIntersections(const SVector2D& pRayStart, const SVector2D& pRayDir, const TemplatePointVector &rPoints)
{
	// Go through every side, check intersections.
	SVector2D vecIntersectPoint, pRayNorm;
	int numIntersections = 0;
	FLOAT_TYPE dIntersect, dSecondLineIntersect, dSecondLineLength;
	FLOAT_TYPE dSign1, dSign2;
	int i, next_v, numVerts, additional_v;
	numVerts = rPoints.size();

	pRayNorm.set(-pRayDir.y, pRayDir.x);

	SRay2D svRay(pRayStart, pRayDir);
	for(i=0;i<numVerts;i++)
	{
		next_v = (i+1)%numVerts;

		dIntersect = svRay.doesIntersectLineSegment(rPoints[i].getPoint(), rPoints[next_v].getPoint(), &dSecondLineIntersect, &dSecondLineLength, FLOAT_EPSILON);
		if(dIntersect>=0.0 && dIntersect!=FLOAT_TYPE_MAX)
		{
			// An intersection of some sort. Need to check if it doesn't happen exactly at
			// the vertex, and if it does, then decide what to do.
			if(dSecondLineIntersect<=FLOAT_EPSILON)
			{
				// Either this or the next if clause should be commented out so that the
				// case when the ray intersects a vertex exactly (and that vertex is in the middle
				// of a poly, so it's not just touching it) isn't counted twice.

			}
			else if(dSecondLineLength-dSecondLineIntersect<=FLOAT_EPSILON)
			{
				// close to vertex next_v
				additional_v = (next_v+1)%numVerts;

				// check i and next_v+1
				dSign1 = (rPoints[i].getPoint() - pRayStart).dot(pRayNorm);
				dSign2 = (rPoints[additional_v].getPoint() - pRayStart).dot(pRayNorm);

				if(dSign1*dSign2 <= 0)
					numIntersections++;
			}
			else
			{
				// just "normal" intersection
				numIntersections++;
			}
		}
	}
	return numIntersections;

}

template < class POINT_TYPE >
bool TPolyline2D<POINT_TYPE>::doesContain(const TemplatePointVector& rShape, const SVector2D& svPoint, bool bAllowOverlap)
{
	if(bAllowOverlap)
	{
		// Check if the vertices match:
		int iCurr, iNum = rShape.size();
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			if((rShape[iCurr].getPoint() - svPoint).lengthSquared() < FLOAT_EPSILON)
				return true;
		}
	}

	// For a point to be contained in the shape, it has to have an odd number of intersections
	SVector2D svDir(-1, 0);
	int iCount = countRayPolygonIntersections(svPoint, svDir, rShape);
	return (iCount % 2) != 0;
}
/*****************************************************************************/
#ifdef HAVE_POLYBOOL
template < class POINT_TYPE >
PLINE2* TPolyline2D<POINT_TYPE>::convertToPBLine(const TemplatePointVector& vecPoints, bool bIsHole)
{
	GRID2 rPoint;
	PLINE2 *pLine = NULL; 
	int iCurr, iNum = vecPoints.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		const SVector2D& svPoint = vecPoints[iCurr].getPoint();
		rPoint.x = svPoint.x;
		rPoint.y = svPoint.y;
		PLINE2::Incl(&pLine, rPoint);
	}

	// Set the hole flag
	if(pLine)
	{
		pLine->Prepare();
		bool bIsOuter = pLine->IsOuter();
		if((bIsHole && bIsOuter) || (!bIsHole && !bIsOuter))
			pLine->Invert();
	}

	return pLine;
}
/*****************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::setFromPBLine(const PLINE2* pLine)
{
	this->clear();
	if(!pLine)
		return;
	
	POINT_TYPE svTemp;
	const VNODE2* pSrcPoint;
	for(pSrcPoint = pLine->head; pSrcPoint; pSrcPoint = pSrcPoint->next)
	{		
		svTemp.getPoint().set(pSrcPoint->p.x, pSrcPoint->p.y);
		this->push_back(svTemp);
	}
}
#endif
/*****************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::copyFrom(const TPolyline2D& rOther)
{
	*this = rOther;
}
/*****************************************************************************/
template < class POINT_TYPE >
void TPolyline2D<POINT_TYPE>::deform(const TemplatePointVector& vecPoints, const SBezierCurve* pSourceCurve, const SBezierCurve* pTargetCurve, TemplatePointVector& rDeformedOut)
{
	if(pSourceCurve->getNumSegments() == 0 || pTargetCurve->getNumSegments() == 0)
	{
		rDeformedOut = vecPoints;
		return;
	}

	// Here, for each point, we approximate the distance change and deform it by that much.
	rDeformedOut.clear();
	POINT_TYPE rTempPoint;
	SVector2D svClosestOut;
	FLOAT_TYPE fParmDist;
	int iSegmentIndex;
	int iPoint, iNumPoints = vecPoints.size();
	SVector2D svNewPos, svDiff;
	for(iPoint = 0; iPoint < iNumPoints; iPoint++)
	{
		rTempPoint = vecPoints[iPoint];
		pSourceCurve->findPointClosestTo(rTempPoint.getPoint(), svClosestOut, &fParmDist, &iSegmentIndex);
		pTargetCurve->getSegment(iSegmentIndex)->evaluate(fParmDist, svNewPos);

		svDiff = svNewPos - svClosestOut;
		rTempPoint.getPoint() += svDiff;
		rDeformedOut.push_back(rTempPoint);
	}
}
/*****************************************************************************/
template class TPolyline2D< SBrushPoint >;
template class TPolyline2D< SVector2D >;
template class TPolyline2D< SColorPoint >;
template class TPolyline2D< SBezierPoint >;
};