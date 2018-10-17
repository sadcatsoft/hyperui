#pragma once

/*****************************************************************************/
enum FileSourceType
{
	FileSourcePackageDir = 0,
	FileSourceUserDir,
	FileSourceFullPath
};
/*****************************************************************************/
enum DriveType
{
	DriveUnknown = 0,
	DriveFixed,
	DriveNetwork,
	DriveRemovable,
	DriveCDRom
};

struct SDriveInfo
{
	string myName;
	string myLabel;
	DriveType myType;
};
typedef vector <SDriveInfo> TDriveInfos;
/*****************************************************************************/
struct SFileInfo
{
	string myFullPath;
	string myFileName;
	bool myIsFolder;
};

typedef vector < SFileInfo > TFileInfos;
/*****************************************************************************/
HYPERCORE_API void gStartOpeningPath(const char* pcsPath);
HYPERCORE_API void gEndOpeningPath(const char* pcsPath);

class ScopedPathAccess
{
public:
	ScopedPathAccess(const CHAR_TYPE* pcsPath, FileSourceType eFileSource) 
	{ 
		if(eFileSource == FileSourceFullPath)
		{
			myPath = pcsPath; 
			gStartOpeningPath(pcsPath); 
		}
	}
	~ScopedPathAccess() 
	{ 
		if(myPath.length() > 0)
			gEndOpeningPath(myPath.c_str()); 
	}

private:
	STRING_TYPE myPath;
};

#define SCOPED_PATH_ACCESS(pcsPath)							ScopedPathAccess rPathAccess(pcsPath, FileSourceFullPath);
#define SCOPED_PATH_SOURCE_ACCESS(pcsPath, eFileSource)		ScopedPathAccess rPathAccess(pcsPath, eFileSource);
/*****************************************************************************/
class FileUtils
{
public:

	// Directory/folder methods
	HYPERCORE_API static void createFolderIn(const char* pcsPath, const char* pcsFolder);
	HYPERCORE_API static void listSubfoldersOf(const char* pcsPath, TStringVector& strSubfoldersOut, bool bRecursive = false, bool bAppendToVector = false);
	HYPERCORE_API static void listFilesOf(const char* pcsPath, TStringVector& strFilesOut, bool bAppendToVector = false);
	HYPERCORE_API static void listAllItemsOf(const char* pcsPath, TFileInfos& vecFileInfosOut);
	HYPERCORE_API static void ensureUserDataFolderExists(const char* pcsSubfolderName);

	HYPERCORE_API static void listAllDrives(TDriveInfos& vecDrivesOut);
	HYPERCORE_API static void applyFilters(TFileInfos& vecFileInfosOut, const char* pcsExtFilter, const char* pcsNameFilter);

	// File ops
	HYPERCORE_API static void deleteFile(const char*pcsFilename, bool bFullPath = false);
	HYPERCORE_API static void deleteFolder(const char*pcsFullFolderPath);
	HYPERCORE_API static bool doesFileExist(const char* pcsFilename, FileSourceType eFileSourceType);
	HYPERCORE_API static bool doesFolderExist(const char* pcsFolderPath, FileSourceType eFileSourceType);
	HYPERCORE_API static bool getIsFolder(const char* pcsFolderPath, FileSourceType eFileSourceType = FileSourceFullPath) { return FileUtils::doesFolderExist(pcsFolderPath, eFileSourceType); }
	HYPERCORE_API static UTC_TYPE getFileModifiedDate(const char* pcsPath);
	HYPERCORE_API static HUGE_SIZE_TYPE getFileSize(const char* pcsPath);
	HYPERCORE_API static void makeFileHidden(const char* pcsPath);
	HYPERCORE_API static bool canAccessFileForWriting(const char* pcsPath);
	HYPERCORE_API static bool canAccessFileForReading(const char* pcsPath);

	// Save/load
	HYPERCORE_API static bool loadFromFile(const char* pcsFilename, string &str, FileSourceType eFileSource);
	HYPERCORE_API static char* loadFromFileBinary(const char* pcsFilename, FileSourceType eFileSource, int &iBytesOut);
	HYPERCORE_API static void loadFromFileBinary(const char* pcsFilename, FileSourceType eFileSource, BinaryData& rDataOut);
	HYPERCORE_API static bool saveToFile(const char* pcsFilename, const char* pcsString, bool bFullPath = false);
	HYPERCORE_API static bool saveToFileBinary(const char* pcsFilename, const char* pcsBuff, int iLen, bool bFullPath = false);

	// Utility
	HYPERCORE_API static void killLoadFileCache();
	HYPERCORE_API static bool getIsPathAbsolute(const char* pcsPath);
	HYPERCORE_API static bool makePathRelative(const char* pcsSrcPath, const char* pcsRelativeToPath, bool bDoesRelativePathIncludeFileName, string& strOut);
	HYPERCORE_API static void makePathAbsolute(const char* pcsRelativePath, const char* pcsCurrPath, bool bIsCurrPathAFile, string& strOut);
	HYPERCORE_API static void ensureIsGoodFileName(string& strInOut);
};
/*****************************************************************************/