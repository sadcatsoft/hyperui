#pragma once

#include "PyUIElement.h"

namespace PythonAPI
{
#define PY_NAME_UISPLITTER_ELEMENT "UISplitterElement"
/*****************************************************************************/
class UISplitterElement: public PythonAPI::UIElement
{
public:
	UISplitterElement();
	UISplitterElement(UNIQUEID_TYPE idWindow, const char* pcsElemId, UNIQUEID_TYPE idMapping);
	virtual ~UISplitterElement();

protected:
	virtual const char* getPythonClassName() const { return PY_NAME_UISPLITTER_ELEMENT; }
};
/*****************************************************************************/
}