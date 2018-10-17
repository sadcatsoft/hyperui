#pragma once

#define SEPARATOR_ID				"__sep__"
#define DEBUG_MENU_SUFFIX			"_DEBUGONLY"
#define NONFULLONLY_MENU_SUFFIX		"_NONFULLONLY"
#define NONAPPSTOREONLY_MENU_SUFFIX "_NONAPPSTOREONLY"

#define MENU_HIGHLIGHT_FADE_DURATION		0.15

class UIMenuElement;
/*****************************************************************************/
struct HYPERUI_API SMenuEntry
{
    string myId;
    string myLabel;
    string myAction;
	string myActionValue;
	string myActionValue2;
	string myActionValue3;
    string mySubmenu;
    string myIcon;
	string myShortcut;

    SVector2D myPosOffset;
    SVector2D myTotalSize;
    FLOAT_TYPE myIconX, myTextX;

	bool myIsEnabled;

	bool myIsCheckmark;
	bool myIsChecked;

	// Used for tracking OS X menu items
	int myMacTag;

	AnimatedValue	myHighlightAnim;

	void select() 
	{ 
#ifdef _DEBUG
		myHighlightAnim.setDebugName(myLabel.c_str());
#endif
		myHighlightAnim.changeAnimation(0, 1, MENU_HIGHLIGHT_FADE_DURATION, ClockUiPrimary); 
	}
	void deselect(bool bInstant) 
	{ 
#ifdef _DEBUG
		myHighlightAnim.setDebugName(myLabel.c_str());
#endif
		if(bInstant)
			myHighlightAnim.setNonAnimValue(0);
		else
			myHighlightAnim.changeAnimation(1, 0, MENU_HIGHLIGHT_FADE_DURATION, ClockUiPrimary); 
	}
};
typedef vector < SMenuEntry > TMenuEntriesVector;
/*****************************************************************************/
class HYPERUI_API IMenuVisitor
{
public:
	IMenuVisitor() {  }
	virtual ~IMenuVisitor() { }

	virtual void visit(SMenuEntry* pEntry, UIMenuElement* pParent, int iEntryIndex) = 0;
};
/*****************************************************************************/
class HYPERUI_API UIMenuElement: public UIElement
{
public:

	DECLARE_STANDARD_UIELEMENT_NO_DESTRUCTOR_DEF(UIMenuElement, UiElemMenu);
	virtual void onDeallocated(void);

	virtual void postInit(void);
	virtual void postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	virtual void onPressed(TTouchVector& vecTouches);
	virtual void onMouseLeave(TTouchVector& vecTouches);
	virtual void onMouseEnter(TTouchVector& vecTouches);
	virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);
	virtual void onCursorGlide(TTouchVector& vecTouches);
	virtual void onCursorGlideLeave(TTouchVector& vecTouches);

	// DO NOT USE. Except as a hack.
	inline void setParentMenuId(const char* pcsValue) { myParentMenuId = pcsValue; }
	inline const char* getParentMenuId() const { return myParentMenuId.c_str(); }

	void setDoShowChildrenOnGlide(bool bValue) { myDoShowChildrenOnGlide = bValue;	}
	virtual bool getDoShowChildrenOnGlide();
	UIMenuElement* getTopLevelMenuParent();

	FLOAT_TYPE initMenuFrom(IMenuContentsProvider* pOptContentsProvider = NULL, FLOAT_TYPE fOptWidth = 0, FLOAT_TYPE fOptMaxHeight = 0, FLOAT_TYPE fOptMaxTextLength = 0);
	void setMenuPosition(SVector2D& svPos);

	virtual void onStartShowing(void);

	inline void setOwnedEnableStatusCallback(const IMenuEnableStatusCallback* pCallback) { _ASSERT(myEnableStatusCalllback == NULL || myEnableStatusCalllback != pCallback); delete myEnableStatusCalllback; myEnableStatusCalllback = pCallback; }
	inline const IMenuEnableStatusCallback* getOwnedEnableStatusCallback() { return myEnableStatusCalllback; }
	void getHierarchyChain(UIElement* pStartElem, TUIElementVector& rVecOut);

	inline void setExtraHidingException(const char* pcsTypeId) { if(IS_VALID_STRING(pcsTypeId)) myExtraHidingException = pcsTypeId; else myExtraHidingException = ""; }
	UIElement* getExtraHidingException();

	inline const TMenuEntriesVector* getMenuEntries() const { return &myEntries; }

	virtual void onTimerTick(GTIME lGlobalTime);

	int getEntryById(const char* pcsId);
	void setPrevSelectedEntry(int iIndex) { myPrevSelectedEntry = iIndex; }
	const SMenuEntry* getEntry(int iIndex) const;
	inline int getNumEntries() const { return myEntries.size(); }

	bool openSubmenu(SMenuEntry* pEntry, bool bAlwaysCloseOthers, bool bCloseIfAlreadyOpen);
	virtual void setupAsSubmenu(UIMenuElement* pParentMenu, SMenuEntry* pEntry);
	void hideAllButSelfChain();

	inline void markAsDirty() { myIsDirty = true; }
	void refreshMenuItemEnabledStatus(bool bForce = false);

	void iterateOverMenuItems(IMenuVisitor& rVisitor);

	void executEntry(const SMenuEntry* pEntry);

	void setEntryTag(int iEntryIndex, int iTag);

	int findSubmenuIndex(const char* pcsId);

	UIMenuElement* getParentMenuElem();

	void removeItem(int iIndex);

protected:

	virtual void onFinishHiding();
	virtual void getSelectionClickExclusionRectangle(SRect2D& srOut) { srOut.w = srOut.h = 0.0; }

	int getEntryAt(FLOAT_TYPE fX, FLOAT_TYPE fY, const SRect2D* pOptExclusionRect = NULL);

	void killHighlightRecursiveUp();
	void killAllHighlights();

	virtual void customMenuItemAction(const char* pcsItemId) {  } 

	FLOAT_TYPE getCommonHorOffset();
	void getHighlightPadding(SVector2D& svHighlightPadding);
	void getHighlightOffset(SVector2D& svHighlightOffset);

	//virtual bool getIsMenuEntryEnabledSubclass(const SMenuEntry* pEntry) { return true; }
	//virtual bool getIsMenuEntryCheckedSubclass(const SMenuEntry* pEntry) { return false; }

	virtual void getAutoScrollingRectangleSizeAndShiftOffsets(SVector2D& svSizeOffsetOut, SVector2D& svShiftOffsetOut) { svSizeOffsetOut.set(0, 0); svShiftOffsetOut.set(0, 0); }
	virtual bool getDoScissorRect() const { return false; }

	void ensureShortcutsValid();

	void markShortcutsAsDirty() { myAreShortcutsDirty = true; }

	//virtual IMenuContentsProvider* getMenuContentsProvider() { return &myDefaultProvider; }
	const IMenuEnableStatusCallback* getEnableStatusCallback();

	void refreshEnableStatusFor(SMenuEntry* pEntry);

	UIElement* getSubmenuForName(const char* pcsSubmenuName);

	void onMenuStatusRefreshed();
	void onMenuShortcutsChanged();
	void onMenuChanged();

protected:

    TMenuEntriesVector myEntries;

private:

    bool myAllowPressing;
	bool myAreShortcutsDirty;
	bool myIsDirty;

    int myHighlightEntry;
    UIButtonElement myDummyButton;

	// Note that this is not set until the submenu is actually shown...
	string myParentMenuId;

	TUIElementVector theElemVector;

	bool myDoShowChildrenOnGlide;
	ResourceItemMenuProvider myDefaultProvider;
	const IMenuEnableStatusCallback* myEnableStatusCalllback;

	int myNoIconSpacing;
	FLOAT_TYPE myIconScale;
	
	// HACK to not hide the other menu when two are shown at the
	// same time. This is used for RMB menus with layer menus
	// for tools that have their own RMB menu.
	string myExtraHidingException;

	// Entry to draw in text sel color. Used for dropdowns, -1
	// for the rest.
	int myPrevSelectedEntry;
};
/*****************************************************************************/
