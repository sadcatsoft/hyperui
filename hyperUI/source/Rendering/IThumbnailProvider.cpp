#include "stdafx.h"

#if defined(_DEBUG) || defined(_DEBUG)
#define DEBUG_THUMBNAIL_GEN
#endif

namespace HyperUI
{
/*****************************************************************************/
void ThumbnailRenderInfo::process(IThumbnailProvider* pProvider)
{
#ifdef DEBUG_THUMBNAIL_GEN
	gLog("THUMBNAIL: Processing %d parent = %s\n", this->myIndex, this->myOptParentId.length() > 0 ? this->myOptParentId.c_str() : "NULL");
#endif

	if(!myDidCancel)
	{
		IBaseImage* pResImage = this->allocateImage();
		pProvider->getThumbnailReentrant(this->myIndex, this->myOptParentId.c_str(), this->mySize, *pResImage);
		Window *pWindow = WindowManager::getInstance()->findItemById(myWindowId);
		IThumbnailRenderCallback* pResElement = NULL;
		if(pWindow)
			pResElement = pWindow->getUIPlane()->getElementById<IThumbnailRenderCallback>(mySourceUiElemId.c_str());
		if(pResElement && !myDidCancel && pResImage->isValid())
			pResElement->onThreadedThumbnailRenderFinished(pResImage, pProvider, this->myIndex, this->myOptParentId.c_str());

		delete pResImage;
	}

	pProvider->cleanup(this);
	delete this;

}
/*****************************************************************************/
// IThumbnailProvider
/*****************************************************************************/
IThumbnailProvider::~IThumbnailProvider()
{
	// We can't do this here, since if any jobs are still not done,
	// we will come back and try to call an function on ourselves, 
	// but at this point we will already have been partially deleted,
	// and will result in a virtual function call fail.
	_ASSERT(myJobs.size() == 0);
	//cancelAllJobsAndWait();
}
/*****************************************************************************/
void IThumbnailProvider::queueThreadedThumbnail(int iIndex, const SVector2D& svSize, UIElement* pCallbackElem, const char* pcsOptParentId, bool bPutInFront)
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	SCOPED_MUTEX_LOCK(&myMapLock);

	ThumbnailRenderInfo *pNewInfo = this->allocateRenderInfo();
	pNewInfo->mySourceUiElemId = pCallbackElem->getStringProp(PropertyId);
	pNewInfo->myWindowId = pCallbackElem->getParentWindow()->getId();
	pNewInfo->myId = generateId();
	pNewInfo->mySize = svSize;
	pNewInfo->myIndex = iIndex;
	if(pcsOptParentId)
		pNewInfo->myOptParentId = pcsOptParentId;
	if(bPutInFront)
		myQueue.insert(myQueue.begin(), pNewInfo);
	else
		myQueue.push_back(pNewInfo);

#ifdef DEBUG_THUMBNAIL_GEN
	gLog("THUMBNAIL: Queueing %d parent = %s\n", iIndex, pcsOptParentId ? pcsOptParentId : "NULL");
#endif
	
	if(myJobs.size() == 0 && getAllowStartingNextJob())
		startNextJob();
}
/*****************************************************************************/
void IThumbnailProvider::cleanup(ThumbnailRenderInfo* pInfo)
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	SCOPED_MUTEX_LOCK(&myMapLock);

	// Remove from map
	TIdThumbnailRenderInfoMap::iterator mi = myJobs.find(pInfo->myId);
	myJobs.erase(mi);

	onJobFinished();

	if(getAllowStartingNextJob())
		startNextJob();
}
/*****************************************************************************/
void IThumbnailProvider::cancelAllJobs()
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	SCOPED_MUTEX_LOCK(&myMapLock);

	// Clear all queued jobs
	clearAndDeleteContainer(myQueue);

	// Cancel all running jobs
	TIdThumbnailRenderInfoMap::iterator mi;
	for(mi = myJobs.begin(); mi != myJobs.end(); mi++)
		mi->second->myDidCancel = true;

#ifdef DEBUG_THUMBNAIL_GEN
	gLog("THUMBNAIL: Cancel all jobs\n");
#endif

}
/*****************************************************************************/
void IThumbnailProvider::cancelAllJobsAndWait()
{
	cancelAllJobs();

	//  Now, block until our jobs queue is empty.
	while(1)
	{
		{
			//boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
			SCOPED_MUTEX_LOCK(&myMapLock);
			if(myJobs.size() <= 0)
				break;
			else
				this->yield();
				//boost::this_thread::yield();
		}
	}
}
/*****************************************************************************/
void IThumbnailProvider::startNextJob()
{
	if(myQueue.size() == 0)
		return;

	// Pop in FIFO order.
	ThumbnailRenderInfo *pNextInfo = *(myQueue.begin());
	myQueue.erase(myQueue.begin());

	onJobStarted();

	myJobs[pNextInfo->myId] = pNextInfo;
	pNextInfo->start(this);
}
/*****************************************************************************/
void IThumbnailProvider::deleteSelf()
{
	cancelAllJobsAndWait();
	delete this;
}
/*****************************************************************************/
bool IThumbnailProvider::getIsRenderingAnything()
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	SCOPED_MUTEX_LOCK(&myMapLock);
	return myJobs.size() > 0 || myQueue.size() > 0;
}
/*****************************************************************************/
bool IThumbnailProvider::getIsQueued(int iIndex, const char* pcsOptParentId)
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	SCOPED_MUTEX_LOCK(&myMapLock);

	// Idea: set the flag on the item instead? I.e. from whatever does the
	// queueing
	TIdThumbnailRenderInfoList::iterator li;
	for(li = myQueue.begin(); li != myQueue.end(); li++)
	{
		if((*li)->myIndex == iIndex && (!pcsOptParentId || (*li)->myOptParentId == pcsOptParentId))
			return true;
	}

	return false;
}
/*****************************************************************************/
bool IThumbnailProvider::moveToFront(int iIndex, const char* pcsOptParentId)
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	SCOPED_MUTEX_LOCK(&myMapLock);

	ThumbnailRenderInfo* pFoundInfo = NULL;
	TIdThumbnailRenderInfoList::iterator li;
	for(li = myQueue.begin(); li != myQueue.end(); li++)
	{
		if((*li)->myIndex == iIndex && (!pcsOptParentId || (*li)->myOptParentId == pcsOptParentId))
		{
			pFoundInfo = (*li);
			myQueue.erase(li);
			break;
		}
	}

	if(pFoundInfo)
	{
		// Push it to front
		myQueue.insert(myQueue.begin(), pFoundInfo);
	}

	return (pFoundInfo != NULL);
}
/*****************************************************************************/
bool IThumbnailProvider::removeFromQueue(int iIndex, const char* pcsOptParentId)
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	SCOPED_MUTEX_LOCK(&myMapLock);
	
	TIdThumbnailRenderInfoList::iterator li;
	for(li = myQueue.begin(); li != myQueue.end(); li++)
	{
		if((*li)->myIndex == iIndex && (!pcsOptParentId || (*li)->myOptParentId == pcsOptParentId))
		{
			delete (*li);
			myQueue.erase(li);
			return true;
		}
	}

	return false;
}
/*****************************************************************************/
void IThumbnailProvider::removeAllThumbnailsFor(const char* pcsParentId)
{
	//boost::recursive_mutex::scoped_lock scoped_lock(myMapLock);
	SCOPED_MUTEX_LOCK(&myMapLock);

	TIdThumbnailRenderInfoList::iterator li;
	TIdThumbnailRenderInfoList::iterator li_next;
	for(li = myQueue.begin(); li != myQueue.end(); )
	{
		li_next = li;
		li_next++;
		if((*li)->myOptParentId == pcsParentId)
		{
			delete (*li);
			myQueue.erase(li);
		}
		li = li_next;
	}
}
/*****************************************************************************/
template < class ID_CLASS >
UNIQUEID_TYPE GlobalIdGeneratorInternal<ID_CLASS>::myGeneratorLastId = 0;
/*****************************************************************************
void IThumbnailProvider::nudgeExistingJobs()
{
	boost::recursive_mutex myMapLock;
	if(myJobs.size() == 0 && getAllowStartingNextJob())
		startNextJob();
}
/*****************************************************************************/
};
