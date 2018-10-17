#pragma once

class UITableCellElement;
class UITableElement;
/*****************************************************************************/
class HYPERUI_API ITableCellInfoProvider
{
public:
	virtual ~ITableCellInfoProvider() { }

	virtual int getNumRows(UIElement* pCaller) { return -1; }
	virtual int getNumColumns(UIElement* pCaller) { return -1; }

	virtual void updateCell(UITableCellElement* pCell) { }
	virtual bool isCellSelected(UITableCellElement* pCell) { return false; }

	virtual bool getAskSelfForCellSelection(UITableElement* pCaller) const { return true; }

	virtual void onPostCreateRows(UITableElement* pCaller) { }

	virtual const char* getCellType(int iColumn, int iRow, UITableElement* pCaller) { return NULL; }
	virtual FLOAT_TYPE getCellHeight(int iColumn, int iRow, UITableCellElement* pCell) { return -1; }
};
/*****************************************************************************/
