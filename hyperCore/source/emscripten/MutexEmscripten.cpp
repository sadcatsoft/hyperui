#include "stdafx.h"
#include "Mutex.h"

#if defined(EMSCRIPTEN)
/*****************************************************************************/
Mutex::Mutex()
{
	pthread_mutexattr_t   mta;
	pthread_mutexattr_init(&mta);
	pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(&myPLock, &mta);
}
/*****************************************************************************/
Mutex::~Mutex()
{
	pthread_mutex_destroy(&myPLock);
}
/*****************************************************************************/
void Mutex::lock()
{
	pthread_mutex_lock(&myPLock);
}
/*****************************************************************************/
void Mutex::unlock()
{
	pthread_mutex_unlock(&myPLock);
}
/*****************************************************************************/
#endif
