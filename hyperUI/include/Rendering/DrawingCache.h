/*
DrawingCache.h

Created by Oleg Samus on 23/05/09.
Copyright 2009 Sad Cat Software. All rights reserved.

There are a number of ways to draw objects:

Sprites:
- Flat on 2D screen using 2D coordinates without any depth setting  in ortho (addSprite()).
- Flat on 2D screen using 2D coordinates with a depth coord for correct clipping in ortho(addDeepSprite() with persp = false).
- Draw a 2D sprite using 2D coordinates as if it was specified on ground plane in 3D with perspective (addDeepSprite() with persp = true).

3D Objects:
- Draw a 3D object using 2D coordinates of the room map in ortho (addObject3D())
- Draw a 3D object using 3D coordinates in 3D space with perspective (addPerspObject3D())
- Draw a custom object using 2D coordinates in orhto (addCustomObject3D())
 
*/
#pragma once

class PathTrail;

#define ALPHA_CUTOFF_VALUE_3D			0.4
#define ARC_ALPHA_CUTOFF_VALUE_3D		0.05
#define ADDITIVE_OBJ3D_ALPHA_CUTOFF		0.32

#define EXTRA_DEPTH_SHIFT		0.0

#define EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH
#define EXPERIMENTAL_SORT_ORTHO_SEQ_BY_DEPTH

class DrawingCache;
class TextCacher;
class HYPERUI_API TextureManager;
class CachedDeepSequence;
class Window;
struct SGenData;
struct SCachedTextObject;
typedef vector < SCachedTextObject* > TCachedTextObjects;

DEFINE_ALPHA_TREE_MEMCACHER(CachedSequence*) TCachedSequenceAlphaMapMemCacher;
typedef AlphaTreeMap < CachedSequence* > TCachedSequenceAlphaMap;

#define SPACE_WIDTH_REFERENCE_LETTER		'a'

typedef TLosslessDynamicArray < TCachedSequences* > TCachedSequenceLosslessDynamicArray;  
/***********************************************************************************************************/
struct SCachedLine
{
	SVector2D myStart, myEnd;
	SColor myColor;
	FLOAT_TYPE myThickness;
};
typedef vector < SCachedLine > TCachedLines;
/***********************************************************************************************************/
struct SCachedRectangle
{
	SRect2D myRect;
	SColor myColor;
	// If thickness < 0, we fill the rectangle, otherwise draw it.
	FLOAT_TYPE myLineThickness;
};
typedef vector < SCachedRectangle > TCachedRectangles;
/***********************************************************************************************************/
struct SCachedGradRectangle
{
	SRect2D myRect;
	SGradient myGradient;
};
typedef vector < SCachedGradRectangle > TCachedGradRectangles;
/***********************************************************************************************************/
struct SCachedArc
{
	SVector2D myCenter;
	FLOAT_TYPE myStartAngle, myEndAngle;
	FLOAT_TYPE myThickness, myRadius, myRadius2;
	FLOAT_TYPE myTaper;
	SColor myColor;
	string myTexture;
	bool myWrapTexture;
};
typedef vector < SCachedArc > TCachedArcs;
/***********************************************************************************************************/
struct SCachedCircularProgress
{
	FLOAT_TYPE myProgress;
	string myTexture;
	SVector2D myCenter;
	FLOAT_TYPE myRadius, myOpacity;
	FLOAT_TYPE myStartAngle, myAngleSpan;
	bool myPremultAlpha;
	HyperCore::OrientationType myOrientation;
};
typedef vector < SCachedCircularProgress > TCachedCircProgresses;
/***********************************************************************************************************/
struct SCachedArc3D
{
	SVector3D myCenter;
	FLOAT_TYPE myStartAngle, myEndAngle;
	FLOAT_TYPE myThickness, myRadius;
	FLOAT_TYPE myTaper;
	SColor myColor;
	string myTexture;
	bool myWrapTexture, myFlatOnGround;
	bool myIsPerspective;
};
typedef vector < SCachedArc3D > TCachedArcs3D;
/***********************************************************************************************************/
struct SCachedCustomObject3D
{
	SVector2D myCenter;
	SVector3D myRotations;	
	FLOAT_TYPE myAlpha, myScale, myElevation;

	SColor myDiffuseMults, myAmbMults;
	SVertex3D* myVertexData;
	int myNumVerts;
	CachedSequence* myTextureSeq;
};
typedef vector < SCachedCustomObject3D > TCachedCustomObjects3D;
/***********************************************************************************************************/
struct SCachedCustomObject2D
{
	SVertexInfo* myVerts;
	int myNumVerts;
};
typedef vector < SCachedCustomObject2D > TCachedCustomObjects2D;
/***********************************************************************************************************/
struct SCachedObject3D
{
	string myObjId;
	string myOverrideTexture;
	FLOAT_TYPE myAlpha, myScale;
	FLOAT_TYPE myAnimProgress;
	int myClipIndex;
	SVector3D myCenter;
	SVector3D myRotations;
};
typedef vector < SCachedObject3D > TCachedObjects3D;
/***********************************************************************************************************/
struct SCachedLetterInfo
{
	TX_MAN_RETURN_TYPE myTextureIndex;
	SVector2D myOrigin;
	SVector2D mySize;
	SColor myColor;
};
typedef vector < SCachedLetterInfo > TCachedLetterInfos;
/***********************************************************************************************************/
enum PerspObjType
{
	PerspObjDeepSeq = 0,
	PerspObjObject3D
};

struct SPerspObjInstance
{
	SPerspObjInstance()
	{
		myRelatedSeqPtr = NULL;
		mySeq = NULL;
		myVertStartIndex = 0;
	}

	// This will get flushed after the main object is.
	CachedSequence* myRelatedSeqPtr;

	CachedDeepSequence* mySeq;
	SCachedObject3D myObj3DInfo;

	FLOAT_TYPE myDepth;
	int myVertStartIndex;
	PerspObjType myType;

	bool operator()(const SPerspObjInstance& s1, const SPerspObjInstance& s2) const
	{
		return s1.myDepth < s2.myDepth;
	}
};
typedef multiset < SPerspObjInstance, SPerspObjInstance > TSortedPerspInstances;
/***********************************************************************************************************/
struct SCachedTrail
{
	PathTrail* myTrail;
	SVector2D myScroll;
	bool myAnimateFrame;
	FLOAT_TYPE myOpacity, myScale;
	CachedSequence* mySampledAnimSeq;
};
typedef vector < SCachedTrail > TCachedTrails;
/***********************************************************************************************************/
struct STexturedLine
{
	TPointVector* myPoints;
	FLOAT_TYPE myThickness;
	string myTextureName;
	FLOAT_TYPE myOpacity;
	FLOAT_TYPE myMaxKinkMult;
	bool myStretchTexture, myMiter;
	FLOAT_TYPE myTextureLengthMult;
	FLOAT_TYPE myInitUOffset;
	SColor myFillColor;

};
typedef vector < STexturedLine > TCachedTexturedLines;
/***********************************************************************************************************/
class DrawingCache
{
public:
	DrawingCache(Window* pWindow);
	~DrawingCache();
	
	CachedSequence* getCachedSequence(const char* pcsType, int* iAtlasFrameOut);
	CachedDeepSequence* getCachedDeepSequence(const char* pcsType, int* iAtlasFrameOut, bool bPerspective);
	
	void flush();
	void addSprite(const char *pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha = false, const SColor* pExtraColor = NULL);
	void addSprite(const char *pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha = false, SUVSet* pUVSet = NULL, bool bIsProgressAFrameNum = false, const SColor* pExtraColor = NULL);
	void addDeepSprite(const char *pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha = false, FLOAT_TYPE fZDepth = FLOAT_TYPE_MAX, bool bPerspective = false);
	void addDeepSprite(const char *pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha = false, SUVSet* pUVSet = NULL, bool bIsProgressAFrameNum = false, FLOAT_TYPE fZDepth = FLOAT_TYPE_MAX, bool bPerspective = false);
	void addObject3D(const char* pcsObjType, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fAlpha, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, int iClipIndex = 0, SVector3D* pRotations = NULL, const char* pcsOverrideTextureAnim = NULL);
	void addPerspObject3D(const char* pcsObjType, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fCenterZ, FLOAT_TYPE fAlpha, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, int iClipIndex = 0, SVector3D* pRotations = NULL, const char* pcsOverrideTextureAnim = NULL);
	void addCustomObject3D(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fCenterZ, FLOAT_TYPE fAlpha, FLOAT_TYPE fScale, CachedSequence* pTexture, SVertex3D* pVertexData, int iNumVerts, SColor& scolDiffuseMults, SColor& scolAmbMults, SVector3D* pRotations = NULL);
	void addCustomObject2D(SVertexInfo* pVerts, int iNumVerts);
	void addScalableButton(const char *pcsType, const SRect2D& srRect, FLOAT_TYPE fAlpha, SColor* pExtraColor = NULL);
	void addScalableButton(const char *pcsType, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fAlpha, SColor* pExtraColor = NULL);
	void addScalableButton(const char *pcsType, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fAlpha, FLOAT_TYPE fCornerXSize, FLOAT_TYPE fCornerYSize, SColor* pExtraColor = NULL);
	void addTexturedLineCollection(PolylineCollection& rColl, const char* pcsTexture, FLOAT_TYPE fAlpha, FLOAT_TYPE fThickness, bool bStretchTexture = true, bool bMiter = false, FLOAT_TYPE fMaxKinkMult = 1.0, FLOAT_TYPE fTextureLengthMult = 1.0, FLOAT_TYPE fInitUOffset = 0.0);
	void addTexturedLine(TPointVector* pPoints, const char* pcsTexture, FLOAT_TYPE fAlpha, FLOAT_TYPE fThickness, bool bStretchTexture = true, bool bMiter = false, FLOAT_TYPE fMaxKinkMult = 1.0, FLOAT_TYPE fTextureLengthMult = 1.0, FLOAT_TYPE fInitUOffset = 0.0, const SColor* pOptFillColor = NULL);

	void addDepthBufferOp(FLOAT_TYPE fXStart, FLOAT_TYPE fYStart, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fDepthValue);

	void getImageSize(const char *pcsType, int& iW, int &iH);
	
	void addPathTrail(PathTrail* pTrail, const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, CachedSequence* pcsSampledAnimSeq, bool bAnimateFrame);
	bool addRectangle(const SRect2D& srRect, const SColor& rColor, FLOAT_TYPE fLineThickness = -1);
	bool addRectangle(FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fW, FLOAT_TYPE fH, const SColor& rColor, FLOAT_TYPE fLineThickness = -1);
	void addGradientRectangle(const SRect2D& srRect, const SGradient& sGrad);
	void addLine(FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fEndX, FLOAT_TYPE fEndY, const SColor& col, FLOAT_TYPE fThickness);
	void addArc(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius, FLOAT_TYPE fThickness, const SColor& scolFill, FLOAT_TYPE fStartAngle = 0.0, FLOAT_TYPE fEndAngle = 360.0, const char* pcsTexture = NULL, FLOAT_TYPE fTaper = 0.0, bool bWrapTexture = false, FLOAT_TYPE fRadius2 = 0);
	void addArc3D(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ, FLOAT_TYPE fRadius, FLOAT_TYPE fThickness, SColor& scolFill, FLOAT_TYPE fStartAngle = 0.0, FLOAT_TYPE fEndAngle = 360.0, const char* pcsTexture = NULL, FLOAT_TYPE fTaper = 0.0, bool bWrapTexture = false, bool bIsPerspective = true, bool bFlatOnGround = true);
	//void addCircularProgress(FLOAT_TYPE fProgress, const char* pcsTexture, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius, FLOAT_TYPE fOpacity, FLOAT_TYPE fStartAngle = 90.0, FLOAT_TYPE fAngleSpan = 360.0, bool bPremultiplyAlpha = false, ::OrientationType eOrientation = OrientationCW);

	void addSequenceToRender(CachedSequence* pSeq);
	void addDeepSequenceToRender(CachedDeepSequence* pSeq, bool bPerspective, FLOAT_TYPE fDepth, int iVertOffset, FLOAT_TYPE fDepthShift, CachedSequence* pRelatedSeq);

	void addText(const char* pcsText, const char* pcsFontName, int iFontSize, FLOAT_TYPE fX, FLOAT_TYPE fY, const SColor &scolText, 
		HorAlignType eHorAlign = HorAlignCenter, VertAlignType eVertAlign = VertAlignCenter, FLOAT_TYPE fExtraKerning = 0.0, SVector2D* svStringMeasurementsOut = NULL,
		const SColor* scolShadow = NULL, SVector2D* svShadowOffset = NULL, FLOAT_TYPE fW = 0.0, FLOAT_TYPE fScale = 1.0, bool bResampleNearest = true, SColor* scolHighlightCol = NULL, BlendModeType eBlendMode = BlendModeNormal, bool bForceDrawingIfNotCached = false);
#ifdef USE_FREETYPE_FONTS
	void addCachedLetter(TX_MAN_RETURN_TYPE rTextureIndex, const SVector2D& svOrigin, const SVector2D& svSize, const SColor& scolColor);
#endif

	// Note: if we pass in the measurement vec, we don't actually draw the string, we *only* measure it.
	// DEPRECATED. Use addText() above.
	void addOrMeasureText(const char* pcsText, const char* pcsFontName, int iFontSize, FLOAT_TYPE fX, FLOAT_TYPE fY, SColor &scolText, 
		HorAlignType eHorAlign = HorAlignCenter, VertAlignType eVertAlign = VertAlignCenter, FLOAT_TYPE fExtraKerning = 0.0, SVector2D* svStringMeasurementsOut = NULL,
						  SColor* scolShadow = NULL, SVector2D* svShadowOffset = NULL, FLOAT_TYPE fW = 0.0, bool bForceDrawUncached = false, SColor* scolHighlightCol = NULL, FLOAT_TYPE fScale = 1.0, bool bDrawNowIfForced = false);
	
	void setAllowFlushingText(bool bValue) { myFlushText = bValue; }

	TextCacher* getTextCacher(void) { return myTextCacher; }

	void fillArea(const char* pcsTextureName, const SRect2D& srRect, FLOAT_TYPE fAlpha, FLOAT_TYPE fAreaScale, FLOAT_TYPE fBitmapScale);
	void fillArea(const char* pcsTextureName, int iStartX, int iStartY, int iAreaW, int iAreaH, FLOAT_TYPE fAlpha, FLOAT_TYPE fAreaScale, FLOAT_TYPE fBitmapScale);
	void onTimerTick(void);

	CachedDeepSequence* getDeepSequenceFromRegularSequence(CachedSequence* pSeq, bool bDoPerspective);

	void reloadAllTextures();
	void unloadTexture(const char* pcsAnimName);

	inline const Window* getWindow() const { return myParentWindow; }
	inline Window* getWindow() { return myParentWindow; }
	TextureManager* getTextureManager();
	void unloadAllUnloadableTextures();

	void resetNumFrameFlushes() { myNumFrameFlushes = 0; }
	inline int getNumFrameFlushes() const { return myNumFrameFlushes; }

private:
	void drawSampledTrail(SGenData &rSharedData, SCachedTrail* pTrail);
	bool drawCachedText(SCachedTextObject* pTextObj);
	

private:
	
	// For now, use a simple map. Later, we may want to have a cache
	// and/or do a one-level tri.
	///TStringCachedSequenceMap mySequences;
	///TCachedSeqCacher *myCacher;
	TCachedSequenceAlphaMap mySequences;
	TCachedSequenceAlphaMapMemCacher mySequencesMapMemCacher;
	TCachedSequences myAllSequences;
	// Sequences which actually have sprites	
	TCachedSequenceLosslessDynamicArray myFullSequences;

	// Deep sequences
	TStringCachedDeepSequenceMap myDeepSequences;
	// Sequences which actually have sprites
	TCachedDeepSequences myFullDeepSequences[MAX_DEEP_RENDER_STAGES];
	TCachedDeepSeqCacher *myDeepCacher;

	// Deep sequences with perspective
	TStringCachedDeepSequenceMap myDeepPerspSequences;
	// Sequences which actually have sprites
	
#if defined(EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH) || defined(EXPERIMENTAL_SORT_ORTHO_SEQ_BY_DEPTH)
	TSortedPerspInstances mySortedDeepPerspSequences;
#else
	TCachedDeepSequences myFullDeepPerspSequences[MAX_DEEP_RENDER_STAGES];
#endif
	TCachedObjects3D myCachedPerspObjects3D;

	TCachedDeepSeqCacher *myDeepPerspCacher;
	
	string myCachedString, myCachedString2;
	
	TCachedTexturedLines myCachedTexturedLines;
	TCachedTrails myTrails;
	TCachedRectangles myRectangles;
	TCachedGradRectangles myGradRectangles;
	TCachedTextObjects myTextObjects;
#ifdef USE_FREETYPE_FONTS
	TCachedLetterInfos myLetterObjects;
#endif
	TCachedArcs myArcs;
	TCachedArcs3D myArcs3D;
//	TObjects myCachedGameViewObjects;
	TCachedLines myLines;

	TCachedObjects3D myCachedObjects3D;
	
	TCachedCustomObjects3D myCachedCustomObjects3D;
	TCachedCustomObjects2D myCachedCustomObjects2D;

	TPointVector myCachedPoints;
	
	static SGenData mySharedTrailData; 
	// TODO: Remove.
	static TIntVector theTextLineLengths;

	TextCacher* myTextCacher;

	// These are operations to the depth buffer, not actual 
	// primitives to be rendered.
	TCachedRectangles myDepthBufferOps;

	bool myFlushText;

	Window* myParentWindow;

	int myNumFrameFlushes;
};
/***********************************************************************************************************/


