#pragma once

/*****************************************************************************/
class Environment
{
public:

	static Environment* getInstance();
	~Environment();

	const char* getStringVar(const char* pcsVarName);
	bool getDoesVarExist(const char* pcsVarName);

private:
	Environment();

	void ensureVarCached(const char* pcsVarName);

private:

	static Environment* theInstance;
	StringResourceItem myCachedVars;

	string mySharedString;
};
/*****************************************************************************/