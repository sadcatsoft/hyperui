#include "stdafx.h"

namespace HyperUI
{

/*****************************************************************************/
UIPlane* UIAllocator::extractParentPlane(IBaseObject* pData)
{
	IPlane* pParentPlane = dynamic_cast<IPlane*>(pData);
	if(!pParentPlane)
	{
		IPlaneObject* pPlaneObject = dynamic_cast<IPlaneObject*>(pData);
		if(pPlaneObject)
			pParentPlane = pPlaneObject->getParentPlane();
	}

	return dynamic_cast<UIPlane*>(pParentPlane);
}
/*****************************************************************************/
ResourceItem* UIAllocator::allocateNewItem(const ResourceItem* pSourceItem, IBaseObject* pData, const char* pcsTag)
{
	const CHAR_TYPE* pcsTypeToAlloc = pSourceItem->getStringProp(PropertyElemType);
	return this->allocateItemFromType(pcsTypeToAlloc, pData, pcsTag);
}
/*****************************************************************************/
ResourceItem* UIAllocator::allocateItemFromType(const CHAR_TYPE* pcsTypeToAlloc, IBaseObject* pData, const char* pcsTag)
{
	TStringAllocatorMap::iterator mi = myConstructors.find(pcsTypeToAlloc);

	// Check that an allocator exists
	if(mi == myConstructors.end())
	{
		gLog("\nERROR: No constructor defined for element of type %s; please use REGISTER_CUSTOM_UI_ELEMENT(%s); to do so.\n", pcsTypeToAlloc, pcsTypeToAlloc);
		ASSERT_RETURN_NULL;
	}

	AllocatorEntry& pAllocator = mi->second;
	UIElement* pRes = (*pAllocator.myConstructor)(this->extractParentPlane(pData));
	_ASSERT(pRes);
	if(pcsTag)
		pRes->setTag(pcsTag);
	return pRes;
}

/*****************************************************************************/
bool UIAllocator::getDoHaveConstructorFor(const CHAR_TYPE* pcsTypeToAlloc)
{
	TStringAllocatorMap::iterator mi = myConstructors.find(pcsTypeToAlloc);
	return mi != myConstructors.end();
}
/*****************************************************************************/
void UIAllocator::registerConstructor(const char* pcsType, UIElementConstructorType pSimpleConstructor)
{
	// Note that it's ok to explicitly overwrite elements, even standard ones, if we need to.
	AllocatorEntry rDummy;
	rDummy.myType = pcsType;
	rDummy.myConstructor = pSimpleConstructor;
	myConstructors[rDummy.myType] = rDummy;
}
/*****************************************************************************/
void UIAllocator::getAllRegisteredTypes(TStringVector& rVecOut)
{
	rVecOut.clear();
	TStringAllocatorMap::iterator mi;
	for(mi = myConstructors.begin(); mi != myConstructors.end(); mi++)
		rVecOut.push_back(mi->first);
}
/*****************************************************************************/
};