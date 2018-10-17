#pragma once

#define ALLOW_MULTITHREADED_RND_GENERATOR

/*****************************************************************************/
class RandomNumberGenerator
{
public:
	HYPERCORE_API static RandomNumberGenerator* getInstance();

	HYPERCORE_API void setFixedSeed(unsigned int iSeed);
	HYPERCORE_API void ensureRandomSeed();

private:
	RandomNumberGenerator();

private:
	static RandomNumberGenerator *theInstance;

	bool myHasSetFixedSeed;

#ifdef ALLOW_MULTITHREADED_RND_GENERATOR

#ifdef USE_BOOST_LOCKS
	mutable boost::shared_mutex mySharedLock;
#else
	Mutex mySharedLock;
#endif
	TThreadIdIntMap myThreadIdMap;
#endif
};
/*****************************************************************************/
