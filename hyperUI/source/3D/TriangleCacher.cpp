#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
TriangleCacher::TriangleCacher()
{
	myCurrTriId = 0;
}
/*****************************************************************************/
void TriangleCacher::addTriangle(SVertex3D& vert1, SVertex3D& vert2, SVertex3D& vert3)
{
	STriangle3D srTri;
	srTri.myPoints[0].set(vert1.x, vert1.y, vert1.z);
	srTri.myPoints[1].set(vert2.x, vert2.y, vert2.z);
	srTri.myPoints[2].set(vert3.x, vert3.y, vert3.z);

	srTri.myU[0] = vert1.u;
	srTri.myU[1] = vert2.u;
	srTri.myU[2] = vert3.u;

	srTri.myV[0] = vert1.v;
	srTri.myV[1] = vert2.v;
	srTri.myV[2] = vert3.v;

	srTri.myId = myCurrTriId;
	myCurrTriId++;

	STriCacherElement rDummy;
	rDummy.myTriangle = srTri;

	rDummy.myVertex = srTri.myPoints[0];
	myTriangles.insert(TTriangleInfosMultimap::value_type(rDummy.myVertex.x, rDummy));

	rDummy.myVertex = srTri.myPoints[1];
	myTriangles.insert(TTriangleInfosMultimap::value_type(rDummy.myVertex.x, rDummy));

	rDummy.myVertex = srTri.myPoints[2];
	myTriangles.insert(TTriangleInfosMultimap::value_type(rDummy.myVertex.x, rDummy));

}
/*****************************************************************************/
void TriangleCacher::findTrianglesForVertex(FLOAT_TYPE vx, FLOAT_TYPE vy, FLOAT_TYPE vz, TTriangleResMap& rVecOut)
{
	rVecOut.clear();

	// Now, find by all three vertices
	TTriangleInfosMultimap::iterator lb, ub, mi;
	lb = myTriangles.lower_bound(vx);
	ub = myTriangles.upper_bound(vx);

	STriCacherElement* pElem;
	SVector3D svTarget(vx, vy, vz);
	SVector3D svDiff;
	for(mi = lb; mi != ub; mi++)
	{
		// For each vertex, first compare that it is exactly the same vertex:
		pElem = &mi->second;
		svDiff = pElem->myVertex - svTarget;
		if(fabs(svDiff.x) > FLOAT_EPSILON || fabs(svDiff.y) > FLOAT_EPSILON || fabs(svDiff.z) > FLOAT_EPSILON)
			continue;

		// Now, we have the exact vertex. Make sure we didn't already include the triangle with this id in our list:
		rVecOut[pElem->myTriangle.myId] = *pElem;
	}
}
/*****************************************************************************/
};