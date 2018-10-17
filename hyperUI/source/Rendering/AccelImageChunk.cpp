#include "stdafx.h"

namespace HyperUI
{
inline float divideByPow2(float y, int n)
{
	unsigned int yi = *((unsigned int *)&y);        // get float value as bits
	unsigned int exponent = yi & 0x7f800000;   // extract exponent bits 30..23
	exponent -= (n << 23);                 // subtract n from exponent
	yi = yi & ~0x7f800000 | exponent;      // insert modified exponent back into bits 30..23
	y = *(float *)&yi;                     // copy bits back to float
	return y;
}

//#define DEBUG_ACCEL_IMAGE

/*****************************************************************************/
AccelImageChunk::AccelImageChunk(int iChunkSizeX, int iChunkSizeY)
{
    myChunkSizeX = iChunkSizeX;
    myChunkSizeY = iChunkSizeY;
    myTextureId = NULL;
#ifdef DIRECTX_PIPELINE
	myActualTexturePtr = NULL;
#endif
	myDidUpdateLast = false;
	myOwnTempStorage = NULL;
}
/*****************************************************************************/
AccelImageChunk::~AccelImageChunk()
{
	clear();
}
/*****************************************************************************/
void AccelImageChunk::render(IWindowBase* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fOpacity) const
{
    SUVSet uvs;

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImageChunk::render begin myTextureId = %d myChunkSizeX = %d myChunkSizeY = %d fScale = (%g, %g)", myTextureId, myChunkSizeX, myChunkSizeY, fScaleX, fScaleY);
#endif
    GraphicsUtils::drawImage(pWindow, myTextureId, fX, fY, myChunkSizeX*fScaleX, myChunkSizeY*fScaleY, fOpacity, 0, true, uvs, BlendModeNormal);
#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImageChunk::render done");
#endif
}
/*****************************************************************************/
void AccelImageChunk::clear(bool bDeleteTexture)
{
#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImageChunk::clear() begin bDelTexture = %d myTextureId = %x myOwnTempStorage = %x", bDeleteTexture, myTextureId, myOwnTempStorage);
#endif
	if(myTextureId != NULL)
	{
#ifdef LINUX_SUPERDEBUG
		Logger::log("AccelImageChunk::clear glDeleteTextures begin myTextureId = %d", myTextureId);
#endif

		GraphicsUtils::deleteTexture(myTextureId);
#ifdef LINUX_SUPERDEBUG
		Logger::log("AccelImageChunk::clear glDeleteTextures done");
#endif

	}
    myTextureId = NULL;

#ifdef DIRECTX_PIPELINE
	if(myActualTexturePtr)
		myActualTexturePtr->Release();
	myActualTexturePtr = NULL;
#endif
	
#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImageChunk::clear() before delete temp storage");
#endif
	delete myOwnTempStorage;
	myOwnTempStorage = NULL;

	myDidUpdateLast = false;
#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImageChunk::clear() done");
#endif
}
/*****************************************************************************/
void AccelImageChunk::setFrom(const IBaseImage* pImage, int iStartX, int iStartY, int iW, int iH, TUCharLossyDynamicArray* pStorage, bool bOnlySetData, int iVisibleChannels)
{
    clear(!bOnlySetData);
	setDataInternal(pImage, iStartX, iStartY, iW, iH, false, pStorage, bOnlySetData, iVisibleChannels);
}
/*****************************************************************************/
void AccelImageChunk::updateFrom(const IBaseImage* pImage, int iStartX, int iStartY, int iW, int iH, TUCharLossyDynamicArray* pStorage, bool bOnlySetData, int iVisibleChannels)
{
	setDataInternal(pImage, iStartX, iStartY, iW, iH, true, pStorage, bOnlySetData, iVisibleChannels);
}
/*****************************************************************************/
void AccelImageChunk::finishTextureCreation(bool bUpdate, unsigned char *pRawData)
{
	if(!pRawData && myOwnTempStorage)
		pRawData = myOwnTempStorage->getArray();
	else 
	{ _ASSERT(pRawData); }

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImageChunk::finishTextureCreation begin pRawData = %x myOwnTempStorage = %x bUpdate = %d myTextureId = %d myChunkSizeX = %d myChunkSizeY = %d", pRawData, myOwnTempStorage ? myOwnTempStorage->getArray() : NULL, bUpdate, myTextureId, myChunkSizeX, myChunkSizeY);
#endif

#ifdef DIRECTX_PIPELINE
	//if(bUpdate && myTextureId != NULL)
	{
		// Update existing texture with new data of the same size
	}
	//else
	{
		// Create a new texture
		if(myTextureId != NULL)
		{
			GraphicsUtils::deleteTexture(myTextureId);
#ifdef LINUX_SUPERDEBUG
			Logger::log("glDeleteTextures done");
#endif
		}

		if(myActualTexturePtr)
			myActualTexturePtr->Release();
		myActualTexturePtr = NULL;

		myActualTexturePtr = GraphicsUtils::createTextureFrom(myChunkSizeX, myChunkSizeY, pRawData, false);
		myTextureId = &myActualTexturePtr;

	}
#else


	if(bUpdate && myTextureId != NULL)
	{
		_ASSERT(myTextureId != NULL);
		GraphicsUtils::setDiffuseTexture(myTextureId);
#ifdef LINUX_SUPERDEBUG
		Logger::log("glBindTexture done");
#endif
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, myChunkSizeX, myChunkSizeY, GL_RGBA, GL_UNSIGNED_BYTE, pRawData);
#ifdef LINUX_SUPERDEBUG
		Logger::log("glTexSubImage2Ddone");
#endif
	}
	else
	{
		if(myTextureId != NULL)
		{
			GraphicsUtils::deleteTexture(myTextureId);
#ifdef LINUX_SUPERDEBUG
			Logger::log("glDeleteTextures done");
#endif
		}

		myTextureId = GraphicsUtils::createTextureFrom(myChunkSizeX, myChunkSizeY, pRawData, false);
		/*
		myTextureId = NULL;

		glGenTextures(1, &myTextureId);
#ifdef LINUX_SUPERDEBUG
		Logger::log("glGenTextures done myTextureId = %d", myTextureId);
#endif

		gSetDiffuseTexture<TX_MAN_RETURN_TYPE>(myTextureId);

#ifdef LINUX_SUPERDEBUG
		Logger::log("glBindTexture done");
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#ifdef LINUX_SUPERDEBUG
		Logger::log("glTexParameteri 1 done");
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef LINUX_SUPERDEBUG
		Logger::log("glTexParameteri 2 done");
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, myChunkSizeX, myChunkSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, pRawData);
#ifdef LINUX_SUPERDEBUG
		Logger::log("glTexImage2D 1 done");
#endif
*/
	}
#endif

#ifdef LINUX_SUPERDEBUG
	Logger::log("AccelImageChunk::finishTextureCreation done");
#endif

}
/*****************************************************************************/
void AccelImageChunk::getColorAt(int iX, int iY, SColor& scolOut)
{
	if(iX < 0 || iX >= myChunkSizeX || iY < 0 || iY >= myChunkSizeY)
	{
		scolOut.set(0,0,0,0);
		return;
	}

	int iOffset = iY*myChunkSizeX + iX;
	scolOut.r = (FLOAT_TYPE)myOwnTempStorage->getArray()[iOffset]/255.0;
	scolOut.g = (FLOAT_TYPE)myOwnTempStorage->getArray()[iOffset + 1]/255.0;
	scolOut.b = (FLOAT_TYPE)myOwnTempStorage->getArray()[iOffset + 2]/255.0;
	scolOut.alpha = (FLOAT_TYPE)myOwnTempStorage->getArray()[iOffset + 3]/255.0;
}
/*****************************************************************************/
};