#pragma once

#ifdef HYPERUI_EXPORTS

    #if defined(WIN32)
        //#define HYPERUI_API __declspec(dllexport)
        #define HYPERUI_API
        #define HYPERUI_EXTERN
    #else
        #define HYPERUI_API
        #define HYPERUI_EXTERN
    #endif
#else
    #if defined(WIN32)
        //#define HYPERUI_API __declspec(dllimport)
        //#define HYPERUI_EXTERN extern
		#define HYPERUI_API
		#define HYPERUI_EXTERN
    #else
        #define HYPERUI_API
        #define HYPERUI_EXTERN
    #endif
#endif

#include "config.h"

#ifdef WINDOWS
#include <windows.h>
#endif

#ifdef HAVE_POLYBOOL
namespace POLYBOOLEAN { struct PAREA; }
namespace POLYBOOLEAN { struct PLINE2; }
#endif

#ifdef WIN32
    #include "HyperCore.h"
#elif defined(MAC_BUILD)
    #import <HyperCore/HyperCore.h>
#endif

using namespace HyperCore;

namespace HyperUI
{
#include "Basic/Basic.h"
#include "Animation/IAnimController.h"
#include "Beziers/Beziers.h"
#include "Animation/Animation.h"
#include "Beziers/DeformableBezier.h"

#include "Events/Events.h"
#include "Utils/Utils.h"
#include "Rendering/Rendering.h"
#include "3D/3D.h"
#include "Text/Text.h"
}

#include "Text/FontManager.h"
namespace HyperUI
{
#include "Basic/PathTrail.h"
#include "UI/UI.h"
#include "StandardUI/StandardUI.h"

#include "Undo/Undo.h"

#include "App/App.h"

HYPERUI_API void initialize(const CHAR_TYPE* pcsSharedFolderAppName);
HYPERUI_API void initialize(const CHAR_TYPE* pcsSharedFolderAppName, const CHAR_TYPE* pcsOptSharedPathEnvVarName, const CHAR_TYPE* pcsOptOverrideAppPath);

struct HYPERUI_API SWindowParms
{
	SWindowParms()
	{
		myIsThinFrame = false;
		myIsFixedSize = false;
		myMinWidth = 800;
		myMinHeight = 600;
		myWidth = myMinWidth;
		myHeight = myMinHeight;
		myTitle = "HyperUI Window";
		myCenterX = -1;
		myCenterY = -1;
		myInitCollectionType = -1;
		myInitLayerToShow = "main";
	}

	int myWidth;
	int myHeight;
	bool myIsThinFrame;
	bool myIsFixedSize;
	int myMinWidth;
	int myMinHeight;
	STRING_TYPE myTitle;
	int myCenterX;
	int myCenterY;
	ResourceType myInitCollectionType;
	STRING_TYPE myMainMenuElementId;
	STRING_TYPE myInitLayerToShow;
};

HYPERUI_API void run();
HYPERUI_API void run(const SWindowParms& rParms);
HYPERUI_API void getHardwareInfo(string& strOut);


HYPERUI_API FLOAT_TYPE getMaxPenPressure();
HYPERUI_API void getChosenPixelFormatDescription(string& strOut);
HYPERUI_API int getMaxScreenScalingFactor();
};
