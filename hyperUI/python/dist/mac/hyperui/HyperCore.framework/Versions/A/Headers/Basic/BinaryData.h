#pragma once

#ifdef WINDOWS
typedef long INT4;
#else
typedef int INT4;
#endif

/*****************************************************************************/
class BinaryData
{
public:
	HYPERCORE_API BinaryData();
	BinaryData(char* pcsData, int iLength, bool bDoOwn) { myData = NULL; setData(pcsData, iLength, bDoOwn); }
	HYPERCORE_API ~BinaryData();

	// Read functions
	template < class TYPE >
	void readChars(int iNum, TYPE* pcsBuffOut) const
	{
		// Check for going over mem chunk
		int iReadSize = sizeof(TYPE)*iNum;
		if(myReadPosition + iReadSize > myLength)
			return;
		memcpy(pcsBuffOut, myData + myReadPosition, iReadSize);
		myReadPosition += iReadSize;
	}

	template < class TYPE > inline void read(TYPE& rDataOut) const	{ rDataOut = read<TYPE>(); }

	template < class TYPE >
	TYPE read() const
	{
		TYPE rData;
		int iReadSize = sizeof(TYPE);
		if(myReadPosition + iReadSize > myLength)
			return 0;

		memcpy(&rData, myData + myReadPosition, iReadSize);
		myReadPosition += iReadSize;

		if(myDoReverseByteOrder)
			reverseByteOrder(rData);

		return rData;
	}

	char* readString()
	{
		// First, we read its size
		int iFullSizeWithZero = 0;
		read<int>(iFullSizeWithZero);

		// Now, read the bytes:
		long lReadPos = myReadPosition;
		myReadPosition += iFullSizeWithZero;
		return (myData + lReadPos);
	}

	template < class TYPE >
	bool write(TYPE rDataIn)
	{
		int iWriteSize = sizeof(TYPE);
		if(myWritePosition + iWriteSize > myLength)
		{
			if(myAllowGrowingData)
				ensureSizeFor(iWriteSize);
			else
				return false;
		}

		if(myDoReverseByteOrder)
			reverseByteOrder(rDataIn);

		memcpy(myData + myWritePosition, &rDataIn, iWriteSize);
		myWritePosition += iWriteSize;
		return true;
	}

	template < class TYPE >
	bool writeRepeatedValue(TYPE rValueIn, int iNumRepetitions)
	{
		int iCurr;
		bool bRes = true;
		for(iCurr = 0; iCurr < iNumRepetitions; iCurr++)
			bRes &= write<TYPE>(rValueIn);

		return bRes;
	}

	template < class TYPE >
	bool writeChars(TYPE* pcsBuff, int iNumChars)
	{
		// Check for going over mem chunk
		int iWriteSize = sizeof(TYPE)*iNumChars;
		if(myWritePosition + iWriteSize > myLength)
		{
			if(myAllowGrowingData)
				ensureSizeFor(iWriteSize);
			else
				return false;
		}

		memcpy(myData + myWritePosition, pcsBuff, iWriteSize);
		myWritePosition += iWriteSize;
		return true;
	}

	// Note the +1 to account for the terminal 0.
	inline bool writeString(const string& strIn) 
	{ 
		int iTotalSize = strIn.length() + 1;
		bool bRes = write<int>(iTotalSize);
		bRes &= writeChars<const char>(strIn.c_str(), iTotalSize); 
		return bRes;
	}

	inline bool writeString(const char* pString) 
	{ 
		int iTotalSize = strlen(pString) + 1;
		bool bRes = write<int>(iTotalSize);
		bRes &= writeChars<const char>(pString, iTotalSize); 
		return bRes;
	}

	inline bool doHaveEnoughSpaceFor(int iChunkSize)
	{
		if(myWritePosition + iChunkSize> myLength)
			return false;
		else
			return true;
	}

	// Position functions
	inline bool isValid() const { return myData && myLength > 0; }
	inline bool isAtEnd() const { return (myReadPosition >= myLength) || myReadPosition < 0; }
	inline long getReadPosition() const { return myReadPosition; }
	inline void resetReadPosition() const { myReadPosition = 0; }
	inline void resetWritePosition() const { myWritePosition = 0; }
	inline void changeReadPositionBy(long lDiff) const { myReadPosition += lDiff; }
	inline void setReadPositionFromOrigin(long lDiff) const { myReadPosition = lDiff; }
	inline void setReadPositionFromEnd(long lDiff) const { myReadPosition = myLength - lDiff; }

	inline void setAllowGrowingData(bool bValue) { _ASSERT(myDoOwnData || !myData); myAllowGrowingData = bValue; myDoOwnData = true; }

	inline void setWritePositionFromOrigin(long lDiff) const { myWritePosition = lDiff; }
	inline long getWritePosition() const { return myWritePosition; }

	// Utility byte functions
	inline void setDoReverseByteOrder(bool bValue) { myDoReverseByteOrder = bValue; }

	static inline void reverseByteOrder(char& x) 
	{ 
		// Nothing - can't reverse a single byte.
	}

	static inline void reverseByteOrder(unsigned char& x) 
	{ 
		// Nothing - can't reverse a single byte.
	}

	static void reverseByteOrder(int& x)
	{
		STATIC_ASSERT(sizeof(int) == 4);

		char bytes[4];
		bytes[0] = (x >> 24) & 0xFF;
		bytes[1] = (x >> 16) & 0xFF;
		bytes[2] = (x >>  8) & 0xFF;
		bytes[3] =  x        & 0xFF;

		x = *((int*)bytes);
	}

	// On Mac 64-bit, a long is 8 bytes.
	// We use int intsead, then...
#ifdef WINDOWS
	static void reverseByteOrder(long& x)
	{
		STATIC_ASSERT(sizeof(long) == 4);

		char bytes[4];
		bytes[0] = (x >> 24) & 0xFF;
		bytes[1] = (x >> 16) & 0xFF;
		bytes[2] = (x >>  8) & 0xFF;
		bytes[3] =  x        & 0xFF;

		x = *((int*)bytes);
	}
#endif

	static void reverseByteOrder(short& x)
	{
		STATIC_ASSERT(sizeof(short) == 2);

		char bytes[2];
		bytes[0] = (x >> 8) & 0xFF;
		bytes[1] = (x) & 0xFF;

		x = *((short*)bytes);
	}

	static void reverseByteOrder(unsigned short& x)
	{
		STATIC_ASSERT(sizeof(unsigned short) == 2);

		char bytes[2];
		bytes[0] = (x >> 8) & 0xFF;
		bytes[1] = (x) & 0xFF;

		x = *((unsigned short*)bytes);
	}

	static void reverseByteOrder(long long& x)
	{
		STATIC_ASSERT(sizeof(long long) == 8);

		char bytes[8];
		bytes[0] = (x >> 56) & 0xFF;
		bytes[1] = (x >> 48) & 0xFF;
		bytes[2] = (x >> 40) & 0xFF;
		bytes[3] = (x >> 32) & 0xFF;
		bytes[4] = (x >> 24) & 0xFF;
		bytes[5] = (x >> 16) & 0xFF;
		bytes[6] = (x >>  8) & 0xFF;
		bytes[7] =  x        & 0xFF;

		x = *((long long*)bytes);
	}

	static void reverseByteOrder(double& xd)
	{
		STATIC_ASSERT(sizeof(double) == 8);

		void* v = (void*)&xd;

		char    in[8], out[8];
		memcpy(in, v, 8);
		out[0] = in[7];
		out[1] = in[6];
		out[2] = in[5];
		out[3] = in[4];
		out[4] = in[3];
		out[5] = in[2];
		out[6] = in[1];
		out[7] = in[0];
		memcpy(v, out, 8);
	}

	template < class TYPE >
	bool testAndSetEndianness(TYPE rExpectedValue, bool bResetReadPosition)
	{
		bool bDidPassTest = false;
		long lPrevReadPos = myReadPosition;
		TYPE sTest;
		read(sTest);
		if(sTest != rExpectedValue)
		{
			myDoReverseByteOrder = true;

			// Test again:
			TYPE sNewVal = sTest;
			reverseByteOrder(sNewVal);
			bDidPassTest = (sNewVal == rExpectedValue);
		}
		else
		{
			myDoReverseByteOrder = false;
			bDidPassTest = true;
		}

		if(bResetReadPosition)
			myReadPosition = lPrevReadPos;

		return bDidPassTest;
	}

	HYPERCORE_API void setData(char* pcsData, int iLength, bool bDoOwn);
	HYPERCORE_API void clear();

	inline int getLength() const { return myLength; }
	inline char* getData() { return myData; }
	inline const char* getData() const { return myData; }

	inline void replaceDataPointer(char* pcsNewPointer) { myData = pcsNewPointer; }

	inline void disownMemory() { myDoOwnData = false; }

	inline bool getDoReverseByteOrder() const { return myDoReverseByteOrder; }

	HYPERCORE_API void ensureSizeFor(int iExtraSize);
	HYPERCORE_API void ensureSizeMultipleOf(int iMultiple);

private:

	char* myData;
	int myLength;

	bool myDoOwnData;
	bool myDoReverseByteOrder;

	mutable long myReadPosition;
	mutable long myWritePosition;

	bool myAllowGrowingData;
};
/*****************************************************************************/