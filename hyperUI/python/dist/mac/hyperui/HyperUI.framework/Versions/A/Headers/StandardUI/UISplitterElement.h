#pragma once

/*****************************************************************************/
enum SplitterSnapType
{
	SplitterSnapNear = 0,
	SplitterSnapNone,
	SplitterSnapFar,
};
/*****************************************************************************/
class HYPERUI_API UISplitterElement : public UIElement
{
public:

	DECLARE_STANDARD_UIELEMENT(UISplitterElement, UiElemSplitter);

    virtual void postInit(void);
    virtual void postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

    virtual void onPressed(TTouchVector& vecTouches);
    virtual void onMouseLeave(TTouchVector& vecTouches);
    virtual void onMouseEnter(TTouchVector& vecTouches);
    virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);
    virtual void onMouseMove(TTouchVector& vecTouches);
	virtual void onCursorGlide(TTouchVector& vecTouches);

    virtual void adjustToScreen(void);
    virtual UIElement* getChildAtRecursiveInternal(const SVector2D& svPoint, const SVector2D& svScroll, bool bFirstNonPassthrough, bool bIgnoreChildren, bool bGlidingOnly);

	virtual void getMinDims(SVector2D& svDimsOut) const;
	virtual void getMaxDims(SVector2D& svDimsOut) const;

	void setLeftPaneSize(FLOAT_TYPE fPixelSize);

	void snapTowardFarEnd();
	void snapTowardNearEnd();
	// If we're toggling several splitters at the same time,
	// we need to first set the state and only then perform the 
	// update. Otherwise, the positions of the nested sliders will change
	// in ensure() before their state is set to far/near, and on return
	// the perc will be differrent.
	void setNewSnapState(SplitterSnapType eState, bool bPerformUpdate, bool bManageDisabledCompletelyFlag = false);
	void onSnapStateChanged();

	inline SplitterSnapType getCurrentSnapState() const { return mySnapState; }

 	bool getIsVertical() const;

	void saveCoreInfoInto(ResourceItem* pOwnItem) const;
	void loadCoreInfoFrom(const ResourceItem* pOwnItem);

	inline void setIsAdjustingFromSavedNormalPosition(bool bValue) { myIsAdjustingFromSavedNormalPosition = bValue; }

	bool getIsChildPaneVisible(const UIElement* pChild) const;

protected:

    void getHandleRectInScreenCoords(SRect2D& srOut);
	void getToFarButtonRectInScreenCoords(SRect2D& srOut);
	void getToNearButtonRectInScreenCoords(SRect2D& srOut);
    virtual void autoLayoutChildren();

    bool getIsFixed();
	virtual CursorType getOwnCursorType() const;

	FLOAT_TYPE getCurrPositionPerc();

	void ensurePositionObeysMinMaxSizes(const SVector2D& svOwnSize);

	void ensurePositionObeysMinSizes(const SVector2D& svOwnSize);
	void ensurePositionObeysMaxSizes(const SVector2D& svOwnSize);

	inline void setPositionInternal(FLOAT_TYPE fValue) { myPosition = fValue; }

private:
	void postRenderSplitter(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

private:

    string myBarAnim;
    string myHandleAnim;

    FLOAT_TYPE myPosition;
	FLOAT_TYPE mySavedNormalPosition;
	SplitterSnapType mySnapState;

    bool myIsDraggingHandle;
    SVector2D myStartPos;
    FLOAT_TYPE myStartSepPos;

	int myFixedChildIndex;

	bool myDidPressButton;

	bool myIsAdjustingFromSavedNormalPosition;
	bool myDisableResizing;
};
/*****************************************************************************/
