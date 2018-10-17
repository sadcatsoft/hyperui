#pragma once

struct SObjInfo3D;
/********************************************************************************************/
struct SMaterialInfo
{
	SMaterialInfo()
	{
		reset();
	}

	void reset() 
	{ 
		myName = ""; 
		myTexture = ""; 
		myNormalTexture = "";
		mySpecularTexture = "";
		mySpecularCoeff = 0;
		myDiffuse.set(1, 1, 1, 1);
		myAmbient.set(1, 1, 1, 1);
		mySpecular.set(1, 1, 1, 1);
	}

	string myName;
	string myTexture;
	string myNormalTexture;
	string mySpecularTexture;

	SColor myDiffuse, myAmbient, mySpecular;
	float mySpecularCoeff;
};

typedef map < string, SMaterialInfo > TStringMatInfoMap;
/*****************************************************************************/
struct SObjFaceInfo
{
	// We only support triangles.
	// Note that these stupidly are 1-based.
	SVector3D myVertIndex[3], myTxIndex[3], myNormIndex[3];
};
typedef vector < SObjFaceInfo > TObjFaceInfos;
/********************************************************************************************/
class CachedPart3D
{
public:

	CachedPart3D();
	~CachedPart3D();

	SVertex3D* getData() { return myData; }
	int getNumVerts() { return myNumVerts; }

	void setName(const char* pcsName)
	{
		myName = pcsName;
	}
	inline const char* getName() const { return myName.c_str(); }

	void initFromFaces(TObjFaceInfos& rFaces, SMaterialInfo* pMaterial, Window* pWindow, bool bFlipU, bool bFlipV, SVector3D& svDefaultScale, bool bFlipAllTris);
	TX_MAN_RETURN_TYPE getTextureIndex(void) { return myCachedTextureIndex; }
	TX_MAN_RETURN_TYPE getNormalMapTextureIndex() { return myCachedNormalTextureIndex; }
	TX_MAN_RETURN_TYPE getSpecularMapTextureIndex() { return myCachedSpecularTextureIndex; }

	FLOAT_TYPE getTotalArea(int iStartVertex = 0, int iNumVerts = 0);

	BlendModeType getBlendMode() { return myCachedBlendMode; }

	bool loadFromClipFile(char* pData, int& iMemPos);

	// Note that this does not preserve anything
	void ensureNumVertices(int iNumVerts, bool bPreserveContents);

	void translate(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ);
	void getBBox(SBBox3D& bbox_out);

	int getDataMemUsage(void);
	void copyInto(Window* pWindow, SObjInfo3D& rInfo, int iStartVertex, int iNumVerts);
	void copyFrom(CachedPart3D* pOther);
	void clear(void);

	void ensureOrientation(SVector3D& svModelCenter);
	void ensureTexturesLoaded(TextureManager* pTextureManager);

	void computeTangetsAndBitangents();

	void getDiffuseMults(SColor& scolOut) { scolOut = myMaterial.myDiffuse; }
	void getAmbientMults(SColor& scolOut) { scolOut = myMaterial.myAmbient; }
	void getSpecularMults(SColor& scolOut) { scolOut = myMaterial.mySpecular; }
	FLOAT_TYPE getSpecularPower() { return myMaterial.mySpecularCoeff; }

	void createVBOOjbect();
	void destroyVBOObject();
#ifdef ENABLE_VBOS
	GLuint getVBOHandle() { return myVBOHandle; }
#endif

protected:

	void computeCentroid(SVector3D& svCenterOut);
	HyperCore::OrientationType getPolyOrientation(SVertex3D* pData, SVector3D& svCenter);

	TextureAnimSequence* loadTextureInto(TextureManager *pTextureManager, string& strInputPath, string& strTextureNameOut, TX_MAN_RETURN_TYPE& rCachedIndexOut, BlendModeType* eBlendModeOut);
	TextureAnimSequence* ensureTextureLoadedInternal(TextureManager *pTextureManager, TextureAnimSequence *pCurrTexture, string& strTextureName, TX_MAN_RETURN_TYPE& rCachedIndexOut, BlendModeType* eBlendModeOut);


private:
	int myNumVerts;
	int myVertexCapacity;
	SVertex3D* myData;

	string myName;

	// Currently used for loading/unloading
	TextureAnimSequence *myTexture;
	string myTextureName;
	TX_MAN_RETURN_TYPE myCachedTextureIndex;
	BlendModeType myCachedBlendMode;
#ifdef _DEBUG
	const char* pcsDebugTextureName;
#endif

	// Normal map
	TextureAnimSequence *myNormalTexture;
	string myNormalTextureName;
	TX_MAN_RETURN_TYPE myCachedNormalTextureIndex;

	// Specular map
	TextureAnimSequence *mySpecularTexture;
	string mySpecularTextureName;
	TX_MAN_RETURN_TYPE myCachedSpecularTextureIndex;

	SMaterialInfo myMaterial;


#ifdef ENABLE_VBOS
	// In this case, this is a series of VBOs per part 
	GLuint myVBOHandle;
#endif
};
/********************************************************************************************/