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

namespace HyperUI
{
/*****************************************************************************/
UIFixedGrid::UIFixedGrid(UIPlane* pParentPlane)
	: UIGrid(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIFixedGrid::onAllocated(IBaseObject* pData)
{
	UIGrid::onAllocated(pData);
	myCellW = myCellH = 0;
}
/*****************************************************************************/
int UIFixedGrid::getNumColumnsForRow(int iRow)
{
	IFixedGridCellInfoProvider* pProvider = getFixedGridCellInfoProvider();
	if(pProvider)
		return pProvider->getNumColumns(this);
	else
		return this->getNumProp(PropertyNumCellsX);
}
/*****************************************************************************/
int UIFixedGrid::getNumRows()
{
	IFixedGridCellInfoProvider* pProvider = getFixedGridCellInfoProvider();
	if(pProvider)
		return pProvider->getNumRows(this);
	else
		return this->getNumProp(PropertyNumCellsY);
}
/*****************************************************************************/
const CHAR_TYPE* UIFixedGrid::getCellType(int iColumn, int iRow)
{
	if(getPropertyDataType(PropertyCellTypes) == PropertyDataStringList)
		return this->getEnumPropValue(PropertyCellTypes, iColumn);
	else
		return this->getStringProp(PropertyCellTypes);
}
/*****************************************************************************/
int UIFixedGrid::getNumTotalCells()
{
	return getNumColumnsForRow(0)*getNumRows();
}
/*****************************************************************************/
void UIFixedGrid::adjustToScreen()
{
	UIGrid::adjustToScreen();
	recomputeCellSizing();
	this->adjustCellsSize(myCellW, myCellH);
}
/*****************************************************************************/
void UIFixedGrid::recomputeCellSizing()
{
	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);

	// Now, assign individual cell dims based on our size and num 
	// fixed cells.
	FLOAT_TYPE fSelfW = this->getNumProp(PropertyWidth)*fLocScale;
	FLOAT_TYPE fSelfH = this->getNumProp(PropertyHeight)*fLocScale;

	myOrigMeasureDims.set(fSelfW, fSelfH);

	int iNumCols = getNumColumnsForRow(0);
	if(iNumCols <= 0)
		iNumCols = 1;
	int iNumRows = getNumRows();
	if(iNumRows <= 0)
		iNumRows = 1;
	myCellW = fSelfW/iNumCols;
	myCellH = fSelfH/iNumRows;
}
/*****************************************************************************/
void UIFixedGrid::onPreCreateRows()
{
	this->recomputeCellSizing();
	UIGrid::onPreCreateRows();
}
/*****************************************************************************/
void UIFixedGrid::onCellCreated(int iColumn, int iRow, UITableCellElement* pCell)
{
	UIGrid::onCellCreated(iColumn, iRow, pCell);

	// Set its dims
	//_ASSERT(myCellH > 0 && myCellW > 0);
	pCell->setNumProp(PropertyWidth, myCellW);
	pCell->setNumProp(PropertyHeight, myCellH);
	pCell->resetEvalCache(true);
}
/*****************************************************************************/
IFixedGridCellInfoProvider* UIFixedGrid::getFixedGridCellInfoProvider()
{
	IFixedGridCellInfoProvider* pProvider;
	UIElement* pCurr = this;
	while(pCurr)
	{
		if(pProvider = dynamic_cast<IFixedGridCellInfoProvider*>(pCurr))
			return pProvider;
		pCurr = pCurr->getParent<UIElement>();
	}

	// Otherwise, see if we're a popup and if our parent has a provider
	UIElement* pTopmostParent = this->getTopmostParent<UIElement>();
	if(pTopmostParent && pTopmostParent->doesPropertyExist(PropertyPopupSourceUiElement))
	{
		pCurr = getUIPlane()->getElementById(pTopmostParent->getStringProp(PropertyPopupSourceUiElement));
		while(pCurr)
		{
			if(pProvider = dynamic_cast<IFixedGridCellInfoProvider*>(pCurr))
				return pProvider;
			pCurr = pCurr->getParent<UIElement>();
		}
	}

	return NULL;
}
/*****************************************************************************/
int UIFixedGrid::getCellIndex(UITableCellElement* pCell)
{
	int iCol = pCell->getCol();
	int iRow = pCell->getRow();
	return getNumColumnsForRow(0)*iRow + iCol;
}
/*****************************************************************************/
UIElement* UIFixedGrid::getChildAtRecursiveInternal(const SVector2D& svPoint, const SVector2D& svScroll, bool bFirstNonPassthrough, bool bIgnoreChildren, bool bGlidingOnly)
{
	// Table will have no children listed,  but check anyway:
	UIElement* pTempFound = UIGrid::getChildAtRecursiveInternal(svPoint, svScroll, bFirstNonPassthrough, bIgnoreChildren, bGlidingOnly);

	// NOTE: The below would have gotten a particular cell or its child element
	// as a result. However, if this is enabled here, we no longer can scroll,
	// since instead of the table being returned, we get a cell or its child
	// returned, which know nothing about scrolling, and the table never gets
	// its mouse events.
	if(pTempFound != this)
		return pTempFound;

	// Otherwise, see if we actually have a child:
	UITableCellElement* pCell = this->getCell(svPoint.x, svPoint.y);
	if(pCell)
		return pCell->getChildAtRecursiveInternal(svPoint, svScroll, bFirstNonPassthrough, bIgnoreChildren, bGlidingOnly);
	else
		return this;
}
/*****************************************************************************/
}