#include "stdafx.h"
#include "OSXCore.h"

#define BOOKMARK_FILENAME		"sec_bookmarks.txt"
#define TAG_BOOKMARK            "bookmark"

#define SAVE_BOOKMARK_PATH      "path"
#define SAVE_BOOKMARK_DATA      "data"

namespace HyperCore
{
BookmarkManager* BookmarkManager::theInstance = NULL;
/*****************************************************************************/
BookmarkManager::BookmarkManager()
{
	// Read the bookmarks that exist already
	load();
}
/*****************************************************************************/
BookmarkManager::~BookmarkManager()
{
	clear();
}
/*****************************************************************************/
BookmarkManager* BookmarkManager::getInstance()
{
	if(!theInstance)
		theInstance = new BookmarkManager;
	return theInstance;
}
/*****************************************************************************/
bool BookmarkManager::addBookmark(NSURL* pUrl)
{
#ifdef APPSTORE_BUILD
	SCOPED_MUTEX_LOCK(&myLock);
	// Make the key
	string strPath = [MacGameView getCStringSafe:[pUrl path]];

	// Now, convert url to bookmark
	NSError *error = nil;
	NSData *bookmarkData = [pUrl bookmarkDataWithOptions:NSURLBookmarkCreationWithSecurityScope includingResourceValuesForKeys:nil relativeToURL:nil error:&error];

	if(error)
    {
        string strError = [MacGameView getCStringSafe:error.domain];
        string strDesc = [MacGameView getCStringSafe:error.localizedDescription];
        Logger::log("BOOKMARK_MGR: Error adding bookmark for %s; domain = %s code = %d desc = %s\n", strPath.c_str(), strError.c_str(), (int)error.code, strDesc.c_str());
		return false;
    }
    
    NSData* pOurCopy = [[NSData alloc] initWithData:bookmarkData];
    
	gLog("BOOKMARK_MGR: Successfully added bookmark for %s\n", strPath.c_str());

	myBookmarkData[strPath] = pOurCopy;
	save();
	return true;
#else
	return false;
#endif
}
/*****************************************************************************/
NSURL* BookmarkManager::getBookmark(const char* pcsPath)
{
#ifdef APPSTORE_BUILD
	SCOPED_MUTEX_LOCK(&myLock);
	TStringNSDataMap::iterator mi = myBookmarkData.find(pcsPath);

	if(mi == myBookmarkData.end())
	{
		// It may be the case that we're trying to open a file within a folder,
		// and while we have no exact match for the full path, we may have a
		// match for some parent somewhere. Try it:

		// Actually, if we've opened this before, we'll have it in a map:
		TStringStringMap::iterator ssmi = myActualBookmarkPaths.find(pcsPath);
		if(ssmi != myActualBookmarkPaths.end())
			mi = myBookmarkData.find(ssmi->second);

		// If we still don't have it, do a full manual traversal
		if(mi == myBookmarkData.end())
		{
			for(mi = myBookmarkData.begin(); mi != myBookmarkData.end(); mi++)
			{
				if(gDoesStringStartWith(pcsPath, mi->first.c_str()))
					break;
			}
		}
	}

	if(mi == myBookmarkData.end())
    {
        Logger::log("BOOKMARK_MGR: Error getting bookmark for %s\n", pcsPath);
		return NULL;
    }

  
	NSError *error = nil;
	BOOL bookmarkDataIsStale;
	NSURL *bookmarkFileURL = nil;
 
	bookmarkFileURL = [NSURL URLByResolvingBookmarkData:mi->second options:NSURLBookmarkResolutionWithSecurityScope relativeToURL:nil bookmarkDataIsStale:&bookmarkDataIsStale error:&error];
	if(error)
    {
        string strError = [MacGameView getCStringSafe:error.domain];
        string strDesc = [MacGameView getCStringSafe:error.localizedDescription];
        Logger::log("BOOKMARK_MGR: Error getting bookmark for %s; domain = %s code = %d desc = %s\n", pcsPath, strError.c_str(), (int)error.code, strDesc.c_str());
		return NULL;
    }
	else
	{
		// We need to remember the path we've actually used, which may be different from the original
		myActualBookmarkPaths[pcsPath] = mi->first;
		return bookmarkFileURL;
	}
#else
	return NULL;
#endif
}
/*****************************************************************************/
void BookmarkManager::save()
{
#ifdef APPSTORE_BUILD
	// Go over all items, convert to base64 strings, save to collection.
	StringResourceCollection rCollection;
	StringResourceItem *pItem;
	TStringNSDataMap::iterator mi;
	NSData* pData;

	char pcsBuff[256];

	unsigned int iSize;
	const unsigned char* pcsRawData;
	char* pcsEncData;
	int iCounter;
	for(mi = myBookmarkData.begin(), iCounter = 0; mi != myBookmarkData.end(); mi++, iCounter++)
	{
		pData = mi->second;
		pcsRawData = (const unsigned char*)[pData bytes];
		iSize = [pData length];
		pcsEncData = Base64Coder::encode(pcsRawData, iSize, false);
		
		sprintf(pcsBuff, "bookmark%d", iCounter);
		pItem = rCollection.addItem(pcsBuff, TAG_BOOKMARK);
		pItem->setStringProp(SAVE_BOOKMARK_PATH, mi->first.c_str());
		pItem->setStringProp(SAVE_BOOKMARK_DATA, pcsEncData);

		delete[] pcsEncData;
	}

	string strSaveData;
	rCollection.saveToString(strSaveData);
	FileUtils::saveToFile(BOOKMARK_FILENAME, strSaveData.c_str());
#endif
}
/*****************************************************************************/
void BookmarkManager::load()
{
#ifdef APPSTORE_BUILD
	string strData;
	FileUtils::loadFromFile(BOOKMARK_FILENAME, strData, FileSourceUserDir, false);
    if(strData.length() == 0)
        return;
    clear();
    
	StringResourceCollection rCollection;
	if(!rCollection.loadFromString(strData))
		ASSERT_RETURN;

	unsigned char* pDecodedData;
	unsigned int iDecSizeOut;

	const char* pcsEncData;
	
	NSData* pData;
	StringResourceItem *pItem;
	int iChild, iNumChildren = rCollection.getNumItems();
	for(iChild = 0; iChild < iNumChildren; iChild++)
	{
		pItem = rCollection.getItem(iChild);

		pcsEncData = pItem->getStringProp(SAVE_BOOKMARK_DATA);
		pDecodedData = Base64Coder::decode(pcsEncData, strlen(pcsEncData), iDecSizeOut, false);

		pData = [[NSData alloc] initWithBytes:pDecodedData length:iDecSizeOut];
		if(pData)
			myBookmarkData[pItem->getStringProp(SAVE_BOOKMARK_PATH)] = pData;

		delete[] pDecodedData;
	}
#endif
}
/*****************************************************************************/
void BookmarkManager::clear()
{
    NSData *pData;
    TStringNSDataMap::iterator mi;
	for(mi = myBookmarkData.begin(); mi != myBookmarkData.end(); mi++)
	{
		pData = mi->second;
        [pData release];
    }
    myBookmarkData.clear();
}
/*****************************************************************************/
}
