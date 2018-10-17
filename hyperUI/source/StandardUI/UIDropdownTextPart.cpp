#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UIDropdownTextPart::UIDropdownTextPart(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{

}
/*****************************************************************************/
void UIDropdownTextPart::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);

}
/*****************************************************************************/
void UIDropdownTextPart::onLostFocus(bool bHasCancelled)
{
	UIElement* pTextSelElem = this->getChildById("defTextPartSel", true, true);
	pTextSelElem->setIsVisible(false);
}
/*****************************************************************************/
void UIDropdownTextPart::onGainedFocus()
{
	UIElement* pTextSelElem = this->getChildById("defTextPartSel", true, true);
	pTextSelElem->setIsVisible(true);

	// Also show dropdown
// 	UIDropdown* pParent = this->getParentOfType<UIDropdown>();
// 	theCommonString3 = DROPDOWN_TOGGLE_ACTION;
// 	pParent->handleAction(theCommonString3, NULL);
}
/*****************************************************************************/
bool UIDropdownTextPart::onKeyUp(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl)
{
	bool bUpdateElement = false;
	UIDropdown* pParent = this->getParentOfType<UIDropdown>();
	if(iKey == SilentKeyUpArrow)
	{
		AutoUndoBlock rUpDownUndo(pParent->getUndoStringForSelfChange(), pParent->createUndoItemForSelfChange(), getParentWindow(), NULL);
		pParent->adjustSelection(-1);
		bUpdateElement = true;
	}
	else if(iKey == SilentKeyDownArrow)
	{
		AutoUndoBlock rUpDownUndo(pParent->getUndoStringForSelfChange(), pParent->createUndoItemForSelfChange(), getParentWindow(), NULL);
		pParent->adjustSelection(1);
		bUpdateElement = true;
	}

	if(bUpdateElement)
		getParentOfType<UIDropdown>()->handleTargetElementUpdate(false);

	return bUpdateElement;
}
/*****************************************************************************/
bool UIDropdownTextPart::onTextPreRender(string& strText, SColor& scolText, int &iCursorPosOut)
{
	// This might be slow...
	iCursorPosOut = -1;
	UIElement* pSubmenuElem = this->getParent<UIElement>()->getChildById("__dropdownTemplate__", true, true);
	if(pSubmenuElem->getIsBeingShown() || pSubmenuElem->getIsFullyShown())
	{
		// The dropdown menu is shown
		scolText.alpha *= 0.45;
		return false;
	}
	else
	{
		// The dropdown menu is hidden
		// See if we're selected
		UIElement* pTextSelElem = this->getChildById("defTextPartSel", true, true);
		// Don't render the shadow if we're selected.
		return !pTextSelElem->getIsVisible();
	}


	return true;
}
/*****************************************************************************/
};