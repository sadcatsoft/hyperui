#pragma once

/*****************************************************************************/
class HYPERUI_API UIDropdownTextPart : public UIElement
{
public:

	DECLARE_STANDARD_UIELEMENT(UIDropdownTextPart, UiElemDropdownTextPart);

	virtual bool getAllowKeepingFocus() { return true; }
	virtual void onLostFocus(bool bHasCancelled);
	virtual void onGainedFocus();

	virtual bool onKeyUp(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl);
	virtual bool onTextPreRender(string& strText, SColor& scolText, int &iCursorPosOut);
};
/*****************************************************************************/