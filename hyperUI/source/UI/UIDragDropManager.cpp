#include "stdafx.h"

#define DRAG_REGION_SIZE		upToScreen(50)
#define REGION_SIDE_OFFSET		upToScreen(2.5)

namespace HyperUI
{
/*****************************************************************************/
UIDragDropManager::UIDragDropManager()
{

}
/*****************************************************************************/
void UIDragDropManager::render(UIElement* pElem, SVector2D svPos)
{
	UIPlane* pUIPlane = pElem->getUIPlane();
	UIElement* pTopmostOverElem = pUIPlane->getTopmostShownElementAt(svPos.x, svPos.y, false);
	pTopmostOverElem = pUIPlane->getFinalChildAtCoords(svPos.x, svPos.y, pTopmostOverElem, false, false);
	UITabWindowElement *pOverTabParent = NULL;
	if(pTopmostOverElem)
	{
		// See if the dragged over elem is a child of the ui tab window. A direct child.
		pOverTabParent = as<UITabWindowElement>(pTopmostOverElem);
		if(!pOverTabParent)
			pOverTabParent = pTopmostOverElem->getParentOfType<UITabWindowElement>();
		if(pOverTabParent)
		{
			// Now, draw the decor:
			SRect2D srRect;
			pOverTabParent->getGlobalRectangle(srRect);
			pElem->getDrawingCache()->addScalableButton("layerDragSelThin", srRect.x + srRect.w/2.0, srRect.y + srRect.h/2.0, srRect.w, srRect.h, 1.0);
			SColor scolColor(15.0/255.0, 156.0/255.0, 1.0, 0.4);
			pElem->getDrawingCache()->addRectangle(srRect, scolColor);


			pElem->getDrawingCache()->flush();

			recomputeDragRegions(pUIPlane, srRect);
		}
	}

	// Render the drag icons
	int iSelRegion = getRegionIndexAt(svPos);

	SUIDragRegion* pRegion;
	const char* pcsAnim;
	SVector2D svRegionPos;
	int iCurr, iNum = myDragRegions.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pRegion = &myDragRegions[iCurr];
		pcsAnim = (iCurr == iSelRegion) ? pRegion->myHoverAnim.c_str() : pRegion->myAnim.c_str();
		svRegionPos = pRegion->myRect.center();
		// Only render the local ones if we have a valid tab elem
		if(pRegion->myIsGlobal || pOverTabParent)
			pElem->getDrawingCache()->addSprite(pcsAnim, svRegionPos.x, svRegionPos.y, 1.0, 0, 1.0, 1.0, true);
	}

}
/*****************************************************************************/
void UIDragDropManager::recomputeDragRegions(UIPlane* pUIPlane, const SRect2D& srNewRect)
{
	SRect2D srIntRegion = srNewRect;
	srIntRegion.castToInt();

	if(srIntRegion == myLastDragRegion)
		return;

	SVector2D svTempPoint;
	const SVector2D svSize(DRAG_REGION_SIZE, DRAG_REGION_SIZE);
	myDragRegions.clear();

	SUIDragRegion rRegion;

	// Center
	rRegion.myRect.setFromCenterAndSize(srIntRegion.center(), svSize);
	rRegion.mySide = SideCenter;
	rRegion.myAnim = "dragDropAnims:5";
	rRegion.myHoverAnim = "dragDropAnims:10";
	rRegion.myIsGlobal = false;
	myDragRegions.push_back(rRegion);

	svTempPoint.x = srIntRegion.x + srIntRegion.w - DRAG_REGION_SIZE/2.0 - REGION_SIDE_OFFSET;
	svTempPoint.y = srIntRegion.center().y;
	rRegion.myRect.setFromCenterAndSize(svTempPoint, svSize);
	rRegion.mySide = SideRight;
	rRegion.myAnim = "dragDropAnims:3";
	rRegion.myHoverAnim = "dragDropAnims:8";
	rRegion.myIsGlobal = false;
	myDragRegions.push_back(rRegion);

	svTempPoint.x = srIntRegion.x + DRAG_REGION_SIZE/2.0 + REGION_SIDE_OFFSET;
	svTempPoint.y = srIntRegion.center().y;
	rRegion.myRect.setFromCenterAndSize(svTempPoint, svSize);
	rRegion.mySide = SideLeft;
	rRegion.myAnim = "dragDropAnims:1";
	rRegion.myHoverAnim = "dragDropAnims:6";
	rRegion.myIsGlobal = false;
	myDragRegions.push_back(rRegion);

	svTempPoint.x = srIntRegion.center().x;
	svTempPoint.y = srIntRegion.y + srIntRegion.h - DRAG_REGION_SIZE/2.0 - REGION_SIDE_OFFSET;
	rRegion.myRect.setFromCenterAndSize(svTempPoint, svSize);
	rRegion.mySide = SideBottom;
	rRegion.myAnim = "dragDropAnims:4";
	rRegion.myHoverAnim = "dragDropAnims:9";
	rRegion.myIsGlobal = false;
	myDragRegions.push_back(rRegion);

	svTempPoint.x = srIntRegion.center().x;
	svTempPoint.y = srIntRegion.y + DRAG_REGION_SIZE/2.0 + REGION_SIDE_OFFSET;
	rRegion.myRect.setFromCenterAndSize(svTempPoint, svSize);
	rRegion.myAnim = "dragDropAnims:2";
	rRegion.myHoverAnim = "dragDropAnims:7";
	rRegion.mySide = SideTop;
	rRegion.myIsGlobal = false;
	myDragRegions.push_back(rRegion);

	// Add the global regions
	UIElement* pGlobalElem = pUIPlane->getElementById("toolSplitter");
	if(pGlobalElem)
		pGlobalElem = pGlobalElem->getChild<UIElement>(1);
	SRect2D srGlobalRect;
	if(pGlobalElem)
	{
		pGlobalElem->getGlobalRectangle(srGlobalRect);
		srGlobalRect.castToInt();

		svTempPoint.x = srGlobalRect.x + srGlobalRect.w - DRAG_REGION_SIZE/2.0 - REGION_SIDE_OFFSET;
		svTempPoint.y = srGlobalRect.center().y;
		rRegion.myRect.setFromCenterAndSize(svTempPoint, svSize);
		rRegion.mySide = SideRight;
		rRegion.myAnim = "dragDropAnims:3";
		rRegion.myHoverAnim = "dragDropAnims:8";
		rRegion.myIsGlobal = true;
		myDragRegions.push_back(rRegion);

		svTempPoint.x = srGlobalRect.x + DRAG_REGION_SIZE/2.0 + REGION_SIDE_OFFSET;
		svTempPoint.y = srGlobalRect.center().y;
		rRegion.myRect.setFromCenterAndSize(svTempPoint, svSize);
		rRegion.mySide = SideLeft;
		rRegion.myAnim = "dragDropAnims:1";
		rRegion.myHoverAnim = "dragDropAnims:6";
		rRegion.myIsGlobal = true;
		myDragRegions.push_back(rRegion);

		svTempPoint.x = srGlobalRect.center().x;
		svTempPoint.y = srGlobalRect.y + srGlobalRect.h - DRAG_REGION_SIZE/2.0 - REGION_SIDE_OFFSET;
		rRegion.myRect.setFromCenterAndSize(svTempPoint, svSize);
		rRegion.mySide = SideBottom;
		rRegion.myAnim = "dragDropAnims:4";
		rRegion.myHoverAnim = "dragDropAnims:9";
		rRegion.myIsGlobal = true;
		myDragRegions.push_back(rRegion);

		svTempPoint.x = srGlobalRect.center().x;
		svTempPoint.y = srGlobalRect.y + DRAG_REGION_SIZE/2.0 + REGION_SIDE_OFFSET;
		rRegion.myRect.setFromCenterAndSize(svTempPoint, svSize);
		rRegion.myAnim = "dragDropAnims:2";
		rRegion.myHoverAnim = "dragDropAnims:7";
		rRegion.mySide = SideTop;
		rRegion.myIsGlobal = true;
		myDragRegions.push_back(rRegion);
	}

	myLastDragRegion = srIntRegion;
}
/*****************************************************************************/
int UIDragDropManager::getRegionIndexAt(const SVector2D& svScreenPos)
{
	int iCurr, iNum = myDragRegions.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(myDragRegions[iCurr].myRect.doesContain(svScreenPos))
			return iCurr;
	}

	return -1;
}
/*****************************************************************************/
SUIDragRegion* UIDragDropManager::getRegionAt(const SVector2D& svScreenPos)
{
	int iRegion = getRegionIndexAt(svScreenPos);
	if(iRegion >= 0 && iRegion < myDragRegions.size())
		return &myDragRegions[iRegion];
	else
		return NULL;
}
/*****************************************************************************/
};