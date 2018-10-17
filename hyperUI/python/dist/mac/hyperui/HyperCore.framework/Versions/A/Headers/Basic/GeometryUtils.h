#pragma once

struct SVector2D;
/*****************************************************************************/
class GeometryUtils
{
public:
	HYPERCORE_API static bool isPointInTriangle(const SVector2D& svPoint1, const SVector2D& svPoint2, const SVector2D& svPoint3, const SVector2D& svTestPoint);
	HYPERCORE_API static OrientationType getTriangleOrientation(const SVector2D& svPoint1, const SVector2D& svPoint2, const SVector2D& svPoint3);
	HYPERCORE_API static FLOAT_TYPE segmentSegmentIntersection(const SVector2D &pLine1Start, const SVector2D &pLine1End, const SVector2D &pLine2Start, const SVector2D &pLine2End, SVector2D* svPointOut, FLOAT_TYPE fTolerance = FLOAT_EPSILON);
	HYPERCORE_API static bool doSegmentsOverlap(const SVector2D &pLine1Start, const SVector2D &pLine1End, const SVector2D &pLine2Start, const SVector2D &pLine2End, FLOAT_TYPE fTolerance = FLOAT_EPSILON);

};
/*****************************************************************************/