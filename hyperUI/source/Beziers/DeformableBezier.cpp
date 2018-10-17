#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
DeformableBezier::DeformableBezier()
{
	myActiveClock = ClockMainGame;
	myIsLinear = false;
	myEdgeId.setFromNumeric(0);
	myParent = NULL;
}
/*****************************************************************************/
DeformableBezier::~DeformableBezier()
{

}
/*****************************************************************************/
void DeformableBezier::setStart(FLOAT_TYPE dx, FLOAT_TYPE dy)
{
	myStart.setNonAnimValue(dx, dy);
}
/*****************************************************************************/
void DeformableBezier::setEnd(FLOAT_TYPE dx, FLOAT_TYPE dy)
{
	myEnd.setNonAnimValue(dx, dy);
}
/*****************************************************************************/
void DeformableBezier::setControl1(FLOAT_TYPE dx, FLOAT_TYPE dy)
{
	myControl1.setNonAnimValue(dx, dy);
}
/*****************************************************************************/
void DeformableBezier::setControl2(FLOAT_TYPE dx, FLOAT_TYPE dy)
{
	myControl2.setNonAnimValue(dx, dy);
}
/*****************************************************************************/
void DeformableBezier::setIsLinear(bool bValue)
{
	myIsLinear = bValue;
}
/*****************************************************************************/
AnimatedPoint* DeformableBezier::getStart(void)
{
	return &myStart;
}
/*****************************************************************************/
AnimatedPoint* DeformableBezier::getEnd(void)
{
	return &myEnd;
}
/*****************************************************************************/
DeformableBezier* DeformableBezier::cloneSelf(FLOAT_TYPE fPerfScalingFactor) const
{
	DeformableBezier* pNew = new DeformableBezier;
	pNew->copyFrom(*this, fPerfScalingFactor);
	pNew->myParent = NULL;
	return pNew;
}
/*****************************************************************************/
void DeformableBezier::getTangent(FLOAT_TYPE dInterpPos, SVector2D& svTangentOut) const
{
	SVector2D svEnd, svStart;
	myStart.getValue(Application::getInstance()->getGlobalTime(myActiveClock), svStart);
	myEnd.getValue(Application::getInstance()->getGlobalTime(myActiveClock), svEnd);

	if(myIsLinear)
	{
		SVector2D svDir, svRes;
		svDir = svEnd - svStart;
		svDir.normalize();
		svTangentOut = svDir;
		return;
	}	

	SVector2D svControl1, svControl2;
	myControl1.getValue(Application::getInstance()->getGlobalTime(myActiveClock), svControl1);
	myControl2.getValue(Application::getInstance()->getGlobalTime(myActiveClock), svControl2);


	// We're interested only in the right tangent (going forward).
	svTangentOut = svStart*(1.0 - dInterpPos)*(1.0 - dInterpPos)*-3.0 +
		svControl1*(3.0*(1.0 - dInterpPos)*(1.0 - dInterpPos) - 6.0*(1.0 - dInterpPos)*dInterpPos) +
		svControl2*(6.0*(1.0 - dInterpPos)*dInterpPos - 3*dInterpPos*dInterpPos) +
		svEnd*dInterpPos*dInterpPos*3.0;
	svTangentOut.normalize();
/*
	SVector2D svPt1, svPt2, svPt3;
	SVector2D svA, svB;
	svPt1 = svStart*(1.0-dInterpPos) + svControl1*dInterpPos;
	svPt2 = svControl1*(1.0-dInterpPos) + svControl2*dInterpPos;
	svPt3 = svControl2*(1.0-dInterpPos) + svEnd*dInterpPos;

	svB = svPt2*(1.0-dInterpPos) + svPt3*dInterpPos;
	svTangentOut = svB;
	svTangentOut.normalize();
	*/
}
/*****************************************************************************/
FLOAT_TYPE DeformableBezier::computeDistancesOnPiecewiseLinear(TPointVector& rRawPoints)
{
	FLOAT_TYPE dTotalLength = 0;

	int iCurrPoint , iNumPoints = rRawPoints.size();
	for(iCurrPoint = 1; iCurrPoint < iNumPoints; iCurrPoint++)
	{
		dTotalLength += (rRawPoints[iCurrPoint] - rRawPoints[iCurrPoint-1]).length();
	}

	return dTotalLength;
}	
/*****************************************************************************/
void DeformableBezier::resample(FLOAT_TYPE dError, FLOAT_TYPE dMaxPointSpacing, bool bAddFirstPoint, TPointVector& rPointsOut, TPointVector* optTangentsOut) const
{
	FLOAT_TYPE dSpacingSq;
	if(dMaxPointSpacing > 0)
		dSpacingSq = dMaxPointSpacing * dMaxPointSpacing;
	else
		dSpacingSq = -1;
	FLOAT_TYPE dErrorSq = dError * dError;

	// Add start point if needed
	if(bAddFirstPoint)
	{
		SVector2D svCompStart;
		evaluate(0.0, svCompStart);
		rPointsOut.push_back(svCompStart);

		if(optTangentsOut)
		{
			getTangent(0.0, svCompStart);

			optTangentsOut->push_back(svCompStart);
		}
	}

	resampleSingleBezierRecursive(0.0, 1.0, dErrorSq, dSpacingSq, rPointsOut, optTangentsOut);
}
/*****************************************************************************/
void DeformableBezier::resampleSingleBezierRecursive(FLOAT_TYPE dTStart, FLOAT_TYPE dTEnd, FLOAT_TYPE dErrorSq, FLOAT_TYPE dMaxPointSpacingSq, TPointVector& rPointsOut, TPointVector* optTangentsOut) const
{
	// Get the point at the start and end
	FLOAT_TYPE dTMid = (dTStart + dTEnd)*0.5;
	SVector2D svCompStart, svCompEnd, svMidPoint, svEvalMid;

	evaluate(dTStart, svCompStart);
	evaluate(dTEnd, svCompEnd);

	// See the difference squared between the middle of the two points and the middle of the spline
	svMidPoint = (svCompStart + svCompEnd)*0.5;

	evaluate(dTMid, svEvalMid);

	bool bForceRecursion = false;
	if(dMaxPointSpacingSq > 0)
	{
		if((svCompStart - svCompEnd).lengthSquared() > dMaxPointSpacingSq)
			bForceRecursion = true;
	}

	if( (svEvalMid - svMidPoint).lengthSquared() > dErrorSq || bForceRecursion)
	{
		// Recurse 
		resampleSingleBezierRecursive(dTStart, dTMid, dErrorSq, dMaxPointSpacingSq, rPointsOut, optTangentsOut);
		resampleSingleBezierRecursive(dTMid, dTEnd, dErrorSq, dMaxPointSpacingSq, rPointsOut, optTangentsOut);
	}
	else
	{
		// We're good!
		rPointsOut.push_back(svCompEnd);

		if(optTangentsOut)
		{
			SVector2D svTan;
			getTangent(dTEnd, svTan);
			optTangentsOut->push_back(svTan);
		}
	}

}
/*****************************************************************************/
FLOAT_TYPE DeformableBezier::getLength(FLOAT_TYPE dTolerance, FLOAT_TYPE dMaxPointSpacing)
{
	if(myIsLinear)
	{
		SVector2D svEnd, svStart;
		myStart.getValue(Application::getInstance()->getGlobalTime(myActiveClock), svStart);
		myEnd.getValue(Application::getInstance()->getGlobalTime(myActiveClock), svEnd);

		SVector2D svDir, svRes;
		svDir = svEnd - svStart;
		return svDir.length();
	}

	SVector2D svTemp;
	TPointVector vecTempPoints;
	evaluate(0, svTemp);
	vecTempPoints.push_back(svTemp);
	resampleSingleBezierRecursive(0, 1, dTolerance*dTolerance, dMaxPointSpacing*dMaxPointSpacing, vecTempPoints, NULL);
	return computeDistancesOnPiecewiseLinear(vecTempPoints);
}
/*****************************************************************************/
void DeformableBezier::evaluate(FLOAT_TYPE dInterpPos, SVector2D& svOut) const
{
	SVector2D svVTemp[4];

	//SVector2D svEnd, svStart;
	myStart.getValue(Application::getInstance()->getGlobalTime(myActiveClock), svVTemp[0]);
	myEnd.getValue(Application::getInstance()->getGlobalTime(myActiveClock), svVTemp[3]);

	if(myIsLinear)
	{
		// Linear interp.
		SVector2D svDir;
		svDir = svVTemp[3] - svVTemp[0];
		FLOAT_TYPE dLen = svDir.normalize();
		svOut = svVTemp[0] + svDir*(dLen*dInterpPos);
		return;
	}

	//SVector2D svControl1, svControl2;
	myControl1.getValue(Application::getInstance()->getGlobalTime(myActiveClock), svVTemp[1]);
	myControl2.getValue(Application::getInstance()->getGlobalTime(myActiveClock), svVTemp[2]);

	// TODO: Replace with 3rd degree eval code.
	const int iDegree = 3;

	int iCurrDegree, iInnerDegree;
/*
	svVTemp[0] = svStart;
	svVTemp[1] = svControl1;
	svVTemp[2] = svControl2;
	svVTemp[3] = svEnd;
	*/

	for(iCurrDegree = 1; iCurrDegree <= iDegree; iCurrDegree++)	
	{
		for(iInnerDegree = 0; iInnerDegree <= iDegree-iCurrDegree; iInnerDegree++)
		{
			svVTemp[iInnerDegree] =  svVTemp[iInnerDegree]*(1.0 - dInterpPos) + svVTemp[iInnerDegree+1]*dInterpPos;
		}
	}

	svOut = svVTemp[0];	
}
/*****************************************************************************/
void DeformableBezier::onTimerTick(GTIME lCurrentTime)
{
/*
	myStart.checkTime(lCurrentTime);
	myEnd.checkTime(lCurrentTime);
	myControl1.checkTime(lCurrentTime);
	myControl2.checkTime(lCurrentTime);
*/
}
/*****************************************************************************/
void DeformableBezier::getBBox(SRect2D& srRectOut) const
{
	SVector2D svTempPoint;
	GTIME lTime = Application::getInstance()->getGlobalTime(myActiveClock);

	myStart.getValue(lTime, svTempPoint);
	srRectOut.x = svTempPoint.x;
	srRectOut.y = svTempPoint.y;
	srRectOut.w = srRectOut.h = 0;

	// And include the rest:
	myEnd.getValue(lTime, svTempPoint);
	srRectOut.includePoint(svTempPoint.x, svTempPoint.y);

	myControl1.getValue(lTime, svTempPoint);
	srRectOut.includePoint(svTempPoint.x, svTempPoint.y);

	myControl2.getValue(lTime, svTempPoint);
	srRectOut.includePoint(svTempPoint.x, svTempPoint.y);
}
/*****************************************************************************/
void DeformableBezier::getPointByType(BezierPointType ePointType, SVector2D& svOut) const
{
	GTIME lTime = Application::getInstance()->getGlobalTime(myActiveClock);

	if(ePointType == BezierPointStart)
		myStart.getValue(lTime, svOut);
	else if(ePointType == BezierPointControl1)
		myControl1.getValue(lTime, svOut);
	else if(ePointType == BezierPointControl2)
		myControl2.getValue(lTime, svOut);
	else if(ePointType == BezierPointEnd)
		myEnd.getValue(lTime, svOut);
	else
	{
		_ASSERT(0);
		svOut.set(0, 0);
	}
}
/*****************************************************************************/
void DeformableBezier::setPointByType(BezierPointType ePointType, const SVector2D& svIn)
{
	if(ePointType == BezierPointStart)
		myStart.setNonAnimValue(svIn.x, svIn.y);
	else if(ePointType == BezierPointControl1)
		myControl1.setNonAnimValue(svIn.x, svIn.y);
	else if(ePointType == BezierPointControl2)
		myControl2.setNonAnimValue(svIn.x, svIn.y);
	else if(ePointType == BezierPointEnd)
		myEnd.setNonAnimValue(svIn.x, svIn.y);
	ELSE_ASSERT;
}
/*****************************************************************************/
void DeformableBezier::movePointBy(BezierPointType ePointType, FLOAT_TYPE fX, FLOAT_TYPE fY, bool bAllowParentCallback, bool bIgnoreTiedControls)
{
	SVector2D svNewPos;
	this->getPointByType(ePointType, svNewPos);
	svNewPos.x += fX;
	svNewPos.y += fY;
	this->setPointByType(ePointType, svNewPos);

	if(myIsLinear)
		ensureIsLinear();

	if(myParent && bAllowParentCallback) // (ePointType == BezierPointStart || ePointType == BezierPointEnd))
		myParent->onSegmentPointChanged(this, ePointType, fX, fY, bIgnoreTiedControls);
}
/*****************************************************************************/
void DeformableBezier::ensureIsLinear()
{
	if(!myIsLinear)
		return;

	SVector2D svTemp;
	SVector2D svStart, svEnd;
	getPointByType(BezierPointStart, svStart);
	getPointByType(BezierPointEnd, svEnd);
	svTemp = (svStart + svEnd)*0.5;

	setPointByType(BezierPointControl1, svTemp);
	setPointByType(BezierPointControl2, svTemp);
}
/*****************************************************************************/
void DeformableBezier::copyFrom(const DeformableBezier& rOther, FLOAT_TYPE fPerfScalingFactor)
{
	myStart = rOther.myStart;
	myEnd = rOther.myEnd;
	myControl1 = rOther.myControl1;
	myControl2 = rOther.myControl2;
	myIsLinear = rOther.myIsLinear;

	if(fPerfScalingFactor != 1.0)
	{
		_ASSERT(0);
/*
		myStart *= fPerfScalingFactor;
		myEnd *= fPerfScalingFactor;
		myControl1 *= fPerfScalingFactor;
		myControl2 *= fPerfScalingFactor;
*/
	}


	myActiveClock = rOther.myActiveClock;

	myEdgeId = rOther.myEdgeId;

	// Don't touch
	// TBezierCurve<DeformableBezier>* myParent;

}
/*****************************************************************************/
void DeformableBezier::transformBy(const SMatrix2D& smTransform)
{
	myStart.transformBy(smTransform);
	myControl1.transformBy(smTransform);
	myControl2.transformBy(smTransform);
	myEnd.transformBy(smTransform);
}
/*****************************************************************************/
void DeformableBezier::reverse()
{
	AnimatedPoint svTemp = myStart;
	myStart = myEnd;
	myEnd = svTemp;

	svTemp = myControl1;
	myControl1 = myControl2;
	myControl2 = svTemp;

}
/*****************************************************************************/
};