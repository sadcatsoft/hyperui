#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
template < class ITEM_TYPE, class PROPERTY_TYPE, class KEY_TYPE, class PROPERTY_KEY_STORAGE_TYPE > 
TResourceItemIdentifier< ITEM_TYPE, PROPERTY_TYPE, KEY_TYPE, PROPERTY_KEY_STORAGE_TYPE >::TResourceItemIdentifier(ITEM_TYPE* pTargetItem, KEY_TYPE tProperty)
{
	_ASSERT(!PROPERTY_TYPE::isNullProperty(tProperty));
	myTargetItem = pTargetItem;
	myTargetElementParmName = tProperty;
}
/*****************************************************************************/
template < class ITEM_TYPE, class PROPERTY_TYPE, class KEY_TYPE, class PROPERTY_KEY_STORAGE_TYPE > 
TResourceItemIdentifier< ITEM_TYPE, PROPERTY_TYPE, KEY_TYPE, PROPERTY_KEY_STORAGE_TYPE >::TResourceItemIdentifier()
{

}
/*****************************************************************************/
template < class ITEM_TYPE, class PROPERTY_TYPE, class KEY_TYPE, class PROPERTY_KEY_STORAGE_TYPE > 
void TResourceItemIdentifier< ITEM_TYPE, PROPERTY_TYPE, KEY_TYPE, PROPERTY_KEY_STORAGE_TYPE >::refreshStoredValueFromUI(UIElement* pSourceElem, bool bIsChangingContinuously)
{
	if(!myTargetItem)
		ASSERT_RETURN;
	PROPERTY_TYPE* pParm = myTargetItem->findProperty(myTargetElementParmName);
	if(!pParm)
		ASSERT_RETURN;
	this->refreshParmFromUI(pSourceElem, pParm, bIsChangingContinuously);
}
/*****************************************************************************/
template < class ITEM_TYPE, class PROPERTY_TYPE, class KEY_TYPE, class PROPERTY_KEY_STORAGE_TYPE > 
void TResourceItemIdentifier< ITEM_TYPE, PROPERTY_TYPE, KEY_TYPE, PROPERTY_KEY_STORAGE_TYPE >::refreshUIFromStoredValue(UIElement* pTargetElem)
{
	if(!myTargetItem)
		ASSERT_RETURN;
	PROPERTY_TYPE* pParm = myTargetItem->findProperty(myTargetElementParmName);

	if(!pParm)
		ASSERT_RETURN;
	this->refreshUIFromParm(pTargetElem, pParm);
}
/*****************************************************************************/
template < class ITEM_TYPE, class PROPERTY_TYPE, class KEY_TYPE, class PROPERTY_KEY_STORAGE_TYPE >
IUndoItem* TResourceItemIdentifier<ITEM_TYPE, PROPERTY_TYPE, KEY_TYPE, PROPERTY_KEY_STORAGE_TYPE>::createUndoItemForSelfChange(UIElement* pSourceElem)
{
	// Note that if we need this elsewhere, we can move this to the base class.
	// Watch out for performance.
	return pSourceElem->provideUndoItemForTargetIdentifierRecursive();
}
/*****************************************************************************/
template class TResourceItemIdentifier< StringResourceItem, StringResourceProperty, const char*, RESOURCEITEM_STRING_TYPE >;
template class TResourceItemIdentifier< ResourceItem, ResourceProperty, PropertyType, PropertyType >;
};