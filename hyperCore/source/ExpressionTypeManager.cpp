#include "stdafx.h"

namespace HyperCore
{
/*****************************************************************************/
ExpressionTypeManager::ExpressionTypeManager()
{
	myMaxExpressionTypeStringLength = 0;
}
ExpressionTypeManager* ExpressionTypeManager::theInstance = NULL;
/*****************************************************************************/
ExpressionTypeManager* ExpressionTypeManager::getInstance()
{
	if(!theInstance)
		theInstance = new ExpressionTypeManager;
	return theInstance;
}
/*****************************************************************************/
void ExpressionTypeManager::registerType(const CHAR_TYPE* pcsType, ExpressionConstructorType pExpressionConstructor, const CHAR_TYPE* pcsContextType, ExpressionContextConstructorType pContextConstructor)
{
	ExpressionAllocatorEntry rInfo;
	rInfo.myType = pcsType;
	rInfo.myConstructor = pExpressionConstructor;
	myExpressionTypes[pcsType] = rInfo;

	if(rInfo.myType.length() > myMaxExpressionTypeStringLength)
		myMaxExpressionTypeStringLength = rInfo.myType.length();

	ExpressionContextAllocatorEntry rContextInfo;
	rContextInfo.myType = pcsContextType;
	rContextInfo.myConstructor = pContextConstructor;
	myContextTypes[pcsType] = rContextInfo;
}
/*****************************************************************************/
IExpressionContext* ExpressionTypeManager::createContextFrom(const CHAR_TYPE* pcsString)
{
	TStringExpressionContextAllocatorMap::iterator mi;

	for(mi = myContextTypes.begin(); mi != myContextTypes.end(); mi++)
	{
		if(StringUtils::doesStartWith(pcsString, mi->second.myType.c_str()))
			return (*mi->second.myConstructor)();
	}

	return NULL;
}
/*****************************************************************************/
ExpressionAllocatorEntry* ExpressionTypeManager::findExpressionTypeEntry(const CHAR_TYPE* pcsString)
{
	if(!pcsString)
		return NULL;

	// This is used during load of any string in resource files, so it must
	// be as fast as is humanly possible. But still skip spaces, tabs, etc.
	int iCurr;
	int iStartPos = -1;
	int iEndPos = -1;
	int iPrefixLength = 0;
	for(iCurr = 0; pcsString[iCurr]; iCurr++)
	{
		if(pcsString[iCurr] == ' ' || pcsString[iCurr] == '\t' || pcsString[iCurr] == '\n' || pcsString[iCurr] == '\r')
			continue;

		// Now, we've got a valid char. If it's not 
		if(iStartPos < 0)
		{
			if(pcsString[iCurr] != '_')
				return NULL;
			iStartPos = iCurr;
			iPrefixLength = 1;
		}
		else if(pcsString[iCurr] == EXPRESSION_PREFIX_SEP_CHAR)
		{
			iEndPos = iCurr;
			break;
		}
		else if(iStartPos >= 0)
		{
			iPrefixLength++;
			if(iPrefixLength > myMaxExpressionTypeStringLength)
				return NULL;
		}
	}

	if(iStartPos < 0) //  || iEndPos < 0)
		return NULL;

	// Otherwise, we've got something that starts with an underscore
	// and has a colon in the string. See if it matches any of the 
	// expression types.
	TStringExpressionAllocatorMap::iterator mi;

	for(mi = myExpressionTypes.begin(); mi != myExpressionTypes.end(); mi++)
	{
		if(StringUtils::doesStartWith(&pcsString[iStartPos], mi->second.myType.c_str()))
			return &mi->second;
	}

	return NULL;
}
/*****************************************************************************/
IExpression* ExpressionTypeManager::createExpressionFrom(const CHAR_TYPE* pcsString)
{
	ExpressionAllocatorEntry* pEntry = this->findExpressionTypeEntry(pcsString);
	if(pEntry)
	{
		IExpression *pResult = (*pEntry->myConstructor)();
		if(pResult)
			pResult->updateWithText(pcsString, true);
		return pResult;
	}
	else
		return NULL;
}
/*****************************************************************************/
const CHAR_TYPE* ExpressionTypeManager::getExpressionType(const CHAR_TYPE* pcsString)
{
	ExpressionAllocatorEntry* pEntry = this->findExpressionTypeEntry(pcsString);
	if(pEntry)
		return pEntry->myType.c_str();
	else
		return EMPTY_STRING;
}
/*****************************************************************************/
};