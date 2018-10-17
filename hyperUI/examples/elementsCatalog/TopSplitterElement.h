#pragma once

/*****************************************************************************/
class TopSplitterElement : public UISplitterElement, public ITableCellInfoProvider, public IGridCellInfoProvider
{
public:
	DECLARE_UIELEMENT(TopSplitterElement);
	virtual bool handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData);

	// Table callback provider
	virtual int getNumRows(UIElement* pCaller);
	virtual int getNumColumns(UIElement* pCaller);
	virtual void updateCell(UITableCellElement* pCell);
	virtual void onPostCreateRows(UITableElement* pCaller);

	virtual void onTimerTick(GTIME lGlobalTime);

	virtual int getNumTotalCells(UIElement* pCaller) { return 32; }

private:

	TStringStringTupleVector myElementTypeStrings;
};
/*****************************************************************************/