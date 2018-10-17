#include "stdafx.h"

const int g_iMaxRecursionDepth = 64;
const int g_iBezierDegree = 3;
const int g_iSolutionDegree = 5;

#define SAVE_BEZIER_STARTX				"bezcvStartX"
#define SAVE_BEZIER_STARTY				"bezcvStartY"
#define SAVE_BEZIER_ENDX				"bezcvEndX"
#define SAVE_BEZIER_ENDY				"bezcvEndY"
#define SAVE_BEZIER_CONTROL1X			"bezcvCtrl1X"
#define SAVE_BEZIER_CONTROL1Y			"bezcvCtrl1Y"
#define SAVE_BEZIER_CONTROL2X			"bezcvCtrl2X"
#define SAVE_BEZIER_CONTROL2Y			"bezcvCtrl2Y"
#define SAVE_BEZIER_START_TYPE			"bezcvStrType"
#define SAVE_BEZIER_END_TYPE			"bezcvEndType"

namespace HyperUI
{
/*****************************************************************************/
// Bezier class
/*****************************************************************************/
SBezierSegment::SBezierSegment()
{
	myStartPointType = myEndPointType = BezierPointCurveTied;
	//myIsLinear = false;
	myEdgeId.setFromNumeric(0);
	myParent = NULL;
}
/*****************************************************************************/
SBezierSegment::~SBezierSegment()
{
	
}
/*****************************************************************************/
void SBezierSegment::setStart(FLOAT_TYPE dx, FLOAT_TYPE dy)
{
	myStart.x = dx;
	myStart.y = dy;
}
/*****************************************************************************/
void SBezierSegment::setEnd(FLOAT_TYPE dx, FLOAT_TYPE dy)
{
	myEnd.x = dx;
	myEnd.y = dy;
}
/*****************************************************************************/
void SBezierSegment::setControl1(FLOAT_TYPE dx, FLOAT_TYPE dy)
{
	myControl1.x = dx;
	myControl1.y = dy;
}
/*****************************************************************************/
void SBezierSegment::setControl2(FLOAT_TYPE dx, FLOAT_TYPE dy)
{
	myControl2.x = dx;
	myControl2.y = dy;
}
/*****************************************************************************/
void SBezierSegment::makeLinear()
{
	myStartPointType = myEndPointType = BezierPointCurveSharp;
	ensureIsLinear();
}
/********************************************************************************************
void Bezier::setIsLinear(bool bValue)
{
	myIsLinear = bValue;
}
/*****************************************************************************/
void SBezierSegment::getTangent(FLOAT_TYPE dInterpPos, SVector2D& svTangentOut) const
{
	if(myEndPointType == BezierPointCurveSharp && myStartPointType == BezierPointCurveSharp)
	{
		SVector2D svDir, svRes;
		svDir = myEnd - myStart;
		svDir.normalize();
		svTangentOut = svDir;
		return;
	}	

	svTangentOut = myStart*(1.0 - dInterpPos)*(1.0 - dInterpPos)*-3.0 +
		myControl1*(3.0*(1.0 - dInterpPos)*(1.0 - dInterpPos) - 6.0*(1.0 - dInterpPos)*dInterpPos) +
		myControl2*(6.0*(1.0 - dInterpPos)*dInterpPos - 3*dInterpPos*dInterpPos) +
		myEnd*dInterpPos*dInterpPos*3.0;
	svTangentOut.normalize();

/*
	// We're interested only in the right tangent (going forward).
	SVector2D svPt1, svPt2, svPt3;
	SVector2D svA, svB;
	svPt1 = myStart*(1.0-dInterpPos) + myControl1*dInterpPos;
	svPt2 = myControl1*(1.0-dInterpPos) + myControl2*dInterpPos;
	svPt3 = myControl2*(1.0-dInterpPos) + myEnd*dInterpPos;

	//svA = (1.0-dT)*svPt1 + dT*svPt2;
	svB = svPt2*(1.0-dInterpPos) + svPt3*dInterpPos;

	//	svLeftControlPoint = svA;
	//	svRightControlPoint = svB;	

	svTangentOut = svB;
	svTangentOut.normalize();
	*/
}
/*****************************************************************************/
void SBezierSegment::movePointBy(BezierPointType ePointType, FLOAT_TYPE fX, FLOAT_TYPE fY, bool bAllowParentCallback, bool bIgnoreTiedControls)
{
	SVector2D svNewPos;
	this->getPointByType(ePointType, svNewPos);
	svNewPos.x += fX;
	svNewPos.y += fY;
	this->setPointByType(ePointType, svNewPos);

	// Move our control point with us as well
	if(ePointType == BezierPointStart || ePointType == BezierPointEnd)
	{
		BezierPointType eRelatedControlType = ePointType == BezierPointStart ? BezierPointControl1 : BezierPointControl2;

		this->getPointByType(eRelatedControlType, svNewPos);
		svNewPos.x += fX;
		svNewPos.y += fY;
		this->setPointByType(eRelatedControlType, svNewPos);
	}

	ensureIsLinear();

	if(bAllowParentCallback && myParent) //  && (ePointType == BezierPointStart || ePointType == BezierPointEnd))
		myParent->onSegmentPointChanged(this, ePointType, fX, fY, bIgnoreTiedControls);
}
/*****************************************************************************/
FLOAT_TYPE SBezierSegment::computeDistancesOnPiecewiseLinear(TPointVector& rRawPoints) const
{
	FLOAT_TYPE dTotalLength = 0;
	
	int iCurrPoint , iNumPoints = rRawPoints.size();
	/*
	if(pDistancesOut)
	{
		pDistancesOut->reserve(iNumPoints);
		pDistancesOut->push_back(dTotalLength);
	} */
	for(iCurrPoint = 1; iCurrPoint < iNumPoints; iCurrPoint++)
	{
		dTotalLength += (rRawPoints[iCurrPoint] - rRawPoints[iCurrPoint-1]).length();
//		if(pDistancesOut)
//			pDistancesOut->push_back(dTotalLength);
	}
	
	return dTotalLength;
}	
/*****************************************************************************/
SBezierSegment* SBezierSegment::cloneSelf(FLOAT_TYPE fPerfScalingFactor) const
{
	SBezierSegment* pNew = new SBezierSegment;
	pNew->copyFrom(*this, fPerfScalingFactor);
	pNew->myParent = NULL;
	return pNew;
}
/*****************************************************************************/
void SBezierSegment::copyFrom(const SBezierSegment& rOther, FLOAT_TYPE fPerfScalingFactor)
{
	myStart = rOther.myStart;
	myEnd = rOther.myEnd;

	myControl1 = rOther.myControl1;
	myControl2 = rOther.myControl2;	
	myStartPointType = rOther.myStartPointType;
	myEndPointType = rOther.myEndPointType;

	myEdgeId = rOther.myEdgeId;

	if(fPerfScalingFactor != 1.0)
	{
		myStart *= fPerfScalingFactor;
		myEnd *= fPerfScalingFactor;
		myControl1 *= fPerfScalingFactor;
		myControl2 *= fPerfScalingFactor;
	}

	// Do not touch the parent. It doesn't change.
	// myParent;

}
/*****************************************************************************/
void SBezierSegment::getBBox(SRect2D& srRectOut) const
{
	srRectOut.x = myStart.x;
	srRectOut.y = myStart.y;
	srRectOut.w = srRectOut.h = 0;

	// And include the rest:
	srRectOut.includePoint(myEnd.x, myEnd.y);
	srRectOut.includePoint(myControl1.x, myControl1.y);
	srRectOut.includePoint(myControl2.x, myControl2.y);
}
/*****************************************************************************/
void SBezierSegment::resample(FLOAT_TYPE dError, FLOAT_TYPE dMaxPointSpacing, bool bAddFirstPoint, TPointVector& rPointsOut, TPointVector* optTangentsOut)
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
void SBezierSegment::resampleSingleBezierRecursive(FLOAT_TYPE dTStart, FLOAT_TYPE dTEnd, FLOAT_TYPE dErrorSq, FLOAT_TYPE dMaxPointSpacingSq, TPointVector& rPointsOut, TPointVector* optTangentsOut) const
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
FLOAT_TYPE SBezierSegment::getLength(FLOAT_TYPE dTolerance, FLOAT_TYPE dMaxPointSpacing) const
{
	if(myEndPointType == BezierPointCurveSharp && myStartPointType == BezierPointCurveSharp)
	{
		SVector2D svDir, svRes;
		svDir = myEnd - myStart;
		return svDir.length();
	}
	
	TPointVector vecTempPoints;
	SVector2D svTemp;
	evaluate(0, svTemp);
	vecTempPoints.push_back(svTemp);
	resampleSingleBezierRecursive(0, 1, dTolerance*dTolerance, dMaxPointSpacing*dMaxPointSpacing, vecTempPoints, NULL);
	return computeDistancesOnPiecewiseLinear(vecTempPoints);
}
/*****************************************************************************/
void SBezierSegment::getPointByType(BezierPointType ePointType, SVector2D& svOut) const
{
	if(ePointType == BezierPointStart)
		svOut = myStart;
	else if(ePointType == BezierPointControl1)
		svOut = myControl1;
	else if(ePointType == BezierPointControl2)
		svOut = myControl2;
	else if(ePointType == BezierPointEnd)
		svOut = myEnd;
	else
	{
		_ASSERT(0);
		svOut.set(0, 0);
	}
}
/*****************************************************************************/
void SBezierSegment::setPointByType(BezierPointType ePointType, const SVector2D& svIn)
{
	if(ePointType == BezierPointStart)
		myStart = svIn;
	else if(ePointType == BezierPointControl1)
		myControl1 = svIn;
	else if(ePointType == BezierPointControl2)
		myControl2 = svIn;
	else if(ePointType == BezierPointEnd)
		myEnd = svIn;
	ELSE_ASSERT;
}
/*****************************************************************************/
void SBezierSegment::evaluate(FLOAT_TYPE dInterpPos, SVector2D& svOut) const
{
	if(myEndPointType == BezierPointCurveSharp && myStartPointType == BezierPointCurveSharp)
	{
		// Linear interp.
		SVector2D svDir;
		svDir = myEnd - myStart;
		FLOAT_TYPE dLen = svDir.normalize();
		svOut = myStart + svDir*(dLen*dInterpPos);
		return;
	}
	
	// TODO: Replace with 3rd degree eval code.
	const int iDegree = 3;
	
	SVector2D svVTemp[4];
	int iCurrDegree, iInnerDegree;
	
	svVTemp[0] = myStart;
	svVTemp[1] = myControl1;
	svVTemp[2] = myControl2;
	svVTemp[3] = myEnd;
	
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
void SBezierSegment::evaluate(FLOAT_TYPE dInterpPos, SVector2D* svPoints, SVector2D& svOut)
{
	const int iDegree = 3;
	int iCurrDegree, iInnerDegree;
	for(iCurrDegree = 1; iCurrDegree <= iDegree; iCurrDegree++)	
	{
		for(iInnerDegree = 0; iInnerDegree <= iDegree - iCurrDegree; iInnerDegree++)
		{
			svPoints[iInnerDegree] =  svPoints[iInnerDegree]*(1.0 - dInterpPos) + svPoints[iInnerDegree + 1]*dInterpPos;
		}
	}

	svOut = svPoints[0];	
}
/*****************************************************************************/
void SBezierSegment::ensureIsLinear()
{
	SVector2D svTemp;
	//svTemp = (myStart + myEnd)*0.5;

	if(myStartPointType == BezierPointCurveSharp)
	{
		svTemp = myStart*(2.0/3.0) + myEnd*(1.0/3.0);
		myControl1 = svTemp;
	}

	if(myEndPointType == BezierPointCurveSharp)
	{
		svTemp = myStart*(1.0/3.0) + myEnd*(2.0/3.0);
		myControl2 = svTemp;
	}	

}
/*****************************************************************************/
bool SBezierSegment::findVertexClosestTo(const SVector2D& svPoint, FLOAT_TYPE fOptMaxRadius, SVector2D& svResPointOut, BezierPointType& ePointTypeOut) const
{
	bool bIsRadiusValid = (fOptMaxRadius != FLOAT_TYPE_MAX && fOptMaxRadius > 0);
	FLOAT_TYPE fRadSq = fOptMaxRadius*fOptMaxRadius;

	// Simply go over our start and end. Note that we ignore controls.
	FLOAT_TYPE fCurrDistStart, fCurrDistEnd;
	
	fCurrDistStart = (svPoint - myStart).lengthSquared();
	fCurrDistEnd = (svPoint - myEnd).lengthSquared();

	if(fCurrDistEnd < fCurrDistStart)
	{
		svResPointOut = myEnd;
		ePointTypeOut = BezierPointEnd;

		return !bIsRadiusValid || fCurrDistEnd <= fRadSq;
	}
	else
	{
		svResPointOut = myStart;
		ePointTypeOut = BezierPointStart;

		return !bIsRadiusValid || fCurrDistStart <= fRadSq;
	}
}
/*****************************************************************************/
void SBezierSegment::findPointClosestTo(const SVector2D& svPoint, SVector2D& svPointOut, FLOAT_TYPE* fOptParmDistOut) const
{
	FLOAT_TYPE fParametricValue = findPositionClosestTo(svPoint);
	if(fOptParmDistOut)
		*fOptParmDistOut = fParametricValue;
	evaluate(fParametricValue, svPointOut);
}
/*****************************************************************************/
FLOAT_TYPE SBezierSegment::findPositionClosestTo(const SVector2D& svPoint) const
{
	// Find the point on the curve closest to the given point.
	// Return dT in the range [0,1].

	SVector2D	w[g_iSolutionDegree + 1];			// Ctl pts for 5th-degree eqn
	FLOAT_TYPE	t_candidate[g_iSolutionDegree];		// Possible roots
	int 	n_solutions;						// Number of roots found
	FLOAT_TYPE	t;									// Parameter value of closest pt

	//  Convert problem to 5th-degree Bezier form
	convertToBezierForm(svPoint, w);

	// Find all possible roots of 5th-degree equation
	n_solutions = findRootsInternal(w, g_iSolutionDegree, t_candidate, 0);

	// Compare distances of P to all candidates, and to t=0, and t=1
	FLOAT_TYPE 	dist, new_dist;
	SVector2D p;
	//SVector2D v;
	int		i;

	// Check distance to beginning of curve, where t = 0
	//dist = V2SquaredLength(V2Sub(&P, &V[0], &v));
	dist = (svPoint - myStart).lengthSquared();
	t = 0.0;

	// Find distances for candidate points
	for (i = 0; i < n_solutions; i++) 
	{
		//		p = Bezier(V, g_iBezierDegree, t_candidate[i],	(Point2 *)NULL, (Point2 *)NULL);
		this->evaluate(t_candidate[i], p);

		//new_dist = V2SquaredLength(V2Sub(&P, &p, &v));
		new_dist = (svPoint - p).lengthSquared();
		if (new_dist < dist) 
		{
			dist = new_dist;
			t = t_candidate[i];
		}
	}

	// Finally, look at distance to end point, where t = 1.0
	//new_dist = V2SquaredLength(V2Sub(&P, &V[DEGREE], &v));
	new_dist = (svPoint - myEnd).lengthSquared();
	if (new_dist < dist) 
	{
		dist = new_dist;
		t = 1.0;
	}

	//  Return the point on the curve at parameter value t
	return t;
}
/*****************************************************************************/
void SBezierSegment::convertToBezierForm(const SVector2D& svPoint, SVector2D* rEquationCtlPointsOut) const
{
	int 	i, j, k, m, n, ub, lb;	
	int 	row, column;					// Table indices		
	SVector2D 	c[g_iBezierDegree+1];		// V(i)'s - P			
	SVector2D 	d[g_iBezierDegree];			// V(i+1) - V(i)		
	FLOAT_TYPE 	cdTable[3][4];					// Dot product of c, d	
	static FLOAT_TYPE z[3][4] =					// Precomputed "z" for cubics	
	{				
		{1.0f, 0.6f, 0.3f, 0.1f},
		{0.4f, 0.6f, 0.6f, 0.4f},
		{0.1f, 0.3f, 0.6f, 1.0f},
	};


	// Determine the c's -- these are vectors created by subtracting
	// point P from each of the control points				
	//	for (i = 0; i <= g_iBezierDegree; i++) 
	//	{
	//		V2Sub(&V[i], &P, &c[i]);
	//	}

	c[0] = myStart - svPoint;
	c[1] = myControl1 - svPoint;
	c[2] = myControl2 - svPoint;
	c[3] = myEnd - svPoint;

	// Determine the d's -- these are vectors created by subtracting
	// each control point from the next					
	//	for (i = 0; i <= DEGREE - 1; i++) { 
	//		d[i] = V2ScaleII(V2Sub(&V[i+1], &V[i], &d[i]), 3.0);
	//	}
	//d[i] = V2ScaleII(V2Sub(&V[i+1], &V[i], &d[i]), 3.0);
	d[0] = (myControl1 - myStart)*3.0;
	d[1] = (myControl2 - myControl1)*3.0;
	d[2] = (myEnd - myControl2)*3.0;

	// Create the c,d table -- this is a table of dot products of the 
	// c's and d's							
	for (row = 0; row <= g_iBezierDegree - 1; row++) 
	{
		for (column = 0; column <= g_iBezierDegree; column++) 
		{
			//cdTable[row][column] = V2Dot(&d[row], &c[column]);
			cdTable[row][column] = d[row].dot(c[column]);
		}
	}

	// Now, apply the z's to the dot products, on the skew diagonal
	// Also, set up the x-values, making these "points"
	//w = (Point2 *)malloc((unsigned)(W_DEGREE+1) * sizeof(Point2));
	for (i = 0; i <= g_iSolutionDegree; i++) 
	{
		rEquationCtlPointsOut[i].y = 0.0;
		rEquationCtlPointsOut[i].x = (FLOAT_TYPE)(i) / g_iSolutionDegree;
	}

	n = g_iBezierDegree;
	m = g_iBezierDegree-1;
	for (k = 0; k <= n + m; k++) 
	{
		lb = max(0, k - m);
		ub = min(k, n);
		for (i = lb; i <= ub; i++) {
			j = k - i;
			rEquationCtlPointsOut[i+j].y += cdTable[j][i] * z[j][i];
		}
	}

	//	return (w);
}
/*****************************************************************************/
int SBezierSegment::computeCrossingCount(SVector2D* rPoints, int degree) const
{
	int 	i;	
	int 	n_crossings = 0;	//  Number of zero-crossings
	int		sign, old_sign;		//  Sign of coefficients

	sign = old_sign = HyperCore::getSign(rPoints[0].y);
	for (i = 1; i <= degree; i++) 
	{
		sign = HyperCore::getSign(rPoints[i].y);
		if (sign != old_sign) n_crossings++;
		old_sign = sign;
	}
	return n_crossings;
}
/*****************************************************************************/
int SBezierSegment::findRootsInternal(SVector2D* rEquationCtlPoints, int degree, FLOAT_TYPE* t, int depth) const
{  
	int 	i;
	SVector2D Left[g_iSolutionDegree+1];	// New left and right
	SVector2D Right[g_iSolutionDegree+1];	// control polygons
	int left_count,right_count;				// Solution count from children
	FLOAT_TYPE left_t[g_iSolutionDegree+1];		// Solutions from kids
	FLOAT_TYPE right_t[g_iSolutionDegree+1];

	memset(t, 0, sizeof(FLOAT_TYPE)*g_iSolutionDegree);
	memset(left_t, 0, sizeof(FLOAT_TYPE)*g_iSolutionDegree + 1);
	memset(right_t, 0, sizeof(FLOAT_TYPE)*g_iSolutionDegree + 1);

	switch(computeCrossingCount(rEquationCtlPoints, degree)) 
	{
	case 0 : return 0; // No solutions here
	case 1 : 
		{	
			// Unique solution
			// Stop recursion when the tree is deep enough
			// if deep enough, return 1 solution at midpoint
			if (depth >= g_iMaxRecursionDepth) 
			{
				t[0] = (rEquationCtlPoints[0].x + rEquationCtlPoints[g_iSolutionDegree].x) / 2.0;
				return 1;
			}
			if(isControlPolygonFlatEnough(rEquationCtlPoints, degree)) 
			{
				t[0] = computeXIntercept(rEquationCtlPoints, degree);
				return 1;
			}
			break;
		}
	}

	// Otherwise, solve recursively after
	// subdividing control polygon
	SVector2D svDummy;
	evaluateBezierInternal(rEquationCtlPoints, degree, 0.5, Left, Right, svDummy);

	left_count  = findRootsInternal(Left,  degree, left_t, depth+1);
	right_count = findRootsInternal(Right, degree, right_t, depth+1);

	// Gather solutions together
	for (i = 0; i < left_count; i++) 
	{
		t[i] = left_t[i];
	}
	for (i = 0; i < right_count; i++) 
	{
		t[i+left_count] = right_t[i];
	}

	// Send back total number of solutions
	return (left_count+right_count);
}
/*****************************************************************************/
// Check if the control polygon of a Bezier curve is flat enough
// for recursive subdivision to bottom out.
int SBezierSegment::isControlPolygonFlatEnough(SVector2D* rPoints, int degree) const
{
	int 	i;					// Index variable		
	//FLOAT_TYPE	*distance;			// Distances from pts to line	
	FLOAT_TYPE distance[g_iSolutionDegree+1];
	FLOAT_TYPE	max_distance_above;	// maximum of these		
	FLOAT_TYPE 	max_distance_below;
	FLOAT_TYPE 	error;				// Precision of root		
	FLOAT_TYPE	intercept_1,
		intercept_2,
		left_intercept,
		right_intercept;
	FLOAT_TYPE	a, b, c;			// Coefficients of implicit
	// eqn for line from V[0]-V[deg]

	// Find the  perpendicular distance
	// from each interior control point to
	// line connecting V[0] and V[degree]
	_ASSERT(degree == g_iSolutionDegree);
	//distance = (FLOAT_TYPE *)malloc((unsigned)(degree + 1) * sizeof(FLOAT_TYPE));

	FLOAT_TYPE	abSquared;

	// Derive the implicit equation for line connecting first 
	//  and last control points 
	a = rPoints[0].y - rPoints[degree].y;
	b = rPoints[degree].x - rPoints[0].x;
	c = rPoints[0].x * rPoints[degree].y - rPoints[degree].x * rPoints[0].y;

	abSquared = (a * a) + (b * b);

	for (i = 1; i < degree; i++) 
	{
		// Compute distance from each of the points to that line
		distance[i] = a * rPoints[i].x + b * rPoints[i].y + c;
		if (distance[i] > 0.0) 
		{
			distance[i] = (distance[i] * distance[i]) / abSquared;
		}
		if (distance[i] < 0.0) 
		{
			distance[i] = -((distance[i] * distance[i]) / abSquared);
		}
	}


	// Find the largest distance
	max_distance_above = 0.0;
	max_distance_below = 0.0;
	for (i = 1; i < degree; i++) 
	{
		if (distance[i] < 0.0) {
			max_distance_below = min(max_distance_below, distance[i]);
		}
		if (distance[i] > 0.0) 
		{
			max_distance_above = max(max_distance_above, distance[i]);
		}
	}
	//free((char *)distance);

	FLOAT_TYPE	det, dInv;
	FLOAT_TYPE	a1, b1, c1, a2, b2, c2;

	//  Implicit equation for zero line
	a1 = 0.0;
	b1 = 1.0;
	c1 = 0.0;

	//  Implicit equation for "above" line
	a2 = a;
	b2 = b;
	c2 = c + max_distance_above;

	det = a1 * b2 - a2 * b1;
	dInv = 1.0/det;

	intercept_1 = (b1 * c2 - b2 * c1) * dInv;

	//  Implicit equation for "below" line
	a2 = a;
	b2 = b;
	c2 = c + max_distance_below;

	det = a1 * b2 - a2 * b1;
	dInv = 1.0/det;

	intercept_2 = (b1 * c2 - b2 * c1) * dInv;

	// Compute intercepts of bounding box
	left_intercept = min(intercept_1, intercept_2);
	right_intercept = max(intercept_1, intercept_2);

	error = 0.5 * (right_intercept-left_intercept);    
	if (error < FLOAT_EPSILON) 
		return 1;
	else
		return 0;
}
/*****************************************************************************/
FLOAT_TYPE SBezierSegment::computeXIntercept(SVector2D* rPoints, int degree) const
{
	FLOAT_TYPE	XLK, YLK, XNM, YNM, XMK, YMK;
	FLOAT_TYPE	det, detInv;
	FLOAT_TYPE	S;//, T;
	FLOAT_TYPE	X;//, Y;

	XLK = 1.0 - 0.0;
	YLK = 0.0 - 0.0;
	XNM = rPoints[degree].x - rPoints[0].x;
	YNM = rPoints[degree].y - rPoints[0].y;
	XMK = rPoints[0].x - 0.0;
	YMK = rPoints[0].y - 0.0;

	det = XNM*YLK - YNM*XLK;
	detInv = 1.0/det;

	S = (XNM*YMK - YNM*XMK) * detInv;
	//  T = (XLK*YMK - YLK*XMK) * detInv;

	X = 0.0 + XLK * S;
	//  Y = 0.0 + YLK * S;

	return X;
}
/*****************************************************************************/
void SBezierSegment::evaluateBezierInternal(SVector2D* svPoints, int degree, FLOAT_TYPE t, SVector2D* Left, SVector2D* Right, SVector2D& svOut) const
{
	int 	i, j;		// Index variables
	SVector2D Vtemp[g_iSolutionDegree+1][g_iSolutionDegree+1];

	// Copy control points
	for (j =0; j <= degree; j++) 
	{
		Vtemp[0][j] = svPoints[j];
	}

	// Triangle computation
	for (i = 1; i <= degree; i++) 
	{	
		for (j =0 ; j <= degree - i; j++) 
		{
			Vtemp[i][j].x =
				(1.0 - t) * Vtemp[i-1][j].x + t * Vtemp[i-1][j+1].x;
			Vtemp[i][j].y =
				(1.0 - t) * Vtemp[i-1][j].y + t * Vtemp[i-1][j+1].y;
		}
	}

	if (Left != NULL) {
		for (j = 0; j <= degree; j++) {
			Left[j]  = Vtemp[j][0];
		}
	}
	if (Right != NULL) {
		for (j = 0; j <= degree; j++) {
			Right[j] = Vtemp[degree-j][j];
		}
	}
	svOut = (Vtemp[degree][0]);
}
/*****************************************************************************/
void SBezierSegment::setEdgeId(UNIQUEID_TYPE id) 
{ 
	EdgeId idOld = myEdgeId; 
	myEdgeId.setFromNumeric(id);
//	_ASSERT(myParent);
	if(myParent) 
		myParent->onChildEdgeIdChanged(idOld, myEdgeId); 
}
/*****************************************************************************/
void SBezierSegment::transformBy(const SMatrix2D& smTransform)
{
	myStart = smTransform*myStart;
	myControl1 = smTransform*myControl1;
	myControl2 = smTransform*myControl2;
	myEnd = smTransform*myEnd;
}
/*****************************************************************************/
void SBezierSegment::reverse()
{
	SVector2D svTemp = myStart;
	myStart = myEnd;
	myEnd = svTemp;

	svTemp = myControl1;
	myControl1 = myControl2;
	myControl2 = svTemp;
}
/*****************************************************************************/
void SBezierSegment::saveToItem(StringResourceItem& rItemOut) const
{
	rItemOut.setNumProp(SAVE_BEZIER_STARTX, myStart.x);
	rItemOut.setNumProp(SAVE_BEZIER_STARTY, myStart.y);

	rItemOut.setNumProp(SAVE_BEZIER_ENDX, myEnd.x);
	rItemOut.setNumProp(SAVE_BEZIER_ENDY, myEnd.y);

	rItemOut.setNumProp(SAVE_BEZIER_CONTROL1X, myControl1.x);
	rItemOut.setNumProp(SAVE_BEZIER_CONTROL1Y, myControl1.y);

	rItemOut.setNumProp(SAVE_BEZIER_CONTROL2X, myControl2.x);
	rItemOut.setNumProp(SAVE_BEZIER_CONTROL2Y, myControl2.y);

	rItemOut.setNumProp(SAVE_BEZIER_START_TYPE, (int)myStartPointType);
	rItemOut.setNumProp(SAVE_BEZIER_END_TYPE, (int)myEndPointType);

	myEdgeId.saveToItem(rItemOut);
}
/*****************************************************************************/
void SBezierSegment::loadFromItem(const StringResourceItem& rItem)
{
	myStart.x = rItem.getNumProp(SAVE_BEZIER_STARTX);
	myStart.y = rItem.getNumProp(SAVE_BEZIER_STARTY);

	myEnd.x = rItem.getNumProp(SAVE_BEZIER_ENDX);
	myEnd.y = rItem.getNumProp(SAVE_BEZIER_ENDY);

	myControl1.x = rItem.getNumProp(SAVE_BEZIER_CONTROL1X);
	myControl1.y = rItem.getNumProp(SAVE_BEZIER_CONTROL1Y);

	myControl2.x = rItem.getNumProp(SAVE_BEZIER_CONTROL2X);
	myControl2.y = rItem.getNumProp(SAVE_BEZIER_CONTROL2Y);

	myStartPointType = (BezierPointCurveType)(int)rItem.getNumProp(SAVE_BEZIER_START_TYPE);
	myEndPointType = (BezierPointCurveType)(int)rItem.getNumProp(SAVE_BEZIER_END_TYPE);

	myEdgeId.loadFromItem(rItem);
}
/*****************************************************************************/
};