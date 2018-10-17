#include "stdafx.h"

#ifdef _DEBUG
//#define DEBUG_EVENT_REGISTRATION
#endif

namespace HyperUI
{
EventManager* EventManager::theInstance = NULL;
/*****************************************************************************/
EventManager::EventManager()
{

}
/*****************************************************************************/
EventManager::~EventManager()
{
	_ASSERT(myReceivers.size() == 0);
}
/*****************************************************************************/
EventManager* EventManager::getInstance()
{
    if(!theInstance)
		theInstance = new EventManager;
    return theInstance;
}
/*****************************************************************************/
void EventManager::registerObject(EventReceiver* pObject)
{
	///boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	myMapLock.lock();

	// Check to make sure we don't register the same object twice.
	TEventReceivers::iterator si = myReceivers.find(pObject);
	if(si == myReceivers.end())	
	    myReceivers[pObject] = 0;
	else
		myReceivers[pObject]++;

#ifdef DEBUG_EVENT_REGISTRATION
	const char *pcsDbName = EMPTY_STRING;
	if(pObject && dynamic_cast<ResourceItem*>(pObject))
		pcsDbName = dynamic_cast<ResourceItem*>(pObject)->getDebugName();
	gLog("EVENT_REG: Registered %x %s count = %d\n", pObject, pcsDbName, myReceivers[pObject]);
#endif

	myMapLock.unlock();
}
/*****************************************************************************/
void EventManager::unregisterObject(EventReceiver* pObject)
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	myMapLock.lock();
    TEventReceivers::iterator si = myReceivers.find(pObject);
    if(si != myReceivers.end())
	{
		// We ignore the count and just de-register it.
		myReceivers.erase(si);
#ifdef DEBUG_EVENT_REGISTRATION
		const char *pcsDbName = EMPTY_STRING;
		if(pObject && dynamic_cast<ResourceItem*>(pObject))
			pcsDbName = dynamic_cast<ResourceItem*>(pObject)->getDebugName();
		gLog("EVENT_REG: Unregistered %x %s count = deregistering all\n", pObject, pcsDbName);
#endif
/*
#ifdef DEBUG_EVENT_REGISTRATION
		int iPrevCount = myReceivers[pObject];
#endif
		if(si->second > 0)
			myReceivers[pObject]--;
		else
			myReceivers.erase(si);

#ifdef DEBUG_EVENT_REGISTRATION
		const char *pcsDbName = EMPTY_STRING;
		if(pObject && dynamic_cast<ResourceItem*>(pObject))
			pcsDbName = dynamic_cast<ResourceItem*>(pObject)->getDebugName();
		gLog("EVENT_REG: Unregistered %x %s count = %d\n", pObject, pcsDbName, iPrevCount);
#endif
		*/
	}
    else
    {
		// Unregistering non-registered receiver!
		//_ASSERT(0);
    }
	myMapLock.unlock();
}
/*****************************************************************************/
void EventManager::sendEvent(EventType eType, IBaseObject* pSender)
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	myMapLock.lock_shared();
    TEventReceivers::iterator si;
    for(si = myReceivers.begin(); si != myReceivers.end(); si++)
		si->first->receiveEvent(eType, pSender);
	myMapLock.unlock_shared();
}
/*****************************************************************************/
void EventManager::onTimerTick()
{
	this->processMainThreadEvents();
}
/*****************************************************************************/
void EventManager::sendMainThreadEvent(StandaloneEvent* pEvent)
{
	myMainThreadEventQueueLock.lock();
	myMainThreadEventQueue.push_back(pEvent);
	myMainThreadEventQueueLock.unlock();
}
/*****************************************************************************/
void EventManager::processMainThreadEvents()
{
	myMainThreadEventQueueLock.lock_shared();
	// Go over each event and take appropriate
	// action
	int iCurr, iNum = myMainThreadEventQueue.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
#ifdef _DEBUG
		//gLog("Executing queued event: %s\n", myEventQueue[iCurr]->getPayload().getString());
#endif
		// Note that we need to lock and lock this while we execute
		// because the execution itself may need to create other events.
		// If we don't do this, we'll have deadlock.
		myMainThreadEventQueueLock.unlock_shared();
		myMainThreadEventQueue[iCurr]->execute();

		myMainThreadEventQueueLock.lock_shared();
		delete myMainThreadEventQueue[iCurr];
	}

	// Note that now that we unlock while executing above, we can't just
	// clear the vector - we need to erase just the events they processed.
	myMainThreadEventQueue.erase(myMainThreadEventQueue.begin(), myMainThreadEventQueue.begin() + iNum);
	myMainThreadEventQueueLock.unlock_shared();
}
/*****************************************************************************/
};