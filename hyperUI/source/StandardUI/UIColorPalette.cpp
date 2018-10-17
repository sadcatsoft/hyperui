#include "stdafx.h"

#define VERT_BAR_THICKNESS			upToScreen(17)
#define VERT_BAR_LEFT_SPACING		upToScreen(10)
#define VERT_BAR_RIGHT_SPACING		upToScreen(5)

#define COLOR_TRIANGLE_SIZE			upToScreen(5.0)

namespace HyperUI
{
/*****************************************************************************/
UIColorPalette::UIColorPalette(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{

}
/*****************************************************************************/
void UIColorPalette::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
	myCurrUndoBlockId = -1;
	changeModeTo("Hue");
}
/*****************************************************************************/
void UIColorPalette::postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	// Compute rectangles for elements
	SRect2D srLargeArea, srVertBar;
	FLOAT_TYPE fFinalOpacity = computeColorAreas(fOpacity, fScale, srLargeArea, srVertBar);

	getDrawingCache()->flush();

	// Render the large area
	RenderUtils::renderSubdividedRectangle(srLargeArea.x, srLargeArea.y, srLargeArea.w, srLargeArea.h, fOpacity, &myLargeAreaColorGetter, COLOR_TRIANGLE_SIZE);
	// Render the small vertical one
	RenderUtils::renderSubdividedRectangle(srVertBar.x, srVertBar.y, srVertBar.w, srVertBar.h, fFinalOpacity, &myVertBarColorGetter, COLOR_TRIANGLE_SIZE);

	// Render our indicators
	SVector2D svScreenPos;
	FLOAT_TYPE fColorU, fColorV;

	// The color palette
	myLargeAreaColorGetter.getInterpsFromBaseColor(fColorU, fColorV);
	svScreenPos.x = fColorU*srLargeArea.w + srLargeArea.x;
	svScreenPos.y = fColorV*srLargeArea.h + srLargeArea.y;
	theCommonString = "tinyElems:20";
	getDrawingCache()->addSprite(theCommonString.c_str(), svScreenPos.x, svScreenPos.y, fFinalOpacity, 0, fScale, 1.0, true);

	// The vert strip - two side-by-side arrows.
	const FLOAT_TYPE fArrowOffset = upToScreen(2.5);
	myVertBarColorGetter.getInterpsFromBaseColor(fColorU, fColorV);
	svScreenPos.x = fColorU*srVertBar.w + srVertBar.x;
	svScreenPos.y = fColorV*srVertBar.h + srVertBar.y;
	theCommonString = "tinyElems:19";
	getDrawingCache()->addSprite(theCommonString.c_str(), srVertBar.x - fArrowOffset, svScreenPos.y, fFinalOpacity, 180, fScale, 1.0, true);
	getDrawingCache()->addSprite(theCommonString.c_str(), srVertBar.x + srVertBar.w + fArrowOffset, svScreenPos.y, fFinalOpacity, 0, fScale, 1.0, true);

	UIElement::postRender(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
FLOAT_TYPE UIColorPalette::computeColorAreas(FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, SRect2D& srLargeAreaOut, SRect2D& srVertAreaOut)
{
	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	getGlobalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity;
	SVector2D svSize;
	this->getBoxSize(svSize);
	svSize *= fLocScale*fScale;

	SVector2D svWindowStart;
	svWindowStart = svPos - svSize*0.5;

	// Compute rectangles for elements
	srVertAreaOut.y = 0;
	srVertAreaOut.w = VERT_BAR_THICKNESS;
	srVertAreaOut.h = svSize.y;

	srLargeAreaOut.x = 0;
	srLargeAreaOut.y = 0;
	srLargeAreaOut.w = svSize.x - srVertAreaOut.w - VERT_BAR_LEFT_SPACING - VERT_BAR_RIGHT_SPACING;
	srLargeAreaOut.h = svSize.y;

	srVertAreaOut.x = srLargeAreaOut.x + srLargeAreaOut.w + VERT_BAR_LEFT_SPACING;

	srLargeAreaOut.x += svWindowStart.x;
	srLargeAreaOut.y += svWindowStart.y;
	srVertAreaOut.x += svWindowStart.x;
	srVertAreaOut.y += svWindowStart.y;

	return fFinalOpac;
}
/*****************************************************************************/
void UIColorPalette::onPressed(TTouchVector& vecTouches)
{
	SColor scolNew;
	myClickStartArea = getColorFromScreenCoords(vecTouches[0].myPoint, AreaNone, scolNew);
	if(myClickStartArea != AreaNone)
	{
		// Save undo...
		_ASSERT(myCurrUndoBlockId < 0);
		UIColorPicker* pParentPickerHack = getParentOfType<UIColorPicker>();
		UIElement* pFinalUndoUiElem = pParentPickerHack;
		if(!pFinalUndoUiElem)
			pFinalUndoUiElem = this;
		if(UndoManager::canAcceptNewUndoNow(NULL, getParentWindow()))
			myCurrUndoBlockId = UndoManager::addUndoItemToCurrentManager(pFinalUndoUiElem->getUndoStringForSelfChange(), pFinalUndoUiElem->createUndoItemForSelfChange(), true, getParentWindow(), NULL);	

		this->setBaseColor(scolNew);
		this->setAsColor(PropertyActionValue3, scolNew);
		theCommonString3 = COLOR_CHANGED_ACTION;
		this->handleActionUpParentChain(theCommonString3, false);

		getUIPlane()->lockMouseCursor(this);
	}
}
/*****************************************************************************/
void UIColorPalette::onMouseMove(TTouchVector& vecTouches)
{ 
	SColor scolNew;
	AreaType eNewArea = getColorFromScreenCoords(vecTouches[0].myPoint, myClickStartArea, scolNew);
	if(eNewArea != AreaNone)
	{
		this->setBaseColor(scolNew);
		this->setAsColor(PropertyActionValue3, scolNew);
		theCommonString3 = COLOR_CHANGED_ACTION;
		this->handleActionUpParentChain(theCommonString3, false);
	}
}
/*****************************************************************************/
void UIColorPalette::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{
	SColor scolNew;
	AreaType eNewArea = getColorFromScreenCoords(vecTouches[0].myPoint, myClickStartArea, scolNew);
	if(eNewArea != AreaNone)
	{
		this->setBaseColor(scolNew);
		this->setAsColor(PropertyActionValue3, scolNew);
		theCommonString3 = COLOR_CHANGED_ACTION;
		this->handleActionUpParentChain(theCommonString3, false);
	}

	finishUndo();
}
/*****************************************************************************/
UIColorPalette::AreaType UIColorPalette::getColorFromScreenCoords(const SVector2D& svScreenPoint, AreaType eForcedArea, SColor& scolOut)
{
	SRect2D srLargeArea, srVertBar;
	computeColorAreas(1.0, 1.0, srLargeArea, srVertBar);

	if(eForcedArea == AreaLarge || (eForcedArea == AreaNone && srLargeArea.doesContain(svScreenPoint)))
	{
		SVector2D svInterps;
		svInterps.x = (svScreenPoint.x - srLargeArea.x)/srLargeArea.w;
		svInterps.y = (svScreenPoint.y - srLargeArea.y)/srLargeArea.h;
		
		svInterps.x = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, svInterps.x);
		svInterps.y = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, svInterps.y);

		myLargeAreaColorGetter.getVertexColor(svInterps.x, svInterps.y, scolOut);
		scolOut.wrapToRange();
		return AreaLarge;
	}
	else if(eForcedArea == AreaVert || (eForcedArea == AreaNone && srVertBar.doesContain(svScreenPoint)))
	{
		SVector2D svInterps;
		svInterps.x = (svScreenPoint.x - srVertBar.x)/srVertBar.w;
		svInterps.y = (svScreenPoint.y - srVertBar.y)/srVertBar.h;

		svInterps.x = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, svInterps.x);
		svInterps.y = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, svInterps.y);

		myVertBarColorGetter.getVertexColor(svInterps.x, svInterps.y, scolOut);
		scolOut.wrapToRange();
		return AreaVert;
	}

	return AreaNone;
}
/*****************************************************************************/
void UIColorPalette::setBaseColor(const SColor& scolBaseColor)
{
	myVertBarColorGetter.setBaseColor(scolBaseColor);
	myLargeAreaColorGetter.setBaseColor(scolBaseColor);
}
/*****************************************************************************/
void UIColorPalette::changeModeTo(const char* pcsMode)
{
	if(IS_STRING_EQUAL(pcsMode, "Hue"))
	{
		myLargeAreaColorGetter.setColorSources(ColorSourceSaturation, ColorSourceBrightness);
		myVertBarColorGetter.setColorSources(ColorSourceNone, ColorSourceHue);
	}
	else if(IS_STRING_EQUAL(pcsMode, "Sat"))
	{
		myLargeAreaColorGetter.setColorSources(ColorSourceHue, ColorSourceBrightness);
		myVertBarColorGetter.setColorSources(ColorSourceNone, ColorSourceSaturation);
	}
	else if(IS_STRING_EQUAL(pcsMode, "Bri"))
	{
		myLargeAreaColorGetter.setColorSources(ColorSourceHue, ColorSourceSaturation);
		myVertBarColorGetter.setColorSources(ColorSourceNone, ColorSourceBrightness);
	}
	else if(IS_STRING_EQUAL(pcsMode, "Red"))
	{
		myLargeAreaColorGetter.setColorSources(ColorSourceBlue, ColorSourceGreen);
		myVertBarColorGetter.setColorSources(ColorSourceNone, ColorSourceRed);
	}
	else if(IS_STRING_EQUAL(pcsMode, "Green"))
	{
		myLargeAreaColorGetter.setColorSources(ColorSourceBlue, ColorSourceRed);
		myVertBarColorGetter.setColorSources(ColorSourceNone, ColorSourceGreen);
	}
	else if(IS_STRING_EQUAL(pcsMode, "Blue"))
	{
		myLargeAreaColorGetter.setColorSources(ColorSourceRed, ColorSourceGreen);
		myVertBarColorGetter.setColorSources(ColorSourceNone, ColorSourceBlue);
	}
}
/*****************************************************************************/
void UIColorPalette::finishUndo()
{
	if(myCurrUndoBlockId >= 0)
	{
		UndoManager::endUndoBlockInCurrentManager(myCurrUndoBlockId, getParentWindow(), NULL);
		myCurrUndoBlockId = -1;
	}
}
/*****************************************************************************/
};