#pragma once

HYPERUI_API HyperUI::ChannelType mapStringToChannelType(const char* pcsType, const char* const *pcsStringsPtr = NULL);
HYPERUI_API void mapChannelTypeToIdString(HyperUI::ChannelType eType, const char* const *pcsStringsPtr, string& strOut);
HYPERUI_API int mapChannelTypeToIndex(HyperUI::ChannelType eType);
HYPERUI_API HyperUI::ChannelType mapIndexToChannelType(int iNum);
