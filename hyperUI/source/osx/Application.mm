#import <Cocoa/Cocoa.h>
#include "stdafx.h"
#include "MacCommon.h"
#import <HyperCore/osx/BookmarkManager.h>
#include "BaseWindowController.h"
#include "BaseGLView.h"

//#define _DEBUG
//#define LOG_THREADING

#ifndef IOS_BUILD
@interface MacOpenWindowDelegate : NSObject < NSApplicationDelegate >
{
    IBOutlet NSWindow *window;
}

@property (assign) IBOutlet NSWindow* window;

@end

@implementation MacOpenWindowDelegate

@synthesize window;

@end
#endif

namespace HyperUI
{

// TODO: Make this cleaner
NSRecursiveLock *myGlobalLock = [[NSRecursiveLock alloc] init];
#if defined(_DEBUG) && defined(LOG_THREADING)
NSRecursiveLock* myPrintingLock = [[NSRecursiveLock alloc] init];
#endif

    string Application::theColorUiElemName;
    
void printStack(void)
{
	NSArray* pStack = [NSThread callStackSymbols];
	int iCurr, iNum = [pStack count];
	for(iCurr = 2; iCurr < iNum; iCurr++)
	{
		NSLog(@"%@", [pStack objectAtIndex:iCurr]);
	}
}

#if defined(_DEBUG) && defined(LOG_THREADING)
int g_iLockDepth = 0;
#endif
/*****************************************************************************/
void Application::quitApp()
{
	LOG_QUIT_ROUTINE("QUIT_APP: gQuitApp called\n");	

	LOG_QUIT_ROUTINE("MAIN_CONTROLLER: this.quitApp called");
    Application::unlockGlobal();
    Application::lockGlobal();
	LOG_QUIT_ROUTINE("MAIN_CONTROLLER: this.quitApp locked global");
    [[NSApplication sharedApplication] terminate:nil];
    // This does not ever call any destructors...
	//exit(0);
	LOG_QUIT_ROUTINE("MAIN_CONTROLLER: this.quitApp terminate done");
    Application::unlockGlobal();
	LOG_QUIT_ROUTINE("MAIN_CONTROLLER: this.quitApp unlock global done");

	LOG_QUIT_ROUTINE("QUIT_APP: gQuitApp done\n");	
}
/*****************************************************************************/
void Application::lockGlobal()
{
#ifndef MAC_BUILD
#else
	#if defined(_DEBUG) && defined(LOG_THREADING)
		[myPrintingLock lock];
		NSLog(@"----------------------------------------------------------------\nREQUEST LOCK by (count = %d): %@", g_iLockDepth, [NSThread currentThread]);
		printStack();
		[myPrintingLock unlock];
	#endif
		[myGlobalLock lock];
	#if defined(_DEBUG) && defined(LOG_THREADING)
		[myPrintingLock lock];
		g_iLockDepth++;
		//if(g_iLockDepth > 1)
		{
			NSLog(@"----------------------------------------------------------------\nLOCKED by (count = %d): %@", g_iLockDepth, [NSThread currentThread]);
			printStack();
		}
		[myPrintingLock unlock];
	#endif
#endif
}
/*****************************************************************************/
void Application::unlockGlobal()
{
#ifndef MAC_BUILD
#else
		[myGlobalLock unlock];	
	#if defined(_DEBUG) && defined(LOG_THREADING)
		[myPrintingLock lock];
		g_iLockDepth--;
		//if(g_iLockDepth > 0)
		{
		NSLog(@"----------------------------------------------------------------\nUNLOCKED by (count = %d): %@", g_iLockDepth, [NSThread currentThread]);
		printStack();
		}
		[myPrintingLock unlock];
	#endif

#endif
}
/*****************************************************************************/
void Application::lockGlobalDisplay(void)
{
#ifndef MAC_BUILD
#else
	NSOpenGLView *pGlView = [MacFormWindowManager::getInstance()->getMainWindow()->myController getOpenGLView];
	CGLLockContext((CGLContextObj)[[pGlView openGLContext] CGLContextObj]);
#endif
}
/*****************************************************************************/
void Application::unlockGlobalDisplay(void)
{
#ifndef MAC_BUILD
#else
	NSOpenGLView *pGlView = [MacFormWindowManager::getInstance()->getMainWindow()->myController getOpenGLView];
	CGLUnlockContext((CGLContextObj)[[pGlView openGLContext] CGLContextObj]);	
#endif	
}
/*****************************************************************************/
bool gShowOpenFileDialogCommon(const char* pcsInitFilePath, bool bAllowMultiple, TStringVector& vecFilesOut, 
								TStringVector& pOptExtensions, TStringVector& pOptDescriptions, bool bCanOpenFolders, bool bCanOpenFiles,
								FormatsManager* pFormatsManager)
{
#ifdef MAC_BUILD
	
	bool bHaveValidPath = IS_VALID_STRING_AND_NOT_NONE(pcsInitFilePath);

	vecFilesOut.clear();

	NSOpenPanel *tvarNSOpenPanelObj	= [NSOpenPanel openPanel];
	[tvarNSOpenPanelObj setAllowsMultipleSelection:bAllowMultiple];
	[tvarNSOpenPanelObj setCanChooseDirectories:(bCanOpenFolders ? YES: NO)];
	[tvarNSOpenPanelObj setCanChooseFiles:(bCanOpenFiles ? YES :NO)];
	[tvarNSOpenPanelObj setAllowsOtherFileTypes:NO];

	if(bHaveValidPath)
	{
		tvarNSOpenPanelObj.directoryURL = [NSURL URLWithString:[NSString stringWithCString:pcsInitFilePath]];
	}
   

	TStringVector strTypes;
	NSArray *pAppleTypes;

	if(pOptExtensions.size() > 0 && pOptDescriptions.size() > 0 )
		pAppleTypes = stringArrayToObjArray(pOptExtensions);
	else
	{
		pFormatsManager->getExtensionsList(true, FormatRead, strTypes);
		pAppleTypes = stringArrayToObjArray(strTypes);
	}
	NSInteger tvarNSInteger	= [tvarNSOpenPanelObj runModalForTypes:pAppleTypes];
	[pAppleTypes release];

	if(tvarNSInteger != NSOKButton)
		return false;
	
    NSArray* pURLs = [tvarNSOpenPanelObj URLs];
	int iNumUrls = [pURLs count];
    if(iNumUrls <= 0)
       return false;  

#ifdef TRACE_FILEOPEN_CRASH
	Logger::log("About to process open dialog results...");
#endif
    string strCurrPath;
	int iCurr;
    NSNumber *isDir;
	bool bIsSucces;
	for(iCurr = 0; iCurr < iNumUrls; iCurr++)
	{
		NSURL * tvarUrl = [pURLs objectAtIndex:iCurr];
		NSString * tvarDirectory = [tvarUrl path];
       /*
        NSString* encodedString = (NSString*)CFURLCreateStringByAddingPercentEscapes(kCFAllocatorDefault,
                                                                           (CFStringRef)tvarDirectory,
                                                                           NULL, CFSTR("?#[]@!$&'()*+,;="), kCFStringEncodingUTF8);
        
      
        strCurrPath = [MacGameView getCStringSafe:encodedString];
    
        [encodedString release];
		*/
        strCurrPath = getCStringSafe(tvarDirectory);
#ifdef TRACE_FILEOPEN_CRASH
	Logger::log("Processing path: [%s]", strCurrPath.c_str());
#endif

		BookmarkManager::getInstance()->addBookmark(tvarUrl);

		bIsSucces = [tvarUrl getResourceValue:&isDir forKey:NSURLIsDirectoryKey error:nil];

		if(bIsSucces && [isDir boolValue] && bCanOpenFiles)
		{
			// Note that this is a shallow iter
			FileUtils::listFilesOf(strCurrPath.c_str(), vecFilesOut, true);
		}
	
		if(strCurrPath.length() > 0)
			vecFilesOut.push_back(strCurrPath);
	}

	return vecFilesOut.size() > 0;
    
#else
	return false;
#endif
}
/*****************************************************************************/
bool Application::showOpenFolderDialog(const char* pcsInitFilePath, TStringVector& vecFilesOut)
{
	string strFinalPath;
	if(IS_VALID_STRING_AND_NOT_NONE(pcsInitFilePath))
	{
		// For this one, we need to extract the path, otherwise, we'll be in an invalid
		// state where a file is selected
        PathUtils::extractPathFromFullFilename(pcsInitFilePath, strFinalPath);
	}
    
    TStringVector dummy1, dummy2;
    return gShowOpenFileDialogCommon(strFinalPath.c_str(), true, vecFilesOut, dummy1, dummy2, true, false, NULL);
}
/*****************************************************************************/
bool Application::showOpenFileDialog(const char* pcsInitFilePath, bool bAllowMultiple, TStringVector& vecFilesOut, TStringVector& pOptExtensions, TStringVector& pOptDescriptions, FormatsManager* pFormatsManager, bool bCanOpenFolders)
{
    return gShowOpenFileDialogCommon(pcsInitFilePath, bAllowMultiple, vecFilesOut, pOptExtensions, pOptDescriptions, bCanOpenFolders, true, pFormatsManager);
}
/*****************************************************************************/
ResultCodeType Application::showSaveFileDialog(string& strPathInOut, string& strErrorOut, TStringVector& pOptExtensions, TStringVector& pOptDescriptions, FormatsManager* pFormatsManager, bool bForceNativeExtension, bool bDontListNative)
{
	strErrorOut = "";
#ifdef MAC_BUILD

	TStringVector* pFinalExensions, *pFinalDescriptions;

	TStringVector strTypes;
	TStringVector strDescs;
	NSArray *pAppleTypes;
	if(pOptExtensions.size() > 0 && pOptDescriptions.size() > 0)
	{
		pAppleTypes = stringArrayToObjArray(pOptExtensions);
		pFinalExensions = &pOptExtensions;
		pFinalDescriptions = &pOptDescriptions;
	}
	else
	{
		pFormatsManager->getExtensionsList(false, FormatWrite, strTypes);
		pFormatsManager->getDescriptionsList(false, FormatWrite, strDescs);

		pFinalExensions = &strTypes;
		pFinalDescriptions = &strDescs;
		pAppleTypes = stringArrayToObjArray(strTypes);
	}
	
	NSSavePanel *tvarNSOpenPanelObj	= [NSSavePanel savePanel];
  // NSSavePanel *tvarNSOpenPanelObj	= [[NSSavePanel alloc] init];
 	
 
	[tvarNSOpenPanelObj setAllowedFileTypes:pAppleTypes];
	[tvarNSOpenPanelObj setAllowsOtherFileTypes:NO];
    [tvarNSOpenPanelObj setCanCreateDirectories:YES];
//[tvarNSOpenPanelObj setCanSelectHiddenExtension:NO];
  // [tvarNSOpenPanelObj setExtensionHidden:YES];
       [tvarNSOpenPanelObj setExtensionHidden:NO];
    [tvarNSOpenPanelObj setCanSelectHiddenExtension:YES];
    

    //[tvarNSOpenPanelObj layoutIfNeeded];
    //CALayer* pLayer = tvarNSOpenPanelObj.accessoryView.superview.layer;
    //pLayer.cornerRadius = 0;
	if(strPathInOut.length() == 0)
		strPathInOut = "Untitled";

    if(strPathInOut.length() > 0)
    {
        string strTemp;
        PathUtils::extractPureFileNameFromPath(strPathInOut.c_str(), strTemp);
        [tvarNSOpenPanelObj setNameFieldStringValue:[NSString stringWithCString:strTemp.c_str()]];

        PathUtils::extractPathFromFullFilename(strPathInOut.c_str(), strTemp);
        if(strTemp.length() > 0)
        {
            [tvarNSOpenPanelObj setDirectoryURL:[NSURL URLWithString:[NSString stringWithCString:strTemp.c_str()]]];
        }
    }
      
	NSObject* pAccessView = [MacFormWindowManager::getInstance()->getMainWindow()->myController getSaveAccessoryView];
	if(pAccessView)
	{
	    [pAccessView setParentSaveDialog:tvarNSOpenPanelObj];
		[pAccessView updateFileTypes:pFinalExensions pDescs:pFinalDescriptions selectExt:strPathInOut];
		[tvarNSOpenPanelObj setAccessoryView:pAccessView];
	}
	NSInteger tvarNSInteger	= [tvarNSOpenPanelObj runModal];
	[pAppleTypes release];
    
   
	if(tvarNSInteger != NSOKButton)
    {
       // [tvarNSOpenPanelObj release];
		return ResultCodeCancel;
	}
    
	//NSString * tvarDirectory = [tvarNSOpenPanelObj directory];
	NSString *tvarDirectory = [[tvarNSOpenPanelObj URL] path];
    
	strPathInOut = getCStringSafe(tvarDirectory);

	if(strPathInOut.length() > 0)
	{
		if(!FileUtils::canAccessFileForWriting(strPathInOut.c_str()))
		{
			strErrorOut = "Cannot access file at this location.";
			return ResultCodeError;
		}
		else
		{
			BookmarkManager::getInstance()->addBookmark([tvarNSOpenPanelObj URL]);
			return ResultCodeOk;
		}
	}
	else
		return ResultCodeCancel;

#if 0
	if(strPathInOut.length() > 0)
	{
		gLog("Path init: %s", strPathInOut.c_str());
    
#ifdef _DEBUG
    
        NSURL* pUrl = [tvarNSOpenPanelObj URL];
    /*
        NSString *arrayCompleto = @"bla bla bla";
        
        NSError *error = nil;
        [arrayCompleto writeToFile:[pUrl path]
                        atomically:NO
                          encoding:NSUTF8StringEncoding
                             error:&error];
        
        bool bSaveRes = [[NSFileManager defaultManager] createFileAtPath:[pUrl path] contents:[NSData data] attributes:nil];
        
        NSError *pError = NULL;
        NSData* data = [pUrl bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope includingResourceValuesForKeys:nil relativeToURL:nil error:&pError];

        NSURL* outURL = [NSURL URLByResolvingBookmarkData:data options:NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:nil error:nil];
 
        bool bResult = [outURL startAccessingSecurityScopedResource];
        FILE *out = fopen(strPathInOut.c_str(), "w");
        fprintf(out, "Test!\n");
        fclose(out);
        [outURL stopAccessingSecurityScopedResource];
         */
        /*
        NSError *pError = NULL;
        NSData* data = [pUrl bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope includingResourceValuesForKeys:nil relativeToURL:nil error:&pError];
         */
#endif
        
     //   string strExtOut;
       // [pAccessView getSelectedExtension:strExtOut];
       // gEnsurePathEndsInExtension(strPathInOut, strExtOut.c_str(), true);

        //[tvarNSOpenPanelObj release];
        
		gLog("Path after: %s", strPathInOut.c_str());        
        return true;
    }
	else
    {
      //  [tvarNSOpenPanelObj release];
		return false;
    }
#endif
#else
	return ResultCodeCancel;
#endif
}
/*****************************************************************************/
#ifdef IOS_BUILD
Window* Application::openNewWindow(Window* pParentWindow, int iResourceType, int iOptWidth, int iOptHeight, bool bThinFrame, bool bFixedSize, int iMinWidth, int iMinHeight, const char* pcsTitle, int iCenterX, int iCenterY)
{
    return NULL;
}
#else
/*****************************************************************************/
NSWindow* openNewWindowNative(NSRect pTargetRect, NSWindow *pNativeParentWindow, int iResourceType, bool bThinFrame, bool bFixedSize, 
								int iMinWidth, int iMinHeight, const char* pcsTitle, bool bIsMainWindow, const char* pcsMainMenuElemId, const char* pcsInitLayerToShow)
{
    NSWindowStyleMask iMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
    if(!bFixedSize)
        iMask |= NSWindowStyleMaskResizable;
    if(bThinFrame)
        iMask |= NSWindowStyleMaskBorderless;
    
    NSWindow* pWindow = [NSWindow alloc];
    [pWindow initWithContentRect:pTargetRect styleMask:iMask backing:NSBackingStoreBuffered defer:false];
    NSRect rInnerRect = NSMakeRect(0, 0, pTargetRect.size.width, pTargetRect.size.height);
    BaseGLView* pView = [[BaseGLView alloc] initWithFrame:rInnerRect];
    pWindow.contentView = pView;
    [pView release];
    BaseWindowController* pController = [[BaseWindowController alloc] init:pView withWindow:pWindow];
    [pView setMainController:pController];
    
    pWindow.delegate = pController;
    
   /// [pWindow orderFrontRegardless];
    
    if(iMinWidth > 0 && iMinHeight > 0)
    {
        NSSize szMinSize;
        szMinSize.width = iMinWidth;
        szMinSize.height = iMinHeight;
        [pWindow setMinSize:szMinSize];
    }
    /*
     if(iCenterX >= 0 && iCenterY >= 0)
     {
     [pCastCont.myMacWindow setFrame:g_pTargetRect display:NO animate:NO];
     [pCastCont.myMacWindow setFrameTopLeftPoint:rTopLeft];
     }
     */
    if(pcsTitle)
        [pWindow setTitle:[NSString stringWithCString:pcsTitle]];
    
   
 
    [pController createAndInitHyperWindow:iResourceType bIsMainWindow:bIsMainWindow pcsMainMenuElemId:pcsMainMenuElemId pcsWindowTitle:pcsTitle pcsInitLayerToShow:pcsInitLayerToShow];
    
    if(pNativeParentWindow)
        [pNativeParentWindow addChildWindow:pWindow ordered:NSWindowAbove];
    
    [pWindow makeKeyAndOrderFront:nil];
    [pWindow orderFrontRegardless];
    
    return pWindow;
}
/*****************************************************************************/
Window* Application::openNewWindow(Window* pParentWindow, int iResourceType, int iOptWidth, int iOptHeight, bool bThinFrame, bool bFixedSize, 
									int iMinWidth, int iMinHeight, const char* pcsTitle, int iCenterX, int iCenterY, bool bIsMainWindow, const char* pcsMainMenuElemId, const char* pcsInitLayerToShow)
{
	Window* pWindow = NULL;

	int iScalingFactor = getScreenDensityScalingFactor(HyperUI::getMaxScreenScalingFactor());

	if(iOptWidth > 0)
		iOptWidth /= iScalingFactor;
	if(iOptHeight > 0)
		iOptHeight /= iScalingFactor;
	if(iMinWidth > 0)
		iMinWidth /= iScalingFactor;
	if(iMinHeight > 0)
		iMinHeight /= iScalingFactor;
	// This might be wrong... may need to translate back to view coords then to screen coords...
	if(iCenterX > 0)
		iCenterX /= iScalingFactor;
	if(iCenterY > 0)
		iCenterY /= iScalingFactor;

	NSWindow *pNativeParentWindow = nil;
	NSResponder* pParentController = findFormByWindow(pParentWindow);
	if(pParentController)
		pNativeParentWindow = [pParentController getNativeWindow];

	NSRect g_pTargetRect;
	g_pTargetRect.origin.x = 0;
	g_pTargetRect.origin.y = 0;
	//MacChildController *controllerWindow = [[MacChildController alloc] initWithWindowNibName:@"ChildWindow"];
    if(iOptWidth > 0 && iOptHeight > 0)
	{
		g_pTargetRect.size.width = iOptWidth;
		g_pTargetRect.size.height = iOptHeight;
        //[controllerWindow setTargetRect:g_pTargetRect];
		//[controllerWindow.openGLView.window setFrame:targetRect display:YES animate:NO];
	}
    else
    {
		g_pTargetRect.size.width = 800;
		g_pTargetRect.size.height = 600;
    }
    
    const FLOAT_TYPE fBottomPadding = 90;
    
    NSPoint rTopLeft;
    NSScreen* pWindowScreen = [pNativeParentWindow screen];
    if(!pWindowScreen)
        pWindowScreen = [NSScreen mainScreen];
    NSRect nsScreenRect = [pWindowScreen visibleFrame];
    if(iCenterX < 0)
        iCenterX = nsScreenRect.size.width/2.0;
    if(iCenterY < 0)
        iCenterY = (nsScreenRect.size.height - fBottomPadding)/2.0;
    
    rTopLeft.x = iCenterX - iOptWidth/2;
    rTopLeft.y = iCenterY - iOptHeight/2;

    if(rTopLeft.x < 0)
        rTopLeft.x = 0;
    if(rTopLeft.x + iOptWidth > nsScreenRect.size.width)
        rTopLeft.x = nsScreenRect.size.width - iOptWidth;
    
    if(rTopLeft.y < 0)
        rTopLeft.y = 0;
    if(rTopLeft.y + iOptHeight > nsScreenRect.size.height - fBottomPadding)
        rTopLeft.y = nsScreenRect.size.height - fBottomPadding - iOptHeight;
    
    // The Y coordinate is actually from the bottom:
    rTopLeft.y = (nsScreenRect.size.height - 0) - (rTopLeft.y + iOptHeight);
    g_pTargetRect.origin = rTopLeft;
    
    NSWindow* pNativeWindow = openNewWindowNative(g_pTargetRect, pNativeParentWindow, iResourceType, bThinFrame, bFixedSize, iMinWidth, iMinHeight, pcsTitle, bIsMainWindow, pcsMainMenuElemId, pcsInitLayerToShow);
    
    pWindow = [pNativeWindow.contentView getWindow];
#if 0
    //[controllerWindow showWindow:g_pMainController];
   ///MacChildController *controllerWindow = [[MacChildController alloc] initWithWindowNibName:@"ChildWindow"];
	NSNib* pNib = [[NSNib alloc] initWithNibNamed:@"ChildWindow" bundle:nil];
    NSArray *pTopLevelObjects = nil;
    [pNib instantiateNibWithOwner:nil topLevelObjects:&pTopLevelObjects];
    int iCurr, iNum = [pTopLevelObjects count];
    for(iCurr = 0; iCurr < iNum; iCurr++)
    {
        id pObj = [pTopLevelObjects objectAtIndex:iCurr];
#if 1
        MacChildController* pCastCont = objc_dynamic_cast(pObj, MacChildController);
        if(pCastCont)
        {
            pCastCont.myNib = pNib;
            
		/*
			NSRect frame = [window frame];
			frame.size = theSizeYouWant;
			[window setFrame: frame display: YES animate: whetherYouWantAnimation];
			*/
			NSUInteger lStyleMask = [pCastCont.myMacWindow styleMask];

			if(bFixedSize)
				lStyleMask = lStyleMask & (~NSResizableWindowMask);
			else
				lStyleMask |= NSResizableWindowMask;	
			[pCastCont.myMacWindow setStyleMask:lStyleMask];

			if(iMinWidth > 0 && iMinHeight > 0)
			{
				NSSize szMinSize;
				szMinSize.width = iMinWidth;
				szMinSize.height = iMinHeight;
				[pCastCont.myMacWindow setMinSize:szMinSize];
			}

			if(iCenterX >= 0 && iCenterY >= 0)
			{
                [pCastCont.myMacWindow setFrame:g_pTargetRect display:NO animate:NO];
				[pCastCont.myMacWindow setFrameTopLeftPoint:rTopLeft];
			}

			if(pcsTitle)
			{
				[pCastCont.myMacWindow setTitle:[NSString stringWithCString:pcsTitle]];
			}
            [pNativeParentWindow addChildWindow:pCastCont.myMacWindow ordered:NSWindowAbove];
            [pCastCont.myMacWindow makeKeyAndOrderFront:nil];
		
			pWindow = pCastCont.myWindow;
            //[pCastCont finishInit:g_pTargetRect resourceColl:iResourceType];
        }
#endif
        //int bp = 0;
    }
#endif
	return pWindow;
}
#endif
/*****************************************************************************/
bool Application::getColorFromColorPicker(SColor& scolExistingColor, const char* pcsStartElemName, UNIQUEID_TYPE idParentWindow, SColor& scolNewOut)
{
#ifdef IOS_BUILD
#else
	string strOsVer = getCStringSafe(getOSVersionInfo());
    bool bForceShowNative = StringUtils::doesStartWith(strOsVer.c_str(), "10.6", false);

	bool bShowNative = SettingsCollection::getInstance()->getSettingsItem()->getBoolProp(PropertySetUseNativeColorPicker) || bForceShowNative;
	if(!bShowNative)
	{
		// Just open our main picker (or bring it to front) and return.
		WindowManager::getInstance()->getMainWindow()->getUIPlane()->showColorPicker(scolExistingColor, pcsStartElemName, idParentWindow);
		return false;
	}

	NSColorPanel *colorPanel = [NSColorPanel sharedColorPanel];
	theColorUiElemName = pcsStartElemName;
	
	[colorPanel orderFront:nil];
	[colorPanel setShowsAlpha:NO];

	[colorPanel setColor:[NSColor colorWithDeviceRed:scolExistingColor.r green:scolExistingColor.g blue:scolExistingColor.b alpha:1.0] ];

    colorPanel.delegate = MacFormWindowManager::getInstance()->getMainWindow()->myController;

	// TODO: Handle teh (void)changeColor:(id)s
#endif
	return false;
}
/*****************************************************************************/
}
