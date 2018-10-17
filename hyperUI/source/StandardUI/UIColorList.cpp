#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UIColorList::UIColorList(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIColorList::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
}
/*****************************************************************************/
void UIColorList::recreateColorUI()
{
	UITableElement* pTable = this->getChildById<UITableElement>("defColorListTable", true, true);
	pTable->markDirty();
}
/*****************************************************************************/
bool UIColorList::isCellSelected(UITableCellElement* pCell)
{
	const SColorListEntry* pEntry = myColorList.getEntry(pCell->getRow());
	return pEntry ? pEntry->myIsSelected : false;
}
/*****************************************************************************/
void UIColorList::updateCell(UITableCellElement* pCell)
{
	int iRow = pCell->getRow();
	const SColorListEntry* pEntry = myColorList.getEntry(iRow);
	if(!pEntry)
		ASSERT_RETURN;

	// Otherwise, refresh all the cell's elems

	// Enabled status
	UIButtonElement* pEnabledCheck = pCell->getChildById<UIButtonElement>("defEnabled", true, true);
	pEnabledCheck->setIsPushed(pEntry->myIsEnabled);
	pEnabledCheck->setNumProp(PropertyActionValue, iRow);

	// Text
	StringUtils::numberToNiceString(iRow + 1, theCommonString);
	theCommonString = "Color #" + theCommonString;
	pCell->setTextForChild("defText", theCommonString.c_str());

	// Color
	UIColorSwatch* pColor = pCell->getChildById<UIColorSwatch>("defColor", true, true);
	pColor->setColor(pEntry->myColor);
	pColor->setColorCallback(this);
	pColor->setNumProp(PropertyActionValue, iRow);

	// And row id for delete button
	UIButtonElement* pDelButton = pCell->getChildById<UIButtonElement>("defRemoveButton", true, true);
	pDelButton->setNumProp(PropertyActionValue, iRow);
	//pDelButton->setNumProp(PropertyUiObjTargetDataSource, iRow);
}
/*****************************************************************************/
int UIColorList::getNumRows(UIElement* pCaller)
{
	return myColorList.getNumEntries();
}
/*****************************************************************************/
bool UIColorList::handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData)
{
	bool bUpdateAll = false;
	bool bDidHandle = false;
	if(strAction == "uiaColorListToggleEnableStatus")
	{
		int iRow = pSourceElem->getNumProp(PropertyActionValue);
		UIButtonElement* pButton = as<UIButtonElement>(pSourceElem);
		myColorList.getEntry(iRow)->myIsEnabled = pButton ? pButton->getIsPushed() : false;
		bUpdateAll = true;
		bDidHandle = true;
	}
	else if(strAction == "uiaColorListDeleteColor")
	{
		int iRow = pSourceElem->getNumProp(PropertyActionValue);
		myColorList.deleteColor(iRow);
		bUpdateAll = true;
		bDidHandle = true;
	}

	if(bUpdateAll)
	{
		recreateColorUI();
		handleTargetElementUpdate(false);
	}

	return bDidHandle;
}
/*****************************************************************************/
void UIColorList::onColorChanged(const SColor& scolNewColor, UIColorSwatch* pElem)
{
	int iRow = pElem->getNumProp(PropertyActionValue);
	myColorList.getEntry(iRow)->myColor = scolNewColor;
	recreateColorUI();
	handleTargetElementUpdate(false);
}
/*****************************************************************************/
void UIColorList::updateOwnData(SUpdateInfo& rRefreshInfo)
{
	if(getHaveValidTargetElement())
		getTargetIdentifier()->refreshUIFromStoredValue(this);
}
/*****************************************************************************/
};