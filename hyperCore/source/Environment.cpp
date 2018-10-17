#include "stdafx.h"

namespace HyperCore
{
Environment* Environment::theInstance = NULL;
/*****************************************************************************/
Environment::Environment()
{

}
/*****************************************************************************/
Environment::~Environment()
{

}
/*****************************************************************************/
Environment* Environment::getInstance()
{
	if(!theInstance)
		theInstance = new Environment;
	return theInstance;
}
/*****************************************************************************/
const char* Environment::getStringVar(const char* pcsVarName)
{
	ensureVarCached(pcsVarName);
	return myCachedVars.getStringProp(pcsVarName);
}
/*****************************************************************************/
bool Environment::getDoesVarExist(const char* pcsVarName)
{
	ensureVarCached(pcsVarName);
	return myCachedVars.doesPropertyExist(pcsVarName);
}
/*****************************************************************************/
void Environment::ensureVarCached(const char* pcsVarName)
{
	if(myCachedVars.doesPropertyExist(pcsVarName))
		return;

	if(getEnvVariable(pcsVarName, mySharedString))
		myCachedVars.setStringProp(pcsVarName, mySharedString.c_str());
}
/*****************************************************************************/
};