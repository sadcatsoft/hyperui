#pragma once

#define MAX_EVENT_DATA_PIECES			2

class Window;
/*****************************************************************************/
typedef int EventType;

#define DECLARE_EVENT(eventName) \
	const int eventName = (__LINE__);

#define DECLARE_EVENT_EXPLICIT(eventName, iValue) \
	const int eventName = (iValue);
/*****************************************************************************/
class HYPERUI_API StandaloneEvent
{
public:
	StandaloneEvent(EventType eType);
	StandaloneEvent(EventType eType, const CHAR_TYPE* pcsValue);
	StandaloneEvent(EventType eType, const CHAR_TYPE* pcsValue, const CHAR_TYPE* pcsData1, const CHAR_TYPE* pcsData2);
	virtual ~StandaloneEvent();

	inline EventType getType() const { return myType; }

	const ResourceProperty& getPayload() const { return myPayload; }
	const ResourceProperty& getData(int iIndex) const { return myData[iIndex]; }

	virtual void execute() { }

protected:
	EventType myType;
	ResourceProperty myPayload;
	ResourceProperty myData[MAX_EVENT_DATA_PIECES];
};
/*****************************************************************************/
class HYPERUI_API ExecuteActionEvent : public StandaloneEvent
{
public:
	ExecuteActionEvent(Window* pWindow, const CHAR_TYPE* pcsValue, const CHAR_TYPE* pcsData1 = NULL, const CHAR_TYPE* pcsData2 = NULL);
	virtual ~ExecuteActionEvent() { }

	virtual void execute();

private:
	UNIQUEID_TYPE myWindowId;
};
/*****************************************************************************/
typedef vector < StandaloneEvent* > TStandaloneEventVector;