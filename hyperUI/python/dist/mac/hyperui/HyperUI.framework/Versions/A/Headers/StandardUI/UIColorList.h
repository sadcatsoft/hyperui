#pragma once

/*****************************************************************************/
class HYPERUI_API UIColorList : public UIElement, public ITableCellInfoProvider, public IColorChangedCallback
{
public:
	DECLARE_STANDARD_UIELEMENT(UIColorList, UiElemColorList);

	void setColorList(const SColorList& rGrad) { myColorList = rGrad;  recreateColorUI(); }
	inline const SColorList* getColorList() const { return &myColorList; }

	virtual void onColorChanged(const SColor& scolNewColor, UIColorSwatch* pElem);
	virtual void updateOwnData(SUpdateInfo& rRefreshInfo);

protected:

	void recreateColorUI();

	// Table cell info provider
	virtual int getNumRows(UIElement* pCaller);
	virtual void updateCell(UITableCellElement* pCell);
	virtual bool isCellSelected(UITableCellElement* pCell);

	virtual bool handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData);

private:

	SColorList myColorList;
};
/*****************************************************************************/