#include "stdafx.h"

#define MAX_TAB_TITLE_CHARS		39

namespace HyperUI
{
/*****************************************************************************/
UITabWindowElement::UITabWindowElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
    onAllocated(pParentPlane);
}
/*****************************************************************************/
void UITabWindowElement::onAllocated(IBaseObject* pData)
{
    UIElement::onAllocated(pData);

	myHorTabScroll = 0.0;
	myTabBeingDragged = -1;
	myLastHoverTabCloseButton = -1;
	myIsDraggingOverTabRow = false;
}
/*****************************************************************************/
void UITabWindowElement::postInit(void)
{
    UIElement::postInit();

	myTabAnim = this->getEnumPropValue(PropertyTabWindowParms, 0);
	mySelTabAnim = this->getEnumPropValue(PropertyTabWindowParms, 1);
	myTabCloseAnim = this->getEnumPropValue(PropertyTabWindowParms, 2);
	if(this->getEnumPropCount(PropertyTabWindowParms) > 3)
		myTabCloseHoverAnim = this->getEnumPropValue(PropertyTabWindowParms, 3);
	else
		myTabCloseHoverAnim = "";

    recomputeTabInfos();

	// We register our persistent rendered. Multiple times is ok.
	getParentWindow()->getDragDropManager()->registerRendererByElemType(g_pcsUiElemTypes[getElemType()], UITabRenderer::getInstance());

	// Set all existing tabs as hidden except the first one
	int iCurr, iNum = getNumChildren();
	UIElement* pChild;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = dynamic_cast<UIElement*>(getChild(iCurr));
		if(!pChild)
			ASSERT_CONTINUE;
		pChild->setIsVisible(iCurr == 0);
	}
}
/*****************************************************************************/
void UITabWindowElement::adjustToScreen(void)
{
    UIElement::adjustToScreen();

	// See if there was a child selected that was already visible
	theCommonString3 = "";
	int iCurr, iNum = getNumChildren();
	UIElement* pChild;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = dynamic_cast<UIElement*>(getChild(iCurr));
		if(!pChild)
			ASSERT_CONTINUE;
		if(pChild->getIsVisible())
		{
			theCommonString3 = pChild->getStringProp(PropertyId);
			break;
		}
	}


    autoLayoutChildrenRecursive();
    setAllChildrenInvisible();
    if(getNumChildren() > 0)
	{
		if(theCommonString3.length() == 0)
			dynamic_cast<UIElement*>(getChild(0))->setIsVisible(true);
		else
		{
			UIElement* pElem = this->getChildById(theCommonString3.c_str(), false, true);
			if(pElem)
				pElem->setIsVisible(true);
			ELSE_ASSERT;
		}
	}

	readjustScroll();
}
/*****************************************************************************/
void UITabWindowElement::autoLayoutChildren()
{
    FLOAT_TYPE fTabHeight = getTabRowHeight(myTabAnim.c_str(), true);

    SVector2D svPos;
    FLOAT_TYPE fGlobalScale;
    this->getGlobalPosition(svPos, NULL, &fGlobalScale);
    SVector2D svOwnSize;
    this->getBoxSize(svOwnSize);
    svOwnSize *= fGlobalScale;

    SRect2D srChildRect;
    srChildRect.x = 0;
    srChildRect.y = fTabHeight;
    srChildRect.w = svOwnSize.x;
    srChildRect.h = svOwnSize.y - fTabHeight;

    int iCurr, iNum = getNumChildren();
    UIElement* pChild;
    for(iCurr = 0; iCurr < iNum; iCurr++)
    {
		pChild = dynamic_cast<UIElement*>(getChild(iCurr));
		if(!pChild)
		{
			_ASSERT(0);
			continue;
		}
		pChild->setWindowRect(srChildRect);
    }
}
/*****************************************************************************/
FLOAT_TYPE UITabWindowElement::getTabRowHeight(const char* pcsAnim, bool bWithPadding)
{
    int iW, iH;
	getTextureManager()->getTextureRealDims(pcsAnim, iW, iH);
    return iH + TAB_TOP_OFFSET_Y;
}
/*****************************************************************************/
void UITabWindowElement::recomputeTabInfos()
{
    const char* pcsFont = this->getCachedFont();
	int iFontSize = this->getCachedFontSize();
    SVector2D svTextDims;
    myTabs.clear();

    // Hor tab padding is the corner size of the tab.
    FLOAT_TYPE fTabHeight = getTabRowHeight(myTabAnim.c_str(), false);
    int iCornerW, iCornerH;
    getTextureManager()->getCornerSize(myTabAnim.c_str(), iCornerW, iCornerH);

    int iCloseW, iCloseH;
    getTextureManager()->getTextureRealDims(myTabCloseAnim.c_str(), iCloseW, iCloseH);

    int iTotalHorPadding = iCornerW*2;
    const int iTabHorPadding = upToScreen(0.5);
    const int iTextCloseButtonSpacing = upToScreen(2.0);

    SVector2D svPrevOffset(TAB_TOP_OFFSET_X, TAB_TOP_OFFSET_Y);
    int iCurr, iNum = getNumChildren();
    UIElement* pChild;
    for(iCurr = 0; iCurr < iNum; iCurr++)
    {
		pChild = dynamic_cast<UIElement*>(getChild(iCurr));
		if(!pChild)
			ASSERT_CONTINUE;

		STabInfo rTabInfo;
		rTabInfo.myText = pChild->getTitle();
		StringUtils::shortenString(rTabInfo.myText, MAX_TAB_TITLE_CHARS, "...");
#ifdef _DEBUG
		string strTempTabAnimName;
		strTempTabAnimName = "TabCloseFadeAnim - " + rTabInfo.myText;
		rTabInfo.myCloseHoverAnim.setDebugName(strTempTabAnimName.c_str());
#endif

		if(pChild->doesPropertyExist(PropertyWindowBarIcon))
		{
			pChild->getAsString(PropertyWindowBarIcon, rTabInfo.myIcon);

			int iDummyVar;
			CachedSequence *pSeq = getDrawingCache()->getCachedSequence(rTabInfo.myIcon.c_str(), &iDummyVar);

			rTabInfo.myIconsDims.x = getTextureManager()->getWidth(pSeq->getName());
			rTabInfo.myIconsDims.y = getTextureManager()->getHeight(pSeq->getName());
		}

		rTabInfo.myNoCloseButton = pChild->getBoolProp(PropertyDisableClosingAsTab);

		RenderUtils::measureText(getParentWindow(), rTabInfo.myText.c_str(), pcsFont, iFontSize, 0, svTextDims);
		rTabInfo.myRect.w = svTextDims.x + iTotalHorPadding;;
		if(!rTabInfo.myNoCloseButton)
			rTabInfo.myRect.w +=  iTextCloseButtonSpacing + iCloseW;
		else
			rTabInfo.myRect.w += NOCLOSE_BUTTON_RIGHT_PADDING;
		if(rTabInfo.myIcon.length() > 0)
			rTabInfo.myRect.w += TAB_ICON_PADDING_X_LEFT + TAB_ICON_PADDING_X_RIGHT + rTabInfo.myIconsDims.x;
		rTabInfo.myRect.h = fTabHeight;
		rTabInfo.myRect.x = svPrevOffset.x;
		rTabInfo.myRect.y = svPrevOffset.y;

		myTabs.push_back(rTabInfo);

		svPrevOffset.x += rTabInfo.myRect.w + iTabHorPadding;
    }

	// This may need to happen if we're changing the title, and therefore the length,
	// of a tab.
	readjustScroll();
}
/*****************************************************************************/
void UITabWindowElement::postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{

	/*
    SVector2D svPos;
    FLOAT_TYPE fFinalOpac, fLocScale;
    this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
    fFinalOpac *= fOpacity;
    SVector2D svSize;
    this->getBoxSize(svSize);
    svSize *= fLocScale*fScale;
    svPos += svScroll;
    if(this->getParent())
	svPos *= fScale;
    svPos -= svSize*0.5;

	FLOAT_TYPE fTabHeight = getTabRowHeight(myTabAnim.c_str(), false);
	FLOAT_TYPE fSelTabHeight = getTabRowHeight(mySelTabAnim.c_str(), false);
	SVector2D svAdditionalTabSize;

    const char* pcsFont = this->getCachedFont();
    if(!pcsFont || strlen(pcsFont) == 0)
		pcsFont = MAIN_FONT_MED;
    SColor scolText;
    this->getAsColor(PropertyUiObjTextColor, scolText);
	SColor scolSelText;
	if(this->doesPropertyExist(PropertyUiObjTextSelColor))
		this->getAsColor(PropertyUiObjTextSelColor, scolSelText);
	else
		scolSelText = scolText;

    SColor *pShadowColor = NULL;
    SColor scolActualShadowColor;
    if(this->doesPropertyExist(PropertyUiObjTextShadowColor))
    {
		this->getAsColor(PropertyUiObjTextShadowColor, scolActualShadowColor);
		scolActualShadowColor.alpha *= fFinalOpac;
		pShadowColor = &scolActualShadowColor;
    }

    SVector2D svShadowOffset;
    getCachedShadowOffset(svShadowOffset);

	FLOAT_TYPE fCloseButtonOpacity;

    scolText.alpha *= fFinalOpac;
	scolSelText.alpha *= fFinalOpac;

    int iCloseW, iCloseH;
    getTextureManager()->getTextureRealDims(myTabCloseAnim.c_str(), iCloseW, iCloseH);

    int iCornerW, iCornerH;
    getTextureManager()->getCornerSize(myTabAnim.c_str(), iCornerW, iCornerH);

	int iSelCornerW, iSelCornerH;
	getTextureManager()->getCornerSize(mySelTabAnim.c_str(), iSelCornerW, iSelCornerH);

	SVector2D svFinalCornerSize;

	FLOAT_TYPE fCloseHoverValue;

    // Render the tabs
    STabInfo* pInfo;
    int iTab, iNumTabs = myTabs.size();
    UIElement* pChild;
    const char* pcsAnimToUse;

	SColor scolFinalText;
	SRect2D srTabRect, srSelTabRect;
	SVector2D svExtraTabOffset;
	int iSelTabIdx = -1;
	
	FLOAT_TYPE fTotalIconSpace;

    for(iTab = 0; iTab < iNumTabs; iTab++)
    {
		pInfo = &myTabs[iTab];

		pChild = dynamic_cast<UIElement*>(getChild(iTab));
		if(pChild && pChild->getIsVisible())
		{
			// Selected tab
			pcsAnimToUse = mySelTabAnim.c_str();
			svAdditionalTabSize.set(0, fSelTabHeight - fTabHeight);
			svFinalCornerSize.set(iSelCornerW, iSelCornerH);
			svExtraTabOffset.set(SEL_TAB_DRAWING_OFFSET_X, SEL_TAB_DRAWING_OFFSET_Y);
			scolFinalText = scolSelText;
		}
		else
		{
			// Unselected tab
			pcsAnimToUse = myTabAnim.c_str();
			svAdditionalTabSize.set(0, 0);
			svExtraTabOffset.set(0, 0);
			svFinalCornerSize.set(iCornerW, iCornerH);
			scolFinalText = scolText;
		}

		// Tab back
		srTabRect.x = svPos.x + pInfo->myRect.x + pInfo->myRect.w/2.0 - svAdditionalTabSize.x/2.0 + svExtraTabOffset.x;
		srTabRect.y = svPos.y + pInfo->myRect.y + pInfo->myRect.h/2.0 - svAdditionalTabSize.y/2.0 + svExtraTabOffset.y;
		srTabRect.w = pInfo->myRect.w + svAdditionalTabSize.x;
		srTabRect.h = pInfo->myRect.h + svAdditionalTabSize.y;
		getDrawingCache()->addScalableButton(pcsAnimToUse, srTabRect.x, srTabRect.y, srTabRect.w, srTabRect.h, fFinalOpac, svFinalCornerSize.x, svFinalCornerSize.y);

		fCloseButtonOpacity = fFinalOpac;
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
			getDrawingCache()->addSprite(pInfo->myIcon.c_str(), svPos.x + pInfo->myRect.x + iCornerW + TAB_ICON_PADDING_X_LEFT + pInfo->myIconsDims.x/2.0, svPos.y + pInfo->myRect.y + pInfo->myRect.h/2, fFinalOpac, 0, 1.0, 1.0, true);

		// Text
		getDrawingCache()->addText(pInfo->myText.c_str(), pcsFont, svPos.x + pInfo->myRect.x + iCornerW + fTotalIconSpace, svPos.y + pInfo->myRect.y + pInfo->myRect.h/2.0, scolFinalText, HorAlignLeft, VertAlignCenter, 0, NULL, pShadowColor, &svShadowOffset, 0, fScale);

		// Close button
		if(!pInfo->myNoCloseButton)
		{
			fCloseHoverValue = pInfo->myCloseHoverAnim.getValue(GameEngine::getGlobalTime(getClockType()));
			getDrawingCache()->addSprite(myTabCloseAnim.c_str(), svPos.x + pInfo->myRect.x + pInfo->myRect.w - iCornerW - iCloseW/2, svPos.y + pInfo->myRect.y + pInfo->myRect.h/2, fCloseButtonOpacity,0, 1.0, 1.0, true);
			if(fCloseHoverValue > 0.0)
				getDrawingCache()->addSprite(myTabCloseHoverAnim.c_str(), svPos.x + pInfo->myRect.x + pInfo->myRect.w - iCornerW - iCloseW/2, svPos.y + pInfo->myRect.y + pInfo->myRect.h/2, fFinalOpac*fCloseHoverValue, 0, 1.0, 1.0, true);
		}

		// Drag drop mode
		if(myIsDraggingOverTabRow && pChild->getCurrentDragDropMode() != AcceptNone)
			getDrawingCache()->addScalableButton("layerDragSelThin", srTabRect.x, srTabRect.y, srTabRect.w, srTabRect.h, fOpacity);
    }
*/
	STabRenderCommonInfo rCommonInfo;
	computeTabRenderCommonInfo(fScale, svScroll, fOpacity, rCommonInfo);

	// We need the stencil region now...
	SRect2D srOwnRect;
	getGlobalRectangle(srOwnRect);
	RenderUtils::beginScissorRectangle(srOwnRect, getDrawingCache());

	SVector2D svTabScroll(myHorTabScroll, 0);
	SRect2D srSelTabRect;
	STabInfo* pInfo;
	int iSelTabIdx = -1;
	int iTab, iNumTabs = myTabs.size();
	for(iTab = 0; iTab < iNumTabs; iTab++)
	{
		pInfo = &myTabs[iTab];
		UITabRenderer::renderTab(this, pInfo, iTab, rCommonInfo, iSelTabIdx, srSelTabRect, svTabScroll);
	}

	// Now, draw two lines to underline unselected tabs
	//FLOAT_TYPE fYPos = svPos.y + fTabHeight;
	FLOAT_TYPE fYBottomPos = rCommonInfo.myPos.y + rCommonInfo.myTabHeight + upToScreen(0.0);

	if(iNumTabs > 0)
	{
		if(iSelTabIdx > 0)
		{
			myLineVecs[0].clear();
			myLineVecs[0].push_back(SVector2D(rCommonInfo.myPos.x , fYBottomPos));
			myLineVecs[0].push_back(SVector2D(srSelTabRect.x, fYBottomPos));
			getDrawingCache()->addTexturedLine(&myLineVecs[0], "tabHorLine", fOpacity, upToScreen(0.51));
		}

		myLineVecs[1].clear();
		myLineVecs[1].push_back(SVector2D(srSelTabRect.x + srSelTabRect.w, fYBottomPos));
		myLineVecs[1].push_back(SVector2D(rCommonInfo.myPos.x + rCommonInfo.mySize.x, fYBottomPos));
		getDrawingCache()->addTexturedLine(&myLineVecs[1], "tabHorLine", fOpacity, upToScreen(0.51));
	}

	RenderUtils::endScissorRectangle(getDrawingCache());

    UIElement::postRender(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
int UITabWindowElement::getTabAt(FLOAT_TYPE fX, FLOAT_TYPE fY, bool& bOnCloseButton)
{
    SVector2D svPos;
    FLOAT_TYPE fGlobalScale;
    this->getGlobalPosition(svPos, NULL, &fGlobalScale);
    SVector2D svOwnSize;
    this->getBoxSize(svOwnSize);
    svOwnSize *= fGlobalScale;
    svPos -= svOwnSize*0.5;

    int iCloseW, iCloseH;
    getTextureManager()->getTextureRealDims(myTabCloseAnim.c_str(), iCloseW, iCloseH);
    int iCornerW, iCornerH;
    getTextureManager()->getCornerSize(myTabAnim.c_str(), iCornerW, iCornerH);

    bOnCloseButton = false;

    SVector2D svPoint(fX, fY);
    SRect2D srScreenTabRect, srCloseButtonRect;
    STabInfo* pInfo;
    int iTab, iNumTabs = myTabs.size();
    for(iTab = 0; iTab < iNumTabs; iTab++)
    {
		pInfo = &myTabs[iTab];

		getTabRect(iTab, svPos, srScreenTabRect);

/*
		srScreenTabRect.x = svPos.x + pInfo->myRect.x + myHorTabScroll;
		srScreenTabRect.y = svPos.y + pInfo->myRect.y;
		srScreenTabRect.w = pInfo->myRect.w;
		srScreenTabRect.h = pInfo->myRect.h;
*/

		if(srScreenTabRect.doesContain(svPoint))
		{
			srCloseButtonRect.x = svPos.x + pInfo->myRect.x + myHorTabScroll + pInfo->myRect.w - iCornerW - iCloseW; 
			srCloseButtonRect.w = iCloseW;
			srCloseButtonRect.h = iCloseH;
			srCloseButtonRect.y = svPos.y + pInfo->myRect.y + pInfo->myRect.h/2 - srCloseButtonRect.h/2.0;

			if(!pInfo->myNoCloseButton)
				bOnCloseButton = srCloseButtonRect.doesContain(svPoint);

			return iTab;
		}
    }

    return -1;
}
/*****************************************************************************/
void UITabWindowElement::getTabRect(int iTabIndex, const SVector2D& svOwnOrigin, SRect2D& srRectOut)
{
	if(iTabIndex < 0 || iTabIndex >= myTabs.size())
	{
		srRectOut.reset();
		return;
	}

	STabInfo *pInfo = &myTabs[iTabIndex];

	srRectOut.x = svOwnOrigin.x + pInfo->myRect.x + myHorTabScroll;
	srRectOut.y = svOwnOrigin.y + pInfo->myRect.y;
	srRectOut.w = pInfo->myRect.w;
	srRectOut.h = pInfo->myRect.h;
}
/*****************************************************************************/
void UITabWindowElement::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{
    if(vecTouches.size() > 0 && !bIgnoreActions)
    {
		bool bOnClose;
		int iTabIdx = getTabAt(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y, bOnClose);

		// Switch to that tab - or close it:
		if(iTabIdx >= 0)
		{
			_ASSERT(iTabIdx < getNumChildren());
			if(getParentWindow()->getCurrMouseButtonDown() == MouseButtonRight)
			{
				// Open the RMB menu if the tab has any. Don't forget, we're generic here.
				UIElement* pTab = as<UIElement>(getChild(iTabIdx));
				if(pTab)
				{
					if(pTab->doesPropertyExist(PropertyProxyTabRMBMenu))
						getUIPlane()->performUiAction("uiaToggleContextMenuAtMouseLocation", pTab->getStringProp(PropertyProxyTabRMBMenu), g_pcsSideStrings[SideBottom], pTab->getStringProp(PropertyId));
				}
				ELSE_ASSERT;
			}
			else
			{
				if(bOnClose)
				{
					if(!as<UIElement>(getChild(iTabIdx))->closeAsTab())
						closeTab(iTabIdx);
				}
				else
					switchToTab(iTabIdx);
			}
		}
    }

    UIElement::onReleased(vecTouches, bIgnoreActions);
	myTabBeingDragged = -1;
}
/*****************************************************************************/
void UITabWindowElement::closeTab(int iTabIdx, bool bDeleteElement)
{
	if(iTabIdx < 0)
		ASSERT_RETURN;

    // Need dynamic elements.
    int iActiveTab = getActiveTabIndex();
    int iNextTab;
    if(iActiveTab > 0)
		iNextTab = iActiveTab - 1;
    else
    {
		// Note that this actually is the next tab,
		// since the active one will be deleted.
		// In effect, it's the iActive + 1 - 1.
		iNextTab = iActiveTab;
    }

	UIElement *pTabToClose =  dynamic_cast<UIElement*>(this->getChild(iTabIdx));

	// Separate closure event
	if(bDeleteElement)
	{
		EventManager::getInstance()->sendEvent(EventTabClosed, pTabToClose);
		getParentWindow()->onTabClosed(pTabToClose);
		getUIPlane()->deleteElement(pTabToClose);
	}
	else
		this->deleteChild(pTabToClose, false);
    recomputeTabInfos();

    int iNumTabs = getNumChildren();
    if(iNextTab < iNumTabs)
		switchToTab(iNextTab);
	else
	{
		// We only send it here because switch to tab sends its own event...
		EventManager::getInstance()->sendEvent(EventTabChanged, this);
	}

	if(getNumChildren() == 0 && !IS_STRING_EQUAL(getStringProp(PropertyId), "mainImageParentWindow"))
	{
		// We should delete self and parent, as well.
		getUIPlane()->scheduleForDeletion(this);
	}
}
/*****************************************************************************/
int UITabWindowElement::getActiveTabIndex()
{
    int iCurr, iNum = getNumChildren();
    UIElement* pChild;
    for(iCurr = 0; iCurr < iNum; iCurr++)
    {
		pChild = dynamic_cast<UIElement*>(getChild(iCurr));
		if(pChild->getIsVisible())
			return iCurr;
    }

    return -1;
}
/*****************************************************************************/
void UITabWindowElement::switchToTab(int iTabIdx)
{
	if(iTabIdx < 0)
		return;

    int iCurr, iNum = getNumChildren();
    UIElement* pChild;
	bool bMakeVisible;

	// Find who's visible now and send a before tab changed event
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = dynamic_cast<UIElement*>(getChild(iCurr));
		if(!pChild)
			ASSERT_CONTINUE;
		if(pChild->getIsVisible())
		{
			EventManager::getInstance()->sendEvent(EventTabAboutToBeChanged, pChild);
			break;
		}
	}

    for(iCurr = 0; iCurr < iNum; iCurr++)
    {
		pChild = dynamic_cast<UIElement*>(getChild(iCurr));
		if(!pChild)
			ASSERT_CONTINUE;

		bMakeVisible = (iCurr == iTabIdx);
		pChild->setIsVisible(bMakeVisible);
		pChild->updateDataRecursive();
		if(bMakeVisible)
		{
			getParentWindow()->onTabActivated(pChild);
			EventManager::getInstance()->sendEvent(EventTabChanged, this);
		}
    }

	readjustScroll();
}
/*****************************************************************************/
UIElement* UITabWindowElement::addNewTabInternal(bool bSwitchTo, const char* optOverrideTabTemplate)
{
    UIPlane* pUiPlane = getUIPlane();
    const char* pcsTabTemplateType = "__tabTemplate__";
    if(optOverrideTabTemplate)
		pcsTabTemplateType = optOverrideTabTemplate;

    UIElement* pNewChild = pUiPlane->addNewElement(pcsTabTemplateType, this);
    recomputeTabInfos();
    autoLayoutChildrenRecursive();

    if(bSwitchTo)
		switchToTab(getNumChildren() - 1);

    return pNewChild;
}
/*****************************************************************************/
void UITabWindowElement::onCursorGlide(TTouchVector& vecTouches)
{
	bool bOnClose;
	int iTab = getTabAt(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y, bOnClose);	
#ifdef _DEBUG
	if(iTab == 0 && bOnClose)
	{
		int bp = 0;
	}
#endif

	int iNewTab = -1;
	if(iTab >= 0 && bOnClose)
		iNewTab = iTab;

	// We only need to do something if we've entered a new close tab or left previous.
	//if( iTab < 0 || !bOnClose || (bOnClose && iTab != myLastHoverTabCloseButton && myLastHoverTabCloseButton >= 0))
	if(iNewTab != myLastHoverTabCloseButton)
	{
		if(myLastHoverTabCloseButton >= 0 && myLastHoverTabCloseButton < myTabs.size())
			myTabs[myLastHoverTabCloseButton].fadeOutCloseHover();
		if(iNewTab >= 0 && iNewTab < myTabs.size())
			myTabs[iTab].fadeInCloseHover();
	}

	myLastHoverTabCloseButton = iNewTab;
}
/*****************************************************************************/
void UITabWindowElement::onCursorGlideLeave(TTouchVector& vecTouches)
{
	int iTab, iNumTabs = myTabs.size();
	for(iTab = 0; iTab < iNumTabs; iTab++)
	{
		if(myTabs[iTab].myCloseHoverAnim.getValue() > 0.0)
			myTabs[iTab].fadeOutCloseHover();
	}
}
/*****************************************************************************/
void UITabWindowElement::getMinDims(SVector2D& svDimsOut) const
{
	SVector2D svTemp;
	svDimsOut.set(0, 0);

	// Go through all our tabs
	int iCurr, iNum = getNumChildren();
	const UIElement* pChild;

	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = dynamic_cast<const UIElement*>(getChild(iCurr));
		if(!pChild)
			ASSERT_CONTINUE;

		pChild->getMinDims(svTemp);
		if(svTemp.x > 0 && svDimsOut.x < svTemp.x)
			svDimsOut.x = svTemp.x;
		if(svTemp.y > 0 && svDimsOut.y < svTemp.y)
			svDimsOut.y = svTemp.y;
	}
}
/*****************************************************************************/
void UITabWindowElement::getMaxDims(SVector2D& svDimsOut) const
{
	SVector2D svTemp;
	svDimsOut.set(0, 0);

	// Go through all our tabs
	int iCurr, iNum = getNumChildren();
	const UIElement* pChild;

	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = dynamic_cast<const UIElement*>(getChild(iCurr));
		if(!pChild)
			ASSERT_CONTINUE;

		pChild->getMaxDims(svTemp);
		if(svTemp.x > 0 && (svDimsOut.x > svTemp.x || svDimsOut.x == 0))
			svDimsOut.x = svTemp.x;
		if(svTemp.y > 0 && (svDimsOut.y > svTemp.y || svDimsOut.y == 0))
			svDimsOut.y = svTemp.y;
	}
}
/*****************************************************************************/
bool UITabWindowElement::onKeyUp(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl)
{
	bool bIsHandled = false;
	if(
#ifdef WIN32
		bControl 
#else
		bMacActualControl
#endif
		&& iKey == SilentKeyTab && myTabs.size() > 0)
	{
		// See if the cursor is on us.
		SVector2D svMousePos;
		getParentWindow()->getLastMousePos(svMousePos);
		SRect2D srOwnRect;
		this->getGlobalRectangle(srOwnRect);
		if(srOwnRect.doesContain(svMousePos))
		{
			// Switch to next tab
			int iSelTab = getActiveTabIndex();
			if(bShift)
			{
				iSelTab--;
				if(iSelTab < 0)
					iSelTab = myTabs.size() - 1;
			}
			else
			{
				iSelTab++;
				if(iSelTab >= myTabs.size())
					iSelTab = 0;
			}
			switchToTab(iSelTab);
			bIsHandled = true;
		}
	}

	return bIsHandled;
}
/*****************************************************************************/
void UITabWindowElement::onTimerTick(GTIME lGlobalTime)
{
	UIElement::onTimerTick(lGlobalTime);
	int iTab, iNumTabs = myTabs.size();
	for(iTab = 0; iTab < iNumTabs; iTab++)
		myTabs[iTab].myCloseHoverAnim.getValue();
		//myTabs[iTab].myCloseHoverAnim.checkTime(lGlobalTime);
}
/*****************************************************************************
// Can't do this because we also need to change the width of the tab...
void UITabWindowElement::refreshTitleFor(UIElement* pElem)
{
	int iCurr, iNum = getNumChildren();
	UIElement* pChild;
	STabInfo* pInfo;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = dynamic_cast<UIElement*>(getChild(iCurr));
		if(pChild == pElem && iCurr >= 0 && iCurr < myTabs.size())
		{
			pInfo = &myTabs[iCurr];
			pInfo->myText = pChild->getTitle();
			shortenString(pInfo->myText, MAX_TAB_TITLE_CHARS, "...");
			break;
		}
	}
}
/*****************************************************************************/
void UITabWindowElement::onDragDropHover(UIElement* pOther, SVector2D& svScreenPos)
{
	// See which tab we're over:
	bool bOnClose;
	int iTabIdx = getTabAt(svScreenPos.x, svScreenPos.y, bOnClose);
	UIElement* pTargetChild = getChild<UIElement>(iTabIdx);
	if(pTargetChild)
		pTargetChild->onDragDropHover(pOther, svScreenPos);

	// Go through all other children  and tell them to cancel their drag:
	cancelDragDrop(pTargetChild);

	myIsDraggingOverTabRow = pTargetChild != NULL;
}
/*****************************************************************************/
bool UITabWindowElement::receiveDragDrop(UIElement* pOther, SVector2D& svScreenPos)
{
	bool bOnClose;
	int iTabIdx = getTabAt(svScreenPos.x, svScreenPos.y, bOnClose);
	UIElement* pTargetChild = getChild<UIElement>(iTabIdx);
	myIsDraggingOverTabRow = false;
	if(pTargetChild)
		return pTargetChild->receiveDragDrop(pOther, svScreenPos);
	return false;
}
/*****************************************************************************/
void UITabWindowElement::cancelDragDrop(UIElement* pExcept)
{
	if(!getParentWindow()->getDragDropManager()->isDragging())
		return;

	UIElement* pCurrChild;
	int iTab, iNumTabs = myTabs.size();
	SVector2D svFake(-100, -100);
	for(iTab = 0; iTab < iNumTabs; iTab++)
	{
		pCurrChild = getChild<UIElement>(iTab);
		if(pCurrChild != pExcept)
			pCurrChild->onDragDropHover(NULL, svFake);
	}
}
/*****************************************************************************/
void UITabWindowElement::onMouseLeave(TTouchVector& vecTouches)
{
	UIElement::onMouseLeave(vecTouches);
	cancelDragDrop(NULL);
}
/*****************************************************************************/
bool UITabWindowElement::allowDragDropStart(SVector2D& svPos)
{
	bool bOnClose;
	myTabBeingDragged = getTabAt(svPos.x, svPos.y, bOnClose);
	return myTabBeingDragged >= 0;
}
/*****************************************************************************/
void UITabWindowElement::computeTabRenderCommonInfo(FLOAT_TYPE fScale, const SVector2D& svScroll, FLOAT_TYPE fOpacity, STabRenderCommonInfo& rInfoOut)
{
	FLOAT_TYPE fLocScale;
	this->getLocalPosition(rInfoOut.myPos, &rInfoOut.myFinalOpacity, &fLocScale);
	rInfoOut.myFinalOpacity *= fOpacity;
	
	this->getBoxSize(rInfoOut.mySize);
	rInfoOut.mySize *= fLocScale*fScale;
	rInfoOut.myPos += svScroll;
	if(this->getParent())
		rInfoOut.myPos *= fScale;
	rInfoOut.myPos -= rInfoOut.mySize*0.5;

	rInfoOut.myScale = fScale;

	rInfoOut.myTabHeight = getTabRowHeight(myTabAnim.c_str(), false);
	rInfoOut.mySelTabHeight = getTabRowHeight(mySelTabAnim.c_str(), false);

	rInfoOut.pcsFont = this->getCachedFont();
	if(!rInfoOut.pcsFont || strlen(rInfoOut.pcsFont) == 0)
		rInfoOut.pcsFont = MAIN_FONT_MED;
	rInfoOut.iFontSize = this->getCachedFontSize();
// 	if(rInfoOut.iFontSize <= 0)
// 		rInfoOut.iFontSize = MAIN_FONT_MED_DEF_SIZE;
	this->getAsColor(PropertyTextColor, rInfoOut.myTextColor);
	if(this->doesPropertyExist(PropertySelectedTextColor))
		this->getAsColor(PropertySelectedTextColor, rInfoOut.mySelTextColor);
	else
		rInfoOut.mySelTextColor = rInfoOut.myTextColor;

	rInfoOut.myShadowColor = NULL;
	rInfoOut.myActualShadowColor;
	if(this->doesPropertyExist(PropertyTextShadowColor))
	{
		this->getAsColor(PropertyTextShadowColor, rInfoOut.myActualShadowColor);
		rInfoOut.myActualShadowColor.alpha *= rInfoOut.myFinalOpacity;
		rInfoOut.myShadowColor = &rInfoOut.myActualShadowColor;
	}

	getCachedShadowOffset(rInfoOut.myShadowOffset);

	rInfoOut.myTextColor.alpha *= rInfoOut.myFinalOpacity;
	rInfoOut.mySelTextColor.alpha *= rInfoOut.myFinalOpacity;

	getTextureManager()->getTextureRealDims(myTabCloseAnim.c_str(), rInfoOut.myCloseW, rInfoOut.myCloseH);
	getTextureManager()->getCornerSize(myTabAnim.c_str(), rInfoOut.myCornerW, rInfoOut.myCornerH);
	getTextureManager()->getCornerSize(mySelTabAnim.c_str(), rInfoOut.mySelCornerW, rInfoOut.mySelCornerH);
}
/*****************************************************************************/
void UITabWindowElement::readjustScroll()
{
	myHorTabScroll = 0;
	int iSelTab = getActiveTabIndex();

	if(iSelTab < 0)
		return;

	SVector2D svOriginPos;
	FLOAT_TYPE fGlobalScale;
	this->getGlobalPosition(svOriginPos, NULL, &fGlobalScale);
	SVector2D svOwnSize;
	this->getBoxSize(svOwnSize);
	svOwnSize *= fGlobalScale;
	svOriginPos -= svOwnSize*0.5;

	SRect2D srSelTabRect;
	getTabRect(iSelTab, svOriginPos, srSelTabRect);

	// If the tab is not within our rect, set it as last visible:
	FLOAT_TYPE fHorEnd = svOriginPos.x + svOwnSize.x;
	FLOAT_TYPE fSelTabEnd = srSelTabRect.farCorner().x;

	myHorTabScroll = fSelTabEnd - fHorEnd;
	if(myHorTabScroll < 0)
		myHorTabScroll = 0;
	else
		myHorTabScroll *= -1.0;
}
/*****************************************************************************/
bool UITabWindowElement::adoptTab(UITabWindowElement* pOtherTabWindow, int iTabIndex)
{
	if(this == pOtherTabWindow)
		return false;
	if(!pOtherTabWindow || iTabIndex < 0)
		return false;
	int iNumOtherTabs = pOtherTabWindow->getNumChildren();
	if(iTabIndex >= iNumOtherTabs)
		return false;

	// Otherwise, get the child:
	UIElement* pElem = pOtherTabWindow->getChild<UIElement>(iTabIndex);
	pOtherTabWindow->closeTab(iTabIndex, false);
	this->addNewTab(pElem, true);

	// Now, if the new tab has a min dims thing on it, we need to invalidate
	// the parent tab child layout to make sure it's adjusted.
	if(pElem->doesPropertyExist(PropertyMinWidth) || pElem->doesPropertyExist(PropertyMinHeight)
		|| pElem->doesPropertyExist(PropertyMaxWidth) || pElem->doesPropertyExist(PropertyMaxHeight))
		readjustClosestSplitterParent();

	readjustScroll();

	return true;
}
/*****************************************************************************/
void UITabWindowElement::addNewTab(UIElement* pExistingElem, bool bSwitchTo)
{
	// Add as child:
	this->parentExistingNode(pExistingElem);
	invalidateSizingInfos();

	recomputeTabInfos();
	autoLayoutChildrenRecursive();

	if(bSwitchTo)
		switchToTab(getNumChildren() - 1);
}
/*****************************************************************************/
bool UITabWindowElement::getDoHaveToolTip(const SVector2D& svScreenCoords) const
{
	// See if we're over a tab, and ask it:
	bool bIsOnClose;
	int iTabIdx = const_cast<UITabWindowElement*>(this)->getTabAt(svScreenCoords.x, svScreenCoords.y, bIsOnClose);

	if(iTabIdx < 0)
		return false;
	else if(bIsOnClose)
		return true;
	else
	{
		const UIElement* pChild = getChild<const UIElement>(iTabIdx);
		if(pChild)
			return pChild->getDoHaveToolTip(svScreenCoords);
	}

	return false;
}
/*****************************************************************************/
void UITabWindowElement::getToolTip(const SVector2D& svScreenCoords, string& strTipOut) const
{
	bool bIsOnClose;
	int iTabIdx = const_cast<UITabWindowElement*>(this)->getTabAt(svScreenCoords.x, svScreenCoords.y, bIsOnClose);

	if(iTabIdx < 0)
	{ ASSERT_RETURN; }
	else if(bIsOnClose)
		strTipOut = "Close this tab";
	else
	{
		const UIElement* pChild = getChild<const UIElement>(iTabIdx);
		if(pChild)
			pChild->getToolTip(svScreenCoords, strTipOut);
	}
}
/*****************************************************************************/
void UITabWindowElement::onDoubleClick(TTouchVector& vecTouches, bool bIgnoreActions)
{
	// Check to see if we're over a tab:
	if(vecTouches.size() > 0 && !bIgnoreActions)
	{
		bool bIsOverClose;
		if(getTabAt(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y, bIsOverClose) >= 0)
			bIgnoreActions = true;
	}

	UIElement::onDoubleClick(vecTouches, bIgnoreActions);
}
/*****************************************************************************/
void UITabWindowElement::readjustClosestSplitterParent()
{
	UISplitterElement *pFirstSplitterParent = getParentOfType<UISplitterElement>();
	if(!pFirstSplitterParent)
		return;
	pFirstSplitterParent->autoLayoutChildrenRecursive();
}
/*****************************************************************************/
};