#include "stdafx.h"

namespace HyperCore
{
/*****************************************************************************/
void gLog(const char* format, ...)
{
#if defined(_DEBUG) || defined(_DEBUG) || defined(DEBUG_MODE)

	va_list argptr;

	va_start(argptr, format);
	int iNeededBufferLen = _vscprintf(format, argptr);
	va_end(argptr);

	if(iNeededBufferLen < 2047)
	{
		char pcsMessage[2048];
		va_start(argptr, format);
		vsprintf(pcsMessage, format, argptr);
		va_end(argptr);
		OutputDebugStringA(pcsMessage);
		//TRACE_RESULT_PRINT(pcsMessage);
	}
	else
	{
		char *pcsMessage = new char[iNeededBufferLen + 2];
		va_start(argptr, format);
		vsprintf(pcsMessage, format, argptr);
		va_end(argptr);
		OutputDebugStringA(pcsMessage);

		//TRACE_RESULT_PRINT(pcsMessage);

		delete[] pcsMessage;
	}

#endif
}
/*****************************************************************************/
extern int g_iScreenScalingFactor;
int getScreenDensityScalingFactor(int iOptMaxFactor)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
		g_iScreenScalingFactor = 1;
	return g_iScreenScalingFactor;
#else

	if(g_iScreenScalingFactor == 0)
	{
		// NO hi-dpi on win just yet
		int iRes = 1;

		HDC hdc = GetDC(NULL);
		if (hdc)
		{
			int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
			int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
			if(dpiX >= 96)
				iRes = dpiX/96;
			ReleaseDC(NULL, hdc);
		}

		gLog("Screen Density Final: %d\n", iRes);
		g_iScreenScalingFactor = iRes;

		if(iOptMaxFactor > 0 && g_iScreenScalingFactor > iOptMaxFactor)
			g_iScreenScalingFactor = iOptMaxFactor;
	}
	return g_iScreenScalingFactor;
#endif
}
/*****************************************************************************/
};