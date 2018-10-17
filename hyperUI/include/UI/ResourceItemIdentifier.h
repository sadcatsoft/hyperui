#pragma once

/*****************************************************************************/
template < class ITEM_TYPE, class PROPERTY_TYPE, class KEY_TYPE, class PROPERTY_KEY_STORAGE_TYPE >
class HYPERUI_API TResourceItemIdentifier : public IParmBasedIdentifier<PROPERTY_TYPE>
{
public:
	TResourceItemIdentifier(ITEM_TYPE* pTargetItem, KEY_TYPE tProperty);
	TResourceItemIdentifier();
	virtual ~TResourceItemIdentifier() { }

	virtual IUndoItem* createUndoItemForSelfChange(UIElement* pSourceElem);
	virtual const char* getUndoStringForSelfChange() { return "change resource item"; }

	virtual void refreshStoredValueFromUI(UIElement* pSourceElem, bool bIsChangingContinuously);
	virtual void refreshUIFromStoredValue(UIElement* pTargetElem);

	virtual bool isValid() const { return myTargetItem && !PROPERTY_TYPE::isNullProperty(myTargetElementParmName); }
	virtual bool getIsEnabled(UIElement* pIdentifierOwnerElem) { return true; }
	virtual bool getIsVisible() { return true; }

	inline KEY_TYPE getParmName() const { return myTargetElementParmName; }

private:

	ITEM_TYPE* myTargetItem;
	PROPERTY_KEY_STORAGE_TYPE myTargetElementParmName;
};
/*****************************************************************************/
typedef TResourceItemIdentifier < StringResourceItem, StringResourceProperty, const char*, RESOURCEITEM_STRING_TYPE > StringResourceItemIdentifier;
typedef TResourceItemIdentifier < ResourceItem, ResourceProperty, PropertyType, PropertyType > ResourceItemIdentifier;