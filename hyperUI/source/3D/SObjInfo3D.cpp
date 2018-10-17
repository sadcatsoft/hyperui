#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
void SObjInfo3D::onTimerTick(void)
{
	// Apply gravity.
	// We assume z is up, x is to the right.
	SVector3D svGravity(0, 0, -1.0);
	//myDir = myDir + svGravity*(1.0/GAME_FRAMERATE);
#ifndef _DEBUG
	// Decelerate
	mySpeed *= 0.95;
	//myRotationDirs *= 0.97;
#endif
	myCenter = myCenter + myDir*mySpeed*(1.0/GAME_FRAMERATE);

	myRotations += myRotationDirs*(1.0/GAME_FRAMERATE);
	myRotations.x = HyperCore::sanitizeDegAngle(myRotations.x);
	myRotations.y = HyperCore::sanitizeDegAngle(myRotations.y);
	myRotations.z = HyperCore::sanitizeDegAngle(myRotations.z);
}
/*****************************************************************************/
void SObjInfo3D::reset()
{
	if(myVertexData && myDoOwnVertexData)
		delete[] myVertexData;
	myVertexData = NULL;

	myDoOwnVertexData = false;
	myScale = 1.0;
	myAlpha = 1.0;
	myNumVerts = 0;
	mySpeed = 0;
	myDiffuseMults.set(1,1,1,1);
	myAmbMults.set(1,1,1,1);
	myTexture = NULL;
	myCenter.set(0,0,0);
	myRotationDirs.set(0,0,0);
}
/*****************************************************************************/
void SObjInfo3D::ensureNumVertsDestructive(int iNumVerts)
{
	// For now, simple:
	reset();
	myVertexData = new SVertex3D[iNumVerts];
	myNumVerts = iNumVerts;
	myDoOwnVertexData = true;
}
/*****************************************************************************/
void SObjInfo3D::copyAndRetainFrom(SVertex3D* pData, int iNumVerts)
{
	ensureNumVertsDestructive(iNumVerts);
	memcpy(myVertexData, pData, sizeof(SVertex3D)*iNumVerts);
}
/*****************************************************************************/
void SObjInfo3D::translateBy(SVector3D& svAmount)
{
	int iVert;
	for(iVert = 0; iVert < myNumVerts; iVert++)
	{
		myVertexData[iVert].x += svAmount.x;
		myVertexData[iVert].y += svAmount.y;
		myVertexData[iVert].z += svAmount.z;
	}

}
/*****************************************************************************/
FLOAT_TYPE SObjInfo3D::computeTotalArea()
{
	FLOAT_TYPE fTotalArea = 0;
	int iVert;
	SVector3D svPoints[3];
	for(iVert = 0; iVert < myNumVerts; iVert+=3)
	{
		svPoints[0].set(myVertexData[iVert].x, myVertexData[iVert].y, myVertexData[iVert].z);
		svPoints[1].set(myVertexData[iVert + 1].x, myVertexData[iVert + 1].y, myVertexData[iVert + 1].z);
		svPoints[2].set(myVertexData[iVert + 2].x, myVertexData[iVert + 2].y, myVertexData[iVert + 2].z);
		fTotalArea += fabs(GeometryUtils3D::computeTriangleArea(svPoints[0], svPoints[1], svPoints[2]));
	}

	return fTotalArea;
}
/*****************************************************************************/
void SObjInfo3D::computeCentroid(SVector3D& svOut)
{
	svOut.set(0,0,0);
	int iVert;
	for(iVert = 0; iVert < myNumVerts; iVert++)
	{
		svOut.x += myVertexData[iVert].x;
		svOut.y += myVertexData[iVert].y;
		svOut.z += myVertexData[iVert].z;
	}

	if(myNumVerts > 0)
		svOut *= 1.0/(FLOAT_TYPE)myNumVerts;
}
/*****************************************************************************/
};