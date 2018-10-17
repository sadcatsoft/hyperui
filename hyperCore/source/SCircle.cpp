#include "stdafx.h"

namespace HyperCore
{
/*****************************************************************************/
void SCircle::fromObb(const SOBB& rOther)
{
	myCenter = rOther.myCenter;

	//	myRadius = F_SQRT(rOther.mySize.x*rOther.mySize.x/4.0 + rOther.mySize.y*rOther.mySize.y/4.0);
	// Unless the object really fills its entire box, which is rare,
	// we can get away with a simple maximum size. This also allows closer spacing
	// to walls.
	myRadius = max(rOther.mySize.x, rOther.mySize.y)/2.0;
}
/*****************************************************************************/
bool SCircle::doesIntersect(const SVector2D& svRayStart, const SVector2D& svRayDir,	FLOAT_TYPE& fT1Out, FLOAT_TYPE& fT2Out) const
{
	SVector2D svDelta;
	svDelta = svRayStart - myCenter;

	fT1Out = fT2Out = FLOAT_TYPE_MAX;

	FLOAT_TYPE fDot1 = svRayDir.dot(svDelta);

	FLOAT_TYPE fDiscrim = fDot1*fDot1 - (svDelta.lengthSquared() - myRadius*myRadius);
	if(fDiscrim < 0)
		return false;

	if(fabs(fDiscrim) <= FLOAT_EPSILON)
	{
		// Line is tangent to the circle
		fT1Out = -svRayDir.dot(svDelta);
		return true;
	}

	FLOAT_TYPE fDiscrimSqRooted = F_SQRT(fDiscrim);

	// Otherwise, we have two solutions
	fT1Out = (-svRayDir.dot(svDelta) + fDiscrimSqRooted); // /fDirSquared;
	fT2Out = (-svRayDir.dot(svDelta) - fDiscrimSqRooted); // /fDirSquared;

	return true;
}
/*****************************************************************************/
bool SCircle::doesIntersectSegment(const SVector2D& svLineStart, const SVector2D& svLineEnd, FLOAT_TYPE& fT1Out, FLOAT_TYPE& fT2Out) const
{
	SVector2D svDir = svLineEnd - svLineStart;
	FLOAT_TYPE fLen = svDir.normalize();
	FLOAT_TYPE fT1, fT2;
	int iNumSolutions = 0;
	fT1Out = fT2Out = FLOAT_TYPE_MAX;
	if(doesIntersect(svLineStart, svDir, fT1, fT2))
	{
		// See if the distances are within our length.
		if(fT1 != FLOAT_TYPE_MAX && fT1 >= 0 && fT1 <= fLen)
		{
			fT1Out = fT1;
			iNumSolutions++;
		}

		if(fT2 != FLOAT_TYPE_MAX && fT2 >= 0 && fT2 <= fLen)
		{
			if(iNumSolutions == 0)
				fT1Out = fT2;
			else
				fT2Out = fT2;
			iNumSolutions++;
		}
	}

	return (iNumSolutions > 0);
}
/*****************************************************************************/
};