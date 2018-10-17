#include "stdafx.h"
#ifdef MAC_BUILD
#include "locale.h"
#endif

namespace HyperCore
{
STRING_TYPE g_strSharedPathEnvVar;
STRING_TYPE g_strSharedFolderAppName;

static bool g_bDidInitialize = false;
int g_iScreenScalingFactor = 0;
int g_iMaxScreenScalingFactor = 0;
FLOAT_TYPE g_fOriginalScaleFactorAtStartup = 1.0;
/*****************************************************************************/
void initialize(const CHAR_TYPE* pcsSharedFolderAppName, int iMaxScreenScalingFactor, const CHAR_TYPE* pcsOptSharedPathEnvVarName)
{
	if(g_bDidInitialize)
		return;

	g_bDidInitialize = true;

	setlocale( LC_ALL, "C");
	//setlocale( LC_ALL, "English_United States.1252" );
	//setlocale( LC_ALL, "en_US.UTF-8");

#ifdef WIN32
	_wsetlocale(LC_ALL, L"C");
	//	_wsetlocale(LC_ALL, L"English_United States.1252");
#endif

	g_iMaxScreenScalingFactor = iMaxScreenScalingFactor;
	if(g_iMaxScreenScalingFactor <= 0)
		g_iMaxScreenScalingFactor = getScreenDensityScalingFactor();

	if(IS_VALID_STRING_AND_NOT_NONE(pcsOptSharedPathEnvVarName))
		g_strSharedPathEnvVar = pcsOptSharedPathEnvVarName;

	setSharedFolderAppName(pcsSharedFolderAppName);

	g_iScreenScalingFactor = getScreenDensityScalingFactor(g_iMaxScreenScalingFactor);
	g_fOriginalScaleFactorAtStartup = upToScreen(1.0);
	
	BasicMath::initialize();

	RandomNumberGenerator::getInstance()->ensureRandomSeed();
}
/*****************************************************************************/
void setSharedFolderAppName(const CHAR_TYPE* pcsSharedFolderAppName)
{
	_ASSERT(IS_VALID_STRING(pcsSharedFolderAppName));
	g_strSharedFolderAppName = pcsSharedFolderAppName;
}
/*****************************************************************************/
void setOverrideAppPathName(const CHAR_TYPE* pcsOverrideAppPath)
{
	ResourceManager::setOverrideAppPathName(pcsOverrideAppPath);
}
/*****************************************************************************/
FLOAT_TYPE upToScreenOriginal(FLOAT_TYPE fUPNumber) 
{ 
	return g_fOriginalScaleFactorAtStartup*fUPNumber; 
}
/*****************************************************************************/
FLOAT_TYPE upToScreen(FLOAT_TYPE fUPNumber) 
{ 
	_ASSERT(g_iScreenScalingFactor > 0); 
	return fUPNumber*2.0*(FLOAT_TYPE)g_iScreenScalingFactor; 
}
/*****************************************************************************/
FLOAT_TYPE convertUPNumberToScreenPixels(const char* pcsNumber)
{
	char *pcsBuff = const_cast<char *>(pcsNumber);
	int iLen = strlen(pcsNumber);
	//int iLen = strNumber.length();

	if(iLen <= 2)
	{
		_ASSERT(0);
		return 0;
	}

	_ASSERT(pcsBuff[iLen - 2] == 'u' && pcsBuff[iLen - 1] == 'p');

	pcsBuff[iLen - 2] = 0;
	FLOAT_TYPE fRes = upToScreen(atof(pcsBuff));
	pcsBuff[iLen - 2] = 'p';

	return fRes;
}
/*****************************************************************************/

};
