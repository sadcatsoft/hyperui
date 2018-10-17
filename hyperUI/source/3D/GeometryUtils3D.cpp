#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
void GeometryUtils3D::extrude(TPointVector& rPoints, SVector3D& svDir, FLOAT_TYPE fAmount, bool bFlipNormal, IGeoData* pGeoDataOut)
{
	// Each pair of vertices will generate 2 triangles. We assume this is an open curve.
	int iNumTriangles = (rPoints.size() - 1)*2;
	if(iNumTriangles < 1)
		return;

	pGeoDataOut->ensureCapacity(iNumTriangles);

	// We need to generate UVs for the resultant mesh. This means
	// we have to know the total length of the curve.
	FLOAT_TYPE fTotalLength = Polyline2D::computeLength(rPoints);
	FLOAT_TYPE fCurrV = 0;
	FLOAT_TYPE fVStep, fCurrLen;

	SVector2D svPoint1, svPoint2, svUVs[4];
	SVector3D svPoints[4], svNormals[4];

	// For now only the V coord varies, so set all UVs to 0 and 1 v coords
	svUVs[0].x = svUVs[1].x = 0.0;
	svUVs[2].x = svUVs[3].x = 1.0;

	int iCurrPoint, iNumPoints = rPoints.size();
	for(iCurrPoint = 1; iCurrPoint < iNumPoints; iCurrPoint++)
	{
		svPoint1 = rPoints[iCurrPoint - 1];
		svPoint2 = rPoints[iCurrPoint];
		
		fCurrLen = (svPoint2 - svPoint1).length();
		fVStep = fCurrLen/fTotalLength;

		svPoints[0].set(svPoint1.x, svPoint1.y, 0.0);
		svPoints[1].set(svPoint2.x, svPoint2.y, 0.0);

		svPoints[2] = svPoints[0] + svDir*fAmount;
		svPoints[3] = svPoints[1] + svDir*fAmount;

		// Now, compute the normals
		computeNormal(svPoints, svNormals[0]);
		if(bFlipNormal)
			svNormals[0] *= -1;
		svNormals[1] = svNormals[2] = svNormals[3] = svNormals[0];

		// Now, compute the UVs at all the points
		svUVs[0].y = svUVs[2].y = fCurrV;
		svUVs[1].y = svUVs[3].y = fCurrV + fVStep;

		// Finally, add two triangles
		pGeoDataOut->addTriangle(&svPoints[0], &svUVs[0], &svNormals[0]);
		pGeoDataOut->addTriangle(&svPoints[1], &svUVs[1], &svNormals[1]);

		fCurrV += fVStep;
	}
}
/*****************************************************************************/
void GeometryUtils3D::computeNormal(SVector3D* svPoints, SVector3D& svNormalOut)
{
	// For now, not very robust - just take the first two edges. We should
	// really look for two longest ones for precision.
	SVector3D svSide1, svSide2;
	svSide1 = svPoints[1] - svPoints[0];
	svSide2 = svPoints[2] - svPoints[0];

	svNormalOut = svSide2.cross(svSide1);
}
/*****************************************************************************/
FLOAT_TYPE GeometryUtils3D::computeTriangleArea(SVector3D& svPoint1, SVector3D& svPoint2, SVector3D& svPoint3)
{
    SVector3D svDiff;
    svDiff = svPoint3 - svPoint1;
	return ((svPoint2 - svPoint1).cross(svDiff)).length()*0.5;
}
/*****************************************************************************/
void GeometryUtils3D::computeTangentAndBiTangent(SVector3D rPoints[3], SVector3D& svNormal, FLOAT_TYPE fUCoords[3], FLOAT_TYPE fVCoords[3],  SVector3D& svTangentOut, SVector3D& svBitangentOut)
{
	SVector3D svDeltaPos1, svDeltaPos2;

	svDeltaPos1 = rPoints[1] - rPoints[0];
	svDeltaPos2 = rPoints[2] - rPoints[0];

	SVector2D svDeltaUV1, svDeltaUV2;
	svDeltaUV1.set(fUCoords[1] - fUCoords[0], fVCoords[1] - fVCoords[0]);
	svDeltaUV2.set(fUCoords[2] - fUCoords[0], fVCoords[2] - fVCoords[0]);

	FLOAT_TYPE r = 1.0;
	FLOAT_TYPE fDenom = (svDeltaUV1.x * svDeltaUV2.y - svDeltaUV1.y * svDeltaUV2.x);
	if(fabs(fDenom) > FLOAT_EPSILON)
		r = 1.0f / fDenom;

	svTangentOut = (svDeltaPos1* svDeltaUV2.y - svDeltaPos2* svDeltaUV1.y)*r;
	svBitangentOut = (svDeltaPos2 * svDeltaUV1.x   - svDeltaPos1 * svDeltaUV2.x)*r;

	svTangentOut = (svTangentOut - svNormal * svNormal.dot(svTangentOut));
	svTangentOut.normalize();

	SVector3D svTemp;
	svTemp = svNormal.cross(svTangentOut);
	if (svTemp.dot(svBitangentOut) < 0.0f)
		svTangentOut = svTangentOut * -1.0f;

	/*
#ifdef _DEBUG
	FLOAT_TYPE fTempDot;

	fTempDot = svNormal.dot(svTangentOut);
	_ASSERT(fabs(fTempDot) < FLOAT_EPSILON);
	fTempDot = svNormal.dot(svBitangentOut);
	_ASSERT(fabs(fTempDot) < FLOAT_EPSILON);
	fTempDot = svBitangentOut.dot(svTangentOut);
	_ASSERT(fabs(fTempDot) < FLOAT_EPSILON);
#endif
	*/
}
/*****************************************************************************/
void GeometryUtils3D::blendFrame(CachedFrame3D* pFrom, CachedFrame3D* pTo, FLOAT_TYPE fInterp, CachedFrame3D& rOut)
{
	int iPart, iNumParts = pFrom->getNumParts();
	_ASSERT(iNumParts == pTo->getNumParts());
	_ASSERT(iNumParts == rOut.getNumParts());

	CachedPart3D* pResPart;
	for(iPart = 0; iPart < iNumParts; iPart++)
	{
		pResPart = rOut.getPart(iPart);
		GeometryUtils3D::blendPart(pFrom->getPart(iPart), pTo->getPart(iPart), fInterp, *pResPart);
	}
}
/*****************************************************************************/
void GeometryUtils3D::blendPart(CachedPart3D* pFrom, CachedPart3D* pTo, FLOAT_TYPE fInterp, CachedPart3D& rOut)
{
	int iVert, iNumVerts = pFrom->getNumVerts();
	_ASSERT(pTo->getNumVerts() == iNumVerts);
	_ASSERT(rOut.getNumVerts() == iNumVerts);

	SVertex3D* pFromData = pFrom->getData();
	SVertex3D* pToData = pTo->getData();
	SVertex3D* pResData = rOut.getData();
	FLOAT_TYPE fOneLessInterp = 1.0 - fInterp;
	for(iVert = 0; iVert < iNumVerts; iVert++)
	{
		pResData[iVert].x = pFromData[iVert].x*fOneLessInterp + pToData[iVert].x*fInterp;
		pResData[iVert].y = pFromData[iVert].y*fOneLessInterp + pToData[iVert].y*fInterp;
		pResData[iVert].z = pFromData[iVert].z*fOneLessInterp + pToData[iVert].z*fInterp;

		pResData[iVert].nx = pFromData[iVert].nx*fOneLessInterp + pToData[iVert].nx*fInterp;
		pResData[iVert].ny = pFromData[iVert].ny*fOneLessInterp + pToData[iVert].ny*fInterp;
		pResData[iVert].nz = pFromData[iVert].nz*fOneLessInterp + pToData[iVert].nz*fInterp;

#ifdef ENABLE_NORMAL_MAPPING
		pResData[iVert].tangent_x = pFromData[iVert].tangent_x*fOneLessInterp + pToData[iVert].tangent_x*fInterp;
		pResData[iVert].tangent_y = pFromData[iVert].tangent_y*fOneLessInterp + pToData[iVert].tangent_y*fInterp;
		pResData[iVert].tangent_z = pFromData[iVert].tangent_z*fOneLessInterp + pToData[iVert].tangent_z*fInterp;

		pResData[iVert].bitangent_x = pFromData[iVert].bitangent_x*fOneLessInterp + pToData[iVert].bitangent_x*fInterp;
		pResData[iVert].bitangent_y = pFromData[iVert].bitangent_y*fOneLessInterp + pToData[iVert].bitangent_y*fInterp;
		pResData[iVert].bitangent_z = pFromData[iVert].bitangent_z*fOneLessInterp + pToData[iVert].bitangent_z*fInterp;
#endif
	}
}
/*****************************************************************************/
FLOAT_TYPE GeometryUtils3D::rayCircleIntersection(const SVector3D& svRayOrigin, const SVector3D& svRayDir, const SVector3D& svDiscCenter, 
									const SVector3D& svNormal, FLOAT_TYPE fDiscRadius, SVector3D* pInterPointOut, FLOAT_TYPE* pDiscDistOut)
{
	if(pDiscDistOut)
		*pDiscDistOut = FLOAT_TYPE_MAX;
	if(pInterPointOut)
		pInterPointOut->set(0, 0, 0);

	FLOAT_TYPE denom = svNormal.dot(svRayDir);
	if (fabs(denom) <= FLOAT_EPSILON)
		return false;

	float t = (svDiscCenter - svRayOrigin).dot(svNormal)/denom;
	if(t > FLOAT_EPSILON)
	{
		SVector3D svRayPoint = svRayOrigin + svRayDir*t;
		if(pInterPointOut)
			*pInterPointOut = svRayPoint;
		FLOAT_TYPE fCenterDist = (svRayPoint - svDiscCenter).length();
		if(pDiscDistOut)
			*pDiscDistOut = fCenterDist;
		if(fCenterDist <= fDiscRadius)
			return t;
		else
			return FLOAT_TYPE_MAX;
	}
	else
		return FLOAT_TYPE_MAX;
}
/*****************************************************************************/
};