#pragma once

#include "PyObjectMap.h"

namespace PythonAPI
{
/*****************************************************************************/
class Window
{
public:
	Window(const char* pcsInitCollectinPath, const char* pcsTitle = NULL, int iScreenW = -1, int iScreenH = -1, const char* pcsFirstLayer = NULL);
	virtual ~Window();

	void applyCreationParms(HyperUI::SWindowParms& rParmsOut);

	void showLayer(const char* pcsLayerName, bool bImmediate = false);
	void hideLayer(const char* pcsLayerName, bool bImmediate = false);

	bool onTimerTickBegin() { return true;  }
	void onInitialized() { }

	bool handleAction(boost::python::object pElem, const char* pcsAction) { return false; }

	void setId(UNIQUEID_TYPE id);
	inline UNIQUEID_TYPE getId() const { return myId; }

	void setPyWindow(boost::python::object pObj) { myPyWindow = pObj; }
	boost::python::object getPyWindow() { return myPyWindow;  }

	bool onKeyUpSubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl) { return false; }
	bool onKeyDownBeforeUISubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl, bool bIsKeyARepeat) { return false; }
	bool onKeyDownAfterUISubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl, bool bIsKeyARepeat) { return false; }

	boost::python::object getElementById(const char* pcsElemId);
	boost::python::dict getElements();

	void setIsVisibleForAllWithTag(const char* pcsTag, bool bIsVisible);
	void updateElement(const char* pcsElemId);

private:

	HyperUI::Window* getNativeWindow();

private:

	// This id is the same as the 
	UNIQUEID_TYPE myId;
	boost::python::object myPyWindow;

	HyperUI::SWindowParms myCreationParms;
};
/*****************************************************************************/
typedef PyObjectMap<PythonAPI::Window> TPyWindowMap;
/*****************************************************************************/
}
