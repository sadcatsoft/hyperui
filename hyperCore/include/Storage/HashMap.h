#pragma once

#define HASH_BUCKET_INDEX_TYPE		short
#define INSERT_GROW_MULT            2

#define ITERATOR_COUNT_TYPE  unsigned short

/********************************************************************************************/
template < class KEY_TYPE, class KEY_STORAGE_TYPE, class VALUE_TYPE, int NUM_BUCKETS >
class HashMap
{
public:
	struct HashPair
	{
		HashPair() { myIsValid = false; myNextIndex = -1; myPrevIndex = -1; }
		VALUE_TYPE myValue;
		KEY_STORAGE_TYPE myKey;
		bool myIsValid;
		int myNextIndex;
		int myPrevIndex;
	};

	HashMap()
	{ 
		myMemory = NULL;
		myFreedMemSlots = NULL;
		_ASSERT(NUM_BUCKETS % 2 == 0); 
		_ASSERT(NUM_BUCKETS > 0 && NUM_BUCKETS < 32767);
		allocate();
	}
	
	~HashMap() 
	{ 
		deallocate(); 
	}

	inline VALUE_TYPE* insertVal(KEY_TYPE iKey, VALUE_TYPE rValue) { return insert(iKey, rValue); }

	VALUE_TYPE* insert(KEY_TYPE iKey, const VALUE_TYPE& rValue)
	{
		HASH_BUCKET_INDEX_TYPE iBucket = hashAndBucket(iKey);
		_ASSERT(iBucket >= 0 && iBucket < NUM_BUCKETS);

		// First, we need to find a free memory cell
		HashPair* pNewPair;
		if(myMemory[iBucket].myIsValid)
		{
			int iTailPos;
			int iFreeMemIdx;

			// See if we find it in this list
			HashPair* pPair = NULL;
			int iIndex;
			for(iIndex = iBucket; iIndex >= 0; iIndex = pPair->myNextIndex)
			{
				pPair = &myMemory[iIndex];
				if(!pPair->myIsValid)
					break;
				else if(pPair->myKey == iKey)
				{
					// Replace the value and return
					pPair->myValue = rValue;
					return &pPair->myValue;
				}
			}

			iTailPos = myBucketTailPositions[iBucket];
			_ASSERT(iTailPos >= 0);
			_ASSERT(myMemory[iTailPos].myIsValid);

			// Then, we need to find the last element of each bucket.
			// Check the list of free spaces
#ifdef _DEBUG
			iFreeMemIdx = -1;
#endif
			if(myFreedMemSlots && myFreedMemSlots->size() > 0)
			{
				// Get one from there
				iFreeMemIdx = (*myFreedMemSlots)[myFreedMemSlots->size() - 1];
				myFreedMemSlots->erase(myFreedMemSlots->end() - 1);
			}
			else
			{
				// Grab the next one in memory
				if(myNextFreeMemPos >= myNumMemElems)
				{
					// Reallocate everything
					int iOldNumElems = myNumMemElems;
					myNumMemElems *= INSERT_GROW_MULT;
					HashPair* pOldMem = myMemory;
					myMemory = new HashPair[myNumMemElems];
					int iCurr;
					for(iCurr = 0; iCurr < iOldNumElems; iCurr++)
					{
						if(pOldMem[iCurr].myIsValid)
							myMemory[iCurr] = pOldMem[iCurr];
						_ASSERT((!myMemory[iCurr].myIsValid && myMemory[iCurr].myNextIndex == -1) || myMemory[iCurr].myIsValid);
					}
					delete[] pOldMem;
				}

				_ASSERT(myNextFreeMemPos >= NUM_BUCKETS && myNextFreeMemPos < myNumMemElems);
				iFreeMemIdx = myNextFreeMemPos;
				myNextFreeMemPos++;
			}

			_ASSERT(iFreeMemIdx >= 0);

			// Now, set that element:
			pNewPair = &myMemory[iFreeMemIdx];
			_ASSERT(pNewPair->myIsValid == false);
			pNewPair->myIsValid = true;
			pNewPair->myKey = iKey;
			pNewPair->myValue = rValue;
			pNewPair->myNextIndex = -1;

			_ASSERT(myMemory[iTailPos].myIsValid);
			myMemory[iTailPos].myNextIndex = iFreeMemIdx;
			pNewPair->myPrevIndex = iTailPos;

			myBucketTailPositions[iBucket] = iFreeMemIdx;
		}
		else
		{
			_ASSERT(myBucketTailPositions[iBucket] == 0);

			// Now, set that element:
			pNewPair = &myMemory[iBucket];
			_ASSERT(pNewPair->myIsValid == false);
			_ASSERT(pNewPair->myPrevIndex == -1);
			_ASSERT(pNewPair->myNextIndex == -1);
			pNewPair->myIsValid = true;
			pNewPair->myKey = iKey;
			pNewPair->myValue = rValue;
			myBucketTailPositions[iBucket] = iBucket;
		}
		return &pNewPair->myValue;
	}

	inline VALUE_TYPE* find(KEY_TYPE iKey)
	{
		HASH_BUCKET_INDEX_TYPE iBucket = hashAndBucket(iKey);
		_ASSERT(iBucket >= 0 && iBucket < NUM_BUCKETS);

		HashPair* pPair;
		for(pPair = &myMemory[iBucket]; pPair; pPair = pPair->myNextIndex < 0 ? NULL : &myMemory[pPair->myNextIndex])
		{
			if(!pPair->myIsValid)
			{
				_ASSERT(pPair->myNextIndex == -1);
				return (VALUE_TYPE*)0;
			}
			else if(pPair->myKey == iKey)
				return &pPair->myValue;
		}	
		return (VALUE_TYPE*)0;
	}

	inline const VALUE_TYPE* find(KEY_TYPE iKey) const
	{
		HASH_BUCKET_INDEX_TYPE iBucket = hashAndBucket(iKey);
		_ASSERT(iBucket >= 0 && iBucket < NUM_BUCKETS);

		HashPair* pPair;
		for(pPair = &myMemory[iBucket]; pPair; pPair = pPair->myNextIndex < 0 ? NULL : &myMemory[pPair->myNextIndex])
		{
			if(!pPair->myIsValid)
			{
				_ASSERT(pPair->myNextIndex == -1);
				return (VALUE_TYPE*)0;
			}
			else if(pPair->myKey == iKey)
				return &pPair->myValue;
		}	
		return (VALUE_TYPE*)0;
	}

	bool remove(KEY_TYPE iKey)
	{
		// To remove, do the find...
		HASH_BUCKET_INDEX_TYPE iBucket = hashAndBucket(iKey);
		_ASSERT(iBucket >= 0 && iBucket < NUM_BUCKETS);

		bool bEraseThisPair;
		HashPair* pPair = NULL;
		int iIndex;
		int iPrevIndex = -1;
		for(iIndex = iBucket; iIndex >= 0; iPrevIndex = iIndex, iIndex = pPair->myNextIndex)
		{
			pPair = &myMemory[iIndex];
			if(!pPair->myIsValid)
				return false;

			if(pPair->myKey != iKey)
				continue;

			// Got it!
			// Now, we must exclude it from the list:
			bEraseThisPair = true;
			if(iPrevIndex >= 0)
			{
				_ASSERT(myMemory[iPrevIndex].myIsValid);

				int iNextIndex = pPair->myNextIndex;
				myMemory[iPrevIndex].myNextIndex = iNextIndex;

				// If this was a tail pointer, also make sure
				// we update our bucket tail list
				if(iNextIndex == -1)
				{
					_ASSERT(myBucketTailPositions[iBucket] == iIndex);
					myBucketTailPositions[iBucket] = iPrevIndex;
				}
                else
                    myMemory[iNextIndex].myPrevIndex = iPrevIndex;

				// And remember that we have a free slot available
				if(!myFreedMemSlots)
					myFreedMemSlots = new std::vector < int >;
				_ASSERT(iIndex >= NUM_BUCKETS);
				myFreedMemSlots->push_back(iIndex);
			}
			else
			{
				// There is no prev index, we're the first.
				// But there may be stuff following behind us...

				if(pPair->myNextIndex == -1)
				{
					// If this was a tail pointer, also make sure
					// we update our bucket tail list
					myBucketTailPositions[iBucket] = 0;
				}
				else
				{
					// If we're here, we're deleting the bucket
					// item with other behind it. We can't really
					// remove it, so instead, we copy the last item
					// here, since the order doesn't matter, and 
					// erase the last item.
					_ASSERT(myBucketTailPositions[iBucket] != iBucket);
					int iTailPos = myBucketTailPositions[iBucket];
					_ASSERT(myMemory[iTailPos].myNextIndex == -1);

					int iPrevInnerIndex = myMemory[iTailPos].myPrevIndex;

					_ASSERT(iPrevInnerIndex >= 0 && iPrevInnerIndex != iTailPos);
					myMemory[iIndex].myValue = myMemory[iTailPos].myValue;
					myMemory[iIndex].myKey = myMemory[iTailPos].myKey;
					myMemory[iPrevInnerIndex].myNextIndex = -1;

					_ASSERT(myMemory[iIndex].myIsValid);

					myBucketTailPositions[iBucket] = iPrevInnerIndex;
					myMemory[iTailPos].myIsValid = false;
					myMemory[iTailPos].myNextIndex = -1;
					myMemory[iTailPos].myPrevIndex = -1;

					if(!myFreedMemSlots)
						myFreedMemSlots = new std::vector < int >;
					myFreedMemSlots->push_back(iTailPos);

					bEraseThisPair = false;
				}
			}

			if(bEraseThisPair)
			{
				pPair->myIsValid = false;
				pPair->myNextIndex = -1;
				pPair->myPrevIndex = -1;
			}
			return true;
		} // end over bucket list
		return false;
	}

	inline void clear()
	{
		// Careful: a lookup on a cleared hashmap shouldn't crash.
		// The lazy way. This resets it to the very beginning.
		deallocate();
		allocate();
	}

	inline void operator=(const HashMap& rOther)
	{
		// Copy...
		deallocate();

		myNumMemElems = rOther.myNumMemElems;
		myNextFreeMemPos = rOther.myNextFreeMemPos;

		memcpy(myBucketTailPositions, rOther.myBucketTailPositions, sizeof(int)*NUM_BUCKETS);

		if(rOther.myFreedMemSlots)
		{
			_ASSERT(myFreedMemSlots == NULL);
			myFreedMemSlots = new std::vector < int >;
			*myFreedMemSlots = *rOther.myFreedMemSlots;
		}

		_ASSERT(myMemory == NULL);
		myMemory = new HashPair[myNumMemElems];
		int iCurr;
		for(iCurr = 0; iCurr < myNumMemElems; iCurr++)
		{
			if(rOther.myMemory[iCurr].myIsValid)
				myMemory[iCurr] = rOther.myMemory[iCurr];
			_ASSERT((!myMemory[iCurr].myIsValid && myMemory[iCurr].myNextIndex == -1) || myMemory[iCurr].myIsValid);
		}
	}
	
	// Iterator
	struct iterator
	{
		iterator()
		{
			myParentMap = NULL;
			myBucketIndex = 0;
			myListIndex = myBucketIndex;
		}

		iterator(const HashMap& rParentMap)
		{
			setParent(rParentMap);
		}

		inline void setParent(const HashMap& rParentMap)
		{
			myParentMap = const_cast<HashMap*>(&rParentMap);
			myBucketIndex = 0;
			myListIndex = myBucketIndex;
			ensureValidIterator();
		}

		inline void toStart()
		{
			myBucketIndex = 0;
			myListIndex = myBucketIndex;
			ensureValidIterator();
		}

		inline void toEnd() 
		{
			myBucketIndex = NUM_BUCKETS;
			myListIndex = -1;
		}

		inline bool isEnd() const { return myBucketIndex >= NUM_BUCKETS || myListIndex < 0; }
		inline bool isInitialized() const { return myParentMap != NULL; }

		inline bool operator==(const iterator& other) const { return (myBucketIndex == other.myBucketIndex && myListIndex == other.myListIndex); }
		inline bool operator!=(const iterator& other) const { return ! (*this == other); }

		inline void operator++(int) 
		{	
			_ASSERT(myListIndex >= 0 && myListIndex <= myParentMap->myNumMemElems);
			myListIndex = myParentMap->myMemory[myListIndex].myNextIndex;
			ensureValidIterator(); 
		}

		void ensureValidIterator()
		{
			if(myBucketIndex >= NUM_BUCKETS || !myParentMap)
				return;

			while(myListIndex == -1 
				|| (myListIndex >= 0 && myListIndex < myParentMap->myNumMemElems && !myParentMap->myMemory[myListIndex].myIsValid))
			{
				myBucketIndex++;
				// We've gone past the last bucket. Set us to end.
				if(myBucketIndex >= NUM_BUCKETS)
				{
					this->toEnd();
					break;
				}
				myListIndex = myBucketIndex;
			}
		}

		inline VALUE_TYPE& getValue() const { return myParentMap->myMemory[myListIndex].myValue; }
		inline KEY_TYPE getKey() const { if(isEnd()) return (KEY_TYPE)0; else return myParentMap->myMemory[myListIndex].myKey; }

	private:
		HASH_BUCKET_INDEX_TYPE myBucketIndex;
		int myListIndex;
		HashMap* myParentMap;
	};

	inline iterator begin() const { iterator  rTemp(*this); return rTemp; }
	inline iterator end() const { iterator rTemp(*this); rTemp.toEnd(); return rTemp; }

	int getMemoryUsed() const
	{
		int iTotal = 0;
		iterator mi;
		for(mi = begin(); mi != end(); mi++)
			iTotal += sizeof(HashPair);

		iTotal += sizeof(HashMap);
		if(myFreedMemSlots)
			iTotal += sizeof(int)*myFreedMemSlots->size();

		return iTotal;
	}


protected:

	inline void allocate()
	{
		_ASSERT(myMemory == NULL);
		myNextFreeMemPos = NUM_BUCKETS;
		myNumMemElems = NUM_BUCKETS*4;
		myMemory = new HashPair[myNumMemElems];
		memset(myBucketTailPositions, 0, sizeof(int)*NUM_BUCKETS);
	}

	inline void deallocate()
	{
		delete[] myMemory;
		myMemory = NULL;
		myNumMemElems = 0;

		delete myFreedMemSlots;
		myFreedMemSlots = NULL;
	}

private:

	// String hashing
	inline HASH_BUCKET_INDEX_TYPE hashAndBucket(const char* a) const
	{
		_ASSERT(a);
		if(!a)
			return 0;

		ITERATOR_COUNT_TYPE iPos;
		size_t hash = 2166136261U;
		for(iPos = 0; a[iPos]; iPos++)
		{
			// xor  the low 8 bits
			hash = hash ^ (a[iPos]);
			// multiply by the magic number
			hash = hash * 16777619;
		}

		return hash % NUM_BUCKETS;
	}

	inline HASH_BUCKET_INDEX_TYPE hashAndBucket(const wchar_t* a) const
	{
		_ASSERT(a);

		ITERATOR_COUNT_TYPE iPos;
		size_t hash = 2166136261U;
		for(iPos = 0; a[iPos]; iPos++)
		{
			// xor  the low 8 bits
			hash = hash ^ (a[iPos]);
			// multiply by the magic number
			hash = hash * 16777619;
		}
		return hash % NUM_BUCKETS;
	}

	inline HASH_BUCKET_INDEX_TYPE hashAndBucket(int a) const
	{
		a = (a ^ 61) ^ (a >> 16);
		a = a + (a << 3);
		a = a ^ (a >> 4);
		a = a * 0x27d4eb2d;
		a = a ^ (a >> 15);
		return a & (NUM_BUCKETS - 1);
	}

	inline HASH_BUCKET_INDEX_TYPE hashAndBucket(unsigned int a) const
	{
		return hashAndBucket((int)a);
	}

	inline HASH_BUCKET_INDEX_TYPE hashAndBucket(unsigned long a) const
	{
#ifdef WIN32
		_ASSERT(sizeof(unsigned long) == 4);
        _ASSERT(sizeof(unsigned long) == sizeof(int));
		return hashAndBucket((unsigned int)a);
#else
		_ASSERT(sizeof(unsigned long) == 8);
        _ASSERT(sizeof(unsigned long) == sizeof(long long));
		return hashAndBucket((long long)a);
#endif
	}

	inline HASH_BUCKET_INDEX_TYPE hashAndBucket(long long key) const
	{
		key += ~(key << 32);
		key ^= (key >> 22);
		key += ~(key << 13);
		key ^= (key >> 8);
		key += (key << 3);
		key ^= (key >> 15);
		key += ~(key << 27);
		key ^= (key >> 31);
		return key % NUM_BUCKETS;
	}

private:

	HashPair* myMemory;
	int myNumMemElems;

	// Holds the index of the last element
	// for each of the buckets.
	int myBucketTailPositions[NUM_BUCKETS];

	// Holds the next free memory position
	// at the end of all data, even if the data
	// includes free spaces.
	int myNextFreeMemPos;

	// Holds the list of freed mem slots
	std::vector < int > *myFreedMemSlots;
};
/********************************************************************************************/
template < class KEY_TYPE, class KEY_STORAGE_TYPE, class VALUE_TYPE, int NUM_BUCKETS >
void clearAndDeleteContainer(HashMap<KEY_TYPE, KEY_STORAGE_TYPE, VALUE_TYPE, NUM_BUCKETS> &rContainer)
{
	typename HashMap<KEY_TYPE, KEY_STORAGE_TYPE, VALUE_TYPE, NUM_BUCKETS>::iterator it;
	for(it = rContainer.begin(); it != rContainer.end(); it++)
		delete it.getValue();
	rContainer.clear();
}
/********************************************************************************************/
