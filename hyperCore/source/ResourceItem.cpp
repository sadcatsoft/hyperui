#include "stdafx.h"

#ifdef _DEBUG
//#define RES_COLL_NAMEGEN_DEBUG
//#define DO_PARANOID_RI_CHECKING
#endif

namespace HyperCore
{
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
TResourceItem<KEY_TYPE, STORAGE_TYPE>::TResourceItem()
{
	myParent = NULL;
	myChildren = NULL;
	myMemId = -1;
	myDataSourceType = SourceNone;
	myConstRefItem = NULL;

#if defined(_DEBUG) || defined(_DEBUG)
	myDebugIdChar = EMPTY_STRING;
#endif
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
TResourceItem<KEY_TYPE, STORAGE_TYPE>::TResourceItem(const char* pcsTag)
{
	myParent = NULL;
	myTag = pcsTag;
	myChildren = NULL;
	myMemId = -1;
	myDataSourceType = SourceNone;
	myConstRefItem = NULL;

#if defined(_DEBUG) || defined(_DEBUG)
	myDebugIdChar = EMPTY_STRING;
#endif
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
TResourceItem<KEY_TYPE, STORAGE_TYPE>::TResourceItem(const char* pcsTag, const char* pcsId)
{
	myParent = NULL;
	myTag = pcsTag;
	myChildren = NULL;
	myMemId = -1;
	myDataSourceType = SourceNone;
	myConstRefItem = NULL;
#if defined(_DEBUG) || defined(_DEBUG)
	myDebugIdChar = EMPTY_STRING;
#endif

	setStringProp(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIdPropertyValue((KEY_TYPE)0), pcsId != NULL ? pcsId : pcsTag);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
TResourceItem<KEY_TYPE, STORAGE_TYPE>::~TResourceItem()
{
	myParent = NULL;
	deleteChildren();
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::clearEverything()
{
	clearModifiedProps();
	deleteChildren();

	myConstRefItem = NULL;
	myDataSourceType = SourceNone;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::clearModifiedProps()
{
	myProps.clear();

#ifdef ENABLE_COW_FOR_PROPERTIES
	// No - we only clear the modified ones.
	// myConstRefItem = NULL;
#endif
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
GTIME TResourceItem<KEY_TYPE, STORAGE_TYPE>::getEnumPropValueAsLong(KEY_TYPE eProperty, int iIndex) const
{
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(pProp)
		return pProp->getEnumPropValueAsLong(iIndex);
	else
	{
		_ASSERT(0);
		return 0;
	}

}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::getAsVector3(KEY_TYPE eProperty, SVector3D& svOut) const
{
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(pProp)
		return pProp->getAsVector3(svOut);
	else
	{
		_ASSERT(0);
		svOut.set(0,0,0);
		return;
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::getAsNumVector(KEY_TYPE eProperty, vector < FLOAT_TYPE >& vecNumbers) const
{
	vecNumbers.clear();
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(pProp)
		return pProp->getAsNumVector(vecNumbers);
	else
	{
		_ASSERT(0);
		return;
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::getAsRect(KEY_TYPE eProperty, SRect2D& srOut) const
{
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(pProp)
		return pProp->getAsRect(srOut);
	else
	{
		srOut.reset();
		_ASSERT(0);
		return;
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
GTIME TResourceItem<KEY_TYPE, STORAGE_TYPE>::getAsLong(KEY_TYPE eProperty) const
{
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(pProp)
		return pProp->getAsLong();
	else
	{
		_ASSERT(0);
		return 0;
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
double TResourceItem<KEY_TYPE, STORAGE_TYPE>::getAsDouble(KEY_TYPE eProperty) const
{
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(pProp)
		return pProp->getAsDouble();
	else
	{
		//_ASSERT(0);
		return 0;
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::getAsColor(KEY_TYPE eProperty, SColor& colOut) const
{
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(pProp)
		pProp->getAsColor(colOut);
	else
	{
		_ASSERT(0);
		colOut.set(0,0,0,1);
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
FLOAT_TYPE TResourceItem<KEY_TYPE, STORAGE_TYPE>::getAsNumber(KEY_TYPE eProperty) const
{
    const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
    if(pProp)
		return pProp->getAsNumber();
    else
    {
		_ASSERT(0);
		return 0;
    }
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::getAsStringVector(KEY_TYPE eProperty, TStringVector& vecOut, const char* pcsOptFloatConvFormat) const
{
    const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
    if(pProp)
		return pProp->getAsStringVector(vecOut, pcsOptFloatConvFormat);
    else
    {
		_ASSERT(0);
		return;
    }
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::convertPropToString(KEY_TYPE eProperty)
{
	string strTemp;
	this->getAsString(eProperty, strTemp);
	this->setStringProp(eProperty, strTemp.c_str());
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::getAsString(KEY_TYPE eProperty, string& strOut, const char* pcsOptFloatConvFormat) const
{
    const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
    if(pProp)
		return pProp->getAsString(strOut, pcsOptFloatConvFormat);
    else
    {
		_ASSERT(0);
		strOut = "";
		return;
    }
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
const char* TResourceItem<KEY_TYPE, STORAGE_TYPE>::getStringProp(KEY_TYPE eProperty) const
{
    const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
    if(pProp)
		return pProp->getString();
    else
    {
		//_ASSERT(0);
		return "";
    }
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
unsigned int TResourceItem<KEY_TYPE, STORAGE_TYPE>::getStringPropLength(KEY_TYPE eProperty) const
{
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(pProp)
		return pProp->getStringLength();
	else
	{
		_ASSERT(0);
		return 0;
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
int TResourceItem<KEY_TYPE, STORAGE_TYPE>::getEnumPropCount(KEY_TYPE eProperty) const
{
    const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
    if(pProp)
		return pProp->getNumEnumValues();
    else
    {
		return 0;
    }
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
const char* TResourceItem<KEY_TYPE, STORAGE_TYPE>::getEnumPropValue(KEY_TYPE eProperty, int iEnumIndex) const
{
    const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
    if(pProp)
		return pProp->getEnumValue(iEnumIndex);
    else
    {
		_ASSERT(0);
		return "";
    }
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
int TResourceItem<KEY_TYPE, STORAGE_TYPE>::getNumericEnumPropCount(KEY_TYPE eProperty) const
{
    const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
    if(pProp)
		return pProp->getNumNumericEnumValues();
    else
		return 0;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
PropertyDataType TResourceItem<KEY_TYPE, STORAGE_TYPE>::getPropertyDataType(KEY_TYPE eProperty) const
{
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(!pProp)
	{
		_ASSERT(0);
		return PropertyDataNumber;
	}
	else
		return pProp->getPropertyDataType();
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
int TResourceItem<KEY_TYPE, STORAGE_TYPE>::findNumInEnumProp(KEY_TYPE eProperty, FLOAT_TYPE fNum)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(pProp)
		return pProp->findNumInEnumProp(fNum);
	else
	{
		// No such prop. This isn't an error, just an empty
		// prop.
		return -1;
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
int TResourceItem<KEY_TYPE, STORAGE_TYPE>::findStringInEnumProp(KEY_TYPE eProperty, const char* pcsString) const
{
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(pProp)
		return pProp->findStringInEnumProp(pcsString);
	else
	{
		// No such prop. This isn't an error, just an empty
		// prop.
		return -1;
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::removeEnumValue(KEY_TYPE eProperty, int iIndex)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(pProp)
		return pProp->removeEnumValue(iIndex);
	else
	{
		_ASSERT(0);
		return;
	}

}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
int TResourceItem<KEY_TYPE, STORAGE_TYPE>::countStringInEnumProp(KEY_TYPE eProperty, const char* pcsString)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(pProp)
		return pProp->countStringInEnumProp(pcsString);
	else
	{
		// No such prop. This isn't an error, just an empty
		// prop.
		return -1;
	}
	
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
FLOAT_TYPE TResourceItem<KEY_TYPE, STORAGE_TYPE>::getNumericEnumPropValue(KEY_TYPE eProperty, int iEnumIndex) const
{
    const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
    if(pProp)
		return pProp->getNumericEnumValue(iEnumIndex);
    else
    {
		_ASSERT(0);
		return 0;
    }
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
FLOAT_TYPE* TResourceItem<KEY_TYPE, STORAGE_TYPE>::getArray2DProp(KEY_TYPE eProperty, int& iW, int& iH) const
{
    const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
    if(pProp)
		return pProp->getFloatArray(iW, iH);
    else
    {
		_ASSERT(0);
		iW = iH = 0;
		return NULL;
    }
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::deleteChild(TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild, bool bDeallocate)
{
	if(!myChildren || !pChild)
		return;

	int iChild, iNumChildren = myChildren->size();
	for(iChild = 0; iChild < iNumChildren; iChild++)
	{
		if((*myChildren)[iChild] == pChild)
		{
			deleteChild(iChild, bDeallocate);
			break;
		}
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::deleteChild(int iIndex, bool bDeallocate)
{
	if(myChildren && iIndex < (int)myChildren->size())
	{
		if(bDeallocate)
			delete (*myChildren)[iIndex];
		myChildren->erase(myChildren->begin() + iIndex);
		if(myChildren->size() == 0)
		{
			delete myChildren;
			myChildren = NULL;
		}
		
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::deleteChildren()
{
	if(myChildren)
	{
		int iChild, iNumChildren = myChildren->size();
		for(iChild = 0; iChild < iNumChildren; iChild++)
			delete (*myChildren)[iChild];
		delete myChildren;
		myChildren = NULL;
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::addChildAndSetId(const char* pcsTag)
{
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pRes = addChild(pcsTag);
	pRes->setStringProp(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIdPropertyValue((KEY_TYPE)0), pcsTag);
	return pRes;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::addChildAndSetId(const char* pcsTag, const char* pcsId)
{
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pRes = addChild(pcsTag);
	pRes->setStringProp(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIdPropertyValue((KEY_TYPE)0), pcsId);
	return pRes;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::addChild(const char* pcsTag, TResourceItemClassAllocator<KEY_TYPE, STORAGE_TYPE>* pAllocator, TResourceItem<KEY_TYPE, STORAGE_TYPE>* pSourceItem)
{
	if(!myChildren)
		myChildren = new vector < TResourceItem< KEY_TYPE, STORAGE_TYPE >* >;
	
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pNewItem;
	if(pAllocator)
		pNewItem = pAllocator->allocateNewItem(pSourceItem, this, pcsTag);
	else
		pNewItem = new TResourceItem<KEY_TYPE, STORAGE_TYPE>(pcsTag);

	pNewItem->myParent = this;
	myChildren->push_back(pNewItem);
	return pNewItem;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::parentExistingNode(TResourceItem<KEY_TYPE, STORAGE_TYPE>* pItem, int iOptPosition)
{
#if defined(_DEBUG) || defined(_DEBUG)
	const char* pcsDbId = pItem->getStringProp( TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIdPropertyValue((KEY_TYPE)0) );
	// Note that if you get a crash after this assert, try commenting it out - if we break here, the app continues
	// to run, deleting whatever the scheduled elements were there for deletion, but the code that asserted may
	// still be assuming they're not gone.
	_ASSERT(strlen(pItem->getDebugId()) > 0);
#endif
	if(!myChildren)
		myChildren = new vector < TResourceItem< KEY_TYPE, STORAGE_TYPE >* >;
	pItem->myParent = this;
	if(iOptPosition >= 0 && iOptPosition < myChildren->size())
		myChildren->insert(myChildren->begin() + iOptPosition, pItem);
	else
		myChildren->push_back(pItem);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::mergeResourcesFrom(const TResourceItem<KEY_TYPE, STORAGE_TYPE>& rSrc, bool bCopyChildren, 
														TResourceItemClassAllocator<KEY_TYPE, STORAGE_TYPE>* pAllocator, bool bPreserveExistingProps, 
														bool bAllowChildLookupByOldId, bool bCallPostInit)
{
	typename HashMap < KEY_TYPE, STORAGE_TYPE, TResourceProperty<KEY_TYPE, STORAGE_TYPE>, RESOURCE_ITEM_NUM_BUCKETS >::iterator mi;

#ifdef ENABLE_COW_FOR_PROPERTIES
	// Now, if the other item has its constant ref item set, we need to copy those props here, too,
	// unless the ref items are exactly the same.
	KEY_TYPE eCurrProp;
	if(rSrc.myConstRefItem && rSrc.myConstRefItem != myConstRefItem)
	{
		for(mi = rSrc.myConstRefItem->myProps.begin(); mi != rSrc.myConstRefItem->myProps.end(); mi++)
		{
			eCurrProp = mi.getValue().getPropertyName();
			if(bPreserveExistingProps && myProps.find(eCurrProp))
				continue;

			myProps.insert(eCurrProp, mi.getValue());
#ifdef DO_PARANOID_RI_CHECKING
			_ASSERT(myProps.find(eCurrProp));
#endif
		}
	}
#endif
	// Now do the modified ones
	for(mi = rSrc.myProps.begin(); mi != rSrc.myProps.end(); mi++)
	{
		eCurrProp = mi.getValue().getPropertyName();
		if(bPreserveExistingProps && myProps.find(eCurrProp))
			continue;
		myProps.insert(eCurrProp, mi.getValue());
#ifdef DO_PARANOID_RI_CHECKING
		_ASSERT(myProps.find(eCurrProp));
#endif
	}

	if(bCopyChildren)
	{
		// Copy all the children, we have any
		if(rSrc.myChildren)
		{
			// No! Merge the children.
			bool bDidCreateChild;
			TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild, *pSrcChild;
			const char* pcsChildType;
			const char* pcsOldChildType;
			int iChildIndex;
			int iCurrChild, iNumChildren = rSrc.myChildren->size();
			for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
			{
				// See if a child with this name already exists
				pSrcChild = (*rSrc.myChildren)[iCurrChild];

				pcsChildType = pSrcChild->getStringProp(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIdPropertyValue((KEY_TYPE)0));
				pChild = NULL;
				iChildIndex = this->getChildIndexById(pcsChildType);
				// Lookup by old id if we haven't found anything
				if(iChildIndex < 0 && bAllowChildLookupByOldId)
				{
					pcsOldChildType = pSrcChild->getStringProp(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getDefOldIdPropertyValue((KEY_TYPE)0));
					iChildIndex = this->getChildIndexByOldId(pcsOldChildType);
					// It may be the case that the old name doesn't exist on the source item, such as when reloading.
					if(iChildIndex < 0)
						iChildIndex = this->getChildIndexByOldId(pcsChildType);
				}
				if(iChildIndex >= 0)
					pChild = (*myChildren)[iChildIndex];
				bDidCreateChild = false;
				if(!pChild)
				{
					bDidCreateChild = true;
					pChild = this->addChild(pSrcChild->getTag(), pAllocator, pSrcChild);
				}

				// Merge props from
				pChild->mergeResourcesFrom(*pSrcChild, true, pAllocator, bPreserveExistingProps, bAllowChildLookupByOldId, false);
				if(bDidCreateChild)
					initAsGameObject(pChild, this);
			}

			if(bCallPostInit)
				this->postInitChildrenRecursive();
		}
	}

#if defined(_DEBUG) || defined(_DEBUG)
	KEY_TYPE eIdProp = TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIdPropertyValue((KEY_TYPE)0);
	if(doesPropertyExist(eIdProp))
	{
		getAsString(eIdProp, myDebugId );
		myDebugIdChar = myDebugId.c_str();
	}
#endif

}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::copyPropertyFrom(const TResourceProperty<KEY_TYPE, STORAGE_TYPE>& rSrc)
{
	_ASSERT(!ResourceProperty::isNullProperty(rSrc.getPropertyName()));
	myProps.insert(rSrc.getPropertyName(), rSrc);
#ifdef DO_PARANOID_RI_CHECKING
	_ASSERT(myProps.find(rSrc.getPropertyName()));
#endif

#if defined(_DEBUG) || defined(_DEBUG)
	if(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::isIdProperty(rSrc.getPropertyName()))
	{
		rSrc.getAsString(myDebugId);
		myDebugIdChar = myDebugId.c_str();
	}
#endif
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::copyPropertyFrom(const TResourceItem<KEY_TYPE, STORAGE_TYPE>& rSrc, KEY_TYPE eProperty)
{
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE> *pSrcProp = const_cast<TResourceItem< KEY_TYPE, STORAGE_TYPE>& >(rSrc).findProperty(eProperty, false, false);
	if(pSrcProp)
	{
		myProps.insert(eProperty, *pSrcProp);
#ifdef DO_PARANOID_RI_CHECKING
		_ASSERT(myProps.find(eProperty));
#endif
	}
	ELSE_ASSERT;

#if defined(_DEBUG) || defined(_DEBUG)
	if(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::isIdProperty(eProperty))
	{
		rSrc.getAsString(eProperty, myDebugId);
		myDebugIdChar = myDebugId.c_str();

	}
#endif
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
int TResourceItem<KEY_TYPE, STORAGE_TYPE>::getChildIndexById(const char* pcsId)
{
	if(!myChildren)
		return -1;

	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild;
	int iCurr, iNum = myChildren->size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = (*myChildren)[iCurr];
		if(strcmp(pChild->getStringProp(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIdPropertyValue((KEY_TYPE)0)), pcsId) == 0)
			return iCurr;
	}

	return -1;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
int TResourceItem<KEY_TYPE, STORAGE_TYPE>::getChildIndexByOldId(const char* pcsId)
{
	if(!myChildren)
		return -1;

	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild;
	int iCurr, iNum = myChildren->size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = (*myChildren)[iCurr];
#ifdef _DEBUG
		const char* pcsOldNameDB = pChild->getStringProp(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getDefOldIdPropertyValue((KEY_TYPE)0));
		int bp = 0;
#endif

		if(strcmp(pChild->getStringProp(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getDefOldIdPropertyValue((KEY_TYPE)0)), pcsId) == 0)
			return iCurr;
	}

	return -1;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
const TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::getChildByPropertyValue(KEY_TYPE eProp, const SColor& scolValue, bool bRecursive) const
{
	if(!myChildren)
		return NULL;

	SColor scolTemp;
	const TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild, *pCurrRes;
	int iCurr, iNum = myChildren->size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = (*myChildren)[iCurr];		
		if(pChild->doesPropertyExist(eProp))
		{
			pChild->getAsColor(eProp, scolTemp);
			if(scolTemp == scolValue)
				return pChild;
		}
			

		if(bRecursive)
		{
			pCurrRes = pChild->getChildByPropertyValue(eProp, scolValue, true);
			if(pCurrRes)
				return pCurrRes;
		}
	}

	return NULL;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
const TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::getChildByPropertyValue(KEY_TYPE eProp, const char* pcsId, bool bRecursive) const
{
	if(!myChildren)
		return NULL;

	const TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild, *pCurrRes;
	int iCurr, iNum = myChildren->size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = (*myChildren)[iCurr];		
		if(pChild->doesPropertyExist(eProp) && strcmp(pChild->getStringProp(eProp), pcsId) == 0)
			return pChild;

		if(bRecursive)
		{
			pCurrRes = pChild->getChildByPropertyValue(eProp, pcsId, true);
			if(pCurrRes)
				return pCurrRes;
		}
	}

	return NULL;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::getChildByPropertyValue(KEY_TYPE eProp, FLOAT_TYPE fValue, bool bRecursive, FLOAT_TYPE fMaxTolerance, bool bGreaterOnly)
{
	if(!myChildren)
		return NULL;

	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild, *pCurrRes;
	int iCurr, iNum = myChildren->size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = (*myChildren)[iCurr];
		//if(pChild->doesPropertyExist(eProp) && pChild->getNumProp(eProp) == fValue)
		if(fMaxTolerance <= 0.0)
		{
			if(pChild->getNumProp(eProp) == fValue)
				return pChild;
		}
		else
		{
			FLOAT_TYPE fDiff = pChild->getNumProp(eProp) - fValue;
			if(bGreaterOnly)
			{
				if(fDiff <= fMaxTolerance && fDiff >= 0.0)
					return pChild;
			}
			else if(fabs(fDiff) <= fMaxTolerance)
				return pChild;
		}
		if(bRecursive)
		{
			pCurrRes = pChild->getChildByPropertyValue(eProp, fValue, true);
			if(pCurrRes)
				return pCurrRes;
		}
	}

	return NULL;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::getChildByPropertyValue(KEY_TYPE eProp, const char* pcsId, bool bRecursive)
{
	if(!myChildren)
		return NULL;

	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild, *pCurrRes;
	int iCurr, iNum = myChildren->size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = (*myChildren)[iCurr];
		if(pChild->doesPropertyExist(eProp) && strcmp(pChild->getStringProp(eProp), pcsId) == 0)
			return pChild;

		if(bRecursive)
		{
			pCurrRes = pChild->getChildByPropertyValue(eProp, pcsId, true);
			if(pCurrRes)
				return pCurrRes;
		}
	}

	return NULL;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::getChildByTag(const char* pcsTag, bool bRecursive)
{
	if(!myChildren)
		return NULL;

	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild, *pCurrRes;
	int iCurr, iNum = myChildren->size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = (*myChildren)[iCurr];
		if(IS_STRING_EQUAL(pChild->getTag(), pcsTag))
			return pChild;

		if(bRecursive)
		{
			pCurrRes = pChild->getChildByTag(pcsTag, true);
			if(pCurrRes)
				return pCurrRes;
		}
	}

	return NULL;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::getParentByTag(const char* pcsTag)
{
	if(!pcsTag)
		return NULL;

	TResourceItem<KEY_TYPE, STORAGE_TYPE> *pCurrRes;
	for(pCurrRes = this; pCurrRes && !IS_STRING_EQUAL(pCurrRes->getTag(), pcsTag); pCurrRes = pCurrRes->getParent())
	{
		// Nothing
	}

	return pCurrRes;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::copyResourcesFrom(const TResourceItem<KEY_TYPE, STORAGE_TYPE>& rSrc, bool bCopyChildren, TResourceItemClassAllocator<KEY_TYPE, STORAGE_TYPE>* pAllocator, bool bCallPostInit)
{
#ifdef ENABLE_COW_FOR_PROPERTIES
	if(rSrc.getDataSourceType() == SourceConst)
	{
		myProps.clear();
		myConstRefItem = const_cast< TResourceItem<KEY_TYPE, STORAGE_TYPE>* >(&rSrc);
	}
	else
	{
		if(rSrc.getDataSourceType() != SourceFile)
			myConstRefItem = rSrc.myConstRefItem;
#endif

	myProps = rSrc.myProps;


#ifdef ENABLE_COW_FOR_PROPERTIES
	}
#endif

#if defined(_DEBUG) || defined(_DEBUG)
	myDebugId = rSrc.myDebugId;
	myDebugIdChar = myDebugId.c_str();
#endif

	if(bCopyChildren)
	{
		// Copy all the children, we have any
		if(myChildren)
			deleteChildren();

		if(rSrc.myChildren)
		{
			TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild;
			int iCurrChild, iNumChildren = rSrc.myChildren->size();
			for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
			{
				pChild = this->addChild((*rSrc.myChildren)[iCurrChild]->getTag(), pAllocator, (*rSrc.myChildren)[iCurrChild]);
				// We never call post-init on children
				pChild->copyResourcesFrom(*(*rSrc.myChildren)[iCurrChild], bCopyChildren, pAllocator, false);
				initAsGameObject(pChild, this);
			}
		}

		if(bCallPostInit)
			this->postInitChildrenRecursive();
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setNumProp(KEY_TYPE eProperty, FLOAT_TYPE fValue)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true);
	pProp->setNum(fValue);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setDoubleProp(KEY_TYPE eProperty, double dValue)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true);
	pProp->setDouble(dValue);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setStringProp(KEY_TYPE eProperty, const char* pcsValue)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true);
	pProp->setString(pcsValue);
#if defined(_DEBUG) || defined(_DEBUG)
	if(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::isIdProperty(eProperty))
	{
		myDebugId = pcsValue;
		myDebugIdChar = myDebugId.c_str();
	}
#endif
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setBoolProp(KEY_TYPE eProperty, bool bValue)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true);
	pProp->setBool(bValue);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setArray2DProp(KEY_TYPE eProperty, FLOAT_TYPE* fArray, int iW, int iH)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true);
	pProp->setFloatArray(fArray, iW, iH);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setEnumProp(KEY_TYPE eProperty, int iIndex, const char* pcsValue)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true, true);
	pProp->setEnumValue(iIndex, pcsValue);	
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setNumericEnumPropValue(KEY_TYPE eProperty, int iIndex, FLOAT_TYPE fValue)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true, true);
	pProp->setNumericEnumValue(iIndex, fValue);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::addToNumericEnumProp(KEY_TYPE eProperty, FLOAT_TYPE fNum)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true, true);
	pProp->addNumericEnumValue(fNum);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::addToEnumPropAsLong(KEY_TYPE eProperty, GTIME lValue)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true, true);
	pProp->addEnumValueAsLong(lValue);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::addToEnumProp(KEY_TYPE eProperty, const char* pcsValue)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true, true);
	pProp->addEnumValue(pcsValue);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::saveToString(STRING_TYPE& strOut, FormatType eFormat, TStringMemoryCacher *pCacher, bool bSaveAsDefault, int iDbSaveLevel) const
{
	if(eFormat == FormatNative)
		saveToStringNative(strOut, pCacher, bSaveAsDefault);
	else if(eFormat == FormatJSON)
		saveToStringJSON(strOut, pCacher);
	ELSE_ASSERT;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::saveToStringNative(STRING_TYPE& strOut, TStringMemoryCacher *pCacher, bool bSaveAsDefault, int iDbSaveLevel) const
{
	_ASSERT(myTag.length() > 0);

	// Open tag
	// Note: Do NOT make this string static. If we have nested saveToString() calls,
	// it will overwrite parts of it.
	/// string strTemp = "<" + myTag + ">\n";

	string *pStrTemp = NULL;
	if(pCacher)
		pStrTemp = &pCacher->getNewObject()->myString;
	else
		pStrTemp = new string();

#ifdef _DEBUG
	int iLevel;
	for(iLevel = 0; iLevel < iDbSaveLevel; iLevel++)
		*pStrTemp += "\t";
#else
	*pStrTemp = "";
#endif

	*pStrTemp += "<" + myTag + ">\n";

	// Note that if enable COW, this will only traverse
	// the properties that were modified. This might be
	// ok, since we almost always use the proper creation
	// path which will reference the default anyway.
#ifdef ENABLE_COW_FOR_PROPERTIES
	// We always need to save the type name. So we force it here.
	bool bSavedId = false;
#endif

	typename HashMap < KEY_TYPE, STORAGE_TYPE, TResourceProperty<KEY_TYPE, STORAGE_TYPE>, RESOURCE_ITEM_NUM_BUCKETS >::iterator mi;
	for(mi = myProps.begin(); mi != myProps.end(); mi++)
	{
#ifdef ENABLE_COW_FOR_PROPERTIES
		if(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::isIdProperty((KEY_TYPE)mi.getKey()))
				bSavedId = true;
#endif
			mi.getValue().saveToStringNative(*pStrTemp, iDbSaveLevel + 1);
	}

#ifdef ENABLE_COW_FOR_PROPERTIES
	if(!bSavedId)
	{
		if(myConstRefItem)
		{
			TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pConstProp = myConstRefItem->myProps.find(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIdPropertyValue((KEY_TYPE)0));
			if(pConstProp)
				pConstProp->saveToStringNative(*pStrTemp, iDbSaveLevel + 1);
			ELSE_ASSERT;
		}
		ELSE_ASSERT;
	}
#endif

	// Now save any children we might have
	if(myChildren)
	{
		int iChild, iNum = myChildren->size();
		for(iChild = 0; iChild < iNum; iChild++)
			(*myChildren)[iChild]->saveToStringNative(*pStrTemp, pCacher, iDbSaveLevel + 1);
	}

	// Close tag
#ifdef _DEBUG
	for(iLevel = 0; iLevel < iDbSaveLevel; iLevel++)
		*pStrTemp += "\t";
#endif
	*pStrTemp += "</" + myTag + ">\n";

	// Append to output
	strOut += *pStrTemp;

	if(!pCacher)
		delete pStrTemp;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::saveToStringJSON(STRING_TYPE& strOut, TStringMemoryCacher *pCacher) const
{
	_ASSERT(myTag.length() > 0);

	// Open tag
	// Note: Do NOT make this string static. If we have nested saveToString() calls,
	// it will overwrite parts of it.
	STRING_TYPE *pStrTemp = NULL;
	if(pCacher)
		pStrTemp = &pCacher->getNewObject()->myString;
	else
		pStrTemp = new STRING_TYPE();
#ifdef SIMPLER_JSON_SAVE_FORMAT
	STRING_TYPE* pTempNameString = NULL;
#endif

#ifdef SIMPLER_JSON_SAVE_FORMAT
	*pStrTemp = STR_LIT("{ \n");
#else
	*pStrTemp = STR_LIT("{ \"") + myTag + STR_LIT("\" : { \n");
#endif
	// Note that if enable COW, this will only traverse
	// the properties that were modified. This might be
	// ok, since we almost always use the proper creation
	// path which will reference the default anyway.
#ifdef ENABLE_COW_FOR_PROPERTIES
	// We always need to save the type name. So we force it here.
	bool bSavedId = false;
#endif

	typename HashMap < KEY_TYPE, STORAGE_TYPE, TResourceProperty<KEY_TYPE, STORAGE_TYPE>, RESOURCE_ITEM_NUM_BUCKETS >::iterator mi;
	for(mi = myProps.begin(); mi != myProps.end(); mi++)
	{
		if(mi != myProps.begin())
			*pStrTemp += STR_LIT(",\n");

#ifdef ENABLE_COW_FOR_PROPERTIES
		if(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::isIdProperty((KEY_TYPE)mi.getKey()))
			bSavedId = true;
#endif
		mi.getValue().saveToStringJSON(*pStrTemp);
	}

#ifdef ENABLE_COW_FOR_PROPERTIES
	if(!bSavedId)
	{
		if(myConstRefItem)
		{
			*pStrTemp += STR_LIT(",\n");

			TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pConstProp = myConstRefItem->myProps.find(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIdPropertyValue((KEY_TYPE)0));
			if(pConstProp)
				pConstProp->saveToStringJSON(*pStrTemp);
			else
			{
				_ASSERT(0);
			}
		}
		else
		{
			// No type name at all??? Unpossible!
			_ASSERT(0);
		}
	}
#endif

#ifndef SIMPLER_JSON_SAVE_FORMAT
	*pStrTemp += STR_LIT(" }");
#endif

	// Now save any children we might have
	if(myChildren)
	{
		int iChild, iNum = myChildren->size();
		if(iNum > 0)
		{
#ifdef SIMPLER_JSON_SAVE_FORMAT
			*pStrTemp += STR_LIT(", \n");
#else
			*pStrTemp += STR_LIT(", \"children\" : [ \n");
#endif
			for(iChild = 0; iChild < iNum; iChild++)
			{
				if(iChild > 0)
					*pStrTemp += STR_LIT(", ");

#ifdef SIMPLER_JSON_SAVE_FORMAT
				if(pCacher)
					pTempNameString = &pCacher->getNewObject()->myString;
				else
					pTempNameString = new STRING_TYPE();

				*pStrTemp += STR_LIT("\"");
				(*myChildren)[iChild]->getAsString(TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIdPropertyValue((KEY_TYPE)0), *pTempNameString);
				*pStrTemp += *pTempNameString;
				*pStrTemp += STR_LIT("\" : ");
#endif
				(*myChildren)[iChild]->saveToStringJSON(*pStrTemp, pCacher);
			}
#ifdef SIMPLER_JSON_SAVE_FORMAT
			//*pStrTemp += " } ";
#else
			*pStrTemp += STR_LIT(" ] ");
#endif
		}
	}

	*pStrTemp += STR_LIT("}\n");

	// Append to output
	strOut += *pStrTemp;

	if(!pCacher)
	{
		delete pStrTemp;
#ifdef SIMPLER_JSON_SAVE_FORMAT
		delete pTempNameString;
#endif
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::removeProperty(KEY_TYPE eProperty)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = myProps.find(eProperty);
	if(!pProp)
	{
		// Removing non-existant property
		// With COW, this may happen if we're trying to
		// remove a property that has not been modified.
		// Our only choice is to copy the whole thing over,
		// and then remove the property.
		if(myConstRefItem)
		{
			typename HashMap < KEY_TYPE, STORAGE_TYPE, TResourceProperty<KEY_TYPE, STORAGE_TYPE>, RESOURCE_ITEM_NUM_BUCKETS >::iterator pi;
			for(pi = myConstRefItem->myProps.begin(); pi != myConstRefItem->myProps.end(); pi++)
			{
				// Do not overwrite anything that already exists
				if(!myProps.find(pi.getValue().getPropertyName()))
					myProps.insert(pi.getValue().getPropertyName(), pi.getValue());
			}
			myDataSourceType = SourceNone;
			myConstRefItem = NULL;

			pProp = myProps.find(eProperty);
		}
	}

	// Try to remove it
	if(!pProp)
	{ 
		// Removing a truly non-existant property.
		_ASSERT(0); 
	}

	myProps.remove(eProperty);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
int TResourceItem<KEY_TYPE, STORAGE_TYPE>::getNumChildren() const
{
	if(!myChildren)
		return 0;
	else
		return myChildren->size();
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
const TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::getChild(int iIndex) const
{
	if(!myChildren)
		return NULL;
	else if(iIndex >= 0 && iIndex < (int)myChildren->size())
		return (*myChildren)[iIndex];
	else
		return NULL;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::getChild(int iIndex)
{
	if(!myChildren)
		return NULL;
	else if(iIndex >= 0 && iIndex < (int)myChildren->size())
		return (*myChildren)[iIndex];
	else
		return NULL;
}
/*****************************************************************************/
TStringVector g_vecTagCheck;
template < class KEY_TYPE, class STORAGE_TYPE > 
bool TResourceItem<KEY_TYPE, STORAGE_TYPE>::hasTag(const char* pcsTag) const
{
	KEY_TYPE eTagsProp = ResourceProperty::getTagsPropertyValue((KEY_TYPE)0);
	//bool bRes = this->findStringInEnumProp(PropertyTags, pcsTag) >= 0;
	bool bRes = this->findStringInEnumProp(eTagsProp, pcsTag) >= 0;
	if(!bRes)
	{
		// We may just have a single property. Check that.
		if(this->doesPropertyExist(eTagsProp) && this->getPropertyDataType(eTagsProp) == PropertyDataString &&
			strcmp(this->getStringProp(eTagsProp), pcsTag) == 0)
			bRes = true;
	}
	return bRes;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::addTag(const char* pcsTag)
{
	// We can have three cases
	// 1) No tag property exists
	// 2) The property exists, but is a single string
	// 3) The property exists as a string list
	KEY_TYPE eTagsProp = ResourceProperty::getTagsPropertyValue((KEY_TYPE)0);
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProperty = this->findProperty(eTagsProp);
	if(!pProperty)
	{
		// No property exists. Create it, set as string.
		pProperty = this->findProperty(eTagsProp, true);
		pProperty->setString(pcsTag);
		return;
	}

	// If we got here, the property already exists.
	if(pProperty->getNumEnumValues() > 0)
	{
		// The enum is already a list of strings
		pProperty->addEnumValue(pcsTag);
	}
	else
	{
		// This is a tag property with one tag. Convert it to an enum.
		string strTag;
		pProperty->getAsString(strTag);
		pProperty->addEnumValue(strTag.c_str());
		pProperty->addEnumValue(pcsTag);
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setDataSourceType(SourceType eValue)
{
	myDataSourceType = eValue;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		this->getChild(iCurrChild)->setDataSourceType(eValue);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setParent(TResourceItem<KEY_TYPE, STORAGE_TYPE>* pParent)
{
	// This is really a method that shouldn't exist. Disallow changing parent.
	_ASSERT(myParent == NULL);
	myParent = pParent;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::initAsGameObject(TResourceItem<KEY_TYPE, STORAGE_TYPE>* pNewChild, TResourceItem<KEY_TYPE, STORAGE_TYPE>* pSrcChild)
{
	/*
	IPlaneObject* pChildObj = dynamic_cast<IPlaneObject*>(pNewChild);

	if(!pChildObj)
	{
		// It's ok if we have a non-game object. This can happen
		// when loading defaults with children from res files.
		// _ASSERT(0);
		return;
	}

	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pTopParent = pSrcChild;
	while(pTopParent->getParent())
		pTopParent = pTopParent->getParent();

	IPlaneObject* pTopParentObj = dynamic_cast<IPlaneObject*>(pTopParent);
	if(!pTopParentObj)
	{
		_ASSERT(0);
		return;
	}

	// Now, init the new object with vars from the top parent.
	pNewChild->onAllocated(pTopParentObj->getParentPlane());
	*/
	pNewChild->onAllocated(pSrcChild);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setFromString(KEY_TYPE eProperty, const char* pcsValue)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true, true);
	pProp->setFromString(pcsValue);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setAsLong(KEY_TYPE eProperty, GTIME lValue)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true, true);
	pProp->setAsLong(lValue);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setAsColor(KEY_TYPE eProperty, const SColor& scolValue, bool bAddAlpha)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true, true);
	pProp->setAsColor(scolValue, bAddAlpha);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setFromVector(KEY_TYPE eProperty, const vector < FLOAT_TYPE >& vecNumbers)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true, true);
	pProp->setFromVector(vecNumbers);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setFromVector(KEY_TYPE eProperty, const TStringVector& vecStrings)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true, true);
	pProp->setFromVector(vecStrings);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setAsRect(KEY_TYPE eProperty, const SRect2D& srRect)
{
	TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty, true, true);
	pProp->setAsRect(srRect);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::setId(const char* pcsId)
{
	KEY_TYPE tIdProp = ResourceProperty::getIdPropertyValue((KEY_TYPE)0);
	this->setStringProp(tIdProp, pcsId);

#if defined(_DEBUG) || defined(DEBUG_MODE)
	myDebugId = pcsId;
	myDebugIdChar = myDebugId.c_str();
#endif
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::makeIdsUniqueRecursive(int& iGlobalCounter)
{
	KEY_TYPE tIdProp = ResourceProperty::getIdPropertyValue((KEY_TYPE)0);
	if(this->doesPropertyExist(tIdProp))
	{
		string strTempName, strNumSuffix;
		strTempName = this->getStringProp(tIdProp);
		StringUtils::stripTrailingDigits(strTempName);
#ifdef RES_COLL_NAMEGEN_DEBUG
		gLog("RESCOLL_NAMEGEN: Name in = %s, glob counter = %d ", strTempName.c_str(), iGlobalCounter);
#endif
		StringUtils::numberToString(iGlobalCounter, strNumSuffix);
		iGlobalCounter++;
		strTempName += strNumSuffix;
#ifdef RES_COLL_NAMEGEN_DEBUG
		gLog("new name out = %s\n", strTempName.c_str());
#endif
		// Save the old id for future lookups.
		// We use that to override defaults.
		KEY_TYPE tDefOldIdProp = ResourceProperty::getDefOldIdPropertyValue((KEY_TYPE)0);

		// Do not overwrite the original old name ever, since for objects with children with
		// presets we may do renaming more than once. Maybe we shouldn't...
		if(!this->doesPropertyExist(tDefOldIdProp))
			this->setStringProp(tDefOldIdProp, this->getStringProp(tIdProp));

		setId(strTempName.c_str());
	}

	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		getChild(iCurrChild)->makeIdsUniqueRecursive(iGlobalCounter);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
bool TResourceItem<KEY_TYPE, STORAGE_TYPE>::isEqualToContentOf(const TResourceItem<KEY_TYPE, STORAGE_TYPE>& rOther, KEY_TYPE eExcludedProperty ) const
{
	if(this->getNumChildren() != rOther.getNumChildren())
		return false;

	if(myConstRefItem != NULL && rOther.myConstRefItem == NULL || 
		myConstRefItem == NULL && rOther.myConstRefItem != NULL)
		return false;

	// Look at the const items
	if(myConstRefItem && rOther.myConstRefItem && myConstRefItem->isEqualToContentOf(*rOther.myConstRefItem, eExcludedProperty) == false)
		return false;

	// Go through all our own properties
	typename HashMap < KEY_TYPE, STORAGE_TYPE, TResourceProperty<KEY_TYPE, STORAGE_TYPE>, RESOURCE_ITEM_NUM_BUCKETS >::iterator mi;

	KEY_TYPE eCurrProp;
	bool bCompRes;
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pOtherProp;
	for(mi = myProps.begin(); mi != myProps.end(); mi++)
	{
		eCurrProp = mi.getValue().getPropertyName();
		if(ResourceProperty::arePropertiesEqual(eCurrProp, eExcludedProperty))
			continue;
		
		// Otherwise, find in the other item and compare by value
		pOtherProp = rOther.findProperty(eCurrProp);
		if(!pOtherProp)
			return false;

		bCompRes = mi.getValue().areEqualByValue(*pOtherProp);
		if(!bCompRes)
			return false;
	}

	// Go through all children
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild;
	if(myChildren)
	{
		int iCurrChild, iNumChildren = myChildren->size();
		for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		{
			pChild = (*rOther.myChildren)[iCurrChild];

			if(!pChild->isEqualToContentOf(*rOther.getChild(iCurrChild), eExcludedProperty))
				return false;		
		}
	}

	return true;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
bool TResourceItem<KEY_TYPE, STORAGE_TYPE>::loadFromStringList(TStringVector& vecStrings)
{
	clearEverything();
	
	int iCurr, iNum = vecStrings.size();
	if(iNum % 2 != 0 || iNum == 0)
		ASSERT_RETURN_FALSE;

	KEY_TYPE eProp;
	const char* pcsValPtr;
	bool bIsAnUPNumber;
	FLOAT_TYPE fValue;
	PropertyMapper* pPropertyMapper = PropertyMapper::getInstance();
	for(iCurr = 0; iCurr < iNum; iCurr += 2)
	{
		// Try to map the first key
		eProp = ResourceProperty::mapStringToPropertyType(pPropertyMapper, vecStrings[iCurr], (KEY_TYPE)0);
		
		// Now see if the value is bool, num, or string
		bIsAnUPNumber = false;
		pcsValPtr = vecStrings[iCurr + 1].c_str();
		if(IS_STRING_EQUAL(pcsValPtr, TRUE_VALUE)
			|| IS_STRING_EQUAL(pcsValPtr, "true")
			|| IS_STRING_EQUAL(pcsValPtr, FALSE_VALUE)
			|| IS_STRING_EQUAL(pcsValPtr, "false"))
		{
			this->setBoolProp(eProp, (IS_STRING_EQUAL(pcsValPtr, TRUE_VALUE) || IS_STRING_EQUAL(pcsValPtr, "true")) );
		}
		else if(StringUtils::isANumber(pcsValPtr, false) || (bIsAnUPNumber = StringUtils::isAnUPNumber(vecStrings[iCurr + 1])) )
		{
			if(bIsAnUPNumber)
				fValue = convertUPNumberToScreenPixels(vecStrings[iCurr + 1].c_str());
			else
				fValue = atof(pcsValPtr);
			this->setNumProp(eProp, fValue);
		}
		else
			this->setStringProp(eProp, pcsValPtr);
	}

	return true;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::removeCOWReferences()
{
	typename HashMap < KEY_TYPE, STORAGE_TYPE, TResourceProperty<KEY_TYPE, STORAGE_TYPE>, RESOURCE_ITEM_NUM_BUCKETS >::iterator mi;
	KEY_TYPE eCurrProp;
	bool bPreserveExistingProps = true;
	if(!myConstRefItem)
		return;

	for(mi = myConstRefItem->myProps.begin(); mi != myConstRefItem->myProps.end(); mi++)
	{
		eCurrProp = mi.getValue().getPropertyName();
		if(bPreserveExistingProps && myProps.find(eCurrProp))
			continue;

		myProps.insert(eCurrProp, mi.getValue());
#ifdef DO_PARANOID_RI_CHECKING
		_ASSERT(myProps.find(eCurrProp));
#endif
	}

	myConstRefItem = NULL;
	myDataSourceType = SourceNone;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
int TResourceItem<KEY_TYPE, STORAGE_TYPE>::getChildIndex(TResourceItem* pTargetChild)
{
	int iCurr, iNum = getNumChildren();
	const TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild;

	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = getChild(iCurr);
		if(!pChild)
			ASSERT_CONTINUE;
		if(pChild == pTargetChild)
			return iCurr;
	}

	return -1;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::postInitRecursive()
{
	postInit();
	postInitChildrenRecursive();
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::postInitChildrenRecursive()
{
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild;
	int iCurr, iNum = getNumChildren();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = getChild(iCurr);
		if(!pChild)
			ASSERT_CONTINUE;
		pChild->postInitRecursive();
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::toStringMap(TStringStringMap& rMapOut) const
{
	PropertyMapper* pPropertyMapper = PropertyMapper::getInstance();

	rMapOut.clear();
	const char* pcsPropString;
	string strTemp;
	typename HashMap < KEY_TYPE, STORAGE_TYPE, TResourceProperty<KEY_TYPE, STORAGE_TYPE>, RESOURCE_ITEM_NUM_BUCKETS >::iterator mi;
	for(mi = myProps.begin(); mi != myProps.end(); mi++)
	{
		pcsPropString = pPropertyMapper->getPropertyString((KEY_TYPE)mi.getKey());
		mi.getValue().getAsString(strTemp);
		rMapOut[pcsPropString] = strTemp;
	}
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::cloneSelf(bool bCopyChildren) const
{
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pRes = new TResourceItem<KEY_TYPE, STORAGE_TYPE>;
	pRes->setTag(this->getTag());
	pRes->copyResourcesFrom(*this, bCopyChildren);
	return pRes;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::getSelfOrParentByPropertyValue(KEY_TYPE eProp, const char* pcsValue)
{
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pCurr;
	for(pCurr = this; pCurr != NULL; pCurr = pCurr->getParent())
	{
		if(pCurr->doesPropertyExist(eProp) && strcmp(pCurr->getStringProp(eProp), pcsValue) == 0)
			return pCurr;
	}
	return NULL;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE > 
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceItem<KEY_TYPE, STORAGE_TYPE>::getSelfOrParentByPropertyValue(KEY_TYPE eProp, bool bValue)
{
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pCurr;
	for(pCurr = this; pCurr != NULL; pCurr = pCurr->getParent())
	{
		if(pCurr->doesPropertyExist(eProp) && pCurr->getBoolProp(eProp) == bValue)
			return pCurr;
	}
	return NULL;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
bool TResourceItem<KEY_TYPE, STORAGE_TYPE>::getIsNone(KEY_TYPE eProperty) const
{
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pProp = this->findProperty(eProperty);
	if(pProp)
		return pProp->getIsNone();
	else
		return true;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::sortChildren(KEY_TYPE eProperty, bool bAscending)
{
	if(!myChildren)
		return;

	std::sort(myChildren->begin(), myChildren->end(), ResItemChildSorter<KEY_TYPE, STORAGE_TYPE>(eProperty, bAscending));
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
bool TResourceItem<KEY_TYPE, STORAGE_TYPE>::getIsLessThan(KEY_TYPE eProperty, const TResourceItem<KEY_TYPE, STORAGE_TYPE>& rOther) const
{
	// Compare the props:
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pOwnProp = this->findProperty(eProperty);
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pOtherProp = rOther.findProperty(eProperty);

	if(!pOwnProp && !pOtherProp)
		return false;
	else if(pOwnProp && !pOtherProp)
		return false;
	else if(!pOwnProp && pOtherProp)
		return true;

	// Otherwise, properties exist on both. Compare.
	return pOwnProp->getIsLessThan(*pOtherProp);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
bool TResourceItem<KEY_TYPE, STORAGE_TYPE>::getIsGreaterThan(KEY_TYPE eProperty, const TResourceItem<KEY_TYPE, STORAGE_TYPE>& rOther) const
{
	// Compare the props:
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pOwnProp = this->findProperty(eProperty);
	const TResourceProperty<KEY_TYPE, STORAGE_TYPE>* pOtherProp = rOther.findProperty(eProperty);

	if(!pOwnProp && !pOtherProp)
		return false;
	else if(pOwnProp && !pOtherProp)
		return true;
	else if(!pOwnProp && pOtherProp)
		return false;

	// Otherwise, properties exist on both. Compare.
	return pOwnProp->getIsGreaterThan(*pOtherProp);
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::flattenCOWReferences(bool bRecursive)
{
	if(myConstRefItem)
	{
		typename HashMap < KEY_TYPE, STORAGE_TYPE, TResourceProperty<KEY_TYPE, STORAGE_TYPE>, RESOURCE_ITEM_NUM_BUCKETS >::iterator pi;
		for(pi = myConstRefItem->myProps.begin(); pi != myConstRefItem->myProps.end(); pi++)
		{
			// Do not overwrite anything that already exists
			if(!myProps.find(pi.getValue().getPropertyName()))
				myProps.insert(pi.getValue().getPropertyName(), pi.getValue());
		}
		myDataSourceType = SourceNone;
		myConstRefItem = NULL;
	}

	if(bRecursive)
	{
		int iChild, iNumChildren = getNumChildren();
		for(iChild = 0; iChild < iNumChildren; iChild++)
			getChild(iChild)->flattenCOWReferences(true);
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
int TResourceItem<KEY_TYPE, STORAGE_TYPE>::findInVectorByPropValue(const vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecIn, KEY_TYPE eProp, const char* pcsValue)
{
	const TResourceItem <KEY_TYPE, STORAGE_TYPE>* pResItem;
	bool bDidMatch;
	const char* pcsCurrValue;
	int iCurr, iNum = vecIn.size();
	PropertyDataType ePropType;
	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
	{
		pResItem = vecIn[iCurr];
		bDidMatch = false;
		if(pResItem->doesPropertyExist(eProp))
		{
			ePropType = pResItem->getPropertyDataType(eProp);
			if(ePropType == PropertyDataStringList)
				bDidMatch = pResItem->findStringInEnumProp(eProp, pcsValue) >= 0;
			else if(ePropType == PropertyDataString)
			{
				pcsCurrValue = pResItem->getStringProp(eProp);
				if(pcsCurrValue && strcmp(pcsCurrValue, pcsValue) == 0)
					bDidMatch = true;
			}
		}

		if(bDidMatch)
			return iCurr;
	}

	return -1;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::filterVectorByPropValue(KEY_TYPE eProp, const char* pcsValue, vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecInOut, bool bInvert)
{
	TResourceItem <KEY_TYPE, STORAGE_TYPE>* pResItem;
	bool bDidMatch;
	const char* pcsCurrValue;
	int iCurr, iNum = vecInOut.size();
	PropertyDataType ePropType;
	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
	{
		pResItem = vecInOut[iCurr];
		bDidMatch = false;
		if(pResItem->doesPropertyExist(eProp))
		{
			ePropType = pResItem->getPropertyDataType(eProp);
			if(ePropType == PropertyDataStringList)
				bDidMatch = pResItem->findStringInEnumProp(eProp, pcsValue) >= 0;
			else if(ePropType == PropertyDataString)
			{
				pcsCurrValue = pResItem->getStringProp(eProp);
				if(pcsCurrValue && strcmp(pcsCurrValue, pcsValue) == 0)
					bDidMatch = true;
			}
		}

		if( (!bDidMatch && !bInvert) || (bDidMatch && bInvert) )
			vecInOut.erase(vecInOut.begin() + iCurr);
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::filterVectorByPropValue(KEY_TYPE eProp, bool bValue, vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecInOut, bool bInvert)
{
	TResourceItem <KEY_TYPE, STORAGE_TYPE>* pResItem;
	bool bDidMatch;
	int iCurr, iNum = vecInOut.size();
	PropertyDataType ePropType;
	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
	{
		pResItem = vecInOut[iCurr];
		bDidMatch = false;
		if(pResItem->doesPropertyExist(eProp))
		{
			ePropType = pResItem->getPropertyDataType(eProp);
			if(ePropType == PropertyDataBool)
				bDidMatch = pResItem->getBoolProp(eProp) == bValue;
		}
		// Absence of a bool prop treated as false
		else if(bValue == false)
			bDidMatch = true;

		if( (!bDidMatch && !bInvert) || (bDidMatch && bInvert) )
			vecInOut.erase(vecInOut.begin() + iCurr);
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::childrenToVector(vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecOut)
{
	int iCurr, iNum = getNumChildren();
	vecOut.clear();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		vecOut.push_back(getChild(iCurr));
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::itemVectorToPropValueVector(const vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecIn, KEY_TYPE eProp, TStringVector& vecOut)
{
	vecOut.clear();
	int iCurr, iNum = vecIn.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		vecOut.EMPLACE_BACK(vecIn[iCurr]->getStringProp(eProp));
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceItem<KEY_TYPE, STORAGE_TYPE>::loadFromString(string& strContents)
{
	clearEverything();

	TResourceCollection<KEY_TYPE, STORAGE_TYPE> rParentColl;
	rParentColl.loadFromString(strContents);

	int iNumItems = rParentColl.getNumItems();
	if(iNumItems == 0)
		return;

	_ASSERT(iNumItems == 1);
    TResourceItem<KEY_TYPE, STORAGE_TYPE>* pMainItem = rParentColl.getItem(0);

	// Now, we copy its properties onto us, but not the children; the children we insert ourselves
	this->copyResourcesFrom(*pMainItem, false);

	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pSrcChild;
	int iChild, iNumChildren = pMainItem->getNumChildren();
	for(iChild = iNumChildren - 1; iChild >= 0; iChild--)
	{
		pSrcChild = pMainItem->getChild(iChild);
		this->parentExistingNode(pSrcChild, 0);
		// Delete but do not deallocate
		pMainItem->deleteChild(iChild, false);
	}
}
/*****************************************************************************/
template class TResourceItem<PropertyType, PropertyType>;
template class TResourceItem<const char*, RESOURCEITEM_STRING_TYPE >;
};