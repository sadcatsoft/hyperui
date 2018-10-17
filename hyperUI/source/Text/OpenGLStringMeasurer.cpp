#include "stdafx.h"

namespace HyperUI
{
OpenGLStringMeasurer * OpenGLStringMeasurer::theInstance = NULL;
/*****************************************************************************/
void OpenGLStringMeasurer::measureString(Window* pWindow, const char* pcsString, const char* pcsFont, FLOAT_TYPE fPixelSize, Material* pMaterial, FLOAT_TYPE fKerning, FLOAT_TYPE fLineSpacing, SVector2D& svSizeOut)
{
	_ASSERT(pWindow);
	RenderUtils::measureText(pWindow, pcsString, pcsFont, fPixelSize, 0, svSizeOut);
}
/*****************************************************************************/
FLOAT_TYPE OpenGLStringMeasurer::getLineHeight(Window* pWindow, const char* pcsFont, FLOAT_TYPE fPixelSize, Material* pMaterial, FLOAT_TYPE fKerning, FLOAT_TYPE fLineSpacing)
{
#ifdef USE_FREETYPE_FONTS
	const CachedFontInfo* pFontInfo = pWindow->getFontManager()->getFontInfo(pcsFont);
	if(pFontInfo)
		return pFontInfo->getLineHeight(fPixelSize) + fLineSpacing + pFontInfo->getExtraLineSpacing(fPixelSize);
#else
	ResourceCollection* pFonts = ResourceManager::getInstance()->getCollection(ResourceRasterFonts);
	ResourceItem* pItem = pFonts->getItemById(pcsFont);
	if(pItem)
		return pItem->getNumProp(PropertyFtHeight) + fLineSpacing + pItem->getNumProp(PropertyExtraLineSpacing);
#endif
	else
	{
		_ASSERT(0);
		return upToScreen(20.) + fLineSpacing;
	}
}
/*****************************************************************************/
};