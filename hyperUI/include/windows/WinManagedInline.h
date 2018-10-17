#pragma once

#ifdef WIN_COMPILE_UNMANAGED
#pragma managed(push, on)
#endif

#ifndef DIRECTX_PIPELINE
#include <vcclr.h>
#endif

namespace HyperUI
{
/*****************************************************************************/
ref class ApplicationRefreshLoop;
extern HYPERUI_API gcroot<ApplicationRefreshLoop^> g_pMainWinProgram;
/*****************************************************************************/
#ifdef DIRECTX_PIPELINE
inline Platform::String^ stringToPlatformString(const char* pcsString)
{
	if(!pcsString)
		return ref new Platform::String(L"");
	wchar_t* wbuff = StringUtils::asciiToWide(pcsString);
	Platform::String ^pUrl = ref new Platform::String(wbuff);
	StringUtils::freeWideText(wbuff);
	return pUrl;
}
/*****************************************************************************/
inline void stringToSTLString(Platform::String^ pStringIn, string& rStringOut)
{
	wchar_t* pSimpleArray = new wchar_t[pStringIn->Length()+1];
	char *pTempArray;
	int iChar;
	auto it = pStringIn->Begin();
	for(iChar = 0; iChar < pStringIn->Length(); iChar++)
		pSimpleArray[iChar] = it[iChar];
	pSimpleArray[iChar] = 0;

	pTempArray = StringUtils::wideToAscii(pSimpleArray);
	rStringOut = pTempArray;
	StringUtils::freeAsciiText(pTempArray);
	delete[] pSimpleArray;
}
#else
/*****************************************************************************/
inline void stringToSTLString(System::String^ pStringIn, string& rStringOut)
{
	cli::array< wchar_t >^ rNotSoSimpleArray = pStringIn->ToCharArray();

	wchar_t* pSimpleArray = new wchar_t[rNotSoSimpleArray->Length+1];
	char *pTempArray;
	int iChar;
	for(iChar = 0; iChar < rNotSoSimpleArray->Length; iChar++)
		pSimpleArray[iChar] = rNotSoSimpleArray[iChar];
	pSimpleArray[iChar] = 0;

	pTempArray = StringUtils::wideToAscii(pSimpleArray);
	rStringOut = pTempArray;
	StringUtils::freeAsciiText(pTempArray);
	delete[] pSimpleArray;
}
/*****************************************************************************/
#endif

HYPERUI_API void getPixelFormatInfo(PIXELFORMATDESCRIPTOR &rFormatIn, string& strOut);

};

#ifdef WIN_COMPILE_UNMANAGED
#pragma managed(pop)
#endif
