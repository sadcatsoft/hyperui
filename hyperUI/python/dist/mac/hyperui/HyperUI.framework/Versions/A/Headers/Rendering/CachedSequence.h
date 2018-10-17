#pragma once

#ifdef DIRECTX_PIPELINE
#include "AdhocDxBuffer.h"
#endif

// #define USE_INT_OPENGL_POSITIONS
class HYPERUI_API DrawingCache;
class HYPERUI_API TextureManager;
/***********************************************************************************************************/
struct HYPERUI_API SExtraSeqDrawInfo
{
	SExtraSeqDrawInfo()
	{
		reset();
	}

	void reset(void)
	{
		myW = myH = 0;
		myUseVertices = false;
		mySkewingPortion = mySkewingSize = 0;
		myPolyColor.set(1,1,1,1);
	}

	SColor myPolyColor;
	FLOAT_TYPE myW, myH;

	// Portion is [0,1], where 0 means no skewing and 1 means
	// skew the entire bitmap. The size relative to the
	// direction being skewed - in our case, width. If this
	// is positive, we skew to the right in x, if negative,
	// to the left.
	FLOAT_TYPE mySkewingPortion, mySkewingSize;

	bool myUseVertices;
	SVector2D myVerts[4];
	SVector2D myCustomUVs[4];
};
/***********************************************************************************************************/
// Stores polygons of the same texture
class HYPERUI_API CachedSequence
{
public:
	CachedSequence(DrawingCache* pParent);
	~CachedSequence();

	void setAnimType(const char *pcsType);
	void flush(void);
	inline bool getNeedsFlushing() { return myNumTris > 0; }
	void addSprite(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, bool bTestScreenIntersection = true);
	void addSprite(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, bool bTestScreenIntersection = true, const SUVSet* pUVSet = NULL, bool bIsProgressAFrameNum = false, const SExtraSeqDrawInfo* pExtraInfo = NULL);	
	inline const char* getName() const { return myAnimType.c_str(); }
	inline const char* getId() const { return myAnimType.c_str(); }
	inline int getRenderStage(void) { return myRenderStage; }
	inline void setRenderStage(int iStage) { myRenderStage = iStage; }

	void resetCache(void);
	void onTimerTick(GTIME lTime);

	FLOAT_TYPE* getSubtileMap(int &iWOut, int &iHOut)  { iWOut = mySubtileMapWidth; iHOut = mySubtileMapHeight; return mySubtileMap; }

	BlendModeType overrideBlendMode(BlendModeType eNewMode) { BlendModeType eOld = myCachedTexBlendMode;  myCachedTexBlendMode = eNewMode; return eOld;  }

	void ensureCapacity(int iNumTris);

	TextureAnimSequence* getTexture() { return myTexture; }
	inline void setMarkRelated(bool bValue) { myIsMarkedRelated = bValue; }
	inline bool getMarkRelated() { return myIsMarkedRelated; }

	TextureManager* getTextureManager();

#ifdef HARDCORE_LINUX_DEBUG_MODE
	unsigned int getCachedTexIndexDEBUG() { return myCachedTexIndex; }
#endif

private:

	void getArrayIndicesFromTriIdx(int iTriIdx, int& iVertIdxOut, int& iColIdxOut, int& iUVIdxOut);

#ifdef CACHE_TEXTURE_INFO
	void initCachedTexParms(FLOAT_TYPE fAnimProgress, bool bIsProgressAFrameNum, TextureAnimSequence *pOptExistingPointer);
#endif

private:
	string myAnimType;

	int myRenderStage;

	DrawingCache *myParentCache;
	SVertexInfo *myVertices;
	int myNumAllocVerts;

	//	float *myColors, *myVerts, *myUVs;
	//	int myNumAllocColors, myNumAllocVerts, myNumAllocUVs; // Allocated

	//GTIME myLastUsedTime;
	int myNumTris; // currently in the array
	bool myIsUnloadable;

	TextureAnimSequence *myTexture;

	// Cached bitmap parms to avoid constant texture manager lookups
#ifdef CACHE_TEXTURE_INFO
	int myCachedTexW, myCachedTexH;
	TX_MAN_RETURN_TYPE myCachedTexIndex;

	SUVSet myCachedTexUVs;
	BlendModeType myCachedTexBlendMode;	
	FLOAT_TYPE myCachedAnimProgress;
#endif

	// Subtile map props
	FLOAT_TYPE *mySubtileMap;
	int mySubtileMapWidth, mySubtileMapHeight;

	// If true, rendered along with the main perps object.
	bool myIsMarkedRelated;

	SVector2D svTempVec[4];

#ifdef DIRECTX_PIPELINE
	AdhocDxBuffer myDxBuffer;
#endif
};
typedef map < string, CachedSequence* > TStringCachedSequenceMap;
typedef vector < CachedSequence* > TCachedSequences;
typedef StringMapCacher < CachedSequence > TCachedSeqCacher;
/***********************************************************************************************************/
