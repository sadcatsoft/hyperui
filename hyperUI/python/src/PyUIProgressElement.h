#pragma once

#include "PyUIElement.h"

namespace PythonAPI
{
#define PY_NAME_UIPROGRESS_ELEMENT "UIProgressElement"
/*****************************************************************************/
class UIProgressElement : public PythonAPI::UIElement
{
public:
	UIProgressElement();
	UIProgressElement(UNIQUEID_TYPE idWindow, const char* pcsElemId, UNIQUEID_TYPE idMapping);
	virtual ~UIProgressElement();

	void setMinProgress(FLOAT_TYPE fValue);
	void setMaxProgress(FLOAT_TYPE fValue);
	void setProgress(FLOAT_TYPE fValue);
	FLOAT_TYPE getProgress() const;

protected:
	virtual const char* getPythonClassName() const { return PY_NAME_UIPROGRESS_ELEMENT; }

};
/*****************************************************************************/
}