#include "stdafx.h"
#include "PyUIElement.h"
#include "PyUISplitterElement.h"
#include "PyUITableCellElement.h"
#include "PyUITableElement.h"
#include "PyUIProgressElement.h"
#include "PythonUtils.h"
#include "PyWindow.h"
#include "PythonObjectFactory.h"
#include "BridgeUIElement.h"

const char* const g_pcsUiElemToPythonTypesMap[HyperUI::UiElemLastPlaceholder] =
{
	PY_NAME_UIELEMENT,
	NULL, // "button",
	PY_NAME_UITABLE_ELEMENT, // "table",
	NULL, // "horizontalTable",
	PY_NAME_UITABLE_CELL_ELEMENT, // "tableCell",
	PY_NAME_UIPROGRESS_ELEMENT, // "progress",
	NULL, // "slider",
	NULL, // "zoomWindow",
	NULL, // "textField",
	NULL, // "expressionTextField",
	NULL, // "expressionEditTextField",
	NULL, // "multilineTextField",
	NULL, // "solidColor",
	NULL, // "checkbox",
	NULL, // "menu",
	PY_NAME_UISPLITTER_ELEMENT,
	NULL, // "tabWindow",
	NULL, // "grid",
	NULL, // "colorSwatch",
	NULL, // "imageDropdown",
	NULL, // "dropdown",
	NULL, // "dropdownTextPart",
	NULL, // "dropdownMenu",
	NULL, // "unitedNumeric",
	NULL, // "gradientEdit",
	NULL, // "roundSlider",
	NULL, // "roundProgress",
	NULL, // "popup",
	NULL, // "thumbnailGrid",
	NULL, // "histogram",
	NULL, // "curveEditor",
	NULL, // "slidingStopsEdit",
	NULL, // "colorList",
	NULL, // "richText",
	NULL, // "colorPalette",
	NULL, // "recentColorsPalette",
	NULL, // "colorPicker",
	NULL // "user"		// really n/a, but just so we don't step over the bounds.
};

namespace PythonAPI
{
/*****************************************************************************/
UIElement::UIElement(UNIQUEID_TYPE idWindow, const char* pcsElemId, UNIQUEID_TYPE idMapping)
{
	myIgnoreGetPropCall = false;
	myMappingId = -1;
	initialize(idWindow, pcsElemId, idMapping);
}
/*****************************************************************************/
UIElement::UIElement()
{
	myIgnoreGetPropCall = false;
	myId = -1;
	myWindowId = -1;
	myMappingId = -1;
}
/*****************************************************************************/
UIElement::~UIElement()
{
	TPyUIElementMap::getInstance()->remove(myMappingId);
	myMappingId = -1;
}
/*****************************************************************************/
void UIElement::initialize(UNIQUEID_TYPE idWindow, const char* pcsElemId, UNIQUEID_TYPE idMapping)
{
	// We may have an empty elem id for dummy elements created in HyperUI
	// (such as for menu clicks). It's fine.
	//_ASSERT(IS_VALID_STRING(pcsElemId));
	_ASSERT(idWindow > 0);

	myId = pcsElemId;
	myWindowId = idWindow;
	if(idMapping > 0)
		setMappingId(idMapping);
}
/*****************************************************************************/
boost::python::object UIElement::getProperty(const char* pcsKey)
{
	if(myIgnoreGetPropCall)
	{
		// We have to do this because we get here if we're checking whether
		// the object has a property defined or not, and unless we throw an
		// actual exception, Python will think the property exists, even if
		// return false or None.
		throw std::runtime_error("No such property.");
	}

	HyperUI::UIElement* pElem = this->getNativeElement();
	if(!pElem)
		return boost::python::object();

	PropertyType eProp = HyperCore::PropertyMapper::getInstance()->mapProperty(pcsKey);
	if(eProp == PropertyUnknown)
	{
		string strMessage;
		strMessage = "Property name '";
		strMessage += pcsKey;
		strMessage += "' is not defined. ";
		strMessage += "Properties for UIElements are pre-defined to a limited set in C++ core. Please see documentations for specific values.";
		throw std::runtime_error(strMessage.c_str());
		return boost::python::object();
	}
	return PythonUtils::resourcePropToObject(pElem->findProperty(eProp));
}
/*****************************************************************************/
void UIElement::delProperty(const char* pcsKey)
{
	HyperUI::UIElement* pElem = this->getNativeElement();
	PropertyType eProp = HyperCore::PropertyMapper::getInstance()->mapProperty(pcsKey);
	if(eProp == PropertyUnknown)
	{
		string strMessage;
		strMessage = "Property name '";
		strMessage += pcsKey;
		strMessage += "' is not defined. ";
		strMessage += "Properties for UIElements are pre-defined to a limited set in C++ core. Please see documentations for specific values.";
		throw std::runtime_error(strMessage.c_str());
		return;
	}

	pElem->removeProperty(eProp);
	pElem->onPropertyChanged(eProp);
}
/*****************************************************************************/
void UIElement::setProperty(const char* pcsKey, boost::python::object pValue)
{
	HyperUI::UIElement* pElem = this->getNativeElement();

	PropertyType eProp = HyperCore::PropertyMapper::getInstance()->mapProperty(pcsKey);
	if(eProp == PropertyUnknown)
	{
		string strMessage;
		strMessage = "Property name '";
		strMessage += pcsKey;
		strMessage += "' is not defined. ";
		strMessage += "Properties for UIElements are pre-defined to a limited set in C++ core. Please see documentations for specific values.";
		throw std::runtime_error(strMessage.c_str());
		return;
	}
	
	// Ensure property is created
	pElem->setNumProp(eProp, 0);
	ResourceProperty* pProp = pElem->findProperty(eProp);
	PythonUtils::objectToResourceProp(pValue, pProp);

	pElem->onPropertyChanged(eProp);
}
/*****************************************************************************/
bool UIElement::hasAttr(const char* pcsAttr)
{
	_ASSERT(!myIgnoreGetPropCall);
	myIgnoreGetPropCall = true;
	boost::python::object pyObj = this->getPyObject();
	bool bRes = boost::python::hasattr(pyObj, pcsAttr);
	myIgnoreGetPropCall = false;
	return bRes;
}
/*****************************************************************************/
void UIElement::updateDataRecursive()
{
	HyperUI::UIElement* pElem = this->getNativeElement();
	pElem->updateDataRecursive();
}
/*****************************************************************************/
boost::python::object UIElement::getChildById(const char* pcsId)
{
	HyperUI::UIElement* pElem = this->getNativeElement();
	if(!pElem)
		return boost::python::object();

	HyperUI::UIElement* pChild = pElem->getChildById(pcsId);
	boost::python::object pPyChild = UIElement::getPyObjectFromUIElement(pChild);
	return pPyChild;
}
/*****************************************************************************/
void UIElement::setIsVisible(bool bIsVisible)
{
	HyperUI::UIElement* pElem = this->getNativeElement();
	if(!pElem)
		return;
	pElem->setIsVisible(bIsVisible);
}
/*****************************************************************************/
bool UIElement::getIsVisible()
{
	HyperUI::UIElement* pElem = this->getNativeElement();
	if(!pElem)
		return false;
	return pElem->getIsVisible();
}
/*****************************************************************************/
boost::python::dict UIElement::getChildren()
{
	boost::python::dict pRes;
	HyperUI::UIElement* pElem = this->getNativeElement();
	if(!pElem)
		return pRes;
	HyperUI::UIElement* pChild;
	int iCurrChild, iNumChildren = pElem->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = dynamic_cast<HyperUI::UIElement*>(pElem->getChild(iCurrChild));
		boost::python::object pPyChild = UIElement::getPyObjectFromUIElement(pChild);
		pRes[pChild->getId()] = pPyChild;
	}

	return pRes;
}
/*****************************************************************************/
boost::python::object UIElement::getParent()
{
	HyperUI::UIElement* pElem = this->getNativeElement()->getParent<HyperUI::UIElement>();
	return UIElement::getPyObjectFromUIElement(pElem);
}
/*****************************************************************************/
boost::python::object UIElement::getWindow()
{
	PythonAPI::Window* pPyWindow = pPyWindow = PythonAPI::TPyWindowMap::getInstance()->findById(myWindowId);
	return pPyWindow->getPyWindow();
}
/*****************************************************************************/
boost::python::list UIElement::getPropertyNames()
{
	boost::python::list pRes;
	HyperUI::UIElement* pElem = this->getNativeElement();
	ResourceItem::PropertyIterator pi;
	
	for(pi = pElem->propertiesFirst(true); !pi.isEnd(); pi++)
		pRes.append(PropertyMapper::getInstance()->getPropertyString(pi.getProperty()));
	return pRes;
}
/*****************************************************************************/
void UIElement::setMappingId(UNIQUEID_TYPE id)
{
	myMappingId = id;
	TPyUIElementMap::getInstance()->add(id, this);
}
/*****************************************************************************/
boost::python::object UIElement::getPyObjectFromUIElement(HyperUI::UIElement* pElem)
{
	const char* pcsPythonClassName = g_pcsUiElemToPythonTypesMap[pElem->getElemType()];
	if(!pcsPythonClassName)
		pcsPythonClassName = PY_NAME_UIELEMENT;

	return ALLOCATE_PYOBJECT(pcsPythonClassName)(pElem->getParentWindow()->getId(), pElem->getId(), -1);
}
/*****************************************************************************/
boost::python::object UIElement::getPyObject()
{
	// Note that if our mapping id is > 0, we already have a node floating
	// around somewhere. 
	if(myMappingId > 0)
	{
		PythonAPI::BridgeElementBase* pBridgeElem = TPyBridgeUIElementMap::getInstance()->findById(myMappingId);
		return pBridgeElem->getPyObject();
	}
	else
		return ALLOCATE_PYOBJECT(this->getPythonClassName())(myWindowId, myId, myMappingId);
}
/*****************************************************************************/
HyperUI::UIElement* UIElement::getNativeElement() const
{
	HyperUI::Window* pWindow = HyperUI::WindowManager::getInstance()->findItemById(myWindowId);
	if(!pWindow->getUIPlane())
		return NULL;

	return pWindow->getUIPlane()->getElementById(myId.c_str());
}
/*****************************************************************************/
}
