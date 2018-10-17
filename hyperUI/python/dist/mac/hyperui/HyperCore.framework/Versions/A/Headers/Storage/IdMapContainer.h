#pragma once

/*****************************************************************************/
template < class TYPE >
class IdMapContainer
{
public:
    virtual ~IdMapContainer() {  }
	// DONT USE THIS! Otherwise, you may then
	// updated the id, but the map itself won't
	// update!
/*
    TYPE* addNewItem()
    {
		TYPE* pItem = new TYPE;
		myItems[pItem->getId()] = pItem;
		return pItem;
    }
*/

	inline TYPE* addNewItem(TYPE* pItem)
	{
		myItems[pItem->getId()] = pItem;
		return pItem;
	}

    inline const TYPE* findItemById(UNIQUEID_TYPE id) const
    {
		typename std::map < UNIQUEID_TYPE, TYPE* >::const_iterator mi = myItems.find(id);
		if(mi == myItems.end())
			return NULL;
		else
			return mi->second;
    }

	inline TYPE* findItemById(UNIQUEID_TYPE id)
	{
		typename std::map < UNIQUEID_TYPE, TYPE* >::iterator mi = myItems.find(id);
		if(mi == myItems.end())
			return NULL;
		else
			return mi->second;
	}


	inline TYPE* removeById(UNIQUEID_TYPE id, bool bDoDeallocate = true)
    {
		TYPE* pItem = NULL;
		typename std::map < UNIQUEID_TYPE, TYPE* >::iterator mi = myItems.find(id);
		if(mi != myItems.end())
		{
			if(bDoDeallocate)
				delete mi->second;
			else
				pItem = mi->second;
			myItems.erase(mi);
		}

		return pItem;
    }

	inline void removeByPointer(TYPE* pItem, bool bDoDeallocate)
	{
		typename std::map < UNIQUEID_TYPE, TYPE* >::iterator mi;
		for(mi = myItems.begin(); mi != myItems.end(); mi++)
		{
			if(mi->second == pItem)
			{
				if(bDoDeallocate)
					delete mi->second;
				myItems.erase(mi);
				return;
			}
		}

		// Item not found. Careful since we might be leaving
		// pointers around.
		_ASSERT(0);
	}

    inline void clearItems()
    {
		typename std::map < UNIQUEID_TYPE, TYPE* >::iterator mi;
		for(mi = myItems.begin(); mi != myItems.end(); mi++)
			delete mi->second;
		myItems.clear();
    }

	inline bool isEmpty() const { return myItems.size() == 0;  }

	class Iterator
	{
	public:
		Iterator() { myParent = NULL; }
		Iterator(IdMapContainer* pParent) { myParent = pParent; li = myParent->myItems.begin(); }
		inline void operator++(int) { li++; }
		bool operator==(const Iterator& other) const { return myParent == other.myParent && li == other.li; }
		bool operator!=(const Iterator& other) const { return ! (*this == other); }
		inline bool isEnd() const {  return myParent == NULL || li == myParent->myItems.end(); }

		inline TYPE* getItem() { return li->second; }
		inline const TYPE* getItem() const { return li->second; }

	private:
		IdMapContainer<TYPE> *myParent;
		typename map < UNIQUEID_TYPE, TYPE* >::iterator li;
	};

	Iterator begin() { return Iterator(this); }


protected:

    map < UNIQUEID_TYPE, TYPE* > myItems;
};
/*****************************************************************************/