#include "stdafx.h"
#include "BridgeWindow.h"
#include "PyUIElement.h"
#include "PyObjectMap.h"
#include "PyWindow.h"

namespace PythonAPI
{
/*****************************************************************************/
BridgeWindow::BridgeWindow(int iScreenW, int iScreenH, bool bIsMainWindow) : HyperUI::Window(iScreenW, iScreenH, bIsMainWindow)
{
	myPyWindowId = -1;
}
/*****************************************************************************/
BridgeWindow::~BridgeWindow()
{

}
/*****************************************************************************/
bool BridgeWindow::onTimerTickBegin()
{
	bool bRes = Window::onTimerTickBegin();
	if(!this->getHaveValidWindow())
		return true;

	this->getPyObject().attr("on_timer_tick")();
	return true;
}
/*****************************************************************************/
void BridgeWindow::onInitialized()
{
	if(!this->getHaveValidWindow())
		return;

	this->getUIPlane()->showUI("main");
	this->getPyObject().attr("on_initialized")();
}
/*****************************************************************************/
bool BridgeWindow::processActionSubclass(HyperUI::UIElement* pElem, const string& strAction, const char* pcsTargetDataSource, PropertyType eActionValue1Prop,
										 PropertyType eActionValue2Prop, PropertyType eActionValue3Prop, HyperUI::MouseButtonType eButton)
{
	if(!this->getHaveValidWindow())
		return false;

	boost::python::object pPyUIElem = PythonAPI::UIElement::getPyObjectFromUIElement(pElem);
	boost::python::object pRes = this->getPyObject().attr("handle_action")(pPyUIElem, strAction.c_str());
	return boost::python::extract<bool>(pRes);
}
/*****************************************************************************/
bool BridgeWindow::onKeyUpSubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl)
{
	if(!this->getHaveValidWindow())
		return false;

	boost::python::object pRes = this->getPyObject().attr("on_key_up")(iKey, bControl, bAlt, bShift, bMacActualControl);
	return boost::python::extract<bool>(pRes);
}
/*****************************************************************************/
bool BridgeWindow::onKeyDownBeforeUISubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl, bool bIsKeyARepeat)
{
	if(!this->getHaveValidWindow())
		return false;

	boost::python::object pRes = this->getPyObject().attr("on_key_down")(iKey, bControl, bAlt, bShift, bMacActualControl, bIsKeyARepeat);
	return boost::python::extract<bool>(pRes);
}
/*****************************************************************************/
bool BridgeWindow::onKeyDownAfterUISubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl, bool bIsKeyARepeat)
{
	if(!this->getHaveValidWindow())
		return false;

	boost::python::object pRes = this->getPyObject().attr("on_key_down_filtered")(iKey, bControl, bAlt, bShift, bMacActualControl, bIsKeyARepeat);
	return boost::python::extract<bool>(pRes);
}
/*****************************************************************************/
PythonAPI::Window* BridgeWindow::getPyWindow()
{
	PythonAPI::Window* pPyWindow = NULL;
	if(myPyWindowId >= 0)
	{
		pPyWindow = PythonAPI::TPyWindowMap::getInstance()->findById(myPyWindowId);
		_ASSERT(pPyWindow);
	}
	return pPyWindow;
}
/*****************************************************************************/
boost::python::object BridgeWindow::getPyObject()
{
	return getPyWindow()->getPyWindow();
}
/*****************************************************************************/
};
