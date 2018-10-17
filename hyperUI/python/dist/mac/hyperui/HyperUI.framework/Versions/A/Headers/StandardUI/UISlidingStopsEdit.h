#pragma once

// Corresponds to the actual handle image width, hence here.
#define HANDLE_VERTICAL_SPACE					upToScreen(20.0)

class UISlidingStopsEdit;
/*****************************************************************************/
enum SlidingStopsSelectionType
{
	SlidingStopsSelectionNone = 0,
	SlidingStopsSelectionSingle,
	SlidingStopsSelectionMultiple
};
/*****************************************************************************/
class HYPERUI_API UISliderStop : public IBaseObject
{
public:
	UISliderStop() { myIsSelected = false; myDragStartPos = -1; }
	virtual ~UISliderStop() { }

	inline bool getIsSelected() const { return myIsSelected; }
	inline void setIsSelected(bool bValue) { myIsSelected = bValue; }

	int myId;

	virtual void getSelectedAnim(string& strOut) = 0;
	virtual void getDeselectedAnim(string& strOut) = 0;

private:

	friend class UISlidingStopsEdit;

	bool myIsSelected;
	FLOAT_TYPE myDragStartPos;
};
typedef vector < UISliderStop* > TUIGradientStops;
/*****************************************************************************/
class HYPERUI_API UISlidingStopsEdit : public UIElement
{
public:
	DECLARE_VIRTUAL_STANDARD_UIELEMENT_NO_DESTRUCTOR_DEF(UISlidingStopsEdit, UiElemSlidingStopsEdit);
	virtual void onDeallocated();

	virtual void onPressed(TTouchVector& vecTouches);
	virtual void onMouseEnter(TTouchVector& vecTouches);
	virtual void onMouseLeave(TTouchVector& vecTouches);
	virtual void onMouseMove(TTouchVector& vecTouches);
	virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);
	virtual void onDoubleClick(TTouchVector& vecTouches, bool bIgnoreActions);

	virtual bool onKeyUp(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl);
	virtual bool getAllowKeepingFocus() { return true; }

	virtual void postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	virtual void updateOwnData(SUpdateInfo& rRefreshInfo);

protected:

	virtual bool getAllowStopAddition() { return false; }
	virtual bool getAllowStopDeletion() { return false; }
	// Returns true if the point is in the region where a new stop can be added
	virtual bool getIsPointInAdditionRegion(const SVector2D& svPoint) { return false; }

	virtual void removeStopCallback(UISliderStop* pStop) { }
	virtual void addNewStop(FLOAT_TYPE fPos, const SVector2D svTouchPoint) { }
	virtual void drawCustomStopInnerContents(UISliderStop* pStop, const SVector2D& svStopCenter, FLOAT_TYPE fOpacity) { }
	virtual bool nudgeStopValue(UISliderStop* pStop, FLOAT_TYPE fByNormAmount) { return false; }

	virtual void recreateAllStops() = 0;
	virtual void refreshStopRelatedUI(UISliderStop* pStopIn, SlidingStopsSelectionType eSelTypeGeneric, bool bIsChangingContinuously) = 0;

	virtual FLOAT_TYPE getGradAreaStartX() = 0;
	virtual FLOAT_TYPE getGradAreaWidth() = 0;
	// Returns true to render text, false otherwise
	virtual bool getStopText(UISliderStop* pStop, string& strOut) = 0;
	virtual void moveStopTo(UISliderStop* pUiStop, FLOAT_TYPE fNewPos) = 0;
	virtual FLOAT_TYPE getStopPos(UISliderStop* pUiStop) = 0;
	virtual void getStopCenter(UISliderStop* pUiStop, const SVector2D& svStopSize, SVector2D& svCenterOut ) = 0;
	virtual void getInnerTextOffset(SVector2D& svOffsetOut) = 0;

	virtual void onStopDoubleClicked(UISliderStop* pUiStop, TTouchVector& vecTouches) { }

	void deselectAllStops();
	SlidingStopsSelectionType getSelectionType(int& iSingleSelIdxOut);
	int getStopAt(const SVector2D& svPoint);
	bool removeSelected();
	void finishUndo();
	void ensureUndoStarted();
	bool getWillRemoveSelected(const SVector2D& svMousePos, bool bHaveSelection);
	void getStopRectangle(UISliderStop* pUiStop, SRect2D& srOut);
	bool nudgeStop(UISliderStop* pStop, KeyMeaningType eKeyMeaning, FLOAT_TYPE fMoveDist);
	void clearAllStops() { clearAndDeleteContainer(myStops); }

	void refreshSelectedStopRelatedUI(bool bIsChangingContinuously);

	virtual void onUndoBegan() { }

	void onReleasedCommon(TTouchVector& vecTouches, bool bIgnoreActions);

protected:
	TUIGradientStops myStops;
	bool myWillRemoveSelected;
	UNIQUEID_TYPE myPressedStopId;
	UNIQUEID_TYPE myCurrUndoBlockId;

	bool myDidHaveSelectionOnPress;
	SVector2D myMouseDownPos;

	bool myIsRefreshingUI;

	bool myDidSendChagneBeginMessage;
};
/*****************************************************************************/