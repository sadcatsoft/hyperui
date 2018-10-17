#pragma once

/*****************************************************************************/
class Base64Coder
{
public:

	// Note that for BOTH of these, you MUST delete[] the memory returned!
	HYPERCORE_API static char* encode(const unsigned char* data, unsigned int dataSize, bool bUseStandardPaddingChar);
	HYPERCORE_API static char* encode(const BinaryData* pData, bool bUseStandardPaddingChar);
	HYPERCORE_API static unsigned char* decode(const char* encoded, unsigned int iSizeIn, unsigned int& dataSizeOut, bool bUseStandardPaddingChar);
};
/*****************************************************************************/