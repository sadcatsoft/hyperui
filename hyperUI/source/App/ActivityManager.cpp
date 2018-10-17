#include "stdafx.h"

//#define LOG_ACTIVITY_MANAGER

namespace HyperUI
{
ActivityManager* ActivityManager::theInstance = NULL;
/*****************************************************************************/
ActivityManager::ActivityManager()
{
	// First time, force redraw.
	myHaveSingleValueChange = true;
	myAnimationsCounter = 0;
	myExtraRenderTicksCounter = 0;
	myCallback = NULL;
}
/*****************************************************************************/
ActivityManager::~ActivityManager()
{
	delete myCallback;
	myCallback = NULL;
}
/*****************************************************************************/
ActivityManager* ActivityManager::getInstance()
{
	if(!theInstance)
		theInstance = new ActivityManager;
	return theInstance;
}
/*****************************************************************************/
#ifdef _DEBUG
#ifdef LOG_ACTIVITY_MANAGER
static bool bIsPrevNeeded = false;
#endif
#endif
void ActivityManager::beginAnimation(const char* pcsOptDebugTag)
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myLock);
	SCOPED_MUTEX_LOCK(&myLock);
	myAnimationsCounter++;

#ifdef _DEBUG
#ifdef LOG_ACTIVITY_MANAGER
	const char* pcsPtr = "UNKNOWN";
	if(pcsOptDebugTag && pcsOptDebugTag[0])
		pcsPtr = pcsOptDebugTag;
	else
	{
		int bp = 0;
	}
	if(bIsPrevNeeded && IS_STRING_EQUAL(pcsPtr, "photo.jpg") && myAnimationsCounter == 1)
	{
		int bp = 0;
	}
	if(IS_STRING_EQUAL(pcsPtr, "visibleButton/visibleButton/visibleButton - pushedDownValue"))
	{
		int bp = 0;
	}
	if(IS_STRING_EQUAL(pcsPtr, "toggleObjSnapButton/toggleObjSnapButton/toggleObjSnapButton - pushedDownValue"))
	{
		int bp = 0;
	}
	if(IS_STRING_EQUAL(pcsPtr, "togglePressureSensButton/togglePressureSensButton/togglePressureSensButton - pushedDownValue"))
	{
		int bp = 0;
	}

	//bIsPrevNeeded = IS_STRING_EQUAL(pcsPtr, "nfdPortraitCheckbox/nfdPortraitCheckbox/nfdPortraitCheckbox/nfdPortraitCheckbox/nfdPortraitCheckbox - pushedDownValue");
	gLog("ActivityManager::beginAnimation for %s, new value = %d\n", pcsPtr, myAnimationsCounter);
#endif
#endif
}
/*****************************************************************************/
void ActivityManager::endAnimation(const char* pcsOptDebugTag)
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myLock);
	SCOPED_MUTEX_LOCK(&myLock);
#ifdef _DEBUG
#ifdef LOG_ACTIVITY_MANAGER
	const char* pcsPtr = "UNKNOWN";
	if(pcsOptDebugTag && pcsOptDebugTag[0])
		pcsPtr = pcsOptDebugTag;
	if(IS_STRING_EQUAL(pcsPtr, "visibleButton/visibleButton/visibleButton - pushedDownValue"))
	{
		int bp = 0;
	}
	gLog("ActivityManager::endAnimation for %s, new value = %d\n", pcsPtr, myAnimationsCounter - 1);
	bIsPrevNeeded = IS_STRING_EQUAL(pcsPtr, "visibleButton/visibleButton/visibleButton - pushedDownValue");
#endif
#endif
	myAnimationsCounter--; 
	_ASSERT(myAnimationsCounter >= 0); 
	if(myAnimationsCounter < 0) 
		myAnimationsCounter = 0;
}
/*****************************************************************************/
bool ActivityManager::getIsActiveNoExtraDrawsCounter()
{
#ifdef RUN_AS_FAST_AS_POSSIBLE
	return true;
#endif
	return myHaveSingleValueChange 
		|| (myAnimationsCounter > 0)
		|| (myCallback && myCallback->getIsActive());
}
/*****************************************************************************/
};