#pragma once

struct SOBB;
/*****************************************************************************/
struct HYPERCORE_API SCircle
{
	SCircle() 
	{ 
		myRadius = 0; 
	}

	void fromObb(const SOBB& rOther);
	bool doesIntersect(const SVector2D& svRayStart, const SVector2D& svRayDir, FLOAT_TYPE& fT1Out, FLOAT_TYPE& fT2Out) const;
	bool doesIntersectSegment(const SVector2D& svLineStart, const SVector2D& svLineEnd, FLOAT_TYPE& fT1Out, FLOAT_TYPE& fT2Out) const;

	bool doesContain(FLOAT_TYPE fX, FLOAT_TYPE fY) const
	{
		SVector2D svDist(fX, fY);
		svDist = svDist - myCenter;
		if(svDist.lengthSquared() <= myRadius*myRadius)
			return true;
		else
			return false;
	}

	SVector2D myCenter;
	FLOAT_TYPE myRadius;
};
/*****************************************************************************/
