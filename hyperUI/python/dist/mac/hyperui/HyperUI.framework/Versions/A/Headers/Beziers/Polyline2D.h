#pragma once

#define TemplatePointVector		vector < POINT_TYPE >
/*****************************************************************************/
template < class POINT_TYPE >
class HYPERUI_API TPolyline2D : public vector < POINT_TYPE > 
{
public:
	TPolyline2D();
	virtual ~TPolyline2D();

public:
	// This section contains operations on this method
	void translate(FLOAT_TYPE fX, FLOAT_TYPE fY);
	void scale(FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY);

	// Rotates around a specified pivot.
	void rotate(FLOAT_TYPE fAngle, FLOAT_TYPE fPivotX = 0, FLOAT_TYPE fPivotY = 0, int iStartIndex = 0, int iEndIndex = -1);

	// Subdivides each segment so that within segments, each
	// point is at most this far apart.
	void subdivide(FLOAT_TYPE fMaxDistance);

	void transform(const SMatrix2D& smMatrix);
	void transform(const CombinedTransform& smCombTransform);

	// Bends the line by the angle specified around the center
	// (center is 0 to 1). Inserts a new point at center if necessary.
	void bend(FLOAT_TYPE fCenter, FLOAT_TYPE fAngle);

	void generateAtOrigin(FLOAT_TYPE fLength, FLOAT_TYPE fPointSpacing = -1);
	void generateNoiseLineAtOrigin(FLOAT_TYPE fLength, FLOAT_TYPE fMinPointSpacing, PerlinNoiseGenerator& rNoiseGenerator, FLOAT_TYPE fNoiseTime);
	void generateRectangle(const SRect2D& srRect);

	// Generate a possibly unrolled spiral that start at the origin.
	void generateSpiralAtOrigin(FLOAT_TYPE fStartRadius, FLOAT_TYPE fRollProgress, FLOAT_TYPE fLength, FLOAT_TYPE fPointSpacing = -1);

	FLOAT_TYPE computeLength(void);

	// Blends this line with the other line. Needs the same number of points.
	// If fAmount == 0, the result is this line. If fAmount == 1, the result
	// is the other line.
	void blend(TemplatePointVector& vecOther, FLOAT_TYPE fAmount);

	// Computes cumulative distances to each point. Returns total length.
	FLOAT_TYPE computePointDistances(TFloatVector& vecDistOut);

	// Compute the curvature (angle from x axis) for the given point in the vector
	FLOAT_TYPE computeAngleAtPoint(int iIndex, SVector2D* pOptNormVecOut);

	// Returns a distance from the start of the curve if the point is on it, -1 if not on it.
	FLOAT_TYPE findPointOn(SVector2D& svPoint, int& iPointPosOut, FLOAT_TYPE fTolerance = FLOAT_EPSILON, SVector2D* svClosestPointOut = NULL);

	// Computes evenly spaced points on the path. The distance is measured along the path,
	// and the points are guaranteed to be on it, though connecting them will results in a
	// coarse approximation of the path.
	void computeSpacedPointsOnPath(FLOAT_TYPE fInitOffset, FLOAT_TYPE fSpacing, TemplatePointVector& vecResOut) const;
	void computeSpacedPointsOnPath(FLOAT_TYPE fInitOffset, TFloatVector& vecSpacings, TemplatePointVector& vecResOut, TPointVector* optTangentsOut, bool bSpacingArePerPoint) const;

	void removeCoincident(FLOAT_TYPE fDistance);
	void removeCollinear(FLOAT_TYPE fDegAngleTolerance, TemplatePointVector& vecResOut) const;

	bool doesIntersect(const TemplatePointVector& rOther, SVector2D& svPointOut) const;

	// Trim the vector to be a certain length (fDistance) from the front (i.e. cutoff the end).
	void trimFromFront(FLOAT_TYPE fDistance);
	// Trim the vector to be a certain length (fDistance) from the end (i.e. cutoff the front).
	void trimFromEnd(FLOAT_TYPE fDistance);

	void getPointByDistanceFromStart(FLOAT_TYPE fLength, SVector2D& svPointOut);

	void computeBBox(SRect2D& srOut) const;

	SVertexInfo* triangulateAsPolygon(int &iNumVertsOut) const;
	HyperCore::OrientationType getPolygonOrientation() const;

#ifdef HAVE_POLYBOOL
	POLYBOOLEAN::PLINE2* convertToPBLine() const;
	void setFromPBLine(const POLYBOOLEAN::PLINE2* pLine);
#endif
	inline bool getIsHole() const { return myIsHole; }
	inline void setIsHole(bool bValue) { myIsHole = bValue; }

	void copyFrom(const TPolyline2D& rOther);

	void deform(const SBezierCurve* pSourceCurve, const SBezierCurve* pTargetCurve, TemplatePointVector& rDeformedOut) const;

public:

	// General useful methods

	// Tests whether the given distance is at or beyond the curve end.
	// Works for both positive and negative fByDistance values. Returns true if reached end, false otherwise.
	static bool advanceBezierCurvePos(TemplatePointVector& vecPoints, TFloatVector& vecDistances, FLOAT_TYPE fByDistance, int& iCurvePointPosInOut, FLOAT_TYPE& fDistTraveledInOut, SVector2D& svNewPosOut);

	// Given a distance on the path and the point index, interpolates the given attribute (whether it is position, tangent, or anything else) exactly.
	// Can be made tepmlated if needed.
	static void interpolatePointAttribute(TFloatVector &vecDistances, int iCurvePointPos, FLOAT_TYPE fCurveDistPos, FLOAT_TYPE fMotionDir, TemplatePointVector& vecAttrsToInterp, SVector2D& svResOut);

public:
	// This section contains static operations to help us with
	// existing TemplatePointVector objects.
	static void translate(TemplatePointVector& vecPoints, FLOAT_TYPE fX, FLOAT_TYPE fY);
	static void scale(TemplatePointVector& vecPoints, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY);
	static void rotate(TemplatePointVector& vecPoints, FLOAT_TYPE fAngle, FLOAT_TYPE fPivotX = 0, FLOAT_TYPE fPivotY = 0, int iStartIndex = 0, int iEndIndex = -1);

	static void subdivide(TemplatePointVector& vecPoints, FLOAT_TYPE fMaxDistance);

	static void transform(TemplatePointVector& vecPoints, const SMatrix2D& smMatrix);
	static void transform(TemplatePointVector& vecPoints, const CombinedTransform& smCombTransform);

	static void bend(TemplatePointVector& vecPoints, FLOAT_TYPE fCenter, FLOAT_TYPE fAngle);

	static void generateAtOrigin(TemplatePointVector& vecPoints, FLOAT_TYPE fLength, FLOAT_TYPE fPointSpacing);

	static FLOAT_TYPE computeLength(TemplatePointVector& vecPoints);
	

	static void generateSpiralAtOrigin(TemplatePointVector& vecPoints, FLOAT_TYPE fStartRadius, FLOAT_TYPE fRollProgress, FLOAT_TYPE fLength, FLOAT_TYPE fPointSpacing = -1);
	static void blend(TemplatePointVector& vecPoints, TemplatePointVector& vecOther, FLOAT_TYPE fAmount);
	static void generateNoiseLineAtOrigin(TemplatePointVector& vecPoints, FLOAT_TYPE fLength, FLOAT_TYPE fMinPointSpacing, PerlinNoiseGenerator& rNoiseGenerator, FLOAT_TYPE fNoiseTime);
	static void generateRectangle(TemplatePointVector& vecPoints, const SRect2D& srRect);

	static FLOAT_TYPE computePointDistances(TemplatePointVector& vecPoints, TFloatVector& vecDistOut);

	static FLOAT_TYPE computeAngleAtPoint(TemplatePointVector& vecPoints, int iIndex, SVector2D* pOptNormVecOut);

	static void computeSpacedPointsOnPath(const TemplatePointVector& vecPoints, FLOAT_TYPE fInitOffset, FLOAT_TYPE fSpacing, TemplatePointVector& vecResOut);
	static void computeSpacedPointsOnPath(const TemplatePointVector& vecPoints, FLOAT_TYPE fInitOffset, TFloatVector& vecSpacings, TemplatePointVector& vecResOut, TPointVector* optTangentsOut, bool bSpacingArePerPoint);
	// fTangentSpaceMult - applied fully to 90 deg facing point sizes.
	static FLOAT_TYPE computeSpacedPointsOnPath(const TemplatePointVector& vecPoints, bool bFlipTangent, FLOAT_TYPE fInitOffset, TFloatVector& vecSpacings, TFloatVector* pTangentsAsAngles, FLOAT_TYPE fTangentSpaceMult, bool bStopAtVectorEnd, TemplatePointVector& vecResOut, TPointVector* optTangentsOut, bool bForceAppendLastPoint, bool bSpacingArePerPoint);
	static FLOAT_TYPE computeSpacedPointsOnMultiplePaths(vector< TPolyline2D<POINT_TYPE> >& vecPaths, bool bFlipTangent, FLOAT_TYPE fInitOffset, TFloatVector& vecSpacings, TFloatVector* pTangentsAsAngles, FLOAT_TYPE fTangentSpaceMult, bool bStopAtVectorEnd, TemplatePointVector& vecResOut, TPointVector* optTangentsOut, bool bForceAppendLastPoint);

	static void removeCoincident(TemplatePointVector& vecPoints, FLOAT_TYPE fDistance);
	static void removeCollinear(const TemplatePointVector& vecPoints, FLOAT_TYPE fDegAngleTolerance, TemplatePointVector& vecResOut);

	static FLOAT_TYPE findPointOn(TemplatePointVector& vecPoints, SVector2D& svPoint, int& iPointPosOut, FLOAT_TYPE fTolerance = FLOAT_EPSILON, SVector2D* svClosestPointOut = NULL);

	static bool doIntersect(const TemplatePointVector& rFirst,  const TemplatePointVector& rSecond, bool bExcludeExactMatches, SVector2D& svPointOut);
	static bool doesContain(const TemplatePointVector& rFirst,  const TemplatePointVector& rSecond, bool bAllowOverlap = false);

	static bool doesContain(const TemplatePointVector& rShape, const SVector2D& svPoint, bool bAllowOverlap = false);

	static void trimFromFront(TemplatePointVector& vecPoints, FLOAT_TYPE fDistance);
	static void trimFromEnd(TemplatePointVector& vecPoints, FLOAT_TYPE fDistance);

	static void getPointByDistanceFromStart(TemplatePointVector& vecPoints, FLOAT_TYPE fLength, SVector2D& svPointOut);

	static void computeBBox(const TemplatePointVector& vecPoints, SRect2D& srOut);
	static SVertexInfo* triangulateAsPolygon(const TemplatePointVector& vecPoints, int &iNumVertsOut);
	static HyperCore::OrientationType getPolygonOrientation(const TemplatePointVector& vecPoints);

#ifdef HAVE_POLYBOOL
	static POLYBOOLEAN::PLINE2* convertToPBLine(const TemplatePointVector& vecPoints, bool bIsHole);
#endif

	static void deform(const TemplatePointVector& vecPoints, const SBezierCurve* pSourceCurve, const SBezierCurve* pTargetCurve, TemplatePointVector& rDeformedOut);

private:

	static void generateNoiseRecursive(TemplatePointVector& vecPoints, FLOAT_TYPE fLength, SVector2D& svStart, SVector2D& svEnd, FLOAT_TYPE fMinPointSpacing, PerlinNoiseGenerator& rNoiseGenerator, FLOAT_TYPE fNoiseTime);

private:

	// Note that because of the, umm, unoptimal way we did this where a polyline
	// may be either an instance of this class or just a vector of points, we 
	// can only support holes for the former... or any other flags, for that matter.
	bool myIsHole;
};
/*****************************************************************************/
typedef TPolyline2D< SVector2D > Polyline2D;
typedef TPolyline2D< SBrushPoint > BrushPolyline2D;
typedef TPolyline2D< SColorPoint > ColorPolyline2D;
typedef TPolyline2D< SBezierPoint > BezierPolyline2D;

typedef vector < Polyline2D > TLineVector;
typedef vector < BrushPolyline2D > TBrushLineVector;
typedef vector < SColorPoint > TColorLineVector;