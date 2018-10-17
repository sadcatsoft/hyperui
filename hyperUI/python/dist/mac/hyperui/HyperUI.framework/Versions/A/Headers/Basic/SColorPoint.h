#pragma once

/********************************************************************************************/
struct SColorPoint
{
	inline SVector2D& getPoint() { return myPoint; }
	inline const SVector2D& getPoint() const { return myPoint; }

	inline const SColor& getColor() const { return myColor; }

	inline void interpolateFrom(const SColorPoint& svPoint1, const SColorPoint& svPoint2, FLOAT_TYPE fInterpFromPoint1)
	{
		myPoint.interpolateFrom(svPoint1.getPoint(), svPoint2.getPoint(), fInterpFromPoint1);
		myColor.interpolateFrom(svPoint1.getColor(), svPoint2.getColor(), fInterpFromPoint1);
	}

	inline void setPoint(FLOAT_TYPE fX, FLOAT_TYPE fY, const SColor& scolIn) { myPoint.set(fX, fY); myColor = scolIn; }

	inline BezierPointCurveType getPointType() const { return BezierPointCurveSharp; }

private:
	SVector2D myPoint;
	SColor myColor;
};
/********************************************************************************************/
