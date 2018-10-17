#pragma once

/*****************************************************************************/
struct SResItemIteratorStackItem
{
	int myPreviousChildIndex;
};
typedef list < SResItemIteratorStackItem > TResItemIteratorStack;
/*****************************************************************************/
template < class RESOURCE_ITEM_TYPE >
class TDepthfirstResourceItemIterator
{
public:
	TDepthfirstResourceItemIterator(RESOURCE_ITEM_TYPE *pStartItem)
	{
		myMainItem = pStartItem;
		myCurrSubItem = pStartItem;
		myChildIndex = 0;
		// Note that this will *not* call the
		// virtual function override. However,
		// we do it anyway to be consistent and 
		// not need it if we're just looking for
		// the traversal of all nodes.
		ensurePointingToValidNode();
		_ASSERT(isEnd() || getItem());
	}

	TDepthfirstResourceItemIterator()
	{
		myMainItem = NULL;
		myCurrSubItem = NULL;
		myChildIndex = -1;
	}

	virtual ~TDepthfirstResourceItemIterator() { }

	void operator++(int)
	{
		advanceOne();
		_ASSERT(isEnd() || getItem());
		ensureSatisfiesConditions();
	}

	void toEnd() { myCurrSubItem = NULL; myChildIndex = -1; }
	bool isEnd() { if(!myCurrSubItem && myChildIndex < 0) return true; else return false; }

	bool operator==(const TDepthfirstResourceItemIterator& other) { return myMainItem == other.myMainItem && myCurrSubItem == other.myCurrSubItem && myChildIndex == other.myChildIndex; }
	bool operator!=(const TDepthfirstResourceItemIterator& other) { return ! (*this == other); }
	RESOURCE_ITEM_TYPE* getItem() { return myCurrSubItem ? dynamic_cast<RESOURCE_ITEM_TYPE*>(myCurrSubItem->getChild(myChildIndex)) : NULL; }

protected:

	// Override this to impose your own conditions on a node
	virtual bool getDoesSatisfyConditions(RESOURCE_ITEM_TYPE* pItem) { return pItem != myMainItem; }

	inline RESOURCE_ITEM_TYPE* getMainItem() { return myMainItem; }

	void ensureSatisfiesConditions()
	{
		_ASSERT(isEnd() || getItem());
		RESOURCE_ITEM_TYPE* pItem = getItem();
		while(pItem && !getDoesSatisfyConditions(pItem))
		{
			advanceOne();
			pItem = getItem();
		}
		_ASSERT(isEnd() || getItem());
	}
private:
	void advanceOne()
	{
		if(!myCurrSubItem || !myMainItem)
			return;

		RESOURCE_ITEM_TYPE* pItem = getItem();
		if(!pItem)
		{
			ensurePointingToValidNode();
			pItem = getItem();

			// Now, if we didn't have a node before
			// but have one now, we haven't visited it
			// yet, so we're done.
			if(pItem)
				return;
		}

		// If not, we must be at the end.
		if(!pItem)
			return;

		// Otherwise, we have a current node that we've already
		// visited. Try to advance from here - depthfirst.
		bool bHaveChildren = pItem->getNumChildren() > 0;
		if(!bHaveChildren)
		{
			// We're not a network. About the only thing we can do 
			// is increment the input index and try to ensure we 
			// have some node.
			myChildIndex++;
			ensurePointingToValidNode();
			return;
		}

		// Otherwise we're pointing at a network. Dive inside and call
		// self recursively.
		// Save our current pos
		SResItemIteratorStackItem rItem;
		rItem.myPreviousChildIndex = myChildIndex;
		myStack.push_back(rItem);

		// Now dive in
		myCurrSubItem = pItem;
		myChildIndex = 0;
		ensurePointingToValidNode();
	}

	void ensurePointingToValidNode()
	{
		if(!myCurrSubItem)
			return;

		RESOURCE_ITEM_TYPE* pItem = getItem();
		if(pItem)
			return;

		if(myChildIndex < 0)
			myChildIndex = 0;

		int iNumChildren = myCurrSubItem->getNumChildren();
		while(myChildIndex < iNumChildren && !getItem())
			myChildIndex++;

		pItem = getItem();
		if(pItem)
			return;

		// Now, we must have ran out of the nodes
		// in the current
		if(myStack.size() == 0)
		{
			// Zee end.
			toEnd();
			return;
		}

		// Dive back up
		TResItemIteratorStack::iterator si = myStack.end();
		si--;
		myChildIndex = (*si).myPreviousChildIndex;
		myCurrSubItem = myCurrSubItem->template getParent<RESOURCE_ITEM_TYPE>();
		myStack.pop_back();

		// Now, increment the input index, since we've already visited this, and call self recursively.
		myChildIndex++;
		ensurePointingToValidNode();
	}


private:

	RESOURCE_ITEM_TYPE *myMainItem;

	RESOURCE_ITEM_TYPE* myCurrSubItem;
	int myChildIndex;

	TResItemIteratorStack myStack;
};
/*****************************************************************************/
typedef TDepthfirstResourceItemIterator < ResourceItem > DepthfirstResourceItemIterator;
typedef TDepthfirstResourceItemIterator < StringResourceItem > DepthfirstStringResourceItemIterator;
