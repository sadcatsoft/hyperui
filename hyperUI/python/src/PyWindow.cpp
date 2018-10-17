#include "stdafx.h"
#include "PyWindow.h"
#include "PyUIElement.h"

namespace PythonAPI
{
/*****************************************************************************/
Window::Window(const char* pcsInitCollectionPath, const char* pcsTitle, int iScreenW, int iScreenH, const char* pcsFirstLayer)
{
	myId = -1;
	myCreationParms.myWidth = iScreenW;
	myCreationParms.myHeight = iScreenH;

	if(IS_VALID_STRING(pcsInitCollectionPath))
		myCreationParms.myInitCollectionType = ResourceManager::getInstance()->addCollection(pcsInitCollectionPath);

	if(IS_VALID_STRING(pcsTitle))
		myCreationParms.myTitle = pcsTitle;

	if(IS_VALID_STRING_AND_NOT_NONE(pcsFirstLayer))
		myCreationParms.myInitLayerToShow = pcsFirstLayer;
}
/*****************************************************************************/
Window::~Window()
{
	TPyWindowMap::getInstance()->remove(myId);
	myId = -1;
}
/*****************************************************************************/
HyperUI::Window* Window::getNativeWindow()
{
	return HyperUI::WindowManager::getInstance()->findItemById(myId);
}
/*****************************************************************************/
void Window::applyCreationParms(HyperUI::SWindowParms& rParmsOut)
{
	rParmsOut = myCreationParms;
}
/*****************************************************************************/
void Window::showLayer(const char* pcsLayerName, bool bImmediate)
{
	this->getNativeWindow()->getUIPlane()->showUI(pcsLayerName, bImmediate);
}
/*****************************************************************************/
void Window::hideLayer(const char* pcsLayerName, bool bImmediate)
{
	this->getNativeWindow()->getUIPlane()->hideUI(pcsLayerName, bImmediate);
}
/*****************************************************************************/
void Window::setId(UNIQUEID_TYPE id)
{
	_ASSERT(myId < 0);
	myId = id; 
	TPyWindowMap::getInstance()->add(id, this);
}
/*****************************************************************************/
boost::python::object Window::getElementById(const char* pcsElemId)
{
	HyperUI::UIPlane* pPlane = this->getNativeWindow()->getUIPlane();
	HyperUI::UIElement* pElem = pPlane->getElementById(pcsElemId);
	if(!pElem)
		return boost::python::object();

	boost::python::object pPyChild = UIElement::getPyObjectFromUIElement(pElem);
	return pPyChild;
}
/*****************************************************************************/
boost::python::dict Window::getElements()
{
	boost::python::dict pRes;

	HyperUI::UIPlane* pPlane = this->getNativeWindow()->getUIPlane();
	HyperUI::TUIElementVector vecElems;
	pPlane->getAllElements(vecElems);
	for(int iCurr = 0; iCurr < vecElems.size(); iCurr++)
	{
		boost::python::object pPyChild = UIElement::getPyObjectFromUIElement(vecElems[iCurr]);
		pRes[vecElems[iCurr]->getId()] = pPyChild;
	}

	return pRes;
}
/*****************************************************************************/
void Window::setIsVisibleForAllWithTag(const char* pcsTag, bool bIsVisible)
{
	HyperUI::UIPlane* pPlane = this->getNativeWindow()->getUIPlane();
	pPlane->setIsVisibleForAllWithTag(pcsTag, bIsVisible);
}
/*****************************************************************************/
void Window::updateElement(const char* pcsElemId)
{
	HyperUI::UIPlane* pPlane = this->getNativeWindow()->getUIPlane();
	if(!pPlane)
		return;
	pPlane->updateElement(pcsElemId);
}
/*****************************************************************************/
};