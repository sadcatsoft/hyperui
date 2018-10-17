#include "stdafx.h"
#include "BridgeUIElement.h"
#include "PyUIElement.h"
#include "PyTableCellInfoProvider.h"
#include "PyGridCellInfoProvider.h"
#include "PythonUtils.h"

namespace PythonAPI
{
/*****************************************************************************/
template < class BASE_TYPE >
TBridgeUIElement<BASE_TYPE>::TBridgeUIElement(HyperUI::UIPlane* pParentPlane)
	: BASE_TYPE(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
template < class BASE_TYPE >
TBridgeUIElement<BASE_TYPE>::~TBridgeUIElement()
{
	onDeallocated();
}
/*****************************************************************************/
template < class BASE_TYPE >
void TBridgeUIElement<BASE_TYPE>::onAllocated(HyperCore::IBaseObject* pData)
{
	BASE_TYPE::onAllocated(pData);
	BridgeElementBase::resetId();
}
/*****************************************************************************/
template < class BASE_TYPE >
void TBridgeUIElement<BASE_TYPE>::onDeallocated()
{
	BASE_TYPE::onDeallocated();

	BridgeElementBase::deallocate();
}
/*****************************************************************************/
template < class BASE_TYPE >
bool TBridgeUIElement<BASE_TYPE>::handleAction(string& strAction, HyperUI::UIElement* pSourceElem, IBaseObject* pData)
{
	PythonAPI::UIElement* pPyElem = TPyUIElementMap::getInstance()->findById(this->getMappingId());
	if(!pPyElem)
		return false;

	boost::python::object pyData;
	HyperCore::ResourceItem* pResData = dynamic_cast<HyperCore::ResourceItem*>(pData);
	HyperCore::BaseInt* pInt = dynamic_cast<HyperCore::BaseInt*>(pData);
	HyperCore::BaseCharPtr* pCharPtr = dynamic_cast<HyperCore::BaseCharPtr*>(pData);
	if(pResData)
		pyData = PythonUtils::resourceItemToObject(pResData);
	else if(pInt)
	{
		int iValue = pInt->getValue();
		pyData = boost::python::object(iValue);
	}
	else if(pCharPtr)
	{
		const char* pcsValue = pCharPtr->getValue();
		pyData = boost::python::object(pcsValue);
	}

	boost::python::object pyObj = pPyElem->getPyObject();
	boost::python::object pRes = pyObj.attr("handle_action")(PythonAPI::UIElement::getPyObjectFromUIElement(pSourceElem), strAction.c_str(), pyData);
	return boost::python::extract<bool>(pRes);
}
/*****************************************************************************/
template < class BASE_TYPE >
void PythonAPI::TBridgeUIElement<BASE_TYPE>::onTimerTick(GTIME lGlobalTime)
{
	BASE_TYPE::onTimerTick(lGlobalTime);

	PythonAPI::UIElement* pPyElem = TPyUIElementMap::getInstance()->findById(this->getMappingId());
	if(!pPyElem)
		return;

	boost::python::object pyObj = pPyElem->getPyObject();
	pyObj.attr("on_timer_tick")(lGlobalTime);
}
/*****************************************************************************/
template < class BASE_TYPE >
void TBridgeUIElement<BASE_TYPE>::updateOwnData(HyperUI::SUpdateInfo& rRefreshInfo)
{
	PythonAPI::UIElement* pPyElem = TPyUIElementMap::getInstance()->findById(this->getMappingId());
	if(!pPyElem)
		return;

	boost::python::object pyObj = pPyElem->getPyObject();
	boost::python::object pRes = pyObj.attr("on_update")();
}
/*****************************************************************************/
template < class BASE_TYPE >
int PythonAPI::TBridgeUIElement<BASE_TYPE>::getNumRows(HyperUI::UIElement* pCaller)
{
	int iNumRows = -1;
	PythonAPI::UIElement* pPyElem = TPyUIElementMap::getInstance()->findById(this->getMappingId());
	if(!pPyElem)
		return iNumRows;

	if(this->getHasPythonBaseClass(PY_NAME_TABLE_CELL_INFO_PROVIDER))
	{
		boost::python::object pyObj = pPyElem->getPyObject();
		boost::python::object pRes = pyObj.attr("get_num_rows")(PythonAPI::UIElement::getPyObjectFromUIElement(pCaller));
		iNumRows = boost::python::extract<int>(pRes);
	}
		
	return iNumRows;
}
/*****************************************************************************/
template < class BASE_TYPE >
int PythonAPI::TBridgeUIElement<BASE_TYPE>::getNumColumns(HyperUI::UIElement* pCaller)
{
	int iNumColumns = -1;
	PythonAPI::UIElement* pPyElem = TPyUIElementMap::getInstance()->findById(this->getMappingId());
	if(!pPyElem)
		return iNumColumns;

	if(this->getHasPythonBaseClass(PY_NAME_TABLE_CELL_INFO_PROVIDER))
	{
		boost::python::object pyObj = pPyElem->getPyObject();
		boost::python::object pRes = pyObj.attr("get_num_columns")(PythonAPI::UIElement::getPyObjectFromUIElement(pCaller));
		iNumColumns = boost::python::extract<int>(pRes);
	}

	return iNumColumns;
}
/*****************************************************************************/
template < class BASE_TYPE >
void PythonAPI::TBridgeUIElement<BASE_TYPE>::updateCell(HyperUI::UITableCellElement* pCell)
{
	PythonAPI::UIElement* pPyElem = TPyUIElementMap::getInstance()->findById(this->getMappingId());
	if(!pPyElem)
		return;

	if(!pPyElem->getNativeElement())
		return;

	if(this->getHasPythonBaseClass(PY_NAME_TABLE_CELL_INFO_PROVIDER))
	{
		boost::python::object pyObj = pPyElem->getPyObject();
		pyObj.attr("update_cell")(PythonAPI::UIElement::getPyObjectFromUIElement(pCell));
	}
}
/*****************************************************************************/
template < class BASE_TYPE >
void PythonAPI::TBridgeUIElement<BASE_TYPE>::onPostCreateRows(HyperUI::UITableElement* pTable)
{
	PythonAPI::UIElement* pPyElem = TPyUIElementMap::getInstance()->findById(this->getMappingId());
	if(!pPyElem)
		return;

	if(!pPyElem->getNativeElement())
		return;

	if(this->getHasPythonBaseClass(PY_NAME_TABLE_CELL_INFO_PROVIDER))
	{
		boost::python::object pyObj = pPyElem->getPyObject();
		pyObj.attr("on_post_create_rows")(PythonAPI::UIElement::getPyObjectFromUIElement(pTable));
	}
}
/*****************************************************************************/
template < class BASE_TYPE >
int PythonAPI::TBridgeUIElement<BASE_TYPE>::getNumTotalCells(HyperUI::UIElement* pCaller)
{
	int iNumCells = 1;
	PythonAPI::UIElement* pPyElem = TPyUIElementMap::getInstance()->findById(this->getMappingId());
	if(!pPyElem)
		return iNumCells;

	if(!pPyElem->getNativeElement())
		return iNumCells;

	if(this->getHasPythonBaseClass(PY_NAME_GRID_CELL_INFO_PROVIDER))
	{
		boost::python::object pyObj = pPyElem->getPyObject();
		boost::python::object pRes = pyObj.attr("get_num_total_cells")(PythonAPI::UIElement::getPyObjectFromUIElement(pCaller));
		iNumCells = boost::python::extract<int>(pRes);
	}

	return iNumCells;
}
/*****************************************************************************/
template class TBridgeUIElement<HyperUI::UIElement>;
template class TBridgeUIElement<HyperUI::UISplitterElement>;
/*****************************************************************************/
};
