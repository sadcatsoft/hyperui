#pragma once

/*****************************************************************************/
// Helper macro so we don't have to write factories for simple cases
#define REGISTER_MAIN_WINDOW_TYPE(x) \
	class x##Factory: public HyperUI::IWindowFactory\
	{\
	public:\
		virtual ~x##Factory() { }\
		virtual HyperUI::Window* allocateWindow(ResourceType eSourceCollection, int iScreenW, int iScreenH, bool bIsMainWindow)\
		{\
			HyperUI::Window* pResWindow = new x(iScreenW, iScreenH, bIsMainWindow);\
			return pResWindow;\
		}\
	};\
	HyperUI::Application::getInstance()->setWindowFactory(new x##Factory);
/*****************************************************************************/
class HYPERUI_API Application
{
public:

	static Application* getInstance();
	virtual ~Application();

	// Timer functions
	inline GTIME getGlobalTime(ClockType eType)
	{
		SCOPED_MUTEX_LOCK(&myInternalsLock);
		return myGlobalTimes[eType];
	}
	void resetAllClocks();
	void loadClocksFrom(ResourceItem* pItem, PropertyType eCurrPropName);
	void onTimerTickBegin(bool bIncrementMainGameClock);

	static void quitApp();
	static void lockGlobal();
	static void unlockGlobal();
	static void lockGlobalDisplay();
	static void unlockGlobalDisplay();

	static bool showOpenFolderDialog(const char* pcsInitFilePath, TStringVector& vecFilesOut);
	static bool showOpenFileDialog(const char* pcsInitFilePath, bool bAllowMultiple, TStringVector& vecFilesOut, TStringVector& pOptExtensions, 
									TStringVector& pOptDescriptions, FormatsManager* pFormatsManager, bool bCanOpenFolders = false);
	static bool showOpenFileDialog(const char* pcsInitFilePath, bool bAllowMultiple, TStringVector& vecFilesOut, FormatsManager* pFormatsManager, bool bCanOpenFolders = false);
	static ResultCodeType showSaveFileDialog(string& strPathInOut, string& strErrorOut, TStringVector& pOptExtensions, TStringVector& pOptDescriptions, FormatsManager* pFormatsManager, bool bForceNativeExtension, bool bDontListNative = false);
	static ResultCodeType showSaveFileDialog(string& strPathInOut, string& strErrorOut, FormatsManager* pFormatsManager, bool bForceNativeExtension, bool bDontListNative = false);

	static Window* openNewWindow(Window* pParentWindow, int iResourceType, int iOptWidth = -1, int iOptHeight = -1, bool bThinFrame = false, bool bFixedSize = false, int iMinWidth = -1, int iMinHeight = -1, const char* pcsTitle = NULL, int iCenterX = -1, int iCenterY = -1, bool bIsMainWindow = false, const char* pcsMainMenuElemId = NULL, const char* pcsInitLayerToShow = NULL);

	static inline GTIME secondsToTicks(FLOAT_TYPE dSeconds) { return (GTIME)(dSeconds * GAME_FRAMERATE); }
	static inline FLOAT_TYPE ticksToSeconds(GTIME lTime) { return (double)lTime/(double)GAME_FRAMERATE; }

	static void setIsDebuggingEnabled(bool bValue) { myIsDebuggingEnabled = bValue; }
	static bool getIsDebuggingEnabled() { return myIsDebuggingEnabled;  }

	static bool getColorFromColorPicker(SColor& scolExistingColor, const char* pcsStartElemName, UNIQUEID_TYPE idParentWindow, SColor& scolNewOut);

	inline bool getShowFramerate() const { return myShowFramerate; }
	inline void setShowFramerate(bool bValue) { myShowFramerate = bValue; }

	inline bool getIsQuittingFinal()  const { return myIsQuittingFinal; }
	inline void setIsQuittingFinal(bool bValue) { myIsQuittingFinal = bValue; }

	inline void setDisableSound(bool bValue) { myDisableSound = bValue; }
	inline bool getDisableSound() const { return myDisableSound; }

	// Called every time the app is switched to
	void onSwitchedTo(Window* pExceptWindow);

	void setWindowFactory(IWindowFactory* pFactory);
	Window* allocateWindow(ResourceType eSourceCollection, int iScreenW, int iScreenH, bool bIsMainWindow = false);

	void setRecentFilesProviderFactory(IMenuContentsProviderFactory* pFactory);
	IMenuContentsProvider* allocateRecentFilesProvider();

	inline UIAllocator* getUIAllocator() { return myAllocator; }
	void setUIAllocator(UIAllocator* pAllocator);

	static void reloadAll();

private:
	Application();
	void registerTypeConstructors();

private:

	static Application* theInstance;

	Mutex myInternalsLock;
	GTIME myGlobalTimes[ClockLastPlaceholder];

	bool myShowFramerate;
	bool myIsQuittingFinal;

	bool myDisableSound;

	static bool myIsDebuggingEnabled;

	IWindowFactory* myWindowFactory;
	IMenuContentsProviderFactory* myRecentFilesProviderFactory;

	UIAllocator *myAllocator;

#ifdef OSX_BUILD
	static string theColorUiElemName;
#endif
};
/*****************************************************************************/
