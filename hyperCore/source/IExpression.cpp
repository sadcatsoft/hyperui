#include "stdafx.h"

/*
Expression save form:

EXPR_TYPE:EXPR_CONTENTSOPT_EXPR_CONTEXT_TOKEN:OPT_EXPR_CONTEXT_TYPE:OPT_CUSTOM_CONTEXT_DATA

Example:
___pyexpr___:15+72___context___:___pynodectx___:13,23
*/

#define CONTEXT_SEPARATOR		"___context___"

namespace HyperCore
{
/*****************************************************************************/
// IExpressionContext
/*****************************************************************************/
void IExpressionContext::loadFrom(const char* pcsBase)
{
	// Skip our context type string:
	//pcsBase += g_pcsExpressionContextTypePrefixes[getType()].length() + strlen(EXPRESSION_PREFIX_SEP);
	pcsBase += STRLEN(getType()) + STRLEN(EXPRESSION_PREFIX_SEP);
	loadFromSubclass(pcsBase);
}
/*****************************************************************************/
void IExpressionContext::saveAppendToString(string& strInOut) const
{
	strInOut += CONTEXT_SEPARATOR;
	strInOut += EXPRESSION_PREFIX_SEP;
	//strInOut += g_pcsExpressionContextTypePrefixes[getType()];
	strInOut += getType();
	strInOut += EXPRESSION_PREFIX_SEP;
	saveAppendToStringSubclass(strInOut);	
}
/*****************************************************************************/
// IExpression
/*****************************************************************************/
IExpression::~IExpression()
{
	clearContext();
}
/*****************************************************************************/
void IExpression::stripExpressionPrefix(string& strInOut)
{
	int iPos = strInOut.find(EXPRESSION_PREFIX_SEP);
	if(iPos == string::npos)
		return;

	strInOut = strInOut.substr(iPos + 1);
}
/*****************************************************************************/
IExpression* IExpression::createExpressionFrom(const char* pcsText)
{
	return ExpressionTypeManager::getInstance()->createExpressionFrom(pcsText);
}
/*****************************************************************************/
void IExpression::updateWithText(const char* pcsText, bool bUpdateContext)
{
	myRawExpression = pcsText; 
	stripExpressionPrefix(myRawExpression);

	// Load any context data we may have:
	int iCtxPos = myRawExpression.find(CONTEXT_SEPARATOR);
	_ASSERT(bUpdateContext || iCtxPos == string::npos);
	if(bUpdateContext)
	{
		clearContext();
		if(iCtxPos != string::npos)
		{
			// Create and load our context
			// +1 for the separator
			const char* pcsContextBeginPtr = myRawExpression.c_str() + iCtxPos + strlen(CONTEXT_SEPARATOR) + strlen(EXPRESSION_PREFIX_SEP);
			//myContext = IExpressionContext::createContextFrom(pcsContextBeginPtr);
			myContext = ExpressionTypeManager::getInstance()->createContextFrom(pcsContextBeginPtr);
			myContext->loadFrom(pcsContextBeginPtr);

			myRawExpression = myRawExpression.substr(0, iCtxPos);
		}
	}

	myIsDirty = true;
}
/*****************************************************************************/
const char* IExpression::getResultAsString()
{
	ensureEvaluated();
	return myLastResult.c_str();
}
/*****************************************************************************/
void IExpression::ensureEvaluated()
{
	// Note that we cannot just set this based on the expression itself...
	// This is because what the expression points to may change...
// 	if(!myIsDirty)
// 		return;

	myLastError = "";
	bool bRes = evaluate(myRawExpression.c_str(), myLastResult);
	if(!bRes)
	{
		// Error. Set the error and set the result to 0.
		myLastError = myLastResult;
		myLastResult = "0";
	}
	myIsDirty = false;
}
/*****************************************************************************/
void IExpression::copyFrom(const IExpression* pOther)
{
	if(!pOther)
		return;
	myIsDirty = pOther->myIsDirty;
	myRawExpression = pOther->myRawExpression;
	myLastResult = pOther->myLastResult;
	myLastError = pOther->myLastError;

	clearContext();
	if(pOther->myContext)
		myContext = pOther->myContext->cloneSelf();
}
/*****************************************************************************/
IExpression* IExpression::cloneSelf() const
{
	IExpression* pNewCopy = IExpression::createExpressionFrom(myType.c_str());
	pNewCopy->copyFrom(this);
	return pNewCopy;
}
/*****************************************************************************/
bool IExpression::isEqualTo(const IExpression* pOther) const
{
	return myRawExpression == pOther->myRawExpression;
}
/*****************************************************************************/
double IExpression::getResultAsDoubleNumber()
{
	ensureEvaluated();
	return atof(myLastResult.c_str());
}
/*****************************************************************************/
FLOAT_TYPE IExpression::getResultAsNumber()
{
	ensureEvaluated();
	return atof(myLastResult.c_str());
}
/*****************************************************************************/
bool IExpression::getResultAsBool()
{
	ensureEvaluated();
	return myLastResult == TRUE_VALUE || myLastResult == "true" || myLastResult == "TRUE";
}
/*****************************************************************************/
void IExpression::getRawExpressionWithPrefix(string& strOut) const
{
	strOut = myType.c_str();
	strOut += ":";
	strOut += this->getRawExpressionContentsOnly();
}
/*****************************************************************************/
void IExpression::clearContext()
{
	delete myContext;
	myContext = NULL;
}
/*****************************************************************************/
void IExpression::saveToString(string& strOut) const
{
	strOut = myType.c_str();
	strOut += EXPRESSION_PREFIX_SEP;
	strOut += myRawExpression;

	// Save context
	if(myContext)
		myContext->saveAppendToString(strOut);
}
/*****************************************************************************/
};