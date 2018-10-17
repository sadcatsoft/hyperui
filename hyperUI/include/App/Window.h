#pragma once

/*

This class represents the internal top window for each form that holds
all the planes, the form dimensions, etc. This is essentially the UI/window
part ripped out of GameEngine so we can have multiple windows.

*/

struct SLightInfo;
class AccelImage;
class UIElement;
class UIPlane;
class TooltipManager;
class DragDropManager;
class HYPERUI_API TextureManager;
class DrawingCache;
class Window;
class UIAllocator;
#ifdef USE_FREETYPE_FONTS
class FontManager;
#endif

#if defined(DEBUG_PERSP_TOP_DOWN_VIEW)
	#define PERSP_CAMERA_ANGLE				0.0
#else
	#if defined(DEBUG_SIDE_VIEW)
		#define PERSP_CAMERA_ANGLE				-90.0
	#else
		#define PERSP_CAMERA_ANGLE				-65.0
	#endif
#endif

// Make these the same as in game plane
#define PERSP_FIXED_Y_OFFSET	-upToScreen(12.5) // -25
#define PERSP_FIXED_Z_OFFSET	-upToScreen(21) // -42

#define PERSP_GLOBAL_SCALE_FACTOR		0.45 // 0.65

#define PERPS_Z_CAM_OFFSET		-upToScreen(218.75)
#define PERSP_FOV_ANGLE			45.0
#define MIN_PERSP_SCALE						0.5

enum ClosingType
{
	ClosingNone = 0,
	ClosingWindow,
	ClosingTerminateApp
};
/*****************************************************************************/
class HYPERUI_API Window : public IWindowBase
{
public:
	Window(int iScreenW, int iScreenH, bool bIsMainWindow = false);
	virtual ~Window();

	// Dimensions
	//inline void setSize(int iW, int iH) { mySize.set(iW, iH); }
	inline void getSize(SVector2D& svOut) const { svOut = mySize; }
	inline const SVector2D& getSize() const { return mySize; }

	inline int getMaxDimension() const { return max(mySize.x, mySize.y); }
	inline void getWindowRectScrolled(SRect2D& srOut) const { srOut.x = myScroll.x; srOut.y = myScroll.y; srOut.w = mySize.x; srOut.h = mySize.y; }
	inline void getWindowRectRelative(SRect2D& srOut) const { srOut.x = srOut.y = 0; srOut.w = mySize.x; srOut.h = mySize.y; }

	// Scroll
	inline void setScroll(FLOAT_TYPE fScrollX, FLOAT_TYPE fScrollY, bool bMakeNice) { myScroll.set(fScrollX, fScrollY); if(bMakeNice) nicefyScroll(); }
	inline const SVector2D& getScroll() const { return myScroll; }
	void resetScroll(const SVector2D& svPlayerPos);
	void nicefyScroll();

	// UI Plane
	inline const UIPlane* getUIPlane() const { return myUIPlane; }
	inline UIPlane* getUIPlane() { return myUIPlane; }
	template < class TYPE > inline TYPE* getUIPlane() { return as<TYPE>(myUIPlane); }
	template < class TYPE >	inline const TYPE* getUIPlane() const { return as<const TYPE>(myUIPlane); }

	// Misc managers
	inline TooltipManager* getTooltipManager() { return myTooltipManager; }
	inline DragDropManager* getDragDropManager() { return myDragDropManager; }
	inline const TextureManager* getTextureManager() const { return myTextureManager; }
	inline TextureManager* getTextureManager() { return myTextureManager; }
	inline TCachedTextObjectCacher* getTextObjectCacher() { return myTextObjectCacher; }
	inline DrawingCache* getDrawingCache() { return myDrawingCache; }
#ifdef USE_FREETYPE_FONTS
	inline FontManager* getFontManager() { return myFontManager; }
#endif

	// Focus
	void setFocusElement(UIElement* pActive, bool bIsCancelling = false);
	inline UIElement* getFocusElement() { return myFocusElement; }
	template < class TYPE > TYPE* getFocusElement(void) { return dynamic_cast<TYPE*>(getFocusElement()); }

	// Initialization
	void finishPrelimInit(ResourceType eSourceCollection, const char* pcsInitLayerToShow);

	// Callbacks
	virtual void onUiElementDeleted(UIElement* pElem);
	virtual bool onTimerTickBegin();
	virtual bool onTimerTickEnd();
	void onUILayerShown(const char* pcsLayer);
	virtual void onWindowSizeChanged(int iNewW, int iNewH);
	virtual void onTabClosed(UIElement *pTab) { }
	virtual void onTabActivated(UIElement *pTab) { }
	virtual bool processActionSubclass(UIElement* pElem, const string& strAction, const char* pcsTargetDataSource, PropertyType eActionValue1Prop, PropertyType eActionValue2Prop, PropertyType eActionValue3Prop, MouseButtonType eButton) { return false; }
	virtual void onAcceptNewColorFromPicker(const SColor& scolNewColor) { }
	virtual const TUniqueIndexedColorSet* getRecentColors() { return NULL; }
	virtual bool getCanFinallyClose() { return true; }

	// Overriding undo managers
	void createAndPushOverrideUndoManager(UIElement* pParentElem);
	void destroyAndPopOverrideUndoManager(UIElement* pParentElem);
	UndoManager* getLastOverrideUndoManager();

	// Keyboard
	bool onKeyDown(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl);
	bool onKeyUp(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl);
	virtual void performOneTimeKeyDownAction(KeyMeaningType eKeyMeaning, const char* pcsUiAction, const char* pcsUiActionValue, bool bIsARepeat, bool bIsAlt);
	virtual void performOneTimeKeyUpAction(KeyMeaningType eKeyMeaning, const char* pcsUiAction, const char* pcsUiActionValue, bool bIsAlt);

	// Mouse
	virtual bool onTouchUp(TTouchVector& vecTouches);
	virtual void onTouchDown(TTouchVector& vecTouches, MouseButtonType eButton);
	virtual bool onTouchMove(TTouchVector& vecTouches);
	virtual void onTouchLeave();

	// The below is only called when the mouse isn't pressed.
	virtual void onCursorGlide(TTouchVector& vecTouches);
	virtual void onMouseWheel(SVector2D& svMousePos, FLOAT_TYPE fDelta);
	virtual void onTabletMousePan(SVector2D& svMousePos, FLOAT_TYPE fDelta);
	inline MouseButtonType getCurrMouseButtonDown() const { return myCurrButtonDown; }
	inline bool getHaveActiveTouches() const { return myActiveTouches.size() > 0; }
	inline int getNumActiveTouches() const { return myActiveTouches.size(); }
	STouchInfo* getTouchSlow(int iIndex);
	STouchInfo* getTouchById(void* pId);
	inline void getLastMousePos(SVector2D& svOut) const { svOut = myLastMousePos; }

	// OpenGL/Graphics
	void setupGlView(int iIncomingW, int iIncomingH);
	void setup2DGlView();
	void setup3DGlView(bool bAllowLighting, bool bPerspective, bool bDisableDepthClearing, bool bEnableAlpha);
	static void restoreAmbient();
	static void turnOnMaxAmbient();
	static void disableLighting();
	void allowLighting(bool bForPerspective);
	void renderToTexture();
	virtual BlendModeType setBlendMode(BlendModeType eNewMode);
	virtual BlendModeType getBlendMode() const { return myCurrentBlendMode; }

	// Misc
	bool getIsShowingAnyUI();
	void render();
	virtual void onSwitchedTo() { }

	UNIQUEID_TYPE pushCursor(CursorType eType);
	void popCursor(UNIQUEID_TYPE idCursor);
	void restoreLastCursor();

	void renderFramerates(bool bRenderSim = true);

	void renderDropAnim();
	void startDropAnim();
	inline UNIQUEID_TYPE getId() const { return myId; }
	inline void setId(UNIQUEID_TYPE idValue) { myId = idValue; }
	void close();
	inline bool getShouldClose() const { return myShouldClose; }
	void setShouldClose(bool bValue) { myShouldClose = bValue; }
	// Called when the form is about to close
	virtual void beginClosing(ClosingType eType);
	void resetBeginClosingFlag() { myBeganClosingFlag = ClosingNone; myHasCalledCloseAlready = false; }
	bool getHasBegunClosing() const { return myBeganClosingFlag != ClosingNone; }
	void checkForClosing();
	void getFramerateSamples(string& strOut, int iMaxSamples = 0);
	inline bool getDidFinishPrelimInit() const { return myDidFinishPrelimInit; }
	void setTitle(const char* pcsTitle);
	void setNewTextToActiveElement(const char* pcsNewText);

	virtual void onInitialized() { }

	static Window* getWindowWithLayerShown(const char* pcsLayer);

	void addFrameCustomLight(SVector2D& svCenter, FLOAT_TYPE fElevation, SColor& scol, FLOAT_TYPE fAtten1, FLOAT_TYPE fAtten2, FLOAT_TYPE fAtten3);

	void initFonts();
	void reloadAll();

	bool processAction(UIElement* pElem, const string& strAction, const char* pcsTargetDataSource, PropertyType eActionValue1Prop, PropertyType eActionValue2Prop, PropertyType eActionValue3Prop, MouseButtonType eButton);

	virtual bool getIsFullLicense() { return true; }

	virtual IBaseImage* allocateImage() { return NULL; }
	virtual AccelImage* allocateAccelImage() { return NULL; }

	virtual void substituteVariables(string& strInOut) { }
	virtual FLOAT_TYPE getCurrentDpi() { return 72.0; }
	virtual UnitType getDisplayUnits() { return UnitPixels; }

	virtual void onDragDropFiles(const TStringVector& vecFilePaths, FLOAT_TYPE fScreenX, FLOAT_TYPE fScreenY) { }

	void bringWindowToFront();
	void convertWindowCoordsToScreenCoords(const SVector2D& svLocalCoords, SVector2D& svScreenCoordsOut);

	virtual void setIsTabletButtonDown(bool bValue);
	// We pretend the button is down a little after it was released because otherwise we will zoom out since
	// we'll get the mouse wheel message after the tablet button up message.
	bool getIsTabletButtonDown();

	inline bool getIsMainWindow() const { return myIsMainWindow; }

	inline void setHasCalledClose(bool bValue) { myHasCalledCloseAlready = bValue; }
	inline bool getHasCalledClose() { return myHasCalledCloseAlready; }

protected:
	virtual bool getPassTouchesToUI() { return true; }
	virtual void modifyTouches(TTouchVector& vecTouches) { }
	virtual bool getIsActionEnabled(const char* pcsUiAction, const char* pcsUiActionValue) { return true; }

	virtual bool onKeyUpSubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl) { return false; }
	virtual bool onKeyDownBeforeUISubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl, bool bIsKeyARepeat) { return false; }
	virtual bool onKeyDownAfterUISubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl, bool bIsKeyARepeat) { return false; }

	virtual void renderBeforeUISubclass() { }
	virtual void renderAfterUISubclass() { }

	virtual UndoManager* allocateUndoManager(UIElement* pParentElem);
	virtual UIPlane* allocateUIPlane(ResourceType eSourceCollection);

	ICursorManager* allocateCursorManager();
	void deallocateCursorManager(ICursorManager* pCursorManager);

	virtual FLOAT_TYPE getPerspDepthLimit() { return 1000.0; }

private:

	void applyLightToGLState();
	//void closeImmediate();

private:

	SVector2D mySize;
	SVector2D myScroll;

	UIPlane* myUIPlane;

	TooltipManager* myTooltipManager;
	DragDropManager* myDragDropManager;

	TextureManager *myTextureManager;
	TCachedTextObjectCacher *myTextObjectCacher;

	DrawingCache* myDrawingCache;

	UIElement* myFocusElement;

	int myLastKey;

	MouseButtonType myCurrButtonDown;

	TTouchMap myActiveTouches;

	AnimatedValue myDropAnim;
	SVector2D myDropInitPos;

#ifdef DEBUG_SHOW_FRAMERATE
	RingBuffer< double > myFrameTimes;
	RingBuffer< double > mySimFrameTimes;
#endif

#ifdef ALLOW_3D
	GTIME myLastDepthClearTime;
#endif
	UNIQUEID_TYPE myId;

	bool myIsMainWindow;

	SVector2D myLastMousePos;

	BlendModeType myCurrentBlendMode;

	// We can't close immediately when clicking on our buttons,
	// for example, since that would kill the window and deallocate
	// the elements before the OnMouseUp() is done; so we set this
	// flag instead.
	bool myShouldClose;

	ClosingType myBeganClosingFlag;
	bool myDidFinishPrelimInit;

	// For setting up 3D lights
	int myCurrNumLights, myTotalLights;
	SLightInfo myLights[MAX_CUSTOM_LIGHTS];

#ifdef USE_OPENGL2
	GLfloat myModelviewMatrix[16];
#endif

#ifdef USE_FREETYPE_FONTS
	FontManager* myFontManager;
#endif

	// An stack of undo managers placed there by a UI element so we can undo local changes.
	// Empty by default.
	TUndoManagersList myOverrideUndoManagers;

	ICursorManager* myCursorManager;

	bool myIsTabletButtonDown;
	GTIME myLastTabletButtonUpTime;

	bool myHasCalledCloseAlready;

	SRect2D myReloadButtonRect;
};
/*****************************************************************************/
typedef TWindowManager< Window > WindowManager;
