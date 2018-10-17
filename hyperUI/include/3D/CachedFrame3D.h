#pragma once

typedef vector < CachedPart3D* > TCachedParts3D;

/********************************************************************************************/
class CachedFrame3D
{
public:

	CachedFrame3D();
	~CachedFrame3D();

	bool loadFromObjFile(const char* pcsName, Window* pWindow, bool bFlipU, bool bFlipV, SVector3D& svDefaultScale, bool bFlipAllTris);

	int getNumParts(void);
	CachedPart3D* getPart(int iPart);

	void clear(void);
	void getBBox(SBBox3D& bbox_out);
	void translate(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ);

	int getDataMemUsage(void);

	bool loadFromClipFile(char* pData, int& iMemPos, CachedFrame3D* pMainFrame);

	void copyFrom(CachedFrame3D* pOther);
	
protected:

	CachedPart3D* addPart(const char* pcsName);
	CachedPart3D* addPartFromFaces(const char* pcsName, TObjFaceInfos& rFaces, SMaterialInfo* pMaterial, Window* pWindow, bool bFlipU, bool bFlipV, SVector3D& svDefaultScale, bool bFlipAllTris);

	bool loadMaterials(const char* pcsName);
	SMaterialInfo* findMaterial(const char* pcsName);

	void centerOnOrigin(void);

	void computeCentroid(SVector3D& svCenterOut);

private:

	TCachedParts3D myParts;
	TStringMatInfoMap myMaterials;

#ifndef MAC_BUILD
	static 
#endif
	string theCommonString, theCommonString2, theCommonString3;
};
/********************************************************************************************/