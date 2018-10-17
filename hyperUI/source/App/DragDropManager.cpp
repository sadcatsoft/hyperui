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

#define  SRC_OPACITY_WHILE_DRAGGING				0.5 // DO NOT MAKE ZERO - WE'RE DIVIDING BY IT
#define  DRAGGED_ELEM_OPACITY_MULT				0.6 // 0.5

namespace HyperUI
{
/*****************************************************************************/
DragDropManager::DragDropManager(Window* pWindow)
{
	myIsDragBeginAllowed = true;
	myDraggedElem = NULL;
	myLastPos.set(-1, -1);
	myOverrideRenderSeq = NULL;
	myParentWindow = pWindow;
	myCallback = NULL;
}
/*****************************************************************************/
DragDropManager::~DragDropManager()
{
	delete myCallback;
	myCallback = NULL;
}
/*****************************************************************************/
bool DragDropManager::isDragging()
{
	return myDraggedElem != NULL;
}
/*****************************************************************************/
void DragDropManager::beginDrag(UIElement* pElem)
{
	if(!myIsDragBeginAllowed)
		return;

	//_ASSERT(myDraggedElem == NULL);
	myDraggedElem = pElem;

	pElem->onDragDropSelfBegin();
	UIElement *pParent;
	pParent = pElem->getTopmostParent<UIElement>();
	if(pParent)
		pParent->onDragDropChildBegin(pElem);
	myOverrideRenderSeq = pElem->getOverrideDragRenderSeq(myAtlasFrame);

	SVector2D svDummy;
	pElem->getLocalPosition(svDummy, &myInitElemOpacity);
	// We reallly shouldn't be draggin 0-opacity elements...
	_ASSERT(myInitElemOpacity > 0);

	pElem->setNumProp(PropertyOpacity, SRC_OPACITY_WHILE_DRAGGING);
	pElem->resetEvalCache(true);


	myLastPos.set(-1, -1);
}
/*****************************************************************************/
void DragDropManager::cancelDragFor(UIElement* pElem)
{
	if(myDraggedElem == pElem)
	{
		onDragEnd(pElem);
		myDraggedElem = NULL;
	}
}
/*****************************************************************************/
void DragDropManager::render()
{
	if(!myDraggedElem)
		return;

	if(myLastPos.x < 0 || myLastPos.y < 0)
		return;

	TStringCustomRendererMap::iterator mi = myCustomRenderersByTypename.find(myDraggedElem->getStringProp(PropertyId));
	if(mi != myCustomRenderersByTypename.end())
	{
		mi->second->render(myDraggedElem, myLastPos);
		return;
	}

	mi = myCustomRenderersByElemType.find(g_pcsUiElemTypes[myDraggedElem->getElemType()]);
	if(mi != myCustomRenderersByElemType.end())
	{
		mi->second->render(myDraggedElem, myLastPos);
		return;
	}

	// Draw the element there
	FLOAT_TYPE fOpacity, fFinalOpacity = 1.0;
	SVector2D svElemPos, svScroll;
	// Local since the render code in UIElement::render() assumes local.
	myDraggedElem->getLocalPosition(svElemPos, &fOpacity, NULL);
	svScroll = myLastPos - svElemPos;
	if(myOverrideRenderSeq)
		myOverrideRenderSeq->addSprite(myLastPos.x, myLastPos.y, fOpacity*0.5, 0, 1,1, myAtlasFrame, true, true, NULL, myAtlasFrame >= 0);
	else
	{
		if(myInitElemOpacity > 0)
			fFinalOpacity = 1.0;
		myDraggedElem->render(svScroll, fFinalOpacity*DRAGGED_ELEM_OPACITY_MULT/SRC_OPACITY_WHILE_DRAGGING, 1.0);
	}
	//g_pDrawingCache->addSprite("detailsEntryAnim", myLastPos.x, myLastPos.y, 1.0, 0, 1,1,1,true);

}
/*****************************************************************************/
void DragDropManager::onTouchMove(TTouchVector& vecTouches)
{
	if(!myDraggedElem)
		return;

	if(vecTouches.size() <= 0)
		return;

	myLastPos = vecTouches[0].myPoint;

	UIPlane* pUIPlane = myParentWindow->getUIPlane();
	UIElement* pElement = pUIPlane->getTopmostShownElementAt(myLastPos.x, myLastPos.y, false);
	pElement = pUIPlane->getFinalChildAtCoords(myLastPos.x, myLastPos.y, pElement, false, false);

	// First, try the ui plane:
	bool bDidProcess = false;
	if(pUIPlane)
		bDidProcess = pUIPlane->onDragDropHover(myDraggedElem, myLastPos, pElement);

	if(!bDidProcess && pElement && myDraggedElem != pElement)
		pElement->onDragDropHover(myDraggedElem, myLastPos);
	else if(!bDidProcess && myCallback)
		myCallback->onTouchMoveSubclass(myParentWindow, vecTouches, myDraggedElem);
}
/*****************************************************************************/
bool DragDropManager::onTouchUp(TTouchVector& vecTouches)
{
	if(!myDraggedElem)
		return false;

	if(vecTouches.size() <= 0)
		return false;

	SVector2D svPos;
	svPos = vecTouches[0].myPoint;
	
	// Drop onto the target elem
	UIPlane* pUIPlane = myParentWindow->getUIPlane();
	UIElement* pElement = pUIPlane->getTopmostShownElementAt(svPos.x, svPos.y, false);
	pElement = pUIPlane->getFinalChildAtCoords(svPos.x, svPos.y, pElement, false, false);

	bool bProcessedDrag = false;

	// First, try the ui plane:
	if(pUIPlane)
	{
		bProcessedDrag = pUIPlane->receiveDragDrop(myDraggedElem, myLastPos, pElement);
		if(myDraggedElem && bProcessedDrag)
			myDraggedElem->onDragDropFinished(pElement);
	}

	if(pElement && myDraggedElem != pElement && !bProcessedDrag)
	{
		if(pElement->receiveDragDrop(myDraggedElem, svPos))
		{
			if(myDraggedElem)
				myDraggedElem->onDragDropFinished(pElement);
			bProcessedDrag = true;
		}
	}
	
	if(!bProcessedDrag && myCallback)
		bProcessedDrag = myCallback->onTouchUpSubclass(myParentWindow, vecTouches, myDraggedElem, pElement);

	if(!bProcessedDrag && myDraggedElem)
		myDraggedElem->onDragDropCancelled(pElement);

	onDragEnd(myDraggedElem);
	myDraggedElem = NULL;
	return true;
}
/*****************************************************************************/
void DragDropManager::registerRendererByTypename(const char* pcsDraggedElemTypeName, ICustomDragRenderer* pRenderer)
{
	myCustomRenderersByTypename[pcsDraggedElemTypeName] = pRenderer;
}
/*****************************************************************************/
void DragDropManager::unregisterRenderer(ICustomDragRenderer* pRenderer)
{
	TStringCustomRendererMap::iterator mi;
	for(mi = myCustomRenderersByTypename.begin(); mi != myCustomRenderersByTypename.end(); mi++)
	{
		if(mi->second == pRenderer)
		{
			myCustomRenderersByTypename.erase(mi);
			break;
		}
	}

	for(mi = myCustomRenderersByElemType.begin(); mi != myCustomRenderersByElemType.end(); mi++)
	{
		if(mi->second == pRenderer)
		{
			myCustomRenderersByElemType.erase(mi);
			break;
		}
	}
}
/*****************************************************************************/
void DragDropManager::onDragEnd(UIElement* pElem)
{
	// This is ok, it can happen if we've deleted the dragged
	// elem, as happens when we rearrange the UI.
	if(!pElem)
		return;

	pElem->setNumProp(PropertyOpacity, myInitElemOpacity);
	pElem->resetEvalCache(true);
}
/*****************************************************************************/
void DragDropManager::registerRendererByElemType(const char* pcsDraggedElemClass, ICustomDragRenderer* pRenderer)
{
	myCustomRenderersByElemType[pcsDraggedElemClass] = pRenderer;
}
/*****************************************************************************/
};