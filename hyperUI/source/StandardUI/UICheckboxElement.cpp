#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UICheckboxElement::UICheckboxElement(UIPlane* pParentPlane)
	: UIButtonElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UICheckboxElement::onAllocated(IBaseObject* pData)
{
	UIButtonElement::onAllocated(pData);
}
/*****************************************************************************/
void UICheckboxElement::onPressed(TTouchVector& vecTouches)
{
	// NOTE: We deliberately skip the UIButtonElement onPressed().
	UIElement::onPressed(vecTouches);
	if(getIsEnabled())
	{
		myAllowPressing = true;
		if(getParentWindow()->getCurrMouseButtonDown() == MouseButtonLeft)
		{
			this->setIsPushed(!this->getIsPushed());
			handleTargetElementUpdate(false);
		}
	}
}
/*****************************************************************************/
void UICheckboxElement::onMouseLeave(TTouchVector& vecTouches)
{
	UIElement::onMouseLeave(vecTouches);
	//myAllowPressing = false;
}
/*****************************************************************************/
void UICheckboxElement::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{
	// Note: skip the button parent on purpose
	UIElement::onReleased(vecTouches, bIgnoreActions);

	if(getIsEnabled())
	{
		if(myAllowPressing)
		{
			if(myCallbacks)
				myCallbacks->onButtonClicked(this, getParentWindow()->getCurrMouseButtonDown());
		}
	}
	myAllowPressing = false;
}
/*****************************************************************************/
bool UICheckboxElement::getIsEnum(void)
{
	return false;
}
/*****************************************************************************
void UICheckboxElement::handleTargetElementUpdate()
{
	Node* pNode;
	StringResourceProperty* pParm = getTargetParm(pNode);
	if(!pParm)
		return;

	pParm->setBool(this->getIsPushed());

	pNode->markCacheDirty();
	pNode->onParmChanged(pParm->getPropertyName());
}
/*****************************************************************************/
};