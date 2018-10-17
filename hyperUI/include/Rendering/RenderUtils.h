#pragma once

#ifdef DIRECTX_PIPELINE
#include "AdhocDxBuffer.h"
#endif

enum ProgressBarStyleType
{
	ProgressBarStyleFromLeft = 0,
	ProgressBarStyleFromRight,
	ProgressBarStyleFromBottom,
	ProgressBarStyleFromTop
};

const char* const g_pcsBarStyleStrings[] = 
{
	"FromLeft",
	"FromRight",
	"FromBottom",
	"FromTop",
};

class CachedPart3D;
class CachedSequence;
class CachedObject3D;
class Window;
/*****************************************************************************/
class HYPERUI_API IVertexColorGetter
{
public:
	~IVertexColorGetter() { }
	virtual void getVertexColor(FLOAT_TYPE fUParm, FLOAT_TYPE fVParm, SColor& scolOut) = 0;
};
/*****************************************************************************/
class HYPERUI_API RenderUtils
{
public:

	static void beginStencilRectangle(DrawingCache* pDrawingCache, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, bool bInverse = false);
	static void endStencil(DrawingCache* pDrawingCache);
	static bool isInsideStencil(void);

	static void beginScissorRectangle(const SRect2D& srRect, DrawingCache* pDrawingCache);
	static void beginScissorRectangle(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, DrawingCache* pDrawingCache);
	static void endScissorRectangle(DrawingCache* pDrawingCache);
	static bool getCurrentScissorRectangle(SRect2D& srRectOut);
	static bool doesIntersectScissorRect(const SRect2D& srRect);
	static bool isInsideScissor();

	static void drawText(DrawingCache* pDrawingCache, const char* pcsText, SCachedTextObject& rInfo);
	static void measureText(Window* pWindow, const char* pcsText, SCachedTextObject& rInfo, SVector2D& svDimsOut);
	static void measureText(Window* pWindow, const char* pcsText, const char* pcsFont, int iFontSize, FLOAT_TYPE fMaxWidth, SVector2D& svDimsOut);
	// Quick but rough text dims measurement.
	static int measureTextRough(Window* pWindow, const char* pcsText, const char* pcsFont, int iFontSize, FLOAT_TYPE fMaxWidth, SVector2D& svDimsOut);
	static void shortenString(Window* pWindow, string& strInOut, const char* pcsFont, int iFontSize, FLOAT_TYPE fMaxWidth, const char* pcsElipsis);
	static void shortenStringFromTheEnd(Window* pWindow, string& strInOut, const char* pcsFont, int iFontSize, FLOAT_TYPE fMaxWidth, const char* pcsElipsis, bool bSlowButAccurate, FLOAT_TYPE fTextScale = 1.0);
	static int shortenStringFromTheFront(Window* pWindow, string& strInOut, const char* pcsFont, int iFontSize, FLOAT_TYPE fMaxWidth, const char* pcsElipsis, bool bSlowButAccurate);

	//static int renderProgressBar(string& strEmptyAnim, string& strFullAnim, FLOAT_TYPE fProgress, SVector2D& svCenter, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale = 1.0, bool bDecreasesFromRight = true);
	static int renderProgressBar(Window* pWindow, string& strEmptyAnim, string& strFullAnim, FLOAT_TYPE fProgress, SVector2D& svCenter, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale = 1.0, ProgressBarStyleType eStyle = ProgressBarStyleFromLeft, SVector2D* pOptSize = NULL, bool bStartInMiddle = false);

	static void renderRectangle(Window* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fThickness, const SColor& scolFill, bool bManageState = true);
	static void renderArc(Window* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius, FLOAT_TYPE fThickness, const SColor& scolFill, FLOAT_TYPE fStartAngle = 0.0, FLOAT_TYPE fEndAngle = 360.0, const char* pcsTexture = NULL, FLOAT_TYPE fTaper = 0.0, bool bWrapTexture = false, bool bManageState = true, FLOAT_TYPE fRadius2 = 0.0);
	static void renderArc3D(Window* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ, FLOAT_TYPE fRadius, FLOAT_TYPE fThickness, SColor& scolFill, FLOAT_TYPE fStartAngle = 0.0, FLOAT_TYPE fEndAngle = 360.0, const char* pcsTexture = NULL, FLOAT_TYPE fTaper = 0.0, bool bWrapTexture = false, bool bManageState = true, bool bFlatOnGround = false);
	static void renderDashedCircle(Window* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius, FLOAT_TYPE fThickness, SColor& scolFill, FLOAT_TYPE fSolidDashAngle, FLOAT_TYPE fEmptyDashAngle, const char* pcsTexture = NULL, bool bManageState = true);
	static void renderDashedEllipse(Window* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius1, FLOAT_TYPE fRadius2, FLOAT_TYPE fThickness, SColor& scolFill, FLOAT_TYPE fSolidDashAngle, FLOAT_TYPE fEmptyDashAngle, const char* pcsTexture = NULL, bool bManageState = true);
	// Draws a clockwise circular progress.
	static void renderCircularProgress(Window* pWindow, FLOAT_TYPE fProgress, const char* pcsTexture, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius, FLOAT_TYPE fOpacity, FLOAT_TYPE fStartAngle = 90.0, FLOAT_TYPE fAngleSpan = 360.0, bool bPremultiplyAlpha = false, HyperCore::OrientationType eOrientation = OrientationCW, const SColor* pOptFillColor = NULL);
	static void renderGradientRectangle(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, const SGradient& rGradient);
	static void renderSubdividedRectangle(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fOpacity, IVertexColorGetter* pCallback, FLOAT_TYPE fTriSideLength);

	// The max kink multiplier is relative to the thickness.
	static void renderTexturedPath(Window* pWindow, TPointVector& rPoints, FLOAT_TYPE fThickness, const char* pcsTexture, FLOAT_TYPE fOpacity, bool bStretchTexture = true, bool bMiter = false, FLOAT_TYPE fMaxKinkMult = 1.0, const SColor* pFillCol = NULL, FLOAT_TYPE fTextureLengthMult = 1.0, FLOAT_TYPE fInitUOffset = 0.0, const SVector2D& svOffset = SVector2D::Zero);
	static void renderTexturedLine(CachedSequence* pMainBeam, SVector2D& svStartPoint, SVector2D& svEndPoint, 
		FLOAT_TYPE fRayOpacity = 1.0, FLOAT_TYPE fScale = 1.0, FLOAT_TYPE fRayAnimFrame = 0.0, bool bIsRayAnimated = false, bool bIsFrameAnAtlasFrame = false, FLOAT_TYPE fThicknessMult = 1.0);

	// NOTE: If calling this directly, we might need to multiply Y by -1.
	static void renderObject3D(Window* pWindow, const char* pcsObjType, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fCenterZ, FLOAT_TYPE fAlpha, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, int iClipIndex, SVector3D &pRotations, const char* pcsOverrideTextureAnim, bool bManageDepthTest);
	static void renderCustomObject3D(Window* pWindow, CachedSequence* pTexture, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fCenterZ, FLOAT_TYPE fAlpha, FLOAT_TYPE fScale, SVertex3D* pVertexData, int iNumVerts, SVector3D &pRotations, SColor& scolDiffuseMults, SColor& scolAmbMults);
	static void renderCustomObject2D(Window* pWindow, SVertexInfo* pVerts, int iNumVerts, bool bManageState);

	static void renderSimpleLightning(Window* pWindow, SVector2D& svFrom, SVector2D& svTo, const char* pcsTexture, FLOAT_TYPE fThickness, FLOAT_TYPE fThicknessArms, FLOAT_TYPE fProgress);

	static int measureTextHelper(const char* pcsText, FLOAT_TYPE fMaxWidth, bool bProcessCustomLineBreakers, FLOAT_TYPE fSpaceWidth,	FLOAT_TYPE fLinePixHeight, FLOAT_TYPE fKerning, FLOAT_TYPE fExraLineSpacing, SVector2D& svDimsOut);

	static void fillRectangle(FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fW, FLOAT_TYPE fH, const SColor& scol, bool bChangeState);
	static void fillRectangleAtDepth(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, SColor& scol, FLOAT_TYPE fDepth, bool bChangeState);
	static void fillRectangleCustomColors(FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fW, FLOAT_TYPE fH, const SColor *pCornerColors, bool bChangeState);

	static void drawVBO(GLuint iBufferIndex, int iNumVertComponents, int iNormalsByteOffset, int iUVsByteOffset, int iNumVertsToRender, int iStrideSize);
	template < class DATA_STRUCT_TYPE >
	static inline void drawBuffer(float *pVertexData, int iNumVertComponents, float* pTextureData, GL_COLOR_TYPE* pColorData, float* pNormalData, int iNumVertsToRender, int iOptStartVertex = 0, GLenum ePrimitiveType = GL_TRIANGLES)
	{
		drawBufferInternal(pVertexData, iNumVertComponents, pTextureData, pColorData, pNormalData, iNumVertsToRender, iOptStartVertex, ePrimitiveType, sizeof(DATA_STRUCT_TYPE));
	}
	static void drawBuffer(const float *pVertexData, const float* pTextureData, const float* pColorData, int iNumVertsToRender, GLenum ePrimitiveType = GL_TRIANGLES);
#ifndef USE_OPENGL2
	static void drawBuffer(float *pVertexData, int iNumVertComponents, unsigned char* pColorData, int iNumVertsToRender, GLenum ePrimitiveType = GL_TRIANGLES);
#endif

	static void drawBitmap(TextureManager* pTextureManager, const char* pcsType, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fOpacity, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha);
	static void drawBitmap(TextureManager* pTextureManager, const char* pcsType, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fOpacity, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, SUVSet* pUVsIn = NULL, bool bIsProgressAFrameNum = false);
	static void drawBitmapFromFrame(Window* pWindow, const char* pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScale, int iFrameNum, bool bPremultiplyAlpha, TextureManager* pTextureManager);
	static void drawBitmapNonUniScale(const char* pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, TextureManager* pTextureManager);

	static void drawLines(Window* pWindow, TPointVector& rPoints, const SColor& scol, FLOAT_TYPE fLineThickness, bool bChangeState);
	static void drawLines(Window* pWindow, SGenData& rDrawData, FLOAT_TYPE fLineThickness, bool bChangeState);

	static void drawRectangle(Window* pWindow, FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fW, FLOAT_TYPE fH, const SColor& scol, FLOAT_TYPE fLineWidth, bool bChangeState);
	static void drawEllipse(Window* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRx, FLOAT_TYPE fRy, SColor& scol, FLOAT_TYPE fLineThickness, bool bChangeState);

protected:
	~RenderUtils() { } 

private:
	// No constructing!
	RenderUtils() { }

	friend class DrawingCache;

#if defined(HARDCORE_LINUX_DEBUG_MODE)
public:
#endif
	static void drawOrMeasureText(DrawingCache* pDrawingCache, const char* pcsText, const char* pcsFontName, int iFontSize, FLOAT_TYPE fX, FLOAT_TYPE fY, const SColor &scolText, 
		HorAlignType eHorAlign = HorAlignCenter, VertAlignType eVertAlign = VertAlignCenter, FLOAT_TYPE fExtraKerning = 0.0, SVector2D* svStringMeasurementsOut = NULL,
		const SColor* scolShadow = NULL, const SVector2D* svShadowOffset = NULL, FLOAT_TYPE fW = 0.0,
		SColor* pHighlightColor = NULL, FLOAT_TYPE fScale = 1.0);

#if defined(HARDCORE_LINUX_DEBUG_MODE)
	static void drawOrMeasureTextDEBUG1(DrawingCache* pDrawingCache, const char* pcsText, const char* pcsFontName, FLOAT_TYPE fX, FLOAT_TYPE fY, const SColor &scolText,
		HorAlignType eHorAlign = HorAlignCenter, VertAlignType eVertAlign = VertAlignCenter, FLOAT_TYPE fExtraKerning = 0.0, SVector2D* svStringMeasurementsOut = NULL,
		const SColor* scolShadow = NULL, SVector2D* svShadowOffset = NULL, FLOAT_TYPE fW = 0.0,
		SColor* pHighlightColor = NULL, FLOAT_TYPE fScale = 1.0);

	static void drawOrMeasureTextDEBUG2(DrawingCache* pDrawingCache, const char* pcsText, const char* pcsFontName, FLOAT_TYPE fX, FLOAT_TYPE fY, const SColor &scolText,
		HorAlignType eHorAlign = HorAlignCenter, VertAlignType eVertAlign = VertAlignCenter, FLOAT_TYPE fExtraKerning = 0.0, SVector2D* svStringMeasurementsOut = NULL,
		const SColor* scolShadow = NULL, SVector2D* svShadowOffset = NULL, FLOAT_TYPE fW = 0.0,
		SColor* pHighlightColor = NULL, FLOAT_TYPE fScale = 1.0);

	static void drawOrMeasureTextDEBUG3(DrawingCache* pDrawingCache, const char* pcsText, const char* pcsFontName, FLOAT_TYPE fX, FLOAT_TYPE fY, const SColor &scolText,
		HorAlignType eHorAlign = HorAlignCenter, VertAlignType eVertAlign = VertAlignCenter, FLOAT_TYPE fExtraKerning = 0.0, SVector2D* svStringMeasurementsOut = NULL,
		const SColor* scolShadow = NULL, SVector2D* svShadowOffset = NULL, FLOAT_TYPE fW = 0.0,
		SColor* pHighlightColor = NULL, FLOAT_TYPE fScale = 1.0);

#endif

	static int generateArcPolys(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius1, FLOAT_TYPE fRadius2, FLOAT_TYPE fThickness, 
		const SColor& scolFill, FLOAT_TYPE fStartAngle, FLOAT_TYPE fEndAngle, CachedSequence* pSeq, 
		int iAtlasFrame, SUVSet& svInitUVs, FLOAT_TYPE fTaper, bool bWrapTexture, int iInitArrayOffset);

	static int generateArc3DPolys(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ, FLOAT_TYPE fRadius1, FLOAT_TYPE fRadius2, FLOAT_TYPE fThickness, 
		SColor& scolFill, FLOAT_TYPE fStartAngle, FLOAT_TYPE fEndAngle, CachedSequence* pSeq, 
		int iAtlasFrame, SUVSet& svInitUVs, FLOAT_TYPE fTaper, bool bWrapTexture, int iInitArrayOffset);

	static void internalBeginScissorRectangle(const SRect2D& srRect, DrawingCache* pDrawingCache, bool bIsPoppingStack);

	static void setObjectMaterial(CachedObject3D* pObj, CachedPart3D* pPart, FLOAT_TYPE fAlpha);
	static void setMaterial(SColor& scolDiffuse, SColor& scolAmbient, SColor& scolSpecular, FLOAT_TYPE fSpecularPower);

	static ShaderSetType setNormalMap(TX_MAN_RETURN_TYPE pTexture, SVertex3D* pData);
	static void resetNormalMap(ShaderSetType eOldType);

	static void setSpecularMap(TX_MAN_RETURN_TYPE pTexture);
	static void resetSpecularMap();

	static void drawBufferInternal(const float *pVertexData, int iNumVertComponents, const float* pTextureData, const GL_COLOR_TYPE* pColorData, const float* pNormalData, int iNumVertsToRender, int iOptStartVertex, GLenum ePrimitiveType, int iStrideSize);

private:

	// blending mode goes here (static, as well)
	static int myStencilDepthCount;
	static string myCachedString3;
	static TIntVector theTextLineLengths;

	static PerlinNoiseGenerator *theLightningNoiseGen;
	static Polyline2D theLightningLines[3];

	// Lightning cached parms
	static int theLightningPointOffset1;
	static int theLightningPointOffset2;
	static int theLightningSeed;
	static FLOAT_TYPE theLightningAngle1;
	static FLOAT_TYPE theLightningAngle2;
	static FLOAT_TYPE theLightningArmLength1;
	static FLOAT_TYPE theLightningArmLength2;

	static TGradientStops theGradientStops;

	static TRectVector theScissorRectsStack;

#ifdef DIRECTX_PIPELINE
	static AdhocDxBuffer myDxBuffer;
#endif
};
/*****************************************************************************/