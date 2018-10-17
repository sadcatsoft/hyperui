#include "stdafx.h"

namespace HyperCore
{
const char* const g_pcsAllowedInternalFileNameChars = "_.-";
#define INTERNAL_FILENAME_REPLACEMENT_CHAR		'_'

/*****************************************************************************/
bool FileUtils::makePathRelative(const char* pcsSrcPath, const char* pcsRelativeToPath, bool bDoesRelativePathIncludeFileName, string& strOut)
{
	strOut = pcsSrcPath;

	// Tokenize both:
	string strDelims(FOLDER_SEP);
	string strSrcPath(pcsSrcPath);
	string strRelToPath(pcsRelativeToPath);
	TCharPtrVector vecSrcTokens, vecRelToTokens;
	TokenizeUtils::tokenizeStringToCharPtrsInPlace(strSrcPath, strDelims, vecSrcTokens);
	TokenizeUtils::tokenizeStringToCharPtrsInPlace(strRelToPath, strDelims, vecRelToTokens);

	// Find common base
	int iPathPos, iMinLen = min(vecSrcTokens.size(), vecRelToTokens.size());
	if(iMinLen <= 0)
		return false;

	for(iPathPos = 0; iPathPos < iMinLen; iPathPos++)
	{
		if(!IS_STRING_EQUAL(vecSrcTokens[iPathPos],vecRelToTokens[iPathPos]))
			break;
	}

	if(iPathPos == 0)
		return false;

	strOut = "";
	int iIndex2;
	int iRelUntil = vecRelToTokens.size();
	if(bDoesRelativePathIncludeFileName)
		iRelUntil--;
	for(iIndex2 = iPathPos; iIndex2 < iRelUntil; iIndex2++)
	{
		strOut += "..";
		strOut += FOLDER_SEP;
	}

	for(iIndex2 = iPathPos; iIndex2 < vecSrcTokens.size(); iIndex2++)
	{
		strOut += vecSrcTokens[iIndex2];
		if(iIndex2 + 1 < vecSrcTokens.size())
			strOut += FOLDER_SEP;
	}

	return true;
}
/*****************************************************************************/
void FileUtils::ensureIsGoodFileName(string& strInOut)
{
	StringUtils::ensureAlphaNumericAnd(g_pcsAllowedInternalFileNameChars, INTERNAL_FILENAME_REPLACEMENT_CHAR, strInOut);
}
/*****************************************************************************/
void FileUtils::listAllItemsOf(const char* pcsPath, TFileInfos& vecFileInfosOut)
{
	TStringVector vecTemp;
	int iCurr, iNum;
	SFileInfo rInfo;
	vecFileInfosOut.clear();

	FileUtils::listSubfoldersOf(pcsPath, vecTemp);
	iNum = vecTemp.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		rInfo.myFullPath = vecTemp[iCurr];
		PathUtils::extractFileNameFromPath(rInfo.myFullPath.c_str(), rInfo.myFileName);
		rInfo.myIsFolder = true;
		vecFileInfosOut.push_back(rInfo);
	}

	FileUtils::listFilesOf(pcsPath, vecTemp);
	iNum = vecTemp.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		rInfo.myFullPath = vecTemp[iCurr];
		PathUtils::extractFileNameFromPath(rInfo.myFullPath.c_str(), rInfo.myFileName);
		rInfo.myIsFolder = false;
		vecFileInfosOut.push_back(rInfo);
	}
}
/*****************************************************************************/
void FileUtils::applyFilters(TFileInfos& vecFileInfosOut, const char* pcsExtFilter, const char* pcsNameFilter)
{
	SFileInfo* pInfo;
	int iCurr, iNum = vecFileInfosOut.size();

	// Filter by extension first
	if(IS_VALID_STRING_AND_NOT_NONE(pcsExtFilter))
	{
		string strExtension;
		for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
		{
			pInfo = &vecFileInfosOut[iCurr];
			if(pInfo->myIsFolder)
				continue;

			strExtension = PathUtils::extractExtensionFromPath(pInfo->myFileName.c_str());
			std::transform(strExtension.begin(), strExtension.end(), strExtension.begin(), ::tolower);
			if(strExtension != pcsExtFilter)
				vecFileInfosOut.erase(vecFileInfosOut.begin() + iCurr);
		}
	}

	// Filter by name next
	if(IS_VALID_STRING_AND_NOT_NONE(pcsNameFilter))
	{
		string strName1(pcsNameFilter), strName2;
		std::transform(strName1.begin(), strName1.end(), strName1.begin(), ::tolower);
		iNum = vecFileInfosOut.size();
		for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
		{
			pInfo = &vecFileInfosOut[iCurr];
			if(pInfo->myIsFolder)
				continue;

			strName2 = pInfo->myFileName;
			std::transform(strName2.begin(), strName2.end(), strName2.begin(), ::tolower);
			if(strName2.find(strName1) == string::npos)
				vecFileInfosOut.erase(vecFileInfosOut.begin() + iCurr);
		}
	}
}
/*****************************************************************************/
void FileUtils::makePathAbsolute(const char* pcsRelativePath, const char* pcsCurrPath, bool bIsCurrPathAFile, string& strOut)
{
	strOut = "";
	if(!pcsRelativePath)
		return;

	// Note that we must make sure that the "relative" path given is actually relative:
	strOut = pcsRelativePath;
	int iInitPathLen = strlen(pcsRelativePath);
#ifdef WIN32
	if(iInitPathLen > 1 && pcsRelativePath[1] == ':')
		return;
#else
	if(iInitPathLen > 0 && pcsRelativePath[0] == FOLDER_SEP[0])
		return;
#endif

	// Tokenize both:
	string strDelims(FOLDER_SEP);
	string strSrcPath(pcsRelativePath);
	string strCurrPath(pcsCurrPath);
	TCharPtrVector vecSrcRelPathTokens, vecCurrPathTokens;
	TokenizeUtils::tokenizeStringToCharPtrsInPlace(strSrcPath, strDelims, vecSrcRelPathTokens);
	TokenizeUtils::tokenizeStringToCharPtrsInPlace(strCurrPath, strDelims, vecCurrPathTokens);

#ifdef WIN32
	// On Windows, the drive must be there.
	const int iMinNumAbsPathTokens = 1;
#else
	const int iMinNumAbsPathTokens = 0;
#endif

	if(bIsCurrPathAFile && vecCurrPathTokens.size() > iMinNumAbsPathTokens)
		vecCurrPathTokens.pop_back();


	// Now, go over all tokens in the relative path and push/pop things off the curr path.
	const char* pcsCurrRelToken;
	int iCurr, iNum = vecSrcRelPathTokens.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pcsCurrRelToken = vecSrcRelPathTokens[iCurr];
		if(IS_STRING_EQUAL(pcsCurrRelToken, "."))
			continue;
		else if(IS_STRING_EQUAL(pcsCurrRelToken, ".."))
		{
			if(vecCurrPathTokens.size() > iMinNumAbsPathTokens)
				vecCurrPathTokens.pop_back();
			ELSE_ASSERT;
		}
		else
		{
			// Push as a folder onto our stack
			vecCurrPathTokens.push_back(pcsCurrRelToken);
		}
	}

	// Now, assemble our final path
#ifdef WIN32
	strOut = "";
#else
	strOut = FOLDER_SEP;
#endif
	iNum = vecCurrPathTokens.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(iCurr > 0)
			strOut += FOLDER_SEP;
		strOut += vecCurrPathTokens[iCurr];
	}
}
/*****************************************************************************/
void FileUtils::ensureUserDataFolderExists(const char* pcsSubfolderName)
{
	// Ensure thumbs folder created
	string strCommonDataPath;
	getCommonAppDataPath(strCommonDataPath);
	strCommonDataPath += FOLDER_SEP;
	FileUtils::createFolderIn(strCommonDataPath.c_str(), pcsSubfolderName);
}
/*****************************************************************************/
};