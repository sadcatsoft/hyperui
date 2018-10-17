#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
// StandaloneEvent
/*****************************************************************************/
StandaloneEvent::StandaloneEvent(EventType eType, const CHAR_TYPE* pcsValue)
{
	myType = eType;
	myPayload.setString(pcsValue);
}
/*****************************************************************************/
StandaloneEvent::StandaloneEvent(EventType eType)
{
	myType = eType;
}
/*****************************************************************************/
StandaloneEvent::StandaloneEvent(EventType eType, const CHAR_TYPE* pcsValue, const CHAR_TYPE* pcsData1, const CHAR_TYPE* pcsData2)
{
	myType = eType;
	myPayload.setString(pcsValue);
	if(pcsData1)
		myData[0].setString(pcsData1);
	if(pcsData2)
		myData[1].setString(pcsData2);

	//BOOST_STATIC_ASSERT(MAX_EVENT_DATA_PIECES == 2);
}
/*****************************************************************************/
StandaloneEvent::~StandaloneEvent()
{

}
/*****************************************************************************/
// ExecuteActionEvent
/*****************************************************************************/
ExecuteActionEvent::ExecuteActionEvent(Window* pWindow, const CHAR_TYPE* pcsValue, const CHAR_TYPE* pcsData1 , const CHAR_TYPE* pcsData2)
	: StandaloneEvent(EventExecuteAction, pcsValue, pcsData1, pcsData2)
{
	myWindowId = pWindow->getId();
}
/*****************************************************************************/
void ExecuteActionEvent::execute()
{	
	Window* pWindow = WindowManager::getInstance()->findItemById(myWindowId);

	if(!pWindow)
		ASSERT_RETURN;

	pWindow->getUIPlane()->performUiAction(myPayload.getString(), 
		myData[0].getPropertyDataType() == PropertyDataString ? myData[0].getString() : NULL,
		myData[1].getPropertyDataType() == PropertyDataString ? myData[1].getString() : NULL);

	//BOOST_STATIC_ASSERT(MAX_EVENT_DATA_PIECES == 2);
}
/*****************************************************************************/
};