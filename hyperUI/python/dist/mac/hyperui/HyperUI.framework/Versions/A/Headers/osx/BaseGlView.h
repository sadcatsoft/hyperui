@class BaseWindowController;
/*****************************************************************************/
@interface BaseGLView : NSOpenGLView
{
    CVDisplayLinkRef displayLink;
    BaseWindowController *myMainController;
    //HyperUI::Window *myWindow;
    bool myHasShutdown;
    
    NSTrackingRectTag myTrackingRectId;
}
/*****************************************************************************/
- (void) startAnimation;
- (void) setMainController:(BaseWindowController*)theController;
- (HyperUI::Window*) getWindow;
/*****************************************************************************/
@end
