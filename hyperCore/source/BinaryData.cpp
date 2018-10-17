#include "stdafx.h"

namespace HyperCore
{
/*****************************************************************************/
BinaryData::BinaryData() 
{
	myData = NULL;
	clear();
}
/*****************************************************************************/
BinaryData::~BinaryData() 
{
	clear();
}
/*****************************************************************************/
void BinaryData::setData(char* pcsData, int iLength, bool bDoOwn)
{
	clear();

	myAllowGrowingData = false;
	myDoOwnData = bDoOwn;
	myData = pcsData;
	myLength = iLength;
}
/*****************************************************************************/
void BinaryData::clear()
{
	if(myDoOwnData)
		delete[] myData;
	myData = NULL;
	myLength = 0;
	myReadPosition = 0;
	myWritePosition = 0;
	myDoReverseByteOrder = false;
	myDoOwnData = true;
}
/*****************************************************************************/
void BinaryData::ensureSizeFor(int iExtraSize)
{
	if(!myAllowGrowingData)
		ASSERT_RETURN;
	if(myWritePosition + iExtraSize <= myLength)
		return;

	// Otherwise, we need to reallocate
	int iNewSize = myLength*2;
	if(iNewSize - myWritePosition < iExtraSize)
		iNewSize = myWritePosition + iExtraSize;
	myData = (char*)realloc(myData, iNewSize);
	myLength = iNewSize;
}
/*****************************************************************************/
void BinaryData::ensureSizeMultipleOf(int iMultiple)
{
	_ASSERT(myDoOwnData);
	_ASSERT(myAllowGrowingData);

	while(getWritePosition() % iMultiple != 0)
		write<char>(0);
}
/*****************************************************************************/
}