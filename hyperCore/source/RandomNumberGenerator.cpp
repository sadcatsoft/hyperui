#include "stdafx.h"

#if defined(WINDOWS) || defined(LINUX)
	#include <time.h>
	#define RANDOM_SEED_FIXED(x) srand((unsigned)x)
	#define RANDOM_SEED() srand((unsigned)time( NULL ))
#else
	#import <mach/mach_time.h>
	#define RANDOM_SEED_FIXED(x) srandom((unsigned)x)
	#define RANDOM_SEED() srandom((unsigned)(mach_absolute_time() & 0xFFFFFFFF))
#endif

namespace HyperCore
{
RandomNumberGenerator *RandomNumberGenerator::theInstance = NULL;

#if !defined(ALLOW_MULTITHREADED_RND_GENERATOR) && !defined(_DEBUG) && !defined(DEBUG_MODE) && !defined(_DEBUG)
#error The random number generator is broken for multithreads!
#endif
/*****************************************************************************/
RandomNumberGenerator* RandomNumberGenerator::getInstance()
{
	if(!theInstance)
		theInstance = new RandomNumberGenerator;
	return theInstance;
}
/*****************************************************************************/
void RandomNumberGenerator::setFixedSeed(unsigned int iSeed)
{
	RANDOM_SEED_FIXED(iSeed);
	myHasSetFixedSeed = true;

#ifdef ALLOW_MULTITHREADED_RND_GENERATOR
	mySharedLock.lock();
	myThreadIdMap.clear();
	mySharedLock.unlock();
#endif
}
/*****************************************************************************/
void RandomNumberGenerator::ensureRandomSeed()
{
#ifdef ALLOW_MULTITHREADED_RND_GENERATOR
	NATIVE_THREAD_TYPE lThreadId = RecursiveSharedLock::getThreadId();
	mySharedLock.lock();
	bool bHasCalledInitialSeedForThisThread = myThreadIdMap.find(lThreadId) != NULL;
	mySharedLock.unlock();
#else
	bool bHasCalledInitialSeedForThisThread = false;
#endif

	if(!myHasSetFixedSeed && bHasCalledInitialSeedForThisThread)
		return;

	RANDOM_SEED();
	myHasSetFixedSeed = false;

#ifdef ALLOW_MULTITHREADED_RND_GENERATOR
	mySharedLock.lock();
	myThreadIdMap.insertVal(lThreadId, 1);
	mySharedLock.unlock();
#endif
}
/*****************************************************************************/
RandomNumberGenerator::RandomNumberGenerator()
{
// 	RANDOM_SEED();
// 	myHasSetFixedSeed = false;

	// Ensure we call 
	myHasSetFixedSeed = true;
	ensureRandomSeed();
}
/*****************************************************************************/
};