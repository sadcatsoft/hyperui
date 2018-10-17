#pragma once

#include "PyUIElement.h"

namespace PythonAPI
{
#define PY_NAME_UITABLE_ELEMENT "UITableElement"
/*****************************************************************************/
class UITableElement : public PythonAPI::UIElement
{
public:
	UITableElement();
	UITableElement(UNIQUEID_TYPE idWindow, const char* pcsElemId, UNIQUEID_TYPE idMapping);
	virtual ~UITableElement();

	void selectRow(int iRow);

protected:
	virtual const char* getPythonClassName() const { return PY_NAME_UITABLE_ELEMENT; }

};
/*****************************************************************************/
}