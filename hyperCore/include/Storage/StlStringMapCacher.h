#pragma once

#define NUM_STL_STRING_CACHED_ITEMS		32 //25

template < class T >
class StlStringMapCacher
{
public:
	StlStringMapCacher()
	{
#ifdef KEEP_STRING_CACHE_STATS
		myNumHits = myNumRequests = 0;
#endif
		clearAll();
	}
	
	~StlStringMapCacher()
	{
		
	}
	
	T* getCached(const char* pcsKey)
	{
#ifdef KEEP_STRING_CACHE_STATS
		myNumRequests++;
#endif		
		
		int iLen = strlen(pcsKey);
		int iCachedLen = iLen % NUM_STL_STRING_CACHED_ITEMS;
		/// _ASSERT(iLen < NUM_STL_STRING_CACHED_ITEMS);
		if(myStrings[iCachedLen].length() == iLen && myStrings[iCachedLen] == pcsKey)
		{
#ifdef KEEP_STRING_CACHE_STATS
			myNumHits++;
#endif		
			return myValues[iCachedLen];
		}
		else if(myStrings2[iCachedLen].length()  && myStrings2[iCachedLen] == pcsKey)
		{
#ifdef KEEP_STRING_CACHE_STATS
			myNumHits++;
#endif		
			return myValues2[iCachedLen];
		}
		else
			return NULL;
	}
	
	void addToCache(const char* pcsKey, T* pValue)
	{
		int iLen = strlen(pcsKey);
		int iCachedLen = iLen % NUM_STL_STRING_CACHED_ITEMS;
		//_ASSERT(iLen < NUM_STL_STRING_CACHED_ITEMS);
		if(myStrings[iCachedLen] == "")
		{
			myStrings[iCachedLen] = pcsKey;
			myValues[iCachedLen] = pValue;
		}
		else if(myStrings2[iCachedLen] == "")
		{
			myStrings2[iCachedLen] = pcsKey;
			myValues2[iCachedLen] = pValue;			
		}
		
	}
	
	void clearAll(void)
	{
		
		memset(myValues, 0, sizeof(T*)*NUM_STL_STRING_CACHED_ITEMS);
		memset(myValues2, 0, sizeof(T*)*NUM_STL_STRING_CACHED_ITEMS);
//		memset(myStrings, 0, sizeof(char*)*NUM_STL_STRING_CACHED_ITEMS);
//		memset(myStrings2, 0, sizeof(char*)*NUM_STL_STRING_CACHED_ITEMS);

		int iCurr;
		for(iCurr = 0; iCurr < NUM_STL_STRING_CACHED_ITEMS; iCurr++)
		{
			myStrings[iCurr] = "";
			myStrings2[iCurr] = "";
		} 
	}
	
	void clear(const char* pcsKey)
	{
		int iLen = strlen(pcsKey);
		int iCachedLen = iLen % NUM_STL_STRING_CACHED_ITEMS;
		//_ASSERT(iLen < NUM_STL_STRING_CACHED_ITEMS);
		//if(iLen < NUM_STL_STRING_CACHED_ITEMS)
		{
			if(myStrings[iCachedLen].length() > 0 && myStrings[iCachedLen] == pcsKey)
			{
				myValues[iCachedLen] = NULL;
				myStrings[iCachedLen] = "";
				// myStrings[iLen] = NULL;
			}
			else if(myStrings2[iCachedLen].length() > 0 && myStrings2[iCachedLen] == pcsKey)
			{
				myValues2[iCachedLen] = NULL;
				myStrings2[iCachedLen] = "";
				// myStrings2[iLen] = NULL;
			}
		}
	}

#ifdef KEEP_STRING_CACHE_STATS
	static int getNumHits(void) { return myNumHits; }
	static int getNumRequests(void) { return myNumRequests; }
#endif
	
private:
	
#ifdef KEEP_STRING_CACHE_STATS
	static int myNumHits, myNumRequests;
#endif

	string myStrings[NUM_STL_STRING_CACHED_ITEMS];
	string myStrings2[NUM_STL_STRING_CACHED_ITEMS];
	T* myValues[NUM_STL_STRING_CACHED_ITEMS];
	T* myValues2[NUM_STL_STRING_CACHED_ITEMS];
};

#ifdef KEEP_STRING_CACHE_STATS

template < class T >
int StlStringMapCacher<T>::myNumHits = 0;

template < class T >
int StlStringMapCacher<T>::myNumRequests = 0;

#endif