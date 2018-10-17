#pragma once

/********************************************************************************************/
struct HYPERUI_API SBezierPoint
{
public:

	SBezierPoint() { myPointType = BezierPointCurveSharp; }
	inline SVector2D& getPoint() { return myPoint; }
	inline const SVector2D& getPoint() const { return myPoint; }

	inline void interpolateFrom(const SBezierPoint& svPoint1, const SBezierPoint& svPoint2, FLOAT_TYPE fInterpFromPoint1)
	{
		myPoint.interpolateFrom(svPoint1.getPoint(), svPoint2.getPoint(), fInterpFromPoint1);
	}

	inline void setPoint(const SVector2D& svPoint, const SVector2D& svControlPrev, const SVector2D& svControlNext, BezierPointCurveType ePointType, const EdgeId& rEdgeId)
	{
		myPoint = svPoint;
		myControlPrev = svControlPrev;
		myControlNext = svControlNext;
		myPointType = ePointType;
		myEdgeId = rEdgeId;
	}

	inline const EdgeId& getEdgeId() const { return myEdgeId; }
	inline BezierPointCurveType getPointType() const { return myPointType; }
	inline void getControlPrev(SVector2D& svOut) const { svOut = myControlPrev; }
	inline void getControlNext(SVector2D& svOut) const { svOut = myControlNext; }

private:
	SVector2D myPoint;
	SVector2D myControlPrev, myControlNext;
	BezierPointCurveType myPointType;
	EdgeId myEdgeId;
};
typedef vector < SBezierPoint > TBezierPointVector;
/********************************************************************************************/
