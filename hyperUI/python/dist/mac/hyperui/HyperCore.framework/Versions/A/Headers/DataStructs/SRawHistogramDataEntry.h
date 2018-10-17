#pragma once

/*****************************************************************************/
struct SRawHistogramDataEntry
{
	SRawHistogramDataEntry() 
	{ 
		myValue = myCount = 0; 
	}
	int myValue, myCount;
};
/*****************************************************************************/
typedef vector < SRawHistogramDataEntry > TRawHistDataEntries;
typedef TIntVector THistDataEntries;
/*****************************************************************************/