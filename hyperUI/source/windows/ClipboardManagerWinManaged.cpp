#include "stdafx.h"
#include <windows/WinIncludes.h>

#ifdef WIN_COMPILE_UNMANAGED
#pragma managed(push, on)
#endif

#ifndef DIRECTX_PIPELINE
using namespace System::Windows::Forms;
#endif

namespace HyperUI
{


/*****************************************************************************/
bool ClipboardManager::getDoHaveTextInClipboard()
{
	bool bRes = false;
#ifdef DIRECTX_PIPELINE
	DXSTAGE2
#else
	try { bRes = Clipboard::ContainsText();	}
	catch (...) { }	
#endif
	return bRes;
}
/*****************************************************************************/
void ClipboardManager::getClipboardText(string& strTextOut)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE2
#else
	try 
	{
		System::String^ pString = Clipboard::GetText(System::Windows::Forms::TextDataFormat::Text);
		stringToSTLString(pString, strTextOut);
	}
	catch(...)
	{
		strTextOut = "";
	}
#endif
}
/*****************************************************************************/
void ClipboardManager::setClipboardText(const char* pcsText)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE2
#else
	try 
	{
		System::String^ pString = gcnew System::String(pcsText);
		Clipboard::SetText(pString, System::Windows::Forms::TextDataFormat::Text);
	}
	catch(...) { }
#endif
}
/*****************************************************************************/
};