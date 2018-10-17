#pragma once

#ifdef HYPERCORE_EXPORTS
    #if defined(WIN32)
		//#define HYPERCORE_API __declspec(dllexport)
        #define HYPERCORE_API 
		#define HYPERCORE_EXTERN
    #else
		#define HYPERCORE_API
		#define HYPERCORE_EXTERN
    #endif
#else
    #if defined(WIN32)
		//#define HYPERCORE_API __declspec(dllimport)
		//#define HYPERCORE_EXTERN extern
        #define HYPERCORE_API
        #define HYPERCORE_EXTERN
    #else
		#define HYPERCORE_API
		#define HYPERCORE_EXTERN
    #endif
#endif

#include "config.h"

#ifdef WINDOWS
	#include <windows.h>
#endif
/*
#ifdef MAC_BUILD
#import <Cocoa/Cocoa.h>

//! Project version number for HyperCore.
FOUNDATION_EXPORT double HyperCoreVersionNumber;

//! Project version string for HyperCore.
FOUNDATION_EXPORT const unsigned char HyperCoreVersionString[];
#endif
*/
#include "Basic/DataTypes.h"

namespace HyperCore
{
#include "Basic/Enums.h"
#include "Basic/IBaseObject.h"
#include "Basic/BaseValue.h"
#include "Basic/BinaryData.h"
#include "Basic/Base64Coder.h"
#include "Basic/BaseUtils.h"
#include "Basic/PackedFile.h"
#include "Storage/Storage.h"

#include "Basic/Mutex.h"
#include "Basic/RecursiveSharedLock.h"
	
#include "MathCore/MathCore.h"

#include "Basic/GeometryUtils.h"
#include "Basic/StringUtils.h"
#include "Basic/TokenizeUtils.h"

HYPERCORE_API void initialize(const CHAR_TYPE* pcsSharedFolderAppName, int iMaxScreenScalingFactor = -1, const CHAR_TYPE* pcsOptSharedPathEnvVarName = NULL);
HYPERCORE_API void setSharedFolderAppName(const CHAR_TYPE* pcsSharedFolderAppName);
HYPERCORE_API void setOverrideAppPathName(const CHAR_TYPE* pcsOverrideAppPath);
HYPERCORE_API FLOAT_TYPE upToScreen(FLOAT_TYPE fUPNumber);
// Returns the original scale applied to UI elements.
// Special use only.
HYPERCORE_API FLOAT_TYPE upToScreenOriginal(FLOAT_TYPE fUPNumber);
// Note that this number must be stored somewhere in a string,
// since we overwrite its memory
HYPERCORE_API FLOAT_TYPE convertUPNumberToScreenPixels(const char* pcsNumber);

HYPERCORE_API void getCommonAppDataPath(string& strPath);
HYPERCORE_API void gLog(const char* format, ...);
// This returns the density factor where:
// Value of 1:
// - On desktop, 1 is the regular pixel density desktop screen (i.e. 96 dpi on Windows) or non-Retina screen on OS X
// - On mobile, 1 is the iPhone with non-Retina screen (i.e. iPhone 3G at 164ppi)
HYPERCORE_API int getScreenDensityScalingFactor(int iOptMaxFactor = -1);
HYPERCORE_API void getAppStartupPath(string& strOut);
HYPERCORE_API bool getEnvVariable(const char* pcsVarName, string& strResOut);

#include "DataStructs/DataStructs.h"
#include "Utils/Utils.h"

#include "Basic/Logger.h"

#include "ResourceData/ResourceData.h"

#include "Basic/EdgeId.h"

// These use resource items, and so need to be included after
#include "DataStructs/SGradientStop.h"
#include "DataStructs/SGradient.h"

#include "Curves/Curves.h"

#include "Basic/Environment.h"

};
