#pragma once

#define RESOURCE_ITEM_NUM_BUCKETS		64

/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
class HYPERCORE_API TResourceItem : public IBaseObject
{
public:

	TResourceItem();
	virtual void onAllocated(IBaseObject* pData) { }
	TResourceItem(const char* pcsTag);
	TResourceItem(const char* pcsTag, const char* pcsId);
	virtual ~TResourceItem();

	void clearEverything();
	void clearModifiedProps();

	// Loads from a list of pairs of { key, value, key, value, ... }
	bool loadFromStringList(TStringVector& vecStrings);

	void saveToString(STRING_TYPE& strOut, FormatType eFormat = FormatNative, TStringMemoryCacher *pCacher = NULL, bool bSaveAsDefault = false, int iDbSaveLevel = 0) const;
	void saveToStringNative(STRING_TYPE& strOut, TStringMemoryCacher *pCacher = NULL, bool bSaveAsDefault = false, int iDbSaveLevel = 0) const;
	void saveToStringJSON(STRING_TYPE& strOut, TStringMemoryCacher *pCacher = NULL) const;
	void loadFromString(string& strContents);

	inline const char* getTag() const { return myTag.c_str(); }
	inline void setTag(const char* pcsTag) { myTag = pcsTag; }

	inline const char* getId() const {  return this->getStringProp(ResourceProperty::getIdPropertyValue((KEY_TYPE)0)); }

	unsigned int getStringPropLength(KEY_TYPE eProperty) const;
	const char* getStringProp(KEY_TYPE eProperty) const;
	inline bool getBoolProp(KEY_TYPE eProperty) const
	{
		const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
		if(pProp) return pProp->getBool(); else return false;
	}
	inline FLOAT_TYPE getNumProp(KEY_TYPE eProperty) const
	{
		const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
		if(pProp) return pProp->getNum(); else return 0;
	}
	inline double getDoubleProp(KEY_TYPE eProperty) const
	{
		const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
		if(pProp) return pProp->getDouble(); else return 0;
	}
	inline const IExpression* getExpressionProp(KEY_TYPE eProperty) const
	{
		const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
		if(pProp) return pProp->getExpression(); else return NULL;
	}

	FLOAT_TYPE* getArray2DProp(KEY_TYPE eProperty, int& iW, int& iH) const;
	int getEnumPropCount(KEY_TYPE eProperty) const;
	const char* getEnumPropValue(KEY_TYPE eProperty, int iEnumIndex) const;
	int getNumericEnumPropCount(KEY_TYPE eProperty) const;
	FLOAT_TYPE getNumericEnumPropValue(KEY_TYPE eProperty, int iEnumIndex) const;
	int findStringInEnumProp(KEY_TYPE eProperty, const char* pcsString) const;
	int countStringInEnumProp(KEY_TYPE eProperty, const char* pcsString);
	void removeEnumValue(KEY_TYPE eProperty, int iIndex);
	int findNumInEnumProp(KEY_TYPE eProperty, FLOAT_TYPE fNum);

	void setNumProp(KEY_TYPE eProperty, FLOAT_TYPE fValue);
	void setDoubleProp(KEY_TYPE eProperty, double dValue);
	void setStringProp(KEY_TYPE eProperty, const char* pcsValue);
	void setBoolProp(KEY_TYPE eProperty, bool bValue);
	void setArray2DProp(KEY_TYPE eProperty, FLOAT_TYPE* fArray, int iW, int iH);
	void addToNumericEnumProp(KEY_TYPE eProperty, FLOAT_TYPE fNum);
	void setNumericEnumPropValue(KEY_TYPE eProperty, int iIndex, FLOAT_TYPE fValue);
	void addToEnumProp(KEY_TYPE eProperty, const char* pcsValue);
	void addToEnumPropAsLong(KEY_TYPE eProperty, GTIME lValue);
	void setEnumProp(KEY_TYPE eProperty, int iIndex, const char* pcsValue);
	void setFromString(KEY_TYPE eProperty, const char* pcsValue);
	void setAsLong(KEY_TYPE eProperty, GTIME lValue);
	void setAsColor(KEY_TYPE eProperty, const SColor& scolValue, bool bAddAlpha = true);
	void setFromVector(KEY_TYPE eProperty, const vector < FLOAT_TYPE >& vecNumbers);
	void setFromVector(KEY_TYPE eProperty, const TStringVector& vecStrings);
	void setAsRect(KEY_TYPE eProperty, const SRect2D& srRect);

	// Force-converts whatever property into a string temporarily (on the fly).
	void getAsString(KEY_TYPE eProperty, string& strOut, const char* pcsOptFloatConvFormat = NULL) const;
	void getAsStringVector(KEY_TYPE eProperty, TStringVector& vecOut, const char* pcsOptFloatConvFormat = NULL) const;
	// Permanently converts this property to string
	void convertPropToString(KEY_TYPE eProperty);
	FLOAT_TYPE getAsNumber(KEY_TYPE eProperty) const;
	void getAsColor(KEY_TYPE eProperty, SColor& colOut) const;
	double getAsDouble(KEY_TYPE eProperty) const;
	GTIME getAsLong(KEY_TYPE eProperty) const;
	GTIME getEnumPropValueAsLong(KEY_TYPE eProperty, int iIndex) const;
	void getAsVector3(KEY_TYPE eProperty, SVector3D& svOut) const;
	void getAsNumVector(KEY_TYPE eProperty, vector < FLOAT_TYPE >& vecNumbers) const;
	void getAsRect(KEY_TYPE eProperty, SRect2D& srOut) const;

	TResourceProperty<KEY_TYPE, STORAGE_TYPE>& operator[](KEY_TYPE eProperty) 
	{
		TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
		if(pProp)
			return *pProp;
		else
			return myDummyProperty;
	}

	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>& operator[](KEY_TYPE eProperty) const
	{
		const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
		if(pProp)
			return *pProp;
		else
			return myDummyProperty;
	}


	bool getIsNone(KEY_TYPE eProperty) const;

	static int findInVectorByPropValue(const vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecIn, KEY_TYPE eProp, const char* pcsValue);

	static void filterVectorByPropValue(KEY_TYPE eProp, const char* pcsValue, vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecInOut, bool bInvert = false);
	static void filterVectorByPropValue(KEY_TYPE eProp, bool bValue, vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecInOut, bool bInvert = false);
	static void itemVectorToPropValueVector(const vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecIn, KEY_TYPE eProp, TStringVector& vecOut);
	void childrenToVector(vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >&  vecOut);

	TResourceItem* cloneSelf(bool bCopyChildren = true) const;
	void mergeResourcesFrom(const TResourceItem& rSrc, bool bCopyChildren, TResourceItemClassAllocator<KEY_TYPE, STORAGE_TYPE>* pAllocator = NULL, bool bPreserveExistingProps = false, bool bAllowChildLookupByOldId = false, bool bCallPostInit = true);
	void copyResourcesFrom(const TResourceItem& rSrc, bool bCopyChildren, TResourceItemClassAllocator<KEY_TYPE, STORAGE_TYPE>* pAllocator = NULL, bool bCallPostInit = true);
	void copyPropertyFrom(const TResourceItem& rSrc, KEY_TYPE eProperty);
	void copyPropertyFrom(const TResourceProperty<KEY_TYPE, STORAGE_TYPE>& rSrc);

	template < class MY_KEY_TYPE, class MY_STORAGE_TYPE, class OTHER_KEY_TYPE, class OTHER_STORAGE_TYPE >
	void copyPropertyFrom(const TResourceItem<OTHER_KEY_TYPE, OTHER_STORAGE_TYPE>& rSrc, OTHER_KEY_TYPE eSrcProperty, KEY_TYPE eDestProperty)
	{
		// Copy the property from another resource item of another type into this type...
		const TResourceProperty<OTHER_KEY_TYPE, OTHER_STORAGE_TYPE> *pOtherProp = rSrc.findProperty(eSrcProperty);
		if(!pOtherProp)
		{
			// No such prop!
			_ASSERT(0);
			return;
		}

		TResourceProperty<KEY_TYPE, STORAGE_TYPE> *pDestProp = this->findProperty(eDestProperty, true, true);
		
		// Copy the actual value now...
		pDestProp->template copyValueFrom<MY_KEY_TYPE, MY_STORAGE_TYPE, OTHER_KEY_TYPE, OTHER_STORAGE_TYPE>(*pOtherProp);
	}

	// Copy resources from a resource item of different type
	template < class MY_KEY_TYPE, class MY_STORAGE_TYPE, class OTHER_KEY_TYPE, class OTHER_STORAGE_TYPE >
	void copyResourcesFrom(const TResourceItem<OTHER_KEY_TYPE, OTHER_STORAGE_TYPE>& rSrc,  const PropertyMapper* pMapper)
	{
		clearEverything();
		mergeResourcesFrom<MY_KEY_TYPE, MY_STORAGE_TYPE, OTHER_KEY_TYPE, OTHER_STORAGE_TYPE>(rSrc, pMapper);
	}

	// Merge resources from a resource item of different template type
	template < class MY_KEY_TYPE, class MY_STORAGE_TYPE, class OTHER_KEY_TYPE, class OTHER_STORAGE_TYPE >
	void mergeResourcesFrom(const TResourceItem<OTHER_KEY_TYPE, OTHER_STORAGE_TYPE>& rSrc,  const PropertyMapper* pMapper)
	{
		// Go through all modified and unmodified props of our source, convert and copy them, then do the same
		// for the children of each.
		// Copy anything from its const item...
		if(rSrc.myConstRefItem)
			mergeResourcesFromRecursiveInternal<MY_KEY_TYPE, MY_STORAGE_TYPE, OTHER_KEY_TYPE, OTHER_STORAGE_TYPE>(*rSrc.myConstRefItem, pMapper);
		mergeResourcesFromRecursiveInternal<MY_KEY_TYPE, MY_STORAGE_TYPE, OTHER_KEY_TYPE, OTHER_STORAGE_TYPE>(rSrc, pMapper);
	}
    
	void removeProperty(KEY_TYPE eProperty);
	PropertyDataType getPropertyDataType(KEY_TYPE eProperty) const;
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* addChild(const char* pcsTag, TResourceItemClassAllocator<KEY_TYPE, STORAGE_TYPE>* pAllocator = NULL, TResourceItem<KEY_TYPE, STORAGE_TYPE>* pSourceItem = NULL);
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* addChildAndSetId(const char* pcsTag);
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* addChildAndSetId(const char* pcsTag, const char* pcsId);
	void parentExistingNode(TResourceItem* pItem, int iOptPosition = -1);

	int getChildIndex(TResourceItem* pTargetChild);

	int getNumChildren() const;
	TResourceItem* getChild(int iIndex);
	template < class TYPE > inline TYPE* getChild(int iIndex) { return dynamic_cast<TYPE*>(const_cast< TResourceItem<KEY_TYPE, STORAGE_TYPE>* >(this)->getChild(iIndex)); }
	template < class TYPE > inline const TYPE* getChild(int iIndex) const { return dynamic_cast<const TYPE*>(const_cast<const TResourceItem<KEY_TYPE, STORAGE_TYPE>* >(this)->getChild(iIndex)); }
	const TResourceItem* getChild(int iIndex) const;
	void deleteChild(int iIndex, bool bDeallocate = true);
	void deleteChild(TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild, bool bDeallocate = true);

	inline void setMemoryBlockId(long rMemId) { myMemId = rMemId; }
	inline long getMemoryBlockId() { return myMemId; }
	
	bool hasTag(const char* pcsTag) const;
	void addTag(const char* pcsTag);
	template < class TYPE > inline TYPE* getParent() { return dynamic_cast<TYPE*>(myParent); }
	inline TResourceItem* getParent() { return myParent; }
	inline const TResourceItem* getParent() const { return myParent; }
	template < class TYPE > inline const TYPE* getParent() const { return dynamic_cast<const TYPE*>(myParent); }

	void sortChildren(KEY_TYPE eProperty, bool bAscending);
	bool getIsLessThan(KEY_TYPE eProp, const TResourceItem<KEY_TYPE, STORAGE_TYPE>& rOther) const;
	bool getIsGreaterThan(KEY_TYPE eProp, const TResourceItem<KEY_TYPE, STORAGE_TYPE>& rOther) const;

	// Don't ever use this method. Except when you really need it.
	void setParent(TResourceItem* pParent);
	void setDataSourceType(SourceType eValue);
	inline SourceType getDataSourceType() const { return myDataSourceType; }

	TResourceItem* getChildById(const char* pcsId, bool bRecursive, bool bIncludeOldDefName = true) 
	{ 
		TResourceItem* pRes = getChildByPropertyValue(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIdPropertyValue((KEY_TYPE)0), pcsId, bRecursive); 
		if(!pRes && bIncludeOldDefName)
			pRes = getChildByPropertyValue(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getDefOldIdPropertyValue((KEY_TYPE)0), pcsId, bRecursive); 
		return pRes;
	}
	template < class TYPE > inline TYPE* getChildById(const char* pcsId, bool bRecursive, bool bIncludeOldDefName = true) { return dynamic_cast<TYPE*>(const_cast< TResourceItem<KEY_TYPE, STORAGE_TYPE>* >(this)->getChildById(pcsId, bRecursive, bIncludeOldDefName)); }
	const TResourceItem* getChildById(const char* pcsId, bool bRecursive, bool bIncludeOldDefName = true) const 
	{ 
		const TResourceItem* pRes = getChildByPropertyValue(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIdPropertyValue((KEY_TYPE)0), pcsId, bRecursive); 
		if(!pRes && bIncludeOldDefName)
			pRes = getChildByPropertyValue(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getDefOldIdPropertyValue((KEY_TYPE)0), pcsId, bRecursive); 
		return pRes;
	}

	TResourceItem* getChildByPropertyValue(KEY_TYPE eProp, const char* pcsId, bool bRecursive);
	const TResourceItem* getChildByPropertyValue(KEY_TYPE eProp, const char* pcsId, bool bRecursive) const;
	const TResourceItem* getChildByPropertyValue(KEY_TYPE eProp, const SColor& scolValue, bool bRecursive) const;
	TResourceItem* getChildByPropertyValue(KEY_TYPE eProp, FLOAT_TYPE fValue, bool bRecursive, FLOAT_TYPE fMaxTolerance = 0.0, bool bGreaterOnly = false);
	TResourceItem* getSelfOrParentByPropertyValue(KEY_TYPE eProp, const char* pcsValue);
	TResourceItem* getSelfOrParentByPropertyValue(KEY_TYPE eProp, bool bValue);
	TResourceItem* getChildByTag(const char* pcsTag, bool bRecursive);
	TResourceItem* getParentByTag(const char* pcsTag);

	void makeIdsUniqueRecursive(int& iGlobalCounter);
	void setId(const char* pcsId);

	inline bool isEmpty() const { return (myChildren == NULL) && (myConstRefItem == NULL || myConstRefItem->isEmpty()); }

	inline bool doesPropertyExist(KEY_TYPE eProperty) const { return this->findProperty(eProperty) != NULL; }

	// Compares the values of all properties and children, and returns true only if the values are exactly 
	// the same (and all props are the same).
	bool isEqualToContentOf(const TResourceItem<KEY_TYPE, STORAGE_TYPE>& rOther, KEY_TYPE eExcludedProperty ) const;

#if defined(_DEBUG) || defined(_DEBUG)
	const char * getDebugId() { return myDebugId.c_str(); }
#endif

	void removeCOWReferences();

	// Special simple non-create version in hopes of better optimization
	inline const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* findProperty(KEY_TYPE eProperty) const { return const_cast< TResourceItem<KEY_TYPE, STORAGE_TYPE>* >(this)->findPropertyInternal(eProperty); }
	inline TResourceProperty<KEY_TYPE, STORAGE_TYPE>* findProperty(KEY_TYPE eProperty) { return findPropertyInternal(eProperty); }

	void postInitRecursive();
	virtual void postInit() { }

	void toStringMap(TStringStringMap& rMapOut) const;

    // Stupid GCC wants this.
    template <class MY_KEY_TYPE, class MY_STORAGE_TYPE > friend class TResourceItem;

#if defined(_DEBUG) || defined(_DEBUG)
	int getHashSizeDb() { return myProps.getMemoryUsed(); }
#endif

	void flattenCOWReferences(bool bRecursive = true);

	// Iterators
	class PropertyIterator
	{
	public:
		PropertyIterator(const TResourceItem *pItem, bool bIncludeConstCOWProps) : myIterator(pItem->myProps), myConstRefItemIter() 
		{ 
			myItem = pItem; 
			if(pItem->myConstRefItem && bIncludeConstCOWProps)
				myConstRefItemIter.setParent(pItem->myConstRefItem->myProps); 
		}
		PropertyIterator()  {  }
		~PropertyIterator() {  }

		inline void toStart() { myIterator.toStart(); myConstRefItemIter.toStart(); }
		inline void toEnd() { myIterator.toEnd(); myConstRefItemIter.toEnd(); }
		inline bool isEnd() const { return myIterator.isEnd() && (myConstRefItemIter.isEnd() || myConstRefItemIter.isInitialized() == false); }

		inline bool operator==(const PropertyIterator& other) const { return myItem == other.myItem && myIterator == other.myIterator && myConstRefItemIter == other.myConstRefItemIter; }
		inline bool operator!=(const PropertyIterator& other) const { return ! (*this == other); }
		inline KEY_TYPE getProperty() const { return (myIterator.isEnd() == false) ? myIterator.getKey() : myConstRefItemIter.getKey(); }

		inline void operator++(int) { if(myIterator.isEnd() == false) myIterator++; else myConstRefItemIter++; }

	private:
		const TResourceItem *myItem;
		typename HashMap < KEY_TYPE, STORAGE_TYPE, TResourceProperty<KEY_TYPE, STORAGE_TYPE>, RESOURCE_ITEM_NUM_BUCKETS >::iterator myIterator;
		typename HashMap < KEY_TYPE, STORAGE_TYPE, TResourceProperty<KEY_TYPE, STORAGE_TYPE>, RESOURCE_ITEM_NUM_BUCKETS >::iterator myConstRefItemIter;
	};
	inline PropertyIterator propertiesFirst(bool bIncludeConstCOWProps = false) const { PropertyIterator rTempIter(this, bIncludeConstCOWProps); return rTempIter; }

#if defined(_DEBUG) || defined(_DEBUG)
	inline const char* getDebugName() const { return myDebugIdChar; }
#endif

protected:

	inline TResourceProperty<KEY_TYPE, STORAGE_TYPE>* findPropertyInternal(KEY_TYPE eProperty)
	{
		TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pResProp = myProps.find(eProperty);
#ifdef DO_PARANOID_HASH_CHECKING
		_ASSERT(pResProp == myProps.findDebug(eProperty));
#endif

#ifdef ENABLE_COW_FOR_PROPERTIES
		if(!pResProp && myConstRefItem)
		{
			// Look in our const item, if any
			pResProp = myConstRefItem->findProperty(eProperty);
		} // if not found
#endif
		return pResProp;
	}

	int getChildIndexById(const char* pcsId);
	int getChildIndexByOldId(const char* pcsId);

private:

	void initAsGameObject(TResourceItem* pNewChild, TResourceItem* pSrcChild);
	void postInitChildrenRecursive();

	inline TResourceProperty<KEY_TYPE, STORAGE_TYPE>* findProperty(KEY_TYPE eProperty, bool bCreate, bool bCopyFromParent = false)
	{
		TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pResProp = myProps.find(eProperty);
#ifdef DO_PARANOID_HASH_CHECKING
		_ASSERT(pResProp == myProps.findDebug(eProperty));
#endif

		if(!pResProp)
		{
			if(bCreate)
			{
				// Create one 
				TResourceProperty<KEY_TYPE, STORAGE_TYPE> rDummy;
				rDummy.setPropertyName(eProperty);
				pResProp = myProps.insert(eProperty, rDummy);
				_ASSERT(pResProp == myProps.find(eProperty));
#ifdef ENABLE_COW_FOR_PROPERTIES
				if(bCopyFromParent && myConstRefItem)
				{
					TResourceProperty<KEY_TYPE, STORAGE_TYPE> *pParentProp = myConstRefItem->findProperty(eProperty, false);
					if(pParentProp)
						*pResProp = *pParentProp;
				}
#endif
			}
#ifdef ENABLE_COW_FOR_PROPERTIES
			else if(myConstRefItem)
			{
				// Look in our const item, if any
				pResProp = myConstRefItem->findProperty(eProperty, false);
			}
#endif

		} // if not found
		return pResProp;
	}
	
	// Merges resources from a given item including its children, ignoring the const item. Used as a helper.
	template < class MY_KEY_TYPE, class MY_STORAGE_TYPE, class OTHER_KEY_TYPE, class OTHER_STORAGE_TYPE >
	void mergeResourcesFromRecursiveInternal(const TResourceItem<OTHER_KEY_TYPE, OTHER_STORAGE_TYPE>& rSrc,  const PropertyMapper* pMapper)
	{
		KEY_TYPE eOwnKey;
		TResourceProperty<MY_KEY_TYPE, MY_STORAGE_TYPE> *pDestProp;
		typename HashMap < OTHER_KEY_TYPE, OTHER_STORAGE_TYPE, TResourceProperty<OTHER_KEY_TYPE, OTHER_STORAGE_TYPE>, RESOURCE_ITEM_NUM_BUCKETS >::iterator mi;

		// Now copy anything from the self...
		for(mi = rSrc.myProps.begin(); mi != rSrc.myProps.end(); mi++)
		{
			eOwnKey = convertPropertyKeyToKeyType(pMapper, mi.getKey());
			pDestProp = this->findProperty(eOwnKey, true, true);
			pDestProp->template copyValueFrom<MY_KEY_TYPE, MY_STORAGE_TYPE, OTHER_KEY_TYPE, OTHER_STORAGE_TYPE>(mi.getValue());
		}

		// Now go through the children
		int iChildIndex;
		TResourceItem< MY_KEY_TYPE, MY_STORAGE_TYPE> *pChild;
		const TResourceItem<OTHER_KEY_TYPE, OTHER_STORAGE_TYPE> *pSrcChild;
		OTHER_KEY_TYPE eIdProp;
		const char* pcsId;
		int iCurrChild, iNumChildren = 0;
		if(rSrc.myChildren)
			iNumChildren = rSrc.myChildren->size();
		for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		{
			// See if a child with this name already exists
			pSrcChild = (*rSrc.myChildren)[iCurrChild];

			eIdProp = ResourceProperty::getIdPropertyValue((OTHER_KEY_TYPE)0);
			pcsId = pSrcChild->getStringProp(eIdProp);
			iChildIndex = this->getChildIndexById(pcsId);
			if(iChildIndex >= 0)
				pChild = (*myChildren)[iChildIndex];
			else
				pChild = this->addChild(pSrcChild->getTag(), NULL);

			pChild->template mergeResourcesFromRecursiveInternal<MY_KEY_TYPE, MY_STORAGE_TYPE, OTHER_KEY_TYPE, OTHER_STORAGE_TYPE>(*pSrcChild, pMapper);
		}
	}

	void deleteChildren();

private:

	string myTag;

	HashMap < KEY_TYPE, STORAGE_TYPE, TResourceProperty<KEY_TYPE, STORAGE_TYPE>, RESOURCE_ITEM_NUM_BUCKETS > myProps;

	vector < TResourceItem < KEY_TYPE, STORAGE_TYPE >* > *myChildren;

	long myMemId;
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* myParent;

	SourceType myDataSourceType;
	// An item from which to take our COW props
	mutable TResourceItem<KEY_TYPE, STORAGE_TYPE>* myConstRefItem;

	// This necessary for access where we need to return a reference
	// and don't really want to crash
	TResourceProperty<KEY_TYPE, STORAGE_TYPE> myDummyProperty;

#if defined(_DEBUG) || defined(_DEBUG)
	string myDebugId;
	const char* myDebugIdChar;
#endif
};
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
class ResItemChildSorter
{
public:
	ResItemChildSorter(KEY_TYPE eSortProp, bool bDoAscending) 
	{
		myDoAscending = bDoAscending;
		mySortProp = eSortProp;
	}

	bool operator()(const TResourceItem < KEY_TYPE, STORAGE_TYPE >* rItem1, const TResourceItem < KEY_TYPE, STORAGE_TYPE >* rItem2) const 
	{
		if(myDoAscending)
			return rItem1->getIsLessThan(mySortProp, *rItem2);
		else
			return rItem1->getIsGreaterThan(mySortProp, *rItem2);
	}
private:
	bool myDoAscending;
	STORAGE_TYPE mySortProp;
};
/*****************************************************************************/
typedef vector < TResourceItem<PropertyType, PropertyType>* > TResourceItemVector;
/*****************************************************************************/
typedef TResourceItem< PropertyType, PropertyType > ResourceItem;
typedef TResourceItem< const char*, RESOURCEITEM_STRING_TYPE > StringResourceItem;
typedef TResourceItemClassAllocator < PropertyType, PropertyType > ResourceItemClassAllocator;