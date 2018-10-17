#import <Cocoa/Cocoa.h>
#include "stdafx.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (nonatomic, assign) HyperUI::SWindowParms myParms;
@end

@implementation AppDelegate
/*****************************************************************************/
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    HyperUI::run(self.myParms);
}
/*****************************************************************************/
- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    // Insert code here to tear down your application
}
/*****************************************************************************/
@end

void runMacApp(HyperUI::SWindowParms& rParms)
{
    NSApplication* app = [NSApplication sharedApplication];
    [NSApp setActivationPolicy:(NSApplicationActivationPolicyRegular)];
    AppDelegate* pDelegate = [[AppDelegate alloc] init];
    pDelegate.myParms = rParms;
    app.delegate = pDelegate;
    [app run];
}
