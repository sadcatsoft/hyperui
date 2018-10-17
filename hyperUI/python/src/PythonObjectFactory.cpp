#include "stdafx.h"
#include "PythonObjectFactory.h"

PythonObjectFactory* PythonObjectFactory::theInstance = NULL;
/*****************************************************************************/
PythonObjectFactory::PythonObjectFactory()
{

}
/*****************************************************************************/
PythonObjectFactory::~PythonObjectFactory()
{

}
/*****************************************************************************/
PythonObjectFactory* PythonObjectFactory::getInstance()
{
	if(!theInstance)
		theInstance = new PythonObjectFactory;
	return theInstance;
}
/*****************************************************************************/
void PythonObjectFactory::registerConstructor(const char* pcsClassName, boost::python::object pClassObject)
{
	myConstructors[pcsClassName] = pClassObject;
}
/*****************************************************************************/
boost::python::object* PythonObjectFactory::getConstructor(const char* pcsClassName)
{
	TStringBoostObjectMap::iterator mi = myConstructors.find(pcsClassName);
	if(mi == myConstructors.end())
		return NULL;
	else
		return &mi->second;
}
/*****************************************************************************/
