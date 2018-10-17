#pragma once

#ifdef _DEBUG
//#define TRACK_STRING_STATS
#endif

#define MAX_STATIC_BUFFER_LEN		16
/*****************************************************************************/
class HYPERCORE_API SString
{
public:
	explicit SString()
	{
		init();
	}

	SString(const SString& strOther)
	{
		init();
		copyFromString(strOther.c_str(), strOther.length());
	}

	SString(const CHAR_TYPE* str)
	{
		init();
		copyFromString(str, STRLEN(str));
	}

	~SString()
	{
		if(myDynamicBuffer)
			delete[] myDynamicBuffer;
		myDynamicBuffer = NULL;
		myLength = 0;
	}

	friend inline bool operator == (const SString& lhs, const CHAR_TYPE* rhs) 
	{
		assert (rhs);
		return (STRLEN(rhs) == lhs.myLength) && (STRCMP(lhs.c_str(), rhs) == 0);
	}

	friend inline bool operator== (const CHAR_TYPE* lhs, const SString& rhs) 
	{
		assert (lhs);
		return (STRLEN(lhs) == rhs.myLength) && (STRCMP(rhs.c_str(), lhs) == 0);
	}
	friend inline bool operator== (const SString& lhs, const SString& rhs) 
	{ 
		return (lhs.myLength == rhs.myLength) && (STRCMP(rhs.c_str(), lhs.c_str()) == 0);
	}


	CHAR_TYPE&       operator[] (int pos)       { return (c_str()[pos]); }
	const CHAR_TYPE&		operator[] (int pos) const { return (c_str()[pos]); }

	inline operator const CHAR_TYPE*() const { return c_str(); }
	inline const CHAR_TYPE* c_str() const 
	{ 
		if(myDynamicBuffer)
			return myDynamicBuffer;
		else
			return myStaticBuffer;
	} 
	inline CHAR_TYPE* c_str()
	{ 
		if(myDynamicBuffer)
			return myDynamicBuffer;
		else
			return myStaticBuffer;
	} 

	inline int length() const { return myLength; }

	inline SString& operator=(const CHAR_TYPE* str) { return copyFromString(str, STRLEN(str)); }
	inline SString& operator=(const SString& str){  return copyFromString(str.c_str(), str.length()); }

	friend inline bool operator<  (const SString& lhs, const SString& rhs) 
	{ 
		return STRCMP(lhs.c_str(), rhs.c_str()) < 0; 
	}
	friend inline bool operator<  (const SString& lhs, const CHAR_TYPE* rhs) 
	{
		assert (rhs);
		return STRCMP(lhs.c_str(), rhs) < 0; 
	}
	friend inline bool operator<  (const CHAR_TYPE* lhs, const SString& rhs) 
	{
		assert (lhs);
		return STRCMP(lhs, rhs.c_str()) < 0; 
	}

	friend inline bool operator!= (const SString& lhs, const SString& rhs) { return !(lhs == rhs); }
	friend inline bool operator!= (const CHAR_TYPE* lhs,   const SString& rhs) { return !(lhs == rhs); }
	friend inline bool operator!= (const SString& lhs, const CHAR_TYPE* rhs)   { return !(lhs == rhs); }

	friend inline bool operator>  (const SString& lhs, const SString& rhs) { return rhs < lhs; }
	friend inline bool operator>  (const SString& lhs, const CHAR_TYPE* rhs)   { return rhs < lhs; }
	friend inline bool operator>  (const CHAR_TYPE* lhs,   const SString& rhs) { return rhs < lhs; }
	friend inline bool operator<= (const SString& lhs, const SString& rhs) { return !(rhs < lhs); }
	friend inline bool operator<= (const SString& lhs, const CHAR_TYPE* rhs)   { return !(rhs < lhs); }
	friend inline bool operator<= (const CHAR_TYPE* lhs,   const SString& rhs) { return !(rhs < lhs); }
	friend inline bool operator>= (const SString& lhs, const SString& rhs) { return !(lhs < rhs); }
	friend inline bool operator>= (const SString& lhs, const CHAR_TYPE* rhs)   { return !(lhs < rhs); }
	friend inline bool operator>= (const CHAR_TYPE* lhs,   const SString& rhs) { return !(lhs < rhs); }

#ifdef TRACK_STRING_STATS
	static void getStats(float& fAvgOut, int& iMaxOut) 
	{
		iMaxOut = theMaxLength;
		if(theNumLengths > 0)
			fAvgOut = (double)theTotalLengths/(double)theNumLengths;
		else
			fAvgOut = 0;
	}
#endif

private:

	inline void init()
	{
		myLength = 0;
		myDynamicBuffer = NULL;
		myDynamicBufferSize = 0;
		myStaticBuffer[0] = 0;
	}

	inline SString& copyFromString(const CHAR_TYPE* s, int iLength)
	{
		assert(s); 
		if(!myDynamicBuffer && iLength < MAX_STATIC_BUFFER_LEN)
		{
			// We go into static buffer
			memcpy(myStaticBuffer, s, sizeof(CHAR_TYPE)*iLength);
			myStaticBuffer[iLength] = 0;
		}
		else
		{
			// We go into dynamic buffer
			if(iLength >= myDynamicBufferSize)
			{
				delete[] myDynamicBuffer;
				myDynamicBufferSize = iLength + 1;
				myDynamicBuffer = new CHAR_TYPE[myDynamicBufferSize];
			}

			memcpy(myDynamicBuffer, s, sizeof(CHAR_TYPE)*iLength);
			myDynamicBuffer[iLength] = 0;
		}
		myLength = iLength;
#ifdef TRACK_STRING_STATS
		theTotalLengths += myLength;
		theNumLengths++;
		if(myLength > theMaxLength)
			theMaxLength = myLength;
#endif
		return *this; 
	}

	SString& operator+= (const SString& str) { _ASSERT(0); return *this; }
	SString& operator+= (const CHAR_TYPE* s)     { _ASSERT(0);  return *this; }
	SString& operator+= (CHAR_TYPE c)            { _ASSERT(0);  return *this; }

private:

	int myLength;
	CHAR_TYPE myStaticBuffer[MAX_STATIC_BUFFER_LEN];
	CHAR_TYPE* myDynamicBuffer;
	int myDynamicBufferSize;

#ifdef TRACK_STRING_STATS
	static long long theTotalLengths;
	static long long theNumLengths;
	static int theMaxLength;
#endif
};
/*****************************************************************************/

