#include "stdafx.h"

namespace HyperUI
{
UITabRenderer* UITabRenderer::theInstance = NULL;
/*****************************************************************************/
void UITabRenderer::render(UIElement* pElem, SVector2D svPos)
{
	UITabWindowElement* pTabHost = as<UITabWindowElement>(pElem);
	if(!pTabHost)
		return;

	int iTabIndex = pTabHost->getTabBeingDragged();
	if(iTabIndex < 0)
		return;

	// Render the tab being dragged
	STabInfo* pTabInfo = pTabHost->getTabInfo(iTabIndex);
	SVector2D svScroll;

	STabRenderCommonInfo rInfo;
	pTabHost->computeTabRenderCommonInfo(1.0, svScroll, 1.0, rInfo);

	svScroll = svPos - (pTabInfo->myRect.center() + rInfo.myPos);

	// Render the decor 
	UIPlane* pUIPlane = pElem->getParentWindow()->getUIPlane();
	pUIPlane->getUIDragDropManager()->render(pElem, svPos);

	// Render the tab itself
	int iSelTabIndex;
	SRect2D srRect;
	UITabRenderer::renderTab(pTabHost, pTabInfo, iTabIndex, rInfo, iSelTabIndex, srRect, svScroll);
}
/*****************************************************************************/
void UITabRenderer::renderTab(UITabWindowElement* pParent, STabInfo* pInfo, int iTab, STabRenderCommonInfo& rCommonInfo,  int& iSelTabIdx, SRect2D& srSelTabRect, const SVector2D& svExtraOffset)
{
	UIElement* pChild;
	const char* pcsAnimToUse;

	SColor scolFinalText;
	SRect2D srTabRect;
	SVector2D svExtraTabOffset;

	SVector2D svAdditionalTabSize;
	SVector2D svFinalCornerSize;
	FLOAT_TYPE fCloseButtonOpacity;
	FLOAT_TYPE fCloseHoverValue;
	FLOAT_TYPE fTotalIconSpace;

	SVector2D svFinalPos = rCommonInfo.myPos + svExtraOffset;

	pChild = dynamic_cast<UIElement*>(pParent->getChild(iTab));
	if(pChild && pChild->getIsVisible())
	{
		// Selected tab
		pcsAnimToUse = pParent->getSelTabAnim();
		svAdditionalTabSize.set(0, rCommonInfo.mySelTabHeight - rCommonInfo.myTabHeight);
		svFinalCornerSize.set(rCommonInfo.mySelCornerW, rCommonInfo.mySelCornerH);
		svExtraTabOffset.set(SEL_TAB_DRAWING_OFFSET_X, SEL_TAB_DRAWING_OFFSET_Y);
		scolFinalText = rCommonInfo.mySelTextColor;
	}
	else
	{
		// Unselected tab
		pcsAnimToUse = pParent->getTabAnim();
		svAdditionalTabSize.set(0, 0);
		svExtraTabOffset.set(0, 0);
		svFinalCornerSize.set(rCommonInfo.myCornerW, rCommonInfo.myCornerH);
		scolFinalText = rCommonInfo.myTextColor;
	}

	// Tab back
	srTabRect.x = svFinalPos.x + pInfo->myRect.x + pInfo->myRect.w/2.0 - svAdditionalTabSize.x/2.0 + svExtraTabOffset.x;
	srTabRect.y = svFinalPos.y + pInfo->myRect.y + pInfo->myRect.h/2.0 - svAdditionalTabSize.y/2.0 + svExtraTabOffset.y;
	srTabRect.w = pInfo->myRect.w + svAdditionalTabSize.x;
	srTabRect.h = pInfo->myRect.h + svAdditionalTabSize.y;
	pParent->getDrawingCache()->addScalableButton(pcsAnimToUse, srTabRect.x, srTabRect.y, srTabRect.w, srTabRect.h, rCommonInfo.myFinalOpacity, svFinalCornerSize.x, svFinalCornerSize.y);

	fCloseButtonOpacity = rCommonInfo.myFinalOpacity;
	if(pChild && pChild->getIsVisible())
	{
		srSelTabRect = srTabRect;

		// Make it an actual rect, not hte centered one.
		srSelTabRect.x -= srSelTabRect.w/2.0;
		srSelTabRect.y -= srSelTabRect.h/2.0;

		iSelTabIdx = iTab;
	}
	else
		fCloseButtonOpacity *= 0.75;

	if(pInfo->myIcon.length() > 0)
		fTotalIconSpace = pInfo->myIconsDims.x + TAB_ICON_PADDING_X_RIGHT + TAB_ICON_PADDING_X_LEFT;
	else	
		fTotalIconSpace = 0;

	// Icon, if any
	if(pInfo->myIcon.length() > 0)
		pParent->getDrawingCache()->addSprite(pInfo->myIcon.c_str(), svFinalPos.x + pInfo->myRect.x + rCommonInfo.myCornerW + TAB_ICON_PADDING_X_LEFT + pInfo->myIconsDims.x/2.0, svFinalPos.y + pInfo->myRect.y + pInfo->myRect.h/2, rCommonInfo.myFinalOpacity, 0, 1.0, 1.0, true);

	// Text
	pParent->getDrawingCache()->addText(pInfo->myText.c_str(), rCommonInfo.pcsFont, rCommonInfo.iFontSize, svFinalPos.x + pInfo->myRect.x + rCommonInfo.myCornerW + fTotalIconSpace, svFinalPos.y + pInfo->myRect.y + pInfo->myRect.h/2.0, scolFinalText, HorAlignLeft, VertAlignCenter, 0, NULL, rCommonInfo.myShadowColor, &rCommonInfo.myShadowOffset, 0, rCommonInfo.myScale);

	// Close button
	if(!pInfo->myNoCloseButton)
	{
		fCloseHoverValue = pInfo->myCloseHoverAnim.getValue();
		pParent->getDrawingCache()->addSprite(pParent->getTabCloseAnim(), svFinalPos.x + pInfo->myRect.x + pInfo->myRect.w - rCommonInfo.myCornerW - rCommonInfo.myCloseW/2, svFinalPos.y + pInfo->myRect.y + pInfo->myRect.h/2, fCloseButtonOpacity,0, 1.0, 1.0, true);
		if(fCloseHoverValue > 0.0)
			pParent->getDrawingCache()->addSprite(pParent->getTabCloseHoverAnim(), svFinalPos.x + pInfo->myRect.x + pInfo->myRect.w - rCommonInfo.myCornerW - rCommonInfo.myCloseW/2, svFinalPos.y + pInfo->myRect.y + pInfo->myRect.h/2, rCommonInfo.myFinalOpacity*fCloseHoverValue, 0, 1.0, 1.0, true);
	}

	// Drag drop mode
	if(pParent->getIsDraggingOverTabRow() && pChild && pChild->getCurrentDragDropMode() != AcceptNone)
		pParent->getDrawingCache()->addScalableButton("layerDragSelThin", srTabRect.x, srTabRect.y, srTabRect.w, srTabRect.h, rCommonInfo.myFinalOpacity);

}
/*****************************************************************************/
};