#pragma once

/*****************************************************************************/
struct HYPERUI_API STriCacherElement
{
	SVector3D myVertex;
	STriangle3D myTriangle;
};

typedef multimap < FLOAT_TYPE, STriCacherElement > TTriangleInfosMultimap;
typedef map < int, STriCacherElement > TTriangleResMap;
/*****************************************************************************/
class HYPERUI_API TriangleCacher
{
public:

	TriangleCacher();

	void addTriangle(SVertex3D& vert1, SVertex3D& vert2, SVertex3D& vert3);
	void findTrianglesForVertex(FLOAT_TYPE vx, FLOAT_TYPE vy, FLOAT_TYPE vz, TTriangleResMap& rVecOut);

private:

	int myCurrTriId;

	TTriangleInfosMultimap myTriangles;
};
/*****************************************************************************/