#include "stdafx.h"
#include "OSXCore.h"

extern NSString* getDocumentsDir(void);

static char* g_pcsLoadFromFileCache = NULL;
static int g_iLoadFromFileCacheSize = 0;

namespace HyperCore
{
/*****************************************************************************/
void FileUtils::createFolderIn(const char* pcsPath, const char* pcsFolder)
{
	SCOPED_PATH_ACCESS(pcsPath);

    string strFullPath(pcsPath);
	if(pcsFolder)
	{
		PathUtils::ensureEndsWithFolderSep(strFullPath);
		strFullPath += pcsFolder;
	}
	NSString *pPath = [NSString stringWithCString:strFullPath.c_str()];
	NSError *error = NULL;
	if (![[NSFileManager defaultManager] createDirectoryAtPath:pPath withIntermediateDirectories:YES attributes:nil error:&error])
	{
		NSLog(@"Create directory error: %@", error);
	}
}
/*****************************************************************************/
void FileUtils::listSubfoldersOf(const char* pcsPath, TStringVector& strSubfoldersOut, bool bRecursive, bool bAppendToVector)
{
	SCOPED_PATH_ACCESS(pcsPath);

	if(!bAppendToVector)
		strSubfoldersOut.clear();
	NSURL *directoryURL = [NSURL fileURLWithPath:[NSString stringWithCString:pcsPath] isDirectory:true]; 
	NSArray *keys = [NSArray arrayWithObjects: NSURLIsDirectoryKey, NSURLIsPackageKey, NSURLLocalizedNameKey, nil];

	//  | NSDirectoryEnumerationSkipsSubdirectoryDescendants ?
	NSDirectoryEnumerator *enumerator = [[NSFileManager defaultManager] enumeratorAtURL:directoryURL includingPropertiesForKeys:keys
                                     options:(NSDirectoryEnumerationSkipsPackageDescendants | NSDirectoryEnumerationSkipsHiddenFiles)
                                     errorHandler:^(NSURL *url, NSError *error) 
									 {
                                         return YES;
                                     }];
	NSString *localizedName = nil;
	NSNumber *isDirectory = nil;
	string strTemp;
    string strTopPath(pcsPath);
	PathUtils::ensureEndsWithFolderSep(strTopPath);
	for (NSURL *url in enumerator)
	{
		[url getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:NULL];
		if ([isDirectory boolValue]) 
		{        
	        [url getResourceValue:&localizedName forKey:NSURLLocalizedNameKey error:NULL];
			strTemp = getCStringSafe(localizedName);
            
            strTemp = strTopPath + strTemp;
			strSubfoldersOut.push_back(strTemp);

			// If recursive, recurse:
			if(bRecursive)
				FileUtils::listSubfoldersOf(strTemp.c_str(), strSubfoldersOut, true, true);
		}
	}
}
/*****************************************************************************/
void FileUtils::deleteFile(const char* pcsFilename, bool bFullPath)
{
	SCOPED_PATH_ACCESS(pcsFilename);

	NSString *appFile;
	if(bFullPath)
		appFile = [NSString stringWithCString:pcsFilename];
	else
	{
		NSString *documentsDirectory = getDocumentsDir();
		// the path to write file
		appFile = [documentsDirectory stringByAppendingPathComponent:[NSString stringWithCString:pcsFilename]];
	}
	
	NSFileManager *fileManager = [NSFileManager defaultManager];
	[fileManager removeItemAtPath:appFile error:NULL];	
}
/*****************************************************************************/
bool FileUtils::doesFileExist(const char* pcsFilename, FileSourceType eFileSourceType)
{
	SCOPED_PATH_SOURCE_ACCESS(pcsFilename, eFileSourceType);

	NSString *appFile;
	if(eFileSourceType == FileSourceUserDir)
	{
		NSString *documentsDirectory = getDocumentsDir();
		// the path to read file
		appFile = [documentsDirectory stringByAppendingPathComponent:[NSString stringWithCString:pcsFilename]];
	}
	else if(eFileSourceType == FileSourcePackageDir)
	{
		NSBundle*				pBundle = [NSBundle mainBundle];
		appFile = [pBundle pathForResource:[NSString stringWithCString:pcsFilename] ofType:nil];
	}
	else if(eFileSourceType == FileSourceFullPath)
	{
		appFile = [NSString stringWithCString:pcsFilename];
	}
	else 
	{
		_ASSERT(0);
	}

	return [[NSFileManager defaultManager] fileExistsAtPath:appFile];
}
/*****************************************************************************/
bool FileUtils::loadFromFile(const char* pcsFilename, string &str, FileSourceType eFileSource)
{
	SCOPED_PATH_SOURCE_ACCESS(pcsFilename, eFileSource);

	NSString *appFile;
	if(eFileSource == FileSourceUserDir)
	{
		NSString *documentsDirectory = getDocumentsDir();
		// the path to read file
		appFile = [documentsDirectory stringByAppendingPathComponent:[NSString stringWithCString:pcsFilename]];
	}
	else if(eFileSource == FileSourcePackageDir)
	{
		NSBundle*				pBundle = [NSBundle mainBundle];
		appFile = [pBundle pathForResource:[NSString stringWithCString:pcsFilename] ofType:nil];
	}
	else if(eFileSource == FileSourceFullPath)
	{
		appFile = [NSString stringWithCString:pcsFilename];
	}
	ELSE_ASSERT;
    
#ifdef _DEBUG
    NSLog(@"Loading %@", appFile);
#endif
	
	bool bRes = false;
	NSData *data = [NSData dataWithContentsOfFile:appFile];
	const char* pcsPtr = (const char*)[data bytes];
	if(pcsPtr)
	{
		int iLength = [data length];
		
		if(iLength + 1 > g_iLoadFromFileCacheSize)
		{
			if(g_pcsLoadFromFileCache)
				delete[] g_pcsLoadFromFileCache;
			g_iLoadFromFileCacheSize = (iLength + 1) + 10;
			g_pcsLoadFromFileCache = new char[g_iLoadFromFileCacheSize];
		}
		
		memcpy(g_pcsLoadFromFileCache, pcsPtr, sizeof(char)*iLength);
		g_pcsLoadFromFileCache[iLength] = 0;
		str = g_pcsLoadFromFileCache;
		bRes = true;
	}
	else
		str = "";

#ifdef _DEBUG
	if(bRes)
		NSLog(@" - SUCCESS\n");
	else
		NSLog(@" - FAIL\n");
#endif
	return bRes;
}
/*****************************************************************************/
char* FileUtils::loadFromFileBinary(const char* pcsFilename, FileSourceType eFileSource, int &iBytesOut)
{
	SCOPED_PATH_SOURCE_ACCESS(pcsFilename, eFileSource);

	iBytesOut = 0;
	
	NSString *appFile;
	
	if(eFileSource == FileSourceUserDir)
	{
		NSString *documentsDirectory = getDocumentsDir();
		// the path to read file
		appFile = [documentsDirectory stringByAppendingPathComponent:[NSString stringWithCString:pcsFilename]];
	}
	else if(eFileSource == FileSourcePackageDir)
	{
		NSBundle*				pBundle = [NSBundle mainBundle];
		appFile = [pBundle pathForResource:[NSString stringWithCString:pcsFilename] ofType:nil];
	}
	else if(eFileSource == FileSourceFullPath)
	{
		appFile = [NSString stringWithCString:pcsFilename];
	}
	ELSE_ASSERT;
	
	NSData *data = [NSData dataWithContentsOfFile:appFile];
	const char* pcsPtr = (const char*)[data bytes];
	if(pcsPtr)
	{
		int iLength = [data length];
	
		char* pcsRes = new char[iLength];
		memcpy(pcsRes, pcsPtr, sizeof(char)*iLength);
		iBytesOut = iLength;
		return pcsRes;
	}
	else
		return NULL;

}
/*****************************************************************************/
bool FileUtils::saveToFile(const char* pcsFilename, const char* pcsString, bool bFullPath /*= false*/)
{
	const char* pcsRes;
	int iSizeExtender = 0;
	pcsRes = pcsString;
	return FileUtils::saveToFileBinary(pcsFilename, pcsRes, strlen(pcsString) + iSizeExtender, bFullPath);
}
/*****************************************************************************/
bool FileUtils::saveToFileBinary(const char* pcsFilename, const char* pcsBuff, int iLen, bool bFullPath)
{
	SCOPED_PATH_SOURCE_ACCESS(pcsFilename, bFullPath ? FileSourceFullPath : FileSourceUserDir);

	// What about the trailing zero?
	NSData *data = [[NSData alloc] initWithBytes:pcsBuff length:iLen]; 

	NSString *appFile;
	if(bFullPath)
	{
		appFile = [NSString stringWithCString:pcsFilename];
	}
	else
	{	
		NSString *documentsDirectory = getDocumentsDir();
		
		// the path to write file
		NSString* pNameString = [NSString stringWithCString:pcsFilename];
		appFile = [documentsDirectory stringByAppendingPathComponent:pNameString];
	}
	[data writeToFile:appFile atomically:YES];
	
	[data release];
    return true;
}
/*****************************************************************************/
UTC_TYPE FileUtils::getFileModifiedDate(const char* pcsPath)
{
	SCOPED_PATH_ACCESS(pcsPath);

	NSError *pErrorOut = nil;
	NSDate *nsDateTemp;
	NSURL *pUrl = [NSURL fileURLWithPath:[NSString stringWithCString:pcsPath] isDirectory:false];
	bool bRes = [pUrl getResourceValue:&nsDateTemp forKey:NSURLContentModificationDateKey error:&pErrorOut];
	_ASSERT(bRes);
	if(bRes)
		return [nsDateTemp timeIntervalSince1970];
	else
		return 0;
}
/*****************************************************************************/
void FileUtils::killLoadFileCache()
{
	if(g_pcsLoadFromFileCache)
		delete[] g_pcsLoadFromFileCache;
	g_pcsLoadFromFileCache = NULL;
	g_iLoadFromFileCacheSize = 0;
}
/*****************************************************************************/
void FileUtils::loadFromFileBinary(const char* pcsFilename, FileSourceType eFileSource, BinaryData& rDataOut)
{
	int iLength;
	char* pcsData = loadFromFileBinary(pcsFilename, eFileSource, iLength);
	rDataOut.setData(pcsData, iLength, true);
	///rDataOut.myData = loadFromFileBinary(pcsFilename, eFileSource, rDataOut.myLength);
}
/*****************************************************************************/
void FileUtils::listFilesOf(const char* pcsPath, TStringVector& strFilesOut, bool bAppendToVector)
{
	if(!bAppendToVector)
		strFilesOut.clear();

	if(!IS_VALID_STRING(pcsPath))
		return;

	SCOPED_PATH_ACCESS(pcsPath);

	NSURL *directoryURL = [NSURL fileURLWithPath:[NSString stringWithCString:pcsPath] isDirectory:true]; 
	NSArray *keys = [NSArray arrayWithObjects: NSURLIsDirectoryKey, NSURLIsPackageKey, NSURLLocalizedNameKey, nil];

	string strTemp;
    string strTopPath(pcsPath);
	PathUtils::ensureEndsWithFolderSep(strTopPath);
	
	NSArray *pFileArray = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:directoryURL includingPropertiesForKeys:keys
                                         options:NSDirectoryEnumerationSkipsHiddenFiles
                                         error:nil];
										 
	
/*
	// This is a DEEP list...
	NSDirectoryEnumerator *enumerator = [[NSFileManager defaultManager] enumeratorAtURL:directoryURL includingPropertiesForKeys:keys
                                     options:(NSDirectoryEnumerationSkipsPackageDescendants | NSDirectoryEnumerationSkipsHiddenFiles)
                                     errorHandler:^(NSURL *url, NSError *error) 
									 {
                                         return YES;
                                     }];


	// This is a shallow list
	NSDirectoryEnumerator *enumerator = [[NSFileManager defaultManager] enumeratorAtURL:directoryURL includingPropertiesForKeys:keys
                                     options:(NSDirectoryEnumerationSkipsPackageDescendants | NSDirectoryEnumerationSkipsHiddenFiles | NSDirectoryEnumerationSkipsSubdirectoryDescendants)
                                     errorHandler:^(NSURL *url, NSError *error) 
									 {
                                         return YES;
                                     }];
*/
	NSString *localizedName = nil;
	NSNumber *isDirectory = nil;
	//for (NSURL *url in enumerator)
	for(NSURL *url in pFileArray)
	{
		[url getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:NULL];
		if (![isDirectory boolValue]) 
		{        
	        [url getResourceValue:&localizedName forKey:NSURLLocalizedNameKey error:NULL];
			strTemp = getCStringSafe(localizedName);
            
            strTemp = strTopPath + strTemp;
			strFilesOut.push_back(strTemp);
		}
	}

}
/*****************************************************************************/
void FileUtils::deleteFolder(const char* pcsFullFolderPath)
{
	SCOPED_PATH_ACCESS(pcsFullFolderPath);

	NSFileManager *fileManager = [NSFileManager defaultManager];
	[fileManager removeItemAtPath:[NSString stringWithCString:pcsFullFolderPath] error:NULL];	
}
/*****************************************************************************/
void FileUtils::makeFileHidden(const char* pcsPath)
{
	// Nothing.
}
/*****************************************************************************/
bool FileUtils::canAccessFileForWriting(const char* pcsPath)
{
	SCOPED_PATH_ACCESS(pcsPath);

	FILE *out = fopen(pcsPath, "w");
	if(out)
	{
		fclose(out);
		return true;
	}
	else
		return false;
	//return [[NSFileManager defaultManager] createFileAtPath:[NSURL URLWithString:[NSString stringWithCString:pcsPath]] contents:[NSData data] attributes:nil];
}
/*****************************************************************************/
bool FileUtils::canAccessFileForReading(const char* pcsPath)
{
	SCOPED_PATH_ACCESS(pcsPath);

	FILE *out = fopen(pcsPath, "rb");
	if(out)
	{
		fclose(out);
		return true;
	}
	else
		return false;
}
/*****************************************************************************/
void FileUtils::listAllDrives(TDriveInfos& vecDrivesOut)
{
	vecDrivesOut.clear();
	_ASSERT(0);
}
/*****************************************************************************/
HUGE_SIZE_TYPE FileUtils::getFileSize(const char* pcsPath)
{
	SCOPED_PATH_ACCESS(pcsPath);
	return [[[NSFileManager defaultManager] attributesOfItemAtPath:[NSString stringWithCString:pcsPath] error:nil] fileSize];
}
/*****************************************************************************/
bool FileUtils::doesFolderExist(const char* pcsFolderPath, FileSourceType eFileSourceType)
{
	SCOPED_PATH_SOURCE_ACCESS(pcsFolderPath, eFileSourceType);
	try
	{
		NSString *appFile;
		if(eFileSourceType == FileSourceUserDir)
		{
			NSString *documentsDirectory = getDocumentsDir();
			// the path to read file
			appFile = [documentsDirectory stringByAppendingPathComponent:[NSString stringWithCString:pcsFolderPath]];
		}
		else if(eFileSourceType == FileSourcePackageDir)
		{
			NSBundle* pBundle = [NSBundle mainBundle];
			appFile = [pBundle pathForResource:[NSString stringWithCString:pcsFolderPath] ofType:nil];
		}
		else if(eFileSourceType == FileSourceFullPath)
		{
			appFile = [NSString stringWithCString:pcsFolderPath];
		}
		else 
		{
			_ASSERT(0);
		}

		BOOL isDir = NO;
		BOOL bDoesExist = [[NSFileManager defaultManager] fileExistsAtPath:appFile isDirectory:&isDir];

		return bDoesExist && isDir;
	}
	catch(...)
	{
		//_ASSERT(0);
	}
	return false;
}
/*****************************************************************************/
bool FileUtils::getIsPathAbsolute(const char* pcsPath)
{
	int iLen = strlen(pcsPath);
	if(iLen == 0)
		return false;
	
	if(iLen > 0 && pcsPath[0] == '/')
		return true;

	if(iLen > 1 && pcsPath[0] == '~' && pcsPath[1] == '/')
		return true;

	return false;
}
/*****************************************************************************/
}
