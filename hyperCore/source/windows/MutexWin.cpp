#include "stdafx.h"

namespace HyperCore
{
#ifdef WINDOWS
/*****************************************************************************/
Mutex::Mutex()
{
	InitializeCriticalSection(&myNativeLock);	
}
/*****************************************************************************/
Mutex::~Mutex()
{
	 DeleteCriticalSection(&myNativeLock);
}
/*****************************************************************************/
void Mutex::lock()
{
	EnterCriticalSection(&myNativeLock);
}
/*****************************************************************************/
void Mutex::unlock()
{
	LeaveCriticalSection(&myNativeLock);
}
/*****************************************************************************/
#endif
};