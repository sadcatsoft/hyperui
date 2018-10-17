#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
HyperUI::ChannelType mapIndexToChannelType(int iNum)
{
	switch(iNum)
	{
	case 0:return ChannelRed;
	case 1:return ChannelGreen;
	case 2:return ChannelBlue;
	case 3:return ChannelIntensity;
	case 4:return ChannelAlpha;
	case 5:return ChannelCustom;
	}

	STATIC_ASSERT(ChannelLastPlaceholder == (0x0020 + 1));

	_ASSERT(0);
	return ChannelRed;
}
/*****************************************************************************/
int mapChannelTypeToIndex(HyperUI::ChannelType eType)
{
	switch(eType)
	{
	case ChannelRed:return 0;
	case ChannelGreen:return 1;
	case ChannelBlue:return 2;
	case ChannelIntensity:return 3;
	case ChannelAlpha:return 4;
	case ChannelCustom:return 5;
	}

	STATIC_ASSERT(ChannelLastPlaceholder == (0x0020 + 1));

	_ASSERT(0);
	return 0;
}
/*****************************************************************************/
void mapChannelTypeToIdString(HyperUI::ChannelType eType, const char* const *pcsStringsPtr, string& strOut)
{
	const char* const *pcsFinalStringsPtr = pcsStringsPtr ? pcsStringsPtr : g_pcsChannelIdStrings;

	strOut = "";
	if(eType == ChannelRed)
		strOut = pcsFinalStringsPtr[0];
	else if(eType == ChannelGreen)
		strOut = pcsFinalStringsPtr[1];
	else if(eType == ChannelBlue)
		strOut = pcsFinalStringsPtr[2];
	else if(eType == ChannelIntensity)
		strOut = pcsFinalStringsPtr[3];
	else if(eType == ChannelAlpha)
		strOut = pcsFinalStringsPtr[4];
	else if(eType == ChannelCustom)
		strOut = pcsFinalStringsPtr[5];
	ELSE_ASSERT;

	STATIC_ASSERT(ChannelLastPlaceholder == (0x0020 + 1));
}
/*****************************************************************************/
HyperUI::ChannelType mapStringToChannelType(const char* pcsType, const char* const *pcsStringsPtr)
{
	const char* const *pcsFinalStringsPtr = pcsStringsPtr ? pcsStringsPtr : g_pcsChannelStrings;

	if(IS_STRING_EQUAL(pcsType, pcsFinalStringsPtr[0]))
		return ChannelRed;
	else if(IS_STRING_EQUAL(pcsType, pcsFinalStringsPtr[1]))
		return ChannelGreen;
	else if(IS_STRING_EQUAL(pcsType, pcsFinalStringsPtr[2]))
		return ChannelBlue;
	else if(IS_STRING_EQUAL(pcsType, pcsFinalStringsPtr[3]))
		return ChannelIntensity;
	else if(IS_STRING_EQUAL(pcsType, pcsFinalStringsPtr[4]))
		return ChannelAlpha;
	else if(IS_STRING_EQUAL(pcsType, pcsFinalStringsPtr[5]))
		return ChannelCustom;
	// This is used to test whether a channel id is one of the standard
	// ones or not.
	else
		return ChannelCustom;

	STATIC_ASSERT(ChannelLastPlaceholder == (0x0020 + 1));

	return ChannelIntensity;
}
/*****************************************************************************/
};