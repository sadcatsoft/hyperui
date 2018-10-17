#pragma once

/*****************************************************************************/
class HYPERUI_API UIImageDropdown : public UIDropdown
{
public:

	DECLARE_STANDARD_UIELEMENT_NO_DESTRUCTOR_DEF(UIImageDropdown, UiElemImageDropdown);
	virtual void onDeallocated();

	virtual const char* getSelectedId() { return mySelectedItemId.c_str(); }
	virtual const char* getSelectedLabel()  { return mySelectedItemId.c_str(); }

	virtual bool handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData);

	virtual void selectItem(const char* pcsItemId, bool bSendAction = false);

	void deselectAllThumbnails();

	void scrollToItem(const char* pcsItemId);

protected:

	virtual bool getIsImageDropDown() { return true; }

	virtual void selectItemSubclass(UIElement* pSubmenuElem, const char* pcsItemId, bool bAutoScrollToItem);
	virtual IThreadlessThumbnailProvider* getThumbnailProvider(const char* pcsDataSource);

private:

	string mySelectedItemId;
};
/*****************************************************************************/