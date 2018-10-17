#pragma once

#include "PyUIElement.h"

namespace PythonAPI
{
#define PY_NAME_UITABLE_CELL_ELEMENT "UITableCellElement"
/*****************************************************************************/
class UITableCellElement : public PythonAPI::UIElement
{
public:
	UITableCellElement();
	UITableCellElement(UNIQUEID_TYPE idWindow, const char* pcsElemId, UNIQUEID_TYPE idMapping);
	virtual ~UITableCellElement();

	int getCol();
	int getRow();

protected:
	virtual const char* getPythonClassName() const { return PY_NAME_UITABLE_CELL_ELEMENT; }

};
/*****************************************************************************/
}