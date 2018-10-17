#pragma once

/*****************************************************************************/
struct HYPERCORE_API STriangle2D
{
	STriangle2D(const SVector2D& svPoint1, const SVector2D& svPoint2, const SVector2D& svPoint3)
	{
		myPoints[0] = svPoint1;
		myPoints[1] = svPoint2;
		myPoints[2] = svPoint3;

		memset(myU, 0, sizeof(FLOAT_TYPE)*3);
		memset(myV, 0, sizeof(FLOAT_TYPE)*3);
	}

	STriangle2D()
	{
		memset(myU, 0, sizeof(FLOAT_TYPE)*3);
		memset(myV, 0, sizeof(FLOAT_TYPE)*3);
	}

	inline void textureFrom(const SRect2D& srRect)
	{
		int iCurr;
		for(iCurr = 0; iCurr < 3; iCurr++)
		{
			myU[iCurr] = (myPoints[iCurr].x - srRect.x)/srRect.w;
			myV[iCurr] = (myPoints[iCurr].y - srRect.y)/srRect.h;
		}
	}

	inline bool isPointInside(const SVector2D& svPoint) const
	{
		return GeometryUtils::isPointInTriangle(myPoints[0], myPoints[1], myPoints[2], svPoint);
	}

	inline OrientationType getOrientation() const
	{
		return GeometryUtils::getTriangleOrientation(myPoints[0], myPoints[1], myPoints[2]);
	}

	SVector2D myPoints[3];
	FLOAT_TYPE myU[3];
	FLOAT_TYPE myV[3];
};
/*****************************************************************************/
typedef vector < STriangle2D > TTriangleVector;
/*****************************************************************************/
