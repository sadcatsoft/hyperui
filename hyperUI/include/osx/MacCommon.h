#pragma once

#include "MacUtils.h"
#include "MainMenuManager.h"

//#define LOG_QUIT_ROUTINE(x) Logger::log
#define LOG_QUIT_ROUTINE(x) 

namespace HyperUI
{
const char* getCStringSafe(NSString* pStringIn);
NSArray* stringArrayToObjArray(TStringVector& vecStrings);
NSResponder* findFormByWindow(Window* pWindow);
NSString* getOSVersionInfo();

bool getHaveCommandKey(NSEvent *event);
bool getHaveAltKey(NSEvent *event);
bool getHaveShiftKey(NSEvent* event);
bool getHaveControlKey(NSEvent *event);
int mapVirtualKeyCode(NSEvent *event, bool &bIsAffectingCommandOut, bool &bIsAffectingAltOut, bool &bIsAffectingShiftOut, bool &bIsAffectingActualControlOut);

struct SMacFormWrapper
{
	void setId(UNIQUEID_TYPE idValue) { myId = idValue; }
	UNIQUEID_TYPE getId() { return myId; }

	NSObject<NSWindowDelegate> *myController;
	UNIQUEID_TYPE myId;
};
typedef TWindowManager< SMacFormWrapper > MacFormWindowManager;

}
