#pragma once

#if defined(LINUX) || defined(ANDROID_BUILD) || defined(EMSCRIPTEN)
#include <pthread.h>
#endif
/*****************************************************************************/
class HYPERCORE_API Mutex
{
public:
	Mutex();
	~Mutex();

	void lock();
	void unlock();

private:
#ifdef WINDOWS
	CRITICAL_SECTION myNativeLock;
#endif
#if defined(LINUX) || defined(ANDROID_BUILD) || defined(EMSCRIPTEN)
	pthread_mutex_t myPLock;
#elif !defined(WINDOWS)
	void *myNativeLock;
#endif
};
/*****************************************************************************/
class HYPERCORE_API ScopedMutex
{
public:
	ScopedMutex(Mutex *pMutex) 
	{ 
		/* LOGE("ScopedMutex locking %x thread %x", pMutex, getThreadId()); */ 
		pMutex->lock(); 
		myMutex = pMutex;  
		/* LOGE("ScopedMutex locked %x thread %x", pMutex, getThreadId()); */ 
	}

	~ScopedMutex() 
	{ 
		myMutex->unlock(); 
		/* LOGE("ScopedMutex unlocked %x thread %x", myMutex, getThreadId()); */ 
	}

private:
	Mutex *myMutex;
};
/*****************************************************************************/
template < class TYPE >
class ScopedReadWriteMutex
{
public:
	ScopedReadWriteMutex(TYPE *pObj, bool bLockForReading) 
	{ 
		/* LOGE("ScopedReadWriteMutex locking %x thread %x", pMutex, getThreadId()); */ 
		myIsReadLock = bLockForReading;
		if(bLockForReading)
			pObj->lockForReading(); 
		else
			pObj->lockForWriting(); 
		myObject = pObj;  
		/* LOGE("ScopedReadWriteMutex locked %x thread %x", pMutex, getThreadId()); */ 
	}
	~ScopedReadWriteMutex() 
	{ 
		if(myIsReadLock)
			myObject->unlockForReading(); 
		else
			myObject->unlockForWriting(); 
		/* LOGE("ScopedReadWriteMutex unlocked %x thread %x", myMutex, getThreadId()); */ 
	}

private:
	TYPE *myObject;
	bool myIsReadLock;
};
/*****************************************************************************/
#define SCOPED_MUTEX_LOCK(pMutex)			ScopedMutex rLock(pMutex);
