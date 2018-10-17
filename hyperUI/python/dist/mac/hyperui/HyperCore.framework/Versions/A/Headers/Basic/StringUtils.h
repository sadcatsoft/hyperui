#pragma once

/*****************************************************************************/
class HYPERCORE_API StringUtils
{
public:	

	static bool isANumber(STRING_TYPE& str, bool bTreatPlusAsNumber, bool *pOptIsDoublePrecisionOut = NULL);
	static bool isANumber(const CHAR_TYPE* str, bool bTreatPlusAsNumber, bool *pOptIsDoublePrecisionOut = NULL);
	static bool isAnUPNumber(const char* pcsTest, int iLen);
	static bool isAnUPNumber(const string& strTest);

	static void trimFront(STRING_TYPE& strInOut, const CHAR_TYPE* pcsChars);
	static void trimBack(STRING_TYPE& strInOut, const CHAR_TYPE* pcsChars);
	static void trim(STRING_TYPE& strInOut, const CHAR_TYPE* pcsChars);
	static bool doesStartWith(const CHAR_TYPE* pcsString, const CHAR_TYPE* pcsPrefix, bool bCaseInsensitive = false);
	static STRING_TYPE encodeUrl(const STRING_TYPE &strStringIn);
	static bool isTrueString(const CHAR_TYPE* pcsString);
	static void shortenString(STRING_TYPE& strInOut, int iMaxChars, const CHAR_TYPE* pcsAppended);
	static void replaceAllInstances(const CHAR_TYPE* pcsFind, const CHAR_TYPE* pcsReplace, STRING_TYPE& strInOut);
	static void replaceAnyCharacterIn(const char* pcsChars, const char* pcsReplacement, string& strInOut);
	static void stripTrailingDigits(string& strInOut);
	static void capitalizeWords(char cSeparator, string& strInOut);
	static bool doesEndInDigits(const char* pcsString);
	static void removeControlCharacters(STRING_TYPE& strInOut, bool bEscapeQuotes = true);
	static void incrementStringSuffix(string& strInOut, int iStartNumber = 1, bool bAddSpace = false);
	static void convertStringVectorToString(TStringVector& rVecIn, string& strOut, bool bAddNewlines);
	static bool isANumberIgnoreSpaces(const char* pcsVal, bool bTreatPlusAsNumber, bool bAllowComma);
	static const CHAR_TYPE* modifyStringPointerToTrim(CHAR_TYPE cChar, STRING_TYPE& strInOut);
	static void ensureAlphaNumericAnd(const char* pcsExtraAllowedChars, char cReplacement, string& strInOut);
	static void ensureValidForAlphaNumMap(string& strInOut);

	static const char* skipSymbols(const char* pcsString, const char* pcsSymbols);
	static const char* skipUntilSymbols(const char* pcsString, const char* pcsSymbols);

	static FLOAT_TYPE convertStringToNumber(const string& strIn);

	// These guys insert commas, so use them for UI, NOT number-to-string conversion.
	static void numberToNiceString(FLOAT_TYPE fNum, string& strOut);
	static void numberToNiceString(FLOAT_TYPE fNum, int iDecimals, bool bDollar, string& strOut);
	static void numberToNiceSizeString(long long llSize, int iPrecDigits, string& strOut);

	// NOTE that this one does not insert commas, and is for pure conversion purposes (non-UI)
	static void numberToString(FLOAT_TYPE fNum, string& strOut);
	static void numberToString(FLOAT_TYPE fNum, const char* pcsFormat, string& strOut);
	static void longNumberToString(UNIQUEID_TYPE lNum, string& strOut);

	static void numberToRomanString(int iNumber, string& strOut);
	static void numberToWords(int iNumber, string& strOut, bool bCapitalize = false);

	static char* wideToAscii(const wchar_t* strPtr);
	static wchar_t* asciiToWide(const char* strPtr);
	static void freeAsciiText(char* strPtr);
	static void freeWideText(wchar_t* strPtr);

	static int findFrameNum(const char* pcsString, int& iAtlasFrameOut);
};
/*****************************************************************************/
