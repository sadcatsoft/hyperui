#pragma once

// If defined, D&D is started when the mouse leaves the element.
// Otherwise, it's started after it moves, pressed, a short radius
// from orig location.
//#define INITIATE_DRAG_AND_DROP_ON_MOUSE_LEAVE
#define DRAG_DROP_MIN_DIST upToScreen(20.0)  //3.0

// These are for active ui elements

//#define SCROLLER_LOOKBACK_TICKS			((GTIME)(GAME_FRAMERATE/30.0))	// 15.0
#define SCROLLER_LOOKBACK_TICKS			((GTIME)(GAME_FRAMERATE/30.0))	// 15.0
#define SCROLL_SPEED_DECAY				0.05 // per tick 0.05
#define PADDING_SCROLL_RETURN_RATE		2.75 // 3

// Size of the weapon/equip holder cell
#define WEAP_MOUNT_SIZE				upToScreen(57)
#define WEAP_MOUNT_ANIM				"levChoiceIcons"
#define WEAP_SELECTION_ANIM			"storeWeapSelection"
#define INVENTORY_STORE_ICON_SCALE	1.0
#define MIN_POSITION_BEFORE_DRAGGING		upToScreen(12)

#define DISABLED_OPACITY_MULT		0.52 // 0.65

#define PARM_HOST_ELEM_ID		"parmHost"

#define MOUSEWHEEL_SCROLL		upToScreen(21)

class UIPlane;
class UIElement;
class UIButtonElement;
class UISliderElement;
class UIElement;
class UISplitterElement;
class IUndoItem;
class IGenericIdentifier;
class Window;

DEFINE_ALPHA_TREE_MEMCACHER(UIElement*) TElementAlphaMapMemCacher;
typedef AlphaTreeMap < UIElement* > TElementAlphaMap;

// Declares a custom UI element with empty default constructor
#define DECLARE_UIELEMENT(elementClass)	\
	elementClass(HyperUI::UIPlane* pParentPlane); \
	virtual ~elementClass(); \
	virtual void onAllocated(IBaseObject* pData); \
	virtual void onDeallocated(); \
	static HyperUI::UIElement* allocateSelf(HyperUI::UIPlane* pParentPlane) { return new elementClass(pParentPlane); } \
	static const char* getTypeString(void) { return #elementClass; }

// Declares a custom UI element with the default constructor to be implemented in the body
#define DECLARE_UIELEMENT_NO_CONSTRUCTOR_DEF(elementClass)	\
	elementClass(HyperUI::UIPlane* pParentPlane); \
	virtual ~elementClass(); \
	virtual void onAllocated(IBaseObject* pData); \
	virtual void onDeallocated(); \
	static HyperUI::UIElement* allocateSelf(HyperUI::UIPlane* pParentPlane) { return new elementClass(pParentPlane); } \
	static const char* getTypeString(void) { return #elementClass; }

#define DECLARE_STANDARD_UIELEMENT(elementClass, elementEnumType)	\
	elementClass(HyperUI::UIPlane* pParentPlane); \
	virtual ~elementClass() { } \
	static HyperUI::UIElement* allocateSelf(HyperUI::UIPlane* pParentPlane) { return new elementClass(pParentPlane); } \
	virtual HyperUI::UiElemType getElemType() const { return elementEnumType; } \
	virtual void onAllocated(IBaseObject* pData);

#define DECLARE_STANDARD_UIELEMENT_NO_DESTRUCTOR_DEF(elementClass, elementEnumType)	\
	elementClass(HyperUI::UIPlane* pParentPlane); \
	virtual ~elementClass(); \
	static HyperUI::UIElement* allocateSelf(HyperUI::UIPlane* pParentPlane) { return new elementClass(pParentPlane); } \
	virtual HyperUI::UiElemType getElemType() const { return elementEnumType; } \
	virtual void onAllocated(IBaseObject* pData);

#define DECLARE_STANDARD_UIELEMENT_NO_CONSTRUCTOR_DESTRUCTOR_DEF(elementClass, elementEnumType)	\
	elementClass(HyperUI::UIPlane* pParentPlane); \
	virtual ~elementClass(); \
	static HyperUI::UIElement* allocateSelf(HyperUI::UIPlane* pParentPlane) { return new elementClass(pParentPlane); } \
	virtual HyperUI::UiElemType getElemType() const { return elementEnumType; } \
	virtual void onAllocated(IBaseObject* pData);


#define DECLARE_VIRTUAL_STANDARD_UIELEMENT_NO_DESTRUCTOR_DEF(elementClass, elementEnumType)	\
	elementClass(HyperUI::UIPlane* pParentPlane); \
	virtual ~elementClass(); \
	virtual HyperUI::UiElemType getElemType() const { return elementEnumType; } \
	virtual void onAllocated(IBaseObject* pData);


enum RenderingPassType
{
	RenderingNormal = 0,
	RenderingPostopnedElements,
};

typedef HashMap < const char*, RESOURCEITEM_STRING_TYPE, int, 64 > TUIElemsUniqueCountHash;

/*****************************************************************************/
class UIElement;

typedef vector < UIElement* > TUIElementVector;
typedef set < UIElement* > TUIElementSet;
typedef map < string, UIElement* > TStringUIElementMap;
/*****************************************************************************/
struct SChildSizingInfo
{
    SChildSizingInfo() { reset(); }
    void reset() { myPixelWidth = 0; myCumulStartPos = 0; myHorMargins[0] = myHorMargins[1] = 0; myIsHidden = false; }

    FLOAT_TYPE myPixelWidth;
	FLOAT_TYPE myHorMargins[2];
    FLOAT_TYPE myCumulStartPos;
	bool myIsHidden;
};
typedef vector < SChildSizingInfo > TChildSizingInfos;
/*****************************************************************************/
struct SUpdateInfo
{
	SUpdateInfo()
	{
		myDataValid = false;
		myPosition = -1;
		myIsError = false;
		myMarkTableDirty = true;
		myIsSliding = false;
		mySourceEventType = EventTabChanged;
	}

	// Position - either slider or table row, or whatever we want it to mean.
	// The window is responsible for interpreting this itself.
	int myPosition;
	
	// A generic thing to indicate whether the data needed by the table is valid
	// or not. Useful for things where the first refresh triggers the loading
	// of the data, and the second, manual one, actually shows the data.
	// Currently used for global scores. Needs to be false by default since
	// the initial refresh is done automatically, and the data is assumed to be
	// invalid. Notice that this is only meaningful for those windows which
	// take the flag into account.
	bool myDataValid;
	
	bool myIsError;
	
	bool myMarkTableDirty;

	// This is used to tell the window whether we're refreshing because
	// we're sliding, or whether we're refreshing because we're showing.
	bool myIsSliding;

	EventType mySourceEventType;
};
/*****************************************************************************/
class HYPERUI_API LinkedToSelfElemIterator : public TDepthfirstResourceItemIterator<UIElement>
{
public:
	LinkedToSelfElemIterator() : TDepthfirstResourceItemIterator<UIElement>() { }
	LinkedToSelfElemIterator(UIElement* pStart, UIElement* pLinkTarget) : TDepthfirstResourceItemIterator<UIElement>(pStart) { myLinkTarget = pLinkTarget; ensureSatisfiesConditions(); }
	virtual ~LinkedToSelfElemIterator() { }
	virtual bool getDoesSatisfyConditions(UIElement* pItem);

private:
	UIElement* myLinkTarget;
};
/*****************************************************************************/
class HYPERUI_API UIElement : public IPlaneObject, public IThumbnailRenderCallback
{
public:
	UIElement() : IPlaneObject(NULL) { }
	UIElement(UIPlane* pParentPlane);
	virtual void onAllocated(IBaseObject* pData);
	virtual ~UIElement();

	static UIElement* allocateSelf(UIPlane* pParentPlane) { return new UIElement(pParentPlane); }

	virtual UIElement* setTextForChild(const char* pcsChildId, const char* pcsText, SColor* pOptColor = NULL);
	virtual UIElement* setTextForChild(const char* pcsChildId, const char* pcsSubChildId, const char* pcsText, SColor* pOptColor = NULL);
	virtual UIElement* setTextForChild(const char* pcsChildId, string& strText, SColor* pOptColor = NULL, FLOAT_TYPE fShortenTextMaxLen = 0);
	UIElement* setNumericValueForChild(const char* pcsChildId, const char* pcsSubChildId, FLOAT_TYPE fValue, UnitType eUnits);
	FLOAT_TYPE getNumericValueForChild(const char* pcsChildId, const char* pcsSubChildId, UnitType eUnits);
	virtual UIElement* setIsVisibleForChild(const char* pcsChildId, bool bIsVisible);
	UIElement* setIsEnabledForChild(const char* pcsChildId, bool bIsEnabled);
	UIElement* setTopAnimForChild(const char* pcsChildId, const char* pcsTopAnim);

	virtual void postInit();

	UIPlane* getUIPlane();
	template < class TYPE > TYPE* getUIPlane()
	{
		return dynamic_cast<TYPE*>(this->getParentPlane());
	}

	// Resets own variables unique to UIElement.
	void resetUiElemCachedVars(bool bRecursive);
	void resetAllCaches(bool bRecursive);
	void resetUiTablesRecursive();

	virtual void onTimerTick(GTIME lGlobalTime);
	virtual void render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	virtual const char* getTitle();

	virtual AnimatedPoint* getPosition() { return NULL; }

	void getLocalPosition(SVector2D& svOut, FLOAT_TYPE* fOpacityOut = NULL, FLOAT_TYPE* fScaleOut = NULL);
	virtual void getGlobalPosition(SVector2D& svOut, FLOAT_TYPE* fOpacityOut = NULL, FLOAT_TYPE* fScaleOut = NULL);

	// Note: Pressed and released are only called when the mouse button is pressed
	// or released over an element; leave/enter events are called any time the mouse
	// enters it. The coords are in window space.
	virtual void onPressed(TTouchVector& vecTouches);
	virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);
	virtual void onMouseEnter(TTouchVector& vecTouches);
	virtual void onMouseLeave(TTouchVector& vecTouches);
	virtual void onMouseMove(TTouchVector& vecTouches);
	virtual void onDoubleClick(TTouchVector& vecTouches, bool bIgnoreActions);
	// These are called for cursor moves when the mouse isn't
	// down.
	virtual void onCursorGlideEnter(TTouchVector& vecTouches);
	virtual void onCursorGlideLeave(TTouchVector& vecTouches);
	virtual void onCursorGlide(TTouchVector& vecTouches) { } 
	// Called with a signed distance of the amount of wheel scroll.
	void onMouseWheelRecursive(FLOAT_TYPE fDelta);

	bool isInsideLocal(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bClickableOnly);

	void show(bool bImmediate = false, AnimationOverCallback* pCallBack = NULL, AnimOverActionType eAnimOverAction = AnimOverActionNone, bool bResetElements = true);
	void hide(bool bImmediate = false, AnimationOverCallback* pCallBack = NULL, AnimOverActionType eAnimOverAction = AnimOverActionNone, FLOAT_TYPE fExtraOffset = 0.0);
	void setAllChildrenInvisible();

	bool getIsFullyShown();
	bool getIsFullyHidden();
	bool getIsFullyHiddenCumulative();
	bool getIsAnimating();
	// True if this element is being shown BUT IS NOT completely shown.
	bool getIsBeingShown();
	// True if this element is being hidden BUT IS NOT completely hidden.
	bool getIsBeingHidden();

	virtual UiElemType getElemType() const { return UiElemStatic; }

	virtual UIElement* getChildAtRecursive(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bFirstNonPassthrough, bool bGlidingOnly);

	inline bool getIsMousePressed() const { return myIsMousePressed; }
	void defineLayersOnChildren(const char* pcsLayerName, int iLevel = 0);

	template < class T >
	T* getChildById(const char* pcsId, bool bRecurse = true, bool bIncludeOldDefId = true)
	{
		return dynamic_cast<T*>(getChildById(pcsId, bRecurse, bIncludeOldDefId));
	}

	template < class T >
	T* getChildByElemType(const char* pcsElemType, bool bRecurse = true)
	{
	    return dynamic_cast<T*>(getChildByElemType(pcsElemType, bRecurse));
	}

	UIElement* getChildById(const char *pcsId, bool bRecurse = true, bool bIncludeOldDefId = true);
	UIElement* getChildByElemType(const char *pcsElemType, bool bRecurse = true);
	void slide(int iDir, int iTableRowCalling, bool bIsVertical);

	virtual bool getIsTextSelected() const { return false; }
	inline bool getIsVisible() const { return myIsVisible; }
	bool getIsVisibleCumulative() const;
	void setIsVisible(bool bValue);
	bool allowRendering();
	inline void setIsRenderable(bool bValue) { myIsRenderable = bValue; }

	void positionAsPopupRelativeTo(UIElement* pRelativeToElem, SideType eSide, FLOAT_TYPE fOffset, bool bRunRefresh = true);
	void positionAsPopupRelativeTo(const SVector2D& svPoint, SideType eSide, FLOAT_TYPE fOffset, bool bExtraOffsetSideways, bool bRunRefresh = true);
	void resetSliderChildrenVisibility();
	void setGenericDataSource(const char* pcsValue);
	inline bool getHaveGenericDataSource() const { return myGenericDataSource && myGenericDataSource->length() > 0; }
	inline const char* getGenericDataSource() { if(myGenericDataSource) return myGenericDataSource->c_str(); else return ""; }

	virtual int getNumInfiniteSlides() { return 2; }
	inline void updateDataRecursive() { SUpdateInfo rRefreshInfo; this->updateDataRecursive(rRefreshInfo); }
	void updateDataRecursive(SUpdateInfo& rRefreshInfo);

	void updateEverything();

	// Updates our enable/disable status
	virtual void updateEnableStatusRecursive();
	// Updates our visible/invisible status
	void updateVisibleStatusRecursive();

	//virtual void animationOver(AnimatedValue *pAValue);

	bool isChildLeaving(UIElement* pChild);
	bool isChildEntering(UIElement* pChild);
	
	void resetPosCache(bool bRecursive);
	virtual void setText(const CHAR_TYPE* pcsText);

	void setTextExt(const CHAR_TYPE* format, ...);

	template < class T > 
	T* getChildByClass(bool bRecursive = true)
	{
		UIElement* pChild;
		int iCurrChild, iNumChildren = this->getNumChildren();
		T *pCElem;
		for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		{
			pChild = dynamic_cast<UIElement*>(this->getChild(iCurrChild));
			pCElem = dynamic_cast<T*>(pChild);
			if(pCElem)
				return pCElem;
			else if(pChild && bRecursive)
			{
				T* pRes = pChild->getChildByClass<T>();
				if(pRes)
					return pRes;
			}
		}
		return NULL;
	}

	int getSliderPosition();
	virtual bool allowSliding(int iDir) { return true; }

	template < class T >
	T* getCurrentSlideElement()
	{
		const char* pcsCurrChildName = this->getEnumPropValue(PropertyUioSlidingChildren, myCurrentSliderIndex);
		return dynamic_cast<T*>(this->getChildById(pcsCurrChildName, false));
	}

	template < class T >
	T* getTopmostParent()
	{
	    ResourceItem* pPrevItem = NULL;
	    ResourceItem* pItem = this->getParent();
	    while(pItem)
	    {
		pPrevItem = pItem;
		pItem = pItem->getParent();
	    }
	    return dynamic_cast<T*>(pPrevItem);
	}

	template < class T >
	T* getParentOfType()
	{
		T* pRes;
		ResourceItem* pItem = this->getParent();
		while(pItem)
		{
			pRes = dynamic_cast<T*>(pItem);
			if(pRes)
				return pRes;
			pItem = pItem->getParent();
		}
		return NULL;
	}

	bool getHasElementAsParent(const UIElement* pOtherElem)
	{
		UIElement* pRes;
		ResourceItem* pItem = this->getParent();
		while(pItem)
		{
			pRes = dynamic_cast<UIElement*>(pItem);
			if(pRes == pOtherElem)
				return true;
			pItem = pItem->getParent();
		}
		return false;
	}

	template < class T >
	T* getParentTagged(const char* pcsTag)
	{
		ResourceItem* pItem = this->getParent();
		while(pItem)
		{
			if(pItem->hasTag(pcsTag))
				return dynamic_cast<T*>(pItem);
			pItem = pItem->getParent();
		}
		return NULL;
	}

	template < class TYPE > TYPE* getParentById(const char* pcsId, bool bAllowOldId = false) { return dynamic_cast<TYPE*>(getParentById(pcsId, bAllowOldId)); }
	UIElement* getParentById(const char* pcsId, bool bAllowOldId = false);

	template < class TYPE > TYPE* getChildAndSubchild(const char* pcsChildId, const char* pcsSubchildOldDefName) { return dynamic_cast<TYPE*>(getChildAndSubchild(pcsChildId, pcsSubchildOldDefName)); }
	UIElement* getChildAndSubchild(const char* pcsChildId, const char* pcsSubchildOldDefName);

	template < class TYPE > TYPE* getLinkedToElement() { return dynamic_cast<TYPE*>(getLinkedToElement()); }
	UIElement* getLinkedToElement();
	// I.e. with valid target part to update
	UIElement* getLinkedToElementWithValidTarget();
	// Gets the element that's linked to us that has the high value
	// of this property. Looks starting at the parent - i.e. starts with siblings
	UIElement* getElementLinkedToSelfWithMaxPropValue(PropertyType eProp);
	UIElement* getElementLinkedToSelfWithMinPropValue(PropertyType eProp);
	LinkedToSelfElemIterator linkedToSelfElemIteratorFirst(UIElement* pOptStartElem = NULL);


	// Find the child (recursive) with the smallest larger value compared to the
	// value passed in.
	UIElement* findNextChildByPropertyValue(PropertyType eProp, FLOAT_TYPE fCurrValue, FLOAT_TYPE fLesserThan = FLOAT_TYPE_MAX);
	UIElement* findChildByPropertyValue(PropertyType eProp, bool bValue);
	template < class TYPE_NAME > TYPE_NAME* findChildWithTag(const char* pcsTag, bool bIncludeSelf = false) { return dynamic_cast<TYPE_NAME*>(findChildWithTag(pcsTag, bIncludeSelf)); }
	UIElement* findChildWithTag(const char* pcsTag, bool bIncludeSelf = false);

	void resetToInitState();
	virtual void setPushedForRadioGroupRecursive(const char* pcsGroup, UIButtonElement* pException, bool bInstant);

	void setIsEnabled(bool bValue);
	inline bool getIsEnabled() { return myIsEnabled; }

	virtual bool canReleaseFocus() { return true; }
	virtual void resetEvalCache(bool bRecursive);
	void resetOnReloadDebug();

	void applyEffect(MotionEffectType eType, FLOAT_TYPE fSecDuration);

	virtual FLOAT_TYPE getScrollPixelTolerance();

	void setStateAnimCallbackData(const char* pcsData);
	FLOAT_TYPE getStateAnimProgress();
	bool getIsHiding();

	// Called when initializing the parent ui plane to
	// try and intellignetly adjust the position to
	// varying screen resolutions and aspect ratios.
	virtual void adjustToScreen();
	void adjustChildrenToScreen();
	ResourceItem* getOwnDefinition();
	// DO NOT USE, except when you know how.
	void setCachedOwnDef(ResourceItem* pDef, bool bRecursive);
	
	virtual void onDragDropChildBegin(UIElement* pDraggedChild) { }
	virtual void onDragDropSelfBegin() { }
	virtual bool receiveDragDrop(UIElement* pOther, SVector2D& svScreenPos) { return false; }
	virtual void onDragDropHover(UIElement* pOther, SVector2D& svScreenPos) { }
	// Called on the dragged element when the drag and drop is successfully accepted.
	// pReceiver is a pointer to the receiving element.
	virtual void onDragDropFinished(UIElement* pReceiver) { }
	virtual void onDragDropCancelled(UIElement* pReceiver) { }
	virtual CachedSequence* getOverrideDragRenderSeq(int& iFrameOut) { return NULL; }
	virtual bool allowDragDropStart(SVector2D& svPos);
	virtual AcceptType getCurrentDragDropMode() const { return AcceptNone; }
	virtual FLOAT_TYPE getMinDistanceBeforeDragStart() const { return DRAG_DROP_MIN_DIST; }

	bool getIsHidingInProgress() { return myHidingInProgress; }
	void onFinishHidingRecursive();

	void setDisabledOpacityMult(FLOAT_TYPE fValue) { myDisabledOpacityMult = fValue; }

	void insertIntoMap(TElementAlphaMap& rMap, TUIElemsUniqueCountHash* pOptUniqueCountMap);
	void removeFromMap(TElementAlphaMap& rMap);

	// Both of these are in local coords
	void setWindowRect(SRect2D& srRect);
	void getWindowRect(SRect2D& srRectOut);
	void setCenter(FLOAT_TYPE fX, FLOAT_TYPE fY);
	//void getLocalRectangle(const SVector2D& svScroll, FLOAT_TYPE fScale, SRect2D &srWindowRect);
	void getGlobalRectangle(SRect2D &srWindowRect);

	void resetInitialStateAnim();
	virtual void setNumericValue(FLOAT_TYPE fValue, UnitType eUnits, bool bIsChangingContinuously = false) { _ASSERT(0); }
	virtual FLOAT_TYPE getNumericValue(UnitType eUnits, FLOAT_TYPE fDpi = FLOAT_TYPE_MAX) { _ASSERT(0); return 0;}
	virtual void changeValueTo(FLOAT_TYPE fValue, UIElement* pOptSourceElem, bool bAnimate, bool bIsChangingContinuously);

	void autoLayoutChildrenRecursive();
	virtual void autoLayoutChildren();

	//void setTargetUpdateElement(UNIQUEID_TYPE id, const char* pcsParmName) { myTargetElementToUpdate = id; if(pcsParmName) myTargetElementParmName = pcsParmName; else myTargetElementParmName = ""; }
	void setTargetResourceItemParm(StringResourceItem* pItem, const char* pcsParmName);
	void setTargetResourceItemParm(ResourceItem* pItem, const char* pcsParmName);
	void setTargetUiElem(UIElement* pElem);
	void setTargetIdentifier(IGenericIdentifier* pIdentifier);

	void clearTargetUpdateIdentifier();
	//inline bool getHaveValidTargetElement() { return myTargetElementToUpdate >= 0; }
	bool getHaveValidTargetElement();
	IUndoItem* createUndoItemForSelfChange();
	const char* getUndoStringForSelfChange();
	inline IGenericIdentifier* getTargetIdentifier() { return myTargetIdentifier; }

	void copyParmValuesFrom(StringResourceItem* pItem, PropertyType eParmPropertyName);
	void copyParmValuesFrom(ResourceItem* pItem, PropertyType eParmPropertyName);
	void handleTargetElementUpdate(bool bIsChangingContinuously, bool bSendValueChangedEvent = true);

	virtual void onLostFocus(bool bHasCancelled) { }
	virtual void onGainedFocus() { }

	virtual bool onKeyUp(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl) { return false; }
	virtual bool onTextInput(int iKey) { return false; }

	void getExtraShiftOffset(SVector2D& svOffsetOut) { svOffsetOut = myExtraScrollOffset; }
	void setExtraScrollOffset(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bUpdateRelatedSlider);
	void updateTitleBarVars();
	void shiftAllChildrenRelativeToDefinition(FLOAT_TYPE fX, FLOAT_TYPE fY);

	int getTitleBarHeight() const;

	virtual bool getAllowKeepingFocus() { return false; }
	virtual bool getAllowKeyboardEntry() { return false; }
	virtual bool getAllowRemovingFocus() { return true; }

	virtual bool handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData) { return false; }
	virtual bool handleRMBAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData) { return false; }
	bool getHasDefaultActionChild() { return myDefaultActionChild.length() > 0; }
	bool getHasDefaultCancelChild() { return myDefaultCancelChild.length() > 0; }
	UIButtonElement* getDefaultActionChild();
	UIButtonElement* getDefaultCancelChild();

	// Return true to allow new units to be assigned, false to convert 
	// the value to current units and keep them.
	virtual bool onUnitsChanging(UnitType eNewUnits) { return false; }

	void invalidateSizingInfos();

	void setUniqueName(UIPlane* pParentPlane, bool bRecursive); // , TElementAlphaMap& rTempStringSet
	inline void setForceDrawUncachedText(bool bValue) { myForceDrawUncachedText = bValue; }

	virtual void getMinDims(SVector2D& svDimsOut) const;
	virtual void getMaxDims(SVector2D& svDimsOut) const;

	inline bool getIsUpdatingFromParm() const { return myIsUpdatingFromParm; }
	inline void setIsUpdatingFromParm(bool bValue) { myIsUpdatingFromParm = bValue; }

	// Callbacks for UIElementIdentifier
	virtual void onRefreshStoredValueFromUICallback(UIElement* pSourceElem, bool bIsChangingContinuously) { }
	virtual void onRefreshUIFromStoredValueCallback(UIElement* pTargetElem) { }

	inline const char* getCachedFont() const 
	{ 
		if(!myCachedFont)
		{
			if(this->doesPropertyExist(PropertyFont))
				return this->getStringProp(PropertyFont);
			else
				return "";
		}
		else
			return myCachedFont; 
	}

	inline FLOAT_TYPE getCachedFontSize() const { return myCachedFontSize;  }

	UIElement* getElementAtPath(const char* pcsPath);
	void getFullPath(string& strOut);
	inline bool getHaveAccelImage() const { return myAccelImage != NULL; }
	void setAccelImage(AccelImage* pNewImage);
	void setOrUpdateAccelImage(const IBaseImage* pImage, bool bOnlySetData);
	virtual void onThreadedThumbnailRenderFinished(const IBaseImage* pThumbnail, IThumbnailProvider* pProvider, int iIndex, const char* pcsOptParentId);

	void setDoPostponeRendering(bool bValue);
	void setHasNoDefinition(bool bValue);

	inline FLOAT_TYPE getTextWidth() { if(myTextWidth < 0) ensureFlexibleWidthValid(); return myTextWidth; }

	virtual bool getDoHaveToolTip(const SVector2D& svScreenCoords) const { return this->doesPropertyExist(PropertyTooltip); }
	virtual void getToolTip(const SVector2D& svScreenCoords, string& strTipOut) const { this->getAsString(PropertyTooltip, strTipOut); }

	virtual bool getAllowChangingTableSelection(FLOAT_TYPE fScreenX, FLOAT_TYPE fScreenY) { return true; }

	bool handleActionUpParentChain(string& strAction, bool bCallRMBPath, IBaseObject* pData = NULL);

	// For UI editor
	void setIsEditorSelected(bool bValue);
	bool getIsEditorSelected();
	void setInitEditorPos();
	void getInitEditorPos(SVector2D& svInit);
	void getAnimBoxSize(SVector2D& svOut);
	void getElementsListAtScreenPositionRecursive(int iX, int iY, TUIElementVector& rVecOut);
	void getSelectedElementsRecursive(TUIElementVector& rVecOut);
	void getAllElementsRecursive(TUIElementVector& rVecOut);

	template < class TYPE >
	void getAllElementsOfTypeRecursive(TUIElementSet& rVecOut)
	{
		if(as<TYPE>(this))
			rVecOut.insert(this);

		// Recurse on children
		UIElement* pChild;
		int iCurrChild, iNumChildren = this->getNumChildren();
		for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		{
			pChild = dynamic_cast<UIElement*>(this->getChild(iCurrChild));
			if(!pChild)
				ASSERT_CONTINUE;
			pChild->getAllElementsOfTypeRecursive<TYPE>(rVecOut);
		}
	}

	// Assumes it exists
	const char* getRadioGroupName();
	virtual bool closeAsTab() { return false; }
	UIElement* testShownDropdownsRecursive(const SVector2D& svPoint, bool bFirstNonPassthrough, bool bGlidingOnly);

	inline void setHandleActionCallback(IHandleActionCallback* pCallback) { myHandleActionCallback = pCallback; }
	inline IHandleActionCallback* getHandleActionCallback() const { return myHandleActionCallback; }

	inline bool getDoPostponeRendering() const { return myCachedDoPostponeRendering; }
	inline bool getHasNoDefinition() const { return myCachedHasNoDefinition; }
	inline AutolayoutType getChildrenLayoutType() const { return myChildrenLayoutType; }

	inline FLOAT_TYPE getFillParentXLessPadding() const { return myCachedFillParentXLessPadding; }
	FLOAT_TYPE getFillParentYLessPadding() const;
	void setFillParentXLessPadding(FLOAT_TYPE fValue);
	void setFillParentYLessPadding(FLOAT_TYPE fValue);

	IUndoItem* provideUndoItemForTargetIdentifierRecursive();
	virtual IUndoItem* provideUndoItemForTargetIdentifier() { return NULL; }
	inline bool getHasChildSliders() const { return myCachedHasChildSliders; }

	// Replaces this element with a UISplitter and sets it as a child.
	UISplitterElement* split(DirectionType eDir, int iOverrideChildPos = -1);

	void parentExistingElement(UIElement* pElem, int iOverrideChildPos = -1);
	void invalidateAndUpdateCursor();

	static ResourceItem* getDefinitionBlindSearch(ResourceType eColl, const char* pcsSelfId);
	void deleteChildrenDelayed();

	virtual void onIdChanged() { }

	void setRotation(FLOAT_TYPE fDegAngle);

	virtual bool getAllowTextCaching() const { return true; }
	inline void invalidateCachedText() { myIsCachedTextValid = false; }
	inline const string& getCachedTextRaw() const { return myCachedText; }

	virtual void getTextAsString(string &strOut, const char* pcsOptFloatConvFormat = NULL);
	virtual FLOAT_TYPE getTextAsNumber();
	virtual const char* getText();
	virtual void getTextAsStringOrExpression(string &strOut, const char* pcsOptFloatConvFormat = NULL);

	virtual bool getSaveInLayout() { return true; }
	void onPropertyChanged(PropertyType eProp);

protected:

	friend class UISliderElement;

	virtual void onPostUpdateChildData() { }
	virtual void onVisibleChangedCumulative(bool bValue) { }

	virtual FLOAT_TYPE getTextWidthForRendering() const { return myTextWidth; }

	// Called with a signed distance of the amount of wheel scroll.
	// If you just want an element to scroll, DO NOT override this. Use
	// applyMouseWheelDelta instead. Note the difference between multiplied
	// delta and just delta. Dont forget to set get getAllowWheelScroll()
	virtual bool onMouseWheel(FLOAT_TYPE fDelta);
	virtual bool applyMouseWheelDelta(FLOAT_TYPE fDeltaMulted);
	virtual bool getAllowWheelScroll() const { return false; }

	UIElement* getElementLinkedToSelfWithMaxPropValueInternal(UIElement* pTargetElem, PropertyType eProp, FLOAT_TYPE& fCurrMax);
	UIElement* getElementLinkedToSelfWithMinPropValueInternal(UIElement* pTargetElem, PropertyType eProp, FLOAT_TYPE& fCurrMin);

	void copyParmValuesFromRecurive(StringResourceItem* pItem, PropertyType eParmPropertyName, UIElement* pParmContainingElem);
	void copyParmValuesFromRecurive(ResourceItem* pItem, PropertyType eParmPropertyName, UIElement* pParmContainingElem);

	// This one is to override a particular aciton.
	void setPushedForRadioGroup(const char* pcsGroup, UIButtonElement* pException, bool bInstant);


	ResourceItem* getOwnDefinitionInternal(const char* pcsSelfId);

	virtual void animationOver(AnimatedValue *pAValue, string* pData);

	FLOAT_TYPE getTimeOffset(bool bIsForShowing);
	
	// Showing callbacks
	virtual void onStartShowing();
	void onStartShowingRecursive();
	virtual void onFinishShowing();
	void onFinishShowingRecursive();

	// Hiding callbacks
	virtual void onStartHiding();
	void onStartHidingRecursive();
	virtual void onFinishHiding();

	virtual void onChildFinishHiding(UIElement* pElem) { }

	void resetSliderPosition();

	//bool getMouseOverCoords(SVector2D& svOut);

	// Convenience methods 
//	void getOffPos(SVector2D& svOut);
//	void getOff2Pos(SVector2D& svOut);
//	void getOnPos(SVector2D& svOut);
	
	void getRawPositions(SVector2D& svOffOut, SVector2D& svOnOut, SVector2D& svOff2Out, FLOAT_TYPE& fOffScale, FLOAT_TYPE& fOnScale, FLOAT_TYPE& fOff2Scale);

	void renderInternal(const string &strAnim, const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);
	void renderTextInternal(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, bool bSelected);
	virtual void renderTextInternalFinal(const string& strText, const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, bool bSelected);
	void renderChildren(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);
	void renderTitleBar(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	virtual void onPreRenderChildren(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale) { }
	virtual void onPostRenderChildren(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale) { }
	virtual void preRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);
	virtual void postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	friend class UITableCellElement;
	virtual UIElement* getChildAtRecursiveInternal(const SVector2D& svPoint, const SVector2D& svScroll, bool bFirstNonPassthrough, bool bIgnoreChildren, bool bGlidingOnly);

	virtual bool onTextPreRender(string& strText, SColor& scolText, int &iCursorPosOut);
	virtual void onTextPostRender() { }
	virtual FLOAT_TYPE getCursorOpacity() { return 0; }
	// If we call this, it must be implemented.
	virtual void getRelativeCursorPos(SVector2D& svOut) { _ASSERT(0); }
	virtual void renderSelection(const SVector2D& svTextPos, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, bool bXorMode) { }
	virtual bool getHaveSelection() const { return false; }

	// On Mac, we may use these in the render thread as well as the normal thread. They can't be static.
	mutable	string theCommonString, theCommonString2, theCommonString3;
	virtual void updateOwnData(SUpdateInfo& rRefreshInfo) { /* Assert to ensure that we handle the update if these guys are set - no: in some cases, the parent updates the elemnt, and it has nothing to do, triggering this asser when it shouldn't _ASSERT(myTargetElementToUpdate < 0); */ }

	virtual ClockType getClockType() const { return ClockUiPrimary; }

	ResourceItem* getParentDefinition();
	void renderBBox();
	void renderBBoxOfSize(SVector2D& svSize, const SColor& scol, bool bTakeShiftIntoAccount);

	inline void getPressedPos(SVector2D& svPos) const { svPos = myPressedPos; }
	inline void getCachedShadowOffset(SVector2D& svOut) const { svOut = myShadowOffset; }

	void recomputeChildSizes();
	bool getChildSpacingInfo(UIElement* pElem, SChildSizingInfo& rInfoOut);
	void formatText(string& strInOut);
	void formatNumber(const SUnitNumber& rNumber, bool bAppendUnits, string& strOut);

	void markOffsetsDirty();
	void markOffsetsDirtyRecursive();

	void recomputeTextOffset();
	void recomputeAnimOffset();

	void ensureFlexibleWidthValid();

	virtual CursorType getOwnCursorType() const;
	inline void invalidateCursorNoUpdate() { myCursorType = CursorInvalidUnset; }

	inline HorAlignType getTextHorAlign() const { return myTextHorAlign; }
	inline VertAlignType getTextVertAlign() const { return myTextVertAlign; }
	void getTextOffset(SVector2D& svOffsetOut) { recomputeTextOffset(); svOffsetOut = myTextOffset; }
	void getRawOpacitites(FLOAT_TYPE& fOffOpac, FLOAT_TYPE& fOnOpac, FLOAT_TYPE& fOff2Opac);

	virtual void onSizeChanged() { }
	// Do NOT make this virtual! This is our internal processing for UIElement.
	void onSizeChangedInternal();

	virtual bool getAllowValuePropagation(FLOAT_TYPE fNewValue, bool bIsChangingContinuously, UIElement* pOptSourceElem, UIElement* pLinkedToElem);

	UISliderElement* getRelatedSlider(UIElement* pOptStartParent);
	void getChildrenBBox(SRect2D& srBBoxOut);

	inline void getOverlayColor(SColor& scolValueOut) const { scolValueOut = myOverlayColor; }
	void setOverlayColor(SColor& scolValue);

	void updateChildDataRecursive(SUpdateInfo& rRefreshInfo);

	virtual void getLastRenderedTextPos(SVector2D& svOut) const { svOut = myLastRenderedTextPos; }
	virtual void getScrolledTextPosition(SVector2D& svInOut) const { }

	virtual void modifyTextBeforeRender(SVector2D& svGlobalTextPos, string& strText) { }
	virtual void onTextWidthUpdated() { }

private:

	void onVisibleChangedCumulativeRecursive(bool bValue);
	void positionAsPopupRelativeToInternal(const SVector2D& svTargetPos, const SVector2D& svTargetSize, SideType eSide, FLOAT_TYPE fOffset, bool bExtraOffsetSideways, bool bRunRefresh);

	void ensureCachedTextValid();

protected:
	UIElementCallback* myCallbacks;
	IHandleActionCallback* myHandleActionCallback;

	bool myIsCallingChangeValue;
	UNIQUEID_TYPE	myPushedCursorId;

private:

	VertAlignType myTextVertAlign;
	HorAlignType myTextHorAlign;

	bool myIsAnimating;
	bool myIsMousePressed, myIsVisible, myIsRenderable;
	AnimatedValue myStateAnim;
	//DataSource myTextDataSource;
	//DataSource myObjAnimDataSource;
	// This is the index into the sliders enum in the config file
	int myCurrentSliderIndex;
	// This is a generic index among the list of items we're scrolling
	// through. Used for refreshing the data values. May or may not
	// be the same as the above (it is for non-infinite sliders).
	int myCurrentSlidePos;
	// We also need to track leaving and incoming children.
	UIElement *myLeavingSlideChild, *myIncomingSlideChild;

	string theSharedString, theSharedString2, theSharedString3;
	
	SVector2D myCachedLocalPosition;
	FLOAT_TYPE myCachedOpacity;
	GTIME myCachedPositionTime;
	FLOAT_TYPE myCachedScale;
	
	// Determines whether our on/off caches are valid.
	bool myValidRawPosCache;
	SVector2D myOffPosCache, myOff2PosCache, myOnPosCache;
	FLOAT_TYPE myOffScaleCache, myOff2ScaleCache, myOnScaleCache;
	
	FLOAT_TYPE myCachedOffOpacity, myCachedOff2Opacity, myCachedOnOpacity;
	
	SVector2D myTextOffset, myShadowOffset, myAnimOffset;
	FLOAT_TYPE myTextWidth;

	SVector2D myPressedPos;
	
	FLOAT_TYPE myCachedProgress;

	BackgroundModeType myBackgroundMode;
	bool myIsEnabled;

	IMotionEffect *myMotionEffect;
	
	const char* myCachedFont;
	FLOAT_TYPE myCachedFontSize;

	bool myHidingInProgress;

	FLOAT_TYPE myDisabledOpacityMult;

	InterpType myStartInterpType, myEndInterpType;

	// Used for elements which are hidden automatically
	AnimatedValue myLifeTimer;

	// Used for auto-sizing children
	TChildSizingInfos* mySizingInfos;

	bool myHasBeenPlacedManually;

	// Extra offset for static shift 
	// of children. This affects both drawing and picking.
	SVector2D myExtraScrollOffset;

	bool myIsTextOffsetDirty, myIsAnimOffsetDirty;

	CursorType myCursorType;

	static TCharPtrVector thePathVector;

	string myDefaultActionChild, myDefaultCancelChild;

	// These store the values (optional) which 
	// we should update when our value changes.
//	UNIQUEID_TYPE myTargetElementToUpdate;
//	string myTargetElementParmName;
	// A generic identifier that points to a thing to update 
	// when our value changes.
	IGenericIdentifier* myTargetIdentifier;

	bool myIsTestingDropdowns;
	bool myIsGettingLinkedToElemWithValidTarget;

	bool myForceDrawUncachedText;
	bool myCachedHasChildSliders;
	bool myCachedDoesSupportAutoScrolling;
	bool myIsUpdatingFromParm;
	bool myCachedDoPostponeRendering;

	bool myCachedHasNoDefinition;
	AutolayoutType myChildrenLayoutType;
	FLOAT_TYPE myCachedFillParentXLessPadding;
	FLOAT_TYPE myCachedFillParentYLessPadding;
	FLOAT_TYPE myCachedAnimOpacity;
	FLOAT_TYPE myCachedAnimScale;

	string *myGenericDataSource;

	FLOAT_TYPE myRotation;
	SColor myOverlayColor;

	SColor myBackgroundColor;
	SColor myBorderColor;
	FLOAT_TYPE myBorderThickness;

#ifdef ALLOW_UI_EDITOR
	// For UI editor
	bool myIsEditorSelected;
	SVector2D myInitEditorPos;
#endif

	// Optional image for using on-the-fly conversion from arbitrary bitmaps.
	AccelImage *myAccelImage;
	//Magick::Image* myPendingAnimImage;
	IBaseImage* myPendingAnimImage;

	Mutex myPendingAnimImageLock;
	Mutex myAccelImageLock;
// 	boost::recursive_mutex myPendingAnimImageLock;
// 	boost::recursive_mutex myAccelImageLock;

	FLOAT_TYPE myPrevChangedToValue;

	ResourceItem* myCachedOwnDefinition;
	SVector2D myLastReleasedPosition;
	GTIME myLastReleasedTime;

	SVector2D myLastRenderedTextPos;

	STRING_TYPE myCachedText;
	bool myIsCachedTextValid;

};
/*****************************************************************************/

