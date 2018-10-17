#pragma once

#define REGISTER_PYOBJECT(strClassName, ...)			PythonObjectFactory::getInstance()->registerConstructor(strClassName, __VA_ARGS__);
#define ALLOCATE_PYOBJECT(strType)						(*PythonObjectFactory::getInstance()->getConstructor(strType))

typedef map < string, boost::python::object > TStringBoostObjectMap;
/*****************************************************************************/
class PythonObjectFactory
{
public:

	static PythonObjectFactory* getInstance();
	~PythonObjectFactory();

	void registerConstructor(const char* pcsClassName, boost::python::object pClassObject);
	boost::python::object* getConstructor(const char* pcsClassName);

private:

	PythonObjectFactory();

private:

	static PythonObjectFactory* theInstance;

	TStringBoostObjectMap myConstructors;
};
/*****************************************************************************/
