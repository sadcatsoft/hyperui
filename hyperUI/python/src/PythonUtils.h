#pragma once

/*****************************************************************************/
class PythonUtils
{
public:

	// Error and extraction functions
	static long getLastErrorInformation(string& strErrorMessage, string& strFunctionName, string& strFileName);
	static void getLastErrorMessage(string& strCombinedErrorMessage);
	static void getStringFromObject(PyObject* pObject, string& strOut);
	static void getStringFromObject(boost::python::api::object_item& pObject, string& strOut);
	static void getStringFromObject(boost::python::object& pObject, string& strOut);

	// Execution
	static bool executeCodeNoReturnValue(const char* pcsCode, boost::python::dict *pOptNamespaceIn = NULL, string* pOptConsoleOutputOut = NULL);
	static bool evaluateExpression(const char* pcsCode, boost::python::dict *pOptNamespaceIn = NULL, string* pOptConsoleOutputOut = NULL, string* pOptTextResultOut = NULL);
	static bool executeFromConsole(const char* pcsCode, boost::python::dict *pOptNamespaceIn = NULL, string* pOptConsoleOutputOut = NULL, string* pOptTextResultOut = NULL);

	// Conversion
	static void dictToResourceItem(boost::python::dict* rDict, StringResourceItem& rItemOut);
	static bool objectToResourceProp(boost::python::object& rObj, StringResourceProperty* pPropOut);
	static bool objectToResourceProp(boost::python::object& rObj, ResourceProperty* pPropOut);
	static boost::python::object resourcePropToObject(ResourceProperty* pProp);
	static boost::python::object resourcePropToObject(StringResourceProperty* pProp);

	static boost::python::dict resourceItemToObject(ResourceItem* pItem);
	static boost::python::dict resourceItemToObject(StringResourceItem* pItem);

	// Misc
	static boost::python::dict *createAndInitNamespace();

private:
	static bool executeCodeuInternal(const char* pcsCode, bool bRunEval, boost::python::dict *pOptNamespaceIn, string* pOptConsoleOutputOut, string* pOptTextResultOut);
};
/*****************************************************************************/
