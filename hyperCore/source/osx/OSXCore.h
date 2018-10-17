#pragma once

#import <mach/mach.h>
#import <mach/mach_time.h>

#import <Cocoa/Cocoa.h>
#import "BookmarkManager.h"

namespace HyperCore
{
const char* getCStringSafe(NSString* pStringIn);
NSString* getDocumentsDir();
}
