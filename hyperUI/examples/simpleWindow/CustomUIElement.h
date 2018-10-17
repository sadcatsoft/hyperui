#pragma once

/*****************************************************************************/
class CustomUIElement : public UIElement
{
public:
	DECLARE_UIELEMENT(CustomUIElement);
	virtual void updateOwnData(SUpdateInfo& rRefreshInfo);
	virtual bool handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData);
};
/*****************************************************************************/