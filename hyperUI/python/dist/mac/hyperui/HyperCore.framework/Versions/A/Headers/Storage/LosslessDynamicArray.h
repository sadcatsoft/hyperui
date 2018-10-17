#pragma once

// Lossy in that it does not preserve array contents when reallocating
/*****************************************************************************/
template < class TYPE >
class TLosslessDynamicArray
{
public:
	TLosslessDynamicArray() 
	{ 
		myArray = NULL; 
		myCurrCapacity = 0; 
	}

	~TLosslessDynamicArray() 
	{ 
		delete[] myArray; 
		myArray = NULL; 
		myCurrCapacity = 0; 
	}

	inline const TYPE* getArray() const 
	{ 
		return myArray; 
	}

	inline TYPE* getArray() 
	{ 
		return myArray; 
	}

	inline TYPE* getArray(int iCapacity) 
	{ 
		ensureCapacity(iCapacity); 
		return myArray; 
	}

	inline void ensureCapacity(int iCapacity)
	{
		if(iCapacity <= myCurrCapacity)
			return;

		// Reallocate, but copy previous contents over
		TYPE* pOldArray = myArray;
		
		myArray = new TYPE[iCapacity];
		if(pOldArray)
			memcpy(myArray, pOldArray, sizeof(TYPE)*myCurrCapacity);

		// Zero out the rest, since we might be dealing with pointers
		memset(myArray + myCurrCapacity, 0, sizeof(TYPE)*(iCapacity - myCurrCapacity));

		myCurrCapacity = iCapacity;
		delete[] pOldArray;
	}

	inline int getCurrentCapacity() const 
	{ 
		return myCurrCapacity; 
	}

	// This disowns the memory we've allocated. BE CAREFUL - if this
	// pointer is not owned by anyone else, it will be leaked!
	inline void disownMemory() { myArray = NULL; myCurrCapacity = 0;}

	inline TYPE& operator[](int iIndex)
	{
		_ASSERT(iIndex >= 0 && iIndex < myCurrCapacity);
		return myArray[iIndex];
	}

	inline const TYPE& operator[](int iIndex) const
	{
		_ASSERT(iIndex >= 0 && iIndex < myCurrCapacity);
		return myArray[iIndex];
	}

	inline HUGE_SIZE_TYPE getMemoryUsage() const 
	{ 
		return sizeof(TYPE)*myCurrCapacity; 
	}

	HUGE_SIZE_TYPE estimateMemoryUsageAt(FLOAT_TYPE fTargetPerfScale) const 
	{ 
		return getMemoryUsage()*fTargetPerfScale; 
	}

private:
	// No equals
	TLosslessDynamicArray<TYPE> operator=(const TLosslessDynamicArray<TYPE>& rOther) { _ASSERT(0); }

private:
	TYPE* myArray;
	int myCurrCapacity;
};
/*****************************************************************************/
typedef TLosslessDynamicArray < double > TDoubleLosslessDynamicArray;
typedef TLosslessDynamicArray < FLOAT_TYPE > TFloatTypeLosslessDynamicArray;
typedef TLosslessDynamicArray < unsigned char > TUCharLosslessDynamicArray;
typedef TLosslessDynamicArray < char > TCharLosslessDynamicArray;
typedef TLosslessDynamicArray < int64 > TInt64LosslessDynamicArray;
typedef TLosslessDynamicArray < int > TIntLosslessDynamicArray;
/*****************************************************************************/
