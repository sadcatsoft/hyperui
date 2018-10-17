#import <Cocoa/Cocoa.h>
#include "AppDelegate.h"

int main(int argc, const char * argv[])
{
    NSApplication* app = [NSApplication sharedApplication];
    [NSApp setActivationPolicy:(NSApplicationActivationPolicyRegular)];
    AppDelegate* pDelegate = [[AppDelegate alloc] init];
    app.delegate = pDelegate;
    [app run];
    return 1;
}
