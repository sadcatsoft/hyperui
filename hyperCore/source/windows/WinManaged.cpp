#include "stdafx.h"
#include "WinManaged.h"

namespace HyperCore
{
extern STRING_TYPE g_strSharedPathEnvVar;
extern STRING_TYPE g_strSharedFolderAppName;

#ifdef DIRECTX_PIPELINE
Platform::String^ stringToPlatformString(const char* pcsString)
{
	if(!pcsString)
		return ref new Platform::String(L"");
	wchar_t* wbuff = StringUtils::asciiToWide(pcsString);
	Platform::String ^pUrl = ref new Platform::String(wbuff);
	StringUtils::freeWideText(wbuff);
	return pUrl;
}

void stringToSTLString(Platform::String^ pStringIn, string& rStringOut)
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

void getCommonAppDataPath(string& strPath)
{
	Platform::String ^pDir = ApplicationData::Current->LocalFolder->Path;
	stringToSTLString(pDir, strPath);
}

void getAppStartupPath(string& strOut)
{
	stringToSTLString(Windows::ApplicationModel::Package::Current->InstalledLocation->Path, strOut);
}

bool getEnvVariable(const char* pcsVarName, string& strResOut)
{
	strResOut = "";
	DXSTAGE2
	return strResOut.length() > 0;
}

#else

void stringToSTLString(System::String^ pStringIn, string& rStringOut)
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

void getCommonAppDataPath(string& strPath)
{
	if(g_strSharedPathEnvVar.length() > 0 && Environment::getInstance()->getDoesVarExist(g_strSharedPathEnvVar.c_str()))
	{
		strPath = Environment::getInstance()->getStringVar(g_strSharedPathEnvVar.c_str());
		return;
	}

	System::String ^pDir = System::Environment::GetFolderPath(System::Environment::SpecialFolder::ApplicationData);

	if(g_strSharedFolderAppName.length() == 0)
		gLog("\nERROR: No application name specified. This probably means you need to call HyperUI::initialize(\"Your_App_Name\"); before using the library.\n");

	_ASSERT(g_strSharedFolderAppName.length() > 0);
	pDir = System::IO::Path::Combine(pDir, gcnew System::String(g_strSharedFolderAppName.c_str()));
	if (!System::IO::Directory::Exists(pDir))
		System::IO::Directory::CreateDirectory(pDir);
	stringToSTLString(pDir, strPath);
}

void getAppStartupPath(string& strOut)
{
	stringToSTLString(System::Windows::Forms::Application::StartupPath, strOut);
}

bool getEnvVariable(const char* pcsVarName, string& strResOut)
{
	strResOut = "";

	#undef GetEnvironmentVariable
	System::String^ pStrRes = System::Environment::GetEnvironmentVariable(gcnew System::String(pcsVarName));

	if(pStrRes)
		stringToSTLString(pStrRes, strResOut);

	return strResOut.length() > 0;
}


#endif
}
