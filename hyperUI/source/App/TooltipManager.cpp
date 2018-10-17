#include "stdafx.h"

#define TOOLTIP_KILL_TOLERANCE_DIST		upToScreen(7.0)
#define TOOLTIP_SHOW_TOLERANCE_DIST		upToScreen(1.5)

//#define TOOLTIP_DELAY			0.75
#define TOOLTIP_FADE_TIME		0.10

#define MAX_TOOLTIP_WIDTH			upToScreen(90)
#define TOOLTIP_FONT				MAIN_FONT_SMALL
#define TOOLTIP_FONT_SIZE			MAIN_FONT_SMALL_DEF_SIZE
#define TEXT_PADDING_FOR_ANIM		upToScreen(12)
#define TOOLTIP_BACK_ANIM			"tooltipBackSharp"
//#define TOOLTIP_NOSE_ANIM			"tooltipNose"

#define MIN_HOR_TOOLTIP_SCREEN_DISTANCE			upToScreen(5)
#define MIN_VERT_TOOLTIP_SCREEN_DISTANCE		upToScreen(5)

#define TOOLTIP_OFFSET_FROM_CURSOR			upToScreen(10)

namespace HyperUI
{
/*****************************************************************************/
TooltipManager::TooltipManager(Window* pWindow)
{
	myIsEnabled = true;
	myTooltipStartPosition.set(0,0);
	myPrevMousePos.set(0,0);
	myParentWindow = pWindow;
	myPrevPosSetTime = 0;
	myIsShowingTooltip = false;
#ifdef _DEBUG
	myTooltipAnim.setDebugName("TooltipAnim");
#endif
	myTooltipAnim.setNonAnimValue(0);
	myTooltipDelay = -1.0;
}
/*****************************************************************************/
TooltipManager::~TooltipManager()
{

}
/*****************************************************************************/
bool TooltipManager::getIsShowingTooltip(void)
{
	return myIsShowingTooltip; 
	//return !(myTooltipStartPosition.x == 0 && myTooltipStartPosition.y == 0);
}
/*****************************************************************************/
void TooltipManager::onTimerTick(GTIME lTime)
{
	if(!myIsEnabled)
	{
		myPrevPosSetTime = lTime;
		return;
	}
	
	FLOAT_TYPE fDistSq;
	SVector2D svLastMousePos;
	myParentWindow->getLastMousePos(svLastMousePos);

	if(getIsShowingTooltip())	
	{
		// See if we have to kill it
		fDistSq = (svLastMousePos - myTooltipStartPosition).lengthSquared();
		if(fDistSq > TOOLTIP_KILL_TOLERANCE_DIST*TOOLTIP_KILL_TOLERANCE_DIST)
		{
			killTooltip();
		}
	}
	else
	{
		// See if we have to show it
		fDistSq = (svLastMousePos - myPrevMousePos).lengthSquared();
		if(fDistSq > TOOLTIP_SHOW_TOLERANCE_DIST*TOOLTIP_SHOW_TOLERANCE_DIST)
		{
			// Set the new last pos and time
			myPrevPosSetTime = lTime;
			myPrevMousePos = svLastMousePos;
		}
		else
		{
			getTooltipDelay();

			// See if we've been hovering long enough
			if(myPrevPosSetTime != 0 && myPrevPosSetTime + Application::secondsToTicks(myTooltipDelay) < lTime && myParentWindow->getNumActiveTouches() == 0)
			{
				// Get the element, see if it has a tooltip
				UIPlane* pUIPlane = myParentWindow->getUIPlane();
				UIElement* pElement = pUIPlane->getTopmostShownElementAt(svLastMousePos.x, svLastMousePos.y, true);
				pElement = pUIPlane->getFinalChildAtCoords(svLastMousePos.x, svLastMousePos.y, pElement, false, true);

				UITableElement* pTable = dynamic_cast<UITableElement* >(pElement);
				if(pTable)
					pElement = pTable->getFinalCellElementAt(svLastMousePos.x, svLastMousePos.y, true);

				if(pElement && pElement->getDoHaveToolTip(svLastMousePos))
				{
					pElement->getToolTip(svLastMousePos, myTooltipText);
					if(myTooltipText != PROPERTY_NONE)
					{
						// ...And one last check - hack. Don't show a tooltip when the operator
						// RMB menu is showing.
						bool bDoShow = true;
						if(myParentWindow && myParentWindow->getUIPlane()->getIsLayerShown("uilDetailsElemRMBMenu"))
							bDoShow = false;
						if(myParentWindow && myParentWindow->getUIPlane()->getIsLayerShown("uilNewNodeMenu"))
							bDoShow = false;

						if(bDoShow)
						{
							myIsShowingTooltip = true; 
							myTooltipStartPosition = svLastMousePos;
							myTooltipAnim.changeAnimation(0, 1, TOOLTIP_FADE_TIME, ClockUiPrimary);

							SCachedTextObject rTempInfo;
							rTempInfo.myBaseInfo.myFont = TOOLTIP_FONT;
							rTempInfo.myBaseInfo.myFontSize = TOOLTIP_FONT_SIZE;
							rTempInfo.myBaseInfo.myWidth = MAX_TOOLTIP_WIDTH;
							RenderUtils::measureText(myParentWindow, myTooltipText.c_str(), rTempInfo, myTextSize);
						}
					}
					
				}			
			}
		}
	}

// 	GTIME lUiTime = GameEngine::getGlobalTime(ClockUiPrimary);
// 	myTooltipAnim.checkTime(lUiTime);
	myTooltipAnim.getValue();
}
/*****************************************************************************/
void TooltipManager::render(void)
{

	if(myTooltipAnim.getIsAnimating(false) == false && !getIsShowingTooltip())
		return;

	SVector2D svFinalPos;

	FLOAT_TYPE fProgValue = myTooltipAnim.getValue();

	svFinalPos = myTooltipStartPosition;

	SVector2D svLastMousePos;
	myParentWindow->getLastMousePos(svLastMousePos);

	svFinalPos += svLastMousePos - myTooltipStartPosition;

	SRect2D srScreenRect;
	myParentWindow->getWindowRectRelative(srScreenRect);

	// Attempt to auto-determine side
	SideType eSide = SideBottom;
	if(myTooltipStartPosition.y > srScreenRect.h - upToScreen(30))
		eSide = SideTop;
		
	SVector2D svDir;
	if(eSide == SideRight || eSide == SideLeft)
		svDir.x = eSide == SideRight ? 1.0 : -1.0;
	else
		svDir.y = eSide == SideBottom ? 1.0 : -1.0;

	svFinalPos += svDir*TOOLTIP_OFFSET_FROM_CURSOR;

	// Now, we need to ensure this is fully within the screen.
	FLOAT_TYPE fMaxTextWidth = upToScreen(90);


	SVector2D svMaxSize;
	svMaxSize = myTextSize;
	svMaxSize.x += TEXT_PADDING_FOR_ANIM;
	svMaxSize.y += TEXT_PADDING_FOR_ANIM;

	if(eSide == SideRight)
		svFinalPos.x -= svMaxSize.x;
	else if(eSide == SideTop)
		svFinalPos.y -= svMaxSize.y;

	if(eSide == SideTop || eSide == SideBottom)
		svFinalPos.x -= svMaxSize.x/2.0;
	else
		svFinalPos.y -= svMaxSize.y/2.0;
/*
	SVector2D svMaxSize(204, 68);
	if(myTextSize.x > svMaxSize.x)
		svMaxSize.x = myTextSize.x;
	if(myTextSize.y > svMaxSize.y)
		svMaxSize.y = myTextSize.y;
*/

	SRect2D srTooltipRect;
	srTooltipRect.x = svFinalPos.x;
	srTooltipRect.y = svFinalPos.y;
	srTooltipRect.w = svMaxSize.x;
	srTooltipRect.h = svMaxSize.y;


	
	if(!srScreenRect.doesContain(srTooltipRect))
	{
		// We're out of bounds. Shift it.
// 		SVector2D svNormalOut;
// 		srScreenRect.getSideNormal(svFinalPos, svNormalOut, true);
// 		svFinalPos += svNormalOut*max(svMaxSize.x, svMaxSize.y)/2.0;
		if(srTooltipRect.x < MIN_HOR_TOOLTIP_SCREEN_DISTANCE)
			srTooltipRect.x = MIN_HOR_TOOLTIP_SCREEN_DISTANCE;
		if(srTooltipRect.x + srTooltipRect.w + MIN_HOR_TOOLTIP_SCREEN_DISTANCE > srScreenRect.w)
			srTooltipRect.x = srScreenRect.w - (srTooltipRect.w + MIN_HOR_TOOLTIP_SCREEN_DISTANCE);

		if(srTooltipRect.y < MIN_VERT_TOOLTIP_SCREEN_DISTANCE)
			srTooltipRect.y = MIN_VERT_TOOLTIP_SCREEN_DISTANCE;
		if(srTooltipRect.y + srTooltipRect.h + MIN_VERT_TOOLTIP_SCREEN_DISTANCE > srScreenRect.h)
			srTooltipRect.y = srScreenRect.h - (srTooltipRect.h + MIN_VERT_TOOLTIP_SCREEN_DISTANCE);
	}

	srTooltipRect.getCenter(svFinalPos);
	

	SColor scolTipCol(0.8,0.8,0.8, fProgValue);

	int iCornerW, iCornerH;
	myParentWindow->getTextureManager()->getCornerSize(TOOLTIP_BACK_ANIM, iCornerW, iCornerH);
	myParentWindow->getDrawingCache()->addScalableButton(TOOLTIP_BACK_ANIM, svFinalPos.x, svFinalPos.y, srTooltipRect.w, srTooltipRect.h, fProgValue, iCornerW, iCornerH);
 	myParentWindow->getDrawingCache()->addText(myTooltipText.c_str(), TOOLTIP_FONT, TOOLTIP_FONT_SIZE, svFinalPos.x, svFinalPos.y, scolTipCol, HorAlignCenter, VertAlignCenter, 0, NULL,NULL,NULL, MAX_TOOLTIP_WIDTH);

	// Draw the nose if there's a

// 	myParentWindow->getDrawingCache()->addSprite("tooltipAnim", svFinalPos.x, svFinalPos.y, fProgValue, 0, 1,1,1.0, true);

}
/*****************************************************************************/
void TooltipManager::onMouseDown(void)
{	
	myPrevPosSetTime = Application::getInstance()->getGlobalTime(ClockUiPrimary);
	if(getIsShowingTooltip())
		killTooltip();
}
/*****************************************************************************/
void TooltipManager::onMouseUp(void)
{
	myPrevPosSetTime = Application::getInstance()->getGlobalTime(ClockUiPrimary);
	if(getIsShowingTooltip())
		killTooltip();
}
/*****************************************************************************/
void TooltipManager::killTooltip(void)
{
	myTooltipAnim.changeAnimation(1, 0, TOOLTIP_FADE_TIME, ClockUiPrimary);
	myIsShowingTooltip = false;
	
}
/*****************************************************************************/
void TooltipManager::setIsEnabled(bool bValue) 
{ 
	myIsEnabled = bValue; 
	if(myIsShowingTooltip)
		killTooltip(); 
}
/*****************************************************************************/
FLOAT_TYPE TooltipManager::getTooltipDelay()
{
	if(myTooltipDelay < 0)
	{
		ResourceItem* pResItem = SettingsCollection::getInstance()->getSettingsItem();
		if(pResItem->doesPropertyExist(PropertySetTooltipDelay))
		{
			myTooltipDelay = pResItem->getNumProp(PropertySetTooltipDelay);
			myTooltipDelay = HyperCore::clampToRange<FLOAT_TYPE>(0, 10.0, myTooltipDelay);
		}
		else
			myTooltipDelay = DEFAULT_TOOLTIP_DELAY;
	}
	return myTooltipDelay;
}
/*****************************************************************************/
};