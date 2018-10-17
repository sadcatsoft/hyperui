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
#pragma once

#ifdef USE_FREETYPE_FONTS

// Forward declare this so we don't have to include freetype headers here.
struct FT_LibraryRec_;
typedef struct FT_LibraryRec_  *FT_Library;
struct FT_FaceRec_;
typedef struct FT_FaceRec_*  FT_Face;

namespace HyperUI
{
#ifdef DIRECTX_PIPELINE
class AdhocDxBuffer;
#endif
class FontManager;
/*****************************************************************************/
struct SFontCharInfo
{
	SFontCharInfo();
	~SFontCharInfo();

	void createAndBindTexture(int xSize, int ySize, unsigned char* pData);

	SVector2D myAdvance;
	TX_MAN_RETURN_TYPE myTexture;
#ifdef DIRECTX_PIPELINE
	ID3D11ShaderResourceView* myActualTexturePtr;
#endif

	// Texture storage params
	SIntVector2D myTexLetterStart;
	SIntVector2D myPower2Size;
	SIntVector2D myGlyphTopLeft;
};
typedef HashMap < CHAR_TYPE, CHAR_TYPE, SFontCharInfo, 64 > TFontCharInfoHash;
typedef HashMap < int, int, TFontCharInfoHash, 64 > TIntFontCharInfoHash;
/*****************************************************************************/
class HYPERUI_API CachedFontInfo
{
public:

	CachedFontInfo();
	~CachedFontInfo();

	FLOAT_TYPE getLineHeight(int iFontSize) const;
	FLOAT_TYPE getExtraLineSpacing(int iFontSize) const;
	FLOAT_TYPE getKerning(int iFontSize) const;
	void getCharDims(UNSIGNED_CHAR_TYPE cChar, int iFontSize, SVector2D& svDimsOut) const;
	void renderCharacterAt(DrawingCache* pDrawingCache, UNSIGNED_CHAR_TYPE cChar, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, const SColor& scolColor, int iFontSize, FLOAT_TYPE fScale = 1.0) const;

	static void renderCharTexture(TX_MAN_RETURN_TYPE rTextureIndex, const SVector2D& svOrigin, const SVector2D& svSize, const SColor& scolColor);

private:

	SFontCharInfo* ensureCharInfoExists(int iFontSize, CHAR_TYPE cChar) const;

	SFontCharInfo* getCharInfo(int iFontSize, CHAR_TYPE cChar) const;
	SFontCharInfo* setCharInfo(int iFontSize, CHAR_TYPE cChar, const SFontCharInfo& rInfo) const;

	void ensureValidFontSize(int& iFontSizeInOut) const;

private:

	friend class FontManager;

	FLOAT_TYPE myDefaultSize;

	FLOAT_TYPE myLineHeightMult;
	FLOAT_TYPE myKerning;
	FLOAT_TYPE myExtraLineSpacing;

	FLOAT_TYPE myDescenderPortion;

	FT_Face myFace;

	mutable TUCharLossyDynamicArray myDataArray;

	mutable TIntFontCharInfoHash myCache;

#ifdef DIRECTX_PIPELINE
	static AdhocDxBuffer* theGeometryBuffer;
#endif
};
typedef HashMap < const CHAR_TYPE*, RESOURCEITEM_STRING_TYPE, CachedFontInfo*, 64 > TFontInfoHash;
/*****************************************************************************/
class HYPERUI_API FontManager
{
public:

	FontManager();
	~FontManager();

	void initFromCollection(ResourceCollection* pCollection);

	const CachedFontInfo* getFontInfo(const char* pcsFontName);

	static void destroyFreelib();

	void clear();

private:

	TFontInfoHash myFonts;

	static bool theIsFreelibInited;
	static FT_Library theFreetypLib;
};
/*****************************************************************************/
};
#endif
