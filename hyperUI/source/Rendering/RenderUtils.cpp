#include "stdafx.h"

namespace HyperUI
{
#ifdef DIRECTX_PIPELINE
#include "D3DRenderer.h"
extern D3DRenderer* g_pDxRenderer;
#endif

// There are TWO of these - one in DrawingCache.cpp one in RenderUtils.cpp!!! Bad Oleg...
#define EXTRA_FONT_PIXEL_PADDING		0.0 // 2.0

int RenderUtils::myStencilDepthCount = 0;
string RenderUtils::myCachedString3;
TIntVector RenderUtils::theTextLineLengths;

PerlinNoiseGenerator* RenderUtils::theLightningNoiseGen = NULL;
Polyline2D RenderUtils::theLightningLines[3];

TRectVector RenderUtils::theScissorRectsStack;

int RenderUtils::theLightningPointOffset1 = 0;
int RenderUtils::theLightningPointOffset2 = 0;
int RenderUtils::theLightningSeed = 0;
FLOAT_TYPE RenderUtils::theLightningAngle1 = 0;
FLOAT_TYPE RenderUtils::theLightningAngle2 = 0;
FLOAT_TYPE RenderUtils::theLightningArmLength1 = 0;
FLOAT_TYPE RenderUtils::theLightningArmLength2 = 0;

#ifdef DIRECTX_PIPELINE
AdhocDxBuffer RenderUtils::myDxBuffer;
#endif

TGradientStops RenderUtils::theGradientStops;

//extern Object3DManager *g_pObject3DManager;

#define NUM_SPRITE_VERTS		4

static
	GLfloat g_nonReusableSpriteFloatColors[] = {
		1,1,1,1,
		1,1,1,1,
		1,1,1,1,
		1,1,1,1,
};


#ifdef USE_OPENGL2
static
	float g_nonReusableSpriteColors[] = {
		1,1,1,1,
		1,1,1,1,
		1,1,1,1,
		1,1,1,1
};
#else

static
	unsigned char g_nonReusableSpriteColors[] = {
		255,255,255,255,
		255,255,255,255,
		255,255,255,255,
		255,255,255,255
};
#endif

#ifdef USE_OPENGL2
static GLfloat g_refSpriteVertices[] =
{
	-0.5f, -0.5f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.0f, 1.0f,
	-0.5f,  0.5f, 0.0f, 1.0f,
	0.5f,  0.5f, 0.0f, 1.0f,
};

static GLfloat g_finalSpriteVertices[] =
{
	-0.5f, -0.5f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.0f, 1.0f,
	-0.5f,  0.5f, 0.0f, 1.0f,
	0.5f,  0.5f, 0.0f, 1.0f,
};

static GLfloat g_refSpriteVertices3D[] =
{
	-0.5f, -0.5f, 0.0, 1.0f,
	0.5f, -0.5f, 0.0, 1.0f,
	-0.5f,  0.5f, 0.0, 1.0f,
	0.5f,  0.5f, 0.0, 1.0f,
};

static GLfloat g_finalSpriteVertices3D[] =
{
	-0.5f, -0.5f, 0.0, 1.0f,
	0.5f, -0.5f, 0.0, 1.0f,
	-0.5f,  0.5f, 0.0, 1.0f,
	0.5f,  0.5f, 0.0, 1.0f,
};
#else
static GLfloat g_refSpriteVertices[] =
{
	-0.5f, -0.5f,
	0.5f, -0.5f,
	-0.5f,  0.5f,
	0.5f,  0.5f,
};

static GLfloat g_finalSpriteVertices[] =
{
	-0.5f, -0.5f,
	0.5f, -0.5f,
	-0.5f,  0.5f,
	0.5f,  0.5f,
};

static GLfloat g_refSpriteVertices3D[] =
{
	-0.5f, -0.5f, 0.0,
	0.5f, -0.5f, 0.0,
	-0.5f,  0.5f, 0.0,
	0.5f,  0.5f, 0.0,
};

static GLfloat g_finalSpriteVertices3D[] =
{
	-0.5f, -0.5f, 0.0,
	0.5f, -0.5f, 0.0,
	-0.5f,  0.5f, 0.0,
	0.5f,  0.5f, 0.0,
};
#endif
/*****************************************************************************/
void RenderUtils::beginStencilRectangle(DrawingCache* pDrawingCache, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, bool bInverse)
{
	myStencilDepthCount++;
	if(myStencilDepthCount > 1)
	{
		// Can't have nested regions.
		//_ASSERT(0);
		return;
	}


	// We must flush everything to avoid it getting caught in the region
	pDrawingCache->flush();

#ifdef DIRECTX_PIPELINE

	//g_pDxRenderer->startDrawingIntoDepthStencil(!bInverse);
	g_pDxRenderer->startDrawingIntoDepthStencil(true);

	if(bInverse)
	{
		//		g_pDxRenderer->startDrawingIntoDepthStencil(1.0);
		g_pDxRenderer->getD3dContext()->ClearDepthStencilView(g_pDxRenderer->getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	else
	{
		//		g_pDxRenderer->startDrawingIntoDepthStencil(0.0);
		g_pDxRenderer->getD3dContext()->ClearDepthStencilView(g_pDxRenderer->getDepthStencilView(), D3D11_CLEAR_DEPTH, 0.0f, 0);		
	}

	SColor scolDummy(1,1,1,0.0);
	FLOAT_TYPE fDepth;
	if(bInverse)
		fDepth = 0.0;		
	else
		fDepth = 1.0;
	RenderUtils::fillRectangleAtDepth(fX, fY, fW, fH, scolDummy, fDepth, true);


	g_pDxRenderer->startDrawingIntoDepthStencil(false);

#else


#if defined(WIN32) || defined(LINUX)
	glClear(GL_STENCIL_BUFFER_BIT);

	glEnable(GL_STENCIL_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	SColor scolDummy(1,1,1,1);
	RenderUtils::fillRectangle(fX, fY, fW, fH, scolDummy, true);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	if(bInverse)
		glStencilFunc(GL_GREATER, 1, 1);
	else
		glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

#else
	// Darned iPhone has no stenicl support. So much for OpenGL ES 1.1 spec
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	if(bInverse)
		gClearDepth(0.0f);
	else
		gClearDepth(-1.0f);

	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Draw our rectangle zoomed in.
	SColor scolDummy(1,1,1,0.0);
	FLOAT_TYPE fDepth;
	if(bInverse)
		fDepth = -1.0;
	else
		fDepth = 0.0;
	RenderUtils::fillRectangleAtDepth(fX, fY, fW, fH, scolDummy, fDepth, true);

	glDepthMask(GL_FALSE);
	if(bInverse)
		glDepthFunc(GL_GREATER);
	else
		glDepthFunc(GL_EQUAL);
#endif

#endif
}
/*****************************************************************************/
void RenderUtils::endStencil(DrawingCache* pDrawingCache)
{
	_ASSERT(myStencilDepthCount > 0);
	myStencilDepthCount--;

	// Flush it all to draw it while the region is active
	pDrawingCache->flush();

#ifdef DIRECTX_PIPELINE
	g_pDxRenderer->endDrawingIntoDepthStencil();
#else


#if defined(WIN32) || defined(LINUX)
	glDisable(GL_STENCIL_TEST);
#else

	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	#ifdef ALLOW_3D
		glClear(GL_DEPTH_BUFFER_BIT);
	#endif
#endif
#endif
}
/*****************************************************************************/
bool RenderUtils::isInsideStencil(void)
{
	if(myStencilDepthCount > 0)
		return true;
	else
		return false;
}
/*****************************************************************************/
bool RenderUtils::getCurrentScissorRectangle(SRect2D& srRectOut)
{
	if(theScissorRectsStack.size() > 0)
	{
		srRectOut = theScissorRectsStack[theScissorRectsStack.size() - 1];
		return true;
	}
	else
		return false;
}
/*****************************************************************************/
void RenderUtils::beginScissorRectangle(const SRect2D& srRect, DrawingCache* pDrawingCache)
{
	// If we have anything on the stack, find our intersection
	SRect2D srFinal;
	if(theScissorRectsStack.size() > 0)
	{
		SRect2D srPrevRect;
		srPrevRect = theScissorRectsStack[theScissorRectsStack.size() - 1];
		srPrevRect.getOverlap(srRect, srFinal);
		if(srFinal.w < 0)
			srFinal.w = 0;
		if(srFinal.h < 0)
			srFinal.h = 0;
	}
	else
		srFinal = srRect;
	internalBeginScissorRectangle(srFinal, pDrawingCache, false);
	theScissorRectsStack.push_back(srFinal);
}
/*****************************************************************************/
void RenderUtils::beginScissorRectangle(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, DrawingCache* pDrawingCache)
{
	SRect2D srRect;
	srRect.x = fX;
	srRect.y = fY;
	srRect.w = fW;
	srRect.h = fH;
	beginScissorRectangle(srRect, pDrawingCache);
}
/*****************************************************************************/
void RenderUtils::internalBeginScissorRectangle(const SRect2D& srRect, DrawingCache* pDrawingCache, bool bIsPoppingStack)
{
	// We need to flip Y since we're flipped.
	SVector2D svScreenSize;
	pDrawingCache->getWindow()->getSize(svScreenSize);
	if(!bIsPoppingStack)
		pDrawingCache->flush();
 	OpenGLStateCleaner::doScissorCall(srRect.x, svScreenSize.y -(srRect.y + srRect.h), srRect.w, srRect.h);
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
 	glEnable(GL_SCISSOR_TEST);
#endif
}
/*****************************************************************************/
void RenderUtils::endScissorRectangle(DrawingCache* pDrawingCache)
{
	theScissorRectsStack.pop_back();

	// Now, if there were any more scissor rects, reactivate them now - before flushing!
	if(theScissorRectsStack.size() > 0)
	{
		pDrawingCache->flush();
		internalBeginScissorRectangle(theScissorRectsStack[theScissorRectsStack.size() - 1], pDrawingCache, true);
	}
	else
	{
		SVector2D svScreenSize;
		pDrawingCache->getWindow()->getSize(svScreenSize);
		pDrawingCache->flush();
#ifdef DIRECTX_PIPELINE
		DXSTAGE1
#else
 		glDisable(GL_SCISSOR_TEST);
#endif
	}
}
/*****************************************************************************/
void RenderUtils::drawText(DrawingCache* pDrawingCache, const char* pcsText, SCachedTextObject& rInfo)
{
	SColor* pColor = NULL;
	SVector2D* pOffset = NULL;
	if(rInfo.myHasShadow)
	{
		pColor = &rInfo.myShadowColor;
		pOffset = &rInfo.myShadowOffset;
	}

	RenderUtils::drawOrMeasureText(pDrawingCache, pcsText, rInfo.myBaseInfo.myFont.c_str(), rInfo.myBaseInfo.myFontSize, rInfo.myCenter.x, rInfo.myCenter.y, rInfo.myColor,
		rInfo.myHorAlign, rInfo.myVertAlign, rInfo.myBaseInfo.myExtraKerning, NULL, pColor, pOffset,
		rInfo.myBaseInfo.myWidth, &rInfo.myBaseInfo.myHighlightColor, rInfo.myScale);
}
/*****************************************************************************/
void RenderUtils::shortenStringFromTheEnd(Window* pWindow, string& strInOut, const char* pcsFont, int iFontSize, FLOAT_TYPE fMaxWidth, const char* pcsElipsis, bool bSlowButAccurate, FLOAT_TYPE fTextScale)
{
	// Measure the string while cutting it
	myCachedString3 = strInOut;
	int iCurrLen = strInOut.length();
	SVector2D svDims;
	do
	{
		if(bSlowButAccurate)
			measureText(pWindow, myCachedString3.c_str(), pcsFont, iFontSize, 0, svDims);
		else
			measureTextRough(pWindow, myCachedString3.c_str(), pcsFont, iFontSize, 0, svDims);
		if(svDims.x*fTextScale > fMaxWidth)
		{
			// Cut a letter:
			iCurrLen--;
			if(iCurrLen > 0)
			{
				myCachedString3 = strInOut.substr(0, iCurrLen);
				if(pcsElipsis)
					myCachedString3 += pcsElipsis;
			}
			else
				myCachedString3 = "";
		}

	} while(svDims.x*fTextScale > fMaxWidth && iCurrLen > 0);

	// Trim any spaces we might have - "word ..." just looks ugly.
	if(iCurrLen > 0 && iCurrLen < strInOut.length())
	{
		myCachedString3 = strInOut.substr(0, iCurrLen);
		while(myCachedString3.length() > 0 && myCachedString3[myCachedString3.length() - 1] == ' ')
			myCachedString3 = myCachedString3.substr(0, myCachedString3.length() - 1);
		if(pcsElipsis && myCachedString3.length() > 0)
			myCachedString3 += pcsElipsis;
	}


	strInOut = myCachedString3;
}
/*****************************************************************************/
int RenderUtils::measureTextHelper(const char* pcsText, FLOAT_TYPE fMaxWidth, bool bProcessCustomLineBreakers, FLOAT_TYPE fSpaceWidth,
									FLOAT_TYPE fLinePixHeight, FLOAT_TYPE fKerning, FLOAT_TYPE fExraLineSpacing, SVector2D& svDimsOut)
{
	int iNum = strlen(pcsText);

	FLOAT_TYPE fSpaceWithKerning = fSpaceWidth + fKerning;
	FLOAT_TYPE fFullLineHeight = fLinePixHeight + fExraLineSpacing;

	int iNumLines = 0;

	// We may have ^ (newlines) and/or text width specified.
	// Walk until we hit that.
	// Quick cases:
	bool bHaveNewlines = (bProcessCustomLineBreakers && (strstr(pcsText, "^") != NULL) || (strstr(pcsText, "\n") != NULL));
	if(!bHaveNewlines && fMaxWidth <= 0)
	{
		// No newlines, no width given:
		svDimsOut.x = iNum*fSpaceWithKerning - fKerning;
		svDimsOut.y = fLinePixHeight;
		iNumLines = 1;
	}
	else if(!bHaveNewlines && fMaxWidth > 0)
	{
		// No newlines, but with a width.
		// See how many lines we have:
		FLOAT_TYPE fCharsPerLine = fMaxWidth/fSpaceWithKerning;
		if(HyperCore::mantissa(fCharsPerLine) > FLOAT_EPSILON)
			fCharsPerLine += 1.0;

		FLOAT_TYPE fNumLines = (FLOAT_TYPE)iNum/(FLOAT_TYPE)((int)fCharsPerLine);
		if(HyperCore::mantissa(fNumLines) > FLOAT_EPSILON)
			fNumLines += 1.0;

		// Now, do the line thing:
		svDimsOut.x = fMaxWidth;
		svDimsOut.y = ((int)fNumLines)*fFullLineHeight;
		if(fNumLines > 1)
			svDimsOut.y -= fExraLineSpacing;
		iNumLines = fNumLines;
	}
	else
	{
		// Slow case - walk everything
		int iChar;
		int iMaxLineChars = 0;
		int iCurrLineChars = 0;
		int iNumLinesInner = 1;
		for(iChar = 0; iChar < iNum; iChar++, iCurrLineChars++)
		{
			if( (bProcessCustomLineBreakers && pcsText[iChar] == '^') || pcsText[iChar] == '\n' ||
				(fMaxWidth > 0 && ((iCurrLineChars + 1)*fSpaceWithKerning - fKerning) > fMaxWidth) )
			{
				iNumLinesInner++;
				if(iCurrLineChars > iMaxLineChars)
					iMaxLineChars = iCurrLineChars;
				iCurrLineChars = 0;
			}
		}

		// Now, get the dims
		if(fMaxWidth > 0)
			svDimsOut.x = fMaxWidth;
		else
			svDimsOut.x = iMaxLineChars*fSpaceWithKerning - fKerning;
		svDimsOut.y = ((int)iNumLinesInner)*fFullLineHeight;
		if(iNumLinesInner > 1)
			svDimsOut.y -= fExraLineSpacing;

		iNumLines = iNumLinesInner;
	}

	svDimsOut.roundToInt();
	return iNumLines;
}
/*****************************************************************************/
void RenderUtils::setObjectMaterial(CachedObject3D* pObj, CachedPart3D* pPart, FLOAT_TYPE fAlpha)
{
	// Get the materials for the part and set them
	SColor scolDiffuse, scolAmbient, scolSpecular;
	FLOAT_TYPE fSpecPower;

	pObj->getAmbientMults(pPart, scolAmbient);
	pObj->getDiffuseMults(pPart, scolDiffuse);
	pObj->getSpecularParms(pPart, scolSpecular, fSpecPower);

	if(fSpecPower <= FLOAT_EPSILON)
	{
		fSpecPower = 1.0;
		scolSpecular.set(0,0,0,0);
	}

	scolDiffuse.alpha *= fAlpha;

	setMaterial(scolDiffuse, scolAmbient, scolSpecular, fSpecPower);
}
/*****************************************************************************/
void RenderUtils::setMaterial(SColor& scolDiffuse, SColor& scolAmbient, SColor& scolSpecular, FLOAT_TYPE fSpecularPower)
{
	_ASSERT(fSpecularPower > 0.0);

#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
#ifdef USE_OPENGL2
	RenderStateManager::getInstance()->setAmbientMaterial(scolAmbient);
	RenderStateManager::getInstance()->setDiffuseMaterial(scolDiffuse);
	RenderStateManager::getInstance()->setSpecularMaterial(scolSpecular, fSpecularPower);
#else
	float colArray[4] = { 1,1,1, scolDiffuse.alpha };

	colArray[0] = scolDiffuse.r;
	colArray[1] = scolDiffuse.g;
	colArray[2] = scolDiffuse.b;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, colArray);

	colArray[0] = scolAmbient.r;
	colArray[1] = scolAmbient.g;
	colArray[2] = scolAmbient.b;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, colArray);

	colArray[0] = scolSpecular.r;
	colArray[1] = scolSpecular.g;
	colArray[2] = scolSpecular.b;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, colArray);

	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, fSpecularPower);
#endif
#endif
}
/*****************************************************************************/
int RenderUtils::measureTextRough(Window* pWindow, const char* pcsText, const char* pcsFont, int iFontSize, FLOAT_TYPE fMaxWidth, SVector2D& svDimsOut)
{
	const FLOAT_TYPE fScale = 1.0;
	const FLOAT_TYPE fExtraKerning = 0.0;

#ifdef USE_FREETYPE_FONTS
/*
	RenderUtils::measureText(pWindow, pcsText, pcsFont, iFontSize, fMaxWidth, svDimsOut);
#if !defined(_DEBUG) && !defined(_DEBUG)
#error Must return the number of lines here
#endif
	return 1;

*/
	FontManager* pFontManager = pWindow->getFontManager();
	const CachedFontInfo* pFontInfo = pFontManager->getFontInfo(pcsFont);

	// Note that we can't necessarily use the same letter as SPACE_WIDTH_REFERENCE_LETTER here, because we want
	// an average letter size, and the size of the space may be much to narrow to correctly estimate the text width.
	SVector2D svTempCharDims;
	pFontInfo->getCharDims('a', iFontSize, svTempCharDims);
	FLOAT_TYPE fSpaceWidth = svTempCharDims.x*fScale;
	FLOAT_TYPE fLinePixHeight = pFontInfo->getLineHeight(iFontSize)*fScale;
	FLOAT_TYPE fKerning = pFontInfo->getKerning(iFontSize)*fScale;
	FLOAT_TYPE fExraLineSpacing = pFontInfo->getExtraLineSpacing(iFontSize)*fScale;
	fKerning += fExtraKerning;

#else
	ResourceItem* pFont = g_pMainEngine->findResourceItem(ResourceRasterFonts, pcsFont);
	if(!pFont)
		ASSERT_RETURN_NULL;

	const char* pcsAllChars = pFont->getStringProp(PropertyFtChars);

	// Note that we can't necessarily use the same letter as SPACE_WIDTH_REFERENCE_LETTER here, because we want
	// an average letter size, and the size of the space may be much to narrow to correctly estimate the text width.
	FLOAT_TYPE fSpaceWidth = pFont->getNumericEnumPropValue(PropertyFtCharWidths, strchr(pcsAllChars, 'a') - pcsAllChars)*fScale;
	FLOAT_TYPE fLinePixHeight = pFont->getNumProp(PropertyFtHeight)*fScale;
	FLOAT_TYPE fKerning = pFont->getNumProp(PropertyKerning)*fScale;
	FLOAT_TYPE fExraLineSpacing = pFont->getNumProp(PropertyExtraLineSpacing)*fScale;
	fKerning += fExtraKerning*fScale;

#endif

	return measureTextHelper(pcsText, fMaxWidth, true, fSpaceWidth, fLinePixHeight, fKerning, fExraLineSpacing, svDimsOut);

	/*
	int iNum = strlen(pcsText);

	FLOAT_TYPE fSpaceWithKerning = fSpaceWidth + fKerning;
	FLOAT_TYPE fFullLineHeight = fLinePixHeight + fExraLineSpacing;

	// We may have ^ (newlines) and/or text width specified.
	// Walk until we hit that.
	// Quick cases:
	bool bHaveNewlines = (strstr(pcsText, "^") != NULL || strstr(pcsText, "\n") != NULL);
	if(!bHaveNewlines && fMaxWidth <= 0)
	{
		// No newlines, no width given:
		svDimsOut.x = iNum*fSpaceWithKerning - fKerning;
		svDimsOut.y = fLinePixHeight;
	}
	else if(!bHaveNewlines && fMaxWidth > 0)
	{
		// No newlines, but with a width.
		// See how many lines we have:
		FLOAT_TYPE fCharsPerLine = fMaxWidth/fSpaceWithKerning;
		if(mantissa(fCharsPerLine) > FLOAT_EPSILON)
			fCharsPerLine += 1.0;

		FLOAT_TYPE fNumLines = (FLOAT_TYPE)iNum/(FLOAT_TYPE)((int)fCharsPerLine);
		if(mantissa(fNumLines) > FLOAT_EPSILON)
			fNumLines += 1.0;

		// Now, do the line thing:
		svDimsOut.x = fMaxWidth;
		svDimsOut.y = ((int)fNumLines)*fFullLineHeight;
		if(fNumLines > 1)
			svDimsOut.y -= fExraLineSpacing;
	}
	else
	{
		// Slow case - walk everything
		int iChar;
		int iMaxLineChars = 0;
		int iCurrLineChars = 0;
		int iNumLines = 1;
		for(iChar = 0; iChar < iNum; iChar++, iCurrLineChars++)
		{
			if(pcsText[iChar] == '^' || pcsText[iChar] == '\n' ||
				(fMaxWidth > 0 && ((iCurrLineChars + 1)*fSpaceWithKerning - fKerning) > fMaxWidth) )
			{
				iNumLines++;
				if(iCurrLineChars > iMaxLineChars)
					iMaxLineChars = iCurrLineChars;
				iCurrLineChars = 0;
			}
		}

		// Now, get the dims
		if(fMaxWidth > 0)
			svDimsOut.x = fMaxWidth;
		else
			svDimsOut.x = iMaxLineChars*fSpaceWithKerning - fKerning;
		svDimsOut.y = ((int)iNumLines)*fFullLineHeight;
		if(iNumLines > 1)
			svDimsOut.y -= fExraLineSpacing;

	}
	*/
}
/*****************************************************************************/
void RenderUtils::shortenString(Window* pWindow, string& strInOut, const char* pcsFont, int iFontSize, FLOAT_TYPE fMaxWidth, const char* pcsElipsis)
{
	// Measure the string while cutting it
	myCachedString3 = strInOut;
	int iCurrLen = strInOut.length();
	SVector2D svDims;
	do
	{
		measureTextRough(pWindow, myCachedString3.c_str(), pcsFont, iFontSize, 0, svDims);
		if(svDims.x > fMaxWidth)
		{
			// Cut a letter:
			iCurrLen--;
			if(iCurrLen > 0)
			{
				myCachedString3 = strInOut.substr(0, iCurrLen);
				if(pcsElipsis)
					myCachedString3 += pcsElipsis;
			}
			else
				myCachedString3 = "";
		}

	} while(svDims.x > fMaxWidth && iCurrLen > 0);

	// Trim any spaces we might have - "word ..." just looks ugly.
	if(iCurrLen > 0 && iCurrLen < strInOut.length())
	{
		myCachedString3 = strInOut.substr(0, iCurrLen);
		while(myCachedString3.length() > 0 && myCachedString3[myCachedString3.length() - 1] == ' ')
			myCachedString3 = myCachedString3.substr(0, myCachedString3.length() - 1);
		if(pcsElipsis && myCachedString3.length() > 0)
			myCachedString3 += pcsElipsis;
	}

	strInOut = myCachedString3;
}
/*****************************************************************************/
void RenderUtils::measureText(Window* pWindow, const char* pcsText, const char* pcsFont, int iFontSize, FLOAT_TYPE fMaxWidth, SVector2D& svDimsOut)
{
	if(!pWindow)
	{
		_ASSERT(0);
		svDimsOut.set(0, 0);
		return;
	}

	SCachedTextObject rInfo;
	rInfo.myBaseInfo.myFont = pcsFont;
	rInfo.myBaseInfo.myFontSize = iFontSize;
	rInfo.myBaseInfo.myWidth = fMaxWidth;
	RenderUtils::measureText(pWindow, pcsText, rInfo, svDimsOut);
}
/*****************************************************************************/
void RenderUtils::measureText(Window* pWindow, const char* pcsText, SCachedTextObject& rInfo, SVector2D& svDimsOut)
{
	SColor* pColor = NULL;
	SVector2D* pOffset = NULL;
	if(rInfo.myHasShadow)
	{
		pColor = &rInfo.myShadowColor;
		pOffset = &rInfo.myShadowOffset;
	}

	RenderUtils::drawOrMeasureText(pWindow->getDrawingCache(), pcsText, rInfo.myBaseInfo.myFont.c_str(), rInfo.myBaseInfo.myFontSize, rInfo.myCenter.x, rInfo.myCenter.y, rInfo.myColor,
		rInfo.myHorAlign, rInfo.myVertAlign,
		rInfo.myBaseInfo.myExtraKerning, &svDimsOut, pColor, pOffset,
		rInfo.myBaseInfo.myWidth, &rInfo.myBaseInfo.myHighlightColor, rInfo.myScale);

}
/*****************************************************************************/
#ifdef USE_FREETYPE_FONTS
void RenderUtils::drawOrMeasureText(DrawingCache* pDrawingCache, const char* pcsText, const char* pcsFontName, int iFontSize, FLOAT_TYPE fX, FLOAT_TYPE fY, const SColor &scolText,
									HorAlignType eHorAlign, VertAlignType eVertAlign, FLOAT_TYPE fExtraKerning, SVector2D* svStringMeasurementsOut, const SColor* scolShadow, 
									const SVector2D* svShadowOffset, FLOAT_TYPE fW, SColor* pHighlightColor, FLOAT_TYPE fScale)
{
	FLOAT_TYPE fProgress = 1.0;

#ifdef _DEBUG
	if(IS_STRING_EQUAL(pcsText, "Begin Period 1"))
	{
		int bp = 0;
	}
#endif

	// For this, we seem to be slightly off in terms of pixels, so we add one:
	fY += upToScreen(0.5);

	ResourceCollection* pFonts = ResourceManager::getInstance()->getCollection(ResourceFonts);

	// Here, we override anything that has Chinese in it with a different font...
#ifdef USE_UNICODE_STRINGS
	STRING_TYPE strTextStorage;
	bool bHasChinese = gHasChineseCharacters(pcsText);
	bool bHasArabic = gHasArabicCharacters(pcsText);
	if(bHasChinese || bHasArabic)
	{
		// We can easily have the font size be zero here. So,  if it is, find the font size and
		// grab the size from there, since we only have a single Chinese font...
		if(iFontSize <= 0)
		{
			ResourceItem* pOrigFont = pFonts->findItemByType(pcsFontName);
			if(pOrigFont)
				iFontSize = pOrigFont->getNumProp(PropertyFontDefaultSize);
			ELSE_ASSERT;
		}

		if(bHasChinese)
			pcsFontName = STR_LIT("ftChineseMain");
		else if(bHasArabic)
		{
			pcsFontName = STR_LIT("ftArabicMain");

			// We also need to reverse the text... damn..
			strTextStorage = pcsText;
			pcsText = strTextStorage.c_str();
			std::reverse(strTextStorage.begin(), strTextStorage.end());
		}
	}
#endif

	// Find the right sequence
	FontManager* pFontManager = pDrawingCache->getWindow()->getFontManager();
	ResourceItem* pFontItems[FontStyleLastPlaceholder];

	pFontItems[FontStyleNormal] = pFonts->getItemById(pcsFontName);

#ifdef _DEBUG
	// For now, just grab the first one
	if(!pFontItems[FontStyleNormal])
		pFontItems[FontStyleNormal] = pFonts->getItem(0);
#endif

	// Could not find the right font
	if(!pFontItems[FontStyleNormal])
		ASSERT_RETURN;

	if(fProgress > 1.0)
		fProgress = 1.0;

	SVector2D svAlignmentOffsets(0,0);
	if(svStringMeasurementsOut)
		svStringMeasurementsOut->set(0,0);

#ifdef _DEBUG
	if(IS_STRING_EQUAL(pcsText, "Begin Playing"))
	{
		int bp = 0;
	}
#endif
	if(!svStringMeasurementsOut && (eHorAlign != HorAlignLeft || eVertAlign != VertAlignTop))
	{
		// Not the default alignment. Measure it.
		SVector2D svTextSize;
		RenderUtils::drawOrMeasureText(pDrawingCache, pcsText, pcsFontName, iFontSize, fX, fY, scolText, 
			eHorAlign, eVertAlign, fExtraKerning, &svTextSize, scolShadow, svShadowOffset, fW, NULL, 1.0);

		svTextSize *= fScale;

		// See what the offset is 
		if(eHorAlign == HorAlignRight)
			svAlignmentOffsets.x = -svTextSize.x;
		else if(eHorAlign == HorAlignCenter)
			svAlignmentOffsets.x = -svTextSize.x/2.0;
		else
			svAlignmentOffsets.x = 0.0;

		if(eVertAlign == VertAlignBottom)
			svAlignmentOffsets.y = -svTextSize.y;
		else if(eVertAlign == VertAlignCenter)
			svAlignmentOffsets.y = -svTextSize.y/2.0;
		else
			svAlignmentOffsets.y = 0.0;
	}
	int iCurr, iNum;
	char cChar;
	//int iCharIdx;

	FontStyleType eCurrFontStyle = FontStyleNormal;

	// Otherwise, see if we have styles:
	FLOAT_TYPE fSpaceWidth[FontStyleLastPlaceholder];
	FLOAT_TYPE fLinePixHeight[FontStyleLastPlaceholder];
	FLOAT_TYPE fKerning[FontStyleLastPlaceholder];
	FLOAT_TYPE fExraLineSpacing[FontStyleLastPlaceholder];
	const CachedFontInfo* pFontInfos[FontStyleLastPlaceholder];
	SVector2D svTempCharDims;

	memset(pFontInfos, 0, sizeof(CachedFontInfo*)*FontStyleLastPlaceholder);
	
	const char* pcsCurrFontName;
	const PropertyType eFontProps[FontStyleLastPlaceholder] = { PropertyNull };
	int iFontStyle = FontStyleNormal;
	//for(iFontStyle = 0; iFontStyle < FontStyleLastPlaceholder; iFontStyle++)
	{
		pcsCurrFontName = pFontItems[FontStyleNormal]->getStringProp(eFontProps[iFontStyle]);
		if(iFontStyle != FontStyleNormal)
			pFontItems[iFontStyle] = pFonts->getItemById(pcsCurrFontName);
		else
			pcsCurrFontName = pFontItems[iFontStyle]->getStringProp(PropertyId);

// 		if(!pFontItems[iFontStyle])
// 			continue;

		pFontInfos[iFontStyle] = pFontManager->getFontInfo(pcsCurrFontName);
// 		if(!pFontInfos[iFontStyle])
// 			continue;

		pFontInfos[iFontStyle]->getCharDims(SPACE_WIDTH_REFERENCE_LETTER, iFontSize, svTempCharDims);
		fSpaceWidth[iFontStyle] = svTempCharDims.x*fScale;
		fLinePixHeight[iFontStyle] = pFontInfos[iFontStyle]->getLineHeight(iFontSize)*fScale;
		fKerning[iFontStyle] = pFontInfos[iFontStyle]->getKerning(iFontSize)*fScale;
		fExraLineSpacing[iFontStyle] = pFontInfos[iFontStyle]->getExtraLineSpacing(iFontSize)*fScale;
		fKerning[iFontStyle] += fExtraKerning;

		// If we have no styles, we only need to populate the first, normal one.
// 		if(!pStyleInfo)
// 			break;
	}

#ifdef _DEBUG
	if(IS_STRING_EQUAL(pcsText, "Begin Playing"))
	{
		int bp = 0;
	}
#endif

	// We're  forced to insert newlines into text here if we're passed in a max width
	if(fW > 0.0)
	{
		iNum = strlen(pcsText);
		char* pcsEvil = const_cast<char*>(pcsText);
		FLOAT_TYPE fCurrXPos = 0;
		int iPrevSpacePosition = -1;
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
// 			if(pStyleInfo)
// 				eCurrFontStyle = pStyleInfo->getFontStyle(iCurr + iOptStyleCharOffset);
			cChar = pcsText[iCurr];
			if(cChar == '\n' || cChar == '^')
			{
				fCurrXPos = 0;
				iPrevSpacePosition = -1;
				continue;
			}

			if(fCurrXPos > fW*fScale && iPrevSpacePosition >= 0)
			{
				// We need to replace the previous space with a newline.
				// We need to insert a newline *before* the previous
				// character, since it already exceeded the limit.
				// And possibly step back.
				// And we need to do it 
				fCurrXPos = 0;
				//if(iPrevSpacePosition >= 0)
				{
					pcsEvil[iPrevSpacePosition] = '\n';
					iCurr = iPrevSpacePosition;
				}
				iPrevSpacePosition = -1;				
				continue;
			}

			if(cChar == ' ')
				iPrevSpacePosition = iCurr;

			pFontInfos[eCurrFontStyle]->getCharDims(cChar, iFontSize, svTempCharDims);
			fCurrXPos += svTempCharDims.x + fKerning[eCurrFontStyle];
		}
	}

	// Now, go letter by letter, computing the right coordinates on the spot
	iNum = strlen(pcsText);

	int iLastNewlinePos = -1;
	int iLineLen;
	for(iCurr = 0; iCurr <= iNum; iCurr++)
	{
		if(pcsText[iCurr] == '\n' || pcsText[iCurr] == '^' || (iLastNewlinePos >= 0 && pcsText[iCurr] == 0))
		{
			// Newline!
			if(iLastNewlinePos < 0)
				theTextLineLengths.clear();

			iLineLen = iCurr - iLastNewlinePos - 1;
			theTextLineLengths.push_back(iLineLen);
			iLastNewlinePos = iCurr;
		}
	}

	int iNextStopDrawingPos = iNum;
	if(iLastNewlinePos < 0)
	{
		iNum = ((FLOAT_TYPE)iNum)*fProgress;
		iNextStopDrawingPos = iNum;
	}
	else
		iNextStopDrawingPos = ((FLOAT_TYPE)theTextLineLengths[0])*fProgress;

	// If we can do progress for lines, we can do the drawing in the same loop.
	// Scan through the string once, tracking line lengths?
	SColor scolCurrent(scolText);

	eCurrFontStyle = FontStyleNormal;
	bool bIsInHighlightPart = false;
	FLOAT_TYPE iX = fX + svAlignmentOffsets.x, iY = fY + svAlignmentOffsets.y;
	int iCurrLineCount = 0;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		cChar = pcsText[iCurr];

		if(cChar == '|' && pHighlightColor)
		{
			// Toggle the highlight mode
			if(bIsInHighlightPart)
			{
				bIsInHighlightPart = false;
				scolCurrent = scolText;
			}
			else
			{
				bIsInHighlightPart = true;
				scolCurrent = *pHighlightColor;
				scolCurrent.alpha = scolText.alpha;
			}

			cChar = ' ';
		}

/*
		if(pStyleInfo)
		{
			pStyleInfo->getColor(iCurr + iOptStyleCharOffset, scolText, scolCurrent);
			eCurrFontStyle = pStyleInfo->getFontStyle(iCurr + iOptStyleCharOffset);
		}
*/

		if(cChar == '\n' || cChar == '^')
		{
			if(svStringMeasurementsOut && iX > svStringMeasurementsOut->x)
				svStringMeasurementsOut->x = iX;

			iCurrLineCount++;
			iNextStopDrawingPos = iCurr + ((FLOAT_TYPE)theTextLineLengths[iCurrLineCount])*fProgress + 1;
			iX = fX + svAlignmentOffsets.x;
			iY += fLinePixHeight[eCurrFontStyle] + fExraLineSpacing[eCurrFontStyle];
			continue;
		}
		else if(iCurr >= iNextStopDrawingPos)
			continue;

		// Find it in the string
		pFontInfos[eCurrFontStyle]->getCharDims(cChar, iFontSize, svTempCharDims);
		svTempCharDims *= fScale;

		if(!svStringMeasurementsOut)
		{
			// Render the shadow if any
			if(svShadowOffset && scolShadow)
				pFontInfos[eCurrFontStyle]->renderCharacterAt(pDrawingCache, cChar, iX + svShadowOffset->x, iY + svShadowOffset->y, *scolShadow, iFontSize, fScale);
			pFontInfos[eCurrFontStyle]->renderCharacterAt(pDrawingCache, cChar, iX, iY, scolCurrent, iFontSize, fScale);
		}			

		iX += svTempCharDims.x + fKerning[eCurrFontStyle];
	}

	if(svStringMeasurementsOut)
	{
		if(iX > svStringMeasurementsOut->x)
			svStringMeasurementsOut->x = iX;

		svStringMeasurementsOut->x -= fX;
		// Line spacing does not apply to the first line.
		svStringMeasurementsOut->y = (fLinePixHeight[eCurrFontStyle] + fExraLineSpacing[eCurrFontStyle])*(iCurrLineCount) + fLinePixHeight[eCurrFontStyle];

		// Increase by the shadow distance
		if(svShadowOffset && scolShadow)
		{
			svStringMeasurementsOut->x += svShadowOffset->x;
			svStringMeasurementsOut->y += svShadowOffset->y;
		}
	}
}
#else
void RenderUtils::drawOrMeasureText(DrawingCache* pDrawingCache, const char* pcsText, const char* pcsFontName, int iFontSize, FLOAT_TYPE fX, FLOAT_TYPE fY, const SColor &scolText,
									HorAlignType eHorAlign, VertAlignType eVertAlign,
									FLOAT_TYPE fExtraKerning, SVector2D* svStringMeasurementsOut, const SColor* scolShadow, const SVector2D* svShadowOffset,
									FLOAT_TYPE fW, SColor* pHighlightColor, FLOAT_TYPE fScale)

{
#ifdef HARDCORE_LINUX_DEBUG_MODE
	static int g_iDbLogCounter = 0;
	static int g_iDbLogCounter2 = 0;
#endif

#ifdef HARDCORE_LINUX_DEBUG_MODE
	bool bTrack = false;
	if(pcsText && IS_STRING_EQUAL(pcsText, "debug1057") && !svStringMeasurementsOut)
	{
		g_iDbLogCounter++;
		if(g_iDbLogCounter == 2)
			bTrack = true;
	}	

	if(pcsText && IS_STRING_EQUAL(pcsText, "test1") && !svStringMeasurementsOut)
	{
		g_iDbLogCounter2++;
		if(g_iDbLogCounter2 == 2)
			bTrack = true;
	}	

	if(bTrack)
		Logger::log("TEXTTRACK: Begin %s at (%g, %g) scolText = (%g,%g,%g,%g) ha = %d va = %d s = %g", pcsFontName, fX, fY, scolText.r, scolText.g, scolText.b, scolText.alpha, eHorAlign, eVertAlign, fScale);
#endif

	// Find the right sequence
	ResourceCollection* pFonts = ResourceManager::getInstance()->getCollection(ResourceRasterFonts);
	ResourceItem* pItem = pFonts->getItemById(pcsFontName);
#ifdef HARDCORE_LINUX_DEBUG_MODE
	if(bTrack)
		Logger::log("TEXTTRACK: item = %x", pItem);
#endif
	if(!pItem)
		ASSERT_RETURN;

	if(fProgress > 1.0)
		fProgress = 1.0;

	SVector2D svAlignmentOffsets(0,0);
	if(svStringMeasurementsOut)
		svStringMeasurementsOut->set(0,0);

	if(!svStringMeasurementsOut && (eHorAlign != HorAlignLeft || eVertAlign != VertAlignTop))
	{
		// Not the default alignment. Measure it.
		SVector2D svTextSize;
		RenderUtils::drawOrMeasureText(pDrawingCache, pcsText, pcsFontName, iFontSize, fX, fY, scolText,
			eHorAlign, eVertAlign, fExtraKerning, &svTextSize, scolShadow, svShadowOffset, fW);

		svTextSize *= fScale;

		// See what the offset is
		if(eHorAlign == HorAlignRight)
			svAlignmentOffsets.x = -svTextSize.x;
		else if(eHorAlign == HorAlignCenter)
			svAlignmentOffsets.x = -svTextSize.x/2.0;
		else
			svAlignmentOffsets.x = 0.0;

		if(eVertAlign == VertAlignBottom)
			svAlignmentOffsets.y = -svTextSize.y;
		else if(eVertAlign == VertAlignCenter)
			svAlignmentOffsets.y = -svTextSize.y/2.0;
		else
			svAlignmentOffsets.y = 0.0;
	}

	FLOAT_TYPE fCharWidth, fCharUVWidth;
	SExtraSeqDrawInfo rExtraInfo, rExtraShadowInfo;
	const char* pcsAllChars = pItem->getStringProp(PropertyFtChars);
	int iCurr, iNum;
	char cChar;
	int iCharIdx;

	FLOAT_TYPE fSpaceWidth = pItem->getNumericEnumPropValue(PropertyFtCharWidths, strchr(pcsAllChars, SPACE_WIDTH_REFERENCE_LETTER) - pcsAllChars)*fScale;
	FLOAT_TYPE fLineUVWidth = pItem->getNumProp(PropertyFtHeightUV);
	FLOAT_TYPE fLinePixHeight = pItem->getNumProp(PropertyFtHeight)*fScale;
	FLOAT_TYPE fKerning = pItem->getNumProp(PropertyKerning)*fScale;
	FLOAT_TYPE fExraLineSpacing = pItem->getNumProp(PropertyExtraLineSpacing)*fScale;
	fKerning += fExtraKerning*fScale;


	// We're  forced to insert newlines into text here if we're passed in a max width
	if(fW > 0.0)
	{
		iNum = strlen(pcsText);

		char* pcsEvil = const_cast<char*>(pcsText);

		FLOAT_TYPE fCurrXPos = 0;
		int iPrevSpacePosition = -1;
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			cChar = pcsText[iCurr];
			if(cChar == '\n' || cChar == '^')
			{
				fCurrXPos = 0;
				iPrevSpacePosition = -1;
				continue;
			}

			if(fCurrXPos > fW*fScale && iPrevSpacePosition >= 0)
			{
				// We need to replace the previous space with a newline.

				// We need to insert a newline *before* the previous
				// character, since it already exceeded the limit.
				// And possibly step back.
				// And we need to do it
				fCurrXPos = 0;
				//if(iPrevSpacePosition >= 0)
				{
					pcsEvil[iPrevSpacePosition] = '\n';
					iCurr = iPrevSpacePosition;
				}
				iPrevSpacePosition = -1;
				continue;
			}

			if(cChar == ' ')
				iPrevSpacePosition = iCurr;

			// Find it in the string
			iCharIdx = strchr(pcsAllChars, cChar) - pcsAllChars;

			if(iCharIdx < 0)
			{
				// Treat as space, skip.
				fCharWidth = fSpaceWidth;
				fCurrXPos += fCharWidth/2.0;
			}
			else
			{
				// Get all the metrics, print it:
				fCharWidth = pItem->getNumericEnumPropValue(PropertyFtCharWidths, iCharIdx)*fScale;
				fCurrXPos += fCharWidth/2.0;
			}
			fCurrXPos += fCharWidth/2.0 + fKerning;
		}
	}

	// Now, go letter by letter, computing the right coordinates on the spot
	iNum = strlen(pcsText);

	int iLastNewlinePos = -1;
	int iLineLen;
	for(iCurr = 0; iCurr <= iNum; iCurr++)
	{
		if(pcsText[iCurr] == '\n' || pcsText[iCurr] == '^' || (iLastNewlinePos >= 0 && pcsText[iCurr] == 0))
		{
			// Newline!
			if(iLastNewlinePos < 0)
				theTextLineLengths.clear();

			iLineLen = iCurr - iLastNewlinePos - 1;
			theTextLineLengths.push_back(iLineLen);
			iLastNewlinePos = iCurr;
		}
	}

	int iNextStopDrawingPos = iNum;
	if(iLastNewlinePos < 0)
	{
		iNum = ((FLOAT_TYPE)iNum)*fProgress;
		iNextStopDrawingPos = iNum;
	}
	else
		iNextStopDrawingPos = ((FLOAT_TYPE)theTextLineLengths[0])*fProgress;

	const char* pcsFontAnim = pItem->getStringProp(PropertyImage);
	CachedSequence* pFontSeq = pDrawingCache->getCachedSequence(pcsFontAnim, NULL);
	int iFontFileW, iFontFileH;
	pDrawingCache->getImageSize(pcsFontAnim, iFontFileW, iFontFileH);
#ifdef HARDCORE_LINUX_DEBUG_MODE
	if(bTrack)
		Logger::log("TEXTTRACK: pFontSeq = %x cachedTextIdx = %d fontFileDims = %dx%d", pFontSeq, pFontSeq->getCachedTexIndexDEBUG(), iFontFileW, iFontFileH);
#endif
	FLOAT_TYPE fSingleUPixel = 0.0;
	if(iFontFileW > 0)
		fSingleUPixel = EXTRA_FONT_PIXEL_PADDING/(FLOAT_TYPE)iFontFileW;
	ELSE_ASSERT;

	rExtraInfo.myPolyColor = scolText;
	rExtraInfo.myH = fLinePixHeight;

	if(scolShadow && svShadowOffset)
	{
		rExtraShadowInfo = rExtraInfo;
		rExtraShadowInfo.myPolyColor = *scolShadow;
	}

	bool bIsInHighlightPart = false;
	SUVSet rUVSet;
	FLOAT_TYPE iX = fX + svAlignmentOffsets.x, iY = fY + fLinePixHeight/2 + svAlignmentOffsets.y;
	int iCurrLineCount = 0;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		cChar = pcsText[iCurr];

		if(cChar == '|' && pHighlightColor)
		{
			// Toggle the highlight mode
			if(bIsInHighlightPart)
			{
				bIsInHighlightPart = false;
				rExtraInfo.myPolyColor = scolText;
			}
			else
			{
				bIsInHighlightPart = true;
				rExtraInfo.myPolyColor = *pHighlightColor;
			}
		}

		if(cChar == '\n' || cChar == '^')
		{
			if(svStringMeasurementsOut && iX > svStringMeasurementsOut->x)
				svStringMeasurementsOut->x = iX;

			iCurrLineCount++;
			iNextStopDrawingPos = iCurr + ((FLOAT_TYPE)theTextLineLengths[iCurrLineCount])*fProgress + 1;
			iX = fX + svAlignmentOffsets.x;
			iY += fLinePixHeight + fExraLineSpacing;
			continue;
		}
		else if(iCurr >= iNextStopDrawingPos)
			continue;

		// Find it in the string
		iCharIdx = strchr(pcsAllChars, cChar) - pcsAllChars;

		if(iCharIdx < 0)
		{
			// Treat as space, skip.
			fCharWidth = fSpaceWidth;
			iX += fCharWidth/2.0;
		}
		else
		{
			// Get all the metrics, print it:
			fCharWidth = pItem->getNumericEnumPropValue(PropertyFtCharWidths, iCharIdx)*fScale;
			fCharUVWidth = pItem->getNumericEnumPropValue(PropertyFtCharWidthsUV, iCharIdx);

			iX += fCharWidth/2.0;

			rUVSet.myStartX = pItem->getNumericEnumPropValue(PropertyFtCharXs, iCharIdx) - fSingleUPixel;
			rUVSet.myStartY = pItem->getNumericEnumPropValue(PropertyFtCharYs, iCharIdx);
			rUVSet.myEndX = rUVSet.myStartX + fCharUVWidth + fSingleUPixel*2.0;
			rUVSet.myEndY = rUVSet.myStartY + fLineUVWidth;
			rUVSet.myApplyToSize = true;

			rExtraInfo.myW = fCharWidth + (int)(EXTRA_FONT_PIXEL_PADDING*2.0);

			if(!svStringMeasurementsOut)
			{
#ifndef HARDCORE_LINUX_DEBUG_MODE
				// Render the shadow if any
				if(svShadowOffset && scolShadow)
				{
					rExtraShadowInfo.myW = rExtraInfo.myW;
					pFontSeq->addSprite(iX + svShadowOffset->x, iY + svShadowOffset->y,
						scolShadow->alpha, // Alpha
						0.0, fScale, fScale,
						1.0, //  anim progress
						true, true, &rUVSet, false, &rExtraShadowInfo);
				}
#endif

#ifdef HARDCORE_LINUX_DEBUG_MODE
				if(bTrack)
					Logger::log("TEXTTRACK: char = %c pt = (%g, %g) uvs = (%g, %g, %g, %g), extraSz = (%g, %g) extraCol = (%g, %g, %g, %g) fSingleUPix = %g", cChar, iX, iY, rUVSet.myStartX, rUVSet.myStartY, rUVSet.myEndX, rUVSet.myEndY, rExtraInfo.myW, rExtraInfo.myH, rExtraInfo.myPolyColor.r, rExtraInfo.myPolyColor.g, rExtraInfo.myPolyColor.b, rExtraInfo.myPolyColor.alpha, fSingleUPixel);
#endif

				pFontSeq->addSprite(iX, iY,
					scolText.alpha, // Alpha
					0.0, fScale, fScale,
					1.0, //  anim progress
					true, true, &rUVSet, false, &rExtraInfo);

#ifndef HARDCORE_LINUX_DEBUG_MODE
				// Draw twice if we're in the highlight part
				if(bIsInHighlightPart)
				{
					pFontSeq->addSprite(iX, iY,
						scolText.alpha, // Alpha
						0.0, fScale, fScale,
						1.0, //  anim progress
						true, true, &rUVSet, false, &rExtraInfo);
				}
#endif
			}
		}


		iX += fCharWidth/2.0 + fKerning;
	}

#ifdef HARDCORE_LINUX_DEBUG_MODE
	if(bTrack)
		Logger::log("TEXTTRACK: done");
#endif

	if(svStringMeasurementsOut)
	{
		if(iX > svStringMeasurementsOut->x)
			svStringMeasurementsOut->x = iX;

		svStringMeasurementsOut->x -= fX;
		// Line spacing does not apply to the first line.
		svStringMeasurementsOut->y = (fLinePixHeight + fExraLineSpacing)*(iCurrLineCount) + fLinePixHeight;

		// Increase by the shadow distance
		if(svShadowOffset && scolShadow)
		{
			svStringMeasurementsOut->x += svShadowOffset->x;
			svStringMeasurementsOut->y += svShadowOffset->y;
		}
	}
}
#endif
/*****************************************************************************/
int RenderUtils::renderProgressBar(Window* pWindow, string& strEmptyAnim, string& strFullAnim, FLOAT_TYPE fProgress, SVector2D& svCenter, 
								   FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, ProgressBarStyleType eStyle, SVector2D* pOptSize, bool bStartInMiddle)
{
	DrawingCache* pDrawingCache = pWindow->getDrawingCache();
	TextureManager* pTextureManager = pWindow->getTextureManager();

	// Since the middle one goes between [-1, 1], we essentially move
	// it to the middle of the slider.
	FLOAT_TYPE fOrigProgress = fProgress;
	if(bStartInMiddle)
	{
		fProgress = (fProgress + 1.0)/2.0;
		if(fOrigProgress < -1.0)
			fOrigProgress = -1.0;
		if(fOrigProgress > 1.0)
			fOrigProgress = 1.0;

	}

	if(fProgress < 0.0)
		fProgress = 0;
	if(fProgress > 1.0)
		fProgress = 1.0;

	if(!bStartInMiddle)
		fOrigProgress = fProgress;


	int iEmptyRealW, iEmptyRealH;
	pTextureManager->getTextureRealDims(strEmptyAnim.c_str(), iEmptyRealW, iEmptyRealH);

	// Draw the empty part
	int iEmptyCornerW, iEmptyCornerH;
	pTextureManager->getCornerSize(strEmptyAnim.c_str(), iEmptyCornerW, iEmptyCornerH);
	if( (iEmptyCornerW > 0 || iEmptyCornerH > 0) && pOptSize)
	{
		FLOAT_TYPE fScaledSize;
		if(eStyle == ProgressBarStyleFromLeft || eStyle == ProgressBarStyleFromRight)
		{
			fScaledSize = pOptSize->x*fScale;
			if(fScaledSize > iEmptyCornerW*2.0)
				pDrawingCache->addScalableButton(strEmptyAnim.c_str(), svCenter.x, svCenter.y, fScaledSize, iEmptyRealH, fOpacity, iEmptyCornerW, iEmptyCornerH);
		}
		else
		{
			fScaledSize = pOptSize->y*fScale;
			if(fScaledSize > iEmptyCornerH*2.0)
				pDrawingCache->addScalableButton(strEmptyAnim.c_str(), svCenter.x, svCenter.y, iEmptyRealW, fScaledSize, fOpacity, iEmptyCornerW, iEmptyCornerH);
		}
	}
	else
		pDrawingCache->addSprite(strEmptyAnim.c_str(), svCenter.x, svCenter.y, fOpacity, 0, fScale, 0, true);

	if(strFullAnim.length() == 0)
	{
		if(eStyle == ProgressBarStyleFromBottom || eStyle == ProgressBarStyleFromTop)
			return iEmptyRealH;
		else
			return iEmptyRealW;
	}

	// Full bitmap
	FLOAT_TYPE fPixelRealSize;
	int iRealSize, iRealW, iRealH;
	pTextureManager->getTextureRealDims(strFullAnim.c_str(), iRealW, iRealH);
	if(eStyle == ProgressBarStyleFromBottom || eStyle == ProgressBarStyleFromTop)
	{
		iRealSize = iRealH;
		if(pOptSize)
			fPixelRealSize = pOptSize->y;
		else
			fPixelRealSize = iRealSize;
	}
	else
	{
		iRealSize = iRealW;
		if(pOptSize)
			fPixelRealSize = pOptSize->x;
		else
			fPixelRealSize = iRealSize;
	}


	int iBitmapW, iBitmapH;
	pDrawingCache->getImageSize(strFullAnim.c_str(), iBitmapW, iBitmapH);

	FLOAT_TYPE fExtraOffset;
	if(bStartInMiddle)
		fExtraOffset = fOrigProgress*(fPixelRealSize/2.0)/2.0;
	else
	{
		if(pOptSize)
			fExtraOffset = fPixelRealSize*fProgress*0.5 + (fPixelRealSize*0.5 - fPixelRealSize*fProgress);
		else
			fExtraOffset = (1.0 - fProgress)*(FLOAT_TYPE)iRealSize/2.0;
	}

	// Get the new center of the bitmap
	SVector2D svFinalPos;
	if(bStartInMiddle)
	{
		if(eStyle == ProgressBarStyleFromLeft || eStyle == ProgressBarStyleFromRight)
			svFinalPos.x = svCenter.x + fExtraOffset;
		else if(eStyle == ProgressBarStyleFromBottom || eStyle == ProgressBarStyleFromTop)
			svFinalPos.y = svCenter.y + fExtraOffset;
	}
	else
	{
		if(eStyle == ProgressBarStyleFromLeft)
			svFinalPos.x = svCenter.x - fExtraOffset;
		else if(eStyle == ProgressBarStyleFromRight)
			svFinalPos.x = svCenter.x + fExtraOffset;
		else if(eStyle == ProgressBarStyleFromBottom)
			svFinalPos.y = svCenter.y + fExtraOffset;
		else if(eStyle == ProgressBarStyleFromTop)
			svFinalPos.y = svCenter.y - fExtraOffset;
	}

	if(eStyle == ProgressBarStyleFromLeft || eStyle == ProgressBarStyleFromRight)
		svFinalPos.y = svCenter.y;
	else
		svFinalPos.x = svCenter.x;

	int iCornerW, iCornerH;
	pTextureManager->getCornerSize(strFullAnim.c_str(), iCornerW, iCornerH);

	// Draw the full part
	bool bDidDraw = false;
	if(iCornerH > 0 || iCornerW > 0)
	{
		FLOAT_TYPE fScaledSize;
		if(eStyle == ProgressBarStyleFromLeft || eStyle == ProgressBarStyleFromRight)
		{
			if(bStartInMiddle)
				fScaledSize = fPixelRealSize*fScale*fabs(fOrigProgress)*0.5;
			else
				fScaledSize = fPixelRealSize*fScale*fProgress;
			if(fScaledSize > iCornerW*2.0)
			{
				pDrawingCache->addScalableButton(strFullAnim.c_str(), svFinalPos.x, svFinalPos.y, fScaledSize, iRealH, fOpacity, iCornerW, iCornerH);
				bDidDraw = true;
			}
		}
		else
		{
			if(bStartInMiddle)
				fScaledSize = fPixelRealSize*fScale*fabs(fOrigProgress)*0.5;
			else
				fScaledSize = fPixelRealSize*fScale*fProgress;
			if(bStartInMiddle)
				fScaledSize *= 0.5;
			if(fScaledSize > iCornerH*2.0)
			{
				pDrawingCache->addScalableButton(strFullAnim.c_str(), svFinalPos.x, svFinalPos.y, iRealW, fScaledSize, fOpacity, iCornerW, iCornerH);
				bDidDraw = true;
			}

		}
	}


	if(!bDidDraw && !bStartInMiddle)
	{
		// Now that we know all the dimensions, figure our how much to scale by...
		SUVSet rUVs;
		FLOAT_TYPE fUVActualImageSize;
		if(eStyle == ProgressBarStyleFromLeft || eStyle == ProgressBarStyleFromRight)
			fUVActualImageSize = (FLOAT_TYPE)iRealSize/(FLOAT_TYPE)iBitmapW;
		else
			fUVActualImageSize = (FLOAT_TYPE)iRealSize/(FLOAT_TYPE)iBitmapH;
		FLOAT_TYPE fUVEmptyHalf = (1.0 - fUVActualImageSize)/2.0;
		if(eStyle == ProgressBarStyleFromLeft)
		{
			rUVs.myStartX = fUVEmptyHalf;
			rUVs.myEndX = fUVEmptyHalf + fUVActualImageSize*fProgress;
		}
		else if(eStyle == ProgressBarStyleFromRight)
		{
			rUVs.myEndX = 1.0 - fUVEmptyHalf;
			rUVs.myStartX = 1.0 - (fUVEmptyHalf + fUVActualImageSize*fProgress);
		}
		else if(eStyle == ProgressBarStyleFromBottom)
		{
			rUVs.myEndY = 1.0 - fUVEmptyHalf;
			rUVs.myStartY = 1.0 - (fUVEmptyHalf + fUVActualImageSize*fProgress);
		}
		else if(eStyle == ProgressBarStyleFromTop)
		{
			rUVs.myStartY = fUVEmptyHalf;
			rUVs.myEndY = fUVEmptyHalf + fUVActualImageSize*fProgress;
		}

		FLOAT_TYPE fScaleX, fScaleY;
		if(eStyle == ProgressBarStyleFromLeft || eStyle == ProgressBarStyleFromRight)
		{
			fScaleX = ((FLOAT_TYPE)iRealSize/(FLOAT_TYPE)iBitmapW)*fProgress*fScale;
			fScaleY = fScale;
		}
		else
		{
			fScaleX = fScale;
			fScaleY = ((FLOAT_TYPE)iRealSize/(FLOAT_TYPE)iBitmapH)*fProgress*fScale;
		}

		pDrawingCache->addSprite(strFullAnim.c_str(), svFinalPos.x, svFinalPos.y,
			fOpacity, 0, fScaleX, fScaleY, 0, true, &rUVs);
	}

	return iRealSize;
}
/*****************************************************************************/
#define NUM_CIRCLE_POINTS		36 // 72
#define DEFAULT_CIRC_RADIUS		100.0
SVector2D *g_svCirclePoints = NULL;
SVertexInfo *g_pCircleRendVertices = NULL;

int RenderUtils::generateArcPolys(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius1, FLOAT_TYPE fRadius2, FLOAT_TYPE fThickness,
								   const SColor& scolFill, FLOAT_TYPE fStartAngle, FLOAT_TYPE fEndAngle, CachedSequence* pSeq,
								   int iAtlasFrame, SUVSet& svInitUVs, FLOAT_TYPE fTaper, bool bWrapTexture, int iInitArrayOffset)
{
	if(fRadius2 <= 0.0)
		fRadius2 = fRadius1;

	if(fRadius1 <= 0.0)
		return iInitArrayOffset;

	if(!g_svCirclePoints)
	{
		// Generate the points
		int iCurr, iNum = NUM_CIRCLE_POINTS;
		FLOAT_TYPE fAngle;
		g_svCirclePoints = new SVector2D[NUM_CIRCLE_POINTS + 1];
		for(iCurr = 0; iCurr <= iNum; iCurr++)
		{
			fAngle = (FLOAT_TYPE)iCurr/(FLOAT_TYPE)iNum*M_PI*2.0;
			g_svCirclePoints[iCurr].x = F_COS(fAngle)*DEFAULT_CIRC_RADIUS;
			g_svCirclePoints[iCurr].y = F_SIN(fAngle)*DEFAULT_CIRC_RADIUS;
		}
	}

	if(!g_pCircleRendVertices)
		g_pCircleRendVertices = new SVertexInfo[NUM_CIRCLE_POINTS*8];

	///FLOAT_TYPE fStepArcLength = (360.0/(FLOAT_TYPE)NUM_CIRCLE_POINTS)*M_PI*fRadius/180.0;
	//FLOAT_TYPE fCircumference = 2.0*M_PI*fRadius;
	// Approximate the circumference of an ellipse:
	FLOAT_TYPE fCircumference;
	if(fabs(fRadius1 - fRadius2) < FLOAT_EPSILON)
		fCircumference = 2.0*M_PI*fRadius1;
	else
	{
		FLOAT_TYPE fProd1 = (fRadius1 - fRadius2)/(fRadius1 + fRadius2);
		fProd1 *= fProd1;
		fCircumference = M_PI*(fRadius1 + fRadius2)*(1.0 + (3.0*fProd1)/(10.0 + F_SQRT(4.0 - 3.0*fProd1)));
	}
	FLOAT_TYPE fStepArcLength = fCircumference/(FLOAT_TYPE)NUM_CIRCLE_POINTS;

	// Select the right points and render them.
	FLOAT_TYPE fInterval = 360.0/(FLOAT_TYPE)NUM_CIRCLE_POINTS;
	int iStartPtIndex = fStartAngle/fInterval;
	int iEndPtIndex = fEndAngle/fInterval;
	int iCurrPoint;
	int iArrayPointer;

#ifdef USE_OPENGL2
	SColor scolVertexColor;
	if(pSeq)
		scolVertexColor = scolFill*scolFill.alpha;
	else
		scolVertexColor = scolFill;
	scolVertexColor.alpha = scolFill.alpha;
#else
	unsigned char r,g,b,a;

	if(pSeq)
	{
		r = (int)(scolFill.r*scolFill.alpha*255.0);
		g = (int)(scolFill.g*scolFill.alpha*255.0);
		b = (int)(scolFill.b*scolFill.alpha*255.0);
	}
	else
	{
		r = (int)(scolFill.r*255.0);
		g = (int)(scolFill.g*255.0);
		b = (int)(scolFill.b*255.0);
	}
	a = (int)(scolFill.alpha*255.0);
#endif
	SVector2D svPoly[4], svDir;

	SUVSet svTextFullUVs = svInitUVs;
	FLOAT_TYPE fUStep = (svTextFullUVs.myEndX - svTextFullUVs.myStartX)*fStepArcLength/fCircumference;
	if(!bWrapTexture)
		fUStep = svTextFullUVs.myEndX - svTextFullUVs.myStartX;

	FLOAT_TYPE fFirstV = svTextFullUVs.myStartY;
	FLOAT_TYPE fFinalV = svTextFullUVs.myEndY;
	FLOAT_TYPE fRadiusLessThickness1 = fRadius1 - fThickness;
	FLOAT_TYPE fRadiusLessThickness2 = fRadius2 - fThickness;

	FLOAT_TYPE fSmallestRadLessThick = min(fRadiusLessThickness1, fRadiusLessThickness2);
	if(fSmallestRadLessThick < 0.0)
	{
		fFinalV = (svTextFullUVs.myEndY - svTextFullUVs.myStartY)*(1.0 + fSmallestRadLessThick/fThickness) + svTextFullUVs.myStartY;

		_ASSERT(fFinalV > 0.0 && fFinalV <= 1.0);
		if(fRadiusLessThickness1 < 0.0)
			fRadiusLessThickness1 = 0.0;
		if(fRadiusLessThickness2 < 0.0)
			fRadiusLessThickness2 = 0.0;
	}
	FLOAT_TYPE fCurrUPos = svTextFullUVs.myStartX;
	int iInner;
	for(iCurrPoint = iStartPtIndex + 1, iArrayPointer = iInitArrayOffset; iCurrPoint <= iEndPtIndex; iCurrPoint++, iArrayPointer += 6)
	{
		svPoly[0].x = g_svCirclePoints[iCurrPoint - 1].x*fRadius1/DEFAULT_CIRC_RADIUS + fX;
		svPoly[0].y = g_svCirclePoints[iCurrPoint - 1].y*fRadius2/DEFAULT_CIRC_RADIUS + fY;
		svPoly[1].x = g_svCirclePoints[iCurrPoint].x*fRadius1/DEFAULT_CIRC_RADIUS + fX;
		svPoly[1].y = g_svCirclePoints[iCurrPoint].y*fRadius2/DEFAULT_CIRC_RADIUS + fY;

		// Tapering
		if(fTaper > 0 && (iStartPtIndex != 0 ||  iEndPtIndex != NUM_CIRCLE_POINTS + 1))
		{
			if(iCurrPoint - 1 == iStartPtIndex)
			{
				svDir = svPoly[0] - svPoly[1];
				svDir.normalize();
				svPoly[0] = svPoly[0] + svDir*fTaper;
			}
			else if(iCurrPoint == iEndPtIndex)
			{
				svDir = svPoly[1] - svPoly[0];
				svDir.normalize();
				svPoly[1] = svPoly[1] + svDir*fTaper;
			}
		}

		svPoly[2].x = g_svCirclePoints[iCurrPoint].x*(fRadiusLessThickness1)/DEFAULT_CIRC_RADIUS + fX;
		svPoly[2].y = g_svCirclePoints[iCurrPoint].y*(fRadiusLessThickness2)/DEFAULT_CIRC_RADIUS + fY;
		svPoly[3].x = g_svCirclePoints[iCurrPoint - 1].x*(fRadiusLessThickness1)/DEFAULT_CIRC_RADIUS + fX;
		svPoly[3].y = g_svCirclePoints[iCurrPoint - 1].y*(fRadiusLessThickness2)/DEFAULT_CIRC_RADIUS + fY;

		//	SRect2D srScreen;
		//	GameEngine::getScreenRectRelative(srScreen);
		//	if(srScreen.doesIntersect())


		g_pCircleRendVertices[iArrayPointer + 0].x = svPoly[0].x;
		g_pCircleRendVertices[iArrayPointer + 0].y = svPoly[0].y;
		g_pCircleRendVertices[iArrayPointer + 1].x = svPoly[1].x;
		g_pCircleRendVertices[iArrayPointer + 1].y = svPoly[1].y;
		g_pCircleRendVertices[iArrayPointer + 2].x = svPoly[3].x;
		g_pCircleRendVertices[iArrayPointer + 2].y = svPoly[3].y;

		g_pCircleRendVertices[iArrayPointer + 3].x = svPoly[3].x;
		g_pCircleRendVertices[iArrayPointer + 3].y = svPoly[3].y;
		g_pCircleRendVertices[iArrayPointer + 4].x = svPoly[1].x;
		g_pCircleRendVertices[iArrayPointer + 4].y = svPoly[1].y;
		g_pCircleRendVertices[iArrayPointer + 5].x = svPoly[2].x;
		g_pCircleRendVertices[iArrayPointer + 5].y = svPoly[2].y;

#ifdef USE_OPENGL2
		g_pCircleRendVertices[iArrayPointer + 0].z = 0.0;
		g_pCircleRendVertices[iArrayPointer + 0].w = 1.0;
		g_pCircleRendVertices[iArrayPointer + 1].z = 0.0;
		g_pCircleRendVertices[iArrayPointer + 1].w = 1.0;
		g_pCircleRendVertices[iArrayPointer + 2].z = 0.0;
		g_pCircleRendVertices[iArrayPointer + 2].w = 1.0;

		g_pCircleRendVertices[iArrayPointer + 3].z = 0.0;
		g_pCircleRendVertices[iArrayPointer + 3].w = 1.0;
		g_pCircleRendVertices[iArrayPointer + 4].z = 0.0;
		g_pCircleRendVertices[iArrayPointer + 4].w = 1.0;
		g_pCircleRendVertices[iArrayPointer + 5].z = 0.0;
		g_pCircleRendVertices[iArrayPointer + 5].w = 1.0;
#endif

		// Texture coords
		if(pSeq)
		{
			g_pCircleRendVertices[iArrayPointer + 0].s0 = fCurrUPos;
			g_pCircleRendVertices[iArrayPointer + 0].t0 = fFirstV;
			g_pCircleRendVertices[iArrayPointer + 1].s0 = fCurrUPos + fUStep;
			g_pCircleRendVertices[iArrayPointer + 1].t0 = fFirstV;
			g_pCircleRendVertices[iArrayPointer + 2].s0 = fCurrUPos;
			g_pCircleRendVertices[iArrayPointer + 2].t0 = fFinalV;

			g_pCircleRendVertices[iArrayPointer + 3].s0 = fCurrUPos;
			g_pCircleRendVertices[iArrayPointer + 3].t0 = fFinalV;
			g_pCircleRendVertices[iArrayPointer + 4].s0 = fCurrUPos + fUStep;
			g_pCircleRendVertices[iArrayPointer + 4].t0 = fFirstV;
			g_pCircleRendVertices[iArrayPointer + 5].s0 = fCurrUPos + fUStep;
			g_pCircleRendVertices[iArrayPointer + 5].t0 = fFinalV;
		}
#ifdef DIRECTX_PIPELINE
		else
		{
			g_pCircleRendVertices[iArrayPointer + 0].s0 = -1;
			g_pCircleRendVertices[iArrayPointer + 0].t0 = -1;
			g_pCircleRendVertices[iArrayPointer + 1].s0 = -1;
			g_pCircleRendVertices[iArrayPointer + 1].t0 = -1;
			g_pCircleRendVertices[iArrayPointer + 2].s0 = -1;
			g_pCircleRendVertices[iArrayPointer + 2].t0 = -1;

			g_pCircleRendVertices[iArrayPointer + 3].s0 = -1;
			g_pCircleRendVertices[iArrayPointer + 3].t0 = -1;
			g_pCircleRendVertices[iArrayPointer + 4].s0 = -1;
			g_pCircleRendVertices[iArrayPointer + 4].t0 = -1;
			g_pCircleRendVertices[iArrayPointer + 5].s0 = -1;
			g_pCircleRendVertices[iArrayPointer + 5].t0 = -1;
		}
#endif

		for(iInner = 0; iInner < 6;  iInner++)
		{
#ifdef USE_OPENGL2
			g_pCircleRendVertices[iArrayPointer + iInner].r = scolVertexColor.r;
			g_pCircleRendVertices[iArrayPointer + iInner].g = scolVertexColor.g;
			g_pCircleRendVertices[iArrayPointer + iInner].b = scolVertexColor.b;
			g_pCircleRendVertices[iArrayPointer + iInner].a = scolVertexColor.alpha;
#else
			g_pCircleRendVertices[iArrayPointer + iInner].r = r;
			g_pCircleRendVertices[iArrayPointer + iInner].g = g;
			g_pCircleRendVertices[iArrayPointer + iInner].b = b;
			g_pCircleRendVertices[iArrayPointer + iInner].a = a;
#endif
		}

		if(bWrapTexture)
			fCurrUPos += fUStep;
	}

	return iArrayPointer;
}

void RenderUtils::renderArc(Window* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius, FLOAT_TYPE fThickness,
							const SColor& scolFill, FLOAT_TYPE fStartAngle, FLOAT_TYPE fEndAngle, const char* pcsTexture,
							FLOAT_TYPE fTaper, bool bWrapTexture, bool bManageState, FLOAT_TYPE fRadius2)
{
	if(fRadius <= 0.0)
		return;

	DrawingCache* pDrawingCache = pWindow->getDrawingCache();
	TextureManager* pTextureManager = pWindow->getTextureManager();

	BlendModeType eOldMode = BlendModeLastPlaceholder;
	if(!pcsTexture && bManageState)
	{
		eOldMode = pWindow->setBlendMode(BlendModeText);
		GraphicsUtils::enableFlatDrawingState(-1.0);
	}
	int iAtlasFrame;
	CachedSequence* pSeq = NULL;

	if(pcsTexture)
		pSeq = pDrawingCache->getCachedSequence(pcsTexture, &iAtlasFrame);

	SUVSet svTextFullUVs;
	if(pcsTexture)
	{
		int iW, iH;
		TX_MAN_RETURN_TYPE iIndex;
		BlendModeType eBlendMode, ePrevMode;
		iIndex = pTextureManager->getTextureIndex(pSeq->getName(), iAtlasFrame >= 0 ? iAtlasFrame : 0, iW, iH, svTextFullUVs, eBlendMode);
		ePrevMode = pWindow->setBlendMode(eBlendMode);
		if(eOldMode == BlendModeLastPlaceholder)
			eOldMode = ePrevMode;
#ifdef DIRECTX_PIPELINE
		g_pDxRenderer->getD3dContext()->PSSetShaderResources(0, 1, iIndex);
#else
		glBindTexture(GL_TEXTURE_2D, iIndex);
#endif
	}

	int iArrayPointer = generateArcPolys(fX, fY, fRadius, fRadius2, fThickness, scolFill, fStartAngle, fEndAngle, pSeq,
								  iAtlasFrame, svTextFullUVs, fTaper, bWrapTexture, 0);

	if(iArrayPointer > 0)
	{
#ifdef DIRECTX_PIPELINE
	_ASSERT(iArrayPointer % 3 == 0);
	myDxBuffer.setFromVertexArray(g_pCircleRendVertices, iArrayPointer/3);

	UINT stride = sizeof(BasicVertex);
	UINT offset = 0;
	g_pDxRenderer->getD3dContext()->IASetVertexBuffers(0, 1, myDxBuffer.getVertexBuffer(), &stride, &offset);
	g_pDxRenderer->getD3dContext()->IASetIndexBuffer(myDxBuffer.getIndexBuffer(), INDEX_BUFFER_FORMAT,	0);
	g_pDxRenderer->getD3dContext()->DrawIndexed(iArrayPointer, 0, 0);

#else

#ifdef USE_OPENGL2
	RenderUtils::drawBuffer<SVertexInfo>(&g_pCircleRendVertices[0].x, SVertexInfo::getNumVertexComponents(), pcsTexture ? &g_pCircleRendVertices[0].s0 : NULL, &g_pCircleRendVertices[0].r, NULL, iArrayPointer);
#else

#ifdef USE_INT_OPENGL_POSITIONS
	OpenGLStateCleaner::doVertexPointerCall(2, GL_SHORT, sizeof(SVertexInfo), &g_pCircleRendVertices[0].x);
#else
	OpenGLStateCleaner::doVertexPointerCall(2, GL_FLOAT, sizeof(SVertexInfo), &g_pCircleRendVertices[0].x);
#endif
	if(pcsTexture)
		OpenGLStateCleaner::doTexturePointerCall(2, GL_FLOAT, sizeof(SVertexInfo), &g_pCircleRendVertices[0].s0);
	OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, sizeof(SVertexInfo), &g_pCircleRendVertices[0].r);

	glDrawArrays(GL_TRIANGLES, 0, iArrayPointer);
#endif

#endif
	}

	if(!pcsTexture && bManageState)
	{
		GraphicsUtils::disableFlatDrawingState(-1.0);
	}

	if(eOldMode != BlendModeLastPlaceholder)
		pWindow->setBlendMode(eOldMode);
}
/*****************************************************************************/
void RenderUtils::renderDashedCircle(Window* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius,
									 FLOAT_TYPE fThickness, SColor& scolFill, FLOAT_TYPE fSolidDashAngle, FLOAT_TYPE fEmptyDashAngle, const char* pcsTexture, bool bManageState)
{
	RenderUtils::renderDashedEllipse(pWindow, fX, fY, fRadius, fRadius,
		fThickness, scolFill, fSolidDashAngle, fEmptyDashAngle, pcsTexture, bManageState);
}
/*****************************************************************************/
void RenderUtils::renderDashedEllipse(Window* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius1, FLOAT_TYPE fRadius2,
									 FLOAT_TYPE fThickness, SColor& scolFill, FLOAT_TYPE fSolidDashAngle, FLOAT_TYPE fEmptyDashAngle, const char* pcsTexture, bool bManageState)
{
	TextureManager* pTextureManager = pWindow->getTextureManager();
	DrawingCache* pDrawingCache = pWindow->getDrawingCache();

	BlendModeType eOldMode = BlendModeLastPlaceholder;
	if(!pcsTexture && bManageState)
	{
		eOldMode = pWindow->setBlendMode(BlendModeText);
		GraphicsUtils::enableFlatDrawingState(-1.0);
	}

	int iAtlasFrame;
	CachedSequence* pSeq = NULL;
	SUVSet svTextFullUVs;
	if(pcsTexture)
	{
		pSeq = pDrawingCache->getCachedSequence(pcsTexture, &iAtlasFrame);

		int iW, iH;
		TX_MAN_RETURN_TYPE iIndex;
		BlendModeType eBlendMode, ePrevMode;
		iIndex = pTextureManager->getTextureIndex(pcsTexture, iAtlasFrame >= 0 ? iAtlasFrame : 0, iW, iH, svTextFullUVs, eBlendMode);
		ePrevMode = pWindow->setBlendMode(eBlendMode);
		if(eOldMode == BlendModeLastPlaceholder)
			eOldMode = ePrevMode;
#ifdef DIRECTX_PIPELINE
		g_pDxRenderer->getD3dContext()->PSSetShaderResources(0, 1, iIndex);
#else
		glBindTexture(GL_TEXTURE_2D, iIndex);
#endif
	}

	// Draw a bunch of arcs
	FLOAT_TYPE fStartAngle;
	int iArrayOffset = 0;
	for(fStartAngle = 0; fStartAngle < 360.0; fStartAngle += fSolidDashAngle + fEmptyDashAngle)
	{
		iArrayOffset = generateArcPolys(fX, fY, fRadius1, fRadius2, fThickness, scolFill, fStartAngle, fStartAngle + fSolidDashAngle, pSeq,
			iAtlasFrame, svTextFullUVs, false, false, iArrayOffset);

///		renderArc(fX, fY, fRadius, fThickness, scolFill, fStartAngle, fStartAngle + fSolidDashAngle, pcsTexture, 0, false, false);
	}

	if(iArrayOffset > 0)
	{
#ifdef DIRECTX_PIPELINE
	_ASSERT(iArrayOffset % 3 == 0);
	myDxBuffer.setFromVertexArray(g_pCircleRendVertices, iArrayOffset/3);

	UINT stride = sizeof(BasicVertex);
	UINT offset = 0;
	g_pDxRenderer->getD3dContext()->IASetVertexBuffers(0, 1, myDxBuffer.getVertexBuffer(), &stride, &offset);
	g_pDxRenderer->getD3dContext()->IASetIndexBuffer(myDxBuffer.getIndexBuffer(), INDEX_BUFFER_FORMAT,	0);
	g_pDxRenderer->getD3dContext()->DrawIndexed(iArrayOffset, 0, 0);
#else

#ifdef USE_OPENGL2
	RenderUtils::drawBuffer<SVertexInfo>(&g_pCircleRendVertices[0].x, SVertexInfo::getNumVertexComponents(), pcsTexture ? &g_pCircleRendVertices[0].s0 : NULL, &g_pCircleRendVertices[0].r, NULL, iArrayOffset);
#else

#ifdef USE_INT_OPENGL_POSITIONS
	OpenGLStateCleaner::doVertexPointerCall(2, GL_SHORT, sizeof(SVertexInfo), &g_pCircleRendVertices[0].x);
#else
	OpenGLStateCleaner::doVertexPointerCall(2, GL_FLOAT, sizeof(SVertexInfo), &g_pCircleRendVertices[0].x);
#endif
	if(pcsTexture)
		OpenGLStateCleaner::doTexturePointerCall(2, GL_FLOAT, sizeof(SVertexInfo), &g_pCircleRendVertices[0].s0);
	OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, sizeof(SVertexInfo), &g_pCircleRendVertices[0].r);

	glDrawArrays(GL_TRIANGLES, 0, iArrayOffset);

#endif

#endif
	}

	if(!pcsTexture && bManageState)
	{
		GraphicsUtils::disableFlatDrawingState(-1.0);
	}

	if(eOldMode != BlendModeLastPlaceholder)
		pWindow->setBlendMode(eOldMode);
}
/*****************************************************************************/
//SVertex3DInfo
SVertex3D *g_pCircleRendVertices3D = NULL;

int RenderUtils::generateArc3DPolys(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ, FLOAT_TYPE fRadius1, FLOAT_TYPE fRadius2, FLOAT_TYPE fThickness,
								  SColor& scolFill, FLOAT_TYPE fStartAngle, FLOAT_TYPE fEndAngle, CachedSequence* pSeq,
								  int iAtlasFrame, SUVSet& svInitUVs, FLOAT_TYPE fTaper, bool bWrapTexture, int iInitArrayOffset)
{
#if 1
	if(fRadius2 <= 0.0)
		fRadius2 = fRadius1;

	if(fRadius1 <= 0.0)
		return iInitArrayOffset;

	if(!g_svCirclePoints)
	{
		// Generate the points
		int iCurr, iNum = NUM_CIRCLE_POINTS;
		FLOAT_TYPE fAngle;
		g_svCirclePoints = new SVector2D[NUM_CIRCLE_POINTS + 1];
		for(iCurr = 0; iCurr <= iNum; iCurr++)
		{
			fAngle = (FLOAT_TYPE)iCurr/(FLOAT_TYPE)iNum*M_PI*2.0;
			g_svCirclePoints[iCurr].x = F_COS(fAngle)*DEFAULT_CIRC_RADIUS;
			g_svCirclePoints[iCurr].y = F_SIN(fAngle)*DEFAULT_CIRC_RADIUS;
		}
	}

	if(!g_pCircleRendVertices3D)
		g_pCircleRendVertices3D = new SVertex3D[NUM_CIRCLE_POINTS*8];

	///FLOAT_TYPE fStepArcLength = (360.0/(FLOAT_TYPE)NUM_CIRCLE_POINTS)*M_PI*fRadius/180.0;
	//FLOAT_TYPE fCircumference = 2.0*M_PI*fRadius;
	// Approximate the circumference of an ellipse:
	FLOAT_TYPE fCircumference;
	if(fabs(fRadius1 - fRadius2) < FLOAT_EPSILON)
		fCircumference = 2.0*M_PI*fRadius1;
	else
	{
		FLOAT_TYPE fProd1 = (fRadius1 - fRadius2)/(fRadius1 + fRadius2);
		fProd1 *= fProd1;
		fCircumference = M_PI*(fRadius1 + fRadius2)*(1.0 + (3.0*fProd1)/(10.0 + F_SQRT(4.0 - 3.0*fProd1)));
	}
	FLOAT_TYPE fStepArcLength = fCircumference/(FLOAT_TYPE)NUM_CIRCLE_POINTS;

	// Select the right points and render them.
	FLOAT_TYPE fInterval = 360.0/(FLOAT_TYPE)NUM_CIRCLE_POINTS;
	int iStartPtIndex = fStartAngle/fInterval;
	int iEndPtIndex = fEndAngle/fInterval;
	int iCurrPoint;
	int iArrayPointer;

	unsigned char r,g,b,a;

	if(pSeq)
	{
		r = (int)(scolFill.r*scolFill.alpha*255.0);
		g = (int)(scolFill.g*scolFill.alpha*255.0);
		b = (int)(scolFill.b*scolFill.alpha*255.0);
	}
	else
	{
		r = (int)(scolFill.r*255.0);
		g = (int)(scolFill.g*255.0);
		b = (int)(scolFill.b*255.0);
	}
	a = (int)(scolFill.alpha*255.0);

	SVector2D svPoly[4], svDir;

	SUVSet svTextFullUVs = svInitUVs;
	FLOAT_TYPE fUStep = (svTextFullUVs.myEndX - svTextFullUVs.myStartX)*fStepArcLength/fCircumference;
	if(!bWrapTexture)
		fUStep = svTextFullUVs.myEndX - svTextFullUVs.myStartX;

	FLOAT_TYPE fFirstV = svTextFullUVs.myStartY;
	FLOAT_TYPE fFinalV = svTextFullUVs.myEndY;
	FLOAT_TYPE fRadiusLessThickness1 = fRadius1 - fThickness;
	FLOAT_TYPE fRadiusLessThickness2 = fRadius2 - fThickness;

	FLOAT_TYPE fSmallestRadLessThick = min(fRadiusLessThickness1, fRadiusLessThickness2);
	if(fSmallestRadLessThick < 0.0)
	{
		fFinalV = (svTextFullUVs.myEndY - svTextFullUVs.myStartY)*(1.0 + fSmallestRadLessThick/fThickness) + svTextFullUVs.myStartY;

		_ASSERT(fFinalV > 0.0 && fFinalV <= 1.0);
		if(fRadiusLessThickness1 < 0.0)
			fRadiusLessThickness1 = 0.0;
		if(fRadiusLessThickness2 < 0.0)
			fRadiusLessThickness2 = 0.0;
	}
	FLOAT_TYPE fCurrUPos = svTextFullUVs.myStartX;
	int iInner;
	for(iCurrPoint = iStartPtIndex + 1, iArrayPointer = iInitArrayOffset; iCurrPoint <= iEndPtIndex; iCurrPoint++, iArrayPointer += 6)
	{
		svPoly[0].x = g_svCirclePoints[iCurrPoint - 1].x*fRadius1/DEFAULT_CIRC_RADIUS + fX;
		svPoly[0].y = g_svCirclePoints[iCurrPoint - 1].y*fRadius2/DEFAULT_CIRC_RADIUS + fY;
		svPoly[1].x = g_svCirclePoints[iCurrPoint].x*fRadius1/DEFAULT_CIRC_RADIUS + fX;
		svPoly[1].y = g_svCirclePoints[iCurrPoint].y*fRadius2/DEFAULT_CIRC_RADIUS + fY;

		// Tapering
		if(fTaper > 0 && (iStartPtIndex != 0 ||  iEndPtIndex != NUM_CIRCLE_POINTS + 1))
		{
			if(iCurrPoint - 1 == iStartPtIndex)
			{
				svDir = svPoly[0] - svPoly[1];
				svDir.normalize();
				svPoly[0] = svPoly[0] + svDir*fTaper;
			}
			else if(iCurrPoint == iEndPtIndex)
			{
				svDir = svPoly[1] - svPoly[0];
				svDir.normalize();
				svPoly[1] = svPoly[1] + svDir*fTaper;
			}
		}

		svPoly[2].x = g_svCirclePoints[iCurrPoint].x*(fRadiusLessThickness1)/DEFAULT_CIRC_RADIUS + fX;
		svPoly[2].y = g_svCirclePoints[iCurrPoint].y*(fRadiusLessThickness2)/DEFAULT_CIRC_RADIUS + fY;
		svPoly[3].x = g_svCirclePoints[iCurrPoint - 1].x*(fRadiusLessThickness1)/DEFAULT_CIRC_RADIUS + fX;
		svPoly[3].y = g_svCirclePoints[iCurrPoint - 1].y*(fRadiusLessThickness2)/DEFAULT_CIRC_RADIUS + fY;

		//	SRect2D srScreen;
		//	GameEngine::getScreenRectRelative(srScreen);
		//	if(srScreen.doesIntersect())


		g_pCircleRendVertices3D[iArrayPointer + 0].x = svPoly[0].x;
		g_pCircleRendVertices3D[iArrayPointer + 0].y = svPoly[0].y;
		g_pCircleRendVertices3D[iArrayPointer + 0].z = fZ;
		g_pCircleRendVertices3D[iArrayPointer + 1].x = svPoly[1].x;
		g_pCircleRendVertices3D[iArrayPointer + 1].y = svPoly[1].y;
		g_pCircleRendVertices3D[iArrayPointer + 1].z = fZ;
		g_pCircleRendVertices3D[iArrayPointer + 2].x = svPoly[3].x;
		g_pCircleRendVertices3D[iArrayPointer + 2].y = svPoly[3].y;
		g_pCircleRendVertices3D[iArrayPointer + 2].z = fZ;
#ifdef USE_OPENGL2
		g_pCircleRendVertices3D[iArrayPointer + 0].w = 1;
		g_pCircleRendVertices3D[iArrayPointer + 1].w = 1;
		g_pCircleRendVertices3D[iArrayPointer + 2].w = 1;
#endif


		g_pCircleRendVertices3D[iArrayPointer + 3].x = svPoly[3].x;
		g_pCircleRendVertices3D[iArrayPointer + 3].y = svPoly[3].y;
		g_pCircleRendVertices3D[iArrayPointer + 3].z = fZ;
		g_pCircleRendVertices3D[iArrayPointer + 4].x = svPoly[1].x;
		g_pCircleRendVertices3D[iArrayPointer + 4].y = svPoly[1].y;
		g_pCircleRendVertices3D[iArrayPointer + 4].z = fZ;
		g_pCircleRendVertices3D[iArrayPointer + 5].x = svPoly[2].x;
		g_pCircleRendVertices3D[iArrayPointer + 5].y = svPoly[2].y;
		g_pCircleRendVertices3D[iArrayPointer + 5].z = fZ;
#ifdef USE_OPENGL2
		g_pCircleRendVertices3D[iArrayPointer + 3].w = 1;
		g_pCircleRendVertices3D[iArrayPointer + 4].w = 1;
		g_pCircleRendVertices3D[iArrayPointer + 5].w = 1;
#endif

		g_pCircleRendVertices3D[iArrayPointer + 0].nx = 0;
		g_pCircleRendVertices3D[iArrayPointer + 0].ny = 0;
		g_pCircleRendVertices3D[iArrayPointer + 0].nz = 1;

		g_pCircleRendVertices3D[iArrayPointer + 1].nx = 0;
		g_pCircleRendVertices3D[iArrayPointer + 1].ny = 0;
		g_pCircleRendVertices3D[iArrayPointer + 1].nz = 1;

		g_pCircleRendVertices3D[iArrayPointer + 2].nx = 0;
		g_pCircleRendVertices3D[iArrayPointer + 2].ny = 0;
		g_pCircleRendVertices3D[iArrayPointer + 2].nz = 1;

		g_pCircleRendVertices3D[iArrayPointer + 3].nx = 0;
		g_pCircleRendVertices3D[iArrayPointer + 3].ny = 0;
		g_pCircleRendVertices3D[iArrayPointer + 3].nz = 1;

		g_pCircleRendVertices3D[iArrayPointer + 4].nx = 0;
		g_pCircleRendVertices3D[iArrayPointer + 4].ny = 0;
		g_pCircleRendVertices3D[iArrayPointer + 4].nz = 1;

		g_pCircleRendVertices3D[iArrayPointer + 5].nx = 0;
		g_pCircleRendVertices3D[iArrayPointer + 5].ny = 0;
		g_pCircleRendVertices3D[iArrayPointer + 5].nz = 1;


		// Texture coords
		if(pSeq)
		{
/*
			g_pCircleRendVertices3D[iArrayPointer + 0].s0 = fCurrUPos;
			g_pCircleRendVertices3D[iArrayPointer + 0].t0 = fFirstV;
			g_pCircleRendVertices3D[iArrayPointer + 1].s0 = fCurrUPos + fUStep;
			g_pCircleRendVertices3D[iArrayPointer + 1].t0 = fFirstV;
			g_pCircleRendVertices3D[iArrayPointer + 2].s0 = fCurrUPos;
			g_pCircleRendVertices3D[iArrayPointer + 2].t0 = fFinalV;

			g_pCircleRendVertices3D[iArrayPointer + 3].s0 = fCurrUPos;
			g_pCircleRendVertices3D[iArrayPointer + 3].t0 = fFinalV;
			g_pCircleRendVertices3D[iArrayPointer + 4].s0 = fCurrUPos + fUStep;
			g_pCircleRendVertices3D[iArrayPointer + 4].t0 = fFirstV;
			g_pCircleRendVertices3D[iArrayPointer + 5].s0 = fCurrUPos + fUStep;
			g_pCircleRendVertices3D[iArrayPointer + 5].t0 = fFinalV;
*/
			g_pCircleRendVertices3D[iArrayPointer + 0].u = fCurrUPos;
			g_pCircleRendVertices3D[iArrayPointer + 0].v = fFirstV;
			g_pCircleRendVertices3D[iArrayPointer + 1].u = fCurrUPos + fUStep;
			g_pCircleRendVertices3D[iArrayPointer + 1].v = fFirstV;
			g_pCircleRendVertices3D[iArrayPointer + 2].u = fCurrUPos;
			g_pCircleRendVertices3D[iArrayPointer + 2].v = fFinalV;

			g_pCircleRendVertices3D[iArrayPointer + 3].u = fCurrUPos;
			g_pCircleRendVertices3D[iArrayPointer + 3].v = fFinalV;
			g_pCircleRendVertices3D[iArrayPointer + 4].u = fCurrUPos + fUStep;
			g_pCircleRendVertices3D[iArrayPointer + 4].v = fFirstV;
			g_pCircleRendVertices3D[iArrayPointer + 5].u = fCurrUPos + fUStep;
			g_pCircleRendVertices3D[iArrayPointer + 5].v = fFinalV;

		}

		for(iInner = 0; iInner < 6;  iInner++)
		{
/*
			g_pCircleRendVertices3D[iArrayPointer + iInner].r = r;
			g_pCircleRendVertices3D[iArrayPointer + iInner].g = g;
			g_pCircleRendVertices3D[iArrayPointer + iInner].b = b;
			g_pCircleRendVertices3D[iArrayPointer + iInner].a = a;
*/
		}

		if(bWrapTexture)
			fCurrUPos += fUStep;
	}

	return iArrayPointer;
#else
	return 0;
#endif
}

void RenderUtils::renderArc3D(Window* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ, FLOAT_TYPE fRadius,
							  FLOAT_TYPE fThickness, SColor& scolFill, FLOAT_TYPE fStartAngle, FLOAT_TYPE fEndAngle, const char* pcsTexture, FLOAT_TYPE fTaper, bool bWrapTexture, bool bManageState, bool bFlatOnGround)
{
	if(fRadius <= 0.0)
		return;

	TextureManager* pTextureManager = pWindow->getTextureManager();
	DrawingCache* pDrawingCache = pWindow->getDrawingCache();
	BlendModeType eOldMode = BlendModeLastPlaceholder;
	if(!pcsTexture && bManageState)
	{
		eOldMode = pWindow->setBlendMode(BlendModeText);
		GraphicsUtils::enableFlatDrawingState(-1.0);
	}
	int iAtlasFrame;
	CachedSequence* pSeq = NULL;

	if(pcsTexture)
		pSeq = pDrawingCache->getCachedSequence(pcsTexture, &iAtlasFrame);

	SUVSet svTextFullUVs;
	if(pcsTexture)
	{
		int iW, iH;
		TX_MAN_RETURN_TYPE iIndex;
		BlendModeType eBlendMode, ePrevMode;
		iIndex = pTextureManager->getTextureIndex(pSeq->getName(), iAtlasFrame >= 0 ? iAtlasFrame : 0, iW, iH, svTextFullUVs, eBlendMode);
		ePrevMode = pWindow->setBlendMode(eBlendMode);
		if(eOldMode == BlendModeLastPlaceholder)
			eOldMode = ePrevMode;
#ifdef DIRECTX_PIPELINE
		g_pDxRenderer->getD3dContext()->PSSetShaderResources(0, 1, iIndex);
#else
		glBindTexture(GL_TEXTURE_2D, iIndex);
#endif
	}

	// We pass 0 for Z since we might have to rotate if we're not flat.
	int iArrayPointer = generateArc3DPolys(0, 0, 0, fRadius, 0, fThickness, scolFill, fStartAngle, fEndAngle, pSeq,
		iAtlasFrame, svTextFullUVs, fTaper, bWrapTexture, 0);

#ifdef DIRECTX_PIPELINE
	DXSTAGE3
#else

#ifdef USE_OPENGL2
	SMatrix4 smatModel;
	smatModel.appendTranslation(fX, fY, fZ);
	if(bFlatOnGround == false)
		smatModel.appendXRotation(sanitizeDegAngle(-PERSP_CAMERA_ANGLE));
	RenderStateManager::getInstance()->setModelMatrix(smatModel);

	SColor scolTemp, svSpec(0,0,0,0);
	scolTemp = scolFill;
	scolTemp.alpha = scolFill.alpha;
	setMaterial(scolTemp, scolTemp, svSpec, 1.0);

	RenderUtils::drawBuffer<SVertex3D>(&g_pCircleRendVertices3D[0].x, SVertex3D::getNumVertexComponents(), pcsTexture ? &g_pCircleRendVertices3D[0].u : NULL, NULL, &g_pCircleRendVertices3D[0].nx, iArrayPointer);

#else
	glPushMatrix();
	glTranslatef(fX, fY, fZ);
	if(bFlatOnGround == false)
	{
		glRotatef(-PERSP_CAMERA_ANGLE, 1, 0, 0);
	}



	SColor scolTemp;
	scolTemp = scolFill;
	scolTemp.alpha = scolFill.alpha;
	float colArray[4] = { 1,1,1, scolTemp.alpha };

	colArray[0] = scolTemp.r;
	colArray[1] = scolTemp.g;
	colArray[2] = scolTemp.b;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, colArray);

	colArray[0] = scolTemp.r;
	colArray[1] = scolTemp.g;
	colArray[2] = scolTemp.b;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, colArray);

	OpenGLStateCleaner::doVertexPointerCall(3, GL_FLOAT, sizeof(SVertex3D), &g_pCircleRendVertices3D[0].x);
	glNormalPointer(GL_FLOAT, sizeof(SVertex3D), &g_pCircleRendVertices3D[0].nx);
	if(pcsTexture)
		OpenGLStateCleaner::doTexturePointerCall(2, GL_FLOAT, sizeof(SVertex3D), &g_pCircleRendVertices3D[0].u);
//	OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, sizeof(SVertex3DInfo), &g_pCircleRendVertices3D[0].r);

	glDrawArrays(GL_TRIANGLES, 0, iArrayPointer);
#endif


#ifdef USE_OPENGL2
	RenderStateManager::getInstance()->resetMaterials();
	RenderStateManager::getInstance()->resetModelMatrix();
#else
	glPopMatrix();
#endif

	if(!pcsTexture && bManageState)
	{
		GraphicsUtils::disableFlatDrawingState(-1.0);
	}

	if(eOldMode != BlendModeLastPlaceholder)
		pWindow->setBlendMode(eOldMode);
#endif
}
/*****************************************************************************/
SVertexInfo *g_pTexPathRendVertices = NULL;
int g_iNumTexPathRenderVertices = 0;

struct SRendDistInfos
{
	FLOAT_TYPE mySide1, mySide2;
	FLOAT_TYPE myExtra, myNormalUCoord;
};
SRendDistInfos *g_sDistInfos = NULL;

#define MITER_UVS_VERSION1

void RenderUtils::renderTexturedPath(Window* pWindow, TPointVector& rPoints, FLOAT_TYPE fThickness, const char* pcsTexture, FLOAT_TYPE fOpacity, bool bStretchTexture, bool bMiter, FLOAT_TYPE fMaxKinkMult, const SColor* pFillCol, FLOAT_TYPE fTextureLengthMult, FLOAT_TYPE fInitUOffset, const SVector2D& svOffset)
{
	int iCurrPoint, iNumPoints = rPoints.size();
	if(iNumPoints < 2)
		return;

	TextureManager* pTextureManager = pWindow->getTextureManager();
	DrawingCache* pDrawingCache = pWindow->getDrawingCache();
	int iVertsNeeded = (iNumPoints - 1)*6;
	// We may or may not need more points, but if we do,
	// it's going to be at most 3 more per corner.
	if(bMiter && iNumPoints > 2)
		iVertsNeeded += (iNumPoints - 2)*3;

	BlendModeType eOldMode = BlendModeLastPlaceholder;
	if(g_iNumTexPathRenderVertices < iVertsNeeded)
	{
		// Reallocate the array
		if(g_pTexPathRendVertices)
			delete[] g_pTexPathRendVertices;
		g_pTexPathRendVertices = new SVertexInfo[iVertsNeeded];
		g_iNumTexPathRenderVertices = iVertsNeeded;

		if(g_sDistInfos)
			delete[] g_sDistInfos;
		g_sDistInfos = new SRendDistInfos[iVertsNeeded];
	}

	// Get the right texture
	int iAtlasFrame;
	CachedSequence* pSeq = pDrawingCache->getCachedSequence(pcsTexture, &iAtlasFrame);


	if(!pSeq)
	{
		// Invalid texture
		_ASSERT(0);
	}

	SUVSet svTextFullUVs;
	if(pcsTexture)
	{
		int iW, iH;
		TX_MAN_RETURN_TYPE iIndex;
		BlendModeType eBlendMode, ePrevMode;
		iIndex = pTextureManager->getTextureIndex(pSeq->getName(), iAtlasFrame >= 0 ? iAtlasFrame : 0, iW, iH, svTextFullUVs, eBlendMode);
		ePrevMode = pWindow->setBlendMode(eBlendMode);
		if(eOldMode == BlendModeLastPlaceholder)
			eOldMode = ePrevMode;
#ifdef DIRECTX_PIPELINE
		g_pDxRenderer->getD3dContext()->PSSetShaderResources(0, 1, iIndex); 
#else
		glBindTexture(GL_TEXTURE_2D, iIndex);
#endif
	}

	int iRealW, iFileW, iRealH;
	if(iAtlasFrame >= 0)
		iRealW = pTextureManager->getWidth(pSeq->getName());
	else
		pTextureManager->getTextureRealDims(pcsTexture, iRealW, iRealH);
	iFileW = pTextureManager->getFileWidth(pSeq->getName());
	// Note: This assumes a single frame file?
	FLOAT_TYPE fUOffset = 0.0;
	if(iAtlasFrame < 0)
		fUOffset = (1.0 - (FLOAT_TYPE)iRealW/(FLOAT_TYPE)iFileW)/2.0;

	svTextFullUVs.myStartX += fUOffset;
	svTextFullUVs.myEndX -= fUOffset;

	//FINISH - WTH is going on with flipping

	// Select the right points and render them.
	int iArrayPointer = 0;
	SColor scolFill(1,1,1,fOpacity);

#ifdef USE_OPENGL2
	SColor scolVertexColor;
	if(pFillCol)
		scolVertexColor = (*pFillCol)*scolFill.alpha;
	else
		scolVertexColor = scolFill*scolFill.alpha;
	scolVertexColor.alpha = scolFill.alpha;
#else
	unsigned char r,g,b,a;

	if(pFillCol)
	{
		r = (int)(pFillCol->r*scolFill.alpha*255.0);
		g = (int)(pFillCol->g*scolFill.alpha*255.0);
		b = (int)(pFillCol->b*scolFill.alpha*255.0);
	}
	else
	{
		r = (int)(scolFill.r*scolFill.alpha*255.0);
		g = (int)(scolFill.g*scolFill.alpha*255.0);
		b = (int)(scolFill.b*scolFill.alpha*255.0);
	}
/*
	else
	{
		r = (int)(scolFill.r*255.0);
		g = (int)(scolFill.g*255.0);
		b = (int)(scolFill.b*255.0);
	}
*/
	a = (int)(scolFill.alpha*255.0);
#endif
	SVector2D svPoly[4];

///	FLOAT_TYPE fUStep = (svTextFullUVs.myEndX - svTextFullUVs.myStartX)*fStepArcLength/fCircumference;
//	if(!bWrapTexture)
//		fUStep = svTextFullUVs.myEndX - svTextFullUVs.myStartX;

//	FLOAT_TYPE fFirstV = svTextFullUVs.myStartY;
//	FLOAT_TYPE fFinalV = svTextFullUVs.myEndY;
	// Swap so we look normal.
	FLOAT_TYPE fFirstV = svTextFullUVs.myEndY;
	FLOAT_TYPE fFinalV = svTextFullUVs.myStartY;

	FLOAT_TYPE fHalfThickness = fThickness*0.5;
	SVector2D svDir, svNorm;
	FLOAT_TYPE fCurrUPos = svTextFullUVs.myStartX + fInitUOffset;
	FLOAT_TYPE fCurrUPos2, fUStep2;
	FLOAT_TYPE fUStep = (svTextFullUVs.myEndX - svTextFullUVs.myStartX)/fTextureLengthMult;
	FLOAT_TYPE fFullUStep = fUStep;
	//FLOAT_TYPE fScaledRealW = (FLOAT_TYPE)iRealW*fTextureLengthMult;
	int iInner;

	FLOAT_TYPE fTempDist;
//	FLOAT_TYPE fSingTwo, fSignOne;
	FLOAT_TYPE fTotalLength = 0;
	FLOAT_TYPE fTotalLength1 = 0;
	FLOAT_TYPE fTotalLength2 = 0;
	if(bStretchTexture)
	{
		// For stretching, we need to know the total length.
		for(iCurrPoint = 1; iCurrPoint < iNumPoints; iCurrPoint++)
			fTotalLength += (rPoints[iCurrPoint] - rPoints[iCurrPoint-1]).length();
	}

	SVector2D svPrevNorm, svPrevDir;

	FLOAT_TYPE fMaxKinkLen = fMaxKinkMult*fThickness;
	FLOAT_TYPE fKinkLen, fNextDotProd, fHypotenuseSq, fPerpBluntKinkLen;
	FLOAT_TYPE fCosHalfAngle, fCosHalfAngle2, fSignFlip;
	SVector2D svDirNext, svNormNext, svKinkDir, svKinkNorm, svMaxKinkLenMidPoint, svTempDist;

	SVector2D svMiteredPrev[2], svBluntPoints[2], svPivotBlunt;
	bool bSavedPrevVerts = false;
	HyperCore::OrientationType eOrient = OrientationCCW;
	if(bMiter)
		eOrient = TPolyline2D<SVector2D>::getPolygonOrientation(rPoints);
/*
	if(eOrient == OrientationCW)
	{
		fTempDist = fFinalV;
		fFinalV = fFirstV;
		fFirstV = fTempDist;
	}
*/
	bool bDoBluntKink;

	//for(iCurrPoint = iStartPtIndex + 1, iArrayPointer = iInitArrayOffset; iCurrPoint <= iEndPtIndex; iCurrPoint++, iArrayPointer += 6)
	FLOAT_TYPE fCurrLen;
	for(iCurrPoint = 1; iCurrPoint < iNumPoints; iCurrPoint++)
	{
		svDir = rPoints[iCurrPoint] - rPoints[iCurrPoint - 1];
		fCurrLen = svDir.normalize();

		svNorm.set(svDir.y, -svDir.x);

		if(bStretchTexture)
			fUStep = ( (fCurrLen/fTotalLength)/fTextureLengthMult)*(svTextFullUVs.myEndX - svTextFullUVs.myStartX);
		else
		{
			fUStep = fCurrLen/(FLOAT_TYPE)iRealW*fFullUStep;
		//	fUStep = fCurrLen/fScaledRealW*fFullUStep;
		}

		if(bSavedPrevVerts)
		{
			svPoly[0] = svMiteredPrev[0];
			svPoly[3] = svMiteredPrev[1];
		}
		else
		{
			// Prev point 1
			svPoly[0].x = rPoints[iCurrPoint - 1].x - svNorm.x*fHalfThickness + svOffset.x;
			svPoly[0].y = rPoints[iCurrPoint - 1].y - svNorm.y*fHalfThickness + svOffset.y;

			// Prev point 2
			svPoly[3].x = rPoints[iCurrPoint - 1].x + svNorm.x*fHalfThickness + svOffset.x;
			svPoly[3].y = rPoints[iCurrPoint - 1].y + svNorm.y*fHalfThickness + svOffset.y;
		}

		bDoBluntKink = false;
		bSavedPrevVerts = false;
		if(bMiter && iCurrPoint + 1 < iNumPoints)
		{
			// Mitering length:
			// Find the average normal
			svDirNext = rPoints[iCurrPoint + 1] - rPoints[iCurrPoint];
			svDirNext.normalize();
			svNormNext.set(svDirNext.y, -svDirNext.x);

			fNextDotProd = svDirNext.dot(svDir);
			fCosHalfAngle = F_SQRT((1.0 + fNextDotProd)/2.0);
			fKinkLen = fabs(fHalfThickness/fCosHalfAngle);

			svKinkDir = (svNorm + svNormNext)*0.5;
			svKinkDir.normalize();

			if(fKinkLen > fMaxKinkLen && fabs(fCosHalfAngle) > FLOAT_EPSILON)
			{
				svKinkNorm.set(svKinkDir.y, -svKinkDir.x);

				if(fNextDotProd < 0)
				{
					fCosHalfAngle2 = F_SQRT((1.0 - fNextDotProd)/2.0);

				}
				else
				{
					fCosHalfAngle2 = F_SQRT((1.0 + fNextDotProd)/2.0);

				}

				//eOrient = getTriangleOrientation(rPoints[iCurrPoint - 1], rPoints[iCurrPoint], rPoints[iCurrPoint + 1]);
				fSignFlip = -1;
				if(eOrient == OrientationCW)
					fSignFlip = 1;

				fHypotenuseSq = (fKinkLen - fMaxKinkLen)/fCosHalfAngle2;
				fHypotenuseSq *= fHypotenuseSq;

				fPerpBluntKinkLen = fHypotenuseSq - (fKinkLen - fMaxKinkLen)*(fKinkLen - fMaxKinkLen);
				if(fPerpBluntKinkLen > 0)
					fPerpBluntKinkLen = F_SQRT(fPerpBluntKinkLen);

				if(fPerpBluntKinkLen > 1.0)
				{
					svMaxKinkLenMidPoint = rPoints[iCurrPoint] + svKinkDir*fMaxKinkLen*fSignFlip + svOffset;

					svBluntPoints[0] = svMaxKinkLenMidPoint - svKinkNorm*fPerpBluntKinkLen*fSignFlip;
					svBluntPoints[1] = svMaxKinkLenMidPoint + svKinkNorm*fPerpBluntKinkLen*fSignFlip;
					if(fSignFlip < 0)
					{
						svPoly[1] = svBluntPoints[0];
						svPoly[2] = rPoints[iCurrPoint] + svKinkDir*fKinkLen + svOffset;

						svMiteredPrev[0] = svBluntPoints[1];
						svMiteredPrev[1] = svPoly[2];

						svPivotBlunt = svPoly[2];
					}
					else
					{
						svPoly[1] = rPoints[iCurrPoint] - svKinkDir*fKinkLen + svOffset;
						svPoly[2] = svBluntPoints[1];

						svMiteredPrev[0] = svPoly[1];
						svMiteredPrev[1] = svBluntPoints[0];

						svPivotBlunt = svPoly[1];
					}

					bDoBluntKink = true;
				}

			}

			if(!bDoBluntKink)
			{
				svPoly[1] = rPoints[iCurrPoint] - svKinkDir*fKinkLen + svOffset;
				svPoly[2] = rPoints[iCurrPoint] + svKinkDir*fKinkLen + svOffset;

				svMiteredPrev[0] = svPoly[1];
				svMiteredPrev[1] = svPoly[2];
			}

			bSavedPrevVerts = true;
		}
		else
		{
			svPoly[1].x = rPoints[iCurrPoint].x - svNorm.x*fHalfThickness + svOffset.x;
			svPoly[1].y = rPoints[iCurrPoint].y - svNorm.y*fHalfThickness + svOffset.y;

			svPoly[2].x = rPoints[iCurrPoint].x + svNorm.x*fHalfThickness + svOffset.x;
			svPoly[2].y = rPoints[iCurrPoint].y + svNorm.y*fHalfThickness + svOffset.y;
		}

		g_sDistInfos[iCurrPoint].myExtra = 0;
		if(bMiter && bStretchTexture)
		{
			svTempDist = svPoly[1] - svPoly[0];
			fTempDist = svTempDist.length();
			fTotalLength1 += fTempDist;
			g_sDistInfos[iCurrPoint].mySide1 = fTempDist;

			svTempDist = svPoly[2] - svPoly[3];
			fTempDist = svTempDist.length();
			fTotalLength2 += fTempDist;
			g_sDistInfos[iCurrPoint].mySide2 = fTempDist;


		}

		_ASSERT(iArrayPointer + 5 < g_iNumTexPathRenderVertices);

		g_pTexPathRendVertices[iArrayPointer + 0].x = svPoly[0].x;
		g_pTexPathRendVertices[iArrayPointer + 0].y = svPoly[0].y;
		g_pTexPathRendVertices[iArrayPointer + 1].x = svPoly[1].x;
		g_pTexPathRendVertices[iArrayPointer + 1].y = svPoly[1].y;
		g_pTexPathRendVertices[iArrayPointer + 2].x = svPoly[3].x;
		g_pTexPathRendVertices[iArrayPointer + 2].y = svPoly[3].y;

		g_pTexPathRendVertices[iArrayPointer + 3].x = svPoly[3].x;
		g_pTexPathRendVertices[iArrayPointer + 3].y = svPoly[3].y;
		g_pTexPathRendVertices[iArrayPointer + 4].x = svPoly[1].x;
		g_pTexPathRendVertices[iArrayPointer + 4].y = svPoly[1].y;
		g_pTexPathRendVertices[iArrayPointer + 5].x = svPoly[2].x;
		g_pTexPathRendVertices[iArrayPointer + 5].y = svPoly[2].y;

#ifdef USE_OPENGL2
		g_pTexPathRendVertices[iArrayPointer + 0].z = 0.0;
		g_pTexPathRendVertices[iArrayPointer + 0].w = 1.0;
		g_pTexPathRendVertices[iArrayPointer + 1].z = 0.0;
		g_pTexPathRendVertices[iArrayPointer + 1].w = 1.0;
		g_pTexPathRendVertices[iArrayPointer + 2].z = 0.0;
		g_pTexPathRendVertices[iArrayPointer + 2].w = 1.0;

		g_pTexPathRendVertices[iArrayPointer + 3].z = 0.0;
		g_pTexPathRendVertices[iArrayPointer + 3].w = 1.0;
		g_pTexPathRendVertices[iArrayPointer + 4].z = 0.0;
		g_pTexPathRendVertices[iArrayPointer + 4].w = 1.0;
		g_pTexPathRendVertices[iArrayPointer + 5].z = 0.0;
		g_pTexPathRendVertices[iArrayPointer + 5].w = 1.0;
#endif

		if(!bStretchTexture || !bMiter)
		{
			// Texture coords
			g_pTexPathRendVertices[iArrayPointer + 0].s0 = fCurrUPos;
			g_pTexPathRendVertices[iArrayPointer + 0].t0 = fFirstV;
			g_pTexPathRendVertices[iArrayPointer + 1].s0 = (fCurrUPos + fUStep);
			g_pTexPathRendVertices[iArrayPointer + 1].t0 = fFirstV;
			g_pTexPathRendVertices[iArrayPointer + 2].s0 = fCurrUPos;
			g_pTexPathRendVertices[iArrayPointer + 2].t0 = fFinalV;

			g_pTexPathRendVertices[iArrayPointer + 3].s0 = fCurrUPos;
			g_pTexPathRendVertices[iArrayPointer + 3].t0 = fFinalV;
			g_pTexPathRendVertices[iArrayPointer + 4].s0 = (fCurrUPos + fUStep);
			g_pTexPathRendVertices[iArrayPointer + 4].t0 = fFirstV;
			g_pTexPathRendVertices[iArrayPointer + 5].s0 = (fCurrUPos + fUStep);
			g_pTexPathRendVertices[iArrayPointer + 5].t0 = fFinalV;
		}

		for(iInner = 0; iInner < 6;  iInner++)
		{
#ifdef USE_OPENGL2
			g_pTexPathRendVertices[iArrayPointer + iInner].r = scolVertexColor.r;
			g_pTexPathRendVertices[iArrayPointer + iInner].g = scolVertexColor.g;
			g_pTexPathRendVertices[iArrayPointer + iInner].b = scolVertexColor.b;
			g_pTexPathRendVertices[iArrayPointer + iInner].a = scolVertexColor.alpha;
#else

			g_pTexPathRendVertices[iArrayPointer + iInner].r = r;
			g_pTexPathRendVertices[iArrayPointer + iInner].g = g;
			g_pTexPathRendVertices[iArrayPointer + iInner].b = b;
			g_pTexPathRendVertices[iArrayPointer + iInner].a = a;
#endif
		}

		if(bStretchTexture)
			fCurrUPos += fUStep;

		iArrayPointer += 6;

		// Add a triangle for the blunt point if we have to
		if(bDoBluntKink)
		{
			g_pTexPathRendVertices[iArrayPointer + 0].x = svPivotBlunt.x;
			g_pTexPathRendVertices[iArrayPointer + 0].y = svPivotBlunt.y;
			g_pTexPathRendVertices[iArrayPointer + 1].x = svBluntPoints[0].x;
			g_pTexPathRendVertices[iArrayPointer + 1].y = svBluntPoints[0].y;
			g_pTexPathRendVertices[iArrayPointer + 2].x = svBluntPoints[1].x;
			g_pTexPathRendVertices[iArrayPointer + 2].y = svBluntPoints[1].y;
#ifdef USE_OPENGL2
			g_pTexPathRendVertices[iArrayPointer + 0].z = 0.0;
			g_pTexPathRendVertices[iArrayPointer + 0].w = 1.0;
			g_pTexPathRendVertices[iArrayPointer + 1].z = 0.0;
			g_pTexPathRendVertices[iArrayPointer + 1].w = 1.0;
			g_pTexPathRendVertices[iArrayPointer + 2].z = 0.0;
			g_pTexPathRendVertices[iArrayPointer + 2].w = 1.0;
#endif

			if(!bStretchTexture)
			{
				g_pTexPathRendVertices[iArrayPointer + 0].s0 = (fCurrUPos - fUStep);
				g_pTexPathRendVertices[iArrayPointer + 0].t0 = (fFinalV - fFirstV)*0.5+fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 1].s0 = (fCurrUPos - fUStep);
				g_pTexPathRendVertices[iArrayPointer + 1].t0 = fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 2].s0 = (fCurrUPos - fUStep);
				g_pTexPathRendVertices[iArrayPointer + 2].t0 = fFirstV;
			}
			else
			{
#ifdef MITER_UVS_VERSION1
				g_sDistInfos[iCurrPoint].myExtra = (svBluntPoints[0] - svBluntPoints[1]).length();
/*
				if(eOrient != OrientationCW)
					fTotalLength1 += g_sDistInfos[iCurrPoint].myExtra;
				else
				*/
					fTotalLength2 += g_sDistInfos[iCurrPoint].myExtra;

				g_sDistInfos[iCurrPoint].myNormalUCoord = fCurrUPos;
#else
				g_sDistInfos[iCurrPoint].myExtra = (svBluntPoints[0] - svBluntPoints[1]).length();
				g_sDistInfos[iCurrPoint].myNormalUCoord = fCurrUPos;
#endif
			}

			for(iInner = 0; iInner < 3;  iInner++)
			{
#ifdef USE_OPENGL2
				g_pTexPathRendVertices[iArrayPointer + iInner].r = scolVertexColor.r;
				g_pTexPathRendVertices[iArrayPointer + iInner].g = scolVertexColor.g;
				g_pTexPathRendVertices[iArrayPointer + iInner].b = scolVertexColor.b;
				g_pTexPathRendVertices[iArrayPointer + iInner].a = scolVertexColor.alpha;
#else

				g_pTexPathRendVertices[iArrayPointer + iInner].r = r;
				g_pTexPathRendVertices[iArrayPointer + iInner].g = g;
				g_pTexPathRendVertices[iArrayPointer + iInner].b = b;
				g_pTexPathRendVertices[iArrayPointer + iInner].a = a;
#endif
			}

			iArrayPointer += 3;

		}
/*
		if(bMiter && iCurrPoint > 1)
		{
			fSignOne = svPrevDir.dot(svDir)*-1;
			if(fSignOne >= 0)
				fSignOne = 1;
			else
				fSignOne = -1;


			if(getTriangleOrientation(rPoints[iCurrPoint - 2], rPoints[iCurrPoint - 1], rPoints[iCurrPoint]) == OrientationCCW)
				fSignOne = -1;
			else
				fSignOne = 1;

			g_pTexPathRendVertices[iArrayPointer + 0].x = rPoints[iCurrPoint - 1].x;
			g_pTexPathRendVertices[iArrayPointer + 0].y = rPoints[iCurrPoint - 1].y;
			g_pTexPathRendVertices[iArrayPointer + 1].x = rPoints[iCurrPoint - 1].x + svPrevNorm.x*fSignOne*fHalfThickness;
			g_pTexPathRendVertices[iArrayPointer + 1].y = rPoints[iCurrPoint - 1].y + svPrevNorm.y*fSignOne*fHalfThickness;
			g_pTexPathRendVertices[iArrayPointer + 2].x = rPoints[iCurrPoint - 1].x + svNorm.x*fSignOne*fHalfThickness;
			g_pTexPathRendVertices[iArrayPointer + 2].y = rPoints[iCurrPoint - 1].y + svNorm.y*fSignOne*fHalfThickness;

			g_pTexPathRendVertices[iArrayPointer + 0].s0 = fCurrUPos - fUStep;
			g_pTexPathRendVertices[iArrayPointer + 0].t0 = (fFinalV - fFirstV)*0.5+fFirstV;
			g_pTexPathRendVertices[iArrayPointer + 1].s0 = fCurrUPos - fUStep;
			g_pTexPathRendVertices[iArrayPointer + 1].t0 = fFirstV;
			g_pTexPathRendVertices[iArrayPointer + 2].s0 = fCurrUPos - fUStep;
			g_pTexPathRendVertices[iArrayPointer + 2].t0 = fFirstV;


			for(iInner = 0; iInner < 3;  iInner++)
			{
				g_pTexPathRendVertices[iArrayPointer + iInner].r = r;
				g_pTexPathRendVertices[iArrayPointer + iInner].g = g;
				g_pTexPathRendVertices[iArrayPointer + iInner].b = b;
				g_pTexPathRendVertices[iArrayPointer + iInner].a = a;
			}

			iArrayPointer += 3;
		}
*/
		svPrevNorm = svNorm;
		svPrevDir = svDir;
	}

#ifdef MITER_UVS_VERSION1
	if(bMiter && bStretchTexture)
	{
		// Go over them again, setting texture coords
		iArrayPointer = 0;
		fCurrUPos = svTextFullUVs.myStartX;
		fCurrUPos2 = svTextFullUVs.myStartX;
		for(iCurrPoint = 1; iCurrPoint < iNumPoints; iCurrPoint++)
		{
			fUStep = (g_sDistInfos[iCurrPoint].mySide1/fTotalLength1)*(svTextFullUVs.myEndX - svTextFullUVs.myStartX);
			fUStep2 = (g_sDistInfos[iCurrPoint].mySide2/fTotalLength2)*(svTextFullUVs.myEndX - svTextFullUVs.myStartX);

			if(eOrient == OrientationCW)
			{
				g_pTexPathRendVertices[iArrayPointer + 0].s0 = fCurrUPos;
				g_pTexPathRendVertices[iArrayPointer + 0].t0 = fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 1].s0 = (fCurrUPos + fUStep);
				g_pTexPathRendVertices[iArrayPointer + 1].t0 = fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 2].s0 = fCurrUPos2;
				g_pTexPathRendVertices[iArrayPointer + 2].t0 = fFinalV;

				g_pTexPathRendVertices[iArrayPointer + 3].s0 = fCurrUPos2;
				g_pTexPathRendVertices[iArrayPointer + 3].t0 = fFinalV;
				g_pTexPathRendVertices[iArrayPointer + 4].s0 = (fCurrUPos + fUStep);
				g_pTexPathRendVertices[iArrayPointer + 4].t0 = fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 5].s0 = (fCurrUPos2 + fUStep2);
				g_pTexPathRendVertices[iArrayPointer + 5].t0 = fFinalV;
			}
			else
			{
				g_pTexPathRendVertices[iArrayPointer + 0].s0 = fCurrUPos2;
				g_pTexPathRendVertices[iArrayPointer + 0].t0 = fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 1].s0 = (fCurrUPos2 + fUStep2);
				g_pTexPathRendVertices[iArrayPointer + 1].t0 = fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 2].s0 = fCurrUPos;
				g_pTexPathRendVertices[iArrayPointer + 2].t0 = fFinalV;

				g_pTexPathRendVertices[iArrayPointer + 3].s0 = fCurrUPos;
				g_pTexPathRendVertices[iArrayPointer + 3].t0 = fFinalV;
				g_pTexPathRendVertices[iArrayPointer + 4].s0 = (fCurrUPos2 + fUStep2);
				g_pTexPathRendVertices[iArrayPointer + 4].t0 = fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 5].s0 = (fCurrUPos + fUStep);
				g_pTexPathRendVertices[iArrayPointer + 5].t0 = fFinalV;
			}

			fCurrUPos += fUStep;
			fCurrUPos2 += fUStep2;

			iArrayPointer += 6;

			if(g_sDistInfos[iCurrPoint].myExtra > 0)
			{
				{
					fUStep2 = (g_sDistInfos[iCurrPoint].myExtra/fTotalLength2)*(svTextFullUVs.myEndX - svTextFullUVs.myStartX);

					g_pTexPathRendVertices[iArrayPointer + 0].s0 = g_sDistInfos[iCurrPoint].myNormalUCoord;
					//g_pTexPathRendVertices[iArrayPointer + 0].t0 = (fFinalV - fFirstV)*0.5+fFirstV;

					if(eOrient == OrientationCW)
					{
						g_pTexPathRendVertices[iArrayPointer + 0].t0 = fFirstV;
						g_pTexPathRendVertices[iArrayPointer + 1].s0 = (fCurrUPos2 + fUStep2);
						g_pTexPathRendVertices[iArrayPointer + 1].t0 = fFinalV;
						g_pTexPathRendVertices[iArrayPointer + 2].s0 = (fCurrUPos2);
						g_pTexPathRendVertices[iArrayPointer + 2].t0 = fFinalV;
					}
					else
					{
						g_pTexPathRendVertices[iArrayPointer + 0].t0 = fFinalV;
						g_pTexPathRendVertices[iArrayPointer + 1].s0 = fCurrUPos2;
						g_pTexPathRendVertices[iArrayPointer + 1].t0 = fFirstV;
						g_pTexPathRendVertices[iArrayPointer + 2].s0 = (fCurrUPos2 + fUStep2);
						g_pTexPathRendVertices[iArrayPointer + 2].t0 = fFirstV;
					}
					fCurrUPos2 += fUStep2;
				}
				iArrayPointer += 3;
			}
		}
	}
#else
	if(bMiter && bStretchTexture)
	{
		// Go over them again, setting texture coords
		FLOAT_TYPE fExtraStep;
		iArrayPointer = 0;
		fCurrUPos = svTextFullUVs.myStartX;
		fCurrUPos2 = svTextFullUVs.myStartX;
		for(iCurrPoint = 1; iCurrPoint < iNumPoints; iCurrPoint++)
		{
			fUStep = (g_sDistInfos[iCurrPoint].mySide1/fTotalLength1)*(svTextFullUVs.myEndX - svTextFullUVs.myStartX);
			fUStep2 = (g_sDistInfos[iCurrPoint].mySide2/fTotalLength2)*(svTextFullUVs.myEndX - svTextFullUVs.myStartX);

			fExtraStep = 0;
			if(g_sDistInfos[iCurrPoint].myExtra > 0)
			{
				fExtraStep = (g_sDistInfos[iCurrPoint].myExtra/fTotalLength2)*(svTextFullUVs.myEndX - svTextFullUVs.myStartX);
			}

//			if(eOrient == OrientationCW)
			{
				g_pTexPathRendVertices[iArrayPointer + 0].s0 = fCurrUPos;
				g_pTexPathRendVertices[iArrayPointer + 0].t0 = fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 1].s0 = fCurrUPos + fUStep - fExtraStep;
				g_pTexPathRendVertices[iArrayPointer + 1].t0 = fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 2].s0 = fCurrUPos2;
				g_pTexPathRendVertices[iArrayPointer + 2].t0 = fFinalV;

				g_pTexPathRendVertices[iArrayPointer + 3].s0 = fCurrUPos2;
				g_pTexPathRendVertices[iArrayPointer + 3].t0 = fFinalV;
				g_pTexPathRendVertices[iArrayPointer + 4].s0 = fCurrUPos + fUStep - fExtraStep;
				g_pTexPathRendVertices[iArrayPointer + 4].t0 = fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 5].s0 = fCurrUPos2 + fUStep2 - fExtraStep;
				g_pTexPathRendVertices[iArrayPointer + 5].t0 = fFinalV;
			}
/*
			else
			{
				g_pTexPathRendVertices[iArrayPointer + 0].s0 = fCurrUPos2;
				g_pTexPathRendVertices[iArrayPointer + 0].t0 = fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 1].s0 = fCurrUPos2 + fUStep2;
				g_pTexPathRendVertices[iArrayPointer + 1].t0 = fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 2].s0 = fCurrUPos;
				g_pTexPathRendVertices[iArrayPointer + 2].t0 = fFinalV;

				g_pTexPathRendVertices[iArrayPointer + 3].s0 = fCurrUPos;
				g_pTexPathRendVertices[iArrayPointer + 3].t0 = fFinalV;
				g_pTexPathRendVertices[iArrayPointer + 4].s0 = fCurrUPos2 + fUStep2;
				g_pTexPathRendVertices[iArrayPointer + 4].t0 = fFirstV;
				g_pTexPathRendVertices[iArrayPointer + 5].s0 = fCurrUPos + fUStep;
				g_pTexPathRendVertices[iArrayPointer + 5].t0 = fFinalV;
			}
*/
			fCurrUPos += fUStep;
			fCurrUPos2 += fUStep2;

			iArrayPointer += 6;

			if(g_sDistInfos[iCurrPoint].myExtra > 0)
			{
				{
					//fUStep2 = (g_sDistInfos[iCurrPoint].myExtra/fTotalLength2)*(svTextFullUVs.myEndX - svTextFullUVs.myStartX);

					g_pTexPathRendVertices[iArrayPointer + 0].s0 = g_sDistInfos[iCurrPoint].myNormalUCoord;
					g_pTexPathRendVertices[iArrayPointer + 0].t0 = (fFinalV - fFirstV)*0.5+fFirstV;
					if(eOrient == OrientationCW)
						g_pTexPathRendVertices[iArrayPointer + 1].s0 = fCurrUPos2; //  + fUStep2;
					else
						g_pTexPathRendVertices[iArrayPointer + 1].s0 = fCurrUPos2 - fUStep2;
					g_pTexPathRendVertices[iArrayPointer + 1].t0 = fFirstV;
					if(eOrient == OrientationCW)
						g_pTexPathRendVertices[iArrayPointer + 2].s0 = fCurrUPos2 - fUStep2;
					else
						g_pTexPathRendVertices[iArrayPointer + 2].s0 = fCurrUPos2; // + fUStep2;
					g_pTexPathRendVertices[iArrayPointer + 2].t0 = fFirstV;

					//fCurrUPos2 += fUStep2;
				}
				iArrayPointer += 3;
			}
		}
	}

#endif

	if(iArrayPointer > 0)
	{

#ifdef DIRECTX_PIPELINE
	_ASSERT(iArrayPointer % 3 == 0);
	myDxBuffer.setFromVertexArray(g_pTexPathRendVertices, iArrayPointer/3, true);

	UINT stride = sizeof(BasicVertex);
	UINT offset = 0;
	g_pDxRenderer->getD3dContext()->IASetVertexBuffers(0, 1, myDxBuffer.getVertexBuffer(), &stride, &offset);
	g_pDxRenderer->getD3dContext()->IASetIndexBuffer(myDxBuffer.getIndexBuffer(), INDEX_BUFFER_FORMAT,	0);
	g_pDxRenderer->getD3dContext()->DrawIndexed(iArrayPointer, 0, 0);

#else

#ifdef USE_OPENGL2
		RenderUtils::drawBuffer<SVertexInfo>(&g_pTexPathRendVertices[0].x, SVertexInfo::getNumVertexComponents(), pcsTexture ? &g_pTexPathRendVertices[0].s0 : NULL, &g_pTexPathRendVertices[0].r, NULL, iArrayPointer);
#else

#ifdef USE_INT_OPENGL_POSITIONS
	OpenGLStateCleaner::doVertexPointerCall(2, GL_SHORT, sizeof(SVertexInfo), &g_pTexPathRendVertices[0].x);
#else
	OpenGLStateCleaner::doVertexPointerCall(2, GL_FLOAT, sizeof(SVertexInfo), &g_pTexPathRendVertices[0].x);
#endif
	if(pcsTexture)
		OpenGLStateCleaner::doTexturePointerCall(2, GL_FLOAT, sizeof(SVertexInfo), &g_pTexPathRendVertices[0].s0);
	OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, sizeof(SVertexInfo), &g_pTexPathRendVertices[0].r);

	glDrawArrays(GL_TRIANGLES, 0, iArrayPointer);
#endif

#endif
	}
	if(eOldMode != BlendModeLastPlaceholder)
		pWindow->setBlendMode(eOldMode);
}

/*****************************************************************************/
// void RenderUtils::renderTexturedLine(CachedSequence* pMainBeam, CachedSequence*  pEndExplAnim,
// 									 SVector2D& svStartPoint, SVector2D& svEndPoint, FLOAT_TYPE fEndExplOpacity,
// 									 FLOAT_TYPE fRayOpacity, FLOAT_TYPE fScale, FLOAT_TYPE fRayAnimFrame,
// 									 FLOAT_TYPE fEndExplAnimFrame, bool bIsRayAnimated)
void RenderUtils::renderTexturedLine(CachedSequence* pMainBeam, SVector2D& svStartPoint, SVector2D& svEndPoint,
 									 FLOAT_TYPE fRayOpacity, FLOAT_TYPE fScale, FLOAT_TYPE fRayAnimFrame, bool bIsRayAnimated, bool bIsFrameAnAtlasFrame, FLOAT_TYPE fThicknessMult)
{
	// To do this, we draw a bitmap in the center of the line scaled appropriately.
	// What about tiling textures? Do we need to lookup the size of the texture and then
	// decide what to tile? I guess this can be determined from the object's size.

	SVector2D svPulseCenter;
//	SRect2D srOwnBBox;
//	this->getCurrBBox(srOwnBBox);

	TextureManager* pTextureManager = pMainBeam->getTextureManager();

	int iRealW, iRealH;
	pTextureManager->getTextureRealDims(pMainBeam->getName(), iRealW, iRealH);

	// If this fails, need to add animRealImageWidth to the appropriate
	// animation passed in.
	_ASSERT(iRealW > 0);


	FLOAT_TYPE fFinalAngle = 0.0;
	FLOAT_TYPE fFinalDist = 0.0;
	SVector2D svDiff;

	//if(!fDist || !fAngle)
	{

		svDiff = svEndPoint - svStartPoint;
		fFinalDist = svDiff.normalize();
		fFinalAngle = svDiff.getAngleFromPositiveXAccurate();
	}
/*
	if(fDist)
		fFinalDist = *fDist;
	if(fAngle)
		fFinalAngle = *fAngle;
*/
	FLOAT_TYPE fScaleX, fScaleY;
	fScaleY = fScale*fThicknessMult;
	fScaleX = (fFinalDist /* + srOwnBBox.w/2.0 */)/((FLOAT_TYPE)iRealW)*fScale;

	SUVSet uvSet;
	uvSet.myStartX = uvSet.myStartY = 0.0;
	uvSet.myEndY = 1.0;
	uvSet.myEndX = fScaleX;

	svPulseCenter = (svStartPoint + svEndPoint)*0.5;
	// The cache currently doesn't support anything with UVs. This may slow us down, too.
	FLOAT_TYPE fAnimValue = fRayAnimFrame;
	/*
	FLOAT_TYPE fAnimValue = 0.0;
	if(bIsRayAnimated)
		fAnimValue = myLastFrameValue;

	if(fRayAnimFrame >= 0)
		fAnimValue = myTopAnimAtlasFrame;
*/
	// The beam is animated if myTopAnimAtlasFrame is < 0 and the texture itself
	// is an atlas.
	// if(pTextureManager->isTextureAnAtlas(pMainBeam->getName()))
	if(bIsRayAnimated)
	{
		// fScaleX at this point has the number (as int) of items we'll need. The last one
		// will be special, since it may be partial.
		SVector2D svSegPos, svDir;
		svDir = svDiff;
		svDir.normalize();
		int iCurrSeg, iNumSegs = fScaleX;
		int iScaledBitmapW = ((FLOAT_TYPE)iRealW)*fScale;
		svSegPos = svStartPoint + svDir*(iScaledBitmapW/2.0);
		for(iCurrSeg = 0; iCurrSeg < iNumSegs; iCurrSeg++)
		{
			pMainBeam->addSprite(svSegPos.x, svSegPos.y, fRayOpacity, fFinalAngle, fScale, fScale, fAnimValue, true, true, NULL, false);
			svSegPos += svDir*iScaledBitmapW;
		}

		// Now, finally, render the last one that's partial.
		svSegPos = svSegPos - svDir*(iScaledBitmapW*0.5);
		// Figure out remaining length
		FLOAT_TYPE fRemainingWidth = (svSegPos - svEndPoint).length();
		FLOAT_TYPE fInnerScaleX = fRemainingWidth/((FLOAT_TYPE)iRealW)*fScale;
		svSegPos = svSegPos + svDir*fRemainingWidth/2.0;
		SUVSet uvsOutTemp;
		BlendModeType eBlendMode;
		int iW, iH;
		/*
		int iFileW;
		iFileW = pTextureManager->getFileWidth(pMainBeam->getName());
		FLOAT_TYPE fFrameUSize = ((FLOAT_TYPE)iRealW)/(FLOAT_TYPE)iFileW;
		*/
		pTextureManager->getTextureIndexFromProgress(pMainBeam->getName(), fAnimValue, iW, iH, uvsOutTemp, eBlendMode);
		uvsOutTemp.myEndX = (uvsOutTemp.myEndX - uvsOutTemp.myStartX)*fInnerScaleX + uvsOutTemp.myStartX;
		pMainBeam->addSprite(svSegPos.x, svSegPos.y, fRayOpacity, fFinalAngle, fInnerScaleX, fScale, fAnimValue, true, true, &uvsOutTemp, false);
	}
	else
	{
		// NOTE: We run into a porblem with UVs - we can't texture an animated beam weapon properly, because we can't repeat its uvs across a single streched poly.
		// We need to build the beam out of many scale 1.0 polys and then set UVs for those. So for now, we set UVs here to NULL.
		pMainBeam->addSprite(svPulseCenter.x, svPulseCenter.y, fRayOpacity, fFinalAngle, fScaleX, fScaleY, fAnimValue, true, true,
			NULL, bIsFrameAnAtlasFrame);
		//	pMainBeam->addSprite(svPulseCenter.x, svPulseCenter.y, fRayOpacity, fFinalAngle, fScaleX, fScaleY, fAnimValue, true, true, &uvSet, myTopAnimAtlasFrame >= 0);
	}

#if 0
	// Now, also draw the bottom animation, which in our case is the animation on the end
	if(myPulseRayLength > 0 && pEndExplAnim) //  && strlen(pcsBaseAnim) > 0)
	{
		fAnimValue = fEndExplAnimFrame;
		/*
		fAnimValue = myLastBaseFrameValue;
		if(myPropertyBeingAnimated == PropertySecondaryImage)
			fAnimValue = myLastFrameValue;

		if(myBaseAnimAtlasFrame >= 0)
			fAnimValue = myBaseAnimAtlasFrame;
*/
		pEndExplAnim->addSprite(svEndPoint.x, svEndPoint.y, fEndExplOpacity, fFinalAngle, 1.0, 1.0, fAnimValue, true, true, NULL, myBaseAnimAtlasFrame >= 0);
	}
#endif
}
/*****************************************************************************/
void RenderUtils::renderObject3D(Window* pWindow, const char* pcsObjType, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fCenterZ, FLOAT_TYPE fAlpha, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, int iClipIndex, SVector3D &pRotations, const char* pcsOverrideTextureAnim, bool bManageDepthTest)
{
	// Need to migrate the 3D obj manager
	_ASSERT(0);
#if 0
	if(fAlpha < FLOAT_EPSILON)
		return;

	// Find the corresponding part
	CachedPart3D* pPart;
	CachedObject3D* pObj = g_pObject3DManager->findObject(pcsObjType);

	if(!pObj)
		return; 

	CachedFrame3D* pFrame = pObj->getFrameForProgress(fAnimProgress, iClipIndex);

#if defined(DIRECTX_PIPELINE) || defined(USE_OPENGL2)

	SMatrix4 smWorldMatrix;
	smWorldMatrix.resetToIdentity();

	smWorldMatrix.appendTranslation(fCenterX, -fCenterY, fCenterZ);

	if(pRotations.x != 0)
		smWorldMatrix.appendXRotation(sanitizeDegAngle(pRotations.x));
	if(pRotations.y != 0)
		smWorldMatrix.appendYRotation(sanitizeDegAngle(pRotations.y));
	if(pRotations.z != 0)
		smWorldMatrix.appendZRotation(sanitizeDegAngle(pRotations.z));

	if(fScale != 1.0)
		smWorldMatrix.appendScale(fScale, fScale, fScale);
#endif

#if defined(DIRECTX_PIPELINE)
	_ASSERT(!bManageDepthTest);
	g_pDxRenderer->setObject3DMatrix(smWorldMatrix);


	BlendModeType eOverallBlendMode = BlendModeNormal;
	TX_MAN_RETURN_TYPE iOverrideIdx = 0;
	if(pcsOverrideTextureAnim && strlen(pcsOverrideTextureAnim) > 0)
	{
		int iDummyW, iDummyH;
		SUVSet uvDummy;
		iOverrideIdx = pWindow->getTextureManager()->getTextureIndex(pcsOverrideTextureAnim, 0, iDummyW, iDummyH, uvDummy, eOverallBlendMode);
#ifdef ENABLE_TEXTURE_UNLOADING
		pWindow->getTextureManager()->ensureLoaded(pcsOverrideTextureAnim);
#endif
	}

	g_pDxRenderer->updateObject3DState();

	SVertex3D* pData;
	TX_MAN_RETURN_TYPE uiTextureIndex;
	int iCurrPart, iNumParts = pFrame->getNumParts();
	for(iCurrPart = 0; iCurrPart < iNumParts; iCurrPart++)
	{
		pPart = pFrame->getPart(iCurrPart);
		pData = pPart->getData();

		SColor scolTemp(1,1,1,1);
		pObj->getDiffuseMults(pPart, scolTemp);
		scolTemp.alpha *= fAlpha;
		g_pDxRenderer->setObject3DDiffuseColor(scolTemp);


		if(iOverrideIdx == 0)
		{
#ifdef ENABLE_TEXTURE_UNLOADING
			pPart->ensureTexturesLoaded(pWindow->getTextureManager());
#endif
			uiTextureIndex = pPart->getTextureIndex();
			if(uiTextureIndex)
				g_pDxRenderer->getD3dContext()->PSSetShaderResources(0, 1, uiTextureIndex);
			eOverallBlendMode = pPart->getBlendMode();
		}
		else
			g_pDxRenderer->getD3dContext()->PSSetShaderResources(0, 1, iOverrideIdx);

		//		bDidEnableAlpha = false;

#ifdef EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH
		if(eOverallBlendMode != BlendModeNormal)
		{
			_ASSERT(0);
			//			if(bManageDepthTest)
			//				glDisable(GL_DEPTH_TEST);
		}
		else
		{

			//			if(fAlpha < 1.0)
			//			bDidEnableAlpha = true;
		}
#else
		TODO
#endif
			_ASSERT(eOverallBlendMode == BlendModeNormal);
		g_pDxRenderer->setBlendState(BlendModeText);

		myDxBuffer.setFromVertexArray(pData, pPart->getNumVerts()/3, true);

		UINT stride = sizeof(BasicVertex);
		UINT offset = 0;
		g_pDxRenderer->getD3dContext()->IASetVertexBuffers(0, 1, myDxBuffer.getVertexBuffer(), &stride, &offset);
		g_pDxRenderer->getD3dContext()->IASetIndexBuffer(myDxBuffer.getIndexBuffer(), INDEX_BUFFER_FORMAT,	0);
		g_pDxRenderer->getD3dContext()->DrawIndexed(pPart->getNumVerts(), 0, 0);


		//		if(bDidEnableAlpha)
		//		glDisable( GL_ALPHA_TEST ); 


#ifdef EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH
		//		if(eOverallBlendMode != BlendModeNormal && bManageDepthTest)
		//			glEnable(GL_DEPTH_TEST);

#else

		if(eOverallBlendMode != BlendModeNormal)
		{
			if(bManageDepthTest)
				glEnable(GL_DEPTH_TEST);
			glAlphaFunc ( GL_GREATER, ALPHA_CUTOFF_VALUE_3D ) ;
			GameEngine::restoreAmbient();
		}
#endif


	}






#else
	bool bDidEnableAlpha = false;

#ifndef USE_OPENGL2
	glPushMatrix();	

	// Note: The negation of the y axis is there because of our camera setup in
	// GameEngine::setup3DGlView(). If changing that, this might also need
	// to change.
	glTranslatef(fCenterX, -fCenterY, fCenterZ);		

	if(pRotations.x != 0)
		glRotatef(pRotations.x, 1, 0, 0);
	if(pRotations.y != 0)
		glRotatef(pRotations.y, 0, 1, 0);
	if(pRotations.z != 0)
		glRotatef(pRotations.z, 0, 0, 1);

	/*
	SVector3D svScale;
	pObj->getDefaultScale(svScale);
	svScale = svScale*fScale;
	if(svScale.x != 1.0 ||
	svScale.y != 1.0 ||
	svScale.z != 1.0)
	{
	glScalef(svScale.x, svScale.y, svScale.z);
	}
	*/
#else
	// Set the matrix we created above
	RenderStateManager::getInstance()->setModelMatrix(smWorldMatrix);
#endif


	bool bEnabledNormalize = false;
	if(fScale != 1.0)
	{
		// Negative scales screw up our normals!
		// Yes, OpenGL is that retarded. Who would ever want to 
		// scale normals by default is beyond me.
		_ASSERT(fScale > 0);

		// NOTE: This is going to be slow (constant on/off of state)
		// if many objects are scaled on the fly. Which they shouldn't be.
		GraphicsUtils::enableNormalization();
#ifndef USE_OPENGL2
		glScalef(fScale, fScale, fScale);
#endif
		bEnabledNormalize = true;
	}

	BlendModeType eOverallBlendMode = BlendModeNormal;
	TX_MAN_RETURN_TYPE iOverrideIdx = 0;
	if(pcsOverrideTextureAnim && strlen(pcsOverrideTextureAnim) > 0)
	{
		int iDummyW, iDummyH;
		SUVSet uvDummy;
		iOverrideIdx = pWindow->getTextureManager()->getTextureIndex(pcsOverrideTextureAnim, 0, iDummyW, iDummyH, uvDummy, eOverallBlendMode);
#ifdef ENABLE_TEXTURE_UNLOADING
		pWindow->getTextureManager()->ensureLoaded(pcsOverrideTextureAnim);
#endif
	}

	ShaderSetType eOldShaderType;
	SVertex3D* pData;
	TX_MAN_RETURN_TYPE uiTextureIndex = 0;
	TX_MAN_RETURN_TYPE txNormalMap = 0;
	TX_MAN_RETURN_TYPE txSpecularMap = 0;
	int iCurrPart, iNumParts = pFrame->getNumParts();
	for(iCurrPart = 0; iCurrPart < iNumParts; iCurrPart++)
	{
		pPart = pFrame->getPart(iCurrPart);
		pData = pPart->getData();

		txNormalMap = 0;
		txSpecularMap = 0;

		setObjectMaterial(pObj, pPart, fAlpha);

		if(iOverrideIdx == 0)
		{
#ifdef ENABLE_TEXTURE_UNLOADING
			pPart->ensureTexturesLoaded(pWindow->getTextureManager());
#endif
			uiTextureIndex = pPart->getTextureIndex();
			GraphicsUtils::setDiffuseTexture(uiTextureIndex);
			eOverallBlendMode = pPart->getBlendMode();
		}
		else
			GraphicsUtils::setDiffuseTexture(iOverrideIdx);

		bDidEnableAlpha = false;

#ifdef EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH
		if(eOverallBlendMode != BlendModeNormal)
		{
			if(bManageDepthTest)
				glDisable(GL_DEPTH_TEST);
		}
		else
		{

			if(fAlpha < 1.0)
				bDidEnableAlpha = true;
		}
#else
		if(eOverallBlendMode != BlendModeNormal)
		{
			_ASSERT(!bDidEnableAlpha);
			if(bManageDepthTest)
				glDisable(GL_DEPTH_TEST);
			glAlphaFunc ( GL_GREATER, ADDITIVE_OBJ3D_ALPHA_CUTOFF ) ;
			GameEngine::turnOnMaxAmbient();

		}
		else
		{

			if(fAlpha < 1.0)
			{
				GraphicsUtils::enableAlphaTest();
				bDidEnableAlpha = true;
			}
		}
#endif
		_ASSERT(eOverallBlendMode == BlendModeNormal);
		pWindow->setBlendMode(BlendModeText);
		//setBlendMode(eOverallBlendMode);

		eOldShaderType = ShaderSetLastPlaceholder;
#ifdef ENABLE_NORMAL_MAPPING
		if(pcsOverrideNormalMap && strlen(pcsOverrideNormalMap) > 0)
			txNormalMap = pWindow->getTextureManager()->getTextureIndex(pcsOverrideNormalMap);

		if(txNormalMap == 0)
		{
#ifdef ENABLE_TEXTURE_UNLOADING
			pPart->ensureTexturesLoaded();
#endif
			txNormalMap = pPart->getNormalMapTextureIndex();
		}

		// If we still don't have any, ask the object
		if(txNormalMap == 0)
			txNormalMap = pObj->getNormalMapIndex();

		if(txNormalMap != 0)
			eOldShaderType = setNormalMap(txNormalMap, pData);
#endif

#ifdef ENABLE_SPECULAR_MAPPING
		if(pcsOverrideSpecularMap && strlen(pcsOverrideSpecularMap) > 0)
			txSpecularMap = pWindow->getTextureManager()->getTextureIndex(pcsOverrideSpecularMap);

		if(txSpecularMap == 0)
		{
#ifdef ENABLE_TEXTURE_UNLOADING
			pPart->ensureTexturesLoaded();
#endif
			txSpecularMap = pPart->getSpecularMapTextureIndex();
		}

		// If we still don't have any, ask the object
		if(txSpecularMap == 0)
			txSpecularMap = pObj->getSpecularMapIndex();

		if(txSpecularMap != 0)
			setSpecularMap(txSpecularMap);
#endif

		// See if we have a VBO object
#ifdef ENABLE_VBOS
		GLuint iVBOIndex = pPart->getVBOHandle();
		if(iVBOIndex > 0)
		{
			RenderUtils::drawVBO(iVBOIndex, SVertex3D::getNumVertexComponents(), sizeof(float)*4, sizeof(float)*7, pPart->getNumVerts(), sizeof(SVertex3D));
		}
		else
#endif

			RenderUtils::drawBuffer<SVertex3D>(&pData->x, SVertex3D::getNumVertexComponents(), &pData->u, NULL, &pData->nx, pPart->getNumVerts());

		/*
		glVertexPointer( 3, GL_FLOAT, sizeof(SVertex3D), &pData->x);
		glNormalPointer(GL_FLOAT, sizeof(SVertex3D), &pData->nx);
		glTexCoordPointer( 2, GL_FLOAT, sizeof(SVertex3D), &pData->u);
		glDrawArrays( GL_TRIANGLES, 0, pPart->getNumVerts() );
		*/

		if(bDidEnableAlpha)
			GraphicsUtils::disableAlphaTest();


#ifdef EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH
		if(eOverallBlendMode != BlendModeNormal && bManageDepthTest)
			glEnable(GL_DEPTH_TEST);

#else

		if(eOverallBlendMode != BlendModeNormal)
		{
			if(bManageDepthTest)
				glEnable(GL_DEPTH_TEST);
			glAlphaFunc ( GL_GREATER, ALPHA_CUTOFF_VALUE_3D ) ;
			GameEngine::restoreAmbient();
		}
#endif

#ifdef ENABLE_NORMAL_MAPPING
		if(eOldShaderType != ShaderSetLastPlaceholder)
			resetNormalMap(eOldShaderType);
#endif


	}
	//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//	glDisableClientState(GL_COLOR_ARRAY);


	//	glEnable(GL_TEXTURE_2D);

	if(bEnabledNormalize)
		GraphicsUtils::disableNormalization();

#ifdef USE_OPENGL2
	RenderStateManager::getInstance()->resetModelMatrix();
	RenderStateManager::getInstance()->resetMaterials();
#else
	glPopMatrix();
#endif

#ifdef ENABLE_NORMAL_MAPPING
	resetSpecularMap();
#endif

#endif

#endif // #if 0
}
/*****************************************************************************/
void RenderUtils::renderCustomObject3D(Window* pWindow, CachedSequence* pTexture, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fCenterZ, FLOAT_TYPE fAlpha, FLOAT_TYPE fScale, SVertex3D* pVertexData, int iNumVerts, SVector3D &pRotations, SColor& scolDiffuseMults, SColor& scolAmbMults)
{

#if defined(DIRECTX_PIPELINE) || defined(USE_OPENGL2)

	SMatrix4 smWorldMatrix;
	smWorldMatrix.resetToIdentity();

	smWorldMatrix.appendTranslation(fCenterX, -fCenterY, fCenterZ);

	if(pRotations.x != 0)
		smWorldMatrix.appendXRotation(sanitizeDegAngle(pRotations.x));
	if(pRotations.y != 0)
		smWorldMatrix.appendYRotation(sanitizeDegAngle(pRotations.y));
	if(pRotations.z != 0)
		smWorldMatrix.appendZRotation(sanitizeDegAngle(pRotations.z));

	if(fScale != 1.0)
		smWorldMatrix.appendScale(fScale, fScale, fScale);
#endif


#ifdef DIRECTX_PIPELINE
	DXSTAGE3
#else

#ifndef USE_OPENGL2
	glPushMatrix();	

	// Note: The negation of the y axis is there because of our camera setup in
	// GameEngine::setup3DGlView(). If changing that, this might also need
	// to change.
	glTranslatef(fCenterX, -fCenterY, fCenterZ);		

	if(pRotations.x != 0)
		glRotatef(pRotations.x, 1, 0, 0);
	if(pRotations.y != 0)
		glRotatef(pRotations.y, 0, 1, 0);
	if(pRotations.z != 0)
		glRotatef(pRotations.z, 0, 0, 1);

	// Find the corresponding part
	//CachedPart3D* pPart;
	///CachedObject3D* pObj = g_pObject3DManager->findObject(pcsObjType);
#else
	// Set the matrix we created above
	RenderStateManager::getInstance()->setModelMatrix(smWorldMatrix);
#endif

	bool bEnabledNormalize = false;
	if(fScale != 1.0)
	{
		// Negative scales screw up our normals!
		// Yes, OpenGL is that retarded. Who would ever want to 
		// scale normals by default is beyond me.
		_ASSERT(fScale > 0);

		// NOTE: This is going to be slow (constant on/off of state)
		// if many objects are scaled on the fly. Which they shouldn't be.
		GraphicsUtils::enableNormalization();
#ifndef USE_OPENGL2
		glScalef(fScale, fScale, fScale);
#endif
		bEnabledNormalize = true;
	}

	//CachedFrame3D* pFrame = pObj->getFrameForProgress(fAnimProgress, iClipIndex);
	scolDiffuseMults.alpha = fAlpha;
	SColor scolSpecular(0,0,0,0);
	setMaterial(scolDiffuseMults, scolAmbMults, scolSpecular, 1.0);

	pWindow->setBlendMode(BlendModeText);


	//setBlendMode(BlendModeDiscolour);
	// 	glEnable( GL_COLOR_MATERIAL );
	//glEnable(GL_BLEND); 

	//	glBlendFunc (GL_ONE, GL_ONE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// 	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	// 	glEnableClientState(GL_COLOR_ARRAY);

	//	SVertex3D* pData;
	TX_MAN_RETURN_TYPE uiTextureIndex;

	SUVSet uvsOut;
	BlendModeType eBlendMode;
	int iW, iH;
	if(pTexture)
		uiTextureIndex = pWindow->getTextureManager()->getTextureIndex(pTexture->getName(), 0, iW, iH, uvsOut, eBlendMode);
	else
		uiTextureIndex = 0;
	GraphicsUtils::setDiffuseTexture(uiTextureIndex);
	// #ifdef DIRECTX_PIPELINE
	// 	DXSTAGE1
	// #else
	// 	glBindTexture(GL_TEXTURE_2D, uiTextureIndex);
	// #endif

	RenderUtils::drawBuffer<SVertex3D>(&pVertexData->x, SVertex3D::getNumVertexComponents(), &pVertexData->u, NULL, &pVertexData->nx, iNumVerts);

	/*
	glVertexPointer( 3, GL_FLOAT, sizeof(SVertex3D), &pVertexData->x);
	glNormalPointer(GL_FLOAT, sizeof(SVertex3D), &pVertexData->nx);
	glTexCoordPointer( 2, GL_FLOAT, sizeof(SVertex3D), &pVertexData->u);
	//glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(SVertex3D), &pVertexData->r);
	glDrawArrays( GL_TRIANGLES, 0, iNumVerts );
	*/

	//glDisableClientState(GL_COLOR_ARRAY);

	if(bEnabledNormalize)
		GraphicsUtils::disableNormalization();

#ifdef USE_OPENGL2
	RenderStateManager::getInstance()->resetModelMatrix();
	RenderStateManager::getInstance()->resetMaterials();
#else
	glPopMatrix();	
#endif
#endif
}
/*****************************************************************************/
void RenderUtils::renderSimpleLightning(Window* pWindow, SVector2D& svFrom, SVector2D& svTo, const char* pcsTexture, FLOAT_TYPE fThickness, FLOAT_TYPE fThicknessArms, FLOAT_TYPE fProgress)
{
	if(!theLightningNoiseGen)
		theLightningNoiseGen = new PerlinNoiseGenerator(4, 8, 20, 1);

	const int iPointVar = 4;

	if(fThicknessArms <= 0)
		fThicknessArms = fThickness;

	if(fProgress < FLOAT_EPSILON)
	{
		// Generate our random parms now. We can't keep regenerating them since they will change
		// every frame then.
		theLightningPointOffset1 = RANDOM_INT_BETWEEN_INCLUSIVE(0, iPointVar) - iPointVar/2;
		theLightningPointOffset2 = RANDOM_INT_BETWEEN_INCLUSIVE(0, iPointVar) - iPointVar/2;
		theLightningSeed++;
		if(theLightningSeed > 100000)
			theLightningSeed = 1;
		theLightningNoiseGen->setSeed(theLightningSeed);

		theLightningAngle1 = RANDOM_FLOAT_BETWEEN_INCLUSIVE(42, 50);
		if(GET_PROBABILITY() < 0.5)
			theLightningAngle1 *= -1.0;

		theLightningAngle2 = RANDOM_FLOAT_BETWEEN_INCLUSIVE(42, 50);
		if(theLightningAngle1*theLightningAngle2 > 0)
			theLightningAngle2 *= -1;

		theLightningArmLength1 = RANDOM_FLOAT_BETWEEN_INCLUSIVE(0.3, 0.8);
		theLightningArmLength2 = RANDOM_FLOAT_BETWEEN_INCLUSIVE(0.3, 0.8);
	}

	SVector2D svDir;
	svDir = svTo - svFrom;
	int iOrigLen = svDir.normalize();
	int iSpacing = 10;
	const FLOAT_TYPE fOpacityCutoff = 0.25;

	FLOAT_TYPE fNoiseProg = fProgress*0.25;
	FLOAT_TYPE fAngle = svDir.getAngleFromPositiveXAccurate();

	theLightningLines[0].generateNoiseLineAtOrigin(iOrigLen, iSpacing, *theLightningNoiseGen, fNoiseProg);

	// Go through the points, and at a couple of them pick one or two others.
	int iNumLinePoints = theLightningLines[0].size();
	int iPointIdx;
	int iLineLen;

	// Do one tendril
	//iPointIdx = RANDOM_INT_BETWEEN_INCLUSIVE(iNumLinePoints/3 - iPointVar, iNumLinePoints/3 + iPointVar);
	iPointIdx = iNumLinePoints/3.0 + theLightningPointOffset1;
	if(iPointIdx < 0)
	iPointIdx = 0;
	else if(iPointIdx >= iNumLinePoints)
	iPointIdx = iNumLinePoints - 1;
	iLineLen = (1.0 - ((FLOAT_TYPE)iPointIdx/(FLOAT_TYPE)iNumLinePoints))*iOrigLen*theLightningArmLength1;
	theLightningLines[1].generateNoiseLineAtOrigin(iLineLen, iSpacing, *theLightningNoiseGen, fNoiseProg);
	theLightningLines[1].rotate(theLightningAngle1);
	theLightningLines[1].translate(theLightningLines[0][iPointIdx].x, theLightningLines[0][iPointIdx].y);
	theLightningLines[1].rotate(fAngle);
	theLightningLines[1].translate(svFrom.x, svFrom.y);

	// Do the next one
	//iPointIdx = RANDOM_INT_BETWEEN_INCLUSIVE(iNumLinePoints*0.66 - iPointVar, iNumLinePoints*0.66 + iPointVar);
	iPointIdx = iNumLinePoints*0.66 + theLightningPointOffset2;
	if(iPointIdx < 0)
		iPointIdx = 0;
	else if(iPointIdx >= iNumLinePoints)
		iPointIdx = iNumLinePoints - 1;
	iLineLen = (1.0 - ((FLOAT_TYPE)iPointIdx/(FLOAT_TYPE)iNumLinePoints))*iOrigLen*theLightningArmLength2;
	theLightningLines[2].generateNoiseLineAtOrigin(iLineLen, iSpacing, *theLightningNoiseGen, fNoiseProg);
	theLightningLines[2].rotate(theLightningAngle2);
	theLightningLines[2].translate(theLightningLines[0][iPointIdx].x, theLightningLines[0][iPointIdx].y);
	theLightningLines[2].rotate(fAngle);

	theLightningLines[2].translate(svFrom.x, svFrom.y);

	FLOAT_TYPE fActualOpacity = 1.0;
	if(fProgress < fOpacityCutoff)
		fActualOpacity = fProgress/fOpacityCutoff;
	else if(fProgress > 1.0 - fOpacityCutoff)
		fActualOpacity = 1.0 -(fProgress - (1.0 - fOpacityCutoff))/(fOpacityCutoff);

	// Render a glow:
	RenderUtils::drawBitmap(pWindow->getTextureManager(), "largePickupShadow", svFrom.x, svFrom.y, fActualOpacity, 0, 0.5, 0.25, 1.0, true);

	theLightningLines[0].rotate(fAngle);
	theLightningLines[0].translate(svFrom.x, svFrom.y);
	RenderUtils::renderTexturedPath(pWindow, theLightningLines[0], fThickness, pcsTexture, fActualOpacity);
	RenderUtils::renderTexturedPath(pWindow, theLightningLines[1], fThicknessArms, pcsTexture, fActualOpacity);
	RenderUtils::renderTexturedPath(pWindow, theLightningLines[2], fThicknessArms, pcsTexture, fActualOpacity);

}
/*****************************************************************************/
void RenderUtils::renderGradientRectangle(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, const SGradient& rGradient)
{
	// Get a homogeneous vector of stops with both opacities and colors multed in.
	rGradient.getSingleStopsArray(theGradientStops);

	int iNumStops = theGradientStops.size();
	if(iNumStops == 0)
		return;
	else if(iNumStops == 1)
	{
		RenderUtils::fillRectangle(fX, fY, fW, fH, (*theGradientStops.begin()).myColor, true);
		return;
	}

	// Otherwise, make sure it has the 0.0 and 1.0 positions...
	TGradientStops::iterator ti;
	ti = theGradientStops.begin();
	if((*ti).getPos() > 0.0)
	{
		SGradientStop rTempStop;
		rTempStop = *ti;
		rTempStop.setPos(0.0);
		theGradientStops.insert(rTempStop);
	}

	ti = theGradientStops.end();
	ti--;
	if((*ti).getPos() < 1.0)
	{
		SGradientStop rTempStop;
		rTempStop = *ti;
		rTempStop.setPos(1.0);
		theGradientStops.insert(rTempStop);
	}


	// Then create n - 1 rectangles where n is the number of total stops
	SColor scolCorners[4];
	TGradientStops::iterator si;
	TGradientStops::iterator pi = theGradientStops.begin();
	FLOAT_TYPE fCurrX = fX;
	FLOAT_TYPE fCurrW;
	const SGradientStop* pStop1, *pStop2;
	for(si = theGradientStops.begin(), si++; si != theGradientStops.end(); si++, pi++)
	{
		// Now, figure out the colors and the sizes
		pStop1 = &(*pi);
		pStop2 = &(*si);

		scolCorners[0] = pStop1->myColor;
		scolCorners[2] = pStop1->myColor;

		scolCorners[1] = pStop2->myColor;
		scolCorners[3] = pStop2->myColor;

		fCurrW = (pStop2->getPos() - pStop1->getPos())*fW;
		RenderUtils::fillRectangleCustomColors(fCurrX, fY, fCurrW, fH, scolCorners, true);

		fCurrX += fCurrW;
	}
}
/*****************************************************************************/
SVertexInfo *g_pProgCircleRendVertices = NULL;

void RenderUtils::renderCircularProgress(Window* pWindow, FLOAT_TYPE fProgress, const char* pcsTexture, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius, FLOAT_TYPE fOpacity, FLOAT_TYPE fStartAngle, FLOAT_TYPE fAngleSpan, bool bPremultiplyAlpha, HyperCore::OrientationType eOrientation, const SColor* pOptFillColor)
{
	_ASSERT(fProgress >= 0 && fProgress <= 1.0);
	const FLOAT_TYPE fNumCirclePoints = 720.0; // 720.0;
	DrawingCache* pDrawingCache = pWindow->getDrawingCache();
	TextureManager* pTextureManager = pWindow->getTextureManager();

	bool bIsCCW;
	if(eOrientation == OrientationCCW)
		bIsCCW = true;
	else if(eOrientation == OrientationCW)
		bIsCCW = false;
	else
		bIsCCW = fProgress < 0.5;

	int iEndPoint = fProgress*(FLOAT_TYPE)(fNumCirclePoints*(fAngleSpan/360.0));
	if(iEndPoint <= 0)
		return;
	if(iEndPoint > fNumCirclePoints)
		iEndPoint = fNumCirclePoints;

	// num triangels is equal to num points
	int iNumTris = iEndPoint;

	SColor scolFinalCol(1,1,1,1);
	if(pOptFillColor)
		scolFinalCol = *pOptFillColor;

	if(!g_pProgCircleRendVertices)
		g_pProgCircleRendVertices = new SVertexInfo[(int)(fNumCirclePoints + 1)*3 + 3];

	// Overwritten onf irst iter
 	SVector2D svPrevPoint(fX, fY - fRadius);
 	SVector2D svPrevUVs(0.5, 0.0);

	SVector2D svNewUVs;

	int iArrayPointer = 0;
	int iCurr, iNum = fNumCirclePoints;
	FLOAT_TYPE fAngle;
	for(iCurr = 0; iCurr <= iEndPoint; iCurr++)
	{
#ifdef _DEBUG
		if(iCurr == iEndPoint)
		{
			int bp = 0;
		}
#endif
		fAngle = (FLOAT_TYPE)iCurr/(FLOAT_TYPE)iNum*360.0;

		if(bIsCCW)
			fAngle = HyperCore::sanitizeDegAngle(-fAngle - fStartAngle);
		else
			fAngle = HyperCore::sanitizeDegAngle(fAngle - fStartAngle);


		//fAngle = ((FLOAT_TYPE)iCurr/(FLOAT_TYPE)iEndPoint)*fAngleSpan - fStartAngle;
		//fAngle = sanitizeDegAngle(fAngle);

		g_pProgCircleRendVertices[iArrayPointer + 0].x = fX;
		g_pProgCircleRendVertices[iArrayPointer + 0].y = fY;
#ifdef USE_OPENGL2
		g_pProgCircleRendVertices[iArrayPointer + 0].z = 0;
		g_pProgCircleRendVertices[iArrayPointer + 0].w = 1;
#endif
		g_pProgCircleRendVertices[iArrayPointer + 0].s0 = 0.5;
		g_pProgCircleRendVertices[iArrayPointer + 0].t0 = 0.5;
#ifdef USE_OPENGL2
		g_pProgCircleRendVertices[iArrayPointer + 0].r = scolFinalCol.r;
		g_pProgCircleRendVertices[iArrayPointer + 0].g = scolFinalCol.g;
		g_pProgCircleRendVertices[iArrayPointer + 0].b = scolFinalCol.b;
		g_pProgCircleRendVertices[iArrayPointer + 0].a = fOpacity*scolFinalCol.alpha;
#else
		g_pProgCircleRendVertices[iArrayPointer + 0].r = 255.0*scolFinalCol.r;
		g_pProgCircleRendVertices[iArrayPointer + 0].g = 255.0*scolFinalCol.g;
		g_pProgCircleRendVertices[iArrayPointer + 0].b = 255.0*scolFinalCol.b;
		g_pProgCircleRendVertices[iArrayPointer + 0].a = fOpacity*255.0*scolFinalCol.alpha;
#endif

		if(bPremultiplyAlpha)
		{
			g_pProgCircleRendVertices[iArrayPointer + 0].r = ((FLOAT_TYPE)g_pProgCircleRendVertices[iArrayPointer + 0].r)*fOpacity*scolFinalCol.alpha;
			g_pProgCircleRendVertices[iArrayPointer + 0].g = ((FLOAT_TYPE)g_pProgCircleRendVertices[iArrayPointer + 0].g)*fOpacity*scolFinalCol.alpha;
			g_pProgCircleRendVertices[iArrayPointer + 0].b = ((FLOAT_TYPE)g_pProgCircleRendVertices[iArrayPointer + 0].b)*fOpacity*scolFinalCol.alpha;
		}



		g_pProgCircleRendVertices[iArrayPointer + 1].x = svPrevPoint.x;
		g_pProgCircleRendVertices[iArrayPointer + 1].y = svPrevPoint.y;
#ifdef USE_OPENGL2
		g_pProgCircleRendVertices[iArrayPointer + 1].z = 0;
		g_pProgCircleRendVertices[iArrayPointer + 1].w = 1;
#endif
		g_pProgCircleRendVertices[iArrayPointer + 1].s0 = svPrevUVs.x;
		g_pProgCircleRendVertices[iArrayPointer + 1].t0 = svPrevUVs.y;
#ifdef USE_OPENGL2
		g_pProgCircleRendVertices[iArrayPointer + 1].r = scolFinalCol.r;
		g_pProgCircleRendVertices[iArrayPointer + 1].g = scolFinalCol.g;
		g_pProgCircleRendVertices[iArrayPointer + 1].b = scolFinalCol.b;
		g_pProgCircleRendVertices[iArrayPointer + 1].a = fOpacity*scolFinalCol.alpha;
#else
		g_pProgCircleRendVertices[iArrayPointer + 1].r = 255.0*scolFinalCol.r;
		g_pProgCircleRendVertices[iArrayPointer + 1].g = 255.0*scolFinalCol.g;
		g_pProgCircleRendVertices[iArrayPointer + 1].b = 255.0*scolFinalCol.b;
		g_pProgCircleRendVertices[iArrayPointer + 1].a = fOpacity*255.0*scolFinalCol.alpha;
#endif

		if(bPremultiplyAlpha)
		{
			g_pProgCircleRendVertices[iArrayPointer + 1].r = ((FLOAT_TYPE)g_pProgCircleRendVertices[iArrayPointer + 1].r)*fOpacity*scolFinalCol.alpha;
			g_pProgCircleRendVertices[iArrayPointer + 1].g = ((FLOAT_TYPE)g_pProgCircleRendVertices[iArrayPointer + 1].g)*fOpacity*scolFinalCol.alpha;
			g_pProgCircleRendVertices[iArrayPointer + 1].b = ((FLOAT_TYPE)g_pProgCircleRendVertices[iArrayPointer + 1].b)*fOpacity*scolFinalCol.alpha;
		}



		g_pProgCircleRendVertices[iArrayPointer + 2].x = F_COS_DEG(fAngle)*fRadius + fX;
		g_pProgCircleRendVertices[iArrayPointer + 2].y = F_SIN_DEG(fAngle)*fRadius + fY;
#ifdef USE_OPENGL2
		g_pProgCircleRendVertices[iArrayPointer + 2].z = 0;
		g_pProgCircleRendVertices[iArrayPointer + 2].w = 1;
#endif
		svNewUVs.x = (g_pProgCircleRendVertices[iArrayPointer + 2].x - (fX - fRadius))/(fRadius*2.0);
		svNewUVs.y = (g_pProgCircleRendVertices[iArrayPointer + 2].y - (fY - fRadius))/(fRadius*2.0);
		if(svNewUVs.x < 0.0)
			svNewUVs.x = 0;
		if(svNewUVs.x > 1.0)
			svNewUVs.x = 1.0;
		if(svNewUVs.y < 0.0)
			svNewUVs.y = 0;
		if(svNewUVs.y > 1.0)
			svNewUVs.y = 1.0;
		g_pProgCircleRendVertices[iArrayPointer + 2].s0 = svNewUVs.x;
		g_pProgCircleRendVertices[iArrayPointer + 2].t0 = svNewUVs.y;
#ifdef USE_OPENGL2
		g_pProgCircleRendVertices[iArrayPointer + 2].r = scolFinalCol.r;
		g_pProgCircleRendVertices[iArrayPointer + 2].g = scolFinalCol.g;
		g_pProgCircleRendVertices[iArrayPointer + 2].b = scolFinalCol.b;
		g_pProgCircleRendVertices[iArrayPointer + 2].a = fOpacity*scolFinalCol.alpha;
#else
		g_pProgCircleRendVertices[iArrayPointer + 2].r = 255.0*scolFinalCol.r;
		g_pProgCircleRendVertices[iArrayPointer + 2].g = 255.0*scolFinalCol.g;
		g_pProgCircleRendVertices[iArrayPointer + 2].b = 255.0*scolFinalCol.b;
		g_pProgCircleRendVertices[iArrayPointer + 2].a = fOpacity*255.0*scolFinalCol.alpha;
#endif

		if(bPremultiplyAlpha)
		{
			g_pProgCircleRendVertices[iArrayPointer + 2].r = ((FLOAT_TYPE)g_pProgCircleRendVertices[iArrayPointer + 2].r)*fOpacity*scolFinalCol.alpha;
			g_pProgCircleRendVertices[iArrayPointer + 2].g = ((FLOAT_TYPE)g_pProgCircleRendVertices[iArrayPointer + 2].g)*fOpacity*scolFinalCol.alpha;
			g_pProgCircleRendVertices[iArrayPointer + 2].b = ((FLOAT_TYPE)g_pProgCircleRendVertices[iArrayPointer + 2].b)*fOpacity*scolFinalCol.alpha;
		}



		svPrevPoint.x = g_pProgCircleRendVertices[iArrayPointer + 2].x;
		svPrevPoint.y = g_pProgCircleRendVertices[iArrayPointer + 2].y;
		svPrevUVs.x = g_pProgCircleRendVertices[iArrayPointer + 2].s0;
		svPrevUVs.y = g_pProgCircleRendVertices[iArrayPointer + 2].t0;

		// The first iter just computes the prev pointers
		if(iCurr > 0)
			iArrayPointer += 3;
	}

	if(pcsTexture)
	{
		int iW, iH;
		TX_MAN_RETURN_TYPE iIndex;
		BlendModeType eBlendMode, ePrevMode;
		SUVSet svTextFullUVs;
		CachedSequence* pSeq = pDrawingCache->getCachedSequence(pcsTexture, NULL);
		iIndex = pTextureManager->getTextureIndex(pSeq->getName(), 0, iW, iH, svTextFullUVs, eBlendMode);
		pWindow->setBlendMode(eBlendMode);
		GraphicsUtils::setDiffuseTexture(iIndex);
	}


	RenderUtils::drawBuffer<SVertexInfo>(&g_pProgCircleRendVertices[0].x, SVertexInfo::getNumVertexComponents(), &g_pProgCircleRendVertices[0].s0, &g_pProgCircleRendVertices[0].r, NULL, iArrayPointer) ;
}
/*****************************************************************************/
void RenderUtils::renderCustomObject2D(Window* pWindow, SVertexInfo* pVerts, int iNumVerts, bool bManageState)
{
	BlendModeType eOldMode = BlendModeLastPlaceholder;
	if(bManageState)
	{
		eOldMode = pWindow->setBlendMode(BlendModeText);
		//eOldMode = setBlendMode(BlendModeNormal);
		GraphicsUtils::enableFlatDrawingState(-1.0);
	}


#ifdef DIRECTX_PIPELINE
	DXSTAGE2
#else

#ifdef USE_OPENGL2
	RenderUtils::drawBuffer<SVertexInfo>(&pVerts[0].x, SVertexInfo::getNumVertexComponents(), NULL, &pVerts[0].r, NULL, iNumVerts);
#else

#ifdef USE_INT_OPENGL_POSITIONS
	OpenGLStateCleaner::doVertexPointerCall(2, GL_SHORT, sizeof(SVertexInfo), &pVerts[0].x);
#else
	OpenGLStateCleaner::doVertexPointerCall(2, GL_FLOAT, sizeof(SVertexInfo), &pVerts[0].x);
#endif
	OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, sizeof(SVertexInfo), &pVerts[0].r);
	glDrawArrays(GL_TRIANGLES, 0, iNumVerts);
	//glDrawArrays(GL_LINE_STRIP, 0, iNumVerts);
#endif
#endif

	if(bManageState)
		GraphicsUtils::disableFlatDrawingState(-1.0);
	if(eOldMode != BlendModeLastPlaceholder)
		pWindow->setBlendMode(eOldMode);
}
/*****************************************************************************/
int RenderUtils::shortenStringFromTheFront(Window* pWindow, string& strInOut, const char* pcsFont, int iFontSize, FLOAT_TYPE fMaxWidth, const char* pcsElipsis, bool bSlowButAccurate)
{
	SVector2D svTextDims;
	int iRes = 0;
	SVector2D svTriPointSize;
	if(bSlowButAccurate)
		RenderUtils::measureText(pWindow, pcsElipsis, pcsFont, iFontSize, 0, svTriPointSize);
	else
		RenderUtils::measureTextRough(pWindow, pcsElipsis, pcsFont, iFontSize, 0, svTriPointSize);

	// Keep growing the text while we plus the tri thingie are under the
	// size limit:
	int iCurr, iLen = strInOut.length();
	string strTemp;
	char pcsSmallBuff[2] = { 0, 0 };

	for(iCurr = iLen - 1; iCurr >= 0; iCurr--)
	{
		pcsSmallBuff[0] = strInOut[iCurr];
		strTemp += pcsSmallBuff;
		if(bSlowButAccurate)
			RenderUtils::measureText(pWindow, strTemp.c_str(), pcsFont, iFontSize, 0, svTextDims);
		else
			RenderUtils::measureTextRough(pWindow, strTemp.c_str(), pcsFont, iFontSize, 0, svTextDims);

		if(svTextDims.x + svTriPointSize.x > fMaxWidth)
		{
			// Change the resultant out cursor pos depending on how much we're cutting
			//iRes = (iCurr + 1) + strlen(pcsElipsis);
			iRes = (iCurr + 1);

			strInOut = pcsElipsis + strInOut.substr(iCurr + 1);
			break;
		}
	}

	return iRes;
}
/*****************************************************************************/
TVertexLossyDynamicArray g_pSubdividedRectVertices;

void RenderUtils::renderSubdividedRectangle(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fOpacity, IVertexColorGetter* pCallback, FLOAT_TYPE fTriSideLength)
{
	int iNumSubdivsX = fW/fTriSideLength;
	int iNumSubdivsY = fH/fTriSideLength;

	SVector2D svSteps(fW/(FLOAT_TYPE)iNumSubdivsX, fH/(FLOAT_TYPE)iNumSubdivsY);
	if(svSteps.x <= FLOAT_EPSILON || svSteps.y <= FLOAT_EPSILON)
		ASSERT_RETURN;

	int iNumGeoVerts = (iNumSubdivsX*iNumSubdivsY)*6;
	SVertexInfo* pVertArray = g_pSubdividedRectVertices.getArray(iNumGeoVerts);

	int iArrayPos = 0;

	// Otherwise, start rendering
	SVector2D svStartPoint, svEndPoint;
	SVector2D svRectPoints[4];
	SColor scolColors[4];
	SColorPoint rColorPoints[4];
	SVector2D svOrigin(fX, fY);
	int iVert;
	int iSegmentX, iSegmentY;
	for(iSegmentY = 0; iSegmentY < iNumSubdivsY; iSegmentY++)
	{
		for(iSegmentX = 0; iSegmentX < iNumSubdivsX; iSegmentX++)
		{
			svStartPoint.set(iSegmentX*svSteps.x, iSegmentY*svSteps.y);
			svEndPoint.set((iSegmentX + 1)*svSteps.x, (iSegmentY + 1)*svSteps.y);

			svRectPoints[0] = svStartPoint;
			svRectPoints[1].set(svEndPoint.x, svStartPoint.y);
			svRectPoints[2].set(svEndPoint.x, svEndPoint.y);
			svRectPoints[3].set(svStartPoint.x, svEndPoint.y);

			for(iVert = 0; iVert < 4; iVert++)
			{
				pCallback->getVertexColor(svRectPoints[iVert].x/fW, svRectPoints[iVert].y/fH, scolColors[iVert]);
				svRectPoints[iVert] += svOrigin;

				scolColors[iVert].alpha = fOpacity;
				rColorPoints[iVert].setPoint(svRectPoints[iVert].x, svRectPoints[iVert].y, scolColors[iVert]);
			}

			pVertArray[iArrayPos + 0].copyFrom(rColorPoints[0]);
			pVertArray[iArrayPos + 1].copyFrom(rColorPoints[1]);
			pVertArray[iArrayPos + 2].copyFrom(rColorPoints[3]);

			pVertArray[iArrayPos + 3].copyFrom(rColorPoints[1]);
			pVertArray[iArrayPos + 4].copyFrom(rColorPoints[2]);
			pVertArray[iArrayPos + 5].copyFrom(rColorPoints[3]);

			iArrayPos += 6;
		}
	}

	GraphicsUtils::enableFlatDrawingState(-1);
	RenderUtils::drawBuffer<SVertexInfo>(&pVertArray[0].x, SVertexInfo::getNumVertexComponents(), NULL, &pVertArray[0].r, NULL, iArrayPos);
	GraphicsUtils::disableFlatDrawingState(-1);
}
/*****************************************************************************/
void RenderUtils::renderRectangle(Window* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fThickness, const SColor& scolFill, bool bManageState)
{
	if(fThickness <= 0)
		RenderUtils::fillRectangle(fX, fY, fW, fH, scolFill, bManageState);
	else
		RenderUtils::drawRectangle(pWindow, fX, fY, fW, fH, scolFill, fThickness, bManageState);
}
/*****************************************************************************/
bool RenderUtils::doesIntersectScissorRect(const SRect2D& srRect)
{
	SRect2D srScissorRect;
	if(RenderUtils::getCurrentScissorRectangle(srScissorRect))
	{
		if(!srScissorRect.doesIntersect(srRect))
			return false;
	}
	return true;
}
/*****************************************************************************/
bool RenderUtils::isInsideScissor()
{
	return theScissorRectsStack.size() > 0;
}
/*****************************************************************************/
#ifdef HARDCORE_LINUX_DEBUG_MODE
void RenderUtils::drawOrMeasureTextDEBUG1(DrawingCache* pDrawingCache, const char* pcsText, const char* pcsFontName, FLOAT_TYPE fX, FLOAT_TYPE fY, const SColor &scolText,
	HorAlignType eHorAlign, VertAlignType eVertAlign,
	FLOAT_TYPE fExtraKerning, SVector2D* svStringMeasurementsOut, const SColor* scolShadow, SVector2D* svShadowOffset,
	FLOAT_TYPE fW, SColor* pHighlightColor, FLOAT_TYPE fScale)

{
#ifdef HARDCORE_LINUX_DEBUG_MODE
	static int g_iDbLogCounter = 0;
	static int g_iDbLogCounter2 = 0;

	bool bTrack = false;
	if(pcsText && IS_STRING_EQUAL(pcsText, "test1_debug1") && !svStringMeasurementsOut)
	{
		g_iDbLogCounter++;
		if(g_iDbLogCounter == 2)
			bTrack = true;
	}	

	if(pcsText && IS_STRING_EQUAL(pcsText, "test1") && !svStringMeasurementsOut)
	{
		g_iDbLogCounter2++;
		if(g_iDbLogCounter2 == 2)
			bTrack = true;
	}	

	if(bTrack)
		Logger::log("TEXTTRACK_DB1: Begin %s at (%g, %g) scolText = (%g,%g,%g,%g) ha = %d va = %d s = %g", pcsFontName, fX, fY, scolText.r, scolText.g, scolText.b, scolText.alpha, eHorAlign, eVertAlign, fScale);
#endif

	// Find the right sequence
	ResourceCollection* pFonts = ResourceManager::getInstance()->getCollection(ResourceRasterFonts);
	ResourceItem* pItem = pFonts->getItemById(pcsFontName);
#ifdef HARDCORE_LINUX_DEBUG_MODE
	if(bTrack)
		Logger::log("TEXTTRACK_DB1: item = %x", pItem);
#endif
	if(!pItem)
		ASSERT_RETURN;

	if(fProgress > 1.0)
		fProgress = 1.0;

	SVector2D svAlignmentOffsets(0,0);

	FLOAT_TYPE fCharWidth, fCharUVWidth;
	SExtraSeqDrawInfo rExtraInfo, rExtraShadowInfo;
	const char* pcsAllChars = pItem->getStringProp(PropertyFtChars);
	int iCurr, iNum;
	char cChar;
	int iCharIdx;

	FLOAT_TYPE fSpaceWidth = pItem->getNumericEnumPropValue(PropertyFtCharWidths, strchr(pcsAllChars, SPACE_WIDTH_REFERENCE_LETTER) - pcsAllChars)*fScale;
	FLOAT_TYPE fLineUVWidth = pItem->getNumProp(PropertyFtHeightUV);
	FLOAT_TYPE fLinePixHeight = pItem->getNumProp(PropertyFtHeight)*fScale;
	FLOAT_TYPE fKerning = pItem->getNumProp(PropertyKerning)*fScale;
	fKerning += fExtraKerning*fScale;

	// Now, go letter by letter, computing the right coordinates on the spot
	iNum = strlen(pcsText);

	int iLastNewlinePos = -1;
	int iLineLen;
	for(iCurr = 0; iCurr <= iNum; iCurr++)
	{
		if(pcsText[iCurr] == '\n' || pcsText[iCurr] == '^' || (iLastNewlinePos >= 0 && pcsText[iCurr] == 0))
		{
			// Newline!
			if(iLastNewlinePos < 0)
				theTextLineLengths.clear();

			iLineLen = iCurr - iLastNewlinePos - 1;
			theTextLineLengths.push_back(iLineLen);
			iLastNewlinePos = iCurr;
		}
	}

	int iNextStopDrawingPos = iNum;
	if(iLastNewlinePos < 0)
	{
		iNum = ((FLOAT_TYPE)iNum)*fProgress;
		iNextStopDrawingPos = iNum;
	}
	else
		iNextStopDrawingPos = ((FLOAT_TYPE)theTextLineLengths[0])*fProgress;

	const char* pcsFontAnim = pItem->getStringProp(PropertyImage);
	CachedSequence* pFontSeq = pDrawingCache->getCachedSequence(pcsFontAnim, NULL);
	int iFontFileW, iFontFileH;
	pDrawingCache->getImageSize(pcsFontAnim, iFontFileW, iFontFileH);
#ifdef HARDCORE_LINUX_DEBUG_MODE
	if(bTrack)
		Logger::log("TEXTTRACK_DB1: pFontSeq = %x cachedTextIdx = %d fontFileDims = %dx%d iNum = %d", pFontSeq, pFontSeq->getCachedTexIndexDEBUG(), iFontFileW, iFontFileH, iNum);
#endif
	FLOAT_TYPE fSingleUPixel = 0.0;
	if(iFontFileW > 0)
		fSingleUPixel = EXTRA_FONT_PIXEL_PADDING/(FLOAT_TYPE)iFontFileW;

	rExtraInfo.myPolyColor = scolText;
	rExtraInfo.myH = fLinePixHeight;

	if(scolShadow && svShadowOffset)
	{
		rExtraShadowInfo = rExtraInfo;
		rExtraShadowInfo.myPolyColor = *scolShadow;
	}

	bool bIsInHighlightPart = false;
	SUVSet rUVSet;
	FLOAT_TYPE iX = fX + svAlignmentOffsets.x, iY = fY + fLinePixHeight/2 + svAlignmentOffsets.y;
	int iCurrLineCount = 0;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		cChar = pcsText[iCurr];

		if(iCurr >= iNextStopDrawingPos)
			continue;

		// Find it in the string
		iCharIdx = strchr(pcsAllChars, cChar) - pcsAllChars;

		if(iCharIdx < 0)
		{
			// Treat as space, skip.
			fCharWidth = fSpaceWidth;
			iX += fCharWidth/2.0;
		}
		else
		{
			// Get all the metrics, print it:
			fCharWidth = pItem->getNumericEnumPropValue(PropertyFtCharWidths, iCharIdx)*fScale;
			fCharUVWidth = pItem->getNumericEnumPropValue(PropertyFtCharWidthsUV, iCharIdx);

			iX += fCharWidth/2.0;

			rUVSet.myStartX = pItem->getNumericEnumPropValue(PropertyFtCharXs, iCharIdx) - fSingleUPixel;
			rUVSet.myStartY = pItem->getNumericEnumPropValue(PropertyFtCharYs, iCharIdx);
			rUVSet.myEndX = rUVSet.myStartX + fCharUVWidth + fSingleUPixel*2.0;
			rUVSet.myEndY = rUVSet.myStartY + fLineUVWidth;
			rUVSet.myApplyToSize = true;

			rExtraInfo.myW = fCharWidth + (int)(EXTRA_FONT_PIXEL_PADDING*2.0);

			if(!svStringMeasurementsOut)
			{
#ifdef HARDCORE_LINUX_DEBUG_MODE
				if(bTrack)
					Logger::log("TEXTTRACK_DB1: char = %c pt = (%g, %g) uvs = (%g, %g, %g, %g), extraSz = (%g, %g) extraCol = (%g, %g, %g, %g)  fSingleUPix = %g", cChar, iX, iY, rUVSet.myStartX, rUVSet.myStartY, rUVSet.myEndX, rUVSet.myEndY, rExtraInfo.myW, rExtraInfo.myH, rExtraInfo.myPolyColor.r, rExtraInfo.myPolyColor.g, rExtraInfo.myPolyColor.b, rExtraInfo.myPolyColor.alpha, fSingleUPixel);
#endif

				pFontSeq->addSprite(iX, iY,
					scolText.alpha, // Alpha
					0.0, fScale, fScale,
					1.0, //  anim progress
					true, true, &rUVSet, false, &rExtraInfo);
			}
		}


		iX += fCharWidth/2.0 + fKerning;
	}

#ifdef HARDCORE_LINUX_DEBUG_MODE
	if(bTrack)
		Logger::log("TEXTTRACK_DB1: done");
#endif
}
/*****************************************************************************/
void RenderUtils::drawOrMeasureTextDEBUG2(DrawingCache* pDrawingCache, const char* pcsText, const char* pcsFontName, FLOAT_TYPE fX, FLOAT_TYPE fY, const SColor &scolText,
	HorAlignType eHorAlign, VertAlignType eVertAlign,
	FLOAT_TYPE fExtraKerning, SVector2D* svStringMeasurementsOut, const SColor* scolShadow, SVector2D* svShadowOffset,
	FLOAT_TYPE fW, SColor* pHighlightColor, FLOAT_TYPE fScale)

{
	// Find the right sequence
	ResourceCollection* pFonts = ResourceManager::getInstance()->getCollection(ResourceRasterFonts);
	ResourceItem* pItem = pFonts->getItemById(pcsFontName);
	if(!pItem)
		ASSERT_RETURN;

	SVector2D svAlignmentOffsets(0,0);

	FLOAT_TYPE fCharWidth, fCharUVWidth;
	SExtraSeqDrawInfo rExtraInfo;
	const char* pcsAllChars = pItem->getStringProp(PropertyFtChars);
	
	FLOAT_TYPE fSpaceWidth = pItem->getNumericEnumPropValue(PropertyFtCharWidths, strchr(pcsAllChars, SPACE_WIDTH_REFERENCE_LETTER) - pcsAllChars)*fScale;
	FLOAT_TYPE fLineUVWidth = pItem->getNumProp(PropertyFtHeightUV);
	FLOAT_TYPE fLinePixHeight = pItem->getNumProp(PropertyFtHeight)*fScale;
	FLOAT_TYPE fKerning = pItem->getNumProp(PropertyKerning)*fScale;
	fKerning += fExtraKerning*fScale;

	// Now, go letter by letter, computing the right coordinates on the spot
	int iCurr, iNum = strlen(pcsText);

	const char* pcsFontAnim = pItem->getStringProp(PropertyImage);
	CachedSequence* pFontSeq = pDrawingCache->getCachedSequence(pcsFontAnim, NULL);
	int iFontFileW, iFontFileH;
	pDrawingCache->getImageSize(pcsFontAnim, iFontFileW, iFontFileH);

	FLOAT_TYPE fSingleUPixel = 0.0;
	if(iFontFileW > 0)
		fSingleUPixel = EXTRA_FONT_PIXEL_PADDING/(FLOAT_TYPE)iFontFileW;

	rExtraInfo.myPolyColor = scolText;
	rExtraInfo.myH = fLinePixHeight;

	bool bIsInHighlightPart = false;
	SUVSet rUVSet;
	FLOAT_TYPE iX = fX + svAlignmentOffsets.x, iY = fY + fLinePixHeight/2 + svAlignmentOffsets.y;
	int iCurrLineCount = 0;
	char cChar;
	int iCharIdx;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		cChar = pcsText[iCurr];

		// Find it in the string
		iCharIdx = strchr(pcsAllChars, cChar) - pcsAllChars;

		if(iCharIdx < 0)
		{
			// Treat as space, skip.
			fCharWidth = fSpaceWidth;
			iX += fCharWidth/2.0;
		}
		else
		{
			// Get all the metrics, print it:
			fCharWidth = pItem->getNumericEnumPropValue(PropertyFtCharWidths, iCharIdx)*fScale;
			fCharUVWidth = pItem->getNumericEnumPropValue(PropertyFtCharWidthsUV, iCharIdx);

			iX += fCharWidth/2.0;

			rUVSet.myStartX = pItem->getNumericEnumPropValue(PropertyFtCharXs, iCharIdx) - fSingleUPixel;
			rUVSet.myStartY = pItem->getNumericEnumPropValue(PropertyFtCharYs, iCharIdx);
			rUVSet.myEndX = rUVSet.myStartX + fCharUVWidth + fSingleUPixel*2.0;
			rUVSet.myEndY = rUVSet.myStartY + fLineUVWidth;
			rUVSet.myApplyToSize = true;

			rExtraInfo.myW = fCharWidth + (int)(EXTRA_FONT_PIXEL_PADDING*2.0);

			if(!svStringMeasurementsOut)
			{
				pFontSeq->addSprite(iX, iY,
					scolText.alpha, // Alpha
					0.0, fScale, fScale,
					1.0, //  anim progress
					true, true, &rUVSet, false, &rExtraInfo);
			}
		}

		iX += fCharWidth/2.0 + fKerning;
	}
}
/*****************************************************************************/
void RenderUtils::drawOrMeasureTextDEBUG3(DrawingCache* pDrawingCache, const char* pcsText, const char* pcsFontName, FLOAT_TYPE fX, FLOAT_TYPE fY, const SColor &scolText,
	HorAlignType eHorAlign, VertAlignType eVertAlign,
	FLOAT_TYPE fExtraKerning, SVector2D* svStringMeasurementsOut, const SColor* scolShadow, SVector2D* svShadowOffset,
	FLOAT_TYPE fW, SColor* pHighlightColor, FLOAT_TYPE fScale)

{
	// Find the right sequence
	ResourceCollection* pFonts = ResourceManager::getInstance()->getCollection(ResourceRasterFonts);
	ResourceItem* pItem = pFonts->getItemById(pcsFontName);
	if(!pItem)
		ASSERT_RETURN;

	SVector2D svAlignmentOffsets(0,0);

	FLOAT_TYPE fCharWidth, fCharUVWidth;
	SExtraSeqDrawInfo rExtraInfo;
	const char* pcsAllChars = pItem->getStringProp(PropertyFtChars);

	FLOAT_TYPE fSpaceWidth = pItem->getNumericEnumPropValue(PropertyFtCharWidths, strchr(pcsAllChars, SPACE_WIDTH_REFERENCE_LETTER) - pcsAllChars)*fScale;
	FLOAT_TYPE fLineUVWidth = pItem->getNumProp(PropertyFtHeightUV);
	FLOAT_TYPE fLinePixHeight = pItem->getNumProp(PropertyFtHeight)*fScale;
	FLOAT_TYPE fKerning = pItem->getNumProp(PropertyKerning)*fScale;
	fKerning += fExtraKerning*fScale;

	// Now, go letter by letter, computing the right coordinates on the spot
	int iCurr, iNum = strlen(pcsText);

	const char* pcsFontAnim = pItem->getStringProp(PropertyImage);
	CachedSequence* pFontSeq = pDrawingCache->getCachedSequence(pcsFontAnim, NULL);
	int iFontFileW, iFontFileH;
	pDrawingCache->getImageSize(pcsFontAnim, iFontFileW, iFontFileH);

	FLOAT_TYPE fSingleUPixel = 0.0;
	if(iFontFileW > 0)
		fSingleUPixel = EXTRA_FONT_PIXEL_PADDING/(FLOAT_TYPE)iFontFileW;

	rExtraInfo.myPolyColor = scolText;
	rExtraInfo.myH = fLinePixHeight;

	bool bIsInHighlightPart = false;
	SUVSet rUVSet;
	FLOAT_TYPE iX = fX + svAlignmentOffsets.x, iY = fY + fLinePixHeight/2 + svAlignmentOffsets.y;
	int iCurrLineCount = 0;
	char cChar;
	int iCharIdx;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		cChar = pcsText[iCurr];

		// Find it in the string
		iCharIdx = 5;

		// Get all the metrics, print it:
		fCharWidth = pItem->getNumericEnumPropValue(PropertyFtCharWidths, iCharIdx)*fScale;
		fCharUVWidth = pItem->getNumericEnumPropValue(PropertyFtCharWidthsUV, iCharIdx);

		iX += fCharWidth/2.0;

		rUVSet.myStartX = pItem->getNumericEnumPropValue(PropertyFtCharXs, iCharIdx) - fSingleUPixel;
		rUVSet.myStartY = pItem->getNumericEnumPropValue(PropertyFtCharYs, iCharIdx);
		rUVSet.myEndX = rUVSet.myStartX + fCharUVWidth + fSingleUPixel*2.0;
		rUVSet.myEndY = rUVSet.myStartY + fLineUVWidth;
		rUVSet.myApplyToSize = true;

		rExtraInfo.myW = fCharWidth + (int)(EXTRA_FONT_PIXEL_PADDING*2.0);

		if(!svStringMeasurementsOut)
		{
			pFontSeq->addSprite(iX, iY,
				scolText.alpha, // Alpha
				0.0, fScale, fScale,
				1.0, //  anim progress
				true, true, &rUVSet, false, &rExtraInfo);
		}
		iX += fCharWidth/2.0 + fKerning;
	}
}
/*****************************************************************************/
void RenderUtils::setMaterial(SColor& scolDiffuse, SColor& scolAmbient, SColor& scolSpecular, FLOAT_TYPE fSpecularPower)
{
	_ASSERT(fSpecularPower > 0.0);

#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
#ifdef USE_OPENGL2
	RenderStateManager::getInstance()->setAmbientMaterial(scolAmbient);
	RenderStateManager::getInstance()->setDiffuseMaterial(scolDiffuse);
	RenderStateManager::getInstance()->setSpecularMaterial(scolSpecular, fSpecularPower);
#else
	float colArray[4] = { 1,1,1, scolDiffuse.alpha };

	colArray[0] = scolDiffuse.r;
	colArray[1] = scolDiffuse.g;
	colArray[2] = scolDiffuse.b;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, colArray);

	colArray[0] = scolAmbient.r;
	colArray[1] = scolAmbient.g;
	colArray[2] = scolAmbient.b;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, colArray);

	colArray[0] = scolSpecular.r;
	colArray[1] = scolSpecular.g;
	colArray[2] = scolSpecular.b;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, colArray);

	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, fSpecularPower);
#endif
#endif
}
/*****************************************************************************/
ShaderSetType RenderUtils::setNormalMap(TX_MAN_RETURN_TYPE pTexture, SVertex3D* pData)
{
#ifdef USE_OPENGL2
	if(pTexture == 0)
		return ShaderSetLastPlaceholder;

	RenderStateManager *rStateManager = RenderStateManager::getInstance();
	ShaderSetType eOld = rStateManager->setShaderType(ShaderSetLightedNormalMapping);
	SGlobalShaderProgramInfo& rInfo = rStateManager->getCurrProgramGlobalInfo();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pTexture);
	glUniform1i(rInfo.myTexUnit1Location, 1);

	int iStrideSize = sizeof(SVertex3D);
	glVertexAttribPointer(rInfo.myTangentLocation, 3, GL_FLOAT, GL_FALSE, iStrideSize, &pData->tangent_x);
	glVertexAttribPointer(rInfo.myBitangentLocation, 3, GL_FLOAT, GL_FALSE, iStrideSize, &pData->bitangent_x);

	// 	glEnableVertexAttribArray(rInfo.myTangentLocation);
	// 	glEnableVertexAttribArray(rInfo.myBitangentLocation);

	//	glUniform1i(rInfo.myNormalMappingType, (int)NormalMapNormalMapping);

	return eOld;
#else
	return ShaderSetLastPlaceholder;
#endif
}
/*****************************************************************************/
void RenderUtils::resetNormalMap(ShaderSetType eOldType)
{
	_ASSERT(eOldType != ShaderSetLastPlaceholder);
	RenderStateManager *rStateManager = RenderStateManager::getInstance();
	rStateManager->setShaderType(eOldType);
	//	SGlobalShaderProgramInfo& rInfo = rStateManager->getCurrProgramGlobalInfo();

	// 	glDisableVertexAttribArray(rInfo.myTangentLocation);
	// 	glDisableVertexAttribArray(rInfo.myBitangentLocation);

	//	glUniform1i(rInfo.myNormalMappingType, (int)NormalMapNone);


}
/*****************************************************************************/
void RenderUtils::setSpecularMap(TX_MAN_RETURN_TYPE pTexture)
{
	RenderStateManager *rStateManager = RenderStateManager::getInstance();
	SGlobalShaderProgramInfo& rInfo = rStateManager->getCurrProgramGlobalInfo();
#ifdef USE_OPENGL2
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, pTexture);
	glUniform1i(rInfo.mySpecularTextureUnitLocation, 2);

	//glUniform1i(rInfo.myUseSpecularMap, true);
#endif
}
/*****************************************************************************/
void RenderUtils::resetSpecularMap()
{
	RenderStateManager *rStateManager = RenderStateManager::getInstance();
	SGlobalShaderProgramInfo& rInfo = rStateManager->getCurrProgramGlobalInfo();
#ifdef USE_OPENGL2
	//glUniform1i(rInfo.myUseSpecularMap, false);
#endif
}
#endif
/*****************************************************************************/
void RenderUtils::fillRectangle(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, const SColor& scol, bool bChangeState)
{
	if(bChangeState)
		GraphicsUtils::enableFlatDrawingState(-1.0);

	int iColOffset = 0;
	int iOffset = 0;
	int iCurrIdx = 0;

	for(iCurrIdx = 0; iCurrIdx < NUM_SPRITE_VERTS; iCurrIdx++)
	{
		g_finalSpriteVertices[iOffset] = g_refSpriteVertices[iOffset] * (FLOAT_TYPE)fW + fX + fW/2.0;
		g_finalSpriteVertices[iOffset + 1] = g_refSpriteVertices[iOffset + 1] * (FLOAT_TYPE)fH + fY + fH/2.0;

#ifdef USE_OPENGL2
		g_nonReusableSpriteColors[iColOffset + 0] = scol.r*scol.alpha;
		g_nonReusableSpriteColors[iColOffset + 1] = scol.g*scol.alpha;
		g_nonReusableSpriteColors[iColOffset + 2] = scol.b*scol.alpha;
		g_nonReusableSpriteColors[iColOffset + 3] = scol.alpha;
#else
		g_nonReusableSpriteColors[iColOffset + 0] = (unsigned char)(scol.r*scol.alpha*255.0);
		g_nonReusableSpriteColors[iColOffset + 1] = (unsigned char)(scol.g*scol.alpha*255.0);
		g_nonReusableSpriteColors[iColOffset + 2] = (unsigned char)(scol.b*scol.alpha*255.0);
		g_nonReusableSpriteColors[iColOffset + 3] = (unsigned char)(scol.alpha*255.0);
#endif

#ifdef DIRECTX_PIPELINE
		g_nonReusableSpriteFloatColors[iColOffset + 0] = (GLfloat)g_nonReusableSpriteColors[iColOffset + 0]/255.0;
		g_nonReusableSpriteFloatColors[iColOffset + 1] = (GLfloat)g_nonReusableSpriteColors[iColOffset + 1]/255.0;
		g_nonReusableSpriteFloatColors[iColOffset + 2] = (GLfloat)g_nonReusableSpriteColors[iColOffset + 2]/255.0;
		g_nonReusableSpriteFloatColors[iColOffset + 3] = (GLfloat)g_nonReusableSpriteColors[iColOffset + 3]/255.0;
#endif

		iOffset += g_iNumVertexComponents2D;
		iColOffset += 4;
	}

#ifdef DIRECTX_PIPELINE
	if(!g_pRectangeBuffer)
		g_pRectangeBuffer = new AdhocDxBuffer;
	g_pRectangeBuffer->setFrom2DArrays(g_finalSpriteVertices, NULL, g_nonReusableSpriteFloatColors, 4);

	g_pDxRenderer->getD3dContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT stride = sizeof(BasicVertex);
	UINT offset = 0;
	g_pDxRenderer->getD3dContext()->IASetVertexBuffers(0, 1, g_pRectangeBuffer->getVertexBuffer(), &stride, &offset);
	g_pDxRenderer->getD3dContext()->IASetIndexBuffer(g_pRectangeBuffer->getIndexBuffer(), INDEX_BUFFER_FORMAT,	0);
	g_pDxRenderer->getD3dContext()->DrawIndexed(4, 0, 0);

	g_pDxRenderer->getD3dContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


#else

#ifdef USE_OPENGL2
	RenderUtils::drawBuffer(g_finalSpriteVertices, NULL, g_nonReusableSpriteColors, 4, GL_TRIANGLE_STRIP);
#else
	OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, 0, g_nonReusableSpriteColors);
	OpenGLStateCleaner::doVertexPointerCall(2, GL_FLOAT, 0, g_finalSpriteVertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif

#endif

	if(bChangeState)
		GraphicsUtils::disableFlatDrawingState(-1.0);
}
/*****************************************************************************/
void RenderUtils::fillRectangleAtDepth(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, SColor& scol, FLOAT_TYPE fDepth, bool bChangeState)
{
	if(bChangeState)
		GraphicsUtils::enableFlatDrawingState(-1.0);

	int iColOffset = 0;
	int iOffset = 0;
	int iCurrIdx = 0;

	for(iCurrIdx = 0; iCurrIdx < NUM_SPRITE_VERTS; iCurrIdx++)
	{
		g_finalSpriteVertices3D[iOffset] = g_refSpriteVertices3D[iOffset] * (FLOAT_TYPE)fW + fX + fW/2.0;
		g_finalSpriteVertices3D[iOffset + 1] = g_refSpriteVertices3D[iOffset + 1] * (FLOAT_TYPE)fH + fY + fH/2.0;
		g_finalSpriteVertices3D[iOffset + 2] = fDepth;

		g_nonReusableSpriteColors[iColOffset + 0] = (unsigned char)(scol.r*scol.alpha*255.0);
		g_nonReusableSpriteColors[iColOffset + 1] = (unsigned char)(scol.g*scol.alpha*255.0);
		g_nonReusableSpriteColors[iColOffset + 2] = (unsigned char)(scol.b*scol.alpha*255.0);
		g_nonReusableSpriteColors[iColOffset + 3] = (unsigned char)(scol.alpha*255.0);

#ifdef DIRECTX_PIPELINE
		g_nonReusableSpriteFloatColors[iColOffset + 0] = (GLfloat)g_nonReusableSpriteColors[iColOffset + 0]/255.0;
		g_nonReusableSpriteFloatColors[iColOffset + 1] = (GLfloat)g_nonReusableSpriteColors[iColOffset + 1]/255.0;
		g_nonReusableSpriteFloatColors[iColOffset + 2] = (GLfloat)g_nonReusableSpriteColors[iColOffset + 2]/255.0;
		g_nonReusableSpriteFloatColors[iColOffset + 3] = (GLfloat)g_nonReusableSpriteColors[iColOffset + 3]/255.0;
#endif

		iOffset += g_iNumVertexComponents3D;
		iColOffset += 4;
	}

#ifdef DIRECTX_PIPELINE
	if(!g_pRectangeBuffer)
		g_pRectangeBuffer = new AdhocDxBuffer;
	g_pRectangeBuffer->setFrom3DArrays(g_finalSpriteVertices3D, NULL, g_nonReusableSpriteFloatColors, 4);

	g_pDxRenderer->getD3dContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT stride = sizeof(BasicVertex);
	UINT offset = 0;
	g_pDxRenderer->getD3dContext()->IASetVertexBuffers(0, 1, g_pRectangeBuffer->getVertexBuffer(), &stride, &offset);
	g_pDxRenderer->getD3dContext()->IASetIndexBuffer(g_pRectangeBuffer->getIndexBuffer(), INDEX_BUFFER_FORMAT,	0);
	g_pDxRenderer->getD3dContext()->DrawIndexed(4, 0, 0);

	g_pDxRenderer->getD3dContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#else

#ifdef USE_OPENGL2
	RenderUtils::drawBuffer(g_finalSpriteVertices3D, NULL, g_nonReusableSpriteColors, 4, GL_TRIANGLE_STRIP);
#else
	OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, 0, g_nonReusableSpriteColors);
	OpenGLStateCleaner::doVertexPointerCall(3, GL_FLOAT, 0, g_finalSpriteVertices3D);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif

#endif
	if(bChangeState)
		GraphicsUtils::disableFlatDrawingState(-1.0);

}
/*****************************************************************************/
void RenderUtils::fillRectangleCustomColors(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, const SColor *pCornerColors, bool bChangeState)
{
	if(bChangeState)
		GraphicsUtils::enableFlatDrawingState(-1.0);

	int iColOffset = 0;
	int iOffset = 0;
	int iCurrIdx = 0;

	for(iCurrIdx = 0; iCurrIdx < NUM_SPRITE_VERTS; iCurrIdx++)
	{
		g_finalSpriteVertices[iOffset] = g_refSpriteVertices[iOffset] * (FLOAT_TYPE)fW + fX + fW/2.0;
		g_finalSpriteVertices[iOffset + 1] = g_refSpriteVertices[iOffset + 1] * (FLOAT_TYPE)fH + fY + fH/2.0;

#ifdef USE_OPENGL2
		g_nonReusableSpriteColors[iColOffset + 0] = (pCornerColors[iCurrIdx].r*pCornerColors[iCurrIdx].alpha);
		g_nonReusableSpriteColors[iColOffset + 1] = (pCornerColors[iCurrIdx].g*pCornerColors[iCurrIdx].alpha);
		g_nonReusableSpriteColors[iColOffset + 2] = (pCornerColors[iCurrIdx].b*pCornerColors[iCurrIdx].alpha);
		g_nonReusableSpriteColors[iColOffset + 3] = (pCornerColors[iCurrIdx].alpha);
#else
		g_nonReusableSpriteColors[iColOffset + 0] = (unsigned char)(pCornerColors[iCurrIdx].r*pCornerColors[iCurrIdx].alpha*255.0);
		g_nonReusableSpriteColors[iColOffset + 1] = (unsigned char)(pCornerColors[iCurrIdx].g*pCornerColors[iCurrIdx].alpha*255.0);
		g_nonReusableSpriteColors[iColOffset + 2] = (unsigned char)(pCornerColors[iCurrIdx].b*pCornerColors[iCurrIdx].alpha*255.0);
		g_nonReusableSpriteColors[iColOffset + 3] = (unsigned char)(pCornerColors[iCurrIdx].alpha*255.0);
#endif

#ifdef DIRECTX_PIPELINE
		g_nonReusableSpriteFloatColors[iColOffset + 0] = (GLfloat)g_nonReusableSpriteColors[iColOffset + 0]/255.0;
		g_nonReusableSpriteFloatColors[iColOffset + 1] = (GLfloat)g_nonReusableSpriteColors[iColOffset + 1]/255.0;
		g_nonReusableSpriteFloatColors[iColOffset + 2] = (GLfloat)g_nonReusableSpriteColors[iColOffset + 2]/255.0;
		g_nonReusableSpriteFloatColors[iColOffset + 3] = (GLfloat)g_nonReusableSpriteColors[iColOffset + 3]/255.0;
#endif


		iOffset += g_iNumVertexComponents2D;
		iColOffset += 4;
	}

#ifdef DIRECTX_PIPELINE
	if(!g_pRectangeBuffer)
		g_pRectangeBuffer = new AdhocDxBuffer;
	g_pRectangeBuffer->setFrom2DArrays(g_finalSpriteVertices, NULL, g_nonReusableSpriteFloatColors, 4);

	g_pDxRenderer->getD3dContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT stride = sizeof(BasicVertex);
	UINT offset = 0;
	g_pDxRenderer->getD3dContext()->IASetVertexBuffers(0, 1, g_pRectangeBuffer->getVertexBuffer(), &stride, &offset);
	g_pDxRenderer->getD3dContext()->IASetIndexBuffer(g_pRectangeBuffer->getIndexBuffer(), INDEX_BUFFER_FORMAT, 0);
	g_pDxRenderer->getD3dContext()->DrawIndexed(4, 0, 0);

	g_pDxRenderer->getD3dContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#else

#ifdef USE_OPENGL2
	RenderUtils::drawBuffer(g_finalSpriteVertices, NULL, g_nonReusableSpriteColors, 4, GL_TRIANGLE_STRIP);
#else
	OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, 0, g_nonReusableSpriteColors);
	OpenGLStateCleaner::doVertexPointerCall(2, GL_FLOAT, 0, g_finalSpriteVertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif

#endif

	if(bChangeState)
		GraphicsUtils::disableFlatDrawingState(-1.0);

}
/*****************************************************************************/
void RenderUtils::drawVBO(GLuint iBufferIndex, int iNumVertComponents, int iNormalsByteOffset, int iUVsByteOffset, int iNumVertsToRender, int iStrideSize)
{
#ifdef ENABLE_VBOS

#ifdef DIRECTX_PIPELINE
#else
#ifdef USE_OPENGL2
	RenderStateManager *rStateManager = RenderStateManager::getInstance();
	SGlobalShaderProgramInfo& rInfo = rStateManager->getCurrProgramGlobalInfo();
	rStateManager->updateGLMatrices();
	rStateManager->updateGLMaterials();

	glBindBuffer(GL_ARRAY_BUFFER, iBufferIndex);
	glVertexAttribPointer(rInfo.myVertexLocation, iNumVertComponents, GL_FLOAT, GL_FALSE, iStrideSize, (void*)0);
	glVertexAttribPointer(rInfo.myNormalLocation, 3, GL_FLOAT, GL_TRUE, iStrideSize, (void*)iNormalsByteOffset);
	glVertexAttribPointer(rInfo.myTexCoord0Location, 2, GL_FLOAT, GL_FALSE, iStrideSize, (void*)iUVsByteOffset);
	glDisableVertexAttribArray(rInfo.myDummyVertexColorLocation);

#else

	glBindBuffer(GL_ARRAY_BUFFER, iBufferIndex);
	glEnableClientState(GL_VERTEX_ARRAY);
	OpenGLStateCleaner::doVertexPointerCall(iNumVertComponents, GL_FLOAT, iStrideSize, (void*)0);   //The starting point of the VBO
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, iStrideSize, (void*)iNormalsByteOffset);   //The starting point of normals
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	OpenGLStateCleaner::doTexturePointerCall(2, GL_FLOAT, iStrideSize, (void*)iUVsByteOffset);   //The starting point of texcoords
#endif

	glDrawArrays(GL_TRIANGLES, 0, iNumVertsToRender);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
#else
	_ASSERT(0);
#endif
}
/*****************************************************************************/
#if defined(_DEBUG) && defined(USE_OPENGL2) && defined(MAC_BUILD)

// EXPERIM CODE
void RenderUtils::drawBufferInternal(const float *pVertexData, int iNumVertComponents, const float* pTextureData, const GL_COLOR_TYPE* pColorData, const float* pNormalData, int iNumVertsToRender, int iOptStartVertex, GLenum ePrimitiveType, int iStrideSize)
{
#ifdef DIRECTX_PIPELINE
#else
#ifdef USE_OPENGL2

	RenderStateManager *rStateManager = RenderStateManager::getInstance();
	ShaderSetType eOld = ShaderSetLastPlaceholder;
	if(!pTextureData)
	{
		_ASSERT(!pNormalData);
		eOld = rStateManager->setShaderType(ShaderSetFlatColor);
	}

	SGlobalShaderProgramInfo& rInfo = rStateManager->getCurrProgramGlobalInfo();

	_ASSERT(iNumVertComponents == 4);

	rStateManager->updateGLMatrices();
	rStateManager->updateGLMaterials();



	checkGlError();

	GLuint buffer;
	GLuint vao;
	//static bool bTemp = false;
	// if(!bTemp)
	{
		glGenVertexArrays(1, &vao);
		checkGlError();
		glBindVertexArray(vao);
		checkGlError();


		glGenBuffers(1, &buffer);
		checkGlError();
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		checkGlError();
		glBufferData(GL_ARRAY_BUFFER, sizeof(pVertexData),pVertexData, GL_STATIC_DRAW);
		checkGlError();

		//bTemp = true;
	}

	// Setup vertices

	//GLuint array;
	//glGenVertexArrays(1, &array);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	checkGlError();
	glVertexAttribPointer(rInfo.myVertexLocation, iNumVertComponents, GL_FLOAT, GL_FALSE, iStrideSize, 0);

	//glVertexAttribPointer(rInfo.myVertexLocation, iNumVertComponents, GL_FLOAT, GL_FALSE, iStrideSize, pVertexData);
	checkGlError();
	glEnableVertexAttribArray(rInfo.myVertexLocation);
	checkGlError();


	// Setup normals
	if(pNormalData)
		glVertexAttribPointer(rInfo.myNormalLocation, 3, GL_FLOAT, GL_TRUE, iStrideSize, pNormalData);

	// Setup UVs
	if(pTextureData)
		glVertexAttribPointer(rInfo.myTexCoord0Location, 2, GL_FLOAT, GL_FALSE, iStrideSize, pTextureData);

	if(pColorData)
	{
		glDisableVertexAttribArray(rInfo.myDummyVertexColorLocation);
		//	glVertexAttribPointer(rInfo.myDummyVertexColorLocation, 4, GL_FLOAT, GL_FALSE, iStrideSize, pColorData);
		//  checkGlError();
		//glEnableVertexAttribArray(rInfo.myDummyVertexColorLocation);
		// checkGlError();
	}
	else
		glDisableVertexAttribArray(rInfo.myDummyVertexColorLocation);

#else
	OpenGLStateCleaner::doVertexPointerCall(iNumVertComponents, GL_FLOAT, iStrideSize, pVertexData);
	if(pNormalData)
		glNormalPointer(GL_FLOAT, iStrideSize, pNormalData);
	if(pColorData)
		OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, iStrideSize, pColorData);
	if(pTextureData)
		OpenGLStateCleaner::doTexturePointerCall( 2, GL_FLOAT, iStrideSize, pTextureData);
#endif

	glDrawArrays( ePrimitiveType, iOptStartVertex, iNumVertsToRender);
	checkGlError();

	glDeleteBuffers(1, &buffer);
	checkGlError();
	glDeleteVertexArrays(1, &vao);
	checkGlError();

#ifdef USE_OPENGL2
	if(eOld != ShaderSetLastPlaceholder)
		rStateManager->setShaderType(eOld);
#endif
#endif
}

#else

void RenderUtils::drawBufferInternal(const float *pVertexData, int iNumVertComponents, const float* pTextureData, const GL_COLOR_TYPE* pColorData, const float* pNormalData, int iNumVertsToRender, int iOptStartVertex, GLenum ePrimitiveType, int iStrideSize)
{
#ifdef DIRECTX_PIPELINE
#else
#ifdef USE_OPENGL2

	RenderStateManager *rStateManager = RenderStateManager::getInstance();
	ShaderSetType eOld = ShaderSetLastPlaceholder;
	if(!pTextureData)
	{
		_ASSERT(!pNormalData);
		eOld = rStateManager->setShaderType(ShaderSetFlatColor);
	}

	SGlobalShaderProgramInfo& rInfo = rStateManager->getCurrProgramGlobalInfo();

	_ASSERT(iNumVertComponents == 4);

	rStateManager->updateGLMatrices();
	rStateManager->updateGLMaterials();

	// Setup vertices
	glVertexAttribPointer(rInfo.myVertexLocation, iNumVertComponents, GL_FLOAT, GL_FALSE, iStrideSize, pVertexData);
	checkGlError();

	// Setup normals
	if(pNormalData)
		glVertexAttribPointer(rInfo.myNormalLocation, 3, GL_FLOAT, GL_TRUE, iStrideSize, pNormalData);

	// Setup UVs
	if(pTextureData)
		glVertexAttribPointer(rInfo.myTexCoord0Location, 2, GL_FLOAT, GL_FALSE, iStrideSize, pTextureData);

	if(pColorData)
	{
		glVertexAttribPointer(rInfo.myDummyVertexColorLocation, 4, GL_FLOAT, GL_FALSE, iStrideSize, pColorData);
		glEnableVertexAttribArray(rInfo.myDummyVertexColorLocation);
	}
	else
		glDisableVertexAttribArray(rInfo.myDummyVertexColorLocation);

#else
	OpenGLStateCleaner::doVertexPointerCall(iNumVertComponents, GL_FLOAT, iStrideSize, pVertexData);
	if(pNormalData)
		glNormalPointer(GL_FLOAT, iStrideSize, pNormalData);
	if(pColorData)
		OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, iStrideSize, pColorData);
	if(pTextureData)
		OpenGLStateCleaner::doTexturePointerCall( 2, GL_FLOAT, iStrideSize, pTextureData);
#endif

	glDrawArrays( ePrimitiveType, iOptStartVertex, iNumVertsToRender);

#ifdef USE_OPENGL2
	if(eOld != ShaderSetLastPlaceholder)
		rStateManager->setShaderType(eOld);
#endif
#endif
}
#endif
/*****************************************************************************/
void RenderUtils::drawBuffer(const float *pVertexData, const float* pTextureData, const float* pColorData, int iNumVertsToRender, GLenum ePrimitiveType)
{
#ifdef DIRECTX_PIPELINE
#else
#ifdef USE_OPENGL2
	drawBufferInternal(pVertexData, 4, pTextureData, pColorData, NULL, iNumVertsToRender, 0, ePrimitiveType, 0);
#else
	OpenGLStateCleaner::doVertexPointerCall(2, GL_FLOAT, 0, pVertexData);
	OpenGLStateCleaner::doColorPointerCall(4, GL_FLOAT, 0, pColorData);
	if(pTextureData)
		OpenGLStateCleaner::doTexturePointerCall( 2, GL_FLOAT, 0, pTextureData);
	glDrawArrays(ePrimitiveType, 0, iNumVertsToRender);
#endif
#endif
}
/*****************************************************************************/
#ifndef USE_OPENGL2
// No unsigned color in GL2 mode.
void RenderUtils::drawBuffer(float *pVertexData, int iNumVertComponents, unsigned char* pColorData, int iNumVertsToRender, GLenum ePrimitiveType)
{
#ifdef DIRECTX_PIPELINE
#else
	OpenGLStateCleaner::doVertexPointerCall(iNumVertComponents, GL_FLOAT, 0, pVertexData);
	OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, 0, pColorData);
	glDrawArrays(ePrimitiveType, 0, iNumVertsToRender);
#endif
}
#endif
/*****************************************************************************/
void RenderUtils::drawBitmap(TextureManager* pTextureManager, const char* pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha)
{
	RenderUtils::drawBitmap(pTextureManager, pcsType, fX, fY, fAlpha, fDegAngle, fScale, fScale, fAnimProgress, bPremultiplyAlpha, NULL);
}
/*****************************************************************************/
void RenderUtils::drawBitmap(TextureManager* pTextureManager, const char* pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, SUVSet* pUVsIn, bool bIsProgressAFrameNum)
{
	// Find out the texture
	if(!pTextureManager || fAlpha < FLOAT_EPSILON)
		return;

	int iW, iH;
	TX_MAN_RETURN_TYPE iIndex;
	BlendModeType eBlendMode;

	SUVSet uvsOut;
	iIndex = pTextureManager->getTextureIndexFromProgress(pcsType, fAnimProgress, iW, iH, uvsOut, eBlendMode, NULL);
	if(pUVsIn)
		uvsOut = *pUVsIn;

	FLOAT_TYPE fW, fH;
	/*
	if(fScale == 1.0)
	{
	fW = iW;
	fH = iH;
	}
	else
	*/
	{
		fW = (FLOAT_TYPE)iW*fScaleX;
		fH = (FLOAT_TYPE)iH*fScaleY;
	}
	GraphicsUtils::drawImage(pTextureManager->getParentWindow(), iIndex, fX, fY, fW, fH, fAlpha, fDegAngle, bPremultiplyAlpha, uvsOut, eBlendMode);
}
/*****************************************************************************/
void RenderUtils::drawBitmapFromFrame(Window* pWindow, const char* pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScale, int iFrameNum, bool bPremultiplyAlpha, TextureManager* pTextureManager)
{
	if(fAlpha < FLOAT_EPSILON)
		return;

	// Find out the texture
	if(!pTextureManager)
		return;

	int iW, iH;
	TX_MAN_RETURN_TYPE iIndex;
	SUVSet uvsOut;
	BlendModeType eBlendMode;
	iIndex = pTextureManager->getTextureIndex(pcsType, iFrameNum, iW, iH, uvsOut, eBlendMode, NULL);
	FLOAT_TYPE fW, fH;
	if(fScale == 1.0)
	{
		fW = iW;
		fH = iH;
	}
	else
	{
		fW = (FLOAT_TYPE)iW*fScale;
		fH = (FLOAT_TYPE)iH*fScale;
	}
	GraphicsUtils::drawImage(pWindow, iIndex, fX, fY, fW, fH, fAlpha, fDegAngle, bPremultiplyAlpha, uvsOut, eBlendMode);

}
/*****************************************************************************/
void RenderUtils::drawBitmapNonUniScale(const char* pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, TextureManager* pTextureManager)
{
	// Find out the texture
	if(!pTextureManager || fAlpha < FLOAT_EPSILON)
		return;

	int iW, iH;
	TX_MAN_RETURN_TYPE iIndex;
	//	iIndex = pTextureManager->getTextureIndex(pcsType, 0, iW, iH);
	SUVSet uvsOut;
	BlendModeType eBlendMode;
	iIndex = pTextureManager->getTextureIndexFromProgress(pcsType, fAnimProgress, iW, iH, uvsOut, eBlendMode, NULL);
	FLOAT_TYPE fW, fH;
	fW = (FLOAT_TYPE)iW*fScaleX;
	fH = (FLOAT_TYPE)iH*fScaleY;
	GraphicsUtils::drawImage(pTextureManager->getParentWindow(), iIndex, fX + fW/2.0, fY, fW, fH, fAlpha, fDegAngle, bPremultiplyAlpha, uvsOut, eBlendMode);
}
/*****************************************************************************/
GLfloat *g_currBezierPoints = NULL;
GLfloat *g_currBezierColors = NULL;
static int g_iCurrBezierPointCount = 0;
void RenderUtils::drawLines(Window* pWindow, TPointVector& rPoints, const SColor& scol, FLOAT_TYPE fLineWidth, bool bChangeState)
{
	//glDisable(GL_TEXTURE_2D);
	//glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	BlendModeType eOldMode = BlendModeLastPlaceholder;
	if(bChangeState)
	{
		eOldMode = pWindow->setBlendMode(BlendModeText);
		GraphicsUtils::enableFlatDrawingState(-1.0);
	}

#ifndef DIRECTX_PIPELINE
	OpenGLStateCleaner::doLineWidth(fLineWidth);
	if(fLineWidth == 1.0)
	{
#ifndef USE_OPENGL2
		glEnable(GL_LINE_SMOOTH);
#endif
	}
#endif

	// Allocate enough storage
	int iSingleStepCntr, iCurrColor, iCurrPoint, iNumPoints = rPoints.size();
	if(iNumPoints > g_iCurrBezierPointCount)
	{
		// Reallocate the buffer
		if(g_currBezierPoints)
			delete[] g_currBezierPoints;
		g_currBezierPoints = new GLfloat[iNumPoints*g_iNumVertexComponents2D];

		if(g_currBezierColors)
			delete[] g_currBezierColors;
		g_currBezierColors = new GLfloat[iNumPoints*4];

		g_iCurrBezierPointCount = iNumPoints;
	}

	// Now, fill up the array
	for(iCurrPoint = 0, iSingleStepCntr = 0, iCurrColor = 0; iCurrPoint < iNumPoints*g_iNumVertexComponents2D; iCurrPoint += g_iNumVertexComponents2D, iCurrColor += 4, iSingleStepCntr++)
	{
		g_currBezierPoints[iCurrPoint + 0] = rPoints[iSingleStepCntr].x;
		g_currBezierPoints[iCurrPoint + 1] = rPoints[iSingleStepCntr].y;
#ifdef USE_OPENGL2
		g_currBezierPoints[iCurrPoint + 2] = 0;
		g_currBezierPoints[iCurrPoint + 3] = 1.0;
#endif

		g_currBezierColors[iCurrColor + 0] = scol.r*scol.alpha;
		g_currBezierColors[iCurrColor + 1] = scol.g*scol.alpha;
		g_currBezierColors[iCurrColor + 2] = scol.b*scol.alpha;
		g_currBezierColors[iCurrColor + 3] = scol.alpha;
	}

#ifdef DIRECTX_PIPELINE
	if(!g_pRectangeBuffer)
		g_pRectangeBuffer = new AdhocDxBuffer;
	g_pRectangeBuffer->setFrom2DArrays(g_currBezierPoints, NULL, g_currBezierColors, iNumPoints);

	g_pDxRenderer->getD3dContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	UINT stride = sizeof(BasicVertex);
	UINT offset = 0;
	g_pDxRenderer->getD3dContext()->IASetVertexBuffers(0, 1, g_pRectangeBuffer->getVertexBuffer(), &stride, &offset);
	g_pDxRenderer->getD3dContext()->IASetIndexBuffer(g_pRectangeBuffer->getIndexBuffer(), INDEX_BUFFER_FORMAT,	0);
	g_pDxRenderer->getD3dContext()->DrawIndexed(iNumPoints, 0, 0);

	g_pDxRenderer->getD3dContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#else

#ifdef USE_OPENGL2
	RenderUtils::drawBuffer(g_currBezierPoints, NULL, g_currBezierColors, iNumPoints, GL_LINE_STRIP);
#else
	OpenGLStateCleaner::doVertexPointerCall(2, GL_FLOAT, 0, g_currBezierPoints);
	OpenGLStateCleaner::doColorPointerCall(4, GL_FLOAT, 0, g_currBezierColors);
	glDrawArrays(GL_LINE_STRIP, 0, iNumPoints);
#endif

	if(fLineWidth == 1.0)
	{
#ifndef USE_OPENGL2
		glDisable(GL_LINE_SMOOTH);
#endif
	}
#endif

	if(bChangeState)
	{
		GraphicsUtils::disableFlatDrawingState(-1.0);
		pWindow->setBlendMode(eOldMode);
	}
	//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//	glEnable(GL_TEXTURE_2D);


}
/*****************************************************************************/
void RenderUtils::drawLines(Window* pWindow, SGenData& rDrawData, FLOAT_TYPE fLineThickness, bool bChangeState)
{
#ifdef DIRECTX_PIPELINE
	// Obsolete, not implemented
	_ASSERT(0);
#else

	if(rDrawData.myNumLines <= 0)
		return;

	BlendModeType eOldMode = BlendModeLastPlaceholder;

	if(bChangeState)
	{
		eOldMode = pWindow->setBlendMode(BlendModeText);
		GraphicsUtils::enableFlatDrawingState(-1.0);
	}

	OpenGLStateCleaner::doLineWidth(fLineThickness);
	if(fLineThickness == 1.0)
	{
#ifndef USE_OPENGL2
		glEnable(GL_LINE_SMOOTH);
#endif
	}

	//	glVertexPointer(2, GL_FLOAT, 0, rDrawData.myVertexArray);
	//	glColorPointer(4, GL_FLOAT, 0, rDrawData.myColorArray);

#ifdef USE_OPENGL2
	RenderUtils::drawBuffer<SPathVertexInfo>(&rDrawData.myVerts[0].x, SPathVertexInfo::getNumVertexComponents(), NULL, &rDrawData.myVerts[0].r, NULL, rDrawData.myNumLines + 1, 0, GL_LINE_STRIP);
#else
	OpenGLStateCleaner::doVertexPointerCall(2, GL_SHORT, sizeof(SPathVertexInfo), &rDrawData.myVerts[0].x);
	OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, sizeof(SPathVertexInfo), &rDrawData.myVerts[0].r);
	glDrawArrays(GL_LINE_STRIP, 0, rDrawData.myNumLines + 1);
#endif

	if(fLineThickness == 1.0)
	{
#ifndef USE_OPENGL2
		glDisable(GL_LINE_SMOOTH);
#endif
	}

	if(bChangeState)
	{
		GraphicsUtils::disableFlatDrawingState(-1.0);
		pWindow->setBlendMode(eOldMode);
	}
#endif
}
/*****************************************************************************/
#define MAX_CIRCLE_POINTS	128
static GLfloat g_preGenUnitCirclePoints[MAX_CIRCLE_POINTS*2];
static GLfloat g_currCirclePoints[MAX_CIRCLE_POINTS*2];
static int g_iActualCirclePoints = 0;

static GLfloat g_circleSpriteColors[MAX_CIRCLE_POINTS*4];

void RenderUtils::drawEllipse(Window* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRx, FLOAT_TYPE fRy, SColor& scol, FLOAT_TYPE fLineWidth, bool bChangeState)
{
#ifdef DIRECTX_PIPELINE
	// Obsolete, not implemented
	_ASSERT(0);
#else

	BlendModeType eOldMode = BlendModeLastPlaceholder;
	if(bChangeState)
	{
		eOldMode = pWindow->setBlendMode(BlendModeText);
		GraphicsUtils::enableFlatDrawingState(-1.0);
	}


	//	glDisable(GL_TEXTURE_2D);
	///	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(fabs(fLineWidth - 1.0) <= FLOAT_EPSILON)
	{
		OpenGLStateCleaner::doLineWidth(1.0);
#ifndef USE_OPENGL2
		glEnable(GL_LINE_SMOOTH);
#endif
	}
	else
		OpenGLStateCleaner::doLineWidth(fLineWidth);

	static bool bDidGenerateArrays = false;
	if(!bDidGenerateArrays)
	{
		// Generate the circle arrays
		int iDeg, iCntr;
		// FLOAT_TYPE fRadAngle;
		for(iDeg = 0, iCntr = 0; iDeg <= 360; iDeg+= 5, iCntr+=2)
		{
			//fRadAngle = (FLOAT_TYPE)iDeg/180.0*M_PI;
			g_preGenUnitCirclePoints[iCntr + 0] = F_COS_DEG(iDeg);
			g_preGenUnitCirclePoints[iCntr + 1] = F_SIN_DEG(iDeg);
		}

		g_iActualCirclePoints = iCntr/2;
	}

	// Now, go scale into an array
	int iPoint, iColorIdx;
	for(iPoint = 0, iColorIdx = 0; iPoint < g_iActualCirclePoints*2; iPoint+=2, iColorIdx += 4)
	{
		g_currCirclePoints[iPoint + 0] = g_preGenUnitCirclePoints[iPoint]*fRx + fX;
		g_currCirclePoints[iPoint + 1] = g_preGenUnitCirclePoints[iPoint + 1]*fRy + fY;

		g_circleSpriteColors[iColorIdx + 0] = scol.r*scol.alpha;
		g_circleSpriteColors[iColorIdx + 1] = scol.g*scol.alpha;
		g_circleSpriteColors[iColorIdx + 2] = scol.b*scol.alpha;
		g_circleSpriteColors[iColorIdx + 3] = scol.alpha;
	}

#ifdef USE_OPENGL2
	RenderUtils::drawBuffer(g_currCirclePoints, NULL, g_circleSpriteColors, g_iActualCirclePoints, GL_LINE_STRIP);
#else
	OpenGLStateCleaner::doVertexPointerCall(2, GL_FLOAT, 0, g_currCirclePoints);
	OpenGLStateCleaner::doColorPointerCall(4, GL_FLOAT, 0, g_circleSpriteColors);
	glDrawArrays(GL_LINE_STRIP, 0, g_iActualCirclePoints);
#endif
	//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//	glEnable(GL_TEXTURE_2D);

	if(bChangeState)
	{
		GraphicsUtils::disableFlatDrawingState(-1.0);
		pWindow->setBlendMode(eOldMode);
	}
#endif
}
/*****************************************************************************/
TPointVector g_rDrawRectVector;
void RenderUtils::drawRectangle(Window* pWindow, FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fW, FLOAT_TYPE fH, const SColor& scol, FLOAT_TYPE fLineWidth, bool bChangeState)
{
	BlendModeType eOldMode = BlendModeLastPlaceholder;
	if(bChangeState)
	{
		eOldMode = pWindow->setBlendMode(BlendModeText);
		GraphicsUtils::enableFlatDrawingState(-1.0);
	}


	if(g_rDrawRectVector.size() == 0)
		g_rDrawRectVector.resize(5);

	g_rDrawRectVector[0].set(fStartX, fStartY);
	g_rDrawRectVector[1].set(fStartX + fW, fStartY);
	g_rDrawRectVector[2].set(fStartX + fW, fStartY + fH);
	g_rDrawRectVector[3].set(fStartX, fStartY + fH);
	g_rDrawRectVector[4].set(fStartX, fStartY);

	RenderUtils::drawLines(pWindow, g_rDrawRectVector, scol, fLineWidth, false);

	if(bChangeState)
	{
		GraphicsUtils::disableFlatDrawingState(-1.0);
		pWindow->setBlendMode(eOldMode);
	}
}
/*****************************************************************************/
};