#include "stdafx.h"

namespace HyperUI
{
TUIElementVector UIDropdown::theUiVElems;
/*****************************************************************************/
UIDropdown::UIDropdown(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{

}
/*****************************************************************************/
void UIDropdown::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
	mySelectedIndex = 0;
	myIsInRefresh = false;
}
/*****************************************************************************/
void UIDropdown::render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	UIElement::render(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
const char* UIDropdown::getSelectedId()
{
	int iActualMenuListOffset = mySelectedIndex + 1;
	if(iActualMenuListOffset >= this->getEnumPropCount(PropertyMenuIds) || iActualMenuListOffset < 0)
		return "";
	else
		return this->getEnumPropValue(PropertyMenuIds, mySelectedIndex + 1);
}
/*****************************************************************************/
const char* UIDropdown::getSelectedLabel()
{
	int iActualMenuListOffset = mySelectedIndex + 1;
    if(iActualMenuListOffset >= this->getEnumPropCount(PropertyMenuLabels) || iActualMenuListOffset < 0)
        return "";
    else
        return this->getEnumPropValue(PropertyMenuLabels, mySelectedIndex + 1);
}
/*****************************************************************************/
int UIDropdown::getNumChoices()
{
	return this->getEnumPropCount(PropertyMenuIds);
}
/*****************************************************************************/
void UIDropdown::postInit(void)
{
	UIElement::postInit();	 
	updateSelectionText();
	myMinimzedHeight = this->getNumProp(PropertyHeight);

	// Set the callback on the child
	resetCallbackName();
}
/*****************************************************************************/
void UIDropdown::updateSelectionText()
{
	UIElement* pTextElem = this->getChildById("defTextPart", true, true);
	if(pTextElem)
		pTextElem->setText(getSelectedLabel());
}
/*****************************************************************************/
void UIDropdown::adjustSelection(int iDir)
{
	if(!getIsEnabled())
		return;

	int iNumChoices = getNumChoices();
	// See below for why we subtract 1 here.
	iNumChoices--;

	mySelectedIndex += iDir;
	// Note that -2 is because while we operate on a 0-based basis,
	// the first element is actually a blank for selection displya.
	// Thus, our valid range here is actually [0, iNumChoices-2], since
	// all the get fucntions add 1 to this value.
	if(mySelectedIndex < 0)
		mySelectedIndex = iNumChoices - 1;
	else if(mySelectedIndex >= iNumChoices)
		mySelectedIndex = 0;

	updateSelectionText();

	theCommonString = UIA_DROPDOWN_ITEM_SELECTED;
	handleActionUpParentChain(theCommonString, false);
}
/*****************************************************************************/
bool UIDropdown::handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData)
{
	bool bRes = false;
	if(strAction == DROPDOWN_TOGGLE_ACTION)
	{
		getParentWindow()->setFocusElement(NULL);

		// Hide all shown menus
		UIElement* pSubmenuElem = this->getChildById("__dropdownTemplate__", true, true);
		UIElement* pSubmenuSelText = this->getChildById("__defDDText__", true, true);
		UIElement *pDDButton = this->getChildById("defButton", true, true);
		UISliderElement *pScrollBar = this->getChildById<UISliderElement>("__dropdownTemplateScrollbar__", true, true);
		theUiVElems.resize(2);
		theUiVElems[0] = pSubmenuElem;
		theUiVElems[1] = pSubmenuSelText;
		getUIPlane()->hideAllWithTag("submenu", &theUiVElems, true, getUIPlane(), AnimOverActionGenericCallback);
		getUIPlane()->setHideAllMenus(false);

		// Show us
		if(pSubmenuElem->getIsBeingShown() || pSubmenuElem->getIsFullyShown())
		{
			pSubmenuElem->hide();
			onDropdownHidden();
		}
		else
		{
			SVector2D svOwnSize;
			getBoxSize(svOwnSize);
			SVector2D svPos(-svOwnSize.x/2.0, -svOwnSize.y/2.0);

			// See if we have our show side
			SideType eSideType = SideBottom;
			if(this->doesPropertyExist(PropertyPopupPosition))
				eSideType = mapStringToType<SideType>(this->getStringProp(PropertyPopupPosition), g_pcsSideStrings, SideBottom);

			AlignType eAlignType = AlignNear;
			if(this->doesPropertyExist(PropertyPopupAlign))
				eAlignType = mapStringToType<AlignType>(this->getStringProp(PropertyPopupAlign), g_pcsGenericAlignStrings, AlignNear);

			UIDropdownMenu* pAsSubmenu = as<UIDropdownMenu>(pSubmenuElem);

			bool bIsImageDropDown = this->getIsImageDropDown();
			myDefaultProvider.setSourceItem(this);

			// Our first entry is reserved for the current selection text. Set it.
			if(pSubmenuSelText)
				pSubmenuSelText->setText(this->getSelectedLabel());

			bool bIsShowingScrollbar = false;
			FLOAT_TYPE fFullSize = 0;
			FLOAT_TYPE fMenuMaxSize = 0.0;
			if(this->doesPropertyExist(PropertyMaxDropdownHeight))
				fMenuMaxSize = this->getNumProp(PropertyMaxDropdownHeight);
			if(fMenuMaxSize <= 0.0)
				fMenuMaxSize = upToScreen(200);

			if(pAsSubmenu)
			{
				fFullSize = pAsSubmenu->initMenuFrom(&myDefaultProvider, svOwnSize.x, fMenuMaxSize, svOwnSize.x - upToScreen(18));
				bIsShowingScrollbar = fFullSize > fMenuMaxSize;
				pAsSubmenu->setIsShowingScrollbar(bIsShowingScrollbar);
				pAsSubmenu->refreshMenuItemEnabledStatus(true);
			}

			SVector2D svMenuSize;
			pSubmenuElem->getBoxSize(svMenuSize);
			if(eAlignType == AlignNear)
				svPos.x += svMenuSize.x/2.0;
			else if(eAlignType == AlignFar)
				svPos.x -= svMenuSize.x/2.0;

			if(eSideType == SideTop)
				svPos.y -= (svMenuSize.y/2.0 + upToScreen(1.5));
			else
				svPos.y += svMenuSize.y/2.0 + upToScreen(1.5);

			SVector2D svImageDDPosition;
			if(bIsImageDropDown)
			{
				if(eAlignType == AlignNear)
					svImageDDPosition.x = -svOwnSize.x/2.0 + svMenuSize.x/2.0;
				else if(eAlignType == AlignFar)
					svImageDDPosition.x = svOwnSize.x/2.0 - svMenuSize.x/2.0;

				if(eSideType == SideTop)
					svImageDDPosition.y = -svOwnSize.y/2.0 - svMenuSize.y/2.0;
				else
					svImageDDPosition.y = svOwnSize.y/2.0 + svMenuSize.y/2.0;
				pSubmenuElem->setNumProp(PropertyX, svImageDDPosition.x);
				pSubmenuElem->setNumProp(PropertyY, svImageDDPosition.y);
			}

			pDDButton->setDoPostponeRendering(true);

			UIElement *pSecondButtonBack = this->getChildById("defImageButtonBack", true, true);
			if(pSecondButtonBack)
				pSecondButtonBack->setDoPostponeRendering(true);

			if(pSourceElem != pDDButton && as<UIButtonElement>(pDDButton))
				as<UIButtonElement>(pDDButton)->setIsPushed(true);

			if(pAsSubmenu)
				pAsSubmenu->setMenuPosition(svPos);
			pSubmenuElem->setIsVisible(true);
			pSubmenuElem->show(false, NULL, AnimOverActionNone, true);
			if(pSubmenuSelText)
				pSubmenuSelText->setIsVisible(true);

			// Set the scroll bar
			if(bIsImageDropDown)
			{
				//pScrollBar->setNumProp(PropertyUiObjOnPosX, svPos.x + svImageDDPosition.x/2.0 - pScrollBar->getNumProp(PropertyObjSizeW)/2.0 + upToScreen(1.5));
				pScrollBar->setNumProp(PropertyX, svMenuSize.x/2.0 - pScrollBar->getNumProp(PropertyWidth)/2.0 - upToScreen(1.0));
				pScrollBar->setNumProp(PropertyY, upToScreen(0.5));
				pScrollBar->setNumProp(PropertyHeight, svMenuSize.y - upToScreen(1.5));
			}
			else
			{
				pScrollBar->setNumProp(PropertyX, svPos.x + svMenuSize.x/2.0 - pScrollBar->getNumProp(PropertyWidth)/2.0 - upToScreen(2.0));
				pScrollBar->setNumProp(PropertyY, svPos.y + DROPDOWN_MENU_SIZE_OFFSET/2.0 - upToScreen(1.0));
				pScrollBar->setNumProp(PropertyHeight, svMenuSize.y - DROPDOWN_MENU_SIZE_OFFSET);
			}
			pScrollBar->resetEvalCache(false);
			pScrollBar->setIsVisible(bIsShowingScrollbar || bIsImageDropDown);
			// Make sure it is always enabled since we tend to disable it in 
			// UIElement::onSizeChangedInternal()
			pScrollBar->setIsEnabled(true);

			FLOAT_TYPE fMaxScroll = 0;
			if(bIsShowingScrollbar)
			{
				pScrollBar->setMinValue(0);
				fMaxScroll = fFullSize - fMenuMaxSize;
				pScrollBar->setMaxValue(fMaxScroll);
			}

			// If we have a selected item, make sure it is in the middle somewhere:
			int iSelEntry = 0;
			if(pAsSubmenu)
			{
				iSelEntry = pAsSubmenu->getEntryById(getSelectedId());
				pAsSubmenu->setPrevSelectedEntry(iSelEntry);
			}

			pScrollBar->setValue(0);
			pSubmenuElem->setExtraScrollOffset(0, 0, false);

			this->setNumProp(PropertyBoxOffsetY, svMenuSize.y/2.0 - myMinimzedHeight/2.0);
			this->setNumProp(PropertyHeight, svMenuSize.y);

			if(bIsImageDropDown)
			{
				if(getBoolProp(PropertyDeselectItemOnOpen))
					selectItem(NULL);
				else
					as<UIImageDropdown>(this)->scrollToItem(getSelectedId());
			}
			else if(pAsSubmenu)
			{
				// Scroll to our selected item
				const SMenuEntry* pSelEntry = pAsSubmenu->getEntry(iSelEntry);
				if(pSelEntry)
				{
					const SMenuEntry* pFirstEmptyEntry = pAsSubmenu->getEntry(0);
					FLOAT_TYPE fScrollOffset = pSelEntry->myPosOffset.y - pFirstEmptyEntry->myPosOffset.y - svMenuSize.y/2.0 + pFirstEmptyEntry->myTotalSize.y;
					if(fScrollOffset > 0 && fScrollOffset < fMaxScroll)
					{
						pSubmenuElem->setExtraScrollOffset(0, -fScrollOffset, false);
						pScrollBar->setValue(fScrollOffset);
					}
				}
			}

			this->resetEvalCache(true);
		}
		bRes = true;
	}

	return bRes;
}
/*****************************************************************************/
void UIDropdown::onDropdownHidden()
{
	this->setNumProp(PropertyBoxOffsetY, 0);
	this->setNumProp(PropertyHeight, myMinimzedHeight);
	this->resetEvalCache(true);

	UIButtonElement* pButton = this->getChildById<UIButtonElement>("defButton", true, true);
	if(pButton->getIsPushed())
		pButton->setIsPushed(false);

	UIButtonElement* pSecodButton = this->getChildById<UIButtonElement>("defImageButton", true, true);
	if(pSecodButton && pSecodButton->getIsPushed())
		pSecodButton->setIsPushed(false);


	UIElement* pSubmenuSelText = this->getChildById("__defDDText__", true, true);
	if(pSubmenuSelText)
		pSubmenuSelText->setIsVisible(false);

	UIElement *pDDButton = this->getChildById("defButton", true, true);
	pDDButton->setDoPostponeRendering(false);

	UIElement *pSecondButtonBack = this->getChildById("defImageButtonBack", true, true);
	if(pSecondButtonBack)
		pSecondButtonBack->setDoPostponeRendering(false);
	

	setIsVisibleForChild("__dropdownTemplateScrollbar__", false);
}
/*****************************************************************************/
void UIDropdown::selectItem(const char* pcsItemId, bool bSendAction)
{
	// Find the new item in our list
	int iNewIndex = this->findStringInEnumProp(PropertyMenuIds, pcsItemId);
	// If not found, try refreshing us. This is needed if one dropdown depends
	// on another parm (such as the font family on the font) - by the time we
	// do get around to setting the right font variant here, which is stored as
	// a parm, we have refreshed with the families of the first font listed.
	// So we return -1 and do nothing - but if we refresh and try again, we
	// should work.
	if(iNewIndex < 0 && !myIsInRefresh)
	{
		SUpdateInfo rInfo;
		updateOwnData(rInfo);
		iNewIndex = this->findStringInEnumProp(PropertyMenuIds, pcsItemId);
	}
	if(iNewIndex >= 1)
		mySelectedIndex = iNewIndex - 1;

	if(bSendAction)
	{
		theCommonString = UIA_DROPDOWN_ITEM_SELECTED_BEFORE_TEXT_UPDATE;
		handleActionUpParentChain(theCommonString, false);
	}

	updateSelectionText();

	if(bSendAction)
	{
		theCommonString = UIA_DROPDOWN_ITEM_SELECTED;
		handleActionUpParentChain(theCommonString, false);
	}
}
/*****************************************************************************
void UIDropdown::handleTargetElementUpdate() 
{
	Node* pNode;
	StringResourceProperty* pParm = getTargetParm(pNode);
	if(!pParm)
		return;

	pParm->setString(getSelectedId());
	pNode->markCacheDirty();
	pNode->onParmChanged(pParm->getPropertyName());
}
/*****************************************************************************/
void UIDropdown::preRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	getDrawingCache()->flush();
	UIElement::preRender(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
void UIDropdown::postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	UIElement::postRender(svScroll, fOpacity, fScale);
	getDrawingCache()->flush();
}
/*****************************************************************************/
void UIDropdown::clearContents()
{
	if(this->doesPropertyExist(PropertyMenuIds))
		this->removeProperty(PropertyMenuIds);
	if(this->doesPropertyExist(PropertyMenuLabels))
		this->removeProperty(PropertyMenuLabels);
	if(this->doesPropertyExist(PropertyMenuIsCheckmark))
		this->removeProperty(PropertyMenuIsCheckmark);

	this->addToEnumProp(PropertyMenuIds, PROPERTY_NONE);
	this->addToEnumProp(PropertyMenuLabels, PROPERTY_NONE);
	this->addToEnumProp(PropertyMenuIsCheckmark, PROPERTY_NONE);
}
/*****************************************************************************/
void UIDropdown::addItem(const char* pcsId, const char* pcsLabel, bool bIsCheckmark)
{
	this->addToEnumProp(PropertyMenuIds, pcsId);
	this->addToEnumProp(PropertyMenuLabels, pcsLabel);
	this->addToEnumProp(PropertyMenuIsCheckmark, bIsCheckmark ? TRUE_VALUE : FALSE_VALUE);
}
/*****************************************************************************/
void UIDropdown::updateOwnData(SUpdateInfo& rRefreshInfo)
{
	// See if we have a target element, and if so, set our value from it.
	myIsInRefresh = true;
	if(getHaveValidTargetElement() && getTargetIdentifier())
		getTargetIdentifier()->refreshUIFromStoredValue(this);
	myIsInRefresh = false;
}
/*****************************************************************************/
void UIDropdown::initFromItemChildren(StringResourceItem* pItem, const char* pcsLabelProp, const char* pcsIdProp)
{
	clearContents();

	if(!pItem)
		return;

	string strTempId, strTempLabel;
	StringResourceItem* pChild;
	int iCurr, iNumChildren = pItem->getNumChildren();
	for(iCurr = 0; iCurr < iNumChildren; iCurr++)
	{
		pChild = pItem->getChild(iCurr);
		pChild->getAsString(pcsIdProp, strTempId);
		StringUtils::trim(strTempId, " ");
		pChild->getAsString(pcsLabelProp, strTempLabel);
		StringUtils::trim(strTempLabel, " ");
		addItem(strTempId.c_str(), strTempLabel.c_str());
	}
}
/*****************************************************************************/
void UIDropdown::setFromVectors(const TStringVector& vecIds, const TStringVector& vecTitles)
{
	clearContents();
	int iCurr, iNum = vecIds.size();
	_ASSERT(iNum == vecTitles.size());
	for(iCurr = 0; iCurr < iNum; iCurr++)
		addItem(vecIds[iCurr].c_str(), vecTitles[iCurr].c_str());
}
/*****************************************************************************/
void UIDropdown::onIdChanged()
{
	resetCallbackName();
}
/*****************************************************************************/
const char* UIDropdown::findIdByLabel(const char* pcsLabelPart)
{
	const char* pcsLabel;
	int iCurr, iNum = this->getEnumPropCount(PropertyMenuLabels);
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pcsLabel = this->getEnumPropValue(PropertyMenuLabels, iCurr);
		if(strstr(pcsLabel, pcsLabelPart) != NULL)
			return this->getEnumPropValue(PropertyMenuIds, iCurr);
	}

	return "";
}
/*****************************************************************************/
void UIDropdown::adjustToScreen()
{
	// Make sure we're closed, otherwise if we're in a vertical layout we
	// will report the wrong size.
	UIElement* pSubmenuElem = this->getChildById("__dropdownTemplate__", true, true);
	if(pSubmenuElem->getIsBeingShown() || pSubmenuElem->getIsFullyShown())
	{
		pSubmenuElem->hide();
		onDropdownHidden();
	}
	UIElement::adjustToScreen();
}
/*****************************************************************************/
void UIDropdown::resetCallbackName()
{
	// Set the callback on the child
	UIElement* pSubmenuElem = this->getChildById("__dropdownTemplate__", true, true);
	if(pSubmenuElem)
		pSubmenuElem->setStateAnimCallbackData(this->getStringProp(PropertyId));
}
/*****************************************************************************/
};