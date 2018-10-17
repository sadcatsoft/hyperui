#pragma once

#define TAB_CLOSE_HOVER_ANIM		0.10

class UITabWindowElement;
/*****************************************************************************/
struct HYPERUI_API STabInfo
{
    SRect2D myRect;
    string myText;
	string myIcon;
	SVector2D myIconsDims;
	bool myNoCloseButton;

	void fadeInCloseHover() {  myCloseHoverAnim.changeAnimation(0, 1, TAB_CLOSE_HOVER_ANIM, ClockUiPrimary); }
	void fadeOutCloseHover() {  myCloseHoverAnim.changeAnimation(1, 0, TAB_CLOSE_HOVER_ANIM, ClockUiPrimary); }

	AnimatedValue myCloseHoverAnim;
};
typedef vector < STabInfo > TTabInfos;
/*****************************************************************************/
class HYPERUI_API UITabWindowElement : public UIElement
{
public:

	DECLARE_STANDARD_UIELEMENT(UITabWindowElement, UiElemTabWindow);

    virtual void adjustToScreen(void);
    virtual void postInit(void);

    virtual void postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

    virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);
	virtual void onMouseLeave(TTouchVector& vecTouches);
	virtual void onDoubleClick(TTouchVector& vecTouches, bool bIgnoreActions);

	virtual void onCursorGlide(TTouchVector& vecTouches);
	virtual void onCursorGlideLeave(TTouchVector& vecTouches);

	virtual void onTimerTick(GTIME lGlobalTime);

    void closeTab(int iTabIdx, bool bDeleteElement = true);
    void switchToTab(int iTabIdx);
    int getActiveTabIndex();

    template < class TYPE >
    TYPE* addNewTab(bool bSwitchTo, const char* optOverrideTabTemplate = NULL) { return dynamic_cast<TYPE*>(addNewTabInternal(bSwitchTo, optOverrideTabTemplate)); }
	void addNewTab(UIElement* pExistingElem, bool bSwitchTo);

    void recomputeTabInfos();
	//void refreshTitleFor(UIElement* pElem);

	virtual void getMinDims(SVector2D& svDimsOut) const;
	virtual void getMaxDims(SVector2D& svDimsOut) const;

	virtual bool onKeyUp(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl);

	virtual void onDragDropHover(UIElement* pOther, SVector2D& svScreenPos);
	virtual bool receiveDragDrop(UIElement* pOther, SVector2D& svScreenPos);

	virtual bool allowDragDropStart(SVector2D& svPos);

	virtual FLOAT_TYPE getMinDistanceBeforeDragStart() const { return upToScreen(5.0); }

	virtual void onDragDropFinished(UIElement* pReceiver) { myTabBeingDragged = -1; }
	virtual void onDragDropCancelled(UIElement* pReceiver) { myTabBeingDragged = -1; }

	inline int getTabBeingDragged() const { return myTabBeingDragged; }

	bool adoptTab(UITabWindowElement* pOtherTabWindow, int iTabIndex);

	virtual bool getDoHaveToolTip(const SVector2D& svScreenCoords) const;
	virtual void getToolTip(const SVector2D& svScreenCoords, string& strTipOut) const;

	void readjustClosestSplitterParent();

protected:

	void cancelDragDrop(UIElement* pExcept);

    UIElement* addNewTabInternal(bool bSwitchTo, const char* optOverrideTabTemplate);
    int getTabAt(FLOAT_TYPE fX, FLOAT_TYPE fY, bool& bOnCloseButton);
	void getTabRect(int iTabIndex, const SVector2D& svOwnOrigin, SRect2D& srRectOut);

    virtual void autoLayoutChildren();

    FLOAT_TYPE getTabRowHeight(const char* pcsAnim, bool bWithPadding);

	friend class UITabRenderer;

	inline const char* getTabAnim() const { return myTabAnim.c_str(); }
	inline const char* getSelTabAnim() const { return mySelTabAnim.c_str(); }
	inline const char* getTabCloseAnim() const { return myTabCloseAnim.c_str(); }
	inline const char* getTabCloseHoverAnim() const { return myTabCloseHoverAnim.c_str(); }

	inline bool getIsDraggingOverTabRow() const { return myIsDraggingOverTabRow; }

	void computeTabRenderCommonInfo(FLOAT_TYPE fScale, const SVector2D& svScroll, FLOAT_TYPE fOpacity, STabRenderCommonInfo& rInfoOut);

	inline STabInfo* getTabInfo(int iIndex) { return &myTabs[iIndex]; }

	void readjustScroll();

private:

    string myTabAnim;
    string mySelTabAnim;
    string myTabCloseAnim;
	string myTabCloseHoverAnim;

	int myLastHoverTabCloseButton;

    TTabInfos myTabs;

	TPointVector myLineVecs[2];
	
	bool myIsDraggingOverTabRow;

	int myTabBeingDragged;

	FLOAT_TYPE myHorTabScroll;
};
/*****************************************************************************/
