#include "stdafx.h"

#define SAVE_LAST_ID			"svUIGLastId"

namespace HyperUI
{
/*****************************************************************************/
void UniqueIdGenerator::saveToResourceItem(StringResourceItem& rItem) const
{
	rItem.setAsLong(SAVE_LAST_ID, myLastId);
}
/*****************************************************************************/
void UniqueIdGenerator::saveToResourceItem(ResourceItem& rItem) const
{
	rItem.setAsLong(PropertySvLastId, myLastId);
}
/*****************************************************************************/
void UniqueIdGenerator::loadFromItem(const StringResourceItem& rItem)
{
	myLastId = rItem.getAsLong(SAVE_LAST_ID);
}
/*****************************************************************************/
void UniqueIdGenerator::loadFromItem(const ResourceItem& rItem)
{
	myLastId = rItem.getAsLong(PropertySvLastId);
}
/*****************************************************************************/
};