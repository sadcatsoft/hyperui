#include "stdafx.h"

namespace HyperCore
{
HYPERCORE_API const SVector2D SVector2D::Zero(0, 0);
/********************************************************************************************/
bool SVector2D::isOnLineSegment(const SVector2D& svStart, const SVector2D& svEnd, FLOAT_TYPE fTolerance, SVector2D* pExactPointOut, FLOAT_TYPE fHorTolerance) const
{
	SVector2D svDir, svNorm;
	svDir = svEnd - svStart;
	FLOAT_TYPE fLen = svDir.normalize();

	svDir.getPerpVector(svNorm);

	bool bRes;
	FLOAT_TYPE fDist = fabs(((*this) - svStart).dot(svNorm));
	if(fDist <= fTolerance)
		bRes = true;
	else
		bRes = false;

	if(bRes)
	{
		FLOAT_TYPE fDistHor = ((*this) - svStart).dot(svDir);
		if(fDistHor <= fHorTolerance || fDistHor >= fLen - fHorTolerance)
			bRes = false;

		if(bRes && pExactPointOut)
			*pExactPointOut = svStart + svDir*fDistHor;

	}

	return bRes;
}
/********************************************************************************************/
};