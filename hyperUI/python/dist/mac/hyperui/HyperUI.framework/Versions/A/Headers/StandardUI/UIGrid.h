#pragma once

/*****************************************************************************/
class HYPERUI_API IGridCellInfoProvider
{
public:
	virtual ~IGridCellInfoProvider() { }

	virtual int getNumTotalCells(UIElement* pCaller) { return 1; }
};
/*****************************************************************************/
class HYPERUI_API UIGrid: public UITableElement
{
public:

	DECLARE_STANDARD_UIELEMENT(UIGrid, UiElemGrid);

    virtual int getNumColumnsForRow(int iRow);
	virtual int getNumTotalCells();
    virtual int getNumRows();

    virtual void adjustToScreen();
    virtual void autoLayoutChildren();

    virtual const char* getCellType(int iColumn, int iRow);

	static int computeCellNumbers(int iForRow, int& iNumRowsOut, const SVector2D& svFullAreaSize, const SVector2D& svCellSize, int iNumCells);

	// Disable this since grids don't scroll
	virtual bool onMouseWheel(FLOAT_TYPE fDelta) { return false; }

protected:

	virtual void ensureRowsUpToDate();
	IGridCellInfoProvider* getGridCellInfoProvider();

private:

	void repositionCells();

    int computeCellNumbers(int iForRow, int& iNumRowsOut);
    bool myIsAdjustingToScreen;

	static TUITableCellElementVector theTempCells;

	bool myIsRepositionDirty;
};
/*****************************************************************************/