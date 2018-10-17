#include "stdafx.h"

#define SAVE_CURVE						"curve"
#define SAVE_CURVE_COLLECTION_BLOCK		"curveColl"
#define SAVE_CURVE_COLLECTION_MAX_ID	"maxId"

namespace HyperUI
{
/*****************************************************************************/
template < class TYPE > 
TBezierCurveCollection<TYPE>::TBezierCurveCollection()
{
	myParent = NULL;
	myMaxId = 0;
}
/*****************************************************************************/
template < class TYPE > 
TBezierCurveCollection<TYPE>::~TBezierCurveCollection()
{
	clear();
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurveCollection<TYPE>::clear()
{
	clearAndDeleteContainer(myCurves);
	myMaxId = 0;
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurveCollection<TYPE>::addCurve(BEZIER_CURVE_TYPE* pCurve, bool bInsertInFront)
{
	pCurve->setParent(this);
	if(bInsertInFront)
		myCurves.insert(myCurves.begin(), pCurve);
	else
		myCurves.push_back(pCurve);
}
/*****************************************************************************/
template < class TYPE >
BEZIER_CURVE_TYPE* TBezierCurveCollection<TYPE>::addNewCurve(bool bIsClosed)
{
	BEZIER_CURVE_TYPE* pNew = new BEZIER_CURVE_TYPE;
	addCurve(pNew);
	pNew->setIsClosed(bIsClosed);
	return pNew;
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurveCollection<TYPE>::deleteCurve(BEZIER_CURVE_TYPE* pCurve)
{
	// Find it in the vector
	typename vector < BEZIER_CURVE_TYPE* >::iterator vi = std::find(myCurves.begin(), myCurves.end(), pCurve);
	if(vi == myCurves.end())
		ASSERT_RETURN;

	// Otherwise, delete the item
	delete pCurve;
	
	// Then delete it in the list:
	myCurves.erase(vi);
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurveCollection<TYPE>::copyFrom(const TBezierCurveCollection& rOther, bool bIgnoreBackgroundCurves, FLOAT_TYPE fPerfScalingFactor)
{
	clear();
	mergeFrom(rOther, bIgnoreBackgroundCurves, false, fPerfScalingFactor);
}
/*****************************************************************************/
template < class TYPE > 
bool TBezierCurveCollection<TYPE>::getBBox(SRect2D& svBBoxOut, bool bIgnoreBackgroundCurves) const 
{
	svBBoxOut.set(0, 0, 0, 0);

	int iCurr, iNum = myCurves.size();
	if(iNum <= 0)
		return false;

	if(!bIgnoreBackgroundCurves || !myCurves[0]->getIsBackground())
		myCurves[0]->getBBox(svBBoxOut);
	SRect2D svTempRect;
	for(iCurr = 1; iCurr < iNum; iCurr++)
	{
		if(!bIgnoreBackgroundCurves || !myCurves[iCurr]->getIsBackground())
		{
			myCurves[iCurr]->getBBox(svTempRect);
			svBBoxOut.includeRect(svTempRect);
		}
	}

	return true;
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurveCollection<TYPE>::mergeFrom(const TBezierCurveCollection& rOther, bool bIgnoreBackgroundCurves, bool bInsertInFront, FLOAT_TYPE fPerfScalingFactor)
{
	const BEZIER_CURVE_TYPE* pCurve;
	UNIQUEID_TYPE iMaxId;
	int iCurr, iNum = rOther.getNumCurves();
	int iCurrIdx;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(bInsertInFront)
			iCurrIdx = iNum - iCurr - 1;
		else
			iCurrIdx = iCurr;

		pCurve = rOther.getCurve(iCurrIdx);
		if(!bIgnoreBackgroundCurves || !pCurve->getIsBackground())
			addCurve(pCurve->cloneSelf(fPerfScalingFactor), bInsertInFront);

		iMaxId = pCurve->getMaxId();
		if(iMaxId > myMaxId)
			myMaxId = iMaxId;
	}
}
/*****************************************************************************/
template < class TYPE > 
TYPE* TBezierCurveCollection<TYPE>::findSegmentByEdgeId(const EdgeId& idEdge) const
{
	TYPE* pRes = NULL;
	int iCurr, iNum = getNumCurves();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pRes = getCurve(iCurr)->findSegmentByEdgeId(idEdge);
		if(pRes)
			break;
	}
	return pRes;
}
/*****************************************************************************/
template < class TYPE > 
const TYPE* TBezierCurveCollection<TYPE>::findVertexClosestTo(const SVector2D& svPoint, FLOAT_TYPE fOptMaxRadius, SVector2D& svResPointOut, BezierPointType& ePointTypeOut) const
{
	SVector2D svTempPoint;
	FLOAT_TYPE fDistSq = FLOAT_TYPE_MAX;
	FLOAT_TYPE fCurrDistSq;

	const TYPE* pRes = NULL;
	const TYPE* pCurrRes;
	BezierPointType eCurrPointType;
	const BEZIER_CURVE_TYPE *pCurve;

	int iCurr, iNum = getNumCurves();
	if(iNum <= 0)
		return pRes;

	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pCurve = getCurve(iCurr);
		pCurrRes = pCurve->findVertexClosestTo(svPoint, fOptMaxRadius, svTempPoint, eCurrPointType);

		fCurrDistSq = (svTempPoint - svPoint).lengthSquared();
		if(pCurrRes && fCurrDistSq < fDistSq)
		{
			fDistSq = fCurrDistSq;
			svResPointOut = svTempPoint;
			pRes = pCurrRes;
			ePointTypeOut = eCurrPointType;
		}
	}

	return pRes;
}
/*****************************************************************************/
template < class TYPE > 
const TYPE* TBezierCurveCollection<TYPE>::findPointClosestTo(const SVector2D& svPoint, FLOAT_TYPE fMaxDistance, SVector2D& svPointOut, FLOAT_TYPE* fOptParmDistOut) const
{
	SVector2D svTempPoint;
	FLOAT_TYPE fDistSq = FLOAT_TYPE_MAX;
	if(fMaxDistance > 0)
		fDistSq = fMaxDistance*fMaxDistance;
	FLOAT_TYPE fCurrDistSq;

	const TYPE* pRes = NULL;
	const TYPE* pCurrRes;

	const BEZIER_CURVE_TYPE *pCurve;

	int iCurr, iNum = getNumCurves();
	if(iNum <= 0)
		return pRes;

	FLOAT_TYPE fParmDist;

	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pCurve = getCurve(iCurr);
		pCurrRes = pCurve->findPointClosestTo(svPoint, svTempPoint, &fParmDist);
		if(!pCurrRes)
			continue;

		fCurrDistSq = (svTempPoint - svPoint).lengthSquared();

		if(fCurrDistSq < fDistSq)
		{
			fDistSq = fCurrDistSq;
			svPointOut = svTempPoint;
			pRes = pCurrRes;
			if(fOptParmDistOut)
				*fOptParmDistOut = fParmDist;
		}
	}

	return pRes;
}
/*****************************************************************************/
template < class TYPE > 
BEZIER_CURVE_TYPE* TBezierCurveCollection<TYPE>::createAndAddRectangle(FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fCornerRadius, CornerType eCornerType)
{
	const FLOAT_TYPE CORNER_CURVE_CONTROL_POINT_INTERP = 0.5;

	if(eCornerType == CornerNone)
		fCornerRadius = 0;

	if(fCornerRadius > fW/2.0)
		fCornerRadius = fW/2.0;
	if(fCornerRadius > fH/2.0)
		fCornerRadius = fH/2.0;

	bool bDoSharpCorners = (eCornerType == CornerStraight);

	SVector2D svStart, svEnd;
	BEZIER_CURVE_TYPE* pNew = addNewCurve(true);

	if(fCornerRadius < 1)
	{
		pNew->makeIntoRectangle(fStartX, fStartY, fW, fH);
/*
		svStart.set(fStartX, fStartY);
		svEnd.set(fStartX + fW, fStartY);
		pNew->addLinear(svStart, svEnd, EDGE_ID_TOP);

		svStart = svEnd;
		svEnd.set(fStartX + fW, fStartY + fH);
		pNew->addLinear(svStart, svEnd, EDGE_ID_RIGHT);

		svStart = svEnd;
		svEnd.set(fStartX, fStartY + fH);
		pNew->addLinear(svStart, svEnd, EDGE_ID_BOTTOM);

		svStart = svEnd;
		svEnd.set(fStartX, fStartY);
		pNew->addLinear(svStart, svEnd, EDGE_ID_LEFT);*/
	}
	else
	{
		SVector2D svControl1, svControl2;

		// Hor piece
		svStart.set(fStartX + fCornerRadius, fStartY);
		svEnd.set(fStartX + fW - fCornerRadius, fStartY);
		if(svEnd.x - svStart.x > FLOAT_EPSILON)
			pNew->addLinear(svStart, svEnd, EDGE_ID_TOP);

		// TR corner
		svStart = svEnd;
		svEnd.set(fStartX + fW, fStartY + fCornerRadius);
		svControl1.x = svStart.x*CORNER_CURVE_CONTROL_POINT_INTERP + svEnd.x*(1.0 - CORNER_CURVE_CONTROL_POINT_INTERP);
		svControl1.y = svStart.y;
		svControl2.x = svEnd.x;
		svControl2.y = svEnd.y*CORNER_CURVE_CONTROL_POINT_INTERP + svStart.y*(1.0 - CORNER_CURVE_CONTROL_POINT_INTERP);
		if(bDoSharpCorners)
			pNew->addLinear(svStart, svEnd, EDGE_ID_TR);
		else
			pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, EDGE_ID_TR);

		// Far vert
		svStart = svEnd;
		svEnd.set(svStart.x, fStartY + fH - fCornerRadius);
		if(svEnd.y - svStart.y > FLOAT_EPSILON)
			pNew->addLinear(svStart, svEnd, EDGE_ID_RIGHT);

		// BR corner
		svStart = svEnd;
		svEnd.set(fStartX + fW - fCornerRadius, fStartY + fH);
		svControl1.x = svStart.x;
		svControl1.y = svStart.y*CORNER_CURVE_CONTROL_POINT_INTERP + svEnd.y*(1.0 - CORNER_CURVE_CONTROL_POINT_INTERP);
		svControl2.x = svEnd.x*CORNER_CURVE_CONTROL_POINT_INTERP + svStart.x*(1.0 - CORNER_CURVE_CONTROL_POINT_INTERP);
		svControl2.y = svEnd.y;
		if(bDoSharpCorners)
			pNew->addLinear(svStart, svEnd, EDGE_ID_BR);
		else
			pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, EDGE_ID_BR);

		// Bottom hors
		svStart = svEnd;
		svEnd.set(fStartX + fCornerRadius, fStartY + fH);
		if(svStart.x - svEnd.x > FLOAT_EPSILON)
			pNew->addLinear(svStart, svEnd, EDGE_ID_BOTTOM);

		// BL corner
		svStart = svEnd;
		svEnd.set(fStartX, fStartY + fH - fCornerRadius);
		svControl1.x = svStart.x*CORNER_CURVE_CONTROL_POINT_INTERP + svEnd.x*(1.0 - CORNER_CURVE_CONTROL_POINT_INTERP);
		svControl1.y = svStart.y;
		svControl2.x = svEnd.x;
		svControl2.y = svEnd.y*CORNER_CURVE_CONTROL_POINT_INTERP + svStart.y*(1.0 - CORNER_CURVE_CONTROL_POINT_INTERP);
		if(bDoSharpCorners)
			pNew->addLinear(svStart, svEnd, EDGE_ID_BL);
		else
			pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, EDGE_ID_BL);

		// Vert near
		svStart = svEnd;
		svEnd.set(svStart.x, fStartY + fCornerRadius);
		if(svStart.y - svEnd.y > FLOAT_EPSILON)
			pNew->addLinear(svStart, svEnd, EDGE_ID_LEFT);

		// TL corner
		svStart = svEnd;
		svEnd.set(fStartX + fCornerRadius, fStartY);
		svControl1.x = svStart.x;
		svControl1.y = svStart.y*CORNER_CURVE_CONTROL_POINT_INTERP + svEnd.y*(1.0 - CORNER_CURVE_CONTROL_POINT_INTERP);
		svControl2.x = svEnd.x*CORNER_CURVE_CONTROL_POINT_INTERP + svStart.x*(1.0 - CORNER_CURVE_CONTROL_POINT_INTERP);
		svControl2.y = svEnd.y;
		if(bDoSharpCorners)
			pNew->addLinear(svStart, svEnd, EDGE_ID_TL);
		else
			pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, EDGE_ID_TL);

	}
	
	return pNew;
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurveCollection<TYPE>::onChildSegmentEdgeIdChanged(const EdgeId& idOld, const EdgeId& idNew)
{
	if(idNew.getNumeric() > myMaxId)
		myMaxId = idNew.getNumeric();
	if(myParent)
		myParent->onChildSegmentEdgeIdChanged(idOld, idNew);
}
/*****************************************************************************/
template < class TYPE > 
void TBezierCurveCollection<TYPE>::transformBy(const SMatrix2D& smTransform)
{
	int iCurr, iNum = myCurves.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		myCurves[iCurr]->transformBy(smTransform);
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurveCollection<TYPE>::translateBy(const SVector2D& svDiff)
{
	SMatrix2D smTemp;
	smTemp.makeTranslationMatrix(svDiff.x, svDiff.y);
	transformBy(smTemp);
}
/*****************************************************************************/
template < class TYPE >
BEZIER_CURVE_TYPE* TBezierCurveCollection<TYPE>::createAndAddEllipse(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fRadiusX, FLOAT_TYPE fRadiusY)
{
	//  0.5522847498307933984022516322796
	//  0.5522847498, some say 0.551784 This magic number is 0.000501 lower than NORMAL.
	//const FLOAT_TYPE fKappa = 4.0*(F_SQRT(2.0) - 1)/3.0;
	const FLOAT_TYPE fKappa = 0.551784;

	FLOAT_TYPE fHorControlOffset = fRadiusX*fKappa;
	FLOAT_TYPE fVertControlOffset = fRadiusY*fKappa;

	SVector2D svStart, svEnd;
	SVector2D svControl1, svControl2;
	BEZIER_CURVE_TYPE* pNew = addNewCurve(true);

	svStart.set(fCenterX, fCenterY - fRadiusY);
	svEnd.set(fCenterX + fRadiusX, fCenterY);
	svControl1 = svStart;
	svControl1.x += fHorControlOffset;
	svControl2 = svEnd;
	svControl2.y -= fVertControlOffset;
	pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, EDGE_ID_TOP);

	svStart = svEnd;
	svEnd.set(fCenterX, fCenterY + fRadiusY);
	svControl1 = svStart;
	svControl1.y += fVertControlOffset;
	svControl2 = svEnd;
	svControl2.x += fHorControlOffset;
	pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, EDGE_ID_RIGHT);

	svStart = svEnd;
	svEnd.set(fCenterX - fRadiusX, fCenterY);
	svControl1 = svStart;
	svControl1.x -= fHorControlOffset;
	svControl2 = svEnd;
	svControl2.y += fVertControlOffset;
	pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, EDGE_ID_BOTTOM);

	svStart = svEnd;
	svEnd.set(fCenterX, fCenterY - fRadiusY);
	svControl1 = svStart;
	svControl1.y -= fVertControlOffset;
	svControl2 = svEnd;
	svControl2.x -= fHorControlOffset;
	pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, EDGE_ID_LEFT);

	return pNew;
}
/*****************************************************************************/
template < class TYPE >
BEZIER_CURVE_TYPE* TBezierCurveCollection<TYPE>::createAndAddPolygon(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fRadiusX, FLOAT_TYPE fRadiusY, int iNumPoints)
{
	if(iNumPoints < 3)
		iNumPoints = 3;

	BEZIER_CURVE_TYPE* pNew = addNewCurve(true);

	SVector2D svPoint, svPrev;
	int iCurr;
	FLOAT_TYPE fAngleInc = 360.0/(FLOAT_TYPE)iNumPoints;
	for(iCurr = 0; iCurr <= iNumPoints; iCurr++)
	{
		svPoint.x = F_COS_DEG(fAngleInc*(iCurr%iNumPoints))*fRadiusX + fCenterX;
		svPoint.y = F_SIN_DEG(fAngleInc*(iCurr%iNumPoints))*fRadiusY + fCenterY;

		if(iCurr > 0)
			pNew->addLinear(svPrev, svPoint, iCurr + 1);
		svPrev = svPoint;
	}

	return pNew;
}
/*****************************************************************************/
template < class TYPE >
BEZIER_CURVE_TYPE* TBezierCurveCollection<TYPE>::createAndAddStar(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fRadiusX, FLOAT_TYPE fRadiusY, int iNumPoints, FLOAT_TYPE fIndentPerc)
{
	if(iNumPoints < 3)
		iNumPoints = 3;

	BEZIER_CURVE_TYPE* pNew = addNewCurve(true);

	iNumPoints *= 2;

	SVector2D svPoint, svPrev;
	int iCurr;
	FLOAT_TYPE fRadMult;
	FLOAT_TYPE fAngleInc = 360.0/(FLOAT_TYPE)iNumPoints;
	for(iCurr = 0; iCurr <= iNumPoints; iCurr++)
	{
		if(iCurr % 2 == 0)
			fRadMult = 1.0;
		else
			fRadMult = fIndentPerc;

		svPoint.x = F_COS_DEG(fAngleInc*(iCurr%iNumPoints) - 90)*fRadiusX*fRadMult + fCenterX;
		svPoint.y = F_SIN_DEG(fAngleInc*(iCurr%iNumPoints) - 90)*fRadiusY*fRadMult + fCenterY;

		if(iCurr > 0)
			pNew->addLinear(svPrev, svPoint, iCurr + 1);
		svPrev = svPoint;
	}

	return pNew;
}
/*****************************************************************************/
template < class TYPE >
BEZIER_CURVE_TYPE* TBezierCurveCollection<TYPE>::createAndAddPieSlice(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fRadiusX, 
	FLOAT_TYPE fRadiusY, FLOAT_TYPE fStartAngle, FLOAT_TYPE fSpan, bool bIsPie, HyperCore::OrientationType eOrientation)
{
	BEZIER_CURVE_TYPE* pNew = addNewCurve(bIsPie);
	setToArc(pNew, fCenterX, fCenterY, fRadiusX, fRadiusY, fStartAngle, fSpan, eOrientation, 2);
	/*
	fStartAngle = sanitizeDegAngle(fStartAngle);
	
	BEZIER_CURVE_TYPE* pNew = addNewCurve(bIsPie);

	FLOAT_TYPE fEndAngle = fStartAngle + fSpan;

	SVector2D svCenter(fCenterX, fCenterY);

	SVector2D svStart, svEnd;
	SVector2D svControl1, svControl2;
	FLOAT_TYPE fTempSpan, fPrevStopAngle;
	int iSegIdCounter = 1;

	// Add the arc before we start complete spans.
	fPrevStopAngle = ((int)(fStartAngle/90.0) + 1)*90;
	fTempSpan = fPrevStopAngle - fStartAngle;
	_ASSERT(fTempSpan > 0 && fTempSpan <= 90.0);
	if(fTempSpan < 90.0)
	{
		computeArcBezierApproximation(fCenterX, fCenterY, fRadiusX, fRadiusY, fStartAngle, fTempSpan, eOrientation, svStart, svControl1, svControl2, svEnd);
		pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, iSegIdCounter);
		iSegIdCounter++;
	}
	else
		fPrevStopAngle = fStartAngle;

	// Add complete spans we might have, if any
	int iCurr90Span, iNum90Spans = (fEndAngle - fPrevStopAngle)/90;
	for(iCurr90Span = 0; iCurr90Span < iNum90Spans; iCurr90Span++)
	{
		computeArcBezierApproximation(fCenterX, fCenterY, fRadiusX, fRadiusY, fPrevStopAngle, 90, eOrientation, svStart, svControl1, svControl2, svEnd);
		pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, iSegIdCounter);
		iSegIdCounter++;
		fPrevStopAngle += 90;
	}

	// Add the last trailing arc, if any
	fTempSpan = fEndAngle - fPrevStopAngle;
	if(fTempSpan > 0)
	{
		computeArcBezierApproximation(fCenterX, fCenterY, fRadiusX, fRadiusY, fPrevStopAngle, fTempSpan, eOrientation, svStart, svControl1, svControl2, svEnd);
		pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, iSegIdCounter);
		iSegIdCounter++;
	}
	*/

	SVector2D svCenter(fCenterX, fCenterY);
	SVector2D svStart, svEnd;
	int iNumSegs = pNew->getNumSegments();
	if(bIsPie && iNumSegs > 0)
	{
		pNew->getSegment(0)->getStart(svStart);
		pNew->getSegment(iNumSegs - 1)->getEnd(svEnd);

		pNew->insertLinear(0, svCenter, svStart,  1);
		pNew->addLinear(svEnd, svCenter, 6);
	}

	return pNew;
}
/*****************************************************************************/
template < class TYPE >
int TBezierCurveCollection<TYPE>::setToArc(BEZIER_CURVE_TYPE* pNew, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fRadiusX, FLOAT_TYPE fRadiusY, 
	FLOAT_TYPE fStartAngle, FLOAT_TYPE fSpan, HyperCore::OrientationType eOrientation, int iStartSegmentId)
{
	fStartAngle = HyperCore::sanitizeDegAngle(fStartAngle);

	FLOAT_TYPE fEndAngle = fStartAngle + fSpan;

	SVector2D svCenter(fCenterX, fCenterY);

	SVector2D svStart, svEnd;
	SVector2D svControl1, svControl2;
	FLOAT_TYPE fTempSpan, fPrevStopAngle;
	int iSegIdCounter = iStartSegmentId;

	// Add the arc before we start complete spans.
	fPrevStopAngle = ((int)(fStartAngle/90.0) + 1)*90;
	fTempSpan = fPrevStopAngle - fStartAngle;
	_ASSERT(fTempSpan > 0 && fTempSpan <= 90.0);
	if(fTempSpan < 90.0)
	{
		computeArcBezierApproximation(fCenterX, fCenterY, fRadiusX, fRadiusY, fStartAngle, fTempSpan, eOrientation, svStart, svControl1, svControl2, svEnd);
		pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, iSegIdCounter);
		iSegIdCounter++;
	}
	else
		fPrevStopAngle = fStartAngle;

	// Add complete spans we might have, if any
	int iCurr90Span, iNum90Spans = (fEndAngle - fPrevStopAngle)/90;
	for(iCurr90Span = 0; iCurr90Span < iNum90Spans; iCurr90Span++)
	{
		computeArcBezierApproximation(fCenterX, fCenterY, fRadiusX, fRadiusY, fPrevStopAngle, 90, eOrientation, svStart, svControl1, svControl2, svEnd);
		pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, iSegIdCounter);
		iSegIdCounter++;
		fPrevStopAngle += 90;
	}

	// Add the last trailing arc, if any
	fTempSpan = fEndAngle - fPrevStopAngle;
	if(fTempSpan > 0)
	{
		computeArcBezierApproximation(fCenterX, fCenterY, fRadiusX, fRadiusY, fPrevStopAngle, fTempSpan, eOrientation, svStart, svControl1, svControl2, svEnd);
		pNew->addBezier(svStart, svControl1, svControl2, svEnd, false, iSegIdCounter);
		iSegIdCounter++;
	}

	return iSegIdCounter;
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurveCollection<TYPE>::computeArcBezierApproximation(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fRadiusX, FLOAT_TYPE fRadiusY, 
	FLOAT_TYPE fStartAngle, FLOAT_TYPE fSpan, HyperCore::OrientationType eOrientation, SVector2D& svStartOut, SVector2D& svControl1Out, SVector2D& svControl2Out, SVector2D& svEndOut)
{
	// To draw CW, set to 1. To draw CCW, set to -1.
	FLOAT_TYPE fYMult = eOrientation == OrientationCCW ? -1.0 : 1.0;
	FLOAT_TYPE fEndAngle = fStartAngle + fSpan;

	SVector2D svCenter(fCenterX, fCenterY);

	FLOAT_TYPE fTanCompSq = tan((fEndAngle - fStartAngle)*0.5/180.0*M_PI);
	fTanCompSq = fTanCompSq*fTanCompSq;

	FLOAT_TYPE fAlpha = F_SIN_DEG(fEndAngle - fStartAngle)*(F_SQRT(4.0 + 3.0*fTanCompSq) - 1.0)/3.0;

	SVector2D svTempDeriv;

	svStartOut.x = F_COS_DEG(fStartAngle)*fRadiusX + fCenterX;
	svStartOut.y = fYMult*F_SIN_DEG(fStartAngle)*fRadiusY + fCenterY;

	svEndOut.x = F_COS_DEG(fEndAngle)*fRadiusX + fCenterX;
	svEndOut.y = fYMult*F_SIN_DEG(fEndAngle)*fRadiusY + fCenterY;

	svTempDeriv.x = -fRadiusX*F_SIN_DEG(fStartAngle);
	svTempDeriv.y = fYMult*fRadiusY*F_COS_DEG(fStartAngle);
	svControl1Out = svStartOut + svTempDeriv*fAlpha;

	svTempDeriv.x = -fRadiusX*F_SIN_DEG(fEndAngle);
	svTempDeriv.y = fYMult*fRadiusY*F_COS_DEG(fEndAngle);
	svControl2Out = svEndOut - svTempDeriv*fAlpha;
}
/*****************************************************************************/
template < class TYPE >
BEZIER_CURVE_TYPE* TBezierCurveCollection<TYPE>::createAndAddRing(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fOuterRadiusX, FLOAT_TYPE fOuterRadiusY, 
	FLOAT_TYPE fInnerRadiusX, FLOAT_TYPE fInnerRadiusY, FLOAT_TYPE fStartAngle, FLOAT_TYPE fSpan, HyperCore::OrientationType eOrientation)
{
	BEZIER_CURVE_TYPE* pNew = addNewCurve(true);
	setToArc(pNew, fCenterX, fCenterY, fOuterRadiusX, fOuterRadiusY, fStartAngle, fSpan, eOrientation, 3);
	int iNumOuterSegs = pNew->getNumSegments();

	TBezierCurveCollection<TYPE> rTempColl;
	BEZIER_CURVE_TYPE* pNew2 = rTempColl.addNewCurve(false);
	
	setToArc(pNew2, fCenterX, fCenterY, fInnerRadiusX, fInnerRadiusY, fStartAngle, fSpan, eOrientation, iNumOuterSegs + 3);
	pNew2->reverse();

	// Now, append the stre
	SVector2D svOuterStart, svOuterEnd;
	pNew->getSegment(0)->getStart(svOuterStart);
	pNew->getSegment(iNumOuterSegs - 1)->getEnd(svOuterEnd);

	SVector2D svInnerStart, svInnerEnd;
	int iNumInnerSegs = pNew2->getNumSegments();
	pNew2->getSegment(0)->getStart(svInnerStart);
	pNew2->getSegment(iNumInnerSegs- 1)->getEnd(svInnerEnd);

	pNew->addLinear(svOuterEnd, svInnerStart, 1);
	pNew->append(*pNew2);
	pNew->addLinear(svInnerEnd, svOuterStart, 2);

	return pNew;
}
/*****************************************************************************/
template < class TYPE >
BEZIER_CURVE_TYPE* TBezierCurveCollection<TYPE>::createAndAddLine(FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fEndX, FLOAT_TYPE fEndY)
{
	BEZIER_CURVE_TYPE* pNew = addNewCurve(false);
	SVector2D svStart(fStartX, fStartY);
	SVector2D svEnd(fEndX, fEndY);

	pNew->addLinear(svStart, svEnd, 1);

	return pNew;
}
/*****************************************************************************/
template < class TYPE >
bool TBezierCurveCollection<TYPE>::canLoadFrom(const StringResourceItem& rItem)
{
	const StringResourceItem* pBlock = rItem.getChildById(SAVE_CURVE_COLLECTION_BLOCK, false);
	return pBlock != NULL;
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurveCollection<TYPE>::loadFromItem(const StringResourceItem& rItem)
{
	clear();

	// Load from item
	const StringResourceItem* pBlock = rItem.getChildById(SAVE_CURVE_COLLECTION_BLOCK, false);
	if(!pBlock)
		ASSERT_RETURN;

	TBezierCurve<TYPE>* pCurrCurve;
	const StringResourceItem* pChild;
	int iChild, iNumChildren = pBlock->getNumChildren();
	for(iChild = 0; iChild < iNumChildren; iChild++)
	{
		pChild = pBlock->getChild(iChild);

		// Add this material and load it
		pCurrCurve = addNewCurve(false);
		pCurrCurve->loadFromItem(*pChild);
	}

	myMaxId = pBlock->getAsLong(SAVE_CURVE_COLLECTION_MAX_ID);
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurveCollection<TYPE>::saveToItem(StringResourceItem& rItemOut) const
{
	StringResourceItem* pChild;
	StringResourceItem* pBlock = rItemOut.addChildAndSetId(SAVE_CURVE_COLLECTION_BLOCK);
	int iCurve, iNumCurves = myCurves.size();
	for(iCurve = 0; iCurve < iNumCurves; iCurve++)
	{
		pChild = pBlock->addChildAndSetId(SAVE_CURVE);
		myCurves[iCurve]->saveToItem(*pChild);
	}

	pBlock->setAsLong(SAVE_CURVE_COLLECTION_MAX_ID, myMaxId);
}
/*****************************************************************************/
template < class TYPE >
TBezierCurveCollection<TYPE>* TBezierCurveCollection<TYPE>::cloneSelf(bool bIgnoreBackgroundCurves) const
{
	TBezierCurveCollection* pNew = new TBezierCurveCollection;
	pNew->copyFrom(*this, bIgnoreBackgroundCurves);
	return pNew;
}
/*****************************************************************************/
template < class TYPE >
UNIQUEID_TYPE TBezierCurveCollection<TYPE>::getNextId() 
{ 
	UNIQUEID_TYPE idNew = 0;
	if(myParent)
	{
		idNew = myParent->getNextId();
		if(idNew >= myMaxId)
			myMaxId = idNew + 1;
	}
	else
	{
		myMaxId++; 
		idNew = myMaxId;
	}
	
	return idNew; 
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurveCollection<TYPE>::deleteCurvesWithBBoxSmallerThan(FLOAT_TYPE fBBoxSize)
{
	BEZIER_CURVE_TYPE* pCurve;
	SRect2D srBBox;
	int iCount, iNum = myCurves.size();
	for(iCount = iNum - 1; iCount >= 0; iCount--)
	{
		pCurve = myCurves[iCount];
		pCurve->getBBox(srBBox);

		if(srBBox.w <= fBBoxSize && srBBox.h <= fBBoxSize)
		{
			// Delete it:
			delete pCurve;
			myCurves.erase(myCurves.begin() + iCount);
		}
	}
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurveCollection<TYPE>::resetEdgeIdsWithString(const char* pcsStringPart)
{
	BEZIER_CURVE_TYPE* pCurve;
	int iCount, iNum = myCurves.size();
	char pcsBuff[256];
	for(iCount = iNum - 1; iCount >= 0; iCount--)
	{
		pCurve = myCurves[iCount];
		sprintf(pcsBuff, "%s%d", pcsStringPart, iCount);
		pCurve->resetEdgeIdsWithString(pcsBuff);
	}
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurveCollection<TYPE>::applyMaterialId(UNIQUEID_TYPE idMaterial)
{
	BEZIER_CURVE_TYPE* pCurve;
	int iCount, iNum = myCurves.size();
	for(iCount = iNum - 1; iCount >= 0; iCount--)
	{
		pCurve = myCurves[iCount];
		pCurve->setMaterialId(idMaterial);
	}
}
/*****************************************************************************/
template < class TYPE >
void TBezierCurveCollection<TYPE>::classifyHoles()
{
	BEZIER_CURVE_TYPE* pCurve;
	BEZIER_CURVE_TYPE* pCurveInner;
	int iCount, iNum = myCurves.size();
	int iCountInner;
	int iContainCount;
	TPointVector vecCurrPoints;
	TPointVector vecCurrInnerPoints;

	for(iCount = iNum - 1; iCount >= 0; iCount--)
	{
		pCurve = myCurves[iCount];
		
		pCurve->resampleCurves(1.0, -1, vecCurrPoints, NULL);

		// Now, go through all other curves and see how many times it's contained
		// in them.
		iContainCount = 0;
		for(iCountInner = iNum - 1; iCountInner >= 0; iCountInner--)
		{
			pCurveInner = myCurves[iCountInner];

			if(pCurveInner == pCurve)
				continue;

			pCurveInner->resampleCurves(1.0, -1, vecCurrInnerPoints, NULL);

			if(Polyline2D::doesContain(vecCurrPoints, vecCurrInnerPoints))
				iContainCount++;
		}

		pCurve->setIsHole(iContainCount % 2 != 0);
	}
}
/*****************************************************************************/
template class HYPERUI_API TBezierCurveCollection<DeformableBezier>;
template class HYPERUI_API TBezierCurveCollection<SAnimBezierSegment>;
template class HYPERUI_API TBezierCurveCollection<SBezierSegment>;
};