#pragma once

/***********************************************************************************************************/
class IGeoData
{
public:

	virtual ~IGeoData() { }

	virtual SVertex3D* getData() = 0;
	virtual CachedSequence* getTexture() = 0;
	virtual void setTexture(DrawingCache* pDrawingCache, const char* pcsTextureAnim) = 0;

	virtual void ensureCapacity(int iNumTriangles) = 0;

	virtual void addTriangle(SVector3D* svPoints, SVector2D* svUVs, SVector3D* svNormals) = 0;
};
/***********************************************************************************************************/