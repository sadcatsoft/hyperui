#pragma once

#define DROPDOWN_MENU_SIZE_OFFSET		upToScreen(24)

#define DROPDOWN_TOGGLE_ACTION			"uiaToggleDropdown"
#define UIA_DROPDOWN_ITEM_SELECTED		"uiaDropdownItemSelected"
#define UIA_DROPDOWN_ITEM_SELECTED_BEFORE_TEXT_UPDATE		"uiaDropdownItemSelectedBeforeTextUpdate"
/*****************************************************************************/
class HYPERUI_API UIDropdown: public UIElement
{
public:

	DECLARE_STANDARD_UIELEMENT(UIDropdown, UiElemDropdown);

	virtual void postInit(void);

	virtual const char* getSelectedId();
	virtual const char* getSelectedLabel();
	int getNumChoices();

	void adjustSelection(int iDir);
	virtual void selectItem(const char* pcsItemId, bool bSendAction = false);

	virtual bool handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData);
	void onDropdownHidden();

	//virtual void handleTargetElementUpdate();

	virtual void preRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);
	virtual void postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	void clearContents();
	void addItem(const char* pcsId, const char* pcsLabel, bool bIsCheckmark = false);

	void initFromItemChildren(StringResourceItem* pItem, const char* pcsLabelProp, const char* pcsIdProp);
	void setFromVectors(const TStringVector& vecIds, const TStringVector& vecTitles);

	virtual void updateOwnData(SUpdateInfo& rRefreshInfo);

	virtual void onIdChanged();

	const char* findIdByLabel(const char* pcsLabelPart);

	virtual void adjustToScreen();

protected:
	virtual void render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	virtual void updateSelectionText();

	virtual bool getIsImageDropDown() { return false; }

	void resetCallbackName();

protected:

	int mySelectedIndex;

	static TUIElementVector theUiVElems;

	FLOAT_TYPE myMinimzedHeight;
	ResourceItemMenuProvider myDefaultProvider;
	bool myIsInRefresh;
};
/*****************************************************************************/