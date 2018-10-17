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

#define RECENT_COLOR_SIZE		upToScreen(9)
#define RECENT_COLOR_SPACING	upToScreen(1)

#define COLOR_PADDING		upToScreen(0.5)

namespace HyperUI
{
/*****************************************************************************/
UIRecentColorsPalette::UIRecentColorsPalette(UIPlane* pParentPlane)
	:  UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIRecentColorsPalette::onAllocated(IBaseObject* pData)
{
	 UIElement::onAllocated(pData);
	 myMaxColors = 0;

#ifdef _DEBUG
	 // Temp
	 myColors.clear();
	 myColors.push_back(SColor::Red);
	 myColors.push_back(SColor::Green);
	 myColors.push_back(SColor::Blue);
#endif
}
/*****************************************************************************/
void UIRecentColorsPalette::updateOwnData(SUpdateInfo& rRefreshInfo)
{
	recomputeNumMaxColors();
}
/*****************************************************************************/
void UIRecentColorsPalette::getColorScreenPosition(int iIndex, const SRect2D& srGlobalRect, SRect2D& srRectOut)
{
	srRectOut.w = srRectOut.h = RECENT_COLOR_SIZE;

	int iColorsPerRow = (int)(srGlobalRect.w/(srRectOut.w + RECENT_COLOR_SPACING));
	if(iColorsPerRow < 1)
		iColorsPerRow = 1;

	int iRow = iIndex/iColorsPerRow;
	srRectOut.y = iRow*(srRectOut.h + RECENT_COLOR_SPACING) + srGlobalRect.y; 
	int iColumn = iIndex - iColorsPerRow*iRow;
	srRectOut.x = iColumn*(srRectOut.w + RECENT_COLOR_SPACING) + srGlobalRect.x; 
}
/*****************************************************************************/
void UIRecentColorsPalette::onPreRenderChildren(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	UIElement::onPreRenderChildren(svScroll, fOpacity, fScale);

	// Now, render our colors
	SRect2D srGlobalRect;
	getGlobalRectangle(srGlobalRect);

	DrawingCache* pDrawingCache = getDrawingCache();
	this->getAsString(PropertyObjThirdAnim, theCommonString);

	int iNumActualColors = myColors.size();

	SColor scolRenderCol;
	SRect2D srColorRect;
	int iCurrCol;
	for(iCurrCol = 0; iCurrCol < myMaxColors; iCurrCol++)
	{
		getColorScreenPosition(iCurrCol, srGlobalRect, srColorRect);

		if(iCurrCol < iNumActualColors)
		{
			scolRenderCol = myColors[iCurrCol];
			scolRenderCol.alpha = fOpacity;
			getDrawingCache()->addRectangle(srColorRect.x + COLOR_PADDING, srColorRect.y + COLOR_PADDING, srColorRect.w - COLOR_PADDING*2.0, srColorRect.h - COLOR_PADDING*2.0, scolRenderCol);
		}

		// Render the frame
		pDrawingCache->addScalableButton(theCommonString.c_str(), srColorRect.center().x, srColorRect.center().y, srColorRect.w, srColorRect.h, fOpacity);
	}
}
/*****************************************************************************/
void UIRecentColorsPalette::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{
	if(!bIgnoreActions)
	{
		int iColIdx = getIndexFromScreenPosition(vecTouches[0].myPoint);
		if(iColIdx >= 0 && iColIdx < myColors.size())
		{
			// We need to tell whoever cares that we've been clicked
			myLastSelectedColor = myColors[iColIdx];
			theCommonString3 = COLOR_SWATCH_CLICK_ACTION;
			handleActionUpParentChain(theCommonString3, false);
		}
	}

	UIElement::onReleased(vecTouches, bIgnoreActions);
}
/*****************************************************************************/
int UIRecentColorsPalette::getIndexFromScreenPosition(const SVector2D& svScreenPoint)
{
	SRect2D srColorRect;
	int iCurrCol;
	SRect2D srGlobalRect;
	getGlobalRectangle(srGlobalRect);
	for(iCurrCol = 0; iCurrCol < myMaxColors; iCurrCol++)
	{
		getColorScreenPosition(iCurrCol, srGlobalRect, srColorRect);
		if(srColorRect.doesContain(svScreenPoint))
			return iCurrCol;
	}

	return -1;
}
/*****************************************************************************/
void UIRecentColorsPalette::onSizeChanged()
{
	recomputeNumMaxColors();
}
/*****************************************************************************/
void UIRecentColorsPalette::recomputeNumMaxColors()
{
	SRect2D srGlobalRect;
	getGlobalRectangle(srGlobalRect);
	int iColorsPerRow = (int)(srGlobalRect.w/(RECENT_COLOR_SIZE + RECENT_COLOR_SPACING));
	if(iColorsPerRow < 1)
		iColorsPerRow = 1;
	int iRowHeight = RECENT_COLOR_SPACING + RECENT_COLOR_SIZE;
	int iNumRows = srGlobalRect.h/iRowHeight;
	if(iNumRows < 1)
		iNumRows = 1;
	myMaxColors = iNumRows*iColorsPerRow;
}
/*****************************************************************************/
bool compareIndexedColors(const SIndexedColor& r1,const SIndexedColor& r2)
{
	return r1.myIndex > r2.myIndex;
}

void UIRecentColorsPalette::setRecentColors(const TUniqueIndexedColorSet* pColorSet)
{
	myColors.clear();

	if(!pColorSet)
		return;

	TIndexedColorVector rTempVec;
	TUniqueIndexedColorSet::const_iterator si;
	for(si = pColorSet->begin(); si != pColorSet->end(); si++)
		rTempVec.push_back(*si);

	std::sort(rTempVec.begin(), rTempVec.end(), compareIndexedColors);
	int iCurrCol, iNumColors = rTempVec.size();
	for(iCurrCol = 0; iCurrCol < iNumColors; iCurrCol++)
		myColors.push_back(rTempVec[iCurrCol].myColor);
}
/*****************************************************************************/
};