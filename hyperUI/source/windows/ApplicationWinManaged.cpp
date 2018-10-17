#include "stdafx.h"
#include "WinManaged.h"
#include <windows/WinIncludes.h>

namespace HyperUI
{
/*****************************************************************************/
void Application::quitApp()
{
#if !defined(DIRECTX_PIPELINE)
	System::Windows::Forms::Application::Exit();
#endif
}
/*****************************************************************************/
void Application::lockGlobal()
{

}
/*****************************************************************************/
void Application::unlockGlobal()
{

}
/*****************************************************************************/
void Application::lockGlobalDisplay()
{

}
/*****************************************************************************/
void Application::unlockGlobalDisplay()
{

}
/*****************************************************************************/
bool Application::showOpenFolderDialog(const char* pcsInitFilePath, TStringVector& vecFilesOut)
{
	vecFilesOut.clear();
#ifdef DIRECTX_PIPELINE
	_ASSERT(0);
	DXSTAGE1
#else
	System::Windows::Forms::FolderBrowserDialog^ pDialog = gcnew System::Windows::Forms::FolderBrowserDialog;

	bool bHaveValidInitPath = IS_VALID_STRING_AND_NOT_NONE(pcsInitFilePath);
	if(bHaveValidInitPath)
		pDialog->SelectedPath = gcnew System::String(pcsInitFilePath);
	pDialog->ShowNewFolderButton = true;

	if(pDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
	{
		string strPathOut;
		stringToSTLString(pDialog->SelectedPath, strPathOut);
		vecFilesOut.push_back(strPathOut);
		return true;
	}
#endif
	return false;
}
/*****************************************************************************/
void getFilterStringFrom(TStringVector& vecExtensions, TStringVector& vecLabels, bool bAddAllSupportedFirst, string& strOut)
{
	string strAllExtensions;

	_ASSERT(vecLabels.size() == vecExtensions.size());
	strOut = "";
	int iIndex;
	int iCountedFormats = 0;
	int iNum = vecExtensions.size();
	for(iIndex = 0; iIndex < iNum; iIndex++)
	{
		if(iCountedFormats > 0)
			strOut += "|";
		strOut += vecLabels[iIndex];
		strOut += "(*.";
		strOut += vecExtensions[iIndex];
		strOut += ")|*.";
		strOut += vecExtensions[iIndex];
		iCountedFormats++;

		if(strAllExtensions.length() > 0)
			strAllExtensions += ";";
		strAllExtensions += "*.";
		strAllExtensions += vecExtensions[iIndex];
	}

	// Have one where all the supported ones have:
	if(bAddAllSupportedFirst)
		strOut = "All Supported File Types|" + strAllExtensions + "|" + strOut;
}
/*****************************************************************************/
bool Application::showOpenFileDialog(const char* pcsInitFilePath, bool bAllowMultiple, TStringVector& vecFilesOut, 
									TStringVector& pOptExtensions, TStringVector& pOptDescriptions, FormatsManager* pFormatsManager, bool bCanOpenFolders)
{    
#ifdef DIRECTX_PIPELINE
	_ASSERT(0);
	DXSTAGE1
		return false;
#else
	string strFilter;
	getFilterStringFrom(pOptExtensions, pOptDescriptions, true, strFilter);

	bool bHaveValidInitPath = IS_VALID_STRING_AND_NOT_NONE(pcsInitFilePath);

	System::Windows::Forms::OpenFileDialog^ openFileDialog1 = gcnew System::Windows::Forms::OpenFileDialog;
	if(bHaveValidInitPath)
	{
		string strInitPath;
		PathUtils::extractPathFromFullFilename(pcsInitFilePath, strInitPath);
		openFileDialog1->InitialDirectory = gcnew System::String(strInitPath.c_str());
	}
	else
		openFileDialog1->InitialDirectory = System::Windows::Forms::Application::ExecutablePath;

	openFileDialog1->Filter = gcnew System::String(strFilter.c_str());
	if(pFormatsManager)
		openFileDialog1->FilterIndex = pFormatsManager->getFilterIndexFromPath(bHaveValidInitPath ? pcsInitFilePath : NULL, false);
	openFileDialog1->RestoreDirectory = true;
	openFileDialog1->Multiselect = bAllowMultiple;

	vecFilesOut.clear();
	string strPathOut;

	if(openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
	{
		cli::array<System::String^> ^pFileNames = openFileDialog1->FileNames;
		int iCurr, iNum = pFileNames->Length;
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			stringToSTLString(pFileNames[iCurr], strPathOut);
			vecFilesOut.push_back(strPathOut);
		}
		return vecFilesOut.size() > 0;
	}
	return false;
#endif
}
/*****************************************************************************/
ResultCodeType Application::showSaveFileDialog(string& strPathInOut, string& strErrorOut, TStringVector& pOptExtensions, TStringVector& pOptDescriptions, FormatsManager* pFormatsManager, 
												bool bForceNativeExtension, bool bDontListNative)
{
#ifdef DIRECTX_PIPELINE
	_ASSERT(0);
	DXSTAGE1
		return ResultCodeCancel;
#else

	strErrorOut = "";
	System::Windows::Forms::SaveFileDialog^ saveFileDialog1 = gcnew System::Windows::Forms::SaveFileDialog;
	if(strPathInOut.length() > 0)
	{
		string strInitPath;
		PathUtils::extractPathFromFullFilename(strPathInOut.c_str(), strInitPath);
		saveFileDialog1->InitialDirectory = gcnew System::String(strInitPath.c_str());
	}
	else
		saveFileDialog1->InitialDirectory = System::Windows::Forms::Application::ExecutablePath;

	string strFilter;
	getFilterStringFrom(pOptExtensions, pOptDescriptions, false, strFilter);

	if(bForceNativeExtension || !pFormatsManager)
		saveFileDialog1->FilterIndex = 1;
	else
		saveFileDialog1->FilterIndex = pFormatsManager->getFilterIndexFromPath(strPathInOut.c_str(), true);

	saveFileDialog1->Filter = gcnew System::String(strFilter.c_str());
	saveFileDialog1->RestoreDirectory = true;

	strPathInOut = "";
	if ( saveFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK )
	{
		stringToSTLString(saveFileDialog1->FileName, strPathInOut);
		if(!FileUtils::canAccessFileForWriting(strPathInOut.c_str()))
		{
			strErrorOut = "Cannot access file at this location.";
			return ResultCodeError;
		}
		else
			return ResultCodeOk;
	}

	return ResultCodeCancel;
#endif
}
/*****************************************************************************/
Window* Application::openNewWindow(Window* pParentWindow, int iResourceType, int iOptWidth, int iOptHeight, bool bThinFrame, bool bFixedSize, 
								   int iMinWidth, int iMinHeight, const char* pcsTitle, int iCenterX, int iCenterY, bool bIsMainWindow, const char* pcsMainMenuElemId, const char* pcsInitLayerToShow)
{
	gLog("----------- Opening new window -----------\n");
#ifdef DIRECTX_PIPELINE
	_ASSERT(0);
	DXSTAGE1
		return NULL;
#else
	// TODO: Have a delegate for overrides of the class?
	IBaseForm^ pNewWindow = gcnew IBaseForm(iOptWidth, iOptHeight, bThinFrame, bFixedSize, (ResourceType)iResourceType, iMinWidth, iMinHeight, pcsTitle, iCenterX, iCenterY, bIsMainWindow, pcsInitLayerToShow);
	IBaseForm::getMainForm()->AddOwnedForm(pNewWindow);
	pNewWindow->Show();
	return pNewWindow->getWindow();
#endif
}
/*****************************************************************************/
bool Application::getColorFromColorPicker(SColor& scolExistingColor, const char* pcsStartElemName, UNIQUEID_TYPE idParentWindow, SColor& scolNewOut)
{
#ifdef DIRECTX_PIPELINE
	bool bShowNative = false;
#else
	bool bShowNative = SettingsCollection::getInstance()->getSettingsItem()->getBoolProp(PropertySetUseNativeColorPicker);
#endif

	if(!bShowNative)
	{
		// Just open our main picker (or bring it to front) and return.
		WindowManager::getInstance()->getMainWindow()->getUIPlane()->showColorPicker(scolExistingColor, pcsStartElemName, idParentWindow);		
		return false;
	}
#ifndef DIRECTX_PIPELINE
	System::Windows::Forms::ColorDialog^ MyDialog = gcnew System::Windows::Forms::ColorDialog;

	// Keeps the user from selecting a custom color.
	MyDialog->AllowFullOpen = true;
	MyDialog->FullOpen = true;
	// Allows the user to get help. (The default is false.)
	MyDialog->ShowHelp = true;
	// Sets the initial color select to the current text color.
	int col[4];
	col[0] = scolExistingColor.alpha*255.0;
	col[1] = scolExistingColor.r*255.0;
	col[2] = scolExistingColor.g*255.0;
	col[3] = scolExistingColor.b*255.0;
	MyDialog->Color = System::Drawing::Color::FromArgb(col[0], col[1], col[2], col[3]);

	// Update the text box color if the user clicks OK 
	if ( MyDialog->ShowDialog() == ::System::Windows::Forms::DialogResult::OK )
	{
		scolNewOut.r = (FLOAT_TYPE)MyDialog->Color.R/255.0;
		scolNewOut.g = (FLOAT_TYPE)MyDialog->Color.G/255.0;
		scolNewOut.b = (FLOAT_TYPE)MyDialog->Color.B/255.0;
		scolNewOut.alpha = (FLOAT_TYPE)MyDialog->Color.A/255.0;
		return true;
	}
	else
		return false;
#endif
}
/*****************************************************************************/
};