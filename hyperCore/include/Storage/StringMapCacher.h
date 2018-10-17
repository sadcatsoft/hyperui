#pragma once

#define NUM_STRING_CACHED_ITEMS		40 //25

//#define PARANOID_CACHE_CHECKS
//#define STRING_CACHER_USE_ALPHA_TREE


#ifdef STRING_CACHER_USE_ALPHA_TREE

/**************************************/
// New cacher
/**************************************/


template < class T >
class StringMapCacher
{
public:
	StringMapCacher(MemoryCacher< TArrayHolder<AlphaTreeNode<T*>, NUM_SYMBOLS > > *pOptNodeMemoryCacher) 
		: myTree(pOptNodeMemoryCacher)
	{
		myInvalidVal = (T*)1;
		clearAll();
	}
	
	~StringMapCacher()
	{
		
	}
	
	T* getCached(const char* pcsKey)
	{
		T** pRes = myTree.findSimple(pcsKey);
		if(!pRes || *pRes == myInvalidVal)
			return NULL;
		else
			return *pRes;
	}
	
	void addToCache(const char* pcsKey, const T* pValue)
	{
		myTree.insert(pcsKey, pValue);
	}
	
	void clearAll(void)
	{
		myTree.clear();
	}
	
	void clear(const char* pcsKey)
	{
		myTree.insert(pcsKey, myInvalidVal);
	}
	
	
private:

	AlphaTreeMap<T*> myTree;
	T* myInvalidVal;
};


#else

/**************************************/
// Old cacher
/**************************************/

template < class T >
class StringMapCacher
{
public:
	StringMapCacher()
	{
#ifdef KEEP_STRING_CACHE_STATS
		myNumHits = myNumRequests = 0;
#endif
		clearAll();
	}
	
	~StringMapCacher()
	{
		
	}
	
	inline T* getCached(const char* pcsKey)
	{
#ifdef KEEP_STRING_CACHE_STATS
		myNumRequests++;
#endif		
		
		int iLen = strlen(pcsKey);
		_ASSERT(iLen < NUM_STRING_CACHED_ITEMS);
		//if(iLen < NUM_STRING_CACHED_ITEMS &&  myStrings[iLen][iLen - 1] == pcsKey[iLen - 1] && myStrings[iLen] == pcsKey)
		if(iLen < NUM_STRING_CACHED_ITEMS && myStrings[iLen] && myStrings[iLen][iLen - 1] == pcsKey[iLen - 1] && strcmp(myStrings[iLen], pcsKey) == 0)
		{
#ifdef KEEP_STRING_CACHE_STATS
			myNumHits++;
#endif		
			return myValues[iLen];
		}
		else if(iLen < NUM_STRING_CACHED_ITEMS && myStrings2[iLen] && myStrings2[iLen][iLen - 1] == pcsKey[iLen - 1] && strcmp(myStrings2[iLen], pcsKey) == 0)
		{
#ifdef KEEP_STRING_CACHE_STATS
			myNumHits++;
#endif		
			return myValues2[iLen];
		}
		else
			return NULL;
	}

	inline const T* getCached(const char* pcsKey) const
	{
#ifdef KEEP_STRING_CACHE_STATS
		myNumRequests++;
#endif		

		int iLen = strlen(pcsKey);
		_ASSERT(iLen < NUM_STRING_CACHED_ITEMS);
		if(iLen < NUM_STRING_CACHED_ITEMS && myStrings[iLen] && myStrings[iLen][iLen - 1] == pcsKey[iLen - 1] && strcmp(myStrings[iLen], pcsKey) == 0)
		{
#ifdef KEEP_STRING_CACHE_STATS
			myNumHits++;
#endif		
			return myValues[iLen];
		}
		else if(iLen < NUM_STRING_CACHED_ITEMS && myStrings2[iLen] && myStrings2[iLen][iLen - 1] == pcsKey[iLen - 1] && strcmp(myStrings2[iLen], pcsKey) == 0)
		{
#ifdef KEEP_STRING_CACHE_STATS
			myNumHits++;
#endif		
			return myValues2[iLen];
		}
		else
			return NULL;
	}
	
	void addToCache(const char* pcsKey, T* pValue)
	{
		int iLen = strlen(pcsKey);
		_ASSERT(iLen < NUM_STRING_CACHED_ITEMS);
//		if(iLen < NUM_STRING_CACHED_ITEMS && myStrings[iLen] == "")
		if(iLen < NUM_STRING_CACHED_ITEMS && myStrings[iLen] == NULL)
		{
			// myStrings[iLen] = pcsKey;
			myStrings[iLen] = pValue->getId();
			myValues[iLen] = pValue;
		}
		else if(iLen < NUM_STRING_CACHED_ITEMS && myStrings2[iLen] == NULL)
		{
			myStrings2[iLen] = pValue->getId();
			myValues2[iLen] = pValue;			
		}
		
	}
	
	void clearAll(void)
	{
		
		memset(myValues, 0, sizeof(T*)*NUM_STRING_CACHED_ITEMS);
		memset(myValues2, 0, sizeof(T*)*NUM_STRING_CACHED_ITEMS);
		memset(myStrings, 0, sizeof(char*)*NUM_STRING_CACHED_ITEMS);
		memset(myStrings2, 0, sizeof(char*)*NUM_STRING_CACHED_ITEMS);
		/*
		int iCurr;
		for(iCurr = 0; iCurr < NUM_STRING_CACHED_ITEMS; iCurr++)
		{
			myValues[iCurr] = NULL;
//			myStrings[iCurr] = "";
			myStrings[iCurr] = NULL;
			
			myValues2[iCurr] = NULL;
			//			myStrings2[iCurr] = "";
			myStrings2[iCurr] = NULL;
			
		} */
	}
	
	void clear(const char* pcsKey)
	{
		int iLen = strlen(pcsKey);
		_ASSERT(iLen < NUM_STRING_CACHED_ITEMS);
		if(iLen < NUM_STRING_CACHED_ITEMS)
		{
			if(myStrings[iLen] && strcmp(pcsKey, myStrings[iLen]) == 0)
			{
				myValues[iLen] = NULL;
				// myStrings[iLen] = "";
				myStrings[iLen] = NULL;
			}
			else if(myStrings2[iLen] && strcmp(pcsKey, myStrings2[iLen]) == 0)
			{
				myValues2[iLen] = NULL;
				// myStrings2[iLen] = "";
				myStrings2[iLen] = NULL;
			}
		}
	}
	
#if defined(_DEBUG) && defined(PARANOID_CACHE_CHECKS)
	T* getCachedItemDebug(int iIndex, bool bFromFirst)
	{
		if(bFromFirst)
			return myValues[iIndex];
		else
			return myValues2[iIndex];
	}
	
	const char* getCachedKeyDebug(int iIndex, bool bFromFirst)
	{
		if(bFromFirst)
			return myStrings[iIndex];
		else
			return myStrings2[iIndex];
	}
#endif

#ifdef KEEP_STRING_CACHE_STATS
	static int getNumHits(void) { return myNumHits; }
	static int getNumRequests(void) { return myNumRequests; }
#endif
	
private:
	
#ifdef KEEP_STRING_CACHE_STATS
	static int myNumHits, myNumRequests;
#endif

	const char* myStrings[NUM_STRING_CACHED_ITEMS];
	const char* myStrings2[NUM_STRING_CACHED_ITEMS];
	T* myValues[NUM_STRING_CACHED_ITEMS];
	T* myValues2[NUM_STRING_CACHED_ITEMS];
};

#endif

#ifdef KEEP_STRING_CACHE_STATS

template < class T >
int StringMapCacher<T>::myNumHits = 0;

template < class T >
int StringMapCacher<T>::myNumRequests = 0;

#endif