#pragma once

//#define LOG_THY_UI

// First episode is used for tutorials.
#define FIRST_REAL_EPISODE			2

class IMenuContentsProvider;
class IMenuEnableStatusCallback;
class Window;
class UIColorPicker;
class UIElement;
class UIAllocator;

typedef TLosslessDynamicArray < TUIElementVector* > TUIElementVectorLosslessDynamicArray;  
/*****************************************************************************/
struct SPostponedElemInfo
{
	SVector2D myScroll;
	FLOAT_TYPE myOpacity;	
	FLOAT_TYPE myScale;
	UIElement* myElem;
};
typedef vector < SPostponedElemInfo > TPostponedElemInfos;
/*****************************************************************************/
class UIPlane : public IPlane, public UIElementCallback, public AnimationOverCallback
{
public:
	UIPlane(Window* pParentWindow, ResourceType eSourceCollection);
	virtual ~UIPlane();

	virtual void onTimerTickBegin(GTIME lGlobalTime);
	virtual void onTimerTickEnd(GTIME lGlobalTime);

	virtual void onTouchDown(TTouchVector& vecTouches);
	virtual bool onTouchUp(TTouchVector& vecTouches, bool bIgnoreActions);
	virtual void onTouchMove(TTouchVector& vecTouches);
	virtual void onCursorGlide(TTouchVector& vecTouches);
	void onMouseWheel(SVector2D& svMousePos, FLOAT_TYPE fDelta);

	// Simulates the action by calling onButtonClicked with dummy button
	void performUiAction(const char* pcsAction, const char* pcsActionValue = NULL, const char* pcsActionValue2 = NULL, const char* pcsTargetData = NULL);
	virtual void onButtonClicked(UIElement *pElem, MouseButtonType eButton, bool bSetSelfAsLastResourceDataSource = false);
	virtual void onSliderValueChanged(UISliderElement* pSlider);
	virtual void showUI(const char* pcsLayer, bool bImmediate = false, const char* pcsWithTargetDataSource = NULL, StringResourceItem* pOptSrcItem = NULL, PropertyType eParmPropertyName = PropertyNull);
	virtual void hideUI(const char* pcsLayer, bool bImmediate = false, FLOAT_TYPE fExtraOffset = 0.0);
	void hideAll(const char* pcsExceptLayer = NULL);
	void hideAllImmediate();
	void hideLayersWithTag(const char* pcsTag);
	void hideAllWithTag(const char* pcsTag, TUIElementVector* pExceptions = NULL, bool bSetMenusAsUnshowableOnGlide = true, AnimationOverCallback* pCallBack = NULL, AnimOverActionType eAnimOverAction = AnimOverActionNone, bool bInstant = false);
	void hideAllWithTagExcept(const char* pcsTag, const char* pcsExceptionId, bool bSetMenusAsUnshowableOnGlide = true, AnimationOverCallback* pCallBack = NULL, AnimOverActionType eAnimOverAction = AnimOverActionNone, bool bInstant = false);

	void setIsVisibleForAllWithTag(const char* pcsTag, bool bIsVisible);

	bool getAreAnyShownWithTag(const char* pcsTag);

	inline const SVector2D& getLastTouchUpPos() const { return myLastTouchUpPos; }

	UIElement* getTopLayerElementAt(FLOAT_TYPE fX, FLOAT_TYPE fY);
	UIElement* getElementInLayerAt(FLOAT_TYPE fX, FLOAT_TYPE fY, const char* pcsLayer, bool bGlidingOnly);
	UIElement* getTopmostShownElementAt(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bGlidingOnly);
	UIElement* getFirstElementAt(FLOAT_TYPE fX, FLOAT_TYPE fY);
	const char* getLastShownLayerName();

	void onWindowSizeChanged(int iNewW, int iNewH);
	
	virtual PlaneType getPlaneType() { return PlaneUI; }
	
	UIElement* getTopLevelElementByLayerName(const char* pcsName);

	inline UIElement* getElementById(const char* pcsId, bool bRecurse = true, bool bIncludeOldDefId = false)
	{ return getElementByIdInternal(pcsId, bRecurse, bIncludeOldDefId); }

	template < class T >
	inline T* getElementById(const char* pcsId, bool bRecurse = true, bool bIncludeOldDefId = false)
		{ return dynamic_cast<T*>(getElementByIdInternal(pcsId, bRecurse, bIncludeOldDefId)); }

	UIElement* getElementWithDefaultActionChild();
	UIElement* getElementWithDefaultCancelChild();

	bool getIsLayerShown(const char* pcsLayer);
	bool getIsAnimatingAnything();
	// Returns true if any layers are fully shown or are being shown.
	bool getIsShowingAnything();
	bool getIsHidingAnything();
	///FLOAT_TYPE getMaxUiOpacity(const char* pcsExludeLayer = NULL);
	FLOAT_TYPE getMaxPausingUiOpacity();
	FLOAT_TYPE getMaxNonPausingUiOpacity();
	FLOAT_TYPE getLayerProgress(const char* pcsLayerName);
	
	void deleteElement(UIElement* pElem);
	
//	void enableRange(int iFrom, int iTo, bool bValue);
	void setPushedForRadioGroup(const char* pcsGroup, UIButtonElement* pException, bool bInstant);

	UIAllocator* getAllocator();

	virtual void render();

	void resetToInitState(const char* pcsExceptLayer);
	void resetAllCachesForAllElements();

	UIElement* getFinalChildAtCoords(FLOAT_TYPE fX, FLOAT_TYPE fY, UIElement* pStartElem, bool bFirstNonPassthrough, bool bGlidingOnly);

	void getAllShownElements(TUIElementVector& rVectorOut);
	void getElemChildrenRec(ResourceItem* pElem, TUIElementVector& rVectorOut);

	bool getIsFullyObscuring() { return myIsFullyObscuring; }

	void setHideAllMenus(bool bValue) { myHideAllSubmenus = bValue; }

	void showAsSubmenu(const char* pcsElemId, const SVector2D& svPoint, SideType eSide, FLOAT_TYPE fOffset, IMenuContentsProvider* pOptContentsProvider = NULL, const IMenuEnableStatusCallback* pOptEnableCallback = NULL, const char* pcsOptTargetDataSource = NULL, bool bOptDoHideAllOtherMenus = true);

	UIElement* addNewElement(const char* pcsType, UIElement* pOptParent, int iOptPosition = -1, const char* pcsOptId = NULL);
	UIElement* addNewElementFromTemplate(ResourceItem* pTemplate, UIElement* pOptParent);
	UIElement* addNewElementFromText(const char* pcsElemSpec, UIElement* pOptParent);
	ResourceItem* getDefaultsForTag(const char* pcsElemTag);
	void insertIntoNamesMap(UIElement* pElem, bool bInsertIntoUniqueNamesMapToo);
	void removeFromNamesMap(UIElement* pElem);

	void onUiElementDeleted(UIElement* pElem);
	void generateUniqueName(UIElement* pElem, string& strNameOut); // TElementAlphaMap& rTempStringSet, 

	void updateElement(const char* pcsElemName);
	void updateElementIfLayerShown(const char* pcsElemName, const char* pcsLayerName);

	void showPopupMessage(const char* pcsMessage, const char* pcsOptIcon = NULL);
	void showMessageBox(const char* pcsMessage, const char* pcsTitle = NULL);
	static UIColorPicker* getCurrentlyOpenColorPicker();
	void showColorPicker(const SColor& scolExistingColor, const char* pcsSrcElemId, UNIQUEID_TYPE idParentWindow);
	void showRenameBox(const char* pcsCurrName, const char* pcsOkActionType, UIElement* pSourceElem);
	void showYesNoBox(const char* pcsMessage, const char* pcsOkActionValue, UIElement* pSourceElem, const char* pcsOkButtonText = NULL, const char* pcsTitle = NULL, bool bShowApplyToAllCheckbox = false, const char* pcsCancelButtonText = NULL, const char* pcsCancelButtonActionValue = NULL);
	void showYesNoCancelBox(const char* pcsMessage, const char* pcsYesActionValue, const char* pcsNoActionValue, const char* pcsCancelActionValue, UIElement* pSourceElemAndCallback, IHandleActionCallback* pHandleCallback = NULL, const char* pcsYesButtonText = NULL, const char* pcsNoButtonText = NULL, 
		const char* pcsCancelButtonText = NULL, const char* pcsTitle = NULL, bool bShowApplyToAllCheckbox = false);

	void addPostponedElement(UIElement* pElem, const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);
	inline RenderingPassType getRenderingPass() const { return myCurrRenderingPass; }

	// Used for UI Editor
	void getElementsListAtScreenPosition(int iX, int iY, TUIElementVector& rVecOut);
	void getSelectedElements(TUIElementVector& rVecOut);
	void getAllElements(TUIElementVector& rVecOut);
	
	UIElement* testShownDropdownsRecursive(const SVector2D& svPoint, bool bFirstNonPassthrough, bool bGlidingOnly);

	template < class TYPE >
	void getAllElementsOfType(TUIElementSet& rVecOut)
	{
		rVecOut.clear();
		int iCurrElements, iNumElements;
		UIElement* pElement;

		iNumElements = myUIElements.size();
		for(iCurrElements = iNumElements - 1; iCurrElements >= 0; iCurrElements--)
		{
			pElement = FAST_CAST<UIElement*>(myUIElements[iCurrElements]);
			if(!pElement)
				continue;
			// Otherwise, see the element
			pElement->getAllElementsOfTypeRecursive<TYPE>(rVecOut);
		}
	}

	inline void lockMouseCursor(UIElement* pElem) { myMouseLockedElem = pElem; }
	inline void unlockMouseCursor() { myMouseLockedElem = NULL; }

	void processAction(UIElement* pElem, const string& strAction, const char* pcsTargetDataSource, PropertyType eActionValue1Prop, PropertyType eActionValue2Prop, PropertyType eActionValue3Prop, MouseButtonType eButton);

	UISplitterElement* splitElement(UIElement* pElem, DirectionType eDir, int iOverrideChildPos = -1);
	void fuseElement(UISplitterElement* pSplitter);

	virtual bool receiveDragDrop(UIElement* pDraggedElem, SVector2D& svScreenPos, UIElement* pTopmostOverElem);
	virtual bool onDragDropHover(UIElement* pDraggedElem, SVector2D& svScreenPos, UIElement* pTopmostOverElem);

	UIDragDropManager* getUIDragDropManager() { return &myUiDragDropManager; }
	void scheduleForDeletion(UIElement* pElem) { myElementsToDelete.insert(pElem); }
	void deleteQueuedElements();

	void queueForFocus(const CHAR_TYPE* pcsElemId);

	inline TUIElementVector* getUIElements() { return &myUIElements; }
	virtual FormatsManager* getFormatsManager() { return NULL; }

	ResourceType getInitCollectionType() const { return myInitCollectionType; }
	void reloadAll();

protected:
	bool loadDefaultParmsFrom(UIElement* pElem, PropertyType eProp, StringResourceItem& rDefaultsOut);

	void finishAddingCreatedElementInternal(UIElement* pNewElem, UIElement* pOptParent, int iOptPosition, const char* pcsOptId);
	void hideAllWithTagRecursive(UIElement* pElem, const char* pcsTag, TUIElementVector* pExceptions, bool bSetMenusAsUnshowableOnGlide, AnimationOverCallback* pCallBack, AnimOverActionType eAnimOverAction, bool bInstant, TStringSet& setHiddenLayers);
	void setIsVisibleForAllWithTagRecursive(UIElement* pElem, const char* pcsTag, bool bIsVisible);

	UIElement* createAndAddUIElement(const char* pcsId);

	int findShownLayer(const char* pcsName);

	void recomputeRenderableFlag();

	void startItemAnim(SVector2D& svInitPos, const char* pcsAnimString, const char *pcsElemId);

	virtual void animationOver(AnimatedValue *pAValue, string* pData);

	void renderPostponedElems();

	bool getAreAnyElementsInLayerShown(const char* pcsLayerName);


#ifdef LOG_THY_UI
	void printTouches(const char* pcsText, TTouchVector* pTouches, UIElement* pElem);
#endif

private:
	UIElement* getElementByIdInternal(const char* pcsId, bool bRecurse, bool bIncludeOldDefId);
	void slideParentElement(UIElement* pElem, int iDir);
	
	UIElement* getElementByIdInternalSLOW(const char* pcsId, bool bRecurse, bool bIncludeOldDefId);

	void ensureElementStageExists(int iRenderStage);

	// Do not make this virtual, since it's called from the constructor, and we won't call the 
	// super variant
	void initFromCollection(ResourceType eCollectionType);

protected:
	string mySharedString, theSharedString2;

private:

	bool myCachedIsShowingAnything;
	bool myIsCachedShowingAnythingValid;

	FLOAT_TYPE myCachedMaxPausingOpacity;
	bool myIsCachedMaxPausingOpacityValid;

	FLOAT_TYPE myCachedMaxNonPausingOpacity;
	bool myIsCachedMaxNonPausingOpacityValid;

	bool myIsRenderableFlagDirty;
	bool myIsFullyObscuring;

	string myLastShownLayerName;
	TStringVector myCurrentlyShownLayers;
	
	TStringSet myCommonStringSet, myCommonStringSet2;
	TStringVector myCommonStringVector, myCommonStringVector2;
	TUIElementVector myCommonUiVElems;
	TElementAlphaMapMemCacher myElementsMapMemCacher;
	TCharPtrVector myCommonCharPtrVector;

	UIElement* myLastMouseElement;
	UIElement* myLastGlideElement;

	TUIElementVectorLosslessDynamicArray myElementsInStages;
	TElementAlphaMap myElementsMap;
	TUIElementVector myUIElements;
	//TElementAlphaMap myTempGeneratedUniqueNames;
	///TStringSet myTempGeneratedUniqueNames;

	bool myHideAllSubmenus;

	TPostponedElemInfos myPostponedElemInfos;
	RenderingPassType myCurrRenderingPass;

	SVector2D myLastTouchUpPos;
	UIElement* myMouseLockedElem;

	UIDragDropManager myUiDragDropManager;

	TUIElementSet myElementsToDelete;

	TUIElemsUniqueCountHash myUniqueCounts;
	TUIElementSet myElemDelayedDeletionParentSet;

	TStringSet myFocusQueue;
	Mutex myRefreshQueueLock;

	ResourceType myInitCollectionType;
};
/*****************************************************************************/