#pragma once

/*****************************************************************************/
// A templated class to hold an array of fixed size. Useful for using MemoryCacher with
// arrays of objects.
// That is, each "object" in the memory cacher is actually an array of size ARRAY_SIZE
// of ARRAY_TYPE objects.
template < class ARRAY_TYPE, int ARRAY_SIZE >
class TArrayHolder
{
public:
	TArrayHolder() { myMemId = -1; }
	inline void onAllocated(IBaseObject* pData) { }
	inline void setMemoryBlockId(long rMemId) { myMemId = rMemId; }
	inline long getMemoryBlockId() const { return myMemId; }

	ARRAY_TYPE myArray[ARRAY_SIZE];
	long myMemId;
};
/*****************************************************************************/
template < class T >
class MemoryCacher
{
public:
	MemoryCacher(int iBlockSize)
		: myBlockSize(iBlockSize)
	{
		_ASSERT(iBlockSize > 0);
		//myBlockSize = iBlockSize;
	}

	~MemoryCacher()
	{
		// When deallocated a mem cacher,
		// it *MUST* be completely unused.
		// Otherwise, we're risking leaving
		// deallocated memory to our users,
		// which will cause a crash.
		// Note that in rare cases for on-the-stack
		// memory cachers, it might be ok to destroy it
		// without return all memory. In that case,
		// call markAllAsUnused() first.
		_ASSERT(myFreeObjects.size() == myObjectArrays.size()*myBlockSize);

		// Deallocate everything
		int iCurr, iNum = myObjectArrays.size();
		for(iCurr = 0; iCurr < iNum; iCurr++)
			delete[] myObjectArrays[iCurr];
		myObjectArrays.clear();
		myFreeObjects.clear();
	}

	void markAllAsUnused()
	{
		myFreeObjects.clear();

		int iCurrBlock, iNum = myObjectArrays.size();

		long lCombId;
		int iCurr;

		myFreeObjects.clear();
		myFreeObjects.reserve(myBlockSize*iNum);		

		for(iCurrBlock = 0; iCurrBlock < iNum; iCurrBlock++)
		{
			lCombId = iCurrBlock*myBlockSize;
			for(iCurr = 0; iCurr < myBlockSize; iCurr++, lCombId++)
				myFreeObjects.push_back(lCombId);
		}		
	}

	T* getNewObject(IBaseObject* pData = NULL)
	{
		T* pRes;
		long lCombId;
		if(myFreeObjects.size() <= 0)
		{
			// We have no more free objects left! Allocate a new one.
			T* pNewBlock = new T[myBlockSize];
			int iBlockId = myObjectArrays.size();
			myObjectArrays.push_back(pNewBlock);

			myFreeObjects.reserve(myFreeObjects.size() + myBlockSize);

			lCombId = iBlockId*myBlockSize;
			int iCurr;
			for(iCurr = 0; iCurr < myBlockSize; iCurr++, lCombId++)
			{				
				myFreeObjects.push_back(lCombId);
				pNewBlock[iCurr].setMemoryBlockId(lCombId);
			}
		}

		_ASSERT(myFreeObjects.size() > 0);

		lCombId = myFreeObjects[myFreeObjects.size() - 1];
		int iBlockId = lCombId/myBlockSize;
		int iObjId = lCombId - iBlockId*myBlockSize;
		pRes = &(myObjectArrays[iBlockId][iObjId]);
		myFreeObjects.pop_back();


		// Reset it:
		pRes->onAllocated(pData);
		return pRes;
	}

	inline void deleteObjectByMemoryIdDontCallOnDeallocate(long lObjMemBlockId)
	{
		_ASSERT(lObjMemBlockId >= 0);
		_ASSERT(myFreeObjects.size() == 0 || std::find(myFreeObjects.begin(), myFreeObjects.end(), lObjMemBlockId) == myFreeObjects.end());
		_ASSERT(myFreeObjects.size() < myObjectArrays.size()*myBlockSize);
		// Simply append it to the free blocks list:
		myFreeObjects.push_back(lObjMemBlockId);
	}

	inline void deleteObject(T* pObject)
	{
		long lObjMemBlockId = pObject->getMemoryBlockId();
		_ASSERT(lObjMemBlockId >= 0);
		_ASSERT(myFreeObjects.size() == 0 || std::find(myFreeObjects.begin(), myFreeObjects.end(), lObjMemBlockId) == myFreeObjects.end());
		_ASSERT(myFreeObjects.size() < myObjectArrays.size()*myBlockSize);
		// Simply append it to the free blocks list:
		myFreeObjects.push_back(lObjMemBlockId);

		pObject->onDeallocated();
	}

	inline void getStats(int &iUsedObjects, int& iFreeObjects) const
	{
		iFreeObjects = myFreeObjects.size();
		iUsedObjects = myObjectArrays.size()*myBlockSize;
	}


private:

	const int myBlockSize;

	vector < T* > myObjectArrays;
	vector < long > myFreeObjects;
};
/*****************************************************************************/
struct SMemStringWrapper
{
	inline void setMemoryBlockId(long lid) { myMemId = lid; }
	inline long getMemoryBlockId() const { return myMemId; }
	inline void onAllocated(IBaseObject* pData) { }
	inline void onAllocated() { }

	long myMemId;
	STRING_TYPE myString;
};
typedef MemoryCacher < SMemStringWrapper > TStringMemoryCacher;
/*****************************************************************************/