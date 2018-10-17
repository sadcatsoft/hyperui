#import <Cocoa/Cocoa.h>
#include "stdafx.h"
#include "MacCommon.h"
#import <mach/mach.h>
#import <mach/mach_host.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include "BaseWindowController.h"

namespace HyperUI
{
/*****************************************************************************/
void releaseAsMutableArray(void* pObj)
{
	NSMutableArray* pArray = (NSMutableArray*)pObj;
	[pArray release];	
}
/*****************************************************************************/
const char* getCStringSafe(NSString* pStringIn)
{
	if(!pStringIn)
		return "";
	const char* pcsString = [pStringIn cStringUsingEncoding:NSASCIIStringEncoding];
	if(pcsString)
		return pcsString;
	else
		return "";
}
/*****************************************************************************/
NSArray* stringArrayToObjArray(TStringVector& vecStrings)
{
	int iCurr, iNum = vecStrings.size();
	NSMutableArray* pRes = [[NSMutableArray alloc] init];

	for(iCurr = 0; iCurr < iNum; iCurr++)
		[pRes addObject:[NSString stringWithCString: vecStrings[iCurr].c_str()]];

	return pRes;
}
/*****************************************************************************/
NSResponder* findFormByWindow(Window* pWindow)
{
    HyperUI::Window* pCurrWindow;
	NSObject* pCurrForm = NULL;
	MacFormWindowManager::Iterator fi;
	for(fi = MacFormWindowManager::getInstance()->windowsBegin(); !fi.isEnd(); fi++)
	{
		pCurrForm = fi.getWindow()->myController;
        pCurrWindow = (HyperUI::Window*)[pCurrForm getWindow];
      
        pCurrWindow = NULL;
        
        BaseWindowController* pCastCont = objc_dynamic_cast(pCurrForm, BaseWindowController);
        if(pCastCont)
            pCurrWindow = [pCastCont getWindow];
        /*
        MacMainController* pCastCont2 = objc_dynamic_cast(pCurrForm, MacMainController);
        if(pCastCont2)
            pCurrWindow = [pCastCont2 getWindow];
        */
		//pCurrWindow = (Window*)[pCurrForm getWindow];
        if(pCurrWindow == pWindow)
			return pCurrForm;
	}
    
	return NULL;
}
/*****************************************************************************/
static NSString* const kVarSysInfoVersionFormat  = @"%@.%@.%@ (%@)";
static NSString* const kVarSysInfoPlatformExpert = @"IOPlatformExpertDevice";
static NSString* const kVarSysInfoKeyOSVersion = @"kern.osrelease";
static NSString* const kVarSysInfoKeyOSBuild   = @"kern.osversion";
static NSString* const kVarSysInfoKeyModel     = @"hw.model";
static NSString* const kVarSysInfoKeyCPUCount  = @"hw.physicalcpu";
static NSString* const kVarSysInfoKeyCPUFreq   = @"hw.cpufrequency";
static NSString* const kVarSysInfoKeyCPUBrand  = @"machdep.cpu.brand_string";
/*****************************************************************************/
NSString* getControlEntry(NSString* ctlKey)
{

    size_t size = 0;
    if ( sysctlbyname([ctlKey UTF8String], NULL, &size, NULL, 0) == -1 ) return nil;

    char *machine = (char*)calloc( 1, size );

    sysctlbyname([ctlKey UTF8String], machine, &size, NULL, 0);
    NSString *ctlValue = [NSString stringWithCString:machine encoding:[NSString defaultCStringEncoding]];

    free(machine); 
	return ctlValue;
}
/*****************************************************************************/
uint64_t getNumControlEntry(NSString* ctlKey)
{

    size_t size = sizeof( uint64_t ); 
	uint64_t ctlValue = 0;
    if ( sysctlbyname([ctlKey UTF8String], &ctlValue, &size, NULL, 0) == -1 ) 
		return 0;
    return ctlValue;
}
/*****************************************************************************/
NSString* getOSVersionInfo()
{
    NSString *darwinVer = getControlEntry(kVarSysInfoKeyOSVersion);
    NSString *buildNo = getControlEntry(kVarSysInfoKeyOSBuild);
    if ( !darwinVer || !buildNo ) return nil;
    
    NSString *majorVer = @"10", *minorVer = @"x", *bugFix = @"x";
    NSArray *darwinChunks = [darwinVer componentsSeparatedByCharactersInSet:[NSCharacterSet punctuationCharacterSet]];
    
    if ( [darwinChunks count] > 0 ) {
        
        NSInteger firstChunk = [(NSString *)[darwinChunks objectAtIndex:0] integerValue];
        minorVer = [NSString stringWithFormat:@"%ld", (firstChunk - 4)];
        bugFix = [darwinChunks objectAtIndex:1];
        return [NSString stringWithFormat:kVarSysInfoVersionFormat, majorVer, minorVer, bugFix, buildNo];
        
    } 
	return nil;
}
/*****************************************************************************/
void getHardwareInfo(string& strOut)
{
    strOut = "";
    
    string strTemp;
    NSProcessInfo *pi = [NSProcessInfo processInfo];
    long long lSize64;
    double dFreq;
    
    strOut += "CPU: ";
    strTemp = getCStringSafe(getControlEntry(kVarSysInfoKeyCPUBrand));
    strOut += strTemp;
    lSize64 = getNumControlEntry(kVarSysInfoKeyCPUFreq);
    dFreq = (double)lSize64/(1000.0*1000.0*1000.0);
    StringUtils::numberToNiceString(dFreq, 2, false, strTemp);
    strOut += " " + strTemp + "GHz";
    strOut += "\n";
    
    lSize64 = pi.physicalMemory/(1024*1024);
    strOut += "System RAM: ";
    StringUtils::longNumberToString(lSize64, strTemp);
    strOut += strTemp + "Mb\n";
    
    strOut += "OS: OS X ";
    strTemp = getCStringSafe(getOSVersionInfo());
    strOut += strTemp;
    
}
/*****************************************************************************/
void refreshSizeOnAllWindows()
{
#ifdef IOS_BUILD
#else
    Window* pCurrWindow;
	NSObject* pCurrForm = NULL;
	MacFormWindowManager::Iterator fi;
    Application::lockGlobal();
	for(fi = MacFormWindowManager::getInstance()->windowsBegin(); !fi.isEnd(); fi++)
	{
		pCurrForm = fi.getWindow()->myController;
        
        pCurrWindow = NULL;
        BaseWindowController* pCastCont = objc_dynamic_cast(pCurrForm, BaseWindowController);
        if(pCastCont)
            pCurrWindow = [pCastCont getWindow];

		if(pCurrWindow)
		{
			SVector2D svCurrSize;
            pCurrWindow->getSize(svCurrSize);
			pCurrWindow->onWindowSizeChanged(svCurrSize.x, svCurrSize.y);
		}
	}
    Application::unlockGlobal();
#endif
}
/*****************************************************************************/
#ifdef IOS_BUILD
#else
bool getHaveCommandKey(NSEvent *event) { return ([event modifierFlags] & NSCommandKeyMask) != 0; }
bool getHaveAltKey(NSEvent *event) { return ([event modifierFlags] & NSAlternateKeyMask) != 0; }
bool getHaveShiftKey(NSEvent* event) { return ([event modifierFlags] & NSShiftKeyMask) != 0; }
bool getHaveControlKey(NSEvent *event) { return ([event modifierFlags] & NSControlKeyMask) != 0; }
/*****************************************************************************/
int mapVirtualKeyCode(NSEvent *event, bool &bIsAffectingCommandOut, bool &bIsAffectingAltOut, bool &bIsAffectingShiftOut, bool &bIsAffectingActualControlOut)
{
	NSString *theArrow = nil;
    if([event type] == NSKeyDown || [event type] == NSKeyUp)
        theArrow = [event charactersIgnoringModifiers];
	int iScanCode = 0;
	unichar tempChar = 0;

	bIsAffectingCommandOut = getHaveCommandKey(event);
	bIsAffectingAltOut = getHaveAltKey(event);
	bIsAffectingShiftOut = getHaveShiftKey(event);
	bIsAffectingActualControlOut = getHaveControlKey(event);
	
	if (theArrow && [theArrow length] > 0 )
	{
		tempChar = [theArrow characterAtIndex:0];
		iScanCode = tempChar;
		if(tempChar == NSUpArrowFunctionKey)
			iScanCode = SilentKeyUpArrow;
		else if(tempChar == NSDownArrowFunctionKey)
			iScanCode = SilentKeyDownArrow;
		else if(tempChar == NSRightArrowFunctionKey)
			iScanCode = SilentKeyRArrow;
		else if(tempChar == NSLeftArrowFunctionKey)
			iScanCode = SilentKeyLArrow;
		else if(tempChar == NSEnterCharacter)
			iScanCode = SilentKeyNumpadEnter;
		else if(tempChar == NSCarriageReturnCharacter)
			iScanCode = SilentKeyCarriageReturn;
		else if(tempChar == NSDeleteFunctionKey || tempChar == NSDeleteCharFunctionKey)
			iScanCode = SilentKeyDelete;
		else if(tempChar == NSTabCharacter)
			iScanCode = SilentKeyTab;
		else if(tempChar == NSBackTabCharacter || tempChar == NSBackspaceCharacter || tempChar == NSDeleteCharacter)
			iScanCode = SilentKeyBackspace;
		else if(tempChar == 27)
			iScanCode = SilentKeyEscape;
		else if(tempChar == NSEndFunctionKey)
			iScanCode = SilentKeyEnd;
		else if(tempChar == NSBeginFunctionKey)
			iScanCode = SilentKeyHome;
		else if(tempChar == NSPageUpFunctionKey)
			iScanCode = SilentKeyPageUp;
		else if(tempChar == NSPageDownFunctionKey)
			iScanCode = SilentKeyPageDown;
		else if(tempChar == NSF1FunctionKey)
			iScanCode = SilentKeyF1;
		else if(tempChar == NSF2FunctionKey)
			iScanCode = SilentKeyF2;
		else if(tempChar == NSF3FunctionKey)
			iScanCode = SilentKeyF3;
		else if(tempChar == NSF4FunctionKey)
			iScanCode = SilentKeyF4;
		else if(tempChar == NSF5FunctionKey)
			iScanCode = SilentKeyF5;
		else if(tempChar == NSF6FunctionKey)
			iScanCode = SilentKeyF6;
		else if(tempChar == NSF7FunctionKey)
			iScanCode = SilentKeyF7;
		else if(tempChar == NSF8FunctionKey)
			iScanCode = SilentKeyF8;
		else if(tempChar == NSF9FunctionKey)
			iScanCode = SilentKeyF9;
		else if(tempChar == NSF10FunctionKey)
			iScanCode = SilentKeyF10;
		else if(tempChar == NSF11FunctionKey)
			iScanCode = SilentKeyF11;
		else if(tempChar == NSF12FunctionKey)
			iScanCode = SilentKeyF12;
	}

	if(isalpha((char)iScanCode))
	{
		if(bIsAffectingShiftOut)
			iScanCode = toupper((char)iScanCode);
		else
			iScanCode = tolower((char)iScanCode);
	}
	
	return iScanCode;

}
/*****************************************************************************/
#endif
/*****************************************************************************/
HYPERUI_API void run()
{
	Window* pWindow = Application::openNewWindow(NULL, -1);
	[NSApp activateIgnoringOtherApps:true];
}
/*****************************************************************************/
HYPERUI_API void run(const SWindowParms& rParms)
{
	Window* pWindow = Application::openNewWindow(NULL, rParms.myInitCollectionType, rParms.myWidth, rParms.myHeight, rParms.myIsThinFrame, 
												rParms.myIsFixedSize, rParms.myMinWidth, rParms.myMinHeight, rParms.myTitle.c_str(), rParms.myCenterX, 
												rParms.myCenterY, true, rParms.myMainMenuElementId.c_str(), rParms.myInitLayerToShow.c_str());
	[NSApp activateIgnoringOtherApps:true];
}
/*****************************************************************************/
void initializeOSSpecific()
{

}
/*****************************************************************************/
};
