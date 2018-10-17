#pragma once

/*****************************************************************************/
template < class TYPE >
TYPE mapStringToType(const char* pcsType, const char* const *pStringConstants, TYPE eDefaultValue = (TYPE)0)
{
	if(!pcsType)
		return eDefaultValue;

	int iCurr;
	for(iCurr = 0; pStringConstants[iCurr]; iCurr++)
	{
		if(strcmp(pcsType, pStringConstants[iCurr]) == 0)
			return (TYPE)iCurr;
	}

	return eDefaultValue;
}
/*****************************************************************************/
template < class TYPE >
TYPE mapStringToTypeByPrefix(const char* pcsType, const string *pStringConstants, TYPE eDefaultValue = (TYPE)0, bool bSkipWhitespace = false)
{
	if(!pcsType)
		return eDefaultValue;

	int iStrIndex;

	int iTypeStartOffset = 0;
	if(bSkipWhitespace)
	{
		for(iTypeStartOffset = 0; pcsType[iTypeStartOffset] != 0 && (pcsType[iTypeStartOffset] == ' ' || pcsType[iTypeStartOffset] == '\t' || pcsType[iTypeStartOffset] == '\n' || pcsType[iTypeStartOffset] == '\r'); iTypeStartOffset++)
			;  // Deliberate
	}	

	if(pcsType[iTypeStartOffset] == 0)
		return eDefaultValue;

	int iCurr;
	for(iCurr = 0; pStringConstants[iCurr].length() > 0; iCurr++)
	{
		const char* pcsBuff = pStringConstants[iCurr].c_str();
		int iTargetLen = pStringConstants[iCurr].length();
		for(iStrIndex = 0; iStrIndex < iTargetLen; iStrIndex++)
		{
			if(pcsType[iStrIndex + iTypeStartOffset] != pcsBuff[iStrIndex] || pcsType[iStrIndex + iTypeStartOffset] == NULL)
				break;
		}

		if(iStrIndex >= iTargetLen)
			return (TYPE)iCurr;
	}

	return eDefaultValue;
}
/*****************************************************************************/