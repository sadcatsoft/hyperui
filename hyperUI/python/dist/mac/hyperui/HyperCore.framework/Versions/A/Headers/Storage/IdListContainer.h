#pragma once

/*****************************************************************************/
template < class TYPE >
class IdListContainer
{
public:
	virtual ~IdListContainer() {  }
	TYPE* addNewItem()
	{
		TYPE* pItem = new TYPE;
		myItems.push_back(pItem);
		return pItem;
	}

	TYPE* addNewItem(TYPE* pItem)
	{
		myItems.push_back(pItem);
		return pItem;
	}

	const TYPE* findItemById(UNIQUEID_TYPE id) const
	{
		typename std::list < TYPE* >::const_iterator li;
		for(li =  myItems.begin(); li != myItems.end(); li++)
		{
			if((*li)->getId() == id)
				return *li;
		}

		return NULL;
	}

	TYPE* findItemById(UNIQUEID_TYPE id)
	{
		typename std::list < TYPE* >::iterator li;
		for(li =  myItems.begin(); li != myItems.end(); li++)
		{
			if((*li)->getId() == id)
				return *li;
		}

		return NULL;
	}

	void removeById(UNIQUEID_TYPE id, bool bDeallocate)
	{
		typename std::list < TYPE* >::iterator li;
		for(li =  myItems.begin(); li != myItems.end(); li++)
		{
			if((*li)->getId() == id)
			{
				if(bDeallocate)
					delete (*li);
				myItems.erase(li);
				break;
			}
		}
	}

	void clearItems()
	{
		typename std::list < TYPE* >::iterator mi;
		for(mi = myItems.begin(); mi != myItems.end(); mi++)
			delete (*mi);
		myItems.clear();
	}

	inline bool isEmpty() const { return myItems.size() == 0; }
	inline int getNumItems() const { return myItems.size(); }

	TYPE* getItem(int iIndex)
	{
		if(iIndex < 0 || iIndex >= myItems.size())
			return NULL;

		int iCurrIndex;
		typename std::list < TYPE* >::iterator li;
		for(li =  myItems.begin(), iCurrIndex = 0; li != myItems.end(); li++, iCurrIndex++)
		{
			if(iCurrIndex == iIndex)
				return (*li);
		}
		return NULL;
	}

	const TYPE* getItem(int iIndex) const
	{
		if(iIndex < 0 || iIndex >= myItems.size())
			return NULL;

		int iCurrIndex;
		typename std::list < TYPE* >::const_iterator li;
		for(li =  myItems.begin(), iCurrIndex = 0; li != myItems.end(); li++, iCurrIndex++)
		{
			if(iCurrIndex == iIndex)
				return (*li);
		}
		return NULL;
	}

	class Iterator
	{
	public:
		Iterator() { myParent = NULL; }
		Iterator(const IdListContainer* pParent) { myParent = pParent; li = myParent->myItems.begin(); }
		inline void operator++(int) { li++; }
		inline void operator--(int) { li--; }
		bool operator==(const Iterator& other) const { return myParent == other.myParent && li == other.li; }
		bool operator!=(const Iterator& other) const { return ! (*this == other); }
		inline bool isEnd() const {  return myParent == NULL || li == myParent->myItems.end(); }
		inline void toEnd() { if(myParent) li = myParent->myItems.end(); }

		inline TYPE* getItem() { return const_cast<TYPE*>(*li); }
		inline const TYPE* getItem() const { return *li; }

	private:
		const IdListContainer<TYPE> *myParent;
		typename list < TYPE* >::const_iterator li;
	};

	Iterator begin() { return Iterator(this); }
	Iterator begin() const { return Iterator(this); }

	Iterator end() { Iterator it = Iterator(this); it.toEnd(); return it; }
	Iterator end() const { Iterator it = Iterator(this); it.toEnd(); return it; }

protected:

	friend class IdListContainer::Iterator;

	list < TYPE* > myItems;
};
/*****************************************************************************/
