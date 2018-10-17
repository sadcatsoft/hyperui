#pragma once

/*****************************************************************************/
class HYPERCORE_API PathUtils
{
public:	

	static const char* extractExtensionFromPath(const char* pcsPath);
	static void extractFileNameFromPath(const char* pcsPath, string& strFileNameOut);
	static void extractPureFileNameFromPath(const char* pcsPath, string& strPureNameOut);
	static void ensurePathEndsInExtension(string& strPathInOut, const char* pcsExtension, bool bReplaceExisting);
	static void extractPathFromFullFilename(const char* pcsFullPath, string& strPathOut);
	static void ensureEndsWithFolderSep(string& strInOut);
};
/*****************************************************************************/