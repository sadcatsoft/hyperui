#include "stdafx.h"

namespace HyperCore
{
/*****************************************************************************/
const char* PathUtils::extractExtensionFromPath(const char* pcsPath)
{
	const char pcsPathSep[2] = { '.', 0 };
	char* pLast = NULL;
	char* pcsPtr = (char*)pcsPath;

	while( (pcsPtr = strstr(pcsPtr, pcsPathSep)) )
		pLast = pcsPtr++;

	if(pLast)
		pLast++;

	if(!pLast)
		return "";

	return pLast;
}
/*****************************************************************************/
void PathUtils::extractFileNameFromPath(const char* pcsPath, string& strFileNameOut)
{
	char* pLast = NULL;
	char* pcsPtr = (char*)pcsPath;

	while( (pcsPtr = strstr(pcsPtr, FOLDER_SEP)) )
		pLast = pcsPtr++;

	if(pLast)
		pLast++;

	if(!pLast)
	{
		strFileNameOut = pcsPath;
		return;
	}

	// Get the string
	strFileNameOut = pLast;
}
/*****************************************************************************/
void PathUtils::extractPureFileNameFromPath(const char* pcsPath, string& strPureNameOut)
{
	strPureNameOut = "";
	PathUtils::extractFileNameFromPath(pcsPath, strPureNameOut);
	pcsPath = strPureNameOut.c_str();

	const char pcsPathSep[2] = { '.', 0 };
	char* pLast = NULL;
	char* pcsPtr = (char*)pcsPath;

	while( (pcsPtr = strstr(pcsPtr, pcsPathSep)) )
		pLast = pcsPtr++;

	if(pLast && pLast != pcsPath)
		pLast--;

	if(!pLast || pLast == pcsPath)
		return;

	strPureNameOut = strPureNameOut.substr(0, pLast - pcsPath + 1);
}
/*****************************************************************************/
void PathUtils::ensurePathEndsInExtension(string& strPathInOut, const char* pcsExtension, bool bReplaceExisting)
{
	if(!IS_VALID_STRING(pcsExtension))
		return;

	const char* pcsCurrExt = PathUtils::extractExtensionFromPath(strPathInOut.c_str());

#ifdef WIN32
	if(_stricmp(pcsCurrExt, pcsExtension) == 0)
		return;
#else
	if(strcasecmp(pcsCurrExt, pcsExtension) == 0)
		return;
#endif

	if(bReplaceExisting)
	{
		string strFilename;
		PathUtils::extractPureFileNameFromPath(strPathInOut.c_str(), strFilename);
		string strTemp(strPathInOut.c_str());
		PathUtils::extractPathFromFullFilename(strTemp.c_str(), strPathInOut);

		strPathInOut += strFilename;
	}

	strPathInOut += ".";
	strPathInOut += pcsExtension;
}
/*****************************************************************************/
void PathUtils::extractPathFromFullFilename(const char* pcsFullPath, string& strPathOut)
{
	char* pLast = NULL;
	char* pcsPtr = (char*)pcsFullPath;

	while( (pcsPtr = strstr(pcsPtr, FOLDER_SEP)) )
		pLast = pcsPtr++;

	if(pLast)
		pLast++;

	if(!pLast)
	{
		strPathOut = "";
		return;
	}

	// Get the string
	strPathOut = pcsFullPath;
	int iSize = (pLast - pcsFullPath);
	strPathOut = strPathOut.substr(0, iSize);
}
/*****************************************************************************/
void PathUtils::ensureEndsWithFolderSep(string& strInOut)
{
	int iLen = strInOut.size();
	if(iLen == 0)
		return;

	if(strInOut[iLen - 1] != FOLDER_SEP[0])
		strInOut += FOLDER_SEP;
}
/*****************************************************************************/
};
