#include "stdafx.h"

namespace HyperCore
{
/*****************************************************************************/
bool SOBB::doesIntersect(const SVector2D& svStartIn, const SVector2D& svDirIn, FLOAT_TYPE& tOut1, FLOAT_TYPE& tOut2) const
{
	SRay2D svRay(svStartIn, svDirIn);

	// Transform the vector into the coordinate system of the box.
	svRay.myDir.rotateCCW(-myAngle);

	// Now, transform the origin.
	svRay.myStart -= myCenter;
	svRay.myStart.rotateCCW(-myAngle);
	svRay.myStart += myCenter;

	SRect2D svRect(myCenter.x - mySize.x/2.0, myCenter.y - mySize.y/2.0, mySize.x, mySize.y);
	return svRect.doesIntersect(svRay, tOut1, tOut2);
}
/*****************************************************************************/
};