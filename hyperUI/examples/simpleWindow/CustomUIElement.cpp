#include "stdafx.h"
#include "CustomUIElement.h"

/*****************************************************************************/
CustomUIElement::CustomUIElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
CustomUIElement::~CustomUIElement()
{
	onDeallocated();
}
/*****************************************************************************/
void CustomUIElement::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
}
/*****************************************************************************/
void CustomUIElement::onDeallocated()
{
	UIElement::onDeallocated();
}
/*****************************************************************************/
void CustomUIElement::updateOwnData(SUpdateInfo& rRefreshInfo)
{
	gLog("updateOwnData() called for %s\n", this->getId());
}
/*****************************************************************************/
bool CustomUIElement::handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData)
{
	gLog("handleAction() called for %s with action = %s\n", this->getId(), strAction.c_str());

	bool bDidHandle = false;
	if(strAction == "changeButtonText")
	{
		this->setTextExt("The %s button has been pressed last", pSourceElem->getId());
		bDidHandle = true;
	}

	return bDidHandle;
}
/*****************************************************************************/