#include "stdafx.h"

namespace HyperCore
{
/*****************************************************************************/
bool TokenizeUtils::tokenizeStringToFloats(const string& str, const char* pcsDelims, TFloatVector& vecOut)
{
	char cOrigChar;

	const int iConstCharRange = 256;
	char pcsDelimsLookupBuffer[iConstCharRange];
	memset(pcsDelimsLookupBuffer, 0, sizeof(char)*iConstCharRange);
	int iDelim;
	for(iDelim = 0; pcsDelims[iDelim]; iDelim++)
		pcsDelimsLookupBuffer[(unsigned char)pcsDelims[iDelim]] = 1;


	// Evil, but in the name of speed.
	vecOut.clear();
	char* pcsString = const_cast<char*>(str.c_str());
	int iChar;
	int iTokenStart = -1;
	for(iChar = 0; pcsString[iChar]; iChar++)
	{
		if(pcsString[iChar] == '<')
			return true;

		if(pcsDelimsLookupBuffer[(unsigned char)pcsString[iChar]] != 0)
		{
			if(iTokenStart + 1 < iChar)
			{

				cOrigChar = pcsString[iChar];
				pcsString[iChar] = 0;
				vecOut.push_back(atof(pcsString + iTokenStart + 1));
				pcsString[iChar] = cOrigChar;
			}
			iTokenStart = iChar;
		}
	}

	if(iTokenStart + 1 < iChar)
	{
		cOrigChar = pcsString[iChar];
		pcsString[iChar] = 0;
		vecOut.push_back(atof(pcsString + iTokenStart + 1));
		pcsString[iChar] = cOrigChar;
	}

	return false;
}
/*****************************************************************************/
bool TokenizeUtils::tokenizeStringToFloats(const string& str, const string& delimiters, FLOAT_TYPE*& fArrayOut, int& iCurrOffset)
{
	char cOrigChar;

	const char* pcsDelims = delimiters.c_str();
	const int iConstCharRange = 256;
	char pcsDelimsLookupBuffer[iConstCharRange];
	memset(pcsDelimsLookupBuffer, 0, sizeof(char)*iConstCharRange);
	int iDelim;
	for(iDelim = 0; pcsDelims[iDelim]; iDelim++)
		pcsDelimsLookupBuffer[(unsigned char)pcsDelims[iDelim]] = 1;

	// Evil, but in the name of speed.
	char* pcsString = const_cast<char*>(str.c_str());
	int iChar;
	//bool bFoundDelim;
	int iTokenStart = -1;
	for(iChar = 0; pcsString[iChar]; iChar++)
	{
		if(pcsString[iChar] == '<')
			return true;

		if(pcsDelimsLookupBuffer[(unsigned char)pcsString[iChar]] != 0)
		{
			if(iTokenStart + 1 < iChar)
			{

				cOrigChar = pcsString[iChar];
				pcsString[iChar] = 0;
				fArrayOut[iCurrOffset] = atof(pcsString + iTokenStart + 1);
				pcsString[iChar] = cOrigChar;

				iCurrOffset++;
			}
			iTokenStart = iChar;
		}
	}

	if(iTokenStart + 1 < iChar)
	{
		cOrigChar = pcsString[iChar];
		pcsString[iChar] = 0;
		fArrayOut[iCurrOffset] = atof(pcsString + iTokenStart + 1);
		pcsString[iChar] = cOrigChar;

		iCurrOffset++;
	}

	return false;
}
/*****************************************************************************/
void TokenizeUtils::tokenizeString(const string& str, const string& delimiters, TStringVector& tokens)
{
	tokens.clear();

	char cOrigChar;

	const char* pcsDelims = delimiters.c_str();
	const int iConstCharRange = 256;
	char pcsDelimsLookupBuffer[iConstCharRange];
	memset(pcsDelimsLookupBuffer, 0, sizeof(char)*iConstCharRange);
	int iDelim;
	for(iDelim = 0; pcsDelims[iDelim]; iDelim++)
		pcsDelimsLookupBuffer[(unsigned char)pcsDelims[iDelim]] = 1;

	// Evil, but in the name of speed.
	char* pcsString = const_cast<char*>(str.c_str());
	int iChar;
	int iTokenStart = -1;
	for(iChar = 0; pcsString[iChar]; iChar++)
	{
		if(pcsDelimsLookupBuffer[(unsigned char)pcsString[iChar]] != 0)
		{
			if(iTokenStart + 1 < iChar)
			{
				cOrigChar = pcsString[iChar];
				pcsString[iChar] = 0;
				tokens.push_back(string(pcsString + iTokenStart + 1));
				pcsString[iChar] = cOrigChar;
			}
			iTokenStart = iChar;
		}
	}

	if(iTokenStart + 1 < iChar)
	{
		 cOrigChar = pcsString[iChar];
		 pcsString[iChar] = 0;
		 tokens.push_back(string(pcsString + iTokenStart + 1));
		 pcsString[iChar] = cOrigChar;
	}
}
/*****************************************************************************/
void TokenizeUtils::tokenizeStringToCharPtrsInPlace(const string& str, const string& delimiters, TCharPtrVector& tokens)
{
	TokenizeUtils::tokenizeStringToCharPtrsInPlace(str.c_str(), delimiters, tokens);
}
/*****************************************************************************/
void TokenizeUtils::tokenizeStringToCharPtrsInPlace(const char* pcsStringIn, const string& delimiters, TCharPtrVector& tokens)
{
	// NOTE: This method is unsafe, but for the sake of speed...
	// The char data in the original string MUST stay the same for as long as the resulting vector is used!
	// Note that it is also DESTRUCTIVE, and will replace all delimeters with 0 chars PERMANENTLY!

	tokens.clear();

	const char* pcsDelims = delimiters.c_str();
	const int iConstCharRange = 256;
	char pcsDelimsLookupBuffer[iConstCharRange];
	memset(pcsDelimsLookupBuffer, 0, sizeof(char)*iConstCharRange);
	int iDelim;
	for(iDelim = 0; pcsDelims[iDelim]; iDelim++)
		pcsDelimsLookupBuffer[(unsigned char)pcsDelims[iDelim]] = 1;

	// Evil, but in the name of speed.
	char* pcsString = const_cast<char*>(pcsStringIn);
	int iChar;
	int iTokenStart = -1;
	for(iChar = 0; pcsString[iChar]; iChar++)
	{
		if(pcsDelimsLookupBuffer[(unsigned char)pcsString[iChar]] != 0)
		{
			if(iTokenStart + 1 < iChar)
			{
				pcsString[iChar] = 0;
				_ASSERT((pcsString + iTokenStart + 1) != NULL);
				tokens.push_back(pcsString + iTokenStart + 1);
			}
			iTokenStart = iChar;
		}
	}

	if(iTokenStart + 1 < iChar)
	{
		pcsString[iChar] = 0;
		_ASSERT((pcsString + iTokenStart + 1) != NULL);
		tokens.push_back(pcsString + iTokenStart + 1);
	}

}
/*****************************************************************************/
void TokenizeUtils::tokenizeStringToCharPtrsInPlace(const char* pcsStringIn, const string& delimitersToRemove, const string& delimitersToKeep, TStringVector& tokens)
{
	// NOTE: This method is unsafe, but for the sake of speed...
	// The char data in the original string MUST stay the same for as long as the resulting vector is used!
	// Note that it is also DESTRUCTIVE, and will replace all delimeters with 0 chars PERMANENTLY!

	tokens.clear();

	const int iConstCharRange = 256;
	int iDelim;

	const char* pcsDelims = delimitersToRemove.c_str();
	char pcsDelimsToRemoveLookupBuffer[iConstCharRange];
	memset(pcsDelimsToRemoveLookupBuffer, 0, sizeof(char)*iConstCharRange);
	for(iDelim = 0; pcsDelims[iDelim]; iDelim++)
		pcsDelimsToRemoveLookupBuffer[(unsigned char)pcsDelims[iDelim]] = 1;

	pcsDelims = delimitersToKeep.c_str();
	char pcsDelimsToKeepLookupBuffer[iConstCharRange];
	memset(pcsDelimsToKeepLookupBuffer, 0, sizeof(char)*iConstCharRange);
	for(iDelim = 0; pcsDelims[iDelim]; iDelim++)
		pcsDelimsToKeepLookupBuffer[(unsigned char)pcsDelims[iDelim]] = 1;


	// Evil, but in the name of speed.
	char pcsDelimTempBuff[2] = { 0, 0 };
	char* pcsString = const_cast<char*>(pcsStringIn);
	int iChar;
	int iTokenStart = -1;
	for(iChar = 0; pcsString[iChar]; iChar++)
	{
		if(pcsDelimsToRemoveLookupBuffer[(unsigned char)pcsString[iChar]] != 0
			|| pcsDelimsToKeepLookupBuffer[(unsigned char)pcsString[iChar]] != 0)
		{
			if(pcsDelimsToKeepLookupBuffer[(unsigned char)pcsString[iChar]] != 0)
				pcsDelimTempBuff[0] = pcsString[iChar];
			else
				pcsDelimTempBuff[0] = 0;
			if(iTokenStart + 1 < iChar)
			{
				pcsString[iChar] = 0;
				_ASSERT((pcsString + iTokenStart + 1) != NULL);
				tokens.push_back(pcsString + iTokenStart + 1);
			}
			if(pcsDelimTempBuff[0] != 0)
				tokens.push_back(pcsDelimTempBuff);
			iTokenStart = iChar;
		}
	}

	if(iTokenStart + 1 < iChar)
	{
		pcsString[iChar] = 0;
		_ASSERT((pcsString + iTokenStart + 1) != NULL);
		tokens.push_back(pcsString + iTokenStart + 1);
	}
}
/*****************************************************************************/
int TokenizeUtils::tokenizeStringToCharPtrsInPlace(const char* pcsStringIn, const string& delimiters, char** pcsTokensOut, int iMaxTokens)
{
	// NOTE: This method is unsafe, but for the sake of speed...
	// The char data in the original string MUST stay the same for as long as the resulting vector is used!
	// Note that it is also DESTRUCTIVE, and will replace all delimeters with 0 chars PERMANENTLY!

	int iNumParsedTokens = 0;
	memset(pcsTokensOut, 0, sizeof(char*)*iMaxTokens);

	const char* pcsDelims = delimiters.c_str();
	const int iConstCharRange = 256;
	char pcsDelimsLookupBuffer[iConstCharRange];
	memset(pcsDelimsLookupBuffer, 0, sizeof(char)*iConstCharRange);
	int iDelim;
	for(iDelim = 0; pcsDelims[iDelim]; iDelim++)
		pcsDelimsLookupBuffer[(unsigned char)pcsDelims[iDelim]] = 1;

	// Evil, but in the name of speed.
	char* pcsString = const_cast<char*>(pcsStringIn);
	int iChar;
	int iTokenStart = -1;
	for(iChar = 0; pcsString[iChar]; iChar++)
	{
		if(pcsDelimsLookupBuffer[(unsigned char)pcsString[iChar]] != 0)
		{
			if(iTokenStart + 1 < iChar)
			{
				pcsString[iChar] = 0;
				pcsTokensOut[iNumParsedTokens] = pcsString + iTokenStart + 1;
				iNumParsedTokens++;
				if(iNumParsedTokens >= iMaxTokens)
					break;
			}
			iTokenStart = iChar;
		}
	}

	if(iTokenStart + 1 < iChar && iNumParsedTokens < iMaxTokens)
	{
		pcsString[iChar] = 0;
		pcsTokensOut[iNumParsedTokens] = pcsString + iTokenStart + 1;
		iNumParsedTokens++;
	}

	return iNumParsedTokens;
}
/*****************************************************************************/
void TokenizeUtils::tokenizeStringUsingMemCacher(const string& str, const string& delimiters, TStringPtrVector& tokens, TStringMemoryCacher* pMemCacher, bool bSpecialUseFirstDelimOnlyOnce)
{
	tokens.clear();

	char cOrigChar;

	string *pStrPtr;

	const char* pcsDelims = delimiters.c_str();
	const int iConstCharRange = 256;
	char pcsDelimsLookupBuffer[iConstCharRange];
	memset(pcsDelimsLookupBuffer, 0, sizeof(char)*iConstCharRange);
	int iDelim;
	for(iDelim = 0; pcsDelims[iDelim]; iDelim++)
		pcsDelimsLookupBuffer[(unsigned char)pcsDelims[iDelim]] = 1;

	// Evil, but in the name of speed.
	char* pcsString = const_cast<char*>(str.c_str());
	int iChar;
	//bool bFoundDelim;
	int iTokenStart = -1;
	int iStartDelim = 0;
	for(iChar = 0; pcsString[iChar]; iChar++)
	{
/*
		bFoundDelim = false;
		for(iDelim = iStartDelim; pcsDelims[iDelim]; iDelim++)
		{
			if(pcsDelims[iDelim] == pcsString[iChar])
			{
				bFoundDelim = true;
				if(iDelim == 0 && bSpecialUseFirstDelimOnlyOnce)
					iStartDelim = 1;
				break;
			}
		}

		if(bFoundDelim)
*/
		if(pcsDelimsLookupBuffer[(unsigned char)pcsString[iChar]] != 0)
		{
			if(iTokenStart + 1 < iChar)
			{

				cOrigChar = pcsString[iChar];
				pcsString[iChar] = 0;
				pStrPtr = &pMemCacher->getNewObject()->myString;
				*pStrPtr = pcsString + iTokenStart + 1;
				tokens.push_back(pStrPtr);
				pcsString[iChar] = cOrigChar;
			}
			iTokenStart = iChar;
		}
	}

	if(iTokenStart + 1 < iChar)
	{
		cOrigChar = pcsString[iChar];
		pcsString[iChar] = 0;
		pStrPtr = &pMemCacher->getNewObject()->myString;
		*pStrPtr = pcsString + iTokenStart + 1;
		tokens.push_back(pStrPtr);
		pcsString[iChar] = cOrigChar;
	}
}
/*****************************************************************************/
void TokenizeUtils::takeFirstToken(const char* str, const string& delimiters, TStringMemoryCacher* pOptMemCacher, string& strOut)
{
	string* pFinalStringPtr;
	string strStorage;
	if(pOptMemCacher)
		pFinalStringPtr = &pOptMemCacher->getNewObject()->myString;
	else
		pFinalStringPtr = &strStorage;
	*pFinalStringPtr = str;

	const int iMaxTokens = 1;
	char* pcsTokens[iMaxTokens];
	int iNumTokensParsed = TokenizeUtils::tokenizeStringToCharPtrsInPlace(pFinalStringPtr->c_str(), delimiters, pcsTokens, iMaxTokens);
	if(iNumTokensParsed > 0)
		strOut = pcsTokens[0];
	else
		strOut = "";
}
/*****************************************************************************/
};