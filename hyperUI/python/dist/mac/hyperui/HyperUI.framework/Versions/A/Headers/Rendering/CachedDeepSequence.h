#pragma once

#ifdef DIRECTX_PIPELINE
#include "AdhocDxBuffer.h"
#endif

class CachedSequence;
/***********************************************************************************************************/
struct SVertex3DInfo
{
	//short x,y,z;
	//unsigned char padding[14];

	FLOAT_TYPE x,y,z;
#ifdef USE_OPENGL2
	FLOAT_TYPE w;
#endif

	// Texture coords
	float s0, t0;

#ifdef USE_OPENGL2
	float r, g, b, a;
	float padding[6];
#else
	unsigned char r,g,b,a;
	unsigned char padding[8];
#endif


#ifdef USE_OPENGL2
	SVertex3DInfo() { w = 1.0; }
	inline static int getNumVertexComponents() { return 4; }
#else
	inline static int getNumVertexComponents() { return 3; }
#endif	

};
/***********************************************************************************************************/
struct SExtraSeqDrawInfo3D
{
	SExtraSeqDrawInfo3D()
	{
		reset();
	}

	void reset(void)
	{
		myW = myH = 0;
		myUseVertices = false;
		mySkewingPortion = mySkewingSize = 0;
		myPolyColor.set(1,1,1,1);
		myVerticalAxisAngle = 0;

		myIsFlatOnGround = false;
		myPerspExtraOffset.set(0,0, 0);
		myRotPivot.set(0,0,0);

		myRelatedSeq = NULL;
	}

	CachedSequence* myRelatedSeq;

	SColor myPolyColor;
	FLOAT_TYPE myW, myH;

	// Portion is [0,1], where 0 means no skewing and 1 means
	// skew the entire bitmap. The size relative to the
	// direction being skewed - in our case, width. If this
	// is positive, we skew to the right in x, if negative,
	// to the left.
	FLOAT_TYPE mySkewingPortion, mySkewingSize;

	bool myUseVertices;
	SVector3D myVerts[4];
	SVector2D myCustomUVs[4];

	SVector3D myPerspExtraOffset;

	// Angle, in degrees, to rotate around our vertical axis.
	FLOAT_TYPE myVerticalAxisAngle;
	SVector3D myRotPivot;

	// If true, the extra item is drawn flat on the ground.
	// Perspective mode only.
	bool myIsFlatOnGround;
};
/***********************************************************************************************************/
// Stores polygons of the same texture
class HYPERUI_API CachedDeepSequence
{
public:
	CachedDeepSequence(DrawingCache* pParent, bool bPerspective);
	~CachedDeepSequence();

	void setAnimType(const char *pcsType);
	void flush(int iVertStartIndex = -1);
	void flushRangeInclusive(int iVertextStartIndex, int iVertextEndIndex);
	void addSprite(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, bool bTestScreenIntersection = true, FLOAT_TYPE fZDepth = FLOAT_TYPE_MAX);
	void addSprite(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, bool bTestScreenIntersection = true, SUVSet* pUVSet = NULL, bool bIsProgressAFrameNum = false, SExtraSeqDrawInfo3D* pExtraInfo = NULL, FLOAT_TYPE fZDepth = FLOAT_TYPE_MAX, FLOAT_TYPE fPerspVertShift = 0.0, FLOAT_TYPE fDepthShift = 0);	
	inline const char* getName() const { return myAnimType.c_str(); }
	inline const char* getId() const { return myAnimType.c_str(); }
	inline int getRenderStage(void) { return myRenderStage; }
	inline void setRenderStage(int iStage) { myRenderStage = iStage; }

	bool getForceAlphaTest() { return myForceAlphaTest; }
	bool getDisableAlphaTest() { return myDisableAlphaTest; }
	void resetCache(void) { myCachedAnimProgress = -2.0; myCachedTexIndex = 0; } 

	void onTimerTick(GTIME lTime);

	TextureAnimSequence* getTexture() { return myTexture; }

	TextureManager* getTextureManager();

private:
	void ensureCapacity(int iNumTris);
	void getArrayIndicesFromTriIdx(int iTriIdx, int& iVertIdxOut, int& iColIdxOut, int& iUVIdxOut);

#ifdef CACHE_TEXTURE_INFO
	void initCachedTexParms(FLOAT_TYPE fAnimProgress, bool bIsProgressAFrameNum);
#endif

private:
	string myAnimType;

	int myRenderStage;

	DrawingCache *myParentCache;
	//SVertexInfo *myVertices;
	SVertex3DInfo *myVertices;
	int myNumAllocVerts;

	bool myDisableAlphaTest, myForceAlphaTest;

	int myNumTris; // currently in the array
	bool myIsUnloadable;

	TextureAnimSequence *myTexture;

	// Cached bitmap parms to avoid constant texture manager lookups
#ifdef CACHE_TEXTURE_INFO
	int myCachedTexW, myCachedTexH;

#ifdef DIRECTX_PIPELINE
	TX_MAN_RETURN_TYPE myCachedTexIndex;
#else
	unsigned int myCachedTexIndex;
#endif

	SUVSet myCachedTexUVs;
	BlendModeType myCachedTexBlendMode;	
	FLOAT_TYPE myCachedAnimProgress;
#endif

	bool myIsPerspective;

	static bool theInitializedPerspPoints;

	// Used to resolve overlapping textures
	int myCallCount;

	static SVector3D svTempVec[4];

	static int thePerspCallCount;

#ifdef DIRECTX_PIPELINE
	AdhocDxBuffer myDxBuffer;
#endif
};

typedef map < string, CachedDeepSequence* > TStringCachedDeepSequenceMap;
typedef vector < CachedDeepSequence* > TCachedDeepSequences;
typedef StringMapCacher < CachedDeepSequence > TCachedDeepSeqCacher;
/***********************************************************************************************************/
