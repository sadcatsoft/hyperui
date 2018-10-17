#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
void FormatsManager::getExtensionsList(bool bIncludeAllCaps, int iIOType, TStringVector& vecOut, bool bDontListNative)
{
	string strTemp;
	vecOut.clear();

	IImageFormatDef** pAllFormats = this->getAllFormats();
	const char* pcsNativeExtension = this->getNativeExtension();

	int iIndex;
	for(iIndex = 0; pAllFormats[iIndex]->getFlags() >= 0; iIndex++)
	{
		if( (pAllFormats[iIndex]->getFlags() & iIOType) == 0)
			continue;

		if(bDontListNative && IS_STRING_EQUAL(pAllFormats[iIndex]->getExtension(), pcsNativeExtension))
			continue;

		vecOut.push_back(pAllFormats[iIndex]->getExtension());
		if(bIncludeAllCaps)
		{
			strTemp = pAllFormats[iIndex]->getExtension();
			std::transform(strTemp.begin(), strTemp.end(), strTemp.begin(), ::tolower);
			vecOut.push_back(strTemp);
		}
	}
}
/*****************************************************************************/
void FormatsManager::getDescriptionsList(bool bIncludeAllCaps, int iIOType, TStringVector& vecOut, bool bDontListNative)
{
	string strTemp;
	vecOut.clear();

	IImageFormatDef** pAllFormats = this->getAllFormats();
	const char* pcsNativeExtension = this->getNativeExtension();

	int iIndex;
	for(iIndex = 0; pAllFormats[iIndex]->getFlags() >= 0; iIndex++)
	{
		if( (pAllFormats[iIndex]->getFlags() & iIOType) == 0)
			continue;

		if(bDontListNative && IS_STRING_EQUAL(pAllFormats[iIndex]->getExtension(), pcsNativeExtension))
			continue;

		vecOut.push_back(pAllFormats[iIndex]->getDescription());
		if(bIncludeAllCaps)
		{
			strTemp = pAllFormats[iIndex]->getDescription();
			std::transform(strTemp.begin(), strTemp.end(), strTemp.begin(), ::tolower);
			vecOut.push_back(strTemp);
		}
	}
}
/*****************************************************************************/
int FormatsManager::getFilterIndexFromPath(const char* pcsPath, bool bForWriting)
{
	if(!pcsPath || strlen(pcsPath) == 0)
		return 0;

	FormatIOType eFilter;
	if(bForWriting)
		eFilter = FormatWrite;
	else
		eFilter = FormatRead;

	IImageFormatDef** pAllFormats = this->getAllFormats();

	const char* pcsExtension = PathUtils::extractExtensionFromPath(pcsPath);
	string strLowerExt(pcsExtension);
	transform(strLowerExt.begin(), strLowerExt.end(), strLowerExt.begin(), ::tolower);

	int iIndex;
	int iCountedFormats = 0;
	for(iIndex = 0; pAllFormats[iIndex]->getFlags() >= 0; iIndex++)
	{
		if( (pAllFormats[iIndex]->getFlags() & eFilter) == 0)
			continue;

		if(strLowerExt == pAllFormats[iIndex]->getExtension())
			return iCountedFormats + 1;

		iCountedFormats++;
	}

	return 1;
}
/*****************************************************************************/
bool FormatsManager::canOpenFile(const char* pcsPath)
{
	IImageFormatDef** pAllFormats = this->getAllFormats();

	const char* pcsExtension = PathUtils::extractExtensionFromPath(pcsPath);
	string strLowerExt(pcsExtension);
	transform(strLowerExt.begin(), strLowerExt.end(), strLowerExt.begin(), ::tolower);
	int iIndex;
	for(iIndex = 0; pAllFormats[iIndex]->getFlags() >= 0; iIndex++)
	{
		if( (pAllFormats[iIndex]->getFlags() & FormatRead) == 0)
			continue;
		if(strLowerExt == pAllFormats[iIndex]->getExtension())
			return true;
	}

	return false;
}
/*****************************************************************************/
bool FormatsManager::canOpenAnyFileFrom(const TStringVector& vecFileNames, bool bAllowFolders)
{
	int iCurr, iNum = vecFileNames.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(canOpenFile(vecFileNames[iCurr].c_str()))
			return true;

		if(bAllowFolders && FileUtils::getIsFolder(vecFileNames[iCurr].c_str()))
			return true;
	}
	return false;
}
/*****************************************************************************/
void FormatsManager::reduceToSupportedOpenTypes(TStringVector& vecFiles)
{
	const char* pcsExt;
	int iCurr, iNum = vecFiles.size();
	for(iCurr = iNum - 1; iCurr >= 0; iCurr--)
	{
		if(!canOpenFile(vecFiles[iCurr].c_str()))
			vecFiles.erase(vecFiles.begin() + iCurr);
	}
}
/*****************************************************************************/
bool FormatsManager::getIsNativeFormat(const char* pcsExtension)
{
	if(!pcsExtension)
		return false;

	string strLowerExt(pcsExtension);
	transform(strLowerExt.begin(), strLowerExt.end(), strLowerExt.begin(), ::tolower);
	return strLowerExt == this->getNativeExtension();
}
/*****************************************************************************/
IImageFormatDef** FormatsManager::getAllFormats()
{
	if(myCachedFormats.size() == 0)
		this->getAllFormats(myCachedFormats);
	return &myCachedFormats[0];
}
/*****************************************************************************/
};
