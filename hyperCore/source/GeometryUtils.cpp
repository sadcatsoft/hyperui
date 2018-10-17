#include "stdafx.h"

namespace HyperCore
{
/********************************************************************************************/
bool GeometryUtils::isPointInTriangle(const SVector2D& svPoint1, const SVector2D& svPoint2, const SVector2D& svPoint3, const SVector2D& svTestPoint)
{
	// Normal
	SVector2D sideDiff(svPoint2.y - svPoint1.y, -(svPoint2.x - svPoint1.x));
	SVector2D fromPtDiff(svTestPoint.x - svPoint1.x, svTestPoint.y - svPoint1.y);
	sideDiff.normalize();
	FLOAT_TYPE d1 = fromPtDiff.dot(sideDiff);
	if(fabs(d1) < FLOAT_EPSILON)
		d1 = 0.0;

	sideDiff.x = svPoint3.y - svPoint2.y;
	sideDiff.y = -(svPoint3.x - svPoint2.x);
	fromPtDiff.x = svTestPoint.x - svPoint2.x;
	fromPtDiff.y = svTestPoint.y - svPoint2.y;
	sideDiff.normalize();
	FLOAT_TYPE d2 = fromPtDiff.dot(sideDiff);
	if(fabs(d2) < FLOAT_EPSILON)
		d2 = 0.0;

	if(d1 * d2 < 0)
		return false;

	sideDiff.x = svPoint1.y - svPoint3.y;
	sideDiff.y = -(svPoint1.x - svPoint3.x);
	fromPtDiff.x = svTestPoint.x - svPoint3.x;
	fromPtDiff.y = svTestPoint.y - svPoint3.y;
	sideDiff.normalize();
	FLOAT_TYPE d3 = fromPtDiff.dot(sideDiff);
	if(fabs(d3) < FLOAT_EPSILON)
		d3 = 0.0;

	// These  will do an inclusive test of points
	// right on the boundary.
	//    if( (d1 >= 0 && d2 >= 0 && d3 >= 0) ||
	//	(d1 <= 0 && d2 <= 0 && d3 <= 0))

	if( (d1 > 0 && d2 > 0 && d3 > 0) ||
		(d1 < 0 && d2 < 0 && d3 < 0))
		return true;
	else
		return false;
}
/********************************************************************************************/
OrientationType GeometryUtils::getTriangleOrientation(const SVector2D& svPoint1, const SVector2D& svPoint2, const SVector2D& svPoint3)
{
	int i,j;
	FLOAT_TYPE dTotalArea=0.0;

	SVector2D rPoints[3];
	rPoints[0] = svPoint1;
	rPoints[1] = svPoint2;
	rPoints[2] = svPoint3;

	for (i = 0; i <3; i++)
	{
		j = (i+1)%3;
		dTotalArea += rPoints[i].x * rPoints[j].y;
		dTotalArea -= rPoints[i].y * rPoints[j].x;
	}
	if(dTotalArea>=0)
		return OrientationCW;
	else
		return OrientationCCW;
}
/********************************************************************************************/
FLOAT_TYPE GeometryUtils::segmentSegmentIntersection(const SVector2D &pLine1Start, const SVector2D &pLine1End, 
										const SVector2D &pLine2Start, const SVector2D &pLine2End,
										SVector2D* svPointOut, FLOAT_TYPE fTolerance)
{
	// First, find the ray intersection, then see
	SRay2D svRay(pLine1Start);
	svRay.myDir = pLine1End - pLine1Start;

	FLOAT_TYPE fLen = svRay.normalize();
	FLOAT_TYPE fSecondLen, fSecondInter;
	FLOAT_TYPE fRes = svRay.doesIntersectLineSegment(pLine2Start, pLine2End, &fSecondInter, &fSecondLen, fTolerance);
	if(fRes == FLOAT_TYPE_MAX)
		return fRes;

	// Otherwise, check if it's within the second line segment
	if(fSecondInter <= fTolerance || fSecondInter >= fSecondLen - fTolerance)
		return FLOAT_TYPE_MAX;
	if(fRes <= fTolerance || fRes >= fLen - fTolerance)
		return FLOAT_TYPE_MAX;

	if(svPointOut)
		*svPointOut = pLine1Start + svRay.myDir*fRes;

	return fRes;
}
/*****************************************************************************/
bool GeometryUtils::doSegmentsOverlap(const SVector2D &pLine1Start, const SVector2D &pLine1End, const SVector2D &pLine2Start, 
					const SVector2D &pLine2End, FLOAT_TYPE fTolerance)
{
	SVector2D svDir1, svNorm1;

	svDir1 = pLine1End - pLine1Start;
	FLOAT_TYPE fLen = svDir1.normalize();
	if(fLen < fTolerance)
		return false;
	svNorm1.set(svDir1.y, -svDir1.x);

	FLOAT_TYPE fHorDist1, fHorDist2;
	SVector2D svDiff1;

	// The need to be close in temrs of perp distance first.
	svDiff1 = pLine2Start - pLine1Start;
	if(fabs(svDiff1.dot(svNorm1)) > fTolerance)
		return false;
	fHorDist1 = svDiff1.dot(svDir1);

	svDiff1 = pLine2End - pLine1Start;
	if(fabs(svDiff1.dot(svNorm1)) > fTolerance)
		return false;
	fHorDist2 = svDiff1.dot(svDir1);

	if( (fHorDist1 < 0 && fHorDist2 < 0) ||
		(fHorDist1 > fLen && fHorDist2 > fLen))
		return false;
	else
		return true;
}
/********************************************************************************************/
};