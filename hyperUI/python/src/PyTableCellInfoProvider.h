#pragma once

namespace PythonAPI
{
#define PY_NAME_TABLE_CELL_INFO_PROVIDER "TableCellInfoProvider"
/*****************************************************************************/
class TableCellInfoProvider
{
public:
	virtual ~TableCellInfoProvider() { }

	int getNumRows(boost::python::object pCaller) { return -1; }
	int getNumColumns(boost::python::object pCaller) { return -1; }

	void updateCell(boost::python::object pCell) { }
	bool isCellSelected(boost::python::object pCell) { return false; }

	bool getAskSelfForCellSelection(boost::python::object pTable) const { return true; }

	void onPostCreateRows(boost::python::object pTable) { }

	const char* getCellType(int iColumn, int iRow, boost::python::object pTable) { return NULL; }
	FLOAT_TYPE getCellHeight(int iColumn, int iRow, boost::python::object pCell) { return -1; }
};
/*****************************************************************************/
}