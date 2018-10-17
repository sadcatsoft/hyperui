#pragma once

#define UIA_TABLE_ROW_SELECTION_CHANGED		"uiaTableRowSelectionChanged"
#define UIA_TABLE_ROW_SELECTED				"uiaTableRowSelected"
#define UIA_TABLE_ROW_DESELECTED			"uiaTableRowDeselected"

#define TABLE_VERT_PADDING		upToScreen(100)

// x dist/y dist ratio for this to be considered scrolling.
#define HOR_VERT_MOTION_RATIO	0.35

//#define ENABLE_TABLE_LOCK

class UISliderElement;
/*****************************************************************************/
struct HYPERUI_API SUITableRow
{
	SUITableRow()
	{
		myHeight = 0;
		myIsSelected = false;
	}

	int myHeight;
	bool myIsSelected;
	TUITableCellElementVector myCells;
};
typedef vector < SUITableRow > TUITableRows;
/*****************************************************************************/
class HYPERUI_API SBaseCellStateInfo
{
public:
	UNIQUEID_TYPE myId;
	bool myIsSelected;

};
typedef map < UNIQUEID_TYPE, SBaseCellStateInfo > TBaseCellStateInfos;
/*****************************************************************************/
enum TableRefreshType
{
	TableRefreshRecreate = 0,
	TableRefreshRefreshDataOnly,
	TableRefreshClean
};
/*****************************************************************************/
class HYPERUI_API UITableElement : public UIElement
{
public:

	DECLARE_STANDARD_UIELEMENT_NO_CONSTRUCTOR_DESTRUCTOR_DEF(UITableElement, UiElemTable);
	virtual void onDeallocated(void);

	virtual void postInit(void);

	virtual void render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	void markDirty(TableRefreshType eDirtyType = TableRefreshRecreate);

	virtual void onPressed(TTouchVector& vecTouches);
	virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);
	virtual void onMouseMove(TTouchVector& vecTouches);
	virtual void onMouseLeave(TTouchVector& vecTouches);

	virtual bool onMouseWheel(FLOAT_TYPE fDelta);

	virtual void onTimerTick(GTIME lGlobalTime);
	void getScreenCellPos(int iCol, int iRow, SRect2D& srRectOut);

	virtual UIElement* getChildAtRecursive(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bFirstNonPassthrough, bool bGlidingOnly);

	bool getIsRowSelected(int iRow);
	int getSelectedRow(void);
	bool getAreMultipleRowsSelected();
	bool getIsAnyRowSelected();
	int getLastSelectedRow();

	void updateCellData();

	UITableCellElement* getSelectedCell() { _ASSERT(!this->getBoolProp(PropertyAllowMultipleSelection)); return mySelectedCell; }
	template < class TYPE > TYPE* getCell(int iCol, int iRow, bool bCreateIfNeeded = false) { return dynamic_cast<TYPE*>(getCell(iCol, iRow, bCreateIfNeeded)); }
	UITableCellElement* getCell(int iCol, int iRow, bool bCreateIfNeeded = false);

	UIElement* getCellWithPropValue(PropertyType eProp, const char* pcsValue, bool bIncludeChildren);
	UISliderElement* getRelatedTableSlider(void);

	// Override these methods to change contents
	// We return one by default to allow tables to be
	// one-celled by default.
	virtual int getNumRows();

	int getNumColumns();

	void selectRow(int iRow, bool bComingFromUI = true);
	void selectCell(int iCol, int iRow, bool bComingFromUI = true);
	virtual void adjustToScreen(void);

	void deselectAllCells(bool bIsComingFromUI);
	void scrollOnSlider(UISliderElement* pSlider);

	UIElement* getFinalCellElementAt(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bGlidingOnly);

	bool getDoTouchesLookLikeScrolling();

	virtual void setPushedForRadioGroupRecursive(const char* pcsGroup, UIButtonElement* pException, bool bInstant);

	// In cases where we want a row to not be complete.
	virtual int getNumColumnsForRow(int iRow) { return -1; }
	int getTotalRowHeight();

	ITableCellInfoProvider* getCellInfoProvider();

	void clearDragDropState();
	inline int getNumExistingRows() const { return myRows.size(); }
	int getNumExistingColumns(int iRow);

	FLOAT_TYPE getScrollOffset() { return myScrollOffset; }

	inline void resetCachedCellDef() { myLastCellDef = NULL; myLastCellDefType = ""; }

	void ensureRowVisible(int iRow);
	virtual void ensureRowsUpToDate();

	virtual void onCellCreated(int iColumn, int iRow, UITableCellElement* pCell) { }

	class SelectedRowIterator
	{
	public:
		SelectedRowIterator(UITableElement& pTable)
		{
			myTable = &pTable;
			myCurrRow = 0;
			ensureSelectedRow();
		}

		SelectedRowIterator()
		{
			myTable = NULL;
			myCurrRow = -1;
		}

		void operator++(int)
		{
			myCurrRow++;
			ensureSelectedRow();
		}

		void toEnd() { myCurrRow = myTable->getNumRows(); }
		bool isEnd() const { return myCurrRow >= myTable->getNumRows(); }

		bool operator==(const SelectedRowIterator& other) const { return myCurrRow == other.myCurrRow && myTable == other.myTable; }
		bool operator!=(const SelectedRowIterator& other) const { return ! (*this == other); }

		inline int getRowIndex() const { return myCurrRow; }

	private:
		void ensureSelectedRow()
		{
			int iNumRows = myTable->getNumRows();
			for(; myCurrRow < iNumRows; myCurrRow++)
			{
				if(myTable->getIsRowSelected(myCurrRow))
					break;
			}
		}

	private:
		UITableElement *myTable;
		int myCurrRow;
	};

	SelectedRowIterator selectedRowsFirst() { SelectedRowIterator rTempIter(*this); return rTempIter; }

	class RowsIterator
	{
	public:
		RowsIterator(UITableElement& pTable)
		{
			myTable = &pTable;
			myCurrRow = 0;
		}

		RowsIterator()
		{
			myTable = NULL;
			myCurrRow = -1;
		}

		void operator++(int) { myCurrRow++; }
		void toEnd() { myCurrRow = myTable->myRows.size(); }
		bool isEnd() const { return myCurrRow >= myTable->myRows.size(); }

		bool operator==(const RowsIterator& other) const { return myCurrRow == other.myCurrRow && myTable == other.myTable; }
		bool operator!=(const RowsIterator& other) const { return ! (*this == other); }

		inline int getRowIndex() const { return myCurrRow; }
	private:
		UITableElement *myTable;
		int myCurrRow;
	};

	RowsIterator rowsFirst() { RowsIterator rTempIter(*this); return rTempIter; }

	class CellsIterator
	{
	public:
		CellsIterator(UITableElement& pTable)
		{
			myTable = &pTable;
			myCurrRow = 0;
			myCurrCell = 0;
		}

		CellsIterator()
		{
			myTable = NULL;
			myCurrRow = -1;
			myCurrCell = -1;
		}

		void operator++(int) 
		{ 
			if(isEnd())
				return;

			myCurrCell++;
			if(myCurrCell >= myTable->myRows[myCurrRow].myCells.size())
			{
				myCurrCell = 0;
				myCurrRow++; 			
			}
		}
		void toEnd() { myCurrRow = myTable->myRows.size(); }
		bool isEnd() const { return myCurrRow >= myTable->myRows.size(); }

		bool operator==(const CellsIterator& other) const { return myCurrRow == other.myCurrRow && myTable == other.myTable && myCurrCell == other.myCurrCell; }
		bool operator!=(const CellsIterator& other) const { return ! (*this == other); }

		inline int getRowIndex() const { return myCurrRow; }
		inline int getCellIndex() const { return myCurrCell; }
		inline UITableCellElement* getCell() { return myTable ? myTable->getCell(myCurrCell, myCurrRow, false) : NULL; }

	private:
		UITableElement *myTable;
		int myCurrRow;
		int myCurrCell;
	};

	CellsIterator cellsFirst() { CellsIterator rTempIter(*this); return rTempIter; }

	class SelectedCellsIterator
	{
	public:
		SelectedCellsIterator(UITableElement& pTable)
		{
			myTable = &pTable;
			myCurrRow = 0;
			myCurrCell = 0;
			if(!isEnd() && !myTable->myRows[myCurrRow].myCells[myCurrCell]->getIsSelected())
				advanceAndEnsureSelected();
		}

		SelectedCellsIterator()
		{
			myTable = NULL;
			myCurrRow = -1;
			myCurrCell = -1;
		}

		void operator++(int) 
		{ 
			advanceAndEnsureSelected();
		}
		void advanceAndEnsureSelected()
		{
			if(isEnd())
				return;

			do
			{
				myCurrCell++;
				if(myCurrCell >= myTable->myRows[myCurrRow].myCells.size())
				{
					myCurrCell = 0;
					myCurrRow++; 			
				}
			} while(!isEnd() && !myTable->myRows[myCurrRow].myCells[myCurrCell]->getIsSelected());
		}

		void toEnd() { myCurrRow = myTable->myRows.size(); }
		bool isEnd() const { return myCurrRow >= myTable->myRows.size(); }

		bool operator==(const SelectedCellsIterator& other) const { return myCurrRow == other.myCurrRow && myTable == other.myTable && myCurrCell == other.myCurrCell; }
		bool operator!=(const SelectedCellsIterator& other) const { return ! (*this == other); }

		inline int getRowIndex() const { return myCurrRow; }
		inline int getCellIndex() const { return myCurrCell; }
		inline UITableCellElement* getCell() { return myTable ? myTable->getCell(myCurrCell, myCurrRow, false) : NULL; }

	private:
		UITableElement *myTable;
		int myCurrRow;
		int myCurrCell;
	};

	SelectedCellsIterator selectedCellsFirst() { SelectedCellsIterator rTempIter(*this); return rTempIter; }

protected:

	virtual bool getAllowValuePropagation(FLOAT_TYPE fNewValue, bool bIsChangingContinuously, UIElement* pOptSourceElem, UIElement* pLinkedToElem);

	void updateRelatedSlider(void);

	bool getHasScrolled() { return myHasScrolled; }


	virtual void updateOwnData(SUpdateInfo& rRefreshInfo);

	virtual void onStartShowing(void);

	// Anything the real table wants to do before the rows are
	// created goes here.
	virtual void onPreCreateRows() { }
	virtual void onPostCreateRows();

	virtual void onCellRendered(UITableCellElement* pCell, const SVector2D& svCenter) { } 

	UITableCellElement* getCell(FLOAT_TYPE fX, FLOAT_TYPE fY);
	void animateScrollIfNeeded(FLOAT_TYPE fX, FLOAT_TYPE fY);

	virtual void setScroll(FLOAT_TYPE fScroll, bool bUseActualContentLimits = false);
	void setIsRowSelected(int iRow, bool bSelected, bool bIsComingFromUI);

	virtual void onRowSelected(int iRow, bool bIsSelected, bool bIsComingFromUI);
	virtual void onRowSelectedSubclass(int iRow, bool bIsSelected, bool bIsComingFromUI) { }
	virtual bool canSelectRow(int iRow) { return true; }

	virtual void onCellSelected(UITableCellElement* pCell) { }
	virtual void onCellDeselected(UITableCellElement* pCell) { }

	void recomputeStoredTableDims(void);
	virtual UIElement* getChildAtRecursiveInternal(const SVector2D& svPoint, const SVector2D& svScroll, bool bFirstNonPassthrough, bool bIgnoreChildren, bool bGlidingOnly);
	virtual const char* getCellType(int iColumn, int iRow);

	virtual void clearSavedStateInfo() { }

	virtual void clearRows(void);

	virtual bool receiveDragDrop(UIElement* pOther, SVector2D& svScreenPos);
	virtual void onDragDropHover(UIElement* pOther, SVector2D& svScreenPos);

	friend class RowsIterator;

	virtual void deselectAllRows(bool bIsComingFromUI);

	void createRows();

	friend class UITableCellElement;

private:

	UITableCellElement* createCell(int iColumn, int iRow);
	int getMaxScroll(void);

	int getCellSpacing(void);

protected:

	void clearBasic();
	void adjustCellsSize(int iOptWidth = -1, int iOptHeight = -1);

	void updateCellRelatedTableInfo(bool bCallAdjustToScreen, bool bCallSelectedCell);

	UIElement* getCellTargetDragTriggerChild(UITableCellElement* pCell);
	virtual bool onTableReceiveDragDrop(UIElement* pTargetElem, UIElement* pDragged, SVector2D& svScreenPos, AcceptType eAcceptType, bool bDoCopy) { return false; }
	virtual bool getAskCellsForSelection() { if(getCellInfoProvider() && getCellInfoProvider()->getAskSelfForCellSelection(this)) return true; else return false; }

	TUITableRows myRows;

	UNIQUEID_TYPE mySavedSelectedRowId;

	inline bool getIsDirty() const { return myIsDirty != TableRefreshClean; }

	virtual bool getAllowScrolling() { return true; }

	inline FLOAT_TYPE getInitScroll() const { return myInitScroll; }
	inline FLOAT_TYPE getScrollOffset() const { return myScrollOffset; }
	inline FLOAT_TYPE getTotalRowWidth() const { return myTotalRowWidth; }
	virtual bool getAllowScrollFromPoint(const SVector2D& svBeginPoint, FLOAT_TYPE fX, FLOAT_TYPE fY);
	virtual FLOAT_TYPE computeScrollFromPoint(const SVector2D& svBeginPoint, FLOAT_TYPE fX, FLOAT_TYPE fY);
	virtual FLOAT_TYPE computeScrollSpeedFromTrail(const SVector2D& svCurrPoint, FLOAT_TYPE fX, FLOAT_TYPE fY, GTIME lTime, GTIME lCurrPointTime);
	virtual void getInitCellPos(const SVector2D& svScroll, const SVector2D& svPos, FLOAT_TYPE fTotalScale, FLOAT_TYPE fSelfW, FLOAT_TYPE fSelfH, SVector2D& svInitPosOut);
	virtual int getMaxScrollInScrollAxisDirection(const SVector2D& svBoxSize, int iSpacing);

private:
	
	int myTotalRowWidth; // , myRowHeight;
	TableRefreshType myIsDirty;

#ifndef MAC_BUILD
	static 
#endif
		string theSharedString;

	bool myHasScrolled;
	SVector2D myBeginPressPos;
	GTIME myPressTime;
	SVector2D myLastLookbackPos, myLatestMouseMove;
	UITableCellElement* myPressedCell;
	UITableCellElement* mySelectedCell;
	int mySelectedRow, myPressedRow;

	UIElement* myLastMouseElement;
	UITableCellElement* myInitPressedCell;
	int myCellSpacing;

	int myInitScroll; // when pressed down
	FLOAT_TYPE myScrollOffset, myScrollSpeed;

	PathTrail myMouseTrail;

	bool myIgnoreSlider;

	bool myAllowScrolling;

	TBaseCellStateInfos myBaseStateInfos;

	// Drag-and-drop markers for rendering.
	UNIQUEID_TYPE myDragHoverTargetCellId;
	AcceptType myDragAcceptType;

	FLOAT_TYPE myAutoscrollDir;
	bool myIsCallingFromReleased;

	UIElement* myOnReleaseFinalChild;

	TElementAlphaMapMemCacher myElementsMapMemCacher;
	//TElementAlphaMap myTempGeneratedNames;

#ifdef ENABLE_TABLE_LOCK
	boost::recursive_mutex myLock;
#endif

	ResourceItem* myLastCellDef;
	string myLastCellDefType;
};
/*****************************************************************************/
