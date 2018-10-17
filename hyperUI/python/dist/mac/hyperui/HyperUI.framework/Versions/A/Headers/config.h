#pragma once

//#define HAVE_POLYBOOL

// If defined, the cached item will keep all the cached
// texture manager info to avoid constantly making queries.
// Note that this presumes the textures are never deleted, 
// otherwise their ids are no longer valid.
#define CACHE_TEXTURE_INFO

#define ALLOW_3D
#define ALLOW_LIGHTING
// Currently only for 3D files.
//#define ALLOW_ENCRYPTION

#if !defined(EMSCRIPTEN) && !defined(HARDCORE_LINUX_DEBUG_MODE)
#define ENABLE_TEXT_CACHING
#endif

#define USE_FREETYPE_FONTS

// Debugging: show frame rate.
// New: Now we only show it if the setting is enabled.
#define DEBUG_SHOW_FRAMERATE


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

#ifndef PURE_CPLUSPLUS
#define WIN_COMPILE_UNMANAGED
#endif
