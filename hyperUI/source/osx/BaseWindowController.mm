#import <Cocoa/Cocoa.h>
#include "stdafx.h"
#include "MacCommon.h"
#include "BaseWindowController.h"
#include "BaseGlView.h"

using namespace HyperUI;

@implementation BaseWindowController

@synthesize myView;
/*****************************************************************************/
- (instancetype)init:(BaseGLView*)pView withWindow:(NSWindow*)pWindow
{
    [super init];

	myLastPenPressure = 1.0;    
    self.myView = pView;
    
    myCurrentTime = 0.0;
    myAccumulator = 0.0;
    myDrawTime = 0.0;
    myDrawDelta = 0.0;
#ifndef IGNORE_HYPER_STUFF
    myWindow = NULL;
#endif
    //NSRect applicationFrame = self.myView.bounds;
    NSWindow *win = self.myView.window;
    [win setAcceptsMouseMovedEvents:YES];
    
    myMacWindow = pWindow;

#ifndef IGNORE_HYPER_STUFF
    myTouches = new TTouchVector;
    
    myFormWrapper = new SMacFormWrapper;
    myFormWrapper->myController = self;
    MacFormWindowManager::getInstance()->onWindowCreated(myFormWrapper, false);
#endif
#ifdef EARLY_WINDOW_OBJECT_CREATION
    myWindow = Application::getInstance()->allocateWindow(upToScreen(400), upToScreen(300), false);
    
    myWindow->getTextureManager()->initFromCollection(ResourceManager::getInstance()->getCollection(ResourcePrelimAnimations), NULL);
    myWindow->finishPrelimInit((ResourceType)g_eResourceType);
#endif
    
    // Activate the display link now
    [self.myView startAnimation];
    
    return self;
}
/*****************************************************************************/
-(HyperUI::Window*)createAndInitHyperWindow:(int)iResourceType bIsMainWindow:(bool)bIsMainWindow pcsMainMenuElemId:(const char*)pcsMainMenuElemId pcsWindowTitle:(const char*)pcsWindowTitle pcsInitLayerToShow:(const char*)pcsInitLayerToShow
{
	int iXSize = self.myView.frame.size.width;
	int iYSize = self.myView.frame.size.height;

#ifndef IGNORE_HYPER_STUFF
    
#ifdef EARLY_WINDOW_OBJECT_CREATION
    myWindow->onWindowSizeChanged(iXSize, iYSize);
    myWindow->getDrawingCache()->reloadAllTextures();
#else
    myWindow = Application::getInstance()->allocateWindow(iResourceType, iXSize, iYSize, bIsMainWindow);

	TCollectionDefMap* pAnimCollections = ResourceManager::getInstance()->getCollectionsForRole(CollectionRoleGraphics);
	TCollectionDefMap::iterator mi;
	for(mi = pAnimCollections->begin(); mi != pAnimCollections->end(); mi++)
		myWindow->getTextureManager()->initFromCollection(ResourceManager::getInstance()->getCollection(mi->second.myType), mi->second.myTargetDataFolderPath.c_str(), NULL);

    myWindow->finishPrelimInit((ResourceType)iResourceType, pcsInitLayerToShow);
#endif
#endif

	if(bIsMainWindow)
	{
		MainMenuManager::getInstance()->createDefaultMenuItem(self, pcsWindowTitle);
		if(IS_VALID_STRING_AND_NOT_NONE(pcsMainMenuElemId))
		{
			IMenuContentsProvider* pRecentFilesProvider = Application::getInstance()->allocateRecentFilesProvider();
			MainMenuManager::getInstance()->generateMainMenu(pcsMainMenuElemId, self, pRecentFilesProvider);
			delete pRecentFilesProvider;
		}
	}
    
    /*
    if(g_pTargetRect.size.width > 0)
    {
        [self.myView.window setFrame:g_pTargetRect display:NO animate:NO];
        [self.myView.window setFrameTopLeftPoint:g_pTargetRect.origin];
    }
    */
    //[self finishInit:g_pTargetRect resourceColl:g_eResourceType];
    return myWindow;
}
/*****************************************************************************/
- (void)menuWillOpen:(NSMenu*)menu
{
	MainMenuManager::getInstance()->onNativeMenuWillOpen(menu);
}
/*****************************************************************************/
-(void)onCustomMenuItem:(id)obj
{
	MainMenuManager::getInstance()->onNativeCustomMenuItem((NSMenuItem*)obj);
}
/*****************************************************************************/
-(void)onCustomRecentFilesMenuItem:(id)obj
{
	MainMenuManager::getInstance()->onNativeRecentFilesMenuItem((NSMenuItem*)obj);
}
/*****************************************************************************/
- (void)windowDidBecomeKey:(NSNotification *)notification
{
    gLog("Window became key\n");
}
/*****************************************************************************/
- (void)windowDidResize:(NSNotification *)notification
{
    NSRect windowSize = self.myView.window.contentLayoutRect;
    
    self.myView.frame = NSMakeRect(0, 0, windowSize.size.width, windowSize.size.height);
}
/*****************************************************************************/
- (void) dealloc
{
    LOG_QUIT_ROUTINE("CHILD_CONTROLLER: dealloc\n");
#ifndef IGNORE_HYPER_STUFF
    MacFormWindowManager::getInstance()->onWindowDestroyed(myFormWrapper, true);
#endif
    
    [self.myView stopAnimation];
#ifndef IGNORE_HYPER_STUFF
    delete myTouches;
#endif
    LOG_QUIT_ROUTINE("CHILD_CONTROLLER: touches deleted\n");
    
    [self.myView onShutdown];
    self.myView.window.contentView = nil;
    
    LOG_QUIT_ROUTINE("CHILD_CONTROLLER: mNib released\n");
    
    [self deleteWindow];
    [super dealloc];
    
    LOG_QUIT_ROUTINE("CHILD_CONTROLLER: dealloc done\n");
}
/*****************************************************************************/
-(NSWindow*)getNativeWindow
{
    return myMacWindow;
}
/*****************************************************************************/
-(HyperUI::Window*)getWindow
{
    return myWindow;
}
/*****************************************************************************/
#ifndef IGNORE_HYPER_STUFF
#else
bool getHaveCommandKey(NSEvent *event) { return false; }
bool getHaveAltKey(NSEvent *event) { return false; }
bool getHaveShiftKey(NSEvent* event) { return false; }
bool getHaveControlKey(NSEvent *event) { return false; }
int mapVirtualKeyCode(NSEvent *event, bool &bIsAffectingCommandOut, bool &bIsAffectingAltOut, bool &bIsAffectingShiftOut, bool &bIsAffectingActualControlOut)
{
    return 'a';
}
#endif
/*****************************************************************************/
-(BOOL)keyUpCommon:(NSEvent *)event
{
#ifndef IGNORE_HYPER_STUFF
    bool bPrevIsCommandDown = KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyControl);
    bool bPrevIsAltDown = KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyAlt);
    bool bPrevIsShfitDown = KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyShift);
    bool bPrevIsActualControlDown = KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyMacControl);
#else
    bool bPrevIsCommandDown = false;
    bool bPrevIsAltDown = false;
    bool bPrevIsShfitDown = false;
    bool bPrevIsActualControlDown = false;
#endif
    bool bIsCommandDown, bIsShiftDown, bIsAltDown, bIsActualControlDown;
    int iScanCode = mapVirtualKeyCode(event, bIsCommandDown, bIsAltDown, bIsShiftDown, bIsActualControlDown);
    
    // Note that for the control keys, we need to negate them, because at this point on Mac, they reflect
    // their current state. That is, if bIsAltDown == false, that means it is not pressed - whether it was
    // before or not. On Windows, and so in engine, we assume the opposite - true here means it was just
    // unpressed. However, we need the previous state, too...
    bool bIsReleasingAlt = false;
    bool bIsReleasingCommand = false;
    bool bIsReleasingShift = false;
    bool bIsReleasingActualControl = false;
    if(bPrevIsAltDown && !bIsAltDown)
        bIsReleasingAlt = true;
    if(bPrevIsCommandDown && !bIsCommandDown)
        bIsReleasingCommand = true;
    if(bPrevIsShfitDown && !bIsShiftDown)
        bIsReleasingShift = true;
    if(bPrevIsActualControlDown && !bIsActualControlDown)
        bIsReleasingActualControl = true;
#ifndef IGNORE_HYPER_STUFF
    Application::lockGlobal();
    bool bHandled = myWindow->onKeyUp(iScanCode, bIsReleasingCommand, bIsReleasingAlt, bIsReleasingShift, bIsReleasingActualControl);
    Application::unlockGlobal();
#else
    bool bHandled = false;
#endif
    return bHandled;
}
/*****************************************************************************/
- (void) keyUp:(NSEvent *)event
{
    [self keyUpCommon:event];
}
/*****************************************************************************/
- (BOOL) keyDownCommon:(NSEvent *)event
{
    bool bIsCommandDown, bIsShiftDown, bIsAltDown, bIsActualControlDown;
    int iScanCode = mapVirtualKeyCode(event, bIsCommandDown, bIsAltDown, bIsShiftDown, bIsActualControlDown);
#ifndef IGNORE_HYPER_STUFF
    Application::lockGlobal();
    bool bHandled = myWindow->onKeyDown(iScanCode, bIsCommandDown, bIsAltDown, bIsShiftDown, bIsActualControlDown);
    Application::unlockGlobal();
#else
    bool bHandled = false;
#endif
    return bHandled;
}
/*****************************************************************************/
- (void) keyDown:(NSEvent *)event
{
    [self keyDownCommon:event];
}
/*****************************************************************************/
- (BOOL)performKeyEquivalent:(NSEvent *)event
{
    // Hack: avoid calling Ctrl+Tab twice.
    // This used to be for the above. But now this seems for
    // handling some combinations, of which tab+control is one.
    // The rest are sent normally to keydown/up.
    bool bIsCommandDown, bIsShiftDown, bIsAltDown, bIsActualControlDown;
    int iScanCode = mapVirtualKeyCode(event, bIsCommandDown, bIsAltDown, bIsShiftDown, bIsActualControlDown);
    if(bIsActualControlDown && iScanCode == SilentKeyTab)
    {
        //gLog("performKeyEquivalent called...\n");
        bool bResult = [self keyDownCommon:event];
        // Match this, otherwise we end up with no ability to zoom in twice with cmd++
        // on a mac, since the last key flag is not reset.
        //[self keyUpCommon:event];
        return bResult;
    }
    else
        return NO;
    
}
/*****************************************************************************/
- (void)flagsChanged:(NSEvent *)event
{
    bool bIsCommandDown = getHaveCommandKey(event);
    bool bIsAltDown = getHaveAltKey(event);
    bool bIsShiftDown = getHaveShiftKey(event);
    bool bIsActualControlDown = getHaveControlKey(event);
#ifndef IGNORE_HYPER_STUFF
    bool bPrevIsCommandDown = KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyControl);
    bool bPrevIsAltDown = KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyAlt);
    bool bPrevIsShfitDown = KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyShift);
    bool bPrevIsActualControlDown = KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyMacControl);
#else
    bool bPrevIsCommandDown = false;
    bool bPrevIsAltDown = false;
    bool bPrevIsShfitDown = false;
    bool bPrevIsActualControlDown = false;
#endif
    bool bIsBeingReleased = false;
    if( (bPrevIsCommandDown && !bIsCommandDown)
       || (bPrevIsAltDown && !bIsAltDown)
       || (bPrevIsShfitDown && !bIsShiftDown)
       || (bPrevIsActualControlDown && !bIsActualControlDown) )
        bIsBeingReleased = true;
    
    if(bIsBeingReleased)
        [self keyUp:event];
    else
        [self keyDown:event];
}
/*****************************************************************************/
- (void)deleteWindow
{
    delete myWindow;
    myWindow = NULL;
}
/*****************************************************************************/
- (void)mouseDraggedCore:(FLOAT_TYPE)fWindowX fWindowY:(FLOAT_TYPE)fWindowY
{
#ifndef IGNORE_HYPER_STUFF
    myTouches->clear();
    STouchInfo rTouchInfo;
    rTouchInfo.myPoint.set(fWindowX, fWindowY);
    rTouchInfo.myTouchId = (void*)1;
    myTouches->push_back(rTouchInfo);
    
    myWindow->onTouchMove(*myTouches);
#endif
}
/*****************************************************************************/
+ (NSPoint)convertPoint:(NSPoint)ptIn view:(NSView*)view svScreenDims:(SVector2D)svScreenDims
{
    NSPoint windowPoint = ptIn;
    windowPoint = [view convertPoint:windowPoint fromView:nil];
    if([view respondsToSelector:@selector(convertPointToBacking)])
    { windowPoint = [view convertPointToBacking:windowPoint]; }
    windowPoint.y = svScreenDims.y - windowPoint.y;
    return windowPoint;
}
/*****************************************************************************/
- (void)mouseDownDel:(NSEvent *)theEvent eMouseButton:(MouseButtonType)eMouseButton view:(NSView*)view
{
    NSEventType evType = [theEvent type];
#ifndef IGNORE_HYPER_STUFF
    myLastPenPressure = [theEvent pressure];
    if( (evType == NSLeftMouseDown || evType == NSLeftMouseDragged) && myLastPenPressure <= 0.0)
        myLastPenPressure = 1.0;
    
    Application::lockGlobal();
    
    SVector2D svScreenDims;
    myWindow->getSize(svScreenDims);
    
    NSPoint windowPoint;
    windowPoint = [theEvent locationInWindow];
    windowPoint = [BaseWindowController convertPoint:windowPoint view:view svScreenDims:svScreenDims];
    
    myTouches->clear();
    
    STouchInfo rTouchInfo;
    rTouchInfo.myPoint.set(windowPoint.x, windowPoint.y);
    rTouchInfo.myTouchId = (void*)1;
    myTouches->push_back(rTouchInfo);
    
    myWindow->onTouchDown(*myTouches, eMouseButton);
#else
    SVector2D windowPoint;
#endif
    [self mouseDraggedCore:windowPoint.x fWindowY:windowPoint.y];
#ifndef IGNORE_HYPER_STUFF
    Application::unlockGlobal();
#endif
}
/*****************************************************************************/
-(void)mouseEntered:(NSEvent *)theEvent
{
#ifndef IGNORE_HYPER_STUFF
    if(myWindow)
    {
        Application::lockGlobal();
        myWindow->getTooltipManager()->setIsEnabled(true);
        Application::unlockGlobal();
    }
#endif
}
/*****************************************************************************/
-(void)mouseMovedCore:(FLOAT_TYPE)fWindowX fWindowY:(FLOAT_TYPE)fWindowY
{
#ifndef IGNORE_HYPER_STUFF
    Application::lockGlobal();
    
    //g_svLastMousePos.set(fWindowX, fWindowY);
    
    myTouches->clear();
    STouchInfo rTouchInfo;
    rTouchInfo.myPoint.set(fWindowX, fWindowY);
    rTouchInfo.myTouchId = (void*)1;
    myTouches->push_back(rTouchInfo);
    myWindow->onCursorGlide(*myTouches);
    
    Application::unlockGlobal();
#endif
}
/*****************************************************************************/
-(void)mouseMoved:(NSEvent *)theEvent view:(NSView*)view
{
    SVector2D svScreenDims;
#ifndef IGNORE_HYPER_STUFF
    Application::lockGlobal();
    myWindow->getSize(svScreenDims);
    Application::unlockGlobal();
    
    NSPoint windowPoint;
    windowPoint = [theEvent locationInWindow];
    windowPoint = [BaseWindowController convertPoint:windowPoint view:view svScreenDims:svScreenDims];

#else
    SVector2D windowPoint;
#endif
    [self mouseMovedCore:windowPoint.x fWindowY:windowPoint.y];
    
}
/*****************************************************************************/
- (void)mouseDragged:(NSEvent *)theEvent view:(NSView*)view
{
    NSEventType evType = [theEvent type];
#ifndef IGNORE_HYPER_STUFF
    myLastPenPressure = [theEvent pressure];
    if( (evType == NSLeftMouseDown || evType == NSLeftMouseDragged) && myLastPenPressure <= 0.0)
        myLastPenPressure = 1.0;
    
    Application::lockGlobal();
#endif
    SVector2D svScreenDims;
    myWindow->getSize(svScreenDims);
    NSPoint windowPoint;
    windowPoint = [theEvent locationInWindow];
    windowPoint = [BaseWindowController convertPoint:windowPoint view:view svScreenDims:svScreenDims];
    
    [self mouseDraggedCore:windowPoint.x fWindowY:windowPoint.y];
#ifndef IGNORE_HYPER_STUFF
    Application::unlockGlobal();
#endif
}
/*****************************************************************************/
-(void)mouseUpCore:(FLOAT_TYPE)fWindowX fWindowY:(FLOAT_TYPE)fWindowY eMouseButton:(MouseButtonType)eMouseButton
{
#ifndef IGNORE_HYPER_STUFF
    Application::lockGlobal();
    
    myTouches->clear();
    STouchInfo rTouchInfo;
    rTouchInfo.myPoint.set(fWindowX, fWindowY);
    rTouchInfo.myTouchId = (void*)1;
    myTouches->push_back(rTouchInfo);
    
    if(myWindow)
        myWindow->onTouchUp(*myTouches);
    Application::unlockGlobal();
#endif
}
/*****************************************************************************/
- (void)mouseUp:(NSEvent *)theEvent eMouseButton:(MouseButtonType)eMouseButton view:(NSView*)view
{
#ifndef IGNORE_HYPER_STUFF
    SVector2D svScreenDims;
    Application::lockGlobal();
    myWindow->getSize(svScreenDims);
    Application::unlockGlobal();
    NSPoint windowPoint;
    windowPoint = [theEvent locationInWindow];
    windowPoint = [BaseWindowController convertPoint:windowPoint view:view svScreenDims:svScreenDims];
#else
    SVector2D windowPoint;
#endif
    [self mouseUpCore:windowPoint.x fWindowY:windowPoint.y eMouseButton:eMouseButton];
}
/*****************************************************************************/
-(void)mouseExited:(NSEvent *)theEvent
{
    
    // Also, this:
    [self mouseUpCore:-100 fWindowY:-100 eMouseButton:MouseButtonLeft];
    if(myWindow)
    {
#ifndef IGNORE_HYPER_STUFF
        Application::lockGlobal();
        
        //if(fullScreenView)
        //    myWindow->getTooltipManager()->setIsEnabled(true);
        //else
        myWindow->getTooltipManager()->setIsEnabled(false);
        
        myWindow->onTouchLeave();
        Application::unlockGlobal();
#endif
    }
    
    [self mouseMovedCore:-100 fWindowY:-100];
}
/*****************************************************************************/
- (void)scrollWheel:(NSEvent *)theEvent view:(NSView*)view
{
    SVector2D svScreenDims;

    Application::lockGlobal();
    if(myWindow)
        myWindow->getSize(svScreenDims);
    Application::unlockGlobal();
    NSPoint windowPoint;
    windowPoint = [theEvent locationInWindow];
    windowPoint = [BaseWindowController convertPoint:windowPoint view:view svScreenDims:svScreenDims];   
    FLOAT_TYPE fDelta = [theEvent scrollingDeltaY];
    SVector2D svPoint(windowPoint.x, windowPoint.y);
    
    bool bHavePrecise = [theEvent hasPreciseScrollingDeltas];
    
    if(myWindow)
    {
        if(bHavePrecise)
            fDelta /= 10.0;
        
        Application::lockGlobal();
#ifndef IGNORE_HYPER_STUFF
        myWindow->onMouseWheel(svPoint, fDelta);
#endif
        Application::unlockGlobal();
    }

}
/*****************************************************************************/
// Holding the mouse button and dragging the mouse changes the "roll" angle (y-axis) and the direction from which sunlight is coming (x-axis).
- (void)mouseDown:(NSEvent *)theEvent eMouseButton:(MouseButtonType)eMouseButton view:(NSView*)view
{
    BOOL dragging = YES;
    BOOL wasAnimating = NO;
    bool bDidPause = false;
#ifndef IGNORE_HYPER_STUFF
    if (wasAnimating && myWindow && myWindow->getTextureManager()->isInitializing())
    {
        bDidPause = true;
        [self stopAnimation];
    }
#endif
    
    [self mouseDownDel:theEvent eMouseButton:eMouseButton view:view];
    
    while (dragging)
    {
        theEvent = [[self.myView window] nextEventMatchingMask:NSAnyEventMask];
        switch ([theEvent type])
        {
            case NSLeftMouseUp:
                [self mouseUp:theEvent eMouseButton:MouseButtonLeft view:view];
                dragging = NO;
                break;
                
            case NSRightMouseUp:
                [self mouseUp:theEvent eMouseButton:MouseButtonRight view:view];
                dragging = NO;
                break;
                
            case NSKeyDown:
                [self keyDown:theEvent];
                break;
                
            case NSKeyUp:
                [self keyUp:theEvent];
                break;
                
            case NSScrollWheel:
                [self scrollWheel:theEvent view:view];
                break;
                
            case NSFlagsChanged:
                [self flagsChanged:theEvent];
                break;
                
            case NSRightMouseDragged:
            case NSLeftMouseDragged:
                [self mouseDragged:theEvent view:view];
                
                if(bDidPause)
                {
                    [self.myView display];
                }
                
                break;
                
            default:
                
            {
                [self.myView display];
            }
                
                break;
        }
        
        
    }
    
    if (wasAnimating && bDidPause)
    {
        [self startAnimation];
    }
}
/*****************************************************************************/
#define kGameFrameStepInterval        1.0/GAME_FRAMERATE
#define kGameFrameDrawInterval        1.0/(GAME_FRAMERATE*1.1)
#define kGameFrameMaxDelta            1.0/10.0

-(double)gameRunFrame:(Window*)pWindow pController:(BaseWindowController*)pController
{
    double newTime, deltaTime;
#ifndef IGNORE_HYPER_STUFF
    if(!pWindow)
        return 0;
    
    newTime = DateUtils::getCurrentTime();
    deltaTime = newTime - myCurrentTime;
    
    if (deltaTime > kGameFrameMaxDelta)
	{
        deltaTime = kGameFrameMaxDelta;
    } 
	else if(deltaTime <= 0.0) 
	{
        deltaTime = 0.0;
        myDrawTime = 0.0;
        myDrawDelta = 0.0;
    }
    
    myCurrentTime = newTime;
    
    myAccumulator += deltaTime;
    while (myAccumulator >= kGameFrameStepInterval)
    {
        myAccumulator -= kGameFrameStepInterval;
        pWindow->onTimerTickBegin();
        pWindow->onTimerTickEnd();
    }
    
    myDrawDelta += deltaTime;
    if (myCurrentTime > myDrawTime) 
	{
        myDrawTime = myCurrentTime + kGameFrameDrawInterval;
        deltaTime = myDrawDelta;
        myDrawDelta = 0.0;
        return deltaTime;
    }
#else
    return 1.0;
#endif
    return -1.0;
}
/*****************************************************************************/
-(bool)drawView
{
    bool bDidDraw = false;
    Application::lockGlobal();

#ifndef IGNORE_HYPER_STUFF
    bool bIsActive = ActivityManager::getInstance()->getIsActive();
#else
    bool bIsActive = true;
    bDidDraw = true;
#endif
    double dt = [self gameRunFrame:myWindow pController:self];
    
    if(dt > 0 && bIsActive)
    {
#ifndef USE_OPENGL2
        glLoadIdentity();
#endif
#ifndef IGNORE_HYPER_STUFF
#else
        glClearColor(1, 0, 0, 0);
#endif
        glClear(GL_COLOR_BUFFER_BIT); //  | GL_STENCIL_BUFFER_BIT);
        
        {
#ifndef IGNORE_HYPER_STUFF
            myWindow->getDrawingCache()->getTextCacher()->enableTextCaching();
            myWindow->renderToTexture();
            glClear(GL_COLOR_BUFFER_BIT);
            
            SVector2D svScreenDims;
            myWindow->getSize(svScreenDims);
            
            GraphicsUtils::transformForGlOrtho(svScreenDims.x, svScreenDims.y);
            
            myWindow->setBlendMode(BlendModeNormal);
            myWindow->render();
#endif
            bDidDraw = true;
        }
    }
    
    Application::unlockGlobal();
    return bDidDraw;
}
/*****************************************************************************/
- (void)windowWillClose:(NSNotification *)notification
{
    //NSWindow* pWindow = (NSWindow*)[notification object];
    [self release];
}
/*****************************************************************************/
template < class ID_CLASS >
UNIQUEID_TYPE GlobalIdGeneratorInternal<ID_CLASS>::myGeneratorLastId = 0;

template < class ITEM_TYPE >
HYPERUI_API TWindowManager<ITEM_TYPE>* TWindowManager<ITEM_TYPE>::theInstance = NULL;
/*****************************************************************************/
@end
