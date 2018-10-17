#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UITableCellElement::UITableCellElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UITableCellElement::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
	myTotalCountIndex = myCol = myRow = -1;
	myIsSelected = false;
}
/*****************************************************************************/
void UITableCellElement::setRowCol(int iCol, int iRow, int iTotalCountIndex)
{
	myCol = iCol;
	myRow = iRow;
	myTotalCountIndex = iTotalCountIndex;
}
/*****************************************************************************/
void UITableCellElement::setIsSelected(bool bIsSelected, bool bIsComingFromUI)
{
	myIsSelected = bIsSelected;
	UITableElement* pParentTable = dynamic_cast<UITableElement*>(this->getParent());
	if(pParentTable)
	{
		if(this->doesPropertyExist(PropertyCellSelectionElem))
		{
			UIElement *pSelectionElem = this->getElementAtPath(this->getStringProp(PropertyCellSelectionElem));
			if(pSelectionElem)
				pSelectionElem->setIsVisible(bIsSelected);
			ELSE_ASSERT;
		}

		if(bIsSelected)
			pParentTable->onCellSelected(this);
		else
			pParentTable->onCellDeselected(this);

		if(bIsSelected)
			theSharedString = UIA_TABLE_CELL_SELECTED;
		else
			theSharedString = UIA_TABLE_CELL_DESELECTED;
		ResourceItem rItem;
		rItem.setNumProp(PropertyColumn, this->getCol());
		rItem.setNumProp(PropertyRow, this->getRow());
		rItem.setBoolProp(PropertyIsComingFromUI, bIsComingFromUI);
		pParentTable->handleActionUpParentChain(theSharedString, false, &rItem);
	}
	ELSE_ASSERT;
}
/*****************************************************************************/
bool UITableCellElement::getIsSelected(void)
{
	// A table is selected if its row is selected, or if it itself
	// is selected.
	if(myIsSelected)
		return true;

	UITableElement* pParentTable = dynamic_cast<UITableElement*>(this->getParent());
	if(pParentTable && pParentTable->getIsRowSelected(myRow))
		return true;

	return false;
}
/*****************************************************************************/
int UITableCellElement::getCol(void)
{
	return myCol;
}
/*****************************************************************************/
int UITableCellElement::getRow(void)
{	
	return myRow;
}
/*****************************************************************************/
UIElement* UITableCellElement::getChildAtRecursive(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bFirstNonPassthrough, bool bGlidingOnly)
{
	// Just pass the coords since we expect everything to be in screen space.
	// We take of it ourselves in the cell positioning code.
	SVector2D svPoint(fX, fY), svScroll;
	return this->getChildAtRecursiveInternal(svPoint, svScroll, bFirstNonPassthrough, false, bGlidingOnly);
}
/*****************************************************************************/
UIElement* UITableCellElement::getChildAtRecursiveInternal(const SVector2D& svPoint, const SVector2D& svScroll, bool bFirstNonPassthrough, bool bIgnoreChildren, bool bGlidingOnly)
{
	UITableElement* pParentTable = dynamic_cast<UITableElement*>(this->getParent());
	if(!pParentTable)
	{
		// A cell should always be parented to a table. But I bet I'll return
		// here one day to remove this assert for a brutal hack.
		_ASSERT(0);
		return NULL;
	}

	SRect2D srRect;
	pParentTable->getScreenCellPos(myCol, myRow, srRect);

	if(!srRect.doesContain(svPoint))
		return NULL;
	else if(bIgnoreChildren)
	{
		if(this->getBoolProp(PropertyIsPassthrough))
			return NULL;
		else
			return this;

	}

	SVector2D svOffset;
	svOffset.x = srRect.x + srRect.w/2.0;
	svOffset.y = srRect.y + srRect.h/2.0;
	UIElement* pFoundSubchild = NULL;
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	// Iterate over children backwards so that the later ones are on top.
	//for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	for(iCurrChild = iNumChildren - 1; iCurrChild >= 0; iCurrChild--)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));

		if(!pChild->getIsVisible() || !pChild->getIsEnabled())
			continue;

		if(bFirstNonPassthrough)
			pFoundSubchild = pChild->getChildAtRecursiveInternal(svPoint, svOffset, false, true, bGlidingOnly);
		else
			pFoundSubchild = pChild->getChildAtRecursiveInternal(svPoint, svOffset, false, false, bGlidingOnly);
		if(pFoundSubchild)
			break;
	}

	if(pFoundSubchild)
		return pFoundSubchild;
	else
	{
		if(this->getBoolProp(PropertyIsPassthrough))
			return NULL;
		else
			return this;
	}

}
/*****************************************************************************/
void UITableCellElement::getGlobalPosition(SVector2D& svOut, FLOAT_TYPE* fOpacityOut, FLOAT_TYPE* fScaleOut)
{
	UITableElement* pParentTable = dynamic_cast<UITableElement*>(this->getParent());
	if(!pParentTable)
		return;
	SRect2D srRect;
	pParentTable->getScreenCellPos(myCol, myRow, srRect);


	FLOAT_TYPE fOpacityMult1, fScaleMult1;
	FLOAT_TYPE fOpacityMult2, fScaleMult2;
	SVector2D svDummy;
	this->getLocalPosition(svDummy, &fOpacityMult2, &fScaleMult2);
	pParentTable->getGlobalPosition(svDummy, &fOpacityMult1, &fScaleMult1);

	svOut.x += (srRect.x + srRect.w/2.0)*fScaleMult1*fScaleMult2;
	svOut.y += (srRect.y + srRect.h/2.0)*fScaleMult1*fScaleMult2;

	if(fOpacityOut)
		(*fOpacityOut) = fOpacityMult1*fOpacityMult2;

	if(fScaleOut)
		*fScaleOut = fScaleMult1*fScaleMult2;
}
/*****************************************************************************/
bool UITableCellElement::getIsSelectedFromContents()
{
	ITableCellInfoProvider* pProvider =	getParent<UITableElement>()->getCellInfoProvider();
	if(pProvider)
		return pProvider->isCellSelected(this);
	else
		return false;
}
/*****************************************************************************/
void UITableCellElement::updateOwnData(SUpdateInfo& rRefreshInfo)
{
	ITableCellInfoProvider* pProvider =	getParent<UITableElement>()->getCellInfoProvider();
	if(pProvider)
		pProvider->updateCell(this);
}
/*****************************************************************************/
void UITableCellElement::onDragDropCancelled(UIElement* pReceiver) 
{
	UITableElement* pParentTable = dynamic_cast<UITableElement*>(this->getParent());
	if(pParentTable)
		pParentTable->clearDragDropState();
}
/*****************************************************************************/
};