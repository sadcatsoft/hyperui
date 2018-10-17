#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UIElementIdentifier::UIElementIdentifier(UIElement* pTargetElem)
{
	myElemId = pTargetElem->getStringProp(PropertyId);
	myParentWindowId = pTargetElem->getParentWindow()->getId();
}
/*****************************************************************************/
UIElementIdentifier::UIElementIdentifier()
{

}
/*****************************************************************************/
IUndoItem* UIElementIdentifier::createUndoItemForSelfChange(UIElement* pSourceElem)
{
	if(myElemId.length() > 0)
	{
		Window* pWindow = WindowManager::getInstance()->findItemById(myParentWindowId);
		if(!pWindow)
			ASSERT_RETURN_NULL;

		UIElement* pElem = pWindow->getUIPlane()->getElementById<UIElement>(myElemId.c_str(), true, false);
		if(!pElem)
			ASSERT_RETURN_NULL;

		return new UndoItemUIValueChange(pElem);
	}
	ELSE_ASSERT_RETURN_NULL;
}
/*****************************************************************************/
void UIElementIdentifier::refreshStoredValueFromUI(UIElement* pSourceElem, bool bIsChangingContinuously)
{
	Window* pWindow = WindowManager::getInstance()->findItemById(myParentWindowId);
	if(!pWindow)
		ASSERT_RETURN;

	UIElement* pElem = pWindow->getUIPlane()->getElementById<UIElement>(myElemId.c_str(), true, false);
	if(!pElem)
		ASSERT_RETURN;
	pElem->onRefreshStoredValueFromUICallback(pSourceElem, bIsChangingContinuously);
}
/*****************************************************************************/
void UIElementIdentifier::refreshUIFromStoredValue(UIElement* pTargetElem)
{
	Window* pWindow = WindowManager::getInstance()->findItemById(myParentWindowId);
	if(!pWindow)
		ASSERT_RETURN;

	UIElement* pElem = pWindow->getUIPlane()->getElementById<UIElement>(myElemId.c_str(), true, false);
	if(!pElem)
		ASSERT_RETURN;
	pElem->onRefreshUIFromStoredValueCallback(pTargetElem);
}
/*****************************************************************************/
bool UIElementIdentifier::getIsEnabled(UIElement* pIdentifierOwnerElem)
{
	Window* pWindow = WindowManager::getInstance()->findItemById(myParentWindowId);
	if(!pWindow)
		ASSERT_RETURN_TRUE;

	UIElement* pElem = pWindow->getUIPlane()->getElementById<UIElement>(myElemId.c_str(), true, false);
	if(!pElem)
		ASSERT_RETURN_TRUE;

	IUIElementIdentifierCallback* pAsCallback = as<IUIElementIdentifierCallback>(pElem);
	if(!pAsCallback)
		return true;
	else
		return pAsCallback->getIsChildIdentifierEnabled(pIdentifierOwnerElem);
}
/*****************************************************************************/
};