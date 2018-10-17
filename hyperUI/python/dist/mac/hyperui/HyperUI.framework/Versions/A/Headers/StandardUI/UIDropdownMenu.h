#pragma once

/********************************************************************************************/
class HYPERUI_API UIDropdownMenu : public UIMenuElement
{
public:
	
	DECLARE_STANDARD_UIELEMENT_NO_DESTRUCTOR_DEF(UIDropdownMenu, UiElemDropdownMenu);
	virtual void onDeallocated(void);

	inline void setIsShowingScrollbar(bool bValue) { myIsShowingScrollbar = bValue; }

protected:

	virtual void getAutoScrollingRectangleSizeAndShiftOffsets(SVector2D& svSizeOffsetOut, SVector2D& svShiftOffsetOut);
	virtual void customMenuItemAction(const char* pcsItemId); 
	virtual bool getDoScissorRect() const { return myIsShowingScrollbar; }

	virtual void getSelectionClickExclusionRectangle(SRect2D& srOut);

private:
	bool myIsShowingScrollbar;
};
/********************************************************************************************/