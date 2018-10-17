#pragma once

// Not needed?
/*****************************************************************************/
class HYPERUI_API UIUnitedNumeric : public UIElement
{
public:
	DECLARE_STANDARD_UIELEMENT(UIUnitedNumeric, UiElemUnitedNumeric);
	virtual bool handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData);
};
/*****************************************************************************/
