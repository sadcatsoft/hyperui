#pragma once

enum RefreshType
{
	RefreshAnimate						= 0x0001,
	RefreshAllowRecreate				= 0x0002,
	RefreshDisableBrushOptionsRefresh	= 0x0004,
};

class StandaloneEvent;
/*****************************************************************************/
class HYPERUI_API EventReceiver
{
public:
    virtual ~EventReceiver() { }
    virtual void receiveEvent(EventType eEventType, IBaseObject* pSender) = 0;
};

// The int is counting registrations for indiv elems for consistency checking.
typedef map < EventReceiver*, int > TEventReceivers;
/*****************************************************************************/
class HYPERUI_API EventManager
{
public:
    static EventManager* getInstance();
    ~EventManager();

    void registerObject(EventReceiver* pObject);
    void unregisterObject(EventReceiver* pObject);

	void sendEvent(EventType eType, IBaseObject* pSender);
	void sendMainThreadEvent(StandaloneEvent* pEvent);

	void onTimerTick();

private:

    EventManager();
	void processMainThreadEvents();

private:

    static  EventManager* theInstance;

    TEventReceivers myReceivers;
	RecursiveSharedLock myMapLock;
	//boost::recursive_mutex myMapLock;

	// These are for main UI thread events
	RecursiveSharedLock myMainThreadEventQueueLock;
	TStandaloneEventVector myMainThreadEventQueue;

};
/*****************************************************************************/