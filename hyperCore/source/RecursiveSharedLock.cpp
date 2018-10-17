#include "stdafx.h"

#if (defined(MAC_BUILD) || defined(LINUX)) && !defined(WIN32)
#include <pthread.h>
#endif

//#define DEBUG_LOCKING

namespace HyperCore
{
#ifdef REC_LOCK_FAST_ID
/*****************************************************************************/
NATIVE_THREAD_TYPE RecursiveSharedLock::getThreadId()
{
#ifdef USE_BOOST_LOCKS
	// Hack. Cast this to a pointer
	boost::thread::id lId = boost::this_thread::get_id();
	return *((NATIVE_THREAD_TYPE*)(&lId));
#elif (defined(MAC_BUILD) || defined(LINUX)) && !defined(WIN32)
    uint64_t threadId;
    pthread_threadid_np(NULL, &threadId);
    return (NATIVE_THREAD_TYPE)threadId;
#else
	//  Other platforms
	return (NATIVE_THREAD_TYPE)GetCurrentThreadId();
#endif
}
/*****************************************************************************/
void RecursiveSharedLock::lock() const
{ 
#ifdef DEBUG_LOCKING
	gLog("REC_LOCK: write lock request\n");
#endif

	if(!didLockAlready())
		mySharedLock.lock(); 
	changeLockCount(true);
}
/*****************************************************************************/
void RecursiveSharedLock::unlock() const  
{ 
#ifdef DEBUG_LOCKING
	gLog("REC_LOCK: write unlock request\n");
#endif
	changeLockCount(false);
	if(!didLockAlready())
		mySharedLock.unlock();  
}
/*****************************************************************************/
void RecursiveSharedLock::lock_shared() const  
{ 
#ifdef DEBUG_LOCKING
	gLog("REC_LOCK: read lock request\n");
#endif
	if(!didLockAlready())
#ifdef USE_BOOST_LOCKS
		mySharedLock.lock_shared(); 
#else
		mySharedLock.lock(); 
#endif
	changeLockCount(true);
}
/*****************************************************************************/
void RecursiveSharedLock::unlock_shared() const   
{ 
#ifdef DEBUG_LOCKING
	gLog("REC_LOCK: read unlock request\n");
#endif
	changeLockCount(false);
	if(!didLockAlready())
#ifdef USE_BOOST_LOCKS
		mySharedLock.unlock_shared(); 
#else
		mySharedLock.unlock(); 
#endif
}
/*****************************************************************************/
bool RecursiveSharedLock::didLockAlready() const
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	NATIVE_THREAD_TYPE lThreadId = getThreadId();
#ifdef DEBUG_LOCKING
	gLog("REC_LOCK: thread %ux didLockAlready = %s\n", lThreadId, (myThreadLockCountMap.find(lThreadId) != NULL) ? "YES" : "NO" );
//	gLog("REC_LOCK: thread %ux didLockAlready = %s\n", lThreadId, (myThreadLockCountMap.find(lThreadId) != myThreadLockCountMap.end()) ? "YES" : "NO" );
#endif

#ifdef USE_BOOST_LOCKS
	//return myThreadLockCountMap.find(lThreadId) != myThreadLockCountMap.end();
	myMapLock.lock_shared();
#else
	myMapLock.lock();
#endif
	bool bDidLock = myThreadLockCountMap.find(lThreadId) != NULL;
#ifdef USE_BOOST_LOCKS
	myMapLock.unlock_shared();
#else
	myMapLock.unlock();
#endif
	return bDidLock;
}
/*****************************************************************************/
void RecursiveSharedLock::changeLockCount(bool bIncrement) const
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	NATIVE_THREAD_TYPE lThreadId = getThreadId();
	//TThreadIdIntMap::iterator mi = myThreadLockCountMap.find(lThreadId);
	//if(mi == myThreadLockCountMap.end())
	myMapLock.lock();
	int *pCount = myThreadLockCountMap.find(lThreadId);
	if(!pCount)
	{
		// Add a new 1-based entry
		_ASSERT(bIncrement);
		//myThreadLockCountMap[lThreadId] = 1;
		myThreadLockCountMap.insertVal(lThreadId, 1);
#ifdef DEBUG_LOCKING
		gLog("REC_LOCK: thread %ux added new entry\n", lThreadId);
#endif
	}
	else
	{
		// We found an entry
		if(bIncrement)
			(*pCount)++;
			//mi->second++;
		else
			(*pCount)--;
			//mi->second--;

#ifdef DEBUG_LOCKING
		gLog("REC_LOCK: thread %ux %s entry by\n", lThreadId, bIncrement ? "incremented" : "decremented");
#endif
		//if(mi->second <= 0)
		if((*pCount) <= 0)
		{
			//myThreadLockCountMap.erase(mi);
			myThreadLockCountMap.remove(lThreadId);
#ifdef DEBUG_LOCKING
			gLog("REC_LOCK: thread %ux erased entry\n", lThreadId);
#endif
		}
	}
	myMapLock.unlock();
}
/*****************************************************************************/
#else
/*****************************************************************************/
void RecursiveSharedLock::getThreadId(string& strOut) const
{
	myTempStream.str("");
	myTempStream.clear();
	myTempStream << boost::this_thread::get_id();
	strOut = myTempStream.str();
}
/*****************************************************************************/
void RecursiveSharedLock::lock() const
{ 
#ifdef DEBUG_LOCKING
	gLog("REC_LOCK: write lock request\n");
#endif

	if(!didLockAlready())
		mySharedLock.lock(); 
	changeLockCount(true);
}
/*****************************************************************************/
void RecursiveSharedLock::unlock() const  
{ 
#ifdef DEBUG_LOCKING
	gLog("REC_LOCK: write unlock request\n");
#endif
	changeLockCount(false);
	if(!didLockAlready())
		mySharedLock.unlock();  
}
/*****************************************************************************/
void RecursiveSharedLock::lock_shared() const  
{ 
#ifdef DEBUG_LOCKING
	gLog("REC_LOCK: read lock request\n");
#endif
	if(!didLockAlready())
		mySharedLock.lock_shared(); 
	changeLockCount(true);
}
/*****************************************************************************/
void RecursiveSharedLock::unlock_shared() const   
{ 
#ifdef DEBUG_LOCKING
	gLog("REC_LOCK: read unlock request\n");
#endif
	changeLockCount(false);
	if(!didLockAlready())
		mySharedLock.unlock_shared(); 
}
/*****************************************************************************/
bool RecursiveSharedLock::didLockAlready() const
{
	boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	getThreadId(myTempString);
#ifdef DEBUG_LOCKING
	gLog("REC_LOCK: thread %s didLockAlready = %s\n", myTempString.c_str(), (myThreadLockCountMap.find(myTempString) != myThreadLockCountMap.end()) ? "YES" : "NO" );
#endif
	return myThreadLockCountMap.find(myTempString) != myThreadLockCountMap.end();
}
/*****************************************************************************/
void RecursiveSharedLock::changeLockCount(bool bIncrement) const
{
	boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	getThreadId(myTempString);
	TStringIntMap::iterator mi = myThreadLockCountMap.find(myTempString);
	if(mi == myThreadLockCountMap.end())
	{
		// Add a new 1-based entry
		_ASSERT(bIncrement);
		myThreadLockCountMap[myTempString.c_str()] = 1;
#ifdef DEBUG_LOCKING
		gLog("REC_LOCK: thread %s added new entry\n", myTempString.c_str());
#endif
	}
	else
	{
		// We found an entry
		if(bIncrement)
			mi->second++;
		else
			mi->second--;

#ifdef DEBUG_LOCKING
		gLog("REC_LOCK: thread %s %s entry by\n", myTempString.c_str(), bIncrement ? "incremented" : "decremented");
#endif
		if(mi->second <= 0)
		{
			myThreadLockCountMap.erase(mi);
#ifdef DEBUG_LOCKING
			gLog("REC_LOCK: thread %s erased entry\n", myTempString.c_str());
#endif
		}
	}
}
/*****************************************************************************/
#endif
};
