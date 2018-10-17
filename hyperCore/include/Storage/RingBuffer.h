#pragma once

/*****************************************************************************/
template < class T >
class RingBuffer
{
public:
	RingBuffer(int iInitCapacity = 32)
	{
		myBuffer = NULL;
		allocate(iInitCapacity);
	}

	~RingBuffer()
	{
		allocate(0);
	}

	inline void push_back(T& pItem)
	{
		if(mySize >= myCapacity)
		{
			// We need to reallocate the whole damned thing.
			// We might as well shift it to the beginning.
			T* pOldArray = myBuffer;
			int iNewCap = myCapacity*2;
			myBuffer = new T[iNewCap];

			int iEnd1 = myStart + (mySize - 1);
			if(iEnd1 >= myCapacity)
				iEnd1 = myCapacity - 1;

			if(mySize > 0)
				memcpy(myBuffer, pOldArray + myStart, sizeof(T)*(iEnd1 - myStart + 1));

			int iEnd2 = mySize - (iEnd1 - myStart + 1) - 1;
			if(iEnd2 > 0)
			{
				memcpy(myBuffer + (iEnd1 - myStart + 1), pOldArray, sizeof(T)*(iEnd2 + 1));
			}

			myStart = 0;
			myEnd = mySize;
			myCapacity = iNewCap;

			delete[] pOldArray;
		}

		_ASSERT(mySize < myCapacity);

		myBuffer[myEnd] = pItem;

		// Now add the actual darned item...
		myEnd++;
		if(myEnd == myCapacity)
			myEnd = 0;

		mySize++;
	}

	inline int getCapacity() const
	{
		return myCapacity;
	}

	void eraseFront(int iNum)
	{
		_ASSERT(mySize >= iNum);

		myStart+= iNum;
		if(myStart >= myCapacity)
			myStart -= myCapacity;

		mySize -= iNum;
		if(mySize <= 0)
		{
			if(mySize < 0)
			{ _ASSERT(0); }

			clear();
		}
	}

	void clear()
	{
		myStart = myEnd = 0;
		mySize = 0;
	}

	inline int size() const
	{
		return mySize;
	}

	inline T& operator[](int iIndex)
	{
		// Do it relative to our stand and end   
		if(iIndex < 0 || iIndex >= mySize)
		{
			_ASSERT(0);
			return myBuffer[0];
		}
		else
		{
			return myBuffer[(iIndex + myStart) % myCapacity];
		}
	}


	inline T* debugInfo(int& iStartOut, int& iEndOut) const
	{
		iStartOut = myStart;
		iEndOut = myEnd;
		return myBuffer;
	}

	inline void operator=(const RingBuffer<T>& rOther)
	{
		this->clear();
		this->allocate(rOther.myCapacity);
		this->myStart = rOther.myStart;
		this->myEnd = rOther.myEnd;
		memcpy(this->myBuffer, rOther.myBuffer, sizeof(T)*myCapacity);
	}

private:

	inline void allocate(int iCapacity)
	{
		if(myBuffer)
			delete[] myBuffer;
		myBuffer = NULL;

		if(iCapacity > 0)
			myBuffer = new T[iCapacity];

		myCapacity = iCapacity;
		mySize = 0;
		myStart = myEnd = 0;
	}

private:

	// The points one past the last element
	int myStart, myEnd;

	T* myBuffer;
	int myCapacity;
	int mySize;
};
/*****************************************************************************/
