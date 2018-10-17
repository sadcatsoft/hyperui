#pragma once

class HYPERUI_API UIElement;
/*****************************************************************************/
class HYPERUI_API IHandleActionCallback
{
public:
	virtual ~IHandleActionCallback() { }
	// Return true to hide the caller, false otherwise.
	virtual bool handleAction(const char* pcsAction, UIElement* pSourceElem) = 0;
};
/*****************************************************************************/