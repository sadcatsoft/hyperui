#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UIDropdownMenu::UIDropdownMenu(UIPlane* pParentPlane)
	: UIMenuElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
UIDropdownMenu::~UIDropdownMenu()
{
	onDeallocated();
}
/*****************************************************************************/
void UIDropdownMenu::onAllocated(IBaseObject* pData)
{
	UIMenuElement::onAllocated(pData);
	myIsShowingScrollbar = false;
}
/*****************************************************************************/
void UIDropdownMenu::onDeallocated(void)
{
	UIMenuElement::onDeallocated();
}
/*****************************************************************************/
void UIDropdownMenu::customMenuItemAction(const char* pcsItemId) 
{ 
	// Since we leave the top entry as none, clicking the close button
	// may result in this being selected. We check and quit that.
	if(!IS_VALID_STRING_AND_NOT_NONE(pcsItemId))
		return;

	UIDropdown* pParent = getParent<UIDropdown>();
	AutoUndoBlock rDropdownUndo(pParent->getUndoStringForSelfChange(), pParent->createUndoItemForSelfChange(), getParentWindow(), NULL);
	pParent->selectItem(pcsItemId, true);
	pParent->handleTargetElementUpdate(false);
}
/*****************************************************************************/
void UIDropdownMenu::getAutoScrollingRectangleSizeAndShiftOffsets(SVector2D& svSizeOffsetOut, SVector2D& svShiftOffsetOut)
{
	svSizeOffsetOut.set(0, -(DROPDOWN_MENU_SIZE_OFFSET + upToScreen(2.0) ));
	svShiftOffsetOut.set(0, (DROPDOWN_MENU_SIZE_OFFSET));
}
/*****************************************************************************/
void UIDropdownMenu::getSelectionClickExclusionRectangle(SRect2D& srOut)
{
	UIDropdown* pParent = getParent<UIDropdown>();
	UIElement *pTextBox = pParent->getChildById("defTextPart");
	pTextBox->getGlobalRectangle(srOut);
}
/*****************************************************************************/
};