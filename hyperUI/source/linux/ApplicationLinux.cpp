#include "stdafx.h"
#include <gtk/gtk.h>
#include "GlutWindow.h"
#include <sys/types.h>
#include <pwd.h>

/*****************************************************************************/
void Application::quitApp()
{
	// Todo: we need to ask all docs
	// whether they need to close...
#ifdef LINUX
	glutLeaveMainLoop();
#endif
	FIRST_LEVEL_TODO;
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
bool gShowOpenFileDialogInternal(const char* pcsInitFilePath, bool bAllowMultiple, TStringVector& vecFilesOut,
	TStringVector* pOptExtensions, TStringVector* pOptDescriptions, bool bCanOpenFolders)
{
#ifdef USE_OWN_FILE_DIALOG
	return true;
#else
	vecFilesOut.clear();
	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open File", NULL, 
		bCanOpenFolders ? GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER : GTK_FILE_CHOOSER_ACTION_OPEN, 
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gMakeTransient(dialog);

	// http://www.gtk.org/api/2.6/gtk/GtkFileChooser.html
	GtkFileChooser* pFileChooser = GTK_FILE_CHOOSER( dialog );
	gtk_file_chooser_set_select_multiple(pFileChooser, bAllowMultiple);

	struct passwd *pw = getpwuid(getuid());
	const char* pcsPath = IS_VALID_STRING_AND_NOT_NONE(pcsInitFilePath) ? pcsInitFilePath : NULL;
	if(!pcsPath)
		pcsPath = pw->pw_dir;
	if(pcsPath)
	{
		string strTemp("file://");
		strTemp += pcsPath;
		bool bRes = gtk_file_chooser_set_current_folder_uri(pFileChooser, strTemp.c_str());
	}

	TStringVector vecTempExtList, vecTempDescList;
	TStringVector* pExtensionsList = pOptExtensions;
	TStringVector* pDescsList = pOptDescriptions;
	if(!pExtensionsList)
	{
		BloomFormatsManager::getInstance()->getExtensionsList(false, FormatRead, vecTempExtList);
		pExtensionsList = &vecTempExtList;
	}
	if(!pDescsList)
	{
		BloomFormatsManager::getInstance()->getDescriptionsList(false, FormatRead, vecTempDescList);
		pDescsList = &vecTempDescList;
	}

	gAssignGTKFiltersToDialog(pFileChooser, pExtensionsList, pDescsList);
	while (gtk_events_pending ())
		gtk_main_iteration();
	bool bRes = false;
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		GSList* pFilesHead = gtk_file_chooser_get_filenames (pFileChooser);
		GSList* pCurrElem;
		for(pCurrElem = pFilesHead; pCurrElem; pCurrElem = pCurrElem->next)
		{
			vecFilesOut.push_back((char*)pCurrElem->data);
			g_free (pCurrElem->data);
		}
		g_slist_free(pFilesHead);

		bRes = true;
	}

	gtk_widget_destroy (dialog);
	// This makes the destroy actually happen:
	while (gtk_events_pending ())
		gtk_main_iteration();
	return bRes;
#endif
}
/*****************************************************************************/
bool Application::showOpenFileDialog(const char* pcsInitFilePath, bool bAllowMultiple, TStringVector& vecFilesOut,
	TStringVector* pOptExtensions, TStringVector* pOptDescriptions, bool bCanOpenFolders)
{
	return gShowOpenFileDialogInternal(pcsInitFilePath, bAllowMultiple, vecFilesOut, pOptExtensions, pOptDescriptions, bCanOpenFolders);
}
/*****************************************************************************/
bool Application::showOpenFolderDialog(const char* pcsInitFilePath, TStringVector& vecFilesOut)
{
	return gShowOpenFileDialogInternal(pcsInitFilePath, true, vecFilesOut, NULL, NULL, true);
}
/*****************************************************************************/
ResultCodeType Application::showSaveFileDialog(string& strPathInOut, string& strErrorOut, bool bForceNativeExtension, bool bDontListNative,
	TStringVector* pOptExtensions, TStringVector* pOptDescriptions)
{
#ifdef USE_OWN_FILE_DIALOG
	return ResultCodeOk;
#else
	ResultCodeType eRes = ResultCodeCancel;

	GtkWidget *dialog = gtk_file_chooser_dialog_new ("Save File", NULL, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	gMakeTransient(dialog);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
	GtkFileChooser* pFileChooser = GTK_FILE_CHOOSER( dialog );
	gtk_file_chooser_set_select_multiple(pFileChooser, false);

	TStringVector vecTempExtList, vecTempDescList;
	TStringVector* pExtensionsList = pOptExtensions;
	TStringVector* pDescsList = pOptDescriptions;
	if(!pExtensionsList)
	{
		BloomFormatsManager::getInstance()->getExtensionsList(false, FormatWrite, vecTempExtList);
		pExtensionsList = &vecTempExtList;
	}
	if(!pDescsList)
	{
		BloomFormatsManager::getInstance()->getDescriptionsList(false, FormatWrite, vecTempDescList);
		pDescsList = &vecTempDescList;
	}

	gAssignGTKFiltersToDialog(pFileChooser, pExtensionsList, pDescsList);

	string strInitName, strInitPath;
	if(strPathInOut.length())
	{
		PathUtils::extractPureFileNameFromPath(strPathInOut.c_str(), strInitName);
		PathUtils::extractPathFromFullFilename(strPathInOut.c_str(), strInitPath);
	}

	if(strInitPath.length() > 0)
		gtk_file_chooser_set_current_folder (pFileChooser, strInitPath.c_str());
	if(strInitName.length() > 0)
		gtk_file_chooser_set_current_name (pFileChooser, strInitName.c_str());
	else
		gtk_file_chooser_set_current_name (pFileChooser, "Untitled");
	while (gtk_events_pending ())
		gtk_main_iteration();
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		GtkFileFilter*  pFilter = gtk_file_chooser_get_filter (pFileChooser);
		const gchar *filterType = gtk_file_filter_get_name      (pFilter);

		string strChosenExt(filterType);
		int iPosEnd = strChosenExt.rfind(")");
		int iPosStart = strChosenExt.rfind(".", iPosEnd);
		strChosenExt = strChosenExt.substr(iPosStart + 1, iPosEnd - iPosStart - 1);

		strPathInOut = filename;
		PathUtils::ensurePathEndsInExtension(strPathInOut, strChosenExt.c_str(), false);
		eRes = ResultCodeOk;
		g_free (filename);
	}

	gtk_widget_destroy (dialog);
	// This makes the destroy actually happen:
	while (gtk_events_pending ())
		gtk_main_iteration ();

	return eRes;
#endif
}
/*****************************************************************************/
Window* gOpenNewWindowLinux(Window* pParentWindow, int iResourceType, int iOptWidth, int iOptHeight, bool bThinFrame, bool bFixedSize, int iMinWidth, int iMinHeight, const char* pcsTitle, int iCenterX, int iCenterY, Window* pExistingWindow)
{
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_ALPHA|GLUT_STENCIL|GLUT_DEPTH);
	if(iOptWidth < 0)
		iOptWidth = upToScreen(1024);
	if(iOptHeight < 0)
		iOptHeight = upToScreen(768);

	SVector2D svScreenDims;
	svScreenDims.x = glutGet(GLUT_SCREEN_WIDTH);
	svScreenDims.y = glutGet(GLUT_SCREEN_HEIGHT);
	if(iCenterX < 0)
		iCenterX = svScreenDims.x/2.0;
	if(iCenterY < 0)
		iCenterY = svScreenDims.y/2.0;

	glutInitWindowSize(iOptWidth, iOptHeight);
	glutInitWindowPosition(iCenterX - iOptWidth/2, iCenterY - iOptHeight/2);
	int window_1 = glutCreateWindow(pcsTitle != NULL ? pcsTitle : APP_NAME);
	InitializeGlutCallbacks();

	GlutWindowManager *pWinManager = GlutWindowManager::getInstance();
	bool bIsMainWindow = pWinManager->windowsBegin().isEnd() || g_pMainEngine->getWindow() == pExistingWindow;
	SGlutWindowWrapper* pNewWindow = new SGlutWindowWrapper(window_1, iOptWidth, iOptHeight, bIsMainWindow, iResourceType, pExistingWindow);

	pWinManager->onWindowCreated(pNewWindow, bIsMainWindow);
	return pNewWindow->myWindow;
}
/*****************************************************************************/
Window* Application::openNewWindow(Window* pParentWindow, int iResourceType, int iOptWidth, int iOptHeight, bool bThinFrame, bool bFixedSize, int iMinWidth, int iMinHeight, const char* pcsTitle, int iCenterX, int iCenterY)
{
	return gOpenNewWindowLinux(pParentWindow, iResourceType, iOptWidth, iOptHeight, bThinFrame, bFixedSize, iMinWidth, iMinHeight, pcsTitle, iCenterX, iCenterY, NULL);
}
/*****************************************************************************/
bool Application::getColorFromColorPicker(SColor& scolExistingColor, const char* pcsStartElemName, UNIQUEID_TYPE idParentWindow, SColor& scolNewOut)
{
	g_pMainEngine->getUIPlane()->showColorPicker(scolExistingColor, pcsStartElemName, idParentWindow);
	return false;
}
/*****************************************************************************/
