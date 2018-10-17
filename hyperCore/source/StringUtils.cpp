#include "stdafx.h"

namespace HyperCore
{
/*****************************************************************************/
bool StringUtils::isANumber(STRING_TYPE& str, bool bTreatPlusAsNumber, bool *pOptIsDoublePrecisionOut)
{
	return StringUtils::isANumber(str.c_str(), bTreatPlusAsNumber, pOptIsDoublePrecisionOut);
}
/*****************************************************************************/
bool StringUtils::isANumber(const CHAR_TYPE* str, bool bTreatPlusAsNumber, bool *pOptIsDoublePrecisionOut)
{
	if(pOptIsDoublePrecisionOut)
		*pOptIsDoublePrecisionOut = false;

	if(!str)
		return false;

	// An empty string is *not* a number
	if(str[0] == 0)
		return false;

	int iNumDigits = 0;
	bool bIsCurrDigit;
	int iChar;
	for(iChar = 0; str[iChar]; iChar++)
	{
		bIsCurrDigit = ISDIGIT(str[iChar]);

		if(!ISDIGIT(str[iChar]) && str[iChar]!= '-' && str[iChar]!= '+'
			&& str[iChar]!= '.')
			return false;

		if(str[iChar] == '-' && iChar > 0)
			return false;

		if(!bTreatPlusAsNumber && str[iChar]== '+')
			return false;

		if(pOptIsDoublePrecisionOut && bIsCurrDigit)
			iNumDigits++;
	}

	if(pOptIsDoublePrecisionOut)
		*pOptIsDoublePrecisionOut = iNumDigits > 7;

	return true;
}
/*****************************************************************************/
void StringUtils::trim(STRING_TYPE& strInOut, const CHAR_TYPE* pcsChars)
{
	StringUtils::trimFront(strInOut, pcsChars);
	StringUtils::trimBack(strInOut, pcsChars);
}
/*****************************************************************************/
void StringUtils::trimFront(STRING_TYPE& strInOut, const CHAR_TYPE* pcsChars)
{
	CHAR_TYPE pcsBuff[2] = { 0, 0 };
	int iCurr, iNum = strInOut.length();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pcsBuff[0] = strInOut[iCurr];
		if(!STRSTR(pcsChars, pcsBuff))
			break;
	}

	if(iCurr > 0)
		strInOut = strInOut.substr(iCurr);
}
/*****************************************************************************/
void StringUtils::trimBack(STRING_TYPE& strInOut, const CHAR_TYPE* pcsChars)
{
	CHAR_TYPE pcsBuff[2] = { 0, 0 };
	int iCurr, iNum = strInOut.length();
	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
	{
		pcsBuff[0] = strInOut[iCurr];
		if(!STRSTR(pcsChars, pcsBuff))
			break;
	}

	if(iCurr < iNum - 1)
		strInOut = strInOut.substr(0, iCurr + 1);
}
/*****************************************************************************/
STRING_TYPE StringUtils::encodeUrl(const STRING_TYPE &strStringIn)
{
	//RFC 3986 section 2.3 Unreserved Characters (January 2005)
	const STRING_TYPE unreserved = STR_LIT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~");

	short iInner, iLen;
	// I'm pretty sure these need to be actual char* s...
	UNSIGNED_CHAR_TYPE cChar;
	CHAR_TYPE pcsTempUniBuffer[4] = { 0,0,0,0 };
	CHAR_TYPE pcsAppendBuff[3] = { 0,0,0 };
	STRING_TYPE escaped = EMPTY_STRING;
	CHAR_TYPE buf[32];
	for(size_t i=0; i < strStringIn.length(); i++)
	{
		if (unreserved.find_first_of(strStringIn[i]) != STRING_TYPE::npos)
		{
			escaped.push_back(strStringIn[i]);
		}
		else
		{
			// This is needed to encode Unicode characters correctly.
			cChar = strStringIn[i];
			if(cChar < 128)
			{
				pcsTempUniBuffer[0] = cChar;
				pcsTempUniBuffer[1] = 0;
			}
			else
			{
				pcsTempUniBuffer[0] = (cChar >> 6) | 0xC0;
				pcsTempUniBuffer[1] = (cChar & 0x3F) | 0x80;
				pcsTempUniBuffer[2] = 0;
			}

			for(iInner = 0; pcsTempUniBuffer[iInner] != 0; iInner++)
			{
				escaped.append(STR_LIT("%"));
				SPRINTF(buf, 32, STR_LIT("%.2X"), pcsTempUniBuffer[iInner]);

				// For negative values, we will have something like FFFFFFC3. We just need the
				// last two bytes.
				iLen = STRLEN(buf);
				pcsAppendBuff[0] = buf[iLen - 2];
				pcsAppendBuff[1] = buf[iLen - 1];
				escaped.append(pcsAppendBuff);
			}
		}
	}
	return escaped;
}
/********************************************************************************************/
bool StringUtils::doesStartWith(const char* pcsString, const char* pcsPrefix, bool bCaseInsensitive)
{
	size_t lenpre = STRLEN(pcsPrefix);
	size_t lenstr = STRLEN(pcsString);

	int iPos;
	if(lenpre > lenstr)
		return false;

	if(bCaseInsensitive)
	{
		for(iPos = 0; iPos < lenpre; iPos++)
		{
			if(TOLOWER(pcsPrefix[iPos]) != TOLOWER(pcsString[iPos]))
				return false;
		}
	}
	else
	{
		return lenstr < lenpre ? false : STRNCMP(pcsPrefix, pcsString, lenpre) == 0;
	}

	return true;
}
/*****************************************************************************/
bool StringUtils::isTrueString(const CHAR_TYPE* pcsString)
{
	if(STRCMP(pcsString, TRUE_VALUE) == 0 || STRCMP(pcsString, "true") == 0)
		return true;
	else
		return false;
}
/********************************************************************************************/
void StringUtils::shortenString(STRING_TYPE& strInOut, int iMaxChars, const CHAR_TYPE* pcsAppended)
{
	if(iMaxChars < 2)
		return;

	// Note that we don't bother if there is only one letter left to remove - since
	// the space taken by the appended string would be the same or more.
	if(strInOut.length() > iMaxChars + 1)
	{
		// If we're shortening at a space, move back one.
		if(strInOut[iMaxChars - 1] == ' ')
			iMaxChars--;

		strInOut = strInOut.substr(0, iMaxChars);
		if(pcsAppended && STRLEN(pcsAppended) > 0)
			strInOut += pcsAppended;
	}
}
/********************************************************************************************/
void StringUtils::replaceAllInstances(const CHAR_TYPE* pcsFind, const CHAR_TYPE* pcsReplace, STRING_TYPE& strInOut)
{
	int iFindLen = STRLEN(pcsFind);
	int iNewLen = STRLEN(pcsReplace);
	size_t pos = 0;
	while((pos = strInOut.find(pcsFind, pos)) != std::string::npos)
	{
		strInOut.replace(pos, iFindLen, pcsReplace);
		pos += iNewLen;
	}
}
/*****************************************************************************/
void StringUtils::replaceAnyCharacterIn(const char* pcsChars, const char* pcsReplacement, string& strInOut)
{
	_ASSERT(pcsReplacement);
	char pcsSmallBuff[2] = { 0, 0 };
	int iCurr, iLen = strlen(pcsChars);
	for(iCurr = 0; iCurr < iLen; iCurr++)
	{
		pcsSmallBuff[0] = pcsChars[iCurr];
		StringUtils::replaceAllInstances(pcsSmallBuff, pcsReplacement, strInOut);
	}
}
/*****************************************************************************/
void StringUtils::stripTrailingDigits(string& strInOut)
{
	int iCurr, iNum = strInOut.length();
	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
	{
		if(!isdigit(strInOut[iCurr]))
			break;
	}

	if(iCurr < iNum - 1)
		strInOut = strInOut.substr(0, iCurr + 1);
}
/********************************************************************************************/
void StringUtils::capitalizeWords(char cSeparator, string& strInOut)
{
	int iCurr, iLen = strInOut.length();
	char* pcsBuff = const_cast<char*>(strInOut.c_str());
	for(iCurr = 0; iCurr < iLen; iCurr++)
	{
		if((iCurr == 0 || (iCurr > 0 && pcsBuff[iCurr - 1] == cSeparator))
			&& isalpha(pcsBuff[iCurr]))
			pcsBuff[iCurr] = toupper(pcsBuff[iCurr]);
	}
}
/*****************************************************************************/
bool StringUtils::doesEndInDigits(const char* pcsString)
{
	if(!pcsString)
		return false;
	int iLen = strlen(pcsString);
	return isdigit(pcsString[iLen - 1]);
}
/*****************************************************************************/
void StringUtils::removeControlCharacters(STRING_TYPE& strInOut, bool bEscapeQuotes)
{
	const CHAR_TYPE* pcsSource[] = { STR_LIT("\\"), STR_LIT("/"), STR_LIT("\b"), STR_LIT("\f"), STR_LIT("\n"), STR_LIT("\r"), STR_LIT("\t"), 0 };
	const CHAR_TYPE* pcsReplacement[] = { STR_LIT("\\\\"), STR_LIT("\/"), STR_LIT("\\b"), STR_LIT("\\f"), STR_LIT("\\n"), STR_LIT("\\r"), STR_LIT("\\t"), 0 };

	int iCurr;
	for(iCurr = 0; pcsSource[iCurr] != NULL; iCurr++)
	{
		StringUtils::replaceAllInstances(pcsSource[iCurr], pcsReplacement[iCurr], strInOut);
	}

	// Note that it's important to escape this *after* the original, not before...
	if(bEscapeQuotes)
		StringUtils::replaceAllInstances(STR_LIT("\""), STR_LIT("\\\""), strInOut);

	// Now, eliminate everything else.
	CHAR_TYPE* pcsData = const_cast<CHAR_TYPE*>(strInOut.c_str());
	int iNum = STRLEN(pcsData);
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(pcsData[iCurr] < 0x1f)
			pcsData[iCurr] = ' ';
	}
}
/*****************************************************************************/
void StringUtils::incrementStringSuffix(string& strInOut, int iStartNumber, bool bAddSpace)
{
	int iNumPos;
	int iCurr, iNum = strInOut.length();
	char pcsDefBuff[2] = { (const char)('0' + iStartNumber), 0 };
	if(iNum <= 0)
	{
		strInOut = pcsDefBuff;
		return;
	}

	char* pcsBuff = const_cast<char*>(strInOut.c_str());

	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
	{
		if(!isdigit(pcsBuff[iCurr]))
			break;
	}
	iNumPos = iCurr;

	if(iNumPos == iNum - 1)
	{
		if(bAddSpace)
			strInOut += " ";
		strInOut = strInOut + pcsDefBuff;
		return;
	}

	_ASSERT(isdigit(pcsBuff[iNum - 1]));

	if(pcsBuff[iNum - 1] != '9')
	{
		pcsBuff[iNum - 1] = ((int)(pcsBuff[iNum - 1] - '0') + 1) + '0';
		return;
	}

	int iSuffix = atoi(pcsBuff + iNumPos + 1);
	iSuffix++;
	char pcsBuff2[128];
	sprintf(pcsBuff2, "%d", iSuffix);
	strInOut = strInOut.substr(0, iNumPos + 1) + pcsBuff2;
}
/*****************************************************************************/
bool StringUtils::isAnUPNumber(const string& strTest)
{
	return StringUtils::isAnUPNumber(strTest.c_str(), strTest.length());
}
/********************************************************************************************/
bool StringUtils::isAnUPNumber(const char* pcsTest, int iLen)
{
	// Careful, this needs to be fast.
	bool bResult = false;
	//char *pcsBuff = const_cast<char *>(strTest.c_str());
	char *pcsBuff = const_cast<char *>(pcsTest);
	//int iLen = strTest.length();
	if(iLen <= 0)
		iLen = strlen(pcsTest);
	if(iLen > 2 && pcsBuff[iLen - 2] == 'u' && pcsBuff[iLen - 1] == 'p')
	{
		// See if the rest of the string is a true number.
		// We can't really do a substring, since this might
		// get called quite often. So we do the evil thing
		// and just insert a zero where we need it.
		pcsBuff[iLen - 2] = 0;
		bResult = StringUtils::isANumber(pcsBuff, true);
		pcsBuff[iLen - 2] = 'u';
	}

	return bResult;
}
/********************************************************************************************/
void StringUtils::convertStringVectorToString(TStringVector& rVecIn, string& strOut, bool bAddNewlines)
{
	int iLine, iNumLines = rVecIn.size();
	strOut = "";
	for(iLine = 0; iLine < iNumLines; iLine++)
	{
		strOut += rVecIn[iLine];
		if(bAddNewlines)
		{
#ifdef WINDOWS
			strOut += "\n";
#else
			// At least on windows, this generates double-spacing...
			strOut += NEWLINE_SEPARATORS;
#endif
		}
	}
}
/********************************************************************************************/
bool StringUtils::isANumberIgnoreSpaces(const char* pcsVal, bool bTreatPlusAsNumber, bool bAllowComma)
{
	if(!pcsVal)
		return true;
	// An empty string is *not* a number
	if(pcsVal[0] == 0)
		return false;

	int iChar;
	for(iChar = 0; pcsVal[iChar]; iChar++)
	{
		if(!isdigit((unsigned char)pcsVal[iChar]) && pcsVal[iChar]!= '-' && pcsVal[iChar]!= '+'
			&& pcsVal[iChar]!= '.' && pcsVal[iChar]!= ' ' && pcsVal[iChar]!= ',')
			return false;

		if(!bTreatPlusAsNumber && pcsVal[iChar]== '+')
			return false;

		if(!bAllowComma && pcsVal[iChar]== ',')
			return false;
	}
	return true;
}
/*****************************************************************************/
const CHAR_TYPE* StringUtils::modifyStringPointerToTrim(CHAR_TYPE cChar, STRING_TYPE& strInOut)
{
	// Walk from the front and from behind. Needed for extreme speed.
	int iLen = strInOut.length();
	if(iLen == 0)
		return strInOut.c_str();

	int iPos = iLen - 1;
	while(iPos > 0 && strInOut[iPos] == cChar)
	{
		strInOut[iPos] = 0;
		iPos--;
	}

	// Now do the front
	iLen = strInOut.length();
	// Special case - the string is entirely spaces
	if(iLen == 0)
	{
		strInOut = EMPTY_STRING;
		return strInOut.c_str();
	}

	iPos = 0;
	while(iPos < iLen && strInOut[iPos] == cChar)
		iPos++;

	return strInOut.c_str() + iPos;
}
/*****************************************************************************/
const char* StringUtils::skipSymbols(const char* pcsString, const char* pcsSymbols)
{
	int iCurr;
	for(iCurr = 0; pcsString[iCurr]; iCurr++)
	{
		// Find the char in the symbols string
		if(!strchr(pcsSymbols, pcsString[iCurr]))
			break;
	}

	return pcsString + iCurr;
}
/********************************************************************************************/
const char* StringUtils::skipUntilSymbols(const char* pcsString, const char* pcsSymbols)
{
	int iCurr;
	for(iCurr = 0; pcsString[iCurr]; iCurr++)
	{
		// Find the char in the symbols string
		if(strchr(pcsSymbols, pcsString[iCurr]))
			break;
	}

	return pcsString + iCurr;
}
/********************************************************************************************/
FLOAT_TYPE StringUtils::convertStringToNumber(const string& strIn)
{
	bool bIsUpNumber = false;
	FLOAT_TYPE fEvalRes = FLOAT_TYPE_MAX;
	if(StringUtils::isANumber(strIn.c_str(), false) || (bIsUpNumber = StringUtils::isAnUPNumber(strIn)) )
	{
		if(bIsUpNumber)
			fEvalRes = convertUPNumberToScreenPixels(strIn.c_str());
		else
			fEvalRes = atof(strIn.c_str());
	}

	return fEvalRes;
}
/********************************************************************************************/
void StringUtils::numberToNiceString(FLOAT_TYPE fNum, string& strOut)
{
	StringUtils::numberToNiceString(fNum, 0, false, strOut);
}
/********************************************************************************************/
void StringUtils::numberToNiceString(FLOAT_TYPE fNum, int iDecimals, bool bDollar, string& strOut)
{
	char pcsBuff[64];
	if(iDecimals == 0)
	{
		int iTemp = fNum;
		sprintf(pcsBuff, "%d", iTemp);
	}
	else if(iDecimals < 0)
	{
		sprintf(pcsBuff, "%f", fNum);
	}
	else
	{
		char pcsFormatBuff[32];
		sprintf(pcsFormatBuff, "%%.%df", iDecimals);
		sprintf(pcsBuff, pcsFormatBuff, fNum);
	}

	strOut = pcsBuff;
	int iCurr, iNum = strOut.length();
	int iDotPos = strOut.rfind('.');
	if(iDotPos >= 0)
		iCurr = iDotPos;
	else
		iCurr = iNum;

	int iStopPos = 0;
	if(strOut.length() > 0 && (strOut[0] == '+' || strOut[0] == '-'))
		iStopPos = 1;


	for(iCurr -= 3; iCurr > iStopPos; iCurr -= 3)
	{
		strOut.insert(strOut.begin() + iCurr, ',');
	}

	if(bDollar)
		strOut = "$" + strOut;
}
/********************************************************************************************/
#define GET_TWO_PREC_DIGITS(x) (((long)((x)*100.0))/100.0)

const long long g_llOneKB = 1024;
const long long g_llOneMB = g_llOneKB*1024;
const long long g_llOneGB = g_llOneMB*1024;
const long long g_llOneTB = g_llOneGB*1024;

void StringUtils::numberToNiceSizeString(long long llSize, int iPrecDigits, string& strOut)
{
	double dTemp;

	char pcsTempBuff[256];
	char pcsFormatSpec[32] = "%.0f";

	if(iPrecDigits > 0)
		sprintf(pcsFormatSpec, "%%.%df", iPrecDigits);

	//	double rTempDb;
	// Max - Tb.
	dTemp = llSize / (double)g_llOneTB;
	if( (long)dTemp > 0)
	{
		sprintf(pcsTempBuff, pcsFormatSpec, dTemp);
		strOut = pcsTempBuff;
		strOut += "Tb";
		return;

		//		dTemp = GET_TWO_PREC_DIGITS(dTemp);
		//		rTempDb = dTemp;
		//		sprintf("%d")
		//		return rTempDb.ToString() + " Tb";
	}

	dTemp = llSize / (double)g_llOneGB;
	if( (long)dTemp > 0)
	{
		//		dTemp = GET_TWO_PREC_DIGITS(dTemp);
		//		rTempDb = dTemp;
		//		return rTempDb.ToString() + " Gb";
		sprintf(pcsTempBuff, pcsFormatSpec, dTemp);
		strOut = pcsTempBuff;
		strOut += "Gb";
		return;
	}

	dTemp = llSize / (double)g_llOneMB;
	if( (long)dTemp > 0)
	{
		//		dTemp = GET_TWO_PREC_DIGITS(dTemp);
		//		rTempDb = dTemp;
		//		return rTempDb.ToString() + " Mb";
		sprintf(pcsTempBuff, pcsFormatSpec, dTemp);
		strOut = pcsTempBuff;
		strOut += "Mb";
		return;

	}

	dTemp = llSize / (double)g_llOneKB;
	if( (long)dTemp > 0)
	{
		//		dTemp = GET_TWO_PREC_DIGITS(dTemp);
		//		rTempDb = dTemp;
		//		return rTempDb.ToString() + " Kb";
		sprintf(pcsTempBuff, pcsFormatSpec, dTemp);
		strOut = pcsTempBuff;
		strOut += "Kb";
		return;
	}

	sprintf(pcsTempBuff, "%lld", llSize);
	strOut = pcsTempBuff;
	strOut += " bytes";
	return;

	//	Int64 iTempInt = llSize;
	//	return iTempInt.ToString() + " bytes";

}
/********************************************************************************************/
void StringUtils::numberToString(FLOAT_TYPE fNum, const char* pcsFormat, string& strOut)
{
	char pcsBuff[256];
	sprintf(pcsBuff, pcsFormat, fNum);
	strOut = pcsBuff;
}
/********************************************************************************************/
void StringUtils::numberToString(FLOAT_TYPE fNum, string& strOut)
{
	char pcsBuff[256];
	sprintf(pcsBuff, "%g", fNum);
	strOut = pcsBuff;
}
/********************************************************************************************/
void StringUtils::longNumberToString(UNIQUEID_TYPE lNum, string& strOut)
{
	char pcsBuff[256];
	sprintf(pcsBuff, "%lld", lNum);
	strOut = pcsBuff;
}
/********************************************************************************************/
void StringUtils::numberToRomanString(int iNumber, string& strOut)
{
	_ASSERT(iNumber >= 0 && iNumber <= 10);

	// Simple
	switch (iNumber) {
	case 0: strOut = "0";return;
	case 1: strOut = "I";return;
	case 2: strOut = "II";return;
	case 3: strOut = "III";return;
	case 4: strOut = "IV";return;
	case 5: strOut = "V";return;
	case 6: strOut = "VI";return;
	case 7: strOut = "VII";return;
	case 8: strOut = "VIII";return;
	case 9: strOut = "IX";return;
	case 10: strOut = "X";return;
	default:
		break;
	}
}
/*****************************************************************************/
void numberToWords(int iNumber, string& strOut, bool bCapitalize)
{
	_ASSERT(iNumber >= 0 && iNumber <= 10);

	switch(iNumber)
	{
		case 0: strOut = "zero";break;
		case 1: strOut = "one";break;
		case 2: strOut = "two";break;
		case 3: strOut = "three";break;
		case 4: strOut = "four";break;
		case 5: strOut = "five";break;
		case 6: strOut = "six";break;
		case 7: strOut = "seven";break;
		case 8: strOut = "eight";break;
		case 9: strOut = "nine";break;
		case 10: strOut = "ten";break;
		default:break;
	}

	if(bCapitalize && strOut.length() > 0)
	{
		// Take that, STL, for not letting me change a single
		// character...
		char* pcsBuff = const_cast<char*>(strOut.c_str());
		pcsBuff[0] = toupper(pcsBuff[0]);
	}
}
/*****************************************************************************/
void StringUtils::ensureAlphaNumericAnd(const char* pcsExtraAllowedChars, char cReplacement, string& strInOut)
{
	char pcsSmallBuff[2] = { 0, 0 };
	int iCurr, iLen = strInOut.length();
	char* pcsBuff = const_cast<char*>(strInOut.c_str());
	for(iCurr = 0; iCurr < iLen; iCurr++)
	{
		if(isalnum(pcsBuff[iCurr]))
			continue;

		pcsSmallBuff[0] = pcsBuff[iCurr];
		if(strstr(pcsExtraAllowedChars, pcsSmallBuff))
			continue;

		pcsBuff[iCurr] = cReplacement;
	}
}
/*****************************************************************************/
char* StringUtils::wideToAscii(const wchar_t* strPtr)
{
	char *new_str;
	if(strPtr == NULL)
		return NULL;
	size_t len = wcslen(strPtr);
	new_str = new char[len+1];
	wcstombs(new_str, strPtr,len);
	new_str[len]='\0';
	return new_str;
}
/*****************************************************************************/
wchar_t* StringUtils::asciiToWide(const char* strPtr)
{
	wchar_t *new_str;
	if(strPtr==NULL)
		return NULL;
	size_t len = strlen(strPtr)+1;
	new_str = new wchar_t[len+1];
	mbstowcs(new_str, strPtr, len);
	new_str[len]='\0';
	return new_str;
}
/*****************************************************************************/
void StringUtils::freeAsciiText(char* strPtr)
{
	if(strPtr)
		delete[] strPtr;
}
/*****************************************************************************/
void StringUtils::freeWideText(wchar_t* strPtr)
{
	if(strPtr)
		delete[] strPtr;
}
/*****************************************************************************/
void StringUtils::ensureValidForAlphaNumMap(string& strInOut)
{
	char* pcsBuff = const_cast<char*>(strInOut.c_str());
	int iCurr, iNum = strInOut.length();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(pcsBuff[iCurr] < ATM_START_SYMBOL || pcsBuff[iCurr] > ATM_END_SYMBOL)
			pcsBuff[iCurr] = ATM_START_SYMBOL;
	}
}
/*****************************************************************************/
int StringUtils::findFrameNum(const char* pcsString, int& iAtlasFrameOut)
{
	char* pcsTemp = const_cast<char*>(pcsString);
	int iKilledPos = -1;

	iAtlasFrameOut = -1;
	int iLen = strlen(pcsTemp);
	if(iLen > 2 && pcsTemp[iLen - 2] == ':')
	{
		iAtlasFrameOut = atoi(&pcsTemp[iLen - 1]) - 1;
		iKilledPos = iLen - 2;
		pcsTemp[iKilledPos] = 0;
	}
	else if(iLen > 3 && pcsTemp[iLen - 3] == ':')
	{
		iAtlasFrameOut = atoi(&pcsTemp[iLen - 2]) - 1;
		iKilledPos = iLen - 3;
		pcsTemp[iKilledPos] = 0;
	}
	else if(iLen > 4 && pcsTemp[iLen - 4] == ':')
	{
		iAtlasFrameOut = atoi(&pcsTemp[iLen - 3]) - 1;
		iKilledPos = iLen - 4;
		pcsTemp[iKilledPos] = 0;
	}


	return iKilledPos;
}
/*****************************************************************************/
};
