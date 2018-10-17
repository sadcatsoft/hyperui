#pragma once

class HYPERUI_API IThumbnailProvider;
class HYPERUI_API UIElement;
/*****************************************************************************/
class HYPERUI_API IThumbnailRenderCallback
{
public:
	virtual ~IThumbnailRenderCallback() { }
	virtual void onThreadedThumbnailRenderFinished(const IBaseImage* pThumbnail, IThumbnailProvider* pProvider, int iIndex, const char* pcsOptParentId) = 0;
};
/*****************************************************************************/
class HYPERUI_API ThumbnailRenderInfo
{
public:
	ThumbnailRenderInfo() { myDidCancel = false;  }
	virtual ~ThumbnailRenderInfo()
	{
		// Note that we deliberately don't delete the result since it's simply assigned
		// as a pointer in UserImageWindow...
	}

	virtual void start(IThumbnailProvider* pProvider) = 0;
	virtual IBaseImage* allocateImage() = 0;

	string mySourceUiElemId;
	UNIQUEID_TYPE myId;
	UNIQUEID_TYPE myWindowId;
	bool myDidCancel;

	SVector2D mySize;
	int myIndex;
	string myOptParentId;

protected:
	void process(IThumbnailProvider* pProvider);
};
typedef map < UNIQUEID_TYPE, ThumbnailRenderInfo* > TIdThumbnailRenderInfoMap;
typedef list < ThumbnailRenderInfo* > TIdThumbnailRenderInfoList;
/*****************************************************************************/
class HYPERUI_API IThumbnailProvider : public GlobalIdGeneratorInternal<ThumbnailRenderInfo>
{
public:
	virtual ~IThumbnailProvider();

	virtual int getNumThumbnails() const = 0;

	void queueThreadedThumbnail(int iIndex, const SVector2D& svSize, UIElement* pCallbackElem, const char* pcsOptParentId = NULL, bool bPutInFront = false);
	virtual const char* getTargetData(int iIndex) { return NULL; }

	virtual FLOAT_TYPE getThumbnailPadding() { return -1.0; }

	virtual ThumbnailRenderInfo* allocateRenderInfo() = 0;

	void deleteSelf();

	bool getIsRenderingAnything();
	bool getIsQueued(int iIndex, const char* pcsOptParentId = NULL);
	bool moveToFront(int iIndex, const char* pcsOptParentId = NULL);
	bool removeFromQueue(int iIndex, const char* pcsOptParentId = NULL);
	void removeAllThumbnailsFor(const char* pcsParentId);

	//void nudgeExistingJobs();

	void cancelAllJobs();

protected:

	virtual void getThumbnailReentrant(int iIndex, const char* pcsOptParentId, const SVector2D& svSize, IBaseImage& rImageOut) const = 0;
	void cleanup(ThumbnailRenderInfo* pInfo);
	void cancelAllJobsAndWait();

	friend class ThumbnailRenderInfo;

	virtual bool getAllowStartingNextJob() const { return true; }
	virtual void onJobStarted() { }
	virtual void onJobFinished() { }

	virtual void yield() = 0;

private:

	void startNextJob();

private:

	TIdThumbnailRenderInfoMap myJobs;
	TIdThumbnailRenderInfoList myQueue;

	//boost::recursive_mutex myMapLock;
	Mutex myMapLock;
};
/*****************************************************************************/
// This is a provider that can do both sync and async thumbnails.
class HYPERUI_API IThreadlessThumbnailProvider : public IThumbnailProvider
{
public:
	virtual ~IThreadlessThumbnailProvider() { }

	virtual AccelImage* getThumbnail(int iIndex, const SVector2D& svSize) const = 0;
	virtual AccelImage* getThumbnail(const char* pcsFullId, const SVector2D& svSize) const = 0;
};
/*****************************************************************************/