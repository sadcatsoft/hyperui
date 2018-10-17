#pragma once

/*
NOTE:
Loading limitations:
 - All items must have a type nameproperty.
 - Type name must always appear before any children. It doesn't have to be the first
 property, but it's better if it is.


*/

#define MAX_TRAVERSE_ITERATORS		3

class IProgressCallbacks;

#ifdef WIN32

	#ifdef DIRECTX_PIPELINE
		#define RESOURCE_PREFIX_PATH		""
		#define RESOURCE_PRO_PREFIX_PATH	""
		#define IMAGES_PREFIX_PATH			""
        #define FONTS_PREFIX_PATH			""

	#else
		#define RESOURCE_PREFIX_PATH		"config"
		#define RESOURCE_PRO_PREFIX_PATH	"configPro"
		#define IMAGES_PREFIX_PATH			"graphics"
        #define FONTS_PREFIX_PATH			"fonts"
		#define SHADERS_PATH				"shaders"
	#endif

#elif defined(LINUX)
		#define RESOURCE_PREFIX_PATH		"config"
		#define RESOURCE_PRO_PREFIX_PATH	"configPro"
		#define IMAGES_PREFIX_PATH			"graphics"
        #define FONTS_PREFIX_PATH			"fonts"
		#define SHADERS_PATH				"shaders"
#else
		#define RESOURCE_PREFIX_PATH		"config"
		#define RESOURCE_PRO_PREFIX_PATH	"configPro"
		#define IMAGES_PREFIX_PATH			"graphics"
        #define FONTS_PREFIX_PATH			"fonts"
		#define SHADERS_PATH				"shaders"
#endif

#define RESOURCE_MAC_SUFFIX			"_mac"
#define RESOURCE_PRO_SUFFIX			"_pro"

#define RESOURCE_FILE_EXT			".txt"

#define RES_COLL_DEFAULTS_MAP		std::map < string, TResourceItem<KEY_TYPE, STORAGE_TYPE>* >

template < class KEY_TYPE, class STORAGE_TYPE > class TResourceCollection;
typedef TResourceCollection < PropertyType, PropertyType > ResourceCollection;
/********************************************************************************************/
class IProgressCallbacks
{
public:
	virtual ~IProgressCallbacks() { }
	virtual void onItemCompleted(ResourceCollection* pCollection, int iIncBy = 1) = 0;
	virtual void setNewProgress(int iCount) = 0;
	virtual void setTotalNumItems(int iNum) = 0;
};
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
class TResourceCollection
{
public:
	HYPERCORE_API TResourceCollection();
	HYPERCORE_API TResourceCollection(string& strContents);
	HYPERCORE_API ~TResourceCollection();

	HYPERCORE_API bool loadFromFile(const char* pcsFilename, FileSourceType eFileSource, IProgressCallbacks* pStatsCallbacks = NULL, bool bPreserveOwnDefaults = false);
	HYPERCORE_API bool loadFromString(string& strContents);
	HYPERCORE_API void setDataSourceTypeForAllItems(SourceType eValue);

	template < class OBJECT_TYPE, class PLANE_TYPE >
	OBJECT_TYPE* createItemOfType(const char* pcsType, PLANE_TYPE* pParentPlane = NULL,	
								MemoryCacher< OBJECT_TYPE > *pMemCacher = NULL, TResourceItemClassAllocator < KEY_TYPE, STORAGE_TYPE >* pAllocator = NULL)
	{
		// Find it in our collection
		TResourceItem<KEY_TYPE, STORAGE_TYPE> *pSrcItem = myCacher.getCached(pcsType);
		if(!pSrcItem)
		{
            typename RES_COLL_DEFAULTS_MAP::iterator mi = myItems.find(pcsType);
			if(mi == myItems.end())
				return NULL;
			else
			{
				myCacher.addToCache(pcsType, mi->second);
				pSrcItem = mi->second;
			}
		}

		// Allocate a new object and copy properties from
		// the item found.
		OBJECT_TYPE* pRes = NULL;
		if(pMemCacher)
			pRes = pMemCacher->getNewObject(pParentPlane);
		else
		{
			if(pAllocator)
			{
				pRes = dynamic_cast<OBJECT_TYPE*>(pAllocator->allocateNewItem(pSrcItem, pParentPlane));
				_ASSERT(pRes);
			}
			else
				pRes = new OBJECT_TYPE(pParentPlane);
		}
		pRes->copyResourcesFrom(*pSrcItem, true, pAllocator, false);
		pRes->setTag(pSrcItem->getTag());
		pRes->postInitRecursive();
		return pRes;
	}

	TResourceItem<KEY_TYPE, STORAGE_TYPE>* operator[](const char* pcsId)
	{
		return this->getItemById(pcsId);
	}

	const TResourceItem<KEY_TYPE, STORAGE_TYPE>* operator[](const char* pcsId) const
	{
		return this->getItemById(pcsId);
	}

	TResourceItem<KEY_TYPE, STORAGE_TYPE>* operator[](int iIndex)
	{
		return this->getItem(iIndex);
	}

	const TResourceItem<KEY_TYPE, STORAGE_TYPE>* operator[](int iIndex) const
	{
		return this->getItem(iIndex);
	}

	inline TResourceItem<KEY_TYPE, STORAGE_TYPE>* getItemById(const char* pcsId)
	{
		return const_cast<TResourceItem<KEY_TYPE, STORAGE_TYPE>*>(const_cast<const TResourceCollection*>(this)->getItemById(pcsId));
	}

	const TResourceItem<KEY_TYPE, STORAGE_TYPE>* getItemById(const char* pcsId) const
	{
		TResourceItem<KEY_TYPE, STORAGE_TYPE> *pRes = myCacher.getCached(pcsId);
		if(pRes)
		{
#if defined(_DEBUG) && defined(PARANOID_CACHE_CHECKS)
			string strTemp;
			pRes->getAsString(ResourceProperty::getIdPropertyValue((KEY_TYPE)0), strTemp);
			if(strTemp != pcsId)
			{
				int bp = 0;
				myCacher.getCached(pcsId);
			}
#endif
			return pRes;
		}
		else
		{
			myCachedFindString = pcsId;
			typename RES_COLL_DEFAULTS_MAP::const_iterator mi = myItems.find(myCachedFindString);
			if(mi == myItems.end())
				return NULL;
			else
			{
#if defined(_DEBUG) && defined(PARANOID_CACHE_CHECKS)
				string strTemp;
				mi->second->getAsString(ResourceProperty::getIdPropertyValue((KEY_TYPE)0), strTemp);
				if(strTemp != pcsId)
				{
					const char* pcsTemp = strTemp.c_str();
					int bp = 0;
				}
#endif

				myCacher.addToCache(pcsId, mi->second);
				return mi->second;
			}
		}
	}

	HYPERCORE_API TResourceItem<KEY_TYPE, STORAGE_TYPE>* findItemWithPropValue(KEY_TYPE eProp, FLOAT_TYPE fValue);
	HYPERCORE_API TResourceItem<KEY_TYPE, STORAGE_TYPE>* findItemWithPropValue(KEY_TYPE eProp, const char* pcsValue, bool bRecursive);

	HYPERCORE_API void findAllItemsWithPropValue(KEY_TYPE eProp, const char* pcsValue, vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecOut, bool bClear = true);
	HYPERCORE_API void findAllItemsWithPropValue(KEY_TYPE eProp, bool bValue, vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecOut, bool bClear = true);
	HYPERCORE_API void itemsToVector(vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecOut);

	// Iterator
	class Iterator
	{
	public:
		Iterator(const TResourceCollection<KEY_TYPE, STORAGE_TYPE>& rCollection)
		{
			myCollection = &rCollection;
			myIterator = myCollection->myItems.begin();
		}
		Iterator() { myCollection = NULL; }

		inline void operator++(int) { myIterator++; }
		inline void operator--(int) { myIterator--; }
		inline void toEnd() { myIterator = myCollection->myItems.end(); }
		inline bool isEnd() const { return myIterator == myCollection->myItems.end(); }
		inline bool operator==(const Iterator& other) const { return myIterator == other.myIterator && myCollection == other.myCollection; }
		inline bool operator!=(const Iterator& other) const { return ! (*this == other); }
		inline TResourceItem<KEY_TYPE, STORAGE_TYPE>* getItem() { if(isEnd()) return NULL; else return myIterator->second; }
	private:
		const TResourceCollection<KEY_TYPE, STORAGE_TYPE> *myCollection;
		typename RES_COLL_DEFAULTS_MAP::const_iterator myIterator;
	};
	Iterator itemsBegin() const { Iterator rTempIter(*this); return rTempIter; }

	// Note: currently horribly inefficient.
	// But supposedly it's good enough for now. Bet I'll
	// see this comment again.
	HYPERCORE_API const TResourceItem<KEY_TYPE, STORAGE_TYPE>* getItem(int iIndex, const char* pcsOptIgnoreId = NULL) const;
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* getItem(int iIndex, const char* pcsOptIgnoreId = NULL) { return const_cast<TResourceItem<KEY_TYPE, STORAGE_TYPE>*>(const_cast<const TResourceCollection*>(this)->getItem(iIndex)); }

	HYPERCORE_API int getNumItems() const;

	HYPERCORE_API virtual void saveToString(string& strOut) const;

	HYPERCORE_API virtual void clear(bool bPreserveOwnDefaults = false);
	HYPERCORE_API void sortItems(KEY_TYPE eProperty, bool bAscending, vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecOut);
	HYPERCORE_API void deleteItem(const char* pcsId);
	HYPERCORE_API TResourceItem<KEY_TYPE, STORAGE_TYPE>* extractItem(int iIndex);

	HYPERCORE_API TResourceItem<KEY_TYPE, STORAGE_TYPE>* addItem(const char* pcsId, const char* pcsTag);

	HYPERCORE_API virtual void addExistingItem(TResourceItem<KEY_TYPE, STORAGE_TYPE>* pItem);

	HYPERCORE_API void mergeFrom(TResourceCollection<KEY_TYPE, STORAGE_TYPE>& rOther);
	HYPERCORE_API void copyFrom(TResourceCollection<KEY_TYPE, STORAGE_TYPE>* pOther);

	HYPERCORE_API void copyDefaultsFrom(TResourceCollection<KEY_TYPE, STORAGE_TYPE>* pOther);

#if defined(_DEBUG) && defined(PARANOID_CACHE_CHECKS)
	void checkCacheConsistency()
	{
		TResourceItem<KEY_TYPE, STORAGE_TYPE>* pTestItem;
		int iCurr;
		string strTest;
		const char* pcsKey;
		for(iCurr = 0; iCurr < NUM_STRING_CACHED_ITEMS; iCurr++)
		{
			pTestItem = myCacher.getCachedItemDebug(iCurr, true);
			if(pTestItem)
			{
				pcsKey = myCacher.getCachedKeyDebug(iCurr, true);
				pTestItem->getAsString(ResourceProperty::getIdPropertyValue((KEY_TYPE)0), strTest);
				if(strTest != pcsKey)
				{
					int bp = 0;
				}
			}
			pTestItem = myCacher.getCachedItemDebug(iCurr, false);
			if(pTestItem)
			{
				pcsKey = myCacher.getCachedKeyDebug(iCurr, false);
				pTestItem->getAsString(ResourceProperty::getIdPropertyValue((KEY_TYPE)0), strTest);
				if(strTest != pcsKey)
				{
					int bp = 0;
				}
			}
		}
	}
#endif

	static void resetUniqueIdCounterDebug() { theUniqueIdCounter = 1; }
	HYPERCORE_API TResourceItem<KEY_TYPE, STORAGE_TYPE>* getDefaultFor(const char* pcsTag);

	HYPERCORE_API static void runThroughPreprocessor(TStringVector& rLines, const CHAR_TYPE* pcsBaseFile, FileSourceType eFileSource, TStringSet* pOptExtraDefinesSet = NULL);

private:

	bool loadFromStringInternal(string& strFile, const char* pcsOptBaseFile, FileSourceType eFileSource, IProgressCallbacks* pStatsCallbacks, bool bEnablePreprocessor, bool bPreserveOwnDefaults);
	int readArray2D(TStringVector& rLines, int iStartLine, TResourceItem<KEY_TYPE, STORAGE_TYPE>* pNode, const char* pcsTag);
	void readArraySizes(TStringVector& rLines, int iStartLine, int& iWOut, int& iHOut);
	TResourceItem<KEY_TYPE, STORAGE_TYPE> * parserFindExistingNodeForOverride(TResourceItem<KEY_TYPE, STORAGE_TYPE>* pParent, const char* pcsIdToOverride);

	static int processIfdefBlock(TStringVector& rLines, int iStartLine, int iEndLine, bool bIsEnabled, bool bEraseLastLine);
	static bool evaluatePreprocessorCondition(string& strCondition, TStringSet* pOptExtraDefinesSet = NULL);

	void initCommon();

private:

	RES_COLL_DEFAULTS_MAP myItems;

	mutable string myCachedFindString;
	mutable StringMapCacher < TResourceItem<KEY_TYPE, STORAGE_TYPE> > myCacher;

	FLOAT_TYPE *myCached2DArray;
	int myCachedArraySize;
	TStringVector myReadArraySizeVec;
	TStringVector myReadArrayVec;

	// Used for default ids renaming
	HYPERCORE_API static int theUniqueIdCounter;

	// Stores defaults for a particualr element
	RES_COLL_DEFAULTS_MAP *myLoadedElemDefaults;
};
/*****************************************************************************/
typedef vector < TResourceCollection < PropertyType, PropertyType > * > TResourceCollections;
typedef map < string, TResourceCollection < PropertyType, PropertyType > * > TStringResourceCollectionsMap;
typedef map < string, TResourceCollection < const char*, RESOURCEITEM_STRING_TYPE > * > TStringStringResourceCollectionsMap;
/*****************************************************************************/
typedef TResourceCollection < PropertyType, PropertyType > ResourceCollection;
typedef TResourceCollection < const char*, RESOURCEITEM_STRING_TYPE > StringResourceCollection;
/*****************************************************************************/