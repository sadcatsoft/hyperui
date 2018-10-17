#include "stdafx.h"

namespace HyperUI
{
extern void initializeOSSpecific();
static bool g_bDidInitialize = false;
FLOAT_TYPE g_fMaxPenPressure = 1.0;
/*****************************************************************************/
void initialize(const CHAR_TYPE* pcsSharedFolderAppName)
{
	initialize(pcsSharedFolderAppName, NULL, NULL);
}
/*****************************************************************************/
void initialize(const CHAR_TYPE* pcsSharedFolderAppName, const CHAR_TYPE* pcsOptSharedPathEnvVarName, const CHAR_TYPE* pcsOptOverrideAppPath)
{
	if(g_bDidInitialize)
		return;

	g_bDidInitialize = true;

	initializeOSSpecific();

	// We need this because the call to Max screen scaling factor actually uses settings, which needs
	// the shared folder name...
	HyperCore::setSharedFolderAppName(pcsSharedFolderAppName);
	HyperCore::setOverrideAppPathName(pcsOptOverrideAppPath);

	HyperCore::initialize(pcsSharedFolderAppName, HyperUI::getMaxScreenScalingFactor(), pcsOptSharedPathEnvVarName);

#ifdef WINDOWS
	// Make sure we compute the GL max image chunk
	// on this (presumably the display...) thread.
	// Otherwise, OS X goes crashy-crashy.
	// And it does, since when we init on OS X, we call
	// this in non-display function.
	int iDummy1, iDummy2;
	AccelImage::getChunkSizesFor(100, 100, iDummy1, iDummy2);
#endif

	//ResourceManager::getInstance()->initialize();
}
/*****************************************************************************/
FLOAT_TYPE getMaxPenPressure()
{
	return g_fMaxPenPressure;
}
/*****************************************************************************/
int getMaxScreenScalingFactor()
{
	int iRes = 1;

	StringResourceItem* pStartupSettings = SettingsCollection::getStartupSettings();
#ifdef ENABLE_RETINA_SUPPORT
#ifndef IMAGE_VIEWER_MODE
	if(pStartupSettings->getBoolProp(START_SETTINGS_ALLOW_RETINA))
#endif
	{
		iRes = getScreenDensityScalingFactor();
		if(iRes > g_iMaxDensityPrefix)
			iRes = g_iMaxDensityPrefix;
	}
#endif

	return iRes;
}
/*****************************************************************************/
};