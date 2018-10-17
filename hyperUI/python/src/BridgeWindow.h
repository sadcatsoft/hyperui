#pragma once

namespace PythonAPI
{
class Window;
/*****************************************************************************/
// This is a bridge window that inherits from our actual HyperUI window,
// and acts as a bridge between that and the python object window.
class BridgeWindow : public HyperUI::Window
{
public:
	BridgeWindow(int iScreenW, int iScreenH, bool bIsMainWindow = false);
	virtual ~BridgeWindow();

	inline void setPyWindowId(UNIQUEID_TYPE id) { myPyWindowId = id; }

	virtual bool onTimerTickBegin();
	virtual void onInitialized();
	virtual bool processActionSubclass(HyperUI::UIElement* pElem, const string& strAction, const char* pcsTargetDataSource, PropertyType eActionValue1Prop,
									   PropertyType eActionValue2Prop, PropertyType eActionValue3Prop, HyperUI::MouseButtonType eButton);

	virtual bool onKeyUpSubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl);
	virtual bool onKeyDownBeforeUISubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl, bool bIsKeyARepeat);
	virtual bool onKeyDownAfterUISubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl, bool bIsKeyARepeat);

private:

	inline bool getHaveValidWindow() const { return (myPyWindowId >= 0); }

	PythonAPI::Window* getPyWindow();

	boost::python::object getPyObject();

private:
	UNIQUEID_TYPE myPyWindowId;

};
/*****************************************************************************/
};