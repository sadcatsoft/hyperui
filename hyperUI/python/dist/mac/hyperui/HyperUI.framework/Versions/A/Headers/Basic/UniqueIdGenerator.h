#pragma once

/*****************************************************************************/
class UniqueIdGenerator
{
public:

    UniqueIdGenerator() { myLastId = 0; }
    virtual ~UniqueIdGenerator() { }

    inline UNIQUEID_TYPE generateId()
    {
		myLastId++;
		return myLastId;
    }

	void reset() { myLastId = 0; }

	HYPERUI_API virtual void saveToResourceItem(StringResourceItem& rItem) const;
	HYPERUI_API virtual void loadFromItem(const StringResourceItem& rItem);

	HYPERUI_API virtual void saveToResourceItem(ResourceItem& rItem) const;
	HYPERUI_API virtual void loadFromItem(const ResourceItem& rItem);

	void onIdLoaded(UNIQUEID_TYPE id) { if(id > myLastId) myLastId = id; }

private:

    UNIQUEID_TYPE myLastId;
};
/*****************************************************************************/
