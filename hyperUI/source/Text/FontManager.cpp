/*****************************************************************************

Disclaimer: This software is supplied to you by Sad Cat Software
("Sad Cat") in consideration of your agreement to the following terms, and 
your use, installation, modification or redistribution of this Sad Cat software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Sad Cat software.

This software is provided "as is". Sad Cat Software makes no warranties, 
express or implied, including without limitation the implied warranties
of non-infringement, merchantability and fitness for a particular
purpose, regarding Sad Cat's software or its use and operation alone
or in combination with other hardware or software products.

In no event shall Sad Cat Software be liable for any special, indirect,
incidental, or consequential damages (including, but not limited to, 
procurement of substitute goods or services; loss of use, data, or profits;
or business interruption) arising in any way out of the use, reproduction,
modification and/or distribution of Sad Cat's software however caused and
whether under theory of contract, tort (including negligence), strict
liability or otherwise, even if Sad Cat Software has been advised of the
possibility of such damage.

Copyright (C) 2012, Sad Cat Software. All Rights Reserved.

*****************************************************************************/
#include "stdafx.h"

#ifdef DIRECTX_PIPELINE
#include "AdhocDxBuffer.h"
#include "D3DRenderer.h"
extern D3DRenderer* g_pDxRenderer;
AdhocDxBuffer* CachedFontInfo::theGeometryBuffer = NULL;
#endif

#include <ft2build.h>
#include FT_FREETYPE_H


#ifdef USE_FREETYPE_FONTS

#define TEMP_DEFAULT_FONT_SIZE	upToScreen(22.0)

namespace HyperUI
{
bool FontManager::theIsFreelibInited = false;
FT_Library FontManager::theFreetypLib;
/*****************************************************************************/
// FontManager
/*****************************************************************************/
FontManager::FontManager()
{
	if(!theIsFreelibInited)
	{
		if(FT_Init_FreeType(&theFreetypLib)) 
			Logger::log(LogLevelError, "Could not initialize FreeType library\n");
		theIsFreelibInited = true;
	}

/*
	glGenTextures(1, &myMainTexture);
	glBindTexture(GL_TEXTURE_2D, myMainTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
*/
}
/*****************************************************************************/
FontManager::~FontManager()
{
	clear();
//	glDeleteTextures(1, &myMainTexture);
}
/*****************************************************************************/
void FontManager::clear()
{
	TFontInfoHash::iterator fi;
	CachedFontInfo* pFontInfo;
	for(fi = myFonts.begin(); fi != myFonts.end(); fi++)
	{
		pFontInfo = fi.getValue();
		FT_Done_Face(pFontInfo->myFace);
		delete pFontInfo;
	}
	myFonts.clear();
}
/*****************************************************************************/
void FontManager::initFromCollection(ResourceCollection* pCollection)
{
	clear();

	if(!pCollection)
		return;

	ResourceItem* pItem;
	CachedFontInfo* pFontInfo;
	ResourceCollection::Iterator ci;

	STRING_TYPE strFontPathPrefix, strFontPath;
	strFontPathPrefix = ResourceManager::getFullAppPath();
	strFontPathPrefix += ResourceManager::getStandardDataPathPrefix();
	strFontPathPrefix += FONTS_PREFIX_PATH;
	strFontPathPrefix += FOLDER_SEP;

#ifdef USE_UNICODE_STRINGS
	string strConvPath;
#endif

	for(ci = pCollection->itemsBegin(); !ci.isEnd(); ci++)
	{
		pItem = ci.getItem();
		pFontInfo = new CachedFontInfo;

		pFontInfo->myExtraLineSpacing = pItem->getNumProp(PropertyExtraLineSpacing);
		pFontInfo->myKerning = pItem->getNumProp(PropertyKerning);

		// Load the font
		strFontPath = strFontPathPrefix;
		strFontPath += pItem->getStringProp(PropertyFile);
#ifdef USE_UNICODE_STRINGS
		wideStringToString(strFontPath.c_str(), strConvPath);
		if(FT_New_Face(theFreetypLib, strConvPath.c_str(), 0, &pFontInfo->myFace)) 
			Logger::log(LogLevelError, STR_LIT("Could not open font\n"));
#else
		if(FT_New_Face(theFreetypLib, strFontPath.c_str(), 0, &pFontInfo->myFace)) 
			Logger::log(LogLevelError, STR_LIT("Could not open font\n"));
#endif

		pFontInfo->myDefaultSize = pItem->getNumProp(PropertyDefaultSize);

		pFontInfo->myLineHeightMult = (FLOAT_TYPE)pFontInfo->myFace->height/(FLOAT_TYPE)pFontInfo->myFace->units_per_EM;
		pFontInfo->myDescenderPortion = fabs((FLOAT_TYPE)pFontInfo->myFace->descender/(FLOAT_TYPE)pFontInfo->myFace->units_per_EM);

		myFonts.insert(pItem->getStringProp(PropertyId), pFontInfo);
	}
}
/*****************************************************************************/
const CachedFontInfo* FontManager::getFontInfo(const CHAR_TYPE* pcsFontName)
{
#ifdef _DEBUG
	if(IS_STRING_EQUAL(pcsFontName, STR_LIT("ftMain52")))
	{
		int bp = 0;
	}
#endif
	CachedFontInfo** pRes = myFonts.find(pcsFontName);

	if(pRes)
		return *pRes;
	else
		return myFonts.begin().getValue();
}
/*****************************************************************************/
void FontManager::destroyFreelib()
{
	FT_Done_FreeType(theFreetypLib);
}
/*****************************************************************************
void FontManager::bindMainTexture()
{
	glBindTexture(GL_TEXTURE_2D, myMainTexture);
}
/*****************************************************************************/
// FontInfo
/*****************************************************************************/
CachedFontInfo::CachedFontInfo()
{
	myLineHeightMult = 0;
	myKerning = 0;
	myExtraLineSpacing = 0;
}
/*****************************************************************************/
CachedFontInfo::~CachedFontInfo()
{
}
/*****************************************************************************/
FLOAT_TYPE CachedFontInfo::getLineHeight(int iFontSize) const
{
	ensureValidFontSize(iFontSize);
	return myLineHeightMult*(FLOAT_TYPE)iFontSize;
}
/*****************************************************************************/
FLOAT_TYPE CachedFontInfo::getExtraLineSpacing(int iFontSize) const
{
	ensureValidFontSize(iFontSize);
	return myExtraLineSpacing/100.0*(FLOAT_TYPE)iFontSize;
}
/*****************************************************************************/
FLOAT_TYPE CachedFontInfo::getKerning(int iFontSize) const
{
	ensureValidFontSize(iFontSize);
	return myKerning/100.0*(FLOAT_TYPE)iFontSize;
}
/*****************************************************************************/
void CachedFontInfo::getCharDims(UNSIGNED_CHAR_TYPE cChar, int iFontSize, SVector2D& svDimsOut) const
{
	svDimsOut.set(0, 0);
	ensureValidFontSize(iFontSize);
	SFontCharInfo* pCharInfo = ensureCharInfoExists(iFontSize, cChar);

	if(pCharInfo->myAdvance.x == 0.0 && pCharInfo->myAdvance.y == 0.0)	
	{
		FT_Set_Pixel_Sizes(myFace, 0, iFontSize);
		if(FT_Load_Char(myFace, cChar, FT_LOAD_DEFAULT))
		{
			_ASSERT(0);
		}

		FT_GlyphSlot pGlyph = myFace->glyph;
		// >> 6 is divide by 64, since FreeType stores these units in 1/64th of a pixel.
		// They must have been bored.
		pCharInfo->myAdvance.set(pGlyph->advance.x >> 6, pGlyph->advance.y >> 6);
	}

	svDimsOut = pCharInfo->myAdvance;
}
/*****************************************************************************/
void CachedFontInfo::renderCharacterAt(DrawingCache* pDrawingCache, UNSIGNED_CHAR_TYPE cChar, FLOAT_TYPE fX, FLOAT_TYPE fY, 
								 const SColor& scolColor, int iFontSize, FLOAT_TYPE fScale) const
{
	if(cChar == '\t' || cChar == '\r')
		cChar = ' ';

	float sx = fScale;
	float sy = fScale;

	ensureValidFontSize(iFontSize);

	SFontCharInfo* pCharInfo = ensureCharInfoExists(iFontSize, cChar);

	if(pCharInfo->myTexture == 0)
	{
		FT_Set_Pixel_Sizes(myFace, 0, iFontSize);
/*
#ifdef _DEBUG
		if(cChar == 138)
		{
			int bp = 0;
		}
		FT_Select_Charmap(myFace, FT_ENCODING_BIG5);
#endif*/

		if(FT_Load_Char(myFace, cChar, FT_LOAD_RENDER))
			ASSERT_RETURN;

		FT_GlyphSlot pGlyph = myFace->glyph;

		pCharInfo->myPower2Size.x = forcePowerOfTwo(pGlyph->bitmap.width + 2);
		pCharInfo->myPower2Size.y = forcePowerOfTwo(pGlyph->bitmap.rows + 2);

		int iDataSize = pCharInfo->myPower2Size.x*pCharInfo->myPower2Size.y*4;

		unsigned char* pData = myDataArray.getArray(iDataSize);
		memset(pData, 0, sizeof(unsigned char)*iDataSize);

		pCharInfo->myTexLetterStart.x = (pCharInfo->myPower2Size.x - pGlyph->bitmap.width)/2;
		pCharInfo->myTexLetterStart.y = (pCharInfo->myPower2Size.y - pGlyph->bitmap.rows)/2;

		int iSrcOffset, iDstOffset;
		int iX, iY;
		for(iX = 0; iX < pGlyph->bitmap.width; iX++)
		{
			for(iY = 0; iY < pGlyph->bitmap.rows; iY++)
			{
				iSrcOffset = (iY*pGlyph->bitmap.pitch) + iX;
				iDstOffset = ((iY + pCharInfo->myTexLetterStart.y) * pCharInfo->myPower2Size.x*4) + (iX + pCharInfo->myTexLetterStart.x)*4;
				pData[iDstOffset] = pGlyph->bitmap.buffer[iSrcOffset];
				pData[iDstOffset + 1] = pGlyph->bitmap.buffer[iSrcOffset];
				pData[iDstOffset + 2] = pGlyph->bitmap.buffer[iSrcOffset];
				pData[iDstOffset + 3] = pGlyph->bitmap.buffer[iSrcOffset];
			}
		}

		//FontManager::getInstance()->bindMainTexture();
		pCharInfo->createAndBindTexture(pCharInfo->myPower2Size.x, pCharInfo->myPower2Size.y, pData);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pCharInfo->myPower2Size.x, pCharInfo->myPower2Size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);

		pCharInfo->myGlyphTopLeft.x = pGlyph->bitmap_left;
		pCharInfo->myGlyphTopLeft.y = pGlyph->bitmap_top;
	}

// 	FLOAT_TYPE x = (int)fX;
// 	FLOAT_TYPE y = (int)fY;
	FLOAT_TYPE x = (int)(fX*10.0)/10;
	FLOAT_TYPE y = (int)(fY*10.0)/10;

	SVector2D svOrigin;

	svOrigin.x = x + pCharInfo->myGlyphTopLeft.x * sx - pCharInfo->myTexLetterStart.x;
	svOrigin.y = y - pCharInfo->myGlyphTopLeft.y * sy - pCharInfo->myTexLetterStart.y;

	// Add the line height:
	FLOAT_TYPE fVertOffset = (int)((myLineHeightMult*iFontSize - myDescenderPortion*iFontSize)*fScale);
	y += fVertOffset;
	svOrigin.y += fVertOffset;

	SVector2D svSize;
	svSize.x = pCharInfo->myPower2Size.x * sx;
	svSize.y = pCharInfo->myPower2Size.y * sy;
	
// 	if(bRenderImmediately)
// 		renderCharTexture(pCharInfo->myTexture, svOrigin, svSize, scolColor);
// 	else
		pDrawingCache->addCachedLetter(pCharInfo->myTexture, svOrigin, svSize, scolColor);
/*
	GLfloat box[] = 
	{
		svOrigin.x,     svOrigin.y,
		svOrigin.x + svSize.x, svOrigin.y,
		svOrigin.x,     svOrigin.y + svSize.y,
		svOrigin.x + svSize.x, svOrigin.y + svSize.y,
	};

	const GLfloat tmpColors[] =
	{
		scolColor.r, scolColor.g, scolColor.b, scolColor.alpha,
		scolColor.r, scolColor.g, scolColor.b, scolColor.alpha,
		scolColor.r, scolColor.g, scolColor.b, scolColor.alpha,
		scolColor.r, scolColor.g, scolColor.b, scolColor.alpha,
	};

	static GLfloat g_NonConstSpriteTexcoords[] =
	{
		0, 0,
		1, 0,
		0, 1,
		1, 1,
	};

	glBindTexture(GL_TEXTURE_2D, pCharInfo->myTexture);
	gDrawBuffer(box, g_NonConstSpriteTexcoords, tmpColors, 4, GL_TRIANGLE_STRIP);*/

	//glBindTexture(GL_TEXTURE_2D, 0);
	//delete[] pData;
}
/*****************************************************************************/
void CachedFontInfo::renderCharTexture(TX_MAN_RETURN_TYPE rTextureIndex, const SVector2D& svOrigin, const SVector2D& svSize, const SColor& scolColor)
{
	GLfloat box[] =
	{
		svOrigin.x,     svOrigin.y,
		svOrigin.x + svSize.x, svOrigin.y,
		svOrigin.x,     svOrigin.y + svSize.y,
		svOrigin.x + svSize.x, svOrigin.y + svSize.y,
	};

	const GLfloat tmpColors[] =
	{
		scolColor.r*scolColor.alpha, scolColor.g*scolColor.alpha, scolColor.b*scolColor.alpha, scolColor.alpha,
		scolColor.r*scolColor.alpha, scolColor.g*scolColor.alpha, scolColor.b*scolColor.alpha, scolColor.alpha,
		scolColor.r*scolColor.alpha, scolColor.g*scolColor.alpha, scolColor.b*scolColor.alpha, scolColor.alpha,
		scolColor.r*scolColor.alpha, scolColor.g*scolColor.alpha, scolColor.b*scolColor.alpha, scolColor.alpha,
	};

	static GLfloat g_NonConstSpriteTexcoords[] =
	{
		0, 0,
		1, 0,
		0, 1,
		1, 1,
	};

	GraphicsUtils::setDiffuseTexture(rTextureIndex);
	//glBindTexture(GL_TEXTURE_2D, rTextureIndex);

#ifdef DIRECTX_PIPELINE
	if(!theGeometryBuffer)
		theGeometryBuffer = new AdhocDxBuffer;
	theGeometryBuffer->setFrom2DArrays(box, g_NonConstSpriteTexcoords, tmpColors, 4);

	g_pDxRenderer->getD3dContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT stride = sizeof(BasicVertex);
	UINT offset = 0;
	g_pDxRenderer->getD3dContext()->IASetVertexBuffers(0, 1, theGeometryBuffer->getVertexBuffer(), &stride, &offset);
	g_pDxRenderer->getD3dContext()->IASetIndexBuffer(theGeometryBuffer->getIndexBuffer(), INDEX_BUFFER_FORMAT, 0);
	g_pDxRenderer->getD3dContext()->DrawIndexed(4, 0, 0);

	g_pDxRenderer->getD3dContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#else
	//setBlendMode(BlendModeNormal);
	RenderUtils::drawBuffer(box, g_NonConstSpriteTexcoords, tmpColors, 4, GL_TRIANGLE_STRIP);

	OpenGLStateCleaner::doVertexPointerCall(2, GL_FLOAT, 0, box);
	OpenGLStateCleaner::doColorPointerCall(4, GL_FLOAT, 0, tmpColors);
	OpenGLStateCleaner::doTexturePointerCall(2, GL_FLOAT, 0, g_NonConstSpriteTexcoords);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif
}
/*****************************************************************************/
SFontCharInfo* CachedFontInfo::ensureCharInfoExists(int iFontSize, char cChar) const
{
	SFontCharInfo* pCharInfo = getCharInfo(iFontSize, cChar);
	if(!pCharInfo)
	{
		SFontCharInfo rInfo;
		return setCharInfo(iFontSize, cChar, rInfo);
	}
	else
		return pCharInfo;
}
/*****************************************************************************/
SFontCharInfo* CachedFontInfo::getCharInfo(int iFontSize, CHAR_TYPE cChar) const
{
	// First, find the font size:
	TFontCharInfoHash* pFontCharHash = myCache.find(iFontSize);
	if(!pFontCharHash)
		return NULL;

	return pFontCharHash->find(cChar);
}
/*****************************************************************************/
SFontCharInfo* CachedFontInfo::setCharInfo(int iFontSize, CHAR_TYPE cChar, const SFontCharInfo& rInfo) const
{
	TFontCharInfoHash* pFontCharHash = myCache.find(iFontSize);
	if(!pFontCharHash)
	{
		TFontCharInfoHash rInfoHash;
		pFontCharHash = myCache.insert(iFontSize, rInfoHash);
		_ASSERT(pFontCharHash);
	}

	return pFontCharHash->insert(cChar, rInfo);
}
/*****************************************************************************/
void CachedFontInfo::ensureValidFontSize(int& iFontSizeInOut) const
{
	if(iFontSizeInOut <= 0)
		iFontSizeInOut = myDefaultSize;

	if(iFontSizeInOut <= 0)
		iFontSizeInOut = TEMP_DEFAULT_FONT_SIZE;
}
/*****************************************************************************/
SFontCharInfo::SFontCharInfo()
{
	myTexture = NULL;
#ifdef DIRECTX_PIPELINE
	myActualTexturePtr = NULL;
#endif
}
/*****************************************************************************/
SFontCharInfo::~SFontCharInfo()
{
	if(myTexture != NULL) 
		GraphicsUtils::deleteTexture(myTexture);

#ifdef DIRECTX_PIPELINE
	if(myActualTexturePtr)
		myActualTexturePtr->Release();
	myActualTexturePtr = NULL;
#endif
}
/*****************************************************************************/
// SFontCharInfo
/*****************************************************************************/
void SFontCharInfo::createAndBindTexture(int xSize, int ySize, unsigned char* pData)
{
#ifdef DIRECTX_PIPELINE
	myActualTexturePtr = GraphicsUtils::createTextureFrom(xSize, ySize, pData, false);
	myTexture = &myActualTexturePtr;
#else
	myTexture = GraphicsUtils::createTextureFrom(xSize, ySize, pData, false);
#endif
/*	
	glGenTextures(1, &myTexture);
	glBindTexture(GL_TEXTURE_2D, myTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	*/
}
/*****************************************************************************/
};
#endif
