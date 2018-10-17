#pragma once

class UITableElement;
class UIFixedGrid;

#define UIA_TABLE_CELL_SELECTED				"uiaTableCellSelected"
#define UIA_TABLE_CELL_DESELECTED			"uiaTableCellDeselected"

/*****************************************************************************/
class HYPERUI_API UITableCellElement : public UIElement
{
public:

	UITableCellElement(): UIElement() { }
	DECLARE_STANDARD_UIELEMENT(UITableCellElement, UiElemTableCell);

	void setRowCol(int iCol, int iRow, int iTotalCountIndex);

	void setIsSelected(bool bIsSelected, bool bIsComingFromUI);
	bool getIsSelected(void);

	int getCol();
	int getRow();
	inline int getTotalCountIndex() const { return myTotalCountIndex; }

	virtual UIElement* getChildAtRecursive(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bFirstNonPassthrough, bool bGlidingOnly);

	virtual UNIQUEID_TYPE getUniqueCellId() { return -1; }
	virtual void getGlobalPosition(SVector2D& svOut, FLOAT_TYPE* fOpacityOut = NULL, FLOAT_TYPE* fScaleOut = NULL);

	// Used for drag and drop containment.
	virtual bool getCanContain(UIElement* pOther) { return false; }
	virtual bool getIsFirstChild() { return false; }
	virtual bool getIsLastChild() { return false; }

	virtual void renderDragDropHoverState(SRect2D& srScreenRect, FLOAT_TYPE fOpacity, AcceptType eDragAcceptType) { }

	// Asks whether the cell is selected from some scene contents. Used when refreshing if the table
	// overrides getAskCellsForSelection() with true.
	virtual bool getIsSelectedFromContents();

	virtual void updateOwnData(SUpdateInfo& rRefreshInfo);

	virtual void onDragDropCancelled(UIElement* pReceiver);

protected:

	// Override this for custom or dynamic cell heights
	virtual FLOAT_TYPE getCellHeight() { return -1; }
	virtual UIElement* getChildAtRecursiveInternal(const SVector2D& svPoint, const SVector2D& svScroll, bool bFirstNonPassthrough, bool bIgnoreChildren, bool bGlidingOnly);

	friend class UITableElement;
	friend class UIFixedGrid;

	virtual void saveStateInto(UITableElement* pParentTable) { }
	
private:

	int myCol, myRow, myTotalCountIndex;
	bool myIsSelected;
};
/*****************************************************************************/
typedef vector < UITableCellElement* > TUITableCellElementVector;
/*****************************************************************************/
