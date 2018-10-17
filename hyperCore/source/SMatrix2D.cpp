#include "stdafx.h"

namespace HyperCore
{
/*****************************************************************************/
SVector2D operator *(const SMatrix2D& smMatrix, const SVector2D& srcVec)
{
	SVector2D svResult;

	svResult.x = smMatrix.myMatrix[0][0]*srcVec.x+
		smMatrix.myMatrix[1][0]*srcVec.y+
		smMatrix.myMatrix[2][0]*1.0f;

	svResult.y = smMatrix.myMatrix[0][1]*srcVec.x+
		smMatrix.myMatrix[1][1]*srcVec.y+
		smMatrix.myMatrix[2][1]*1.0f;

	return svResult;
}
/*****************************************************************************/
SVector3D operator *(const SMatrix2D& smMatrix, const SVector3D& srcVec)
{
	SVector3D svResult;

	svResult.x = smMatrix.myMatrix[0][0]*srcVec.x+
		smMatrix.myMatrix[1][0]*srcVec.y+
		smMatrix.myMatrix[2][0]*srcVec.z;

	svResult.y = smMatrix.myMatrix[0][1]*srcVec.x+
		smMatrix.myMatrix[1][1]*srcVec.y+
		smMatrix.myMatrix[2][1]*srcVec.z;

	svResult.z = smMatrix.myMatrix[0][2]*srcVec.x+
		smMatrix.myMatrix[1][2]*srcVec.y+
		smMatrix.myMatrix[2][2]*srcVec.z;

	return svResult;
}
/*****************************************************************************/
};