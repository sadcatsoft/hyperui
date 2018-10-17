#include "stdafx.h"

namespace HyperUI
{
TUITableCellElementVector UIGrid::theTempCells;
/*****************************************************************************/
UIGrid::UIGrid(UIPlane* pParentPlane)
	: UITableElement(pParentPlane)
{
    onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIGrid::onAllocated(IBaseObject* pData)
{
    UITableElement::onAllocated(pData);
	myIsRepositionDirty = false;
    myIsAdjustingToScreen = false;
}
/*****************************************************************************/
int UIGrid::getNumColumnsForRow(int iRow)
{
    int iDummy;
    return computeCellNumbers(iRow, iDummy);
}
/*****************************************************************************/
int UIGrid::getNumTotalCells()
{
	IGridCellInfoProvider* pProvider = getGridCellInfoProvider();
	if(pProvider)
		return pProvider->getNumTotalCells(this);
	else
		return 1;
}
/*****************************************************************************/
IGridCellInfoProvider* UIGrid::getGridCellInfoProvider()
{
	IGridCellInfoProvider* pProvider;
	UIElement* pCurr = this;
	while(pCurr)
	{
		if(pProvider = dynamic_cast<IGridCellInfoProvider*>(pCurr))
			return pProvider;
		pCurr = pCurr->getParent<UIElement>();
	}

	return NULL;
}
/*****************************************************************************/
int UIGrid::getNumRows()
{
    int iDummy;
    computeCellNumbers(0, iDummy);
    return iDummy;
}
/*****************************************************************************/
int UIGrid::computeCellNumbers(int iForRow, int& iNumRowsOut)
{
    // See what's our current size
    SVector2D svSize;
    this->getBoxSize(svSize);

	int iNumCells = getNumTotalCells();
    SVector2D svCellSize(this->getNumProp(PropertyCellWidth), this->getNumProp(PropertyCellHeight));
	return UIGrid::computeCellNumbers(iForRow, iNumRowsOut, svSize, svCellSize, iNumCells);

    /*
    _ASSERT(svCellSize.x > 0 && svCellSize.y > 0);
    int iCellsPerRow = svSize.x/svCellSize.x;
    if(iCellsPerRow < 1)
		iCellsPerRow = 1;

	int iNumCells = getNumTotalCells();
    iNumRowsOut = iNumCells/iCellsPerRow;
    if(iNumRowsOut*iCellsPerRow < iNumCells)
	iNumRowsOut++;

    if(iForRow != iNumRowsOut - 1)
    {
		// Not the last row
		return iCellsPerRow;
    }
    else
    {
		// Last row, compute the remainder.
		return iNumCells - (iNumRowsOut - 1)*iCellsPerRow;
    }
	*/
}
/*****************************************************************************/
int UIGrid::computeCellNumbers(int iForRow, int& iNumRowsOut, const SVector2D& svFullAreaSize, const SVector2D& svCellSize, int iNumCells)
{
	_ASSERT(svCellSize.x > 0 && svCellSize.y > 0);
	int iCellsPerRow = svFullAreaSize.x/svCellSize.x;
	if(iCellsPerRow < 1)
		iCellsPerRow = 1;

	iNumRowsOut = iNumCells/iCellsPerRow;
	if(iNumRowsOut*iCellsPerRow < iNumCells)
		iNumRowsOut++;

	if(iForRow != iNumRowsOut - 1)
	{
		// Not the last row
		return iCellsPerRow;
	}
	else
	{
		// Last row, compute the remainder.
		return iNumCells - (iNumRowsOut - 1)*iCellsPerRow;
	}
}
/*****************************************************************************/
void UIGrid::adjustToScreen()
{
    if(myIsAdjustingToScreen)
		return;
    myIsAdjustingToScreen = true;

    // First, do the actual sizing:
    UIElement::adjustToScreen();

	// This causes the tool window color swatch to not align correctly
	// when we resize the pane...
 	if(!getIsDirty())
 		repositionCells();
 	else
 		getCell(0, 0, true);

    // Now, recompute our dims.
//    markDirty();
    // Force to recompute
   // getCell(0, 0, true);


    // Now do the table stuff. This will call
    // UIElement::adjustToScreen() but that's ok.
    UITableElement::adjustToScreen();

    myIsAdjustingToScreen = false;
}
/*****************************************************************************/
void UIGrid::autoLayoutChildren()
{
    // Now, recompute our dims.
    //markDirty();
	
    this->resetEvalCache(true);
	myIsRepositionDirty = true;

    // Force to recompute
    //getCell(0, 0, true);
}
/*****************************************************************************/
const char* UIGrid::getCellType(int iColumn, int iRow)
{
    // Return the only cell type for this grid for now. Note that we may
    // have [1, N] rows, so a simple enum isn't going to work.
    return this->getEnumPropValue(PropertyCellTypes, 0);
}
/*****************************************************************************/
void UIGrid::repositionCells()
{
	if(getIsDirty())
		return;

	int iCurrCell, iNumCells;

	// First, grab all the cells we have
	theTempCells.clear();
	int iRow, iCurrNumRows = myRows.size();
	for(iRow = 0; iRow < iCurrNumRows; iRow++)
	{
		iNumCells = myRows[iRow].myCells.size();
		for(iCurrCell = 0; iCurrCell < iNumCells; iCurrCell++)
			theTempCells.push_back(myRows[iRow].myCells[iCurrCell]);
	}

	clearBasic();

	int iCurrCol, iNumColumns;

	// Now, shove them onto our new dims
	iCurrCell = 0;
	int iOldCellCount = theTempCells.size();
	UITableCellElement* pCurrCell;
	// DO NOT replace it with iCurrNumRows; the idea is
	// that we may reconfigure our size and thus rows x columns
	// configurate, and the getNumRows() below gets us the new
	// number of rows. Otherwise, we may crash!
	int iCurrRow, iNumRows = this->getNumRows();
	for(iCurrRow = 0; iCurrRow < iNumRows && iCurrCell < iOldCellCount; iCurrRow++)
	{
		iNumColumns = this->getNumColumnsForRow(iCurrRow);
		SUITableRow rDummyRow;
		for(iCurrCol = 0; iCurrCol < iNumColumns && iCurrCell < iOldCellCount; iCurrCol++)
		{
			pCurrCell = theTempCells[iCurrCell];
			pCurrCell->setRowCol(iCurrCol, iCurrRow, iCurrCell);

			iCurrCell++;

			rDummyRow.myCells.push_back(pCurrCell);
		}
		myRows.push_back(rDummyRow);
	}

	updateCellRelatedTableInfo(true, false);

	mySavedSelectedRowId = -1;

	theTempCells.clear();
}
/*****************************************************************************/
void UIGrid::ensureRowsUpToDate()
{
	if(!getIsDirty() && myIsRepositionDirty)
		repositionCells();
	UITableElement::ensureRowsUpToDate();
	myIsRepositionDirty = false;
}
/*****************************************************************************/
};