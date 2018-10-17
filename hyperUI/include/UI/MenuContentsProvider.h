#pragma once

struct SMenuEntry;
/*****************************************************************************/
class HYPERUI_API IMenuEnableStatusCallback
{
public:
	virtual ~IMenuEnableStatusCallback() { }

	virtual bool getIsMenuItemEnabled(const SMenuEntry* pEntry, const char* pcsTargetData) const = 0;
	virtual bool getIsMenuItemChecked(const SMenuEntry* pEntry, const char* pcsTargetData) const { return false; }
};
/*****************************************************************************/
class HYPERUI_API IMenuContentsProvider
{
public:
	virtual ~IMenuContentsProvider() { }

	virtual int getNumItems() = 0;
	virtual const char* getItemLabel(int iIndex) = 0;
	virtual const char* getItemAction(int iIndex) = 0;

	virtual const char* getItemId(int iIndex) { return getItemLabel(iIndex); }
	virtual const char* getItemActionValue(int iIndex) { return ""; }
	virtual const char* getItemActionValue2(int iIndex) { return ""; }
	virtual const char* getItemActionValue3(int iIndex) { return ""; }
	virtual const char* getItemIcon(int iIndex) { return ""; }

	virtual const char* getItemSubmenu(int iIndex) { return ""; }

	virtual bool getIsItemCheckmark(int iIndex) const { return false; }
};
/*****************************************************************************/
class HYPERUI_API IMenuContentsProviderFactory
{
public:
	virtual ~IMenuContentsProviderFactory() { }
	virtual IMenuContentsProvider* allocateProvider() = 0;
};
/*****************************************************************************/
class HYPERUI_API ResourceItemMenuProvider : public IMenuContentsProvider
{
public:
	ResourceItemMenuProvider()  { myItem = NULL; }
	ResourceItemMenuProvider(const ResourceItem* pItem) { setSourceItem(pItem); }
	virtual ~ResourceItemMenuProvider() { }

	void setSourceItem(const ResourceItem* pItem) { myItem = pItem; _ASSERT(myItem); }

	virtual int getNumItems() { return myItem->getEnumPropCount(PropertyMenuIds); }

	virtual const char* getItemId(int iIndex) { return myItem->getEnumPropValue(PropertyMenuIds, iIndex); }
	virtual const char* getItemLabel(int iIndex) { return myItem->doesPropertyExist(PropertyMenuLabels) ? myItem->getEnumPropValue(PropertyMenuLabels, iIndex) : ""; }
	virtual const char* getItemAction(int iIndex) { return myItem->doesPropertyExist(PropertyMenuActions) ? myItem->getEnumPropValue(PropertyMenuActions, iIndex) : ""; }
	virtual const char* getItemActionValue(int iIndex) { return myItem->doesPropertyExist(PropertyMenuActionValues) ? myItem->getEnumPropValue(PropertyMenuActionValues, iIndex) : ""; }
	virtual const char* getItemActionValue2(int iIndex) { return myItem->doesPropertyExist(PropertyMenuActionValues2) ? myItem->getEnumPropValue(PropertyMenuActionValues2, iIndex) : ""; }
	virtual const char* getItemActionValue3(int iIndex) { return myItem->doesPropertyExist(PropertyMenuActionValues3) ? myItem->getEnumPropValue(PropertyMenuActionValues3, iIndex) : ""; }
	virtual const char* getItemIcon(int iIndex) { return myItem->doesPropertyExist(PropertyMenuIcons) ? myItem->getEnumPropValue(PropertyMenuIcons, iIndex) : ""; }

	virtual const char* getItemSubmenu(int iIndex) { return myItem->doesPropertyExist(PropertyMenuSubitems) ? myItem->getEnumPropValue(PropertyMenuSubitems, iIndex) : ""; }
	virtual bool getIsItemCheckmark(int iIndex) const { return myItem->doesPropertyExist(PropertyMenuIsCheckmark) ? IS_STRING_EQUAL(myItem->getEnumPropValue(PropertyMenuIsCheckmark, iIndex), TRUE_VALUE) : false; }

private:

	const ResourceItem* myItem;
};
/*****************************************************************************/