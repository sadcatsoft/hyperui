#include "stdafx.h"
#include "PythonUtils.h"

using namespace boost::python;
/*****************************************************************************/
template < class TYPE, class PROPERTY_TYPE > 
bool objectToResourcePropTemplated(TYPE& rObj, bool bIgnoreListsAndComplexTypes, PROPERTY_TYPE* pPropOut)
{
	bool bDidSet = false;

	if(!pPropOut)
		return false;
	if(!bDidSet)
	{
		extract<FLOAT_TYPE> rFloatVal(rObj);
		if(rFloatVal.check())
		{
			FLOAT_TYPE fVal = rFloatVal();
			pPropOut->setNum(fVal);
			bDidSet = true;
		}
	}

	if(!bDidSet)
	{
		extract<int> rIntVal(rObj);
		if(rIntVal.check())
		{
			int iVal = rIntVal();
			pPropOut->setNum(iVal);
			bDidSet = true;
		}
	}

	if(!bDidSet)
	{
		extract<const char*> rStrVal(rObj);
		if(rStrVal.check())
		{
			const char* pcsVal = rStrVal();
			if(pcsVal)
			{
				// Check if this is an up number
				if(StringUtils::isAnUPNumber(pcsVal, -1))
				{
					// The function below modifies the actual string, so we must
					// store a copy:
					STRING_TYPE strCopy(pcsVal);
					pPropOut->setNum(HyperCore::convertUPNumberToScreenPixels(strCopy.c_str()));
				}
				else
					pPropOut->setString(pcsVal);
				bDidSet = true;
			}
		}
	}

	if(!bDidSet)
	{
		extract<const wchar_t*> errMessageGetterInner(rObj);
		if(errMessageGetterInner.check())
		{
			const wchar_t* pcsTemp = errMessageGetterInner();
			if(pcsTemp)
			{
				char* pcsAnsi = StringUtils::wideToAscii(pcsTemp);
				pPropOut->setString(pcsAnsi);
				StringUtils::freeAsciiText(pcsAnsi);
				bDidSet = true;
			}
		}
	}

	if(bIgnoreListsAndComplexTypes)
		return bDidSet;

	if(!bDidSet)
	{
		extract<boost::python::list> rListVal(rObj);
		if(rListVal.check())
		{
			boost::python::list rList = rListVal();

			int iCurr, iNumEntries = len(rList);

			bool bIsNumeric = false;
			bool bIsString = false;
			if(iNumEntries > 0)
			{
				// Test to see if it's a string or a num list
				boost::python::extract<FLOAT_TYPE> rNumVal(rList[0]);
				if(rNumVal.check())
					bIsNumeric = true;
				else
				{
					boost::python::extract<const char*> rStrVal(rList[0]);
					if(rStrVal.check())
						bIsString = true;
				}
			}

			pPropOut->reset(true);
			if(bIsNumeric)
			{
				for(iCurr = 0; iCurr < iNumEntries; iCurr++)
				{
					FLOAT_TYPE fVal = boost::python::extract<FLOAT_TYPE>(rList[iCurr]);
					pPropOut->addNumericEnumValue(fVal);
				}
			}
			else if(bIsString)
			{
				for(iCurr = 0; iCurr < iNumEntries; iCurr++)
				{
					const char* pcsString = boost::python::extract<const char*>(rList[iCurr]);
					pPropOut->addEnumValue(pcsString);
				}
			}
			if(bIsNumeric || bIsString)
				bDidSet = true;
		}
	}

	return bDidSet;
}
/*****************************************************************************/
template < class TYPE > 
bool convertComplexTypeToStringList(TYPE& rObj, string& strOut)
{
	bool bDidSet = false;
	strOut = "";

	boost::python::str strTemp(rObj);
	const char* pcsString = boost::python::extract<const char*>(strTemp);
	if(pcsString)
	{
		strOut = pcsString;
		bDidSet = true;
	}

	/*
	if(!bDidSet)
	{
		extract<boost::python::dict> rDictVal(rObj);
		if(rDictVal.check())
		{
			// Dicts not support for now! Should be!
			_ASSERT(0);
		}
	}
	if(!bDidSet)
	{
		extract<boost::python::list> rListVal(rObj);
		if(rListVal.check())
		{
			boost::python::list rList = rListVal();

			strOut += "[";
			int iCurr, iNumEntries = len(rList);
			for(iCurr = 0; iCurr < iNumEntries; iCurr++)
			{
				//fails here
				boost::python::str strTemp(rList[iCurr]);
				//object strFunc = rList[iCurr].attr("__str__");
				//const char* pcsString = boost::python::extract<const char*>(strFunc);
				const char* pcsString = boost::python::extract<const char*>(strTemp);

				if(iCurr > 0)
					strOut += ", ";
				strOut += pcsString;
			}

			strOut += "]";
			bDidSet = true;
		}
	}
	*/

	return bDidSet;
}
/*****************************************************************************/
template <  > 
bool convertComplexTypeToStringList<PyObject*>(PyObject*& rObj, string& strOut)
{
	return false;
}
/*****************************************************************************/
template < class TYPE > 
static void getStringFrom(TYPE& pObject, string& strOut)
{
	strOut = "";	

	StringResourceProperty rTempProp;
	if(objectToResourcePropTemplated<TYPE, StringResourceProperty>(pObject, true, &rTempProp))
		rTempProp.getAsString(strOut);
	// We might be a list or a map of things. Convert those
	else
		convertComplexTypeToStringList(pObject, strOut);

#if 0

    extract<const char*> errMessageGetter(pObject);
	if(errMessageGetter.check())
	{
		const char* pcsPtr = errMessageGetter();
		if(pcsPtr)
		{
			strOut = pcsPtr;
			return;
		}
	}

/*
	if(strOut.length() == 0)
	{
		extract<string> errMessageGetterInner(pObject);
		if(errMessageGetterInner.check())
			strOut = errMessageGetterInner();
	}
*/
	if(strOut.length() == 0)
	{
		extract<const wchar_t*> errMessageGetterInner(pObject);
		if(errMessageGetterInner.check())
		{
			const wchar_t* pcsTemp = errMessageGetterInner();
			if(pcsTemp)
			{
				char* pcsAnsi = WideToAnsi(pcsTemp);
				strOut = pcsAnsi;
				FreeAnsiText(pcsAnsi);
				return;
			}
		}
	}

// 	if(strOut.length() == 0)
// 		strOut = PROPERTY_NONE;
#endif

}
/*****************************************************************************/
long PythonUtils::getLastErrorInformation(string& strErrorMessage, string& strFunctionName, string& strFileName)
{

	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	PyErr_Clear();

	handle<> hType(ptype);
	object extype(hType);

	handle<> vType(pvalue);
	object vtype(vType);

	getStringFromObject(pvalue, strErrorMessage);

	long lineno = 0;
	if(ptraceback)
	{
		handle<> hTraceback(ptraceback);
		object traceback(hTraceback);

		lineno = extract<long> (traceback.attr("tb_lineno"));

		object objFilename(traceback.attr("tb_frame").attr("f_code").attr("co_filename"));
		getStringFrom<object>(objFilename, strFileName);
		object objFuncName(traceback.attr("tb_frame").attr("f_code").attr("co_name"));
		getStringFrom<object>(objFuncName, strFunctionName);
	}
	else
	{
		strFileName = "";
		strFunctionName = "";
	}

	return lineno;
}
/*****************************************************************************/
void PythonUtils::getStringFromObject(PyObject* pObject, string& strOut)
{
	PyObject *pStrValue = PyObject_Str(pObject);
	getStringFrom<PyObject*>(pStrValue, strOut);
	Py_XDECREF(pStrValue);
}
/*****************************************************************************/
void PythonUtils::getStringFromObject(boost::python::api::object_item& pObject, string& strOut)
{
	getStringFrom<boost::python::api::object_item>(pObject, strOut);
}
/*****************************************************************************/
void PythonUtils::getStringFromObject(boost::python::object& pObject, string& strOut)
{
	getStringFrom<boost::python::object>(pObject, strOut);
}
/*****************************************************************************/
void PythonUtils::getLastErrorMessage(string& strCombinedErrorMessage)
{
	string strErrorMessage;
	string strFunctionName; 
	string strFileName;
	string strLineNum;
	long lLineNum = getLastErrorInformation(strErrorMessage, strFunctionName, strFileName);

	StringUtils::longNumberToString(lLineNum, strLineNum);
	strCombinedErrorMessage = "Error in file " + strFileName + " function " + strFunctionName + " line " + strLineNum + " : " + strErrorMessage + "\n";
}
/*****************************************************************************/
bool PythonUtils::executeCodeNoReturnValue(const char* pcsCode, boost::python::dict *pOptNamespaceIn /*= NULL*/, string* pOptConsoleOutputOut /*= NULL*/)
{
	return executeCodeuInternal(pcsCode, false, pOptNamespaceIn, pOptConsoleOutputOut, NULL);
}
/*****************************************************************************/
bool PythonUtils::evaluateExpression(const char* pcsCode, boost::python::dict *pOptNamespaceIn /*= NULL*/, string* pOptConsoleOutputOut /*= NULL*/, string* pOptTextResultOut /*= NULL*/)
{
	bool bRes = executeCodeuInternal(pcsCode, true, pOptNamespaceIn, pOptConsoleOutputOut, pOptTextResultOut);
	// This is needed for statements such as print('test'), which will produce both the result
	// (None) and the actual printed output.
	if(pOptConsoleOutputOut && pOptConsoleOutputOut->length() && pOptTextResultOut)
		*pOptTextResultOut = "";
	return bRes;
}
/*****************************************************************************/
void PythonUtils::dictToResourceItem(boost::python::dict* rDict, StringResourceItem& rItemOut)
{
	rItemOut.clearEverything();

	if(!rDict)
		return;

	const char* pcsKey;
	boost::python::list rKeys = rDict->keys();
	boost::python::object rTempObj;
	int iCurr, iNumEntries = len(rKeys);
	for(iCurr = 0; iCurr < iNumEntries; iCurr++)
	{
		pcsKey = boost::python::extract<const char*>(rKeys[iCurr]);
	
		// Create the default prop
		rTempObj = rDict->get(pcsKey);
		rItemOut.setNumProp(pcsKey, 0.0);
		objectToResourceProp(rTempObj, rItemOut.findProperty(pcsKey));
	}

}
/*****************************************************************************/
bool PythonUtils::objectToResourceProp(boost::python::object& rObj, StringResourceProperty* pPropOut)
{
	return objectToResourcePropTemplated<boost::python::object, StringResourceProperty>(rObj, false, pPropOut);
}
/*****************************************************************************/
bool PythonUtils::objectToResourceProp(boost::python::object& rObj, ResourceProperty* pPropOut)
{
	return objectToResourcePropTemplated<boost::python::object, ResourceProperty>(rObj, false, pPropOut);
}
/*****************************************************************************/
template < class PROP_TYPE >
boost::python::object resourcePropToObjectTemplated(PROP_TYPE* pProp)
{
	if(!pProp)
		return boost::python::object();
	
	PropertyDataType eDataType = pProp->getPropertyDataType();
	if(eDataType == PropertyDataNumber)
	{
		FLOAT_TYPE fValue = pProp->getAsNumber();
		if((FLOAT_TYPE)((int)fValue) == fValue)
			return boost::python::object((int)fValue);
		else
			return boost::python::object(fValue);
	}
	else if(eDataType == PropertyDataString)
	{
		string strValue;
		pProp->getAsString(strValue);
		return boost::python::object(strValue);
	}
	else if(eDataType == PropertyDataBool)
		return boost::python::object(pProp->getBool());
	else if(eDataType == PropertyDataDouble)
		return boost::python::object(pProp->getDouble());
	else if(eDataType == PropertyDataStringList)
	{
		string strValue;
		boost::python::list vecResult;
		int iCurr, iNum = pProp->getNumEnumValues();
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			strValue = pProp->getEnumValue(iCurr);
			vecResult.append(boost::python::object(strValue));
		}
		return vecResult;
	}
	else if(eDataType == PropertyDataNumberList)
	{
		boost::python::list vecResult;
		int iCurr, iNum = pProp->getNumNumericEnumValues();
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			FLOAT_TYPE fValue = pProp->getNumericEnumValue(iCurr);
			if((FLOAT_TYPE)((int)fValue) == fValue)
				vecResult.append(boost::python::object((int)fValue));
			else
				vecResult.append(boost::python::object(fValue));
		}
		return vecResult;
	}
	ELSE_ASSERT;

	return boost::python::object();
}
/*****************************************************************************/
boost::python::object PythonUtils::resourcePropToObject(ResourceProperty* pProp)
{
	return resourcePropToObjectTemplated<ResourceProperty>(pProp);
}
/*****************************************************************************/
boost::python::object PythonUtils::resourcePropToObject(StringResourceProperty* pProp)
{
	return resourcePropToObjectTemplated<StringResourceProperty>(pProp);
}
/*****************************************************************************/
bool PythonUtils::executeCodeuInternal(const char* pcsCode, bool bRunEval, boost::python::dict *pOptNamespaceIn, 
													string* pOptConsoleOutputOut, string* pOptTextResultOut)
{

	bool bDeallocNameSpace = false;
	bool bResult = false;
	if(pOptConsoleOutputOut)
		*pOptConsoleOutputOut = "";
	if(pOptTextResultOut)
		*pOptTextResultOut = "";

	try
	{
		// Redirect out std out:
		std::string strOutputRedirect =
"import sys\n\
class CatchOutErr:\n\
	def __init__(self):\n\
		self.value = ''\n\
	def write(self, txt):\n\
		self.value += txt\n\
catchOutErr = CatchOutErr()\n\
sys.stdout = catchOutErr\n\
";

		// Run the actual code
		object main_module = import("__main__");
		boost::python::dict main_namespace;
		if(!pOptNamespaceIn)
		{
			pOptNamespaceIn = createAndInitNamespace();
			bDeallocNameSpace = true;
		}
		main_namespace = *pOptNamespaceIn;

/*
		object main_namespace;
		if(pOptNamespaceIn)
		{
			main_module = *pOptNamespaceIn;
			main_namespace = pOptNamespaceIn->attr("__dict__");
		}
		else
		{
			main_module = import("__main__");
			main_namespace = main_module.attr("__dict__");
		}
*/
		//object ignored = exec("import " APP_PYTHON_MODULE, main_namespace);
		//object ignoredVal = exec(strOutputRedirect.c_str(), main_namespace);
		///////strOutputRedirect = ("import " APP_PYTHON_MODULE "\n") + strOutputRedirect;
		object ignoredVal = exec(strOutputRedirect.c_str(), main_namespace);
		object ignoredVal2;

		if(bRunEval)
			ignoredVal2 = eval(pcsCode, main_namespace);
		else
			ignoredVal2 = exec(pcsCode, main_namespace);

		// Get the std out:
		//object catcher = main_module.attr("catchOutErr");
		object catcher = main_namespace.get("catchOutErr");
		object stdOutput = catcher.attr("value");

 		if(pOptConsoleOutputOut)
		{
			if(stdOutput.is_none())
				*pOptConsoleOutputOut = "None";
			else
 				getStringFrom<object>(stdOutput, *pOptConsoleOutputOut);
		}

		if(pOptTextResultOut)
		{
			if(ignoredVal2.is_none())
				*pOptTextResultOut = "None";
			else
				getStringFrom<object>(ignoredVal2, *pOptTextResultOut);
		}
		bResult = true;
	}
	catch (error_already_set const &)
	{
		string strError;
		getLastErrorMessage(strError);
		if(pOptConsoleOutputOut)
			*pOptConsoleOutputOut = strError;
		gLog("PYTHON ERROR: \n");
		gLog(strError.c_str());
		gLog("Code:\n");
		gLog(pcsCode);
		gLog("\n");
	}

	if(bDeallocNameSpace)
	{
		pOptNamespaceIn->clear();
		delete pOptNamespaceIn;
	}

	return bResult;
}
/*****************************************************************************/
bool PythonUtils::executeFromConsole(const char* pcsCode, boost::python::dict *pOptNamespaceIn, string* pOptConsoleOutputOut, string* pOptTextResultOut)
{
	// We first attempt to evaluate it, if that fails, we execute the code.
	bool bResult = evaluateExpression(pcsCode, pOptNamespaceIn, pOptConsoleOutputOut, pOptTextResultOut);
	if(bResult)
		return true;

	return executeCodeNoReturnValue(pcsCode, pOptNamespaceIn, pOptConsoleOutputOut);
}
/*****************************************************************************/
template < class ITEM_TYPE, class PROP_TYPE >
boost::python::dict resourceItemToObjectTemplated(ITEM_TYPE* pItem)
{
	boost::python::dict pRes;
	PROP_TYPE *pProp;
	typename ITEM_TYPE::PropertyIterator pi;
	const char* pcsPropString;
	for(pi = pItem->propertiesFirst(); !pi.isEnd(); pi++)
	{
		pProp = pItem->findProperty(pi.getProperty());
		boost::python::object pyProp = resourcePropToObjectTemplated<PROP_TYPE>(pProp);
		pcsPropString = HyperCore::PropertyMapper::getInstance()->getPropertyString(pi.getProperty());
		pRes[pcsPropString] = pyProp;
	}

	return pRes;
}
/*****************************************************************************/
boost::python::dict PythonUtils::resourceItemToObject(ResourceItem* pItem)
{
	return resourceItemToObjectTemplated<ResourceItem, ResourceProperty>(pItem);
}
/*****************************************************************************/
boost::python::dict PythonUtils::resourceItemToObject(StringResourceItem* pItem)
{
	return resourceItemToObjectTemplated<StringResourceItem, StringResourceProperty>(pItem);
}
/*****************************************************************************/
boost::python::dict* PythonUtils::createAndInitNamespace()
{

	object main_module = import("__main__");
	boost::python::dict origNamespace = extract<boost::python::dict>(main_module.attr("__dict__"));
	boost::python::dict* pRes = new boost::python::dict();
	*pRes = origNamespace.copy();
	return pRes;
}
/*****************************************************************************/

