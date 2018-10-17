#pragma once

namespace PythonAPI
{
#define PY_NAME_GRID_CELL_INFO_PROVIDER "GridCellInfoProvider"
/*****************************************************************************/
class GridCellInfoProvider
{
public:
	virtual ~GridCellInfoProvider() { }
	int getNumTotalCells(boost::python::object pCaller) { return 1; }
};
/*****************************************************************************/
}