#include "stdafx.h"

namespace HyperCore
{
/*****************************************************************************/
void gLog(const char* format, ...)
{
#ifdef _DEBUG
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
#endif
}
/*****************************************************************************/
extern int g_iScreenScalingFactor;
int getScreenDensityScalingFactor(int iOptMaxFactor)
{
	if(g_iScreenScalingFactor == 0)
	{
		int iRes = 1;

		GdkScreen *pScreen = gdk_screen_get_default();
		if(pScreen)
		{
			int iDpi = gdk_screen_get_resolution(pScreen);
			gLog("Screen DPI: %d\n", iDpi);
			if(iDpi >= 96)
				iRes = iDpi/96;
		}

		gLog("Screen Density Final: %d\n", iRes);
		g_iScreenScalingFactor = iRes;

		if(iOptMaxFactor > 0 && g_iScreenScalingFactor > iOptMaxFactor)
			g_iScreenScalingFactor = iOptMaxFactor;
	}
	return g_iScreenScalingFactor;
}
/*****************************************************************************/
bool getEnvVariable(const char* pcsVarName, string& strResOut)
{
	strResOut = "";
	const char *pcsRes = getenv(pcsVarName);
	if(pcsRes)
		strResOut = pcsRes;
	return strResOut.length() > 0;
}
/*****************************************************************************/
};