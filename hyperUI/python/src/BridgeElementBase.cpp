#include "stdafx.h"
#include "BridgeElementBase.h"
#include "PyUIElement.h"

namespace PythonAPI
{
/*****************************************************************************/
void BridgeElementBase::setMappingId(UNIQUEID_TYPE id, boost::python::object pyObj)
{
	myMappingId = id;
	TPyBridgeUIElementMap::getInstance()->add(id, this);

	myPyObject = pyObj;
}
/*****************************************************************************/
void BridgeElementBase::deallocate()
{
	TPyBridgeUIElementMap::getInstance()->remove(myMappingId);
	myPyObject = boost::python::object();
}
/*****************************************************************************/
bool BridgeElementBase::getHasPythonBaseClass(const char* pcsBase)
{
	boost::python::object pyObj = this->getPyObject();
	boost::python::object pClass = pyObj.attr("__class__");
	boost::python::object pBases = pClass.attr("__bases__");
	boost::python::extract<boost::python::tuple> rTuples(pBases);
	boost::python::tuple rTuple = rTuples();

	int iCurrBase, iNumBaseEntries = len(rTuple);
	for(iCurrBase = 0; iCurrBase < iNumBaseEntries; iCurrBase++)
	{
		boost::python::object pCurrElem = rTuple[iCurrBase];
		boost::python::object pCurrElemClassName = pCurrElem.attr("__name__");
		boost::python::extract<const char*> rStrVal(pCurrElemClassName);
		const char* pcsCurrBase = rStrVal();
		if(IS_STRING_EQUAL(pcsCurrBase, pcsBase))
			return true;
	}
	
	return false;
}
/*****************************************************************************/
}
