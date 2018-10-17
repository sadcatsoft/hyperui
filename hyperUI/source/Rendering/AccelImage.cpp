#include "stdafx.h"

#ifdef _DEBUG
//#define DEBUG_CHUNKS
#endif

namespace HyperUI
{
int AccelImage::theMaxChunkSize = 0;
/*****************************************************************************/
AccelImage::AccelImage()
{
	myIsUnfinished = false;
	myIsDoEraseChunks = false;
    myNumRows = myNumColumns = 0;
	//myX = myY = 0;
    myW = myH = 0;
	mySamplingMode = TextureSamplingLastPlaceholder;
}
/*****************************************************************************/
AccelImage::~AccelImage()
{
	clear();
#if defined(DEBUG_CHUNKS) || defined(LINUX_SUPERDEBUG)
	if(myChunksToDelete.size() > 0)
		Logger::log("AccelImage: clearing delayed chunks destructor\n");
#endif
	clearAndDeleteContainer(myChunksToDelete);
}
/*****************************************************************************/
void AccelImage::clear(bool bClearChunks)
{
#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::clear begin bClearChunks = %d", bClearChunks);
#endif
	//SCOPED_LOCK(myImageLock);
	if(bClearChunks)
		deleteChunks();
	else
	{
		// Move the chunks to the "to-delete-later" list.
		int iCurrChunk, iNumChunks = myChunks.size();
		for(iCurrChunk = 0; iCurrChunk < iNumChunks; iCurrChunk++)
			myChunksToDelete.push_back(myChunks[iCurrChunk]);
		myChunks.clear();
	}

    myNumRows = myNumColumns = 0;
	//myX = myY = 0;
	myW = myH = 0;
	mySamplingMode = TextureSamplingLastPlaceholder;
	myIsUnfinished = false;

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::clear done");
#endif
}
/*****************************************************************************/
void AccelImage::getChunkSizesFor(int iW, int iH, int& iChunkSizeXOut, int& iChunkSizeYOut)
{
    if(theMaxChunkSize == 0)
    {
		GLint texSize = GraphicsUtils::getMaxTextureSize();

		// On some devices, we may return 0. Especially on Linux.
		// In this case, we *must* set the value, since otherwise
		// we would attempt to get it when baking an accel image in
		// from a different (cooking) thread, which has no OpenGL context,
		// and so crash.
		// 2048 because apparently less than 1% has a smaller max dim.
		// Note that in this case we *2 because we want the actual size
		// to be 2048, not half that.
		if(texSize == 0)
		{
			texSize = 2048;
#ifndef WIN32
			texSize *= 2;
#endif
		}
		theMaxChunkSize = texSize;
        
        // On the Mac, this seems to lie to us, and actually fail
        // to create textures of the max size. So we use half...
#ifndef WIN32
        theMaxChunkSize /= 2;
#endif
		gLog("Max texture size: %d\n", texSize);
#ifdef LINUX_SUPERDEBUG
		Logger::log("Max texture size: %d", texSize);
#endif
    }

    // Now that we know the maximum chunk size, see if there's a smaller
    // one to match this better. Modern cards can have insane max texture
    // sizes, and we don't want to hog that much for smaller images.
    iChunkSizeXOut = theMaxChunkSize;
    iChunkSizeYOut = theMaxChunkSize;
    if(iW < theMaxChunkSize)
    {
		iW = forcePowerOfTwo(iW);
		if(iW < theMaxChunkSize)
			iChunkSizeXOut = iW;
    }
    if(iH < theMaxChunkSize)
    {
		iH = forcePowerOfTwo(iH);
		if(iH < theMaxChunkSize)
			iChunkSizeYOut = iH;
    }

	if(iChunkSizeXOut <= 0)
		iChunkSizeXOut = 32;

	if(iChunkSizeYOut <= 0)
		iChunkSizeYOut = 32;
}
/*****************************************************************************/
void AccelImage::render(IWindowBase* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fOpacity) const
{
    if(myW == 0 || myH == 0)
		return;

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::render begin");
#endif


	//SCOPED_LOCK(myImageLock);

	// If we're unfinished, finish initializing us:
    int iCurrX, iCurrY;
	const_cast<AccelImage*>(this)->ensureFinished();

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::render ensureFinished() done");
#endif


	FLOAT_TYPE fScaledChunkSizeX = myChunkSizeX*fScaleX;
    FLOAT_TYPE fScaledChunkSizeY = myChunkSizeY*fScaleY;

    SVector2D svImageSize(myNumColumns*fScaledChunkSizeX, myNumRows*fScaledChunkSizeY);
    // Offset to compensate for chunks that may hold less than the actual image size.
    SVector2D svChunkCompOffset( (svImageSize.x - myW*fScaleX)/2.0, (svImageSize.y - myH*fScaleY)/2.0);

	SVector2D svFinalChunkCenterPos;
    SVector2D svPixelOffset;
    for(iCurrY = 0; iCurrY < myNumRows; iCurrY++)
    {
		for(iCurrX = 0; iCurrX < myNumColumns; iCurrX++)
		{
			svPixelOffset.x = iCurrX*fScaledChunkSizeX + fScaledChunkSizeX/2.0 - svImageSize.x/2.0 + svChunkCompOffset.x;
			svPixelOffset.y = iCurrY*fScaledChunkSizeY + fScaledChunkSizeY/2.0 - svImageSize.y/2.0 + svChunkCompOffset.y;
			svFinalChunkCenterPos.x = fX + svPixelOffset.x;
			svFinalChunkCenterPos.y = fY + svPixelOffset.y;
//svFinalChunkCenterPos.roundToInt();
///TRACE_RESULT_PRINT("UserImageWindow::renderAccelImage: center = (%g, %g)\n", svFinalChunkCenterPos.x, svFinalChunkCenterPos.y);
			myChunks[getOffset(iCurrX, iCurrY)]->render(pWindow, svFinalChunkCenterPos.x, svFinalChunkCenterPos.y, fScaleX, fScaleY, fOpacity);
		}
    }

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::render done");
#endif
}
/*****************************************************************************/
void AccelImage::setFromImageInternal(const IBaseImage* pImage, const SRect2D* pOptRect, bool bOnlySetData, int iVisibleChannels)
{
#ifdef DEFER_NODE_COOKING
	bool bIsDeferring = Node::getIsDoingDeferredCookingPass();
	_ASSERT(bIsDeferring);
#endif

    clear(!bOnlySetData);

	myIsUnfinished = bOnlySetData;

	if(!pImage)
		return;

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::setFromImageInternal() begin with pImage = %x bOnlySetData = %d", pImage, bOnlySetData);
#endif

	int iW = pImage->getWidth();
	int iH = pImage->getHeight();

	int iStartXOffset = 0;
	int iStartYOffset = 0;
	if(pOptRect)
	{
		iStartXOffset = pOptRect->x;
		iStartYOffset = pOptRect->y;
		iW = pOptRect->w;
		iH = pOptRect->h;
	}

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::setFromImageInternal() before chunk size: imgSize = (%d, %d) startOffset = (%d, %d)", iW, iH, iStartXOffset, iStartYOffset);
#endif

    getChunkSizesFor(iW, iH, myChunkSizeX, myChunkSizeY);

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::setFromImageInternal() chunk size done: chunkSize = (%d, %d)",myChunkSizeX, myChunkSizeY);
#endif

    myNumColumns = iW/myChunkSizeX;
	if(myNumColumns*myChunkSizeX < iW)
		myNumColumns++;
    if(myNumColumns <= 0)
		myNumColumns = 1;

    myNumRows = iH/myChunkSizeY;
	if(myNumRows*myChunkSizeY < iH)
		myNumRows++;
    if(myNumRows <= 0)
		myNumRows = 1;

// 	myX = iStartXOffset;
// 	myY = iStartYOffset;

    myW = iW;
    myH = iH;

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::setFromImageInternal() before resize numChunks = (%d, %d)", myNumColumns, myNumRows);
#endif

    myChunks.resize(myNumRows*myNumColumns);

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::setFromImageInternal() resize done");
#endif
    AccelImageChunk *pChunk;
    int iX = 0, iY = 0;
    int iChunk, iNumChunks = myChunks.size();
    for(iChunk = 0; iChunk < iNumChunks; iChunk++)
    {
		pChunk = this->allocateChunk(myChunkSizeX, myChunkSizeY);
#ifdef LINUX_SUPERDEBUG
		Logger::log("AccelImage::setFromImageInternal() before setFrom = %d", iChunk);
#endif
		pChunk->setFrom(pImage, iX*myChunkSizeX + iStartXOffset, iY*myChunkSizeY + iStartYOffset, myChunkSizeX, myChunkSizeY, &myStorage, bOnlySetData, iVisibleChannels);
#ifdef LINUX_SUPERDEBUG
		Logger::log("AccelImage::setFromImageInternal() after setFrom");
#endif
		myChunks[iChunk] = pChunk;

		iX++;
		if(iX >= myNumColumns)
		{
			iX = 0;
			iY++;
		}
    }

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::setFromImageInternal() all done");
#endif
}
/*****************************************************************************/
void AccelImage::setSamplingMode(TextureSamplingType eType)
{
	//SCOPED_LOCK(myImageLock);
	ensureFinished();

	if(mySamplingMode == eType)
		return;

	mySamplingMode = eType;

	int iCurrX, iCurrY;
	for(iCurrY = 0; iCurrY < myNumRows; iCurrY++)
	{
		for(iCurrX = 0; iCurrX < myNumColumns; iCurrX++)
			GraphicsUtils::setTextureSamplingMode(myChunks[getOffset(iCurrX, iCurrY)]->getTextureId(), eType);
	}
}
/*****************************************************************************/
void AccelImage::updateFrom(const IBaseImage* pImage, const SRect2D* pOptRect, bool bOnlySetData, int iVisibleChannels)
{
	//SCOPED_LOCK(myImageLock);
	if(!pImage)
	{
		if(!bOnlySetData)
			clear();
		myIsUnfinished = bOnlySetData;
		myIsDoEraseChunks = true;
		return;
	}

	myIsDoEraseChunks = false;
	myIsUnfinished = bOnlySetData;

	int iW = pImage->getWidth();
	int iH = pImage->getHeight();

#ifdef _DEBUG
/*
	if(pOptRect)
	{ TRACE_RESULT_PRINT("AccelImage::updateFrom: imgSize = (%d, %d) optRect = (%g, %g, %g, %g)\n", iW, iH, pOptRect->x, pOptRect->y, pOptRect->w, pOptRect->h); } 
	else
	{ TRACE_RESULT_PRINT("AccelImage::updateFrom: imgSize = (%d, %d) optRect = null\n", iW, iH); }
	static int iSaveCntr = 0;
	iSaveCntr++;
	char pcsBuff[128];
	sprintf(pcsBuff, "db_accelupdate%d.png", iSaveCntr);
	//SAVE_IMAGE_COPY(*pImage, pcsBuff);*/
#endif

	int iStartXOffset = 0;
	int iStartYOffset = 0;
	if(pOptRect)
	{
		iStartXOffset = pOptRect->x;
		iStartYOffset = pOptRect->y;
		iW = pOptRect->w;
		iH = pOptRect->h;
	}

	if(myW == iW && myH == iH && myW > 0 && myH > 0) //  && myX == iStartXOffset && myY == iStartYOffset)
	{
#ifdef LINUX_SUPERDEBUG
		Logger::log("AccelImage::updateFrom: Updating exising image bOnlySetData = %s\n", bOnlySetData ? TRUE_VALUE : FALSE_VALUE);
#endif
		mySamplingMode = TextureSamplingLastPlaceholder;

		// We can update
		int iX = 0, iY = 0;
		int iChunk, iNumChunks = myChunks.size();
		for(iChunk = 0; iChunk < iNumChunks; iChunk++)
		{
			myChunks[iChunk]->updateFrom(pImage, iX*myChunkSizeX + iStartXOffset, iY*myChunkSizeY + iStartYOffset, myChunkSizeX, myChunkSizeY, &myStorage, bOnlySetData, iVisibleChannels);
			iX++;
			if(iX >= myNumColumns)
			{
				iX = 0;
				iY++;
			}
		}
	}
	else
	{
#ifdef LINUX_SUPERDEBUG
		Logger::log("AccelImage::updateFrom: setting new image bOnlySetData = %s\n", bOnlySetData ? TRUE_VALUE : FALSE_VALUE);
#endif
		// We need a new image
		setFromImageInternal(pImage, pOptRect, bOnlySetData, iVisibleChannels);
	}
}
/*****************************************************************************/
void AccelImage::ensureFinished()
{
	if(!myIsUnfinished)
		return; 

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::ensureFinished begin");
#endif

	if(myIsDoEraseChunks)
	{
		clear();
		myIsUnfinished = false;
		return;
	}
#if defined(DEBUG_CHUNKS) || defined(LINUX_SUPERDEBUG)
	if(myChunksToDelete.size() > 0)
		Logger::log("AccelImage: clearing delayed chunks ensureFinished\n");
#endif
	clearAndDeleteContainer(myChunksToDelete);
#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::ensureFinished clearAndDeleteContainer done");
#endif


	myIsUnfinished = false;

	int iCurrX, iCurrY;
	AccelImageChunk* pChunk = NULL;
	for(iCurrY = 0; iCurrY < myNumRows; iCurrY++)
	{
		for(iCurrX = 0; iCurrX < myNumColumns; iCurrX++)
		{
			pChunk = myChunks[getOffset(iCurrX, iCurrY)];
			pChunk->finishTextureCreation(pChunk->getDidUpdateLast(), NULL);
		}
	}

	if(pChunk && !pChunk->getDidUpdateLast())
		mySamplingMode = TextureSamplingLastPlaceholder;

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImage::ensureFinished done");
#endif
}
/*****************************************************************************/
void AccelImage::deleteChunks()
{
#if defined(DEBUG_CHUNKS) || defined(LINUX_SUPERDEBUG)
	if(myChunks.size() > 0)
		Logger::log("AccelImage: deleting chunks\n");
#endif
/*
	int iCurrChunk, iNumChunks = myChunks.size();
	for(iCurrChunk = 0; iCurrChunk < iNumChunks; iCurrChunk++)
		delete myChunks[iCurrChunk];
	myChunks.clear();
*/
	clearAndDeleteContainer(myChunks);
	myIsDoEraseChunks = false;
	mySamplingMode = TextureSamplingLastPlaceholder;
}
/*****************************************************************************/
void AccelImage::getColorAt(int iX, int iY, SColor& scolOut)
{
	int iChunkX = iX/theMaxChunkSize;
	int iChunkY = iY/theMaxChunkSize;

	scolOut.set(0,0,0,0);
	if(iChunkX < 0 || iChunkX >= myNumColumns || iChunkY < 0 || iChunkY >= myNumRows)
		return;

	// Otherwise, get the chunk
	int iRelX = iX - iChunkX*theMaxChunkSize;
	int iRelY = iY - iChunkY*theMaxChunkSize;
	myChunks[getOffset(iChunkX, iChunkY)]->getColorAt(iRelX, iRelY, scolOut);
}
/*****************************************************************************/
void AccelImage::getBBox(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, SRect2D& srOut)
{
	SVector2D svSize(myW*fScaleX, myH*fScaleY);
	SVector2D svCenter(fCenterX, fCenterY);
	srOut.setFromCenterAndSize(svCenter, svSize);
}
/*****************************************************************************/
};