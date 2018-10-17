#pragma once

// Valid tokens for OSes:
// WINDOWS - for any Windows-based machine
// LINUX - for any Linux-based machine
// OSX_BUILD - macOS build
// IOS_BUILD - iOS build (iPad or iPhone)
// ANDROID_BUILD - Android

// Figure out our OS
#if (defined(WIN32) || defined(_WIN32)) && !defined(WINDOWS)
    #define WINDOWS
#endif

#if (defined(__APPLE__) && defined(__MACH__))

    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
        #define IOS_BUILD
    #elif TARGET_OS_IPHONE
        #define IOS_BUILD
    #elif TARGET_OS_MAC
        #define OSX_BUILD
    #else
        #error "Unknown Apple platform"
    #endif

#endif

#if (defined(__linux__) || defined(unix)) && !defined(LINUX)
    #define LINUX
#endif

#if defined(__ANDROID__) && !defined(__ANDROID__)
    #define ANDROID_BUILD
#endif

// TEMPORARY:
#if defined(IOS_BUILD) || defined(OSX_BUILD)
    #define MAC_BUILD
#endif
