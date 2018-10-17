#include "stdafx.h"
#include "PyWindow.h"
#include "PyUIElement.h"
#include "PyUISplitterElement.h"
#include "PyUITableCellElement.h"
#include "PyUITableElement.h"
#include "PyUIProgressElement.h"
#include "PyTableCellInfoProvider.h"
#include "PyGridCellInfoProvider.h"
#include "PythonObjectFactory.h"
#include "PythonUtils.h"
#include "BridgeUIElement.h"
#include "BridgeWindow.h"

using namespace HyperUI;
using namespace boost::python;

extern void runMacApp(HyperUI::SWindowParms& rParms);

/*
The idea is:
- We implement Py::Window that inherits from Window;
- It gets called with callbacks on the functions we need
- Then, in python, we declare a python class that inherits from Py:Window
- We need to pass its instance to us...
- And we then use that in a factory (i.e. extracted PyWindwo from it...)

Or: do we even need to have the python wnd inherit from us? Can't we just store a reference to it in PyWnd and call that? Much simpler for the user...
Although on the other hand, they lose access to all other wnd props that we may expose, so probably not.

*/

/*****************************************************************************/
class BridgeWindowFactory : public IWindowFactory
{
public:
	static BridgeWindowFactory* getInstance()
	{
		if(!theInstance)
			theInstance = new BridgeWindowFactory;
		return theInstance;
	}
	virtual ~BridgeWindowFactory() { }
	virtual Window* allocateWindow(ResourceType eSourceCollection, int iScreenW, int iScreenH, bool bIsMainWindow)
	{
		PythonAPI::BridgeWindow* pResWindow = new PythonAPI::BridgeWindow(iScreenW, iScreenH, bIsMainWindow);
		if(myPyWindow)
		{
			myPyWindow->setId(pResWindow->getId());
			pResWindow->setPyWindowId(myPyWindow->getId());
		}

		myPyWindow = NULL;
		return pResWindow;
	}

	void setCurrentPyWindow(PythonAPI::Window* pPyWindow)
	{
		myPyWindow = pPyWindow;
	}
private:
	BridgeWindowFactory()
	{
		myPyWindow = NULL;
	}

private:
	PythonAPI::Window* myPyWindow;
	static BridgeWindowFactory* theInstance;
};
BridgeWindowFactory* BridgeWindowFactory::theInstance = NULL;
/*****************************************************************************/
class PyUIAllocator: public HyperUI::UIAllocator
{
public:
	static PyUIAllocator* getInstance()
	{
		if(!theInstance)
			theInstance = new PyUIAllocator;
		return theInstance;
	}
	virtual ~PyUIAllocator() { }

	virtual ResourceItem* allocateNewItem(const ResourceItem* pSourceItem, IBaseObject* pData, const char* pcsTag = NULL)
	{
		// See if this is one of our python type
		const CHAR_TYPE* pcsTypeToAlloc = pSourceItem->getStringProp(PropertyElemType);
		TStringSet::iterator si = myPythonCustomTypes.find(pcsTypeToAlloc);
		if(si == myPythonCustomTypes.end())
			return UIAllocator::allocateNewItem(pSourceItem, pData, pcsTag);

		// Otherwise, we have a type!
		// Create its python object. In Python, of course...
		object _mainModule = import("__main__");
		object _mainNamespace = _mainModule.attr("__dict__");

		// Allocate the python override object
		string strExecCode;
		strExecCode = (*si) + "()";
		boost::python::object pRes = eval(strExecCode.c_str(), _mainNamespace, _mainNamespace);
		PythonAPI::UIElement* pPyUIElem = extract<PythonAPI::UIElement*>(pRes);

		// We allocate our bridge element and return that
		PythonAPI::BridgeElementBase* pBridgeElem = NULL;
		HyperUI::UIElement *pHyperCastBridgeElem = NULL;

		// We need to find the base type from which this custom Python element inherits, if any,
		// and create the right bridge elem type.
		string strBridgeTypeToAlloc = PY_NAME_UIELEMENT;
		string strExecCode2;
		strExecCode2 = "[ base.__name__ for base in " + (*si) + ".__bases__ ]";
		boost::python::object pBaseClassesList = eval(strExecCode2.c_str(), _mainNamespace, _mainNamespace);
		extract<boost::python::list> rListVal(pBaseClassesList);
		boost::python::list rList = rListVal();
		int iCurrBase, iNumBaseEntries = len(rList);
		for(iCurrBase = 0; iCurrBase < iNumBaseEntries; iCurrBase++)
		{
			boost::python::extract<const char*> rStrVal(rList[iCurrBase]);
			const char* pcsCurrBase = rStrVal();
			if(myBridgeElemConstructors.getDoHaveConstructorFor(pcsCurrBase))
			{
				strBridgeTypeToAlloc = pcsCurrBase;
				break;
			}
		}

		pHyperCastBridgeElem = dynamic_cast<HyperUI::UIElement*>(myBridgeElemConstructors.allocateItemFromType(strBridgeTypeToAlloc.c_str(), pData, pcsTag));
		pBridgeElem = dynamic_cast<PythonAPI::BridgeElementBase*>(pHyperCastBridgeElem);

		// Register one with the other
		UNIQUEID_TYPE id = this->getNextId();
		pBridgeElem->setMappingId(id, pRes);
		pPyUIElem->initialize(pHyperCastBridgeElem->getParentWindow()->getId(), pSourceItem->getId(), id);

// 		string strTest;
// 		PythonUtils::getStringFromObject(pRes, strTest);

		return pHyperCastBridgeElem;
	}
	
	void registerCustomUIPythonConstructor(const char* pcsType)
	{
		myPythonCustomTypes.insert(pcsType);
	}

	void registerBridgeElementConstructor(const char* pcsType, UIElementConstructorType pConstructor)
	{
		myBridgeElemConstructors.registerConstructor(pcsType, pConstructor);
	}

private:
	PyUIAllocator()
	{
		myNextId = 0;
	}

	UNIQUEID_TYPE getNextId()
	{
		myNextId++;
		return myNextId;
	}

private:
	TStringSet myPythonCustomTypes;
	UNIQUEID_TYPE myNextId;

	UIAllocator myBridgeElemConstructors;

	static PyUIAllocator* theInstance;
};
PyUIAllocator* PyUIAllocator::theInstance = NULL;
/*****************************************************************************/
void initializeModule(const char* pcsFullModulePath)
{
	HyperUI::initialize("PyHUI", NULL, pcsFullModulePath);

	Application::getInstance()->setUIAllocator(PyUIAllocator::getInstance());
	Application::getInstance()->setWindowFactory(BridgeWindowFactory::getInstance());

	Py_Initialize();
	object _mainModule = import("__main__");
	object _mainNamespace = _mainModule.attr("__dict__");
    
/*
	string strTest;
	PythonUtils::getStringFromObject(_mainNamespace, strTest);
	FileUtils::saveToFile("D:\\Ketara\\ProjectAurora\\Source\\hyperUI\\python\\dist\\out.txt", strTest.c_str(), true);
	int bp = 0;
*/

	//boost::python::dict origNamespace = extract<boost::python::dict>(_mainModule.attr("__dict__"));
	//exec("testPrint()", _mainNamespace, _mainNamespace);
	//exec("print (\"Hello from python string code \")", _mainNamespace, _mainNamespace);
}
/*****************************************************************************/
int createWindow(boost::python::object pObj)
{
	PythonAPI::Window* pInWindow = extract<PythonAPI::Window*>(pObj);
	if(pInWindow)
	{
		BridgeWindowFactory::getInstance()->setCurrentPyWindow(pInWindow);
		pInWindow->setPyWindow(pObj);
	}

	// Does not call overloaded member
	//string pcsTitle = pInWindow->getTitle();
	//////////////////boost::python::object pRes = pObj.attr("get_title")();
	//////////////////const char* pcsTitle = boost::python::extract<const char*>(pRes);

	// We can get dimensions from the actual object passed in
	SWindowParms rParms;
	if(pInWindow)
		pInWindow->applyCreationParms(rParms);
	//rParms.myInitCollectionType = ResourceManager::getInstance()->addCollection("restPyTest.txt");
#ifdef WINDOWS
	HyperUI::run(rParms);
#else
    runMacApp(rParms);
#endif
	//HyperUI::run();
	return HyperCore::getScreenDensityScalingFactor();
	//return "Hello, from c++ dll2!";
}
/*****************************************************************************/
void enableDebugging()
{
	Application::setIsDebuggingEnabled(true);
}
/*****************************************************************************/
void addCollection(const char* pcsCollectionPath, CollectionRoleType eRole, const char* pcsTargetDataPathPrefix)
{
	ResourceManager::getInstance()->addCollection(pcsCollectionPath, false, eRole, pcsTargetDataPathPrefix);
}
/*****************************************************************************/
void registerUIElement(const char* pcsClassName)
{
	PyUIAllocator::getInstance()->registerCustomUIPythonConstructor(pcsClassName);
}
/*****************************************************************************/
// http://www.discoversdk.com/blog/python-cpp-interop
/*
bootstrap
b2 address-model=64 -j 8 link=static,shared toolset=msvc-14.0 runtime-link=shared --with-python python=3.6
b2 --clean-all -n

https://www.boost.org/build/doc/html/bbv2/overview/invocation.html
*/

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(UIElementOverloadsShowLayer, showLayer, 1, 2);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(UIElementOverloadsHideLayer, hideLayer, 1, 2);

using namespace boost::python;
BOOST_PYTHON_MODULE(pyhyperui)
{
	docstring_options docOpts(true, true, false);

	// These are necessary for the user to be able to inherit from the types listed here.
	PyUIAllocator::getInstance()->registerBridgeElementConstructor(PY_NAME_UIELEMENT, PythonAPI::BridgeUIElement::allocateSelf);
	PyUIAllocator::getInstance()->registerBridgeElementConstructor(PY_NAME_UISPLITTER_ELEMENT, PythonAPI::BridgeUISplitterElement::allocateSelf);
	//PyUIAllocator::getInstance()->registerBridgeElementConstructor(PY_NAME_UITABLE_CELL_ELEMENT, PythonAPI::BridgeUISplitterElement::allocateSelf);
	
	// Some references:
	// https://docs.python.org/3/reference/datamodel.html#object.__getitem__
	// https://stackoverflow.com/questions/13733098/boost-python-passing-boostpythonobject-as-argument-to-python-function

	// Global functions
	def("create_window", createWindow);
	def("initialize", ::initializeModule);
	def("register_uielement", registerUIElement);
	def("add_collection_internal", addCollection);
	def("enable_debugging", enableDebugging);

	// Enums
	REGISTER_PYOBJECT("CollectionRole", enum_<CollectionRoleType>("CollectionRole")
					  .value(g_pcsCollectionRoleTypeStrings[CollectionRoleGraphics], CollectionRoleGraphics)
					  .value(g_pcsCollectionRoleTypeStrings[CollectionRoleUI], CollectionRoleUI)
					  .value("Generic", CollectionRoleLastPlaceholder)
	);

	// PyWindow
	class_<PythonAPI::Window>("Window", "Represents a native window.", init<const char*, const char*, int, int, const char*>())
		.def(init<const char*>())
		.add_property("elements", &PythonAPI::Window::getElements, "Get all UI elements of this window")
		.def("on_timer_tick", &PythonAPI::Window::onTimerTickBegin)
		.def("on_initialized", &PythonAPI::Window::onInitialized)
		.def("handle_action", &PythonAPI::Window::handleAction)
		.def("on_key_up", &PythonAPI::Window::onKeyUpSubclass)
		.def("on_key_down", &PythonAPI::Window::onKeyDownBeforeUISubclass)
		.def("on_key_down_filtered", &PythonAPI::Window::onKeyDownAfterUISubclass)
		.def("show_layer", &PythonAPI::Window::showLayer, UIElementOverloadsShowLayer())
		.def("hide_layer", &PythonAPI::Window::hideLayer, UIElementOverloadsHideLayer())
		.def("get_element_by_id", &PythonAPI::Window::getElementById)
		.def("set_is_visible_for_all_with_tag", &PythonAPI::Window::setIsVisibleForAllWithTag)
		.def("update_element", &PythonAPI::Window::updateElement)
		;

	// PyUIElement
	REGISTER_PYOBJECT(PY_NAME_UIELEMENT, class_<PythonAPI::UIElement>(PY_NAME_UIELEMENT, "Represents a generic UI element.", init<>())
					  .def(init<UNIQUEID_TYPE, const char*, UNIQUEID_TYPE>())
					  .add_property("children", &PythonAPI::UIElement::getChildren, "Get child elements of this element")
					  .add_property("parent", &PythonAPI::UIElement::getParent, "Get the parent of this element")
					  .add_property("window", &PythonAPI::UIElement::getWindow)
					  .def("__getattr__", &PythonAPI::UIElement::getProperty)
					  .def("__setattr__", &PythonAPI::UIElement::setProperty)
					  .def("__delattr__", &PythonAPI::UIElement::delProperty)
					  .def("__getitem__", &PythonAPI::UIElement::getProperty)
					  .def("update", &PythonAPI::UIElement::updateDataRecursive)
					  .def("keys", &PythonAPI::UIElement::getPropertyNames)
					  .def("handle_action", &PythonAPI::UIElement::handleAction)
					  .def("on_update", &PythonAPI::UIElement::onUpdate)
					  .def("on_timer_tick", &PythonAPI::UIElement::onTimerTick)
					  .def("get_child_by_id", &PythonAPI::UIElement::getChildById)
					  .def("set_is_visible", &PythonAPI::UIElement::setIsVisible)
					  .def("get_is_visible", &PythonAPI::UIElement::getIsVisible)
	);

	// PyUISplitterElement
	REGISTER_PYOBJECT(PY_NAME_UISPLITTER_ELEMENT, class_<PythonAPI::UISplitterElement, bases<PythonAPI::UIElement> >(PY_NAME_UISPLITTER_ELEMENT, "Represents a UI splitter element.", init<>())
					  .def(init<UNIQUEID_TYPE, const char*, UNIQUEID_TYPE>())
	);

	// PyUITableElement
	REGISTER_PYOBJECT(PY_NAME_UITABLE_ELEMENT, class_<PythonAPI::UITableElement, bases<PythonAPI::UIElement> >(PY_NAME_UITABLE_ELEMENT, "Represents a table element.", init<>())
					  .def(init<UNIQUEID_TYPE, const char*, UNIQUEID_TYPE>())
					  .def("select_row", &PythonAPI::UITableElement::selectRow)
	);

	// PyUITableCellElement
	REGISTER_PYOBJECT(PY_NAME_UITABLE_CELL_ELEMENT, class_<PythonAPI::UITableCellElement, bases<PythonAPI::UIElement> >(PY_NAME_UITABLE_CELL_ELEMENT, "Represents a table cell element.", init<>())
					  .def(init<UNIQUEID_TYPE, const char*, UNIQUEID_TYPE>())
					  .add_property("column", &PythonAPI::UITableCellElement::getCol, "Get the column of this cell in its table")
					  .add_property("row", &PythonAPI::UITableCellElement::getRow, "Get the row of this cell in its table")
	);
	
	// PyUIProgressElement
	REGISTER_PYOBJECT(PY_NAME_UIPROGRESS_ELEMENT, class_<PythonAPI::UIProgressElement, bases<PythonAPI::UIElement> >(PY_NAME_UIPROGRESS_ELEMENT, "Represents a progress bar element.", init<>())
					  .def(init<UNIQUEID_TYPE, const char*, UNIQUEID_TYPE>())
					  .def("set_min_progress", &PythonAPI::UIProgressElement::setMinProgress)
					  .def("set_max_progress", &PythonAPI::UIProgressElement::setMaxProgress)
					  .def("set_progress", &PythonAPI::UIProgressElement::setProgress)
					  .def("get_progress", &PythonAPI::UIProgressElement::getProgress)
	);

	// PyTableCellInfoProvider
	REGISTER_PYOBJECT(PY_NAME_TABLE_CELL_INFO_PROVIDER, class_<PythonAPI::TableCellInfoProvider>(PY_NAME_TABLE_CELL_INFO_PROVIDER, "Represents information provider for table elements.", init<>())
					  .def("get_num_rows", &PythonAPI::TableCellInfoProvider::getNumRows)
					  .def("get_num_columns", &PythonAPI::TableCellInfoProvider::getNumColumns)
					  .def("update_cell", &PythonAPI::TableCellInfoProvider::updateCell)
					  .def("on_post_create_rows", &PythonAPI::TableCellInfoProvider::onPostCreateRows)
	);

	// PyGridCellInfoProvider
	REGISTER_PYOBJECT(PY_NAME_GRID_CELL_INFO_PROVIDER, class_<PythonAPI::GridCellInfoProvider>(PY_NAME_GRID_CELL_INFO_PROVIDER, "Represents information provider for grid elements.", init<>())
					  .def("get_num_total_cells", &PythonAPI::GridCellInfoProvider::getNumTotalCells)
	);
}
/*****************************************************************************/
