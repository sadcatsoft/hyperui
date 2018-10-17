#pragma once

namespace HyperCore
{
typedef map < string, NSData* > TStringNSDataMap;
/*****************************************************************************/
class BookmarkManager
{
public:
	~BookmarkManager();
	static BookmarkManager* getInstance();

	bool addBookmark(NSURL* pUrl);
	NSURL* getBookmark(const char* pcsPath);

private:
	BookmarkManager();

	void load();
	void save();

    void clear();
    
private:

	static BookmarkManager* theInstance;
	TStringNSDataMap myBookmarkData;

	TStringStringMap myActualBookmarkPaths;

	Mutex myLock;
};
/*****************************************************************************/
}
