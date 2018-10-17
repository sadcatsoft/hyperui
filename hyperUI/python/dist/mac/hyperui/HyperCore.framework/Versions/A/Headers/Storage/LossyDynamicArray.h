#pragma once

// Lossy in that it does not preserve array contents when reallocating
/*****************************************************************************/
template < class TYPE >
class TLossyDynamicArray
{
public:
	TLossyDynamicArray() 
	{ 
		myArray = NULL; 
		myCurrCapacity = 0; 
	}

	~TLossyDynamicArray() 
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

		// Reallocate
		delete[] myArray;
		myArray = new TYPE[iCapacity];
		myCurrCapacity = iCapacity;
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
	TLossyDynamicArray<TYPE> operator=(const TLossyDynamicArray<TYPE>& rOther) { _ASSERT(0); }

private:
	TYPE* myArray;
	int myCurrCapacity;
};
/*****************************************************************************/
typedef TLossyDynamicArray < double > TDoubleLossyDynamicArray;
typedef TLossyDynamicArray < FLOAT_TYPE > TFloatTypeLossyDynamicArray;
typedef TLossyDynamicArray < unsigned char > TUCharLossyDynamicArray;
typedef TLossyDynamicArray < char > TCharLossyDynamicArray;
typedef TLossyDynamicArray < int64 > TInt64LossyDynamicArray;
typedef TLossyDynamicArray < int > TIntLossyDynamicArray;
/*****************************************************************************/
