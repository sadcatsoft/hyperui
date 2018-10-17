#pragma once

#define REC_LOCK_FAST_ID

#ifdef REC_LOCK_FAST_ID

#ifdef WIN32
	#define NATIVE_THREAD_TYPE		unsigned int
#else
	#define NATIVE_THREAD_TYPE		unsigned long
#endif
//typedef map < NATIVE_THREAD_TYPE, int > TThreadIdIntMap;
typedef HashMap < NATIVE_THREAD_TYPE, NATIVE_THREAD_TYPE, int, 32 > TThreadIdIntMap;
/*****************************************************************************/
class RecursiveSharedLock
{
public:

	HYPERCORE_API void lock() const;
	HYPERCORE_API void unlock() const;
	HYPERCORE_API void lock_shared() const;
	HYPERCORE_API void unlock_shared() const;

	inline bool getIsLocked() const { return didLockAlready(); }

	static NATIVE_THREAD_TYPE getThreadId();

protected:

	HYPERCORE_API bool didLockAlready() const;
	HYPERCORE_API void changeLockCount(bool bIncrement) const;

private:

	mutable TThreadIdIntMap myThreadLockCountMap;
#ifdef USE_BOOST_LOCKS
	mutable boost::shared_mutex myMapLock;
	mutable boost::shared_mutex mySharedLock;
#else
	mutable Mutex myMapLock;
	mutable Mutex mySharedLock;
#endif
};
/*****************************************************************************/
#else
fdafd
/*****************************************************************************/
class RecursiveSharedLock
{
public:

	void lock() const;
	void unlock() const;
	void lock_shared() const;
	void unlock_shared() const;

	inline bool getIsLocked() const { return didLockAlready(); }

protected:

	void getThreadId(string& strOut) const;
	bool didLockAlready() const;
	void changeLockCount(bool bIncrement) const;

private:

	mutable boost::shared_mutex mySharedLock;
	mutable std::ostringstream myTempStream;
	mutable string myTempString;

	mutable TStringIntMap myThreadLockCountMap;
	mutable boost::recursive_mutex myMapLock;
};
/*****************************************************************************/
#endif