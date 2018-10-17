#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UIImageDropdown::UIImageDropdown(UIPlane* pParentPlane)
	: UIDropdown(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
UIImageDropdown::~UIImageDropdown()
{
	onDeallocated();
}
/*****************************************************************************/
void UIImageDropdown::onAllocated(IBaseObject* pData)
{
	UIDropdown::onAllocated(pData);
	//EventManager::getInstance()->registerObject(this);
}
/*****************************************************************************/
void UIImageDropdown::onDeallocated()
{
	UIDropdown::onDeallocated();
}
/*****************************************************************************/
bool UIImageDropdown::handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData)
{
	bool bIsHandled = false;
	if(strAction == "uiaChangeThumbnailSelection")
	{
		// The image drop down sends this when an image is selected
		const char* pcsSelImageId = pSourceElem->getParentOfType<UITableCellElement>()->getStringProp(PropertyTargetDataSource);

		selectItem(pcsSelImageId);
		handleTargetElementUpdate(false);

		// Now, we have to hide the drop down
		theCommonString3 = DROPDOWN_TOGGLE_ACTION;
		UIDropdown::handleAction(theCommonString3, pSourceElem, pData);

		// Note that we deliberately set this to false to allow
		// any further controls up the chain to pickup on the change.
		// In effect, we're saying "Ok, we need this message to close
		// our dropdown, but now that we did, someone probably needs it
		// to do something real such as change the active brush, so let's
		// pretend we never saw this message".
		bIsHandled = false;
	}

	if(!bIsHandled)
		bIsHandled = UIDropdown::handleAction(strAction, pSourceElem, pData);

	return bIsHandled;
}
/*****************************************************************************/
void UIImageDropdown::selectItem(const char* pcsItemId, bool bSendAction)
{
	if(IS_VALID_STRING_AND_NOT_NONE(pcsItemId))
		mySelectedItemId = pcsItemId;
	else
		mySelectedItemId = "";

	// Update our image button 
	UIElement* pButton = this->getChildById<UIElement>("defImageButton", true, true);
	UIElement* pSubmenuElem = this->getChildById("__dropdownTemplate__", true, true);

	this->selectItemSubclass(pSubmenuElem, pcsItemId, false);

	// Get out provider. 
	if(pButton->getIsVisible())
	{
		IThreadlessThumbnailProvider* pProvider = this->getThumbnailProvider(pSubmenuElem->getStringProp(PropertyDataSource));
		SVector2D svButtonSize;
		pButton->getBoxSize(svButtonSize);
		AccelImage* pAccelImage = pProvider->getThumbnail(pcsItemId, svButtonSize);
		pButton->setAccelImage(pAccelImage);
		pProvider->deleteSelf();
	}
}
/*****************************************************************************/
void UIImageDropdown::scrollToItem(const char* pcsItemId)
{
	// Just re-select it and set the scroll flag:
	UIElement* pSubmenuElem = this->getChildById("__dropdownTemplate__", true, true);
	this->selectItemSubclass(pSubmenuElem, pcsItemId, true);
}
/*****************************************************************************/
void UIImageDropdown::selectItemSubclass(UIElement* pSubmenuElem, const char* pcsItemId, bool bAutoScrollToItem)
{
	// TODO: Need to implement some sort of resonable default behaviour
	_ASSERT(0);
}
/*****************************************************************************/
IThreadlessThumbnailProvider* UIImageDropdown::getThumbnailProvider(const char* pcsDataSource)
{
	// TODO: Need to implement some sort of resonable default behaviour
	_ASSERT(0);
	return NULL;
}
/*****************************************************************************/
};