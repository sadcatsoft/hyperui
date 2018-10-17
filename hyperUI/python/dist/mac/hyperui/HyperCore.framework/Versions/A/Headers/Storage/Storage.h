#pragma once

#include "HashMap.h"
#include "RingBuffer.h"
#include "LossyDynamicArray.h"
#include "LosslessDynamicArray.h"
#include "MemoryCacher.h"
#include "StringMapCacher.h"
#include "AlphaTreeMap.h"
#include "StlStringMapCacher.h"
#include "IdListContainer.h"
#include "IdMapContainer.h"

/*****************************************************************************/
template < class TYPE >
void containerToString(const TYPE& rContainer, string& strOut)
{
	typename TYPE::const_iterator li;
	int iIter;
	strOut = "";
	for(li = rContainer.begin(), iIter = 0; li != rContainer.end(); li++, iIter++)
	{
		if(iIter != 0)
			strOut += ", ";
		strOut += (*li);
	}
}

template < class TYPE >
void clearAndDeleteContainer(TYPE& rContainer)
{
	typename TYPE::iterator it;
	for(it = rContainer.begin(); it != rContainer.end(); it++)
		delete (*it);
	rContainer.clear();
}
/*****************************************************************************/
template < class KEY_TYPE, class VALUE_TYPE >
void clearAndDeleteContainer(map< KEY_TYPE, VALUE_TYPE >& rContainer)
{
	typename map< KEY_TYPE, VALUE_TYPE >::iterator it;
	for(it = rContainer.begin(); it != rContainer.end(); it++)
		delete it->second;
	rContainer.clear();
}
/*****************************************************************************/