#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
IOffscreenCacher::IOffscreenCacher(Window* pWindow)
{
	// This is because on a mac with dual monitors, we may get
	// corrupted texture memory (diff monitor?) if we cache text before
	// the app has been fully initialized. So we reset this flag from
	// applicationDidFinishLaunching() in MacGameController.
#if defined(WIN32) || defined(LINUX)
	myGlobalEnableCaching = true;
#else
	myGlobalEnableCaching = false;
#endif
	myParentWindow = pWindow;
}
/*****************************************************************************/
IOffscreenCacher::~IOffscreenCacher()
{

}
/*****************************************************************************/
void IOffscreenCacher::getSrcScreenArea(int iW, int iH, SRect2D& srAreaOut) const
{
#if defined(WIN32) || defined(MAC_BUILD)
	srAreaOut.x = srAreaOut.y = 0;
	srAreaOut.w = iW;
	srAreaOut.h = iH;
#else
#ifndef ORIENTATION_LANDSCAPE
	// Won't work with left orientations since we need
	// to make copy func be aware of the different rotation.
	_ASSERT(0);
#endif
	SVector2D svScreen;
	myParentWindow->getSize(svScreen);
	srAreaOut.x = svScreen.y - iW;
	srAreaOut.y = svScreen.x - iH;
	srAreaOut.w = iW;
	srAreaOut.h = iH;
#endif
}
/*****************************************************************************/
};