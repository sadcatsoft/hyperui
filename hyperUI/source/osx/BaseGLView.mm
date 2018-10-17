#import <Cocoa/Cocoa.h>
#include "stdafx.h"
#include "MacCommon.h"
#include "BaseGlView.h"
#include "BaseWindowController.h"

using namespace HyperUI;

@implementation BaseGLView
/*****************************************************************************/
- (instancetype)initWithFrame:(NSRect)frameRect
{
    [super initWithFrame:frameRect];
    
    LOG_QUIT_ROUTINE("CHILD_WINDOW: Awake from nib\n");
    
    myHasShutdown = false;
    
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 24,
        // Must specify the 3.2 Core Profile to use OpenGL 3.2
#if defined(USE_OPENGL2)
        NSOpenGLPFAOpenGLProfile,  (NSOpenGLPixelFormatAttribute)NSOpenGLProfileVersion3_2Core,
#endif
        0
    };
    
    NSOpenGLPixelFormat *pf = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs] autorelease];
    
    if (!pf)
    {
        NSLog(@"No OpenGL pixel format");
    }
    
    NSOpenGLContext* context = [[[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil] autorelease];
    
#if defined(USE_OPENGL2) && defined(DEBUG)
    // When we're using a CoreProfile context, crash if we call a legacy OpenGL function
    // This will make it much more obvious where and when such a function call is made so
    // that we can remove such calls.
    // Without this we'd simply get GL_INVALID_OPERATION error for calling legacy functions
    // but it would be more difficult to see where that function was called.
    CGLEnable([context CGLContextObj], kCGLCECrashOnRemovedFunctions);
#endif
    
    [self setPixelFormat:pf];
    
    [self setOpenGLContext:context];
    
#ifdef USE_OPENGL2
    gInitShaders();
#endif
    
    // Look for changes in view size
    // Note, -reshape will not be called automatically on size changes because NSView does not export it to override
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(reshape)
                                                 name:NSViewGlobalFrameDidChangeNotification
                                               object:self];
    
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(reshapeView)
                                                 name:NSViewFrameDidChangeNotification
                                               object:self];
    
#ifdef ENABLE_RETINA_SUPPORT
    bool bResponds = [self respondsToSelector:@selector(setWantsBestResolutionOpenGLSurface)];
    StringResourceItem* pStartupSettings = SettingsCollection::getStartupSettings();
    if(bResponds
#ifndef IMAGE_VIEWER_MODE
       && pStartupSettings->getBoolProp(START_SETTINGS_ALLOW_RETINA)
#endif
       )
    {
        [self  setWantsBestResolutionOpenGLSurface:YES];
    }
#endif
    
    LOG_QUIT_ROUTINE("CHILD_WINDOW: Awake from nib done\n");
    
    return self;
}
/*****************************************************************************/
- (Window*) getWindow
{
    return [self->myMainController getWindow];
}
/*****************************************************************************/
- (void) setMainController:(BaseWindowController*)theController
{
    self->myMainController = theController;
}
/*****************************************************************************/
- (void) prepareOpenGL
{
    [super prepareOpenGL];
    
    [[self openGLContext] makeCurrentContext];
    
    // Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    // Create a display link capable of being used with all active displays
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    
    // Set the renderer output callback function
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
    
    // Set the display link for the current renderer
    CGLContextObj cglContext = (CGLContextObj)[[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    
#ifndef IGNORE_HYPER_STUFF
    /*
    if(!myWindow)
    {
        // Should be called in window creation func
        //myWindow = [BaseWindowController createAndInitHyperWindow:[self scaleBoundsToPixels:self.bounds]];
        //self.myMainController.myWindow = myWindow;
    }
    */
    Window* pWindow = [self getWindow];
    SVector2D svSize;
    pWindow->getSize(svSize);
    pWindow->setupGlView(svSize.x, svSize.y);
#endif
    // Activate the display link
    CVDisplayLinkStart(displayLink);
    
    // Register to be notified when the window closes so we can stop the displaylink
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowWillClose:)
                                                 name:NSWindowWillCloseNotification
                                               object:[self window]];
}
/*****************************************************************************/
- (void) startAnimation
{
    if (displayLink && !CVDisplayLinkIsRunning(displayLink))
        CVDisplayLinkStart(displayLink);
}
/*****************************************************************************/
- (void) stopAnimation
{
    if (displayLink && CVDisplayLinkIsRunning(displayLink))
        CVDisplayLinkStop(displayLink);
}
/*****************************************************************************/
- (void) windowWillClose:(NSNotification*)notification
{
    // Stop the display link when the window is closing because default
    // OpenGL render buffers will be destroyed.  If display link continues to
    // fire without renderbuffers, OpenGL draw calls will set errors.
    
    CVDisplayLinkStop(displayLink);
}
/*****************************************************************************/
- (CVReturn) getFrameForTime:(const CVTimeStamp*)outputTime
{
    // There is no autorelease pool when this method is called because it will be called from a background thread
    // It's important to create one or you will leak objects
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    // Update the animation
    //    CFAbsoluteTime currentTime = CFAbsoluteTimeGetCurrent();
    //    [[myMainController scene] advanceTimeBy:(currentTime - [controller renderTime])];
    //    [myMainController setRenderTime:currentTime];
    
    [self drawView];
    
    [pool release];
    return kCVReturnSuccess;
}
/*****************************************************************************/
- (void) lockFocus
{
    [super lockFocus];
}
/*****************************************************************************/
-(CGRect)scaleBoundsToPixels:(CGRect)srBounds
{
    // return srBounds;
    //return CGRectMake(srBounds.origin.x, srBounds.origin.y, srBounds.size.width*2, srBounds.size.height*2);
    // return CGRectMake(srBounds.origin.x, srBounds.origin.y, srBounds.size.width*4, srBounds.size.height*4);
    
    NSRect backingBounds;
    if([self respondsToSelector:@selector(convertRectToBacking:)])
    { backingBounds = [self convertRectToBacking:srBounds]; }
    else
    {
        backingBounds.size.width = srBounds.size.width;
        backingBounds.size.height = srBounds.size.height;
    }
    return CGRectMake(srBounds.origin.x, srBounds.origin.y, backingBounds.size.width, backingBounds.size.height);
    
    //    NSRect backingBounds = [self convertRectToBacking:srBounds];
    //    return CGRectMake(srBounds.origin.x, srBounds.origin.y, backingBounds.size.width, backingBounds.size.height);
}
/*****************************************************************************/
-(void)unlockFocus
{
    [super unlockFocus];
}
/*****************************************************************************/
- (void) reshapeView
{
    int iNewW = [self scaleBoundsToPixels:self.bounds].size.width;
    int iNewH = [self scaleBoundsToPixels:self.bounds].size.height;
    //int bp = 0;
#ifndef IGNORE_HYPER_STUFF
    Application::lockGlobal();
    Window* pWindow = [self->myMainController getWindow];
    if(pWindow)
        pWindow->onWindowSizeChanged(iNewW, iNewH);
    Application::unlockGlobal();
#endif
}
/*****************************************************************************/
- (void) reshape
{
    if(myHasShutdown)
        return;
    
    // This method will be called on the main thread when resizing, but we may be drawing on a secondary thread through the display link
    // Add a mutex around to avoid the threads accessing the context simultaneously
    CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    // Delegate to the scene object to update for a change in the view size
    ///    [[myMainController scene] setViewportRect:[self bounds]];
    int iNewW = [self scaleBoundsToPixels:self.bounds].size.width;
    int iNewH = [self scaleBoundsToPixels:self.bounds].size.height;
    [self removeTrackingRect:myTrackingRectId];
#ifndef IGNORE_HYPER_STUFF
    Application::lockGlobal();

    Window* pWindow = [self->myMainController getWindow];
    if(pWindow)
        pWindow->onWindowSizeChanged(iNewW, iNewH);
    Application::unlockGlobal();
#endif
    [[self openGLContext] update];
    
    myTrackingRectId = [self addTrackingRect:self.frame owner:self userData:nil assumeInside:YES];
    
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    
}
/*****************************************************************************/
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
	// This is the renderer output callback function
    CVReturn result = [(BaseGLView*)displayLinkContext getFrameForTime:outputTime];
    return result;
}
/*****************************************************************************/
- (void) drawRect:(NSRect)dirtyRect
{
    // Ignore if the display link is still running
    if (!CVDisplayLinkIsRunning(displayLink))
        [self drawView];
}
/*****************************************************************************/
- (void) drawView
{
    if(myHasShutdown)
        return;
    
    // This method will be called on both the main thread (through -drawRect:) and a secondary thread (through the display link rendering loop)
    // Also, when resizing the view, -reshape is called on the main thread, but we may be drawing on a secondary thread
    // Add a mutex around to avoid the threads accessing the context simultaneously
    CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    
    // Make sure we draw to the right context
    [[self openGLContext] makeCurrentContext];
    
    // Delegate to the scene object for rendering
    // [[myMainController scene] render];
    if([myMainController drawView])
        [[self openGLContext] flushBuffer];
    
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}
/*****************************************************************************/
- (BOOL) acceptsFirstResponder
{
    // We want this view to be able to receive key events
    return YES;
}
/*****************************************************************************/
- (void) keyDown:(NSEvent *)theEvent
{
    // Delegate to the controller object for handling key events
    [myMainController keyDown:theEvent];
}
/*****************************************************************************/
- (void) keyUp:(NSEvent *)theEvent
{
    // Delegate to the controller object for handling key events
    [myMainController keyUp:theEvent];
}
/*****************************************************************************/
- (BOOL)performKeyEquivalent:(NSEvent *)event
{
    return [myMainController performKeyEquivalent:event];
}
/*****************************************************************************/
- (void)flagsChanged:(NSEvent *)event
{
    [myMainController flagsChanged:event];
}
/*****************************************************************************/
- (void)mouseDown:(NSEvent *)theEvent
{
    // Delegate to the controller object for handling mouse events
    [myMainController mouseDown:theEvent eMouseButton:MouseButtonLeft view:self];
}
/*****************************************************************************/
- (void)rightMouseDown:(NSEvent *)theEvent
{
    // Delegate to the controller object for handling mouse events
    [myMainController mouseDown:theEvent eMouseButton:MouseButtonRight view:self];
}
/*****************************************************************************/
-(void)mouseMoved:(NSEvent *)theEvent
{
    // Delegate to controller. NOTE: THis is special!
    [myMainController mouseMoved:theEvent view:self];
}
/*****************************************************************************/
-(void)mouseEntered:(NSEvent *)theEvent
{
    [myMainController mouseEntered:theEvent];
}
/*****************************************************************************/
-(void)mouseExited:(NSEvent *)theEvent
{
    [myMainController mouseExited:theEvent];
}
/*****************************************************************************/
- (void)scrollWheel:(NSEvent *)theEvent
{
    [myMainController scrollWheel:theEvent view:self];
}
/*****************************************************************************/
-(void)deallocOwnWindow
{
#ifndef IGNORE_HYPER_STUFF
    [self->myMainController deleteWindow];

   // delete myWindow;
  //  myWindow = NULL;
#endif
}
/*****************************************************************************/
-(void)onShutdown
{
    
    LOG_QUIT_ROUTINE("CHILD_NOTIFCATION_CENTER: onShutdown");
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:NSWindowWillCloseNotification
                                                  object:[self window]];
    
    LOG_QUIT_ROUTINE("CHILD_NOTIFCATION_CENTER: Removed self as observer on window will close");
    
    
    [self removeTrackingRect:myTrackingRectId];
    
    [self unregisterDraggedTypes];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:NSViewGlobalFrameDidChangeNotification
                                                  object:self];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:NSViewFrameDidChangeNotification
                                                  object:self];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    LOG_QUIT_ROUTINE("CHILD_NOTIFCATION_CENTER: onShutdown done");
}
/*****************************************************************************/
- (void) dealloc
{
    LOG_QUIT_ROUTINE("CHILD_DEALLOC: called");
    
    // Stop and release the display link
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
    myHasShutdown = true;
    
    LOG_QUIT_ROUTINE("CHILD_DEALLOC: display link stopped");
    
    [self onShutdown];
#ifndef IGNORE_HYPER_STUFF
   // [self->myMainController deleteWindow];
   // Window* pWindow = [self->myMainController getWindow];
  //  delete myWindow;
  //  myWindow = NULL;
#endif
    [super dealloc];
    
    LOG_QUIT_ROUTINE("CHILD_DEALLOC: super dealloc done, about to refresh size");
 #ifndef IGNORE_HYPER_STUFF
    refreshSizeOnAllWindows();
#endif
    LOG_QUIT_ROUTINE("CHILD_DEALLOC: done");
}
/*****************************************************************************/
@end
