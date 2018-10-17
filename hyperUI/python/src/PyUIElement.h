#pragma once

#include "PyObjectMap.h"

namespace PythonAPI
{
#define PY_NAME_UIELEMENT "UIElement"
/*****************************************************************************/
class UIElement
{
public:
	UIElement();
	UIElement(UNIQUEID_TYPE idWindow, const char* pcsElemId, UNIQUEID_TYPE idMapping);
	virtual ~UIElement();

	void initialize(UNIQUEID_TYPE idWindow, const char* pcsElemId, UNIQUEID_TYPE idMapping);
	std::string getId() const { return myId; }

	boost::python::object getProperty(const char* pcsKey);
	void delProperty(const char* pcsKey);
	void setProperty(const char* pcsKey, boost::python::object pValue);

	bool hasAttr(const char* pcsAttr);

	void updateDataRecursive();
	boost::python::dict getChildren();
	boost::python::object getParent();
	boost::python::object getWindow();

	boost::python::object getChildById(const char* pcsId);

	void setIsVisible(bool bIsVisible);
	bool getIsVisible();

	boost::python::list getPropertyNames();

	void setMappingId(UNIQUEID_TYPE id);
	//inline UNIQUEID_TYPE getMappingId() const { return myMappingId; }

	static boost::python::object getPyObjectFromUIElement(HyperUI::UIElement* pElem);
	boost::python::object getPyObject();

	HyperUI::UIElement* getNativeElement() const;

	// These are exposed to be overriden in classes in Python
	bool handleAction(boost::python::object pElem, const char* pcsAction, boost::python::object pData) { return false; }
	void onUpdate() { }
	void onTimerTick(GTIME lGlobalTime) { }

protected:

	virtual const char* getPythonClassName() const { return PY_NAME_UIELEMENT; }

private:

	string myId;
	UNIQUEID_TYPE myWindowId;

	UNIQUEID_TYPE myMappingId;

	bool myIgnoreGetPropCall;
};
/*****************************************************************************/
typedef PyObjectMap<PythonAPI::UIElement> TPyUIElementMap;
/*****************************************************************************/
};