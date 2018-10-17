#pragma once

/*****************************************************************************/
class TokenizeUtils
{
public:
	HYPERCORE_API static void tokenizeString(const string& str, const string& delimiters, TStringVector& tokens);
	HYPERCORE_API static void tokenizeStringToCharPtrsInPlace(const string& str, const string& delimiters, TCharPtrVector& tokens);
	HYPERCORE_API static void tokenizeStringToCharPtrsInPlace(const char* pcsStringIn, const string& delimiters, TCharPtrVector& tokens);
	HYPERCORE_API static void tokenizeStringToCharPtrsInPlace(const char* pcsStringIn, const string& delimitersToRemove, const string& delimitersToKeep, TStringVector& tokens);
	HYPERCORE_API static int tokenizeStringToCharPtrsInPlace(const char* pcsStringIn, const string& delimiters, char** pcsTokensOut, int iMaxTokens);
	HYPERCORE_API static bool tokenizeStringToFloats(const string& str, const string& delimiters, FLOAT_TYPE*& fArrayOut, int& iCurrOffset);
	HYPERCORE_API static bool tokenizeStringToFloats(const string& str, const char* delimiters, TFloatVector& vecOut);

	HYPERCORE_API static void tokenizeStringUsingMemCacher(const string& str, const string& delimiters, TStringPtrVector& tokens, TStringMemoryCacher* pMemCacher, bool bSpecialUseFirstDelimOnlyOnce);
	HYPERCORE_API static void takeFirstToken(const char* str, const string& delimiters, TStringMemoryCacher* pOptMemCacher, string& strOut);

};
/*****************************************************************************/