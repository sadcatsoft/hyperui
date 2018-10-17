#pragma once

#include "platformConfig.h"

// If defined, we use fast, but slightly less precise trig and sqrt
// functions using lookup tables for the former.
#define USE_FAST_MATH_ROUTINES

// If defined, we use boost locking primitives, otherwise, we rely
// on native mutex implementation. Note that the RecursiveSharedLock
// becomes slower without boost.
//#define USE_BOOST_LOCKS

// #define KEEP_STRING_CACHE_STATS
// #define PROPCACHER_KEEP_CACHE_STATS

// Which string to use for string resource items
#define RESOURCEITEM_STRING_TYPE		SString

#define ISOMETRIC_VERT_ANGLE			0.0 // 45.0

// If enabled, we use COW for resource items.
#define ENABLE_COW_FOR_PROPERTIES

#pragma warning(disable: 4244 4305 4267 4949 4793 4800 4251)

// If defined, we're building for the iPad.
// On Mac, we just add it to the target defines.
#if defined(WINDOWS) || defined(LINUX)
// TODO: We should really clean this up
	#define MAC_BUILD

	// On windows, allows BASS sound to be used. Requires linking against extralibs\bass sometghing
	// Now defined in proj. settings.
	#ifndef _DEBUG
	//#define ALLOW_WIN_SOUND
	#endif
#endif
