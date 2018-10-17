
#import <Cocoa/Cocoa.h>

@class BaseGLView;
/*****************************************************************************/
@interface BaseWindowController : NSObject < NSWindowDelegate, NSMenuDelegate >
{
    BaseGLView* myView;
    
    NSWindow *myMacWindow;
    HyperUI::Window *myWindow;
#ifndef IGNORE_HYPER_STUFF
    HyperUI::TTouchVector *myTouches;
    HyperUI::SMacFormWrapper* myFormWrapper;
#endif
    NSRect myTargetRect;
 
    double    myCurrentTime;
    double    myAccumulator;
    double    myDrawTime;
    double    myDrawDelta;

	double    myLastPenPressure;
}
/*****************************************************************************/
@property (nonatomic, assign) BaseGLView* myView;

- (instancetype)init:(BaseGLView*)pView withWindow:(NSWindow*)pWindow;
- (bool)drawView;
- (HyperUI::Window*)getWindow;
- (void) deleteWindow;
/*****************************************************************************/
@end
