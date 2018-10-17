#include "stdafx.h"

#ifdef _DEBUG
	#include <iostream>
	#include <fstream>
#endif

namespace HyperUI
{

#ifdef DIRECTX_PIPELINE
#include "D3DRenderer.h"
extern D3DRenderer* g_pDxRenderer;
#endif

/********************************************************************************************/
TextureManager::TextureManager(Window* pParentWindow)
{
	myParentWindow = pParentWindow;
	myIsInitializing = false;
	///memset(myTextures, 0, sizeof(TextureAnimSequence*)*AnimationLastPlaceholder);
}
/********************************************************************************************/
TextureManager::~TextureManager()
{

	TTexturesMap::iterator mi;
	for(mi = myTextures.begin(); mi != myTextures.end(); mi++)
	{

#ifdef _DEBUG
		TextureAnimSequence* pDbSeq = mi->second;
		const char* pcsKey = mi->first.c_str();
		const char* pcsDbName = pDbSeq->getName();
#endif
		mi->second->release();
		//		delete mi->second;
	}
	myTextures.clear();
}
/********************************************************************************************/
void TextureManager::initAnimation(const char* pcsAnimName, const CHAR_TYPE* pcsFileName, bool bIsCompressed, BlendModeType eBlendMode, bool bIsUnloadable, const ResourceItem* pItem, const CHAR_TYPE* pcsOptTargetFolderDataPath)
{
	string str_name(pcsFileName);
	if(str_name == PROPERTY_NONE)
		return;

	int iFirstFrame = pItem ? pItem->getNumProp(PropertyFirstFrame) : 1;
	int iLastFrame = pItem ? pItem->getNumProp(PropertyLastFrame) : 1;
	int iFrameStep = pItem ? pItem->getNumProp(PropertyFrameStep) : 1;
	int iFrameWidth = pItem ? pItem->getNumProp(PropertyFrameWidth) : -1;
	int iFrameHeight = pItem ? pItem->getNumProp(PropertyFrameHeight) : -1;
	bool bResampleNearest = pItem ? pItem->getBoolProp(PropertyResampleNearest) : false;
	int iRenderStage = pItem ? (int)pItem->getNumProp(PropertyZIndex) : 2;

	if(iFirstFrame < 1)
		iFirstFrame = 1;
	if(iLastFrame < 1)
		iLastFrame = 1;
	if(iFrameStep < 1)
		iFrameStep = 1;
	if(iFrameWidth <= 0.0)
		iFrameWidth = -1;
	if(iFrameHeight <= 0.0)
		iFrameHeight = -1;
	if(iRenderStage <= 0)
		iRenderStage = 2;

	// See if the file name already exists, and if it has a texture in it:
	TextureAnimSequence* pSeq = NULL;
	int iAtlasFrame;
	int iKilledPos = StringUtils::findFrameNum(pcsFileName, iAtlasFrame);
	if(iAtlasFrame >= 0)
	{
		// We've got a frame number!
		// See if we've already added the file
		TTexturesMap::iterator mi = myLoadedFiles.find(pcsFileName);
		if(mi != myLoadedFiles.end())
			pSeq = mi->second;
	}

	if(!pSeq)
	{
		pSeq = new TextureAnimSequence(pcsAnimName, pcsFileName, iFirstFrame, iLastFrame, iFrameStep, iFrameWidth, iFrameHeight, bIsCompressed, eBlendMode, bResampleNearest, iRenderStage, bIsUnloadable, pItem, pcsOptTargetFolderDataPath);
		_ASSERT(!myLoadedFiles[pcsFileName] || strcmp(pcsFileName, "twall") == 0 || strcmp(pcsFileName, "dummy32_") == 0);
		myLoadedFiles[pcsFileName] = pSeq;
		int iRealW = pItem ? pItem->getNumProp(PropertyRealContentWidth) : 0;
		int iRealH = pItem ? pItem->getNumProp(PropertyRealContentHeight) : 0;
		pSeq->setRealImageDims(iRealW, iRealH);

		iRealW = pItem ? pItem->getNumProp(PropertyCornerWidth) : 0;
		iRealH = pItem ? pItem->getNumProp(PropertyCornerHeight) : 0;
		pSeq->setCornerSize(iRealW, iRealH);

//		if(pItem->doesPropertyExist(PropertyAnimRealImageWidth))
//			pSeq->setRealImageDims(pItem->getNumProp(PropertyAnimRealImageWidth));
	}

	pSeq->addRef();
	_ASSERT(pcsAnimName && strlen(pcsAnimName) > 0);
	_ASSERT(pSeq);
	myTextures[pcsAnimName] = pSeq;

	if(iKilledPos >= 0)
	{
		const_cast<char*>(pcsFileName)[iKilledPos] = ':';
	}
}
/********************************************************************************************
int TextureManager::getFrameNumberFromProgress(AnimationType eType, FLOAT_TYPE fProgress)
{
TTexturesMap::iterator mi;
mi = myTextures.find(eType);
if(mi == myTextures.end())
return 0;

TextureAnimSequence* pAnimSeq = mi->second;
return pAnimSeq->getFrameNumberFromProgress(fProgress);
}
/********************************************************************************************/
TX_MAN_RETURN_TYPE TextureManager::getTextureIndexFromProgress(const char* pcsAnimName, FLOAT_TYPE fProgress, int& iWidth, int& iHeight, SUVSet& uvsOut, BlendModeType& eBlendMode, TextureAnimSequence *pOptExistingPointer)
{
	/*
	TTexturesMap::iterator mi;
	mi = myTextures.find(eType);
	if(mi == myTextures.end())
	return 0;

	TextureAnimSequence* pAnimSeq = mi->second;
	*/

	TextureAnimSequence* pAnimSeq = pOptExistingPointer;
	if(!pAnimSeq)
		pAnimSeq = myCacher.getCached(pcsAnimName);
	if(!pAnimSeq)
	{
		pAnimSeq = getTexture(pcsAnimName);
		if(pAnimSeq)
			myCacher.addToCache(pcsAnimName, pAnimSeq);
	}

	if(pAnimSeq)
	{
		pAnimSeq->ensureLoaded();
		eBlendMode = pAnimSeq->getBlendMode();
		iWidth = pAnimSeq->getWidth();
		iHeight = pAnimSeq->getHeight();
		return pAnimSeq->getFrameFromProgress(fProgress, uvsOut);
	}
	else
	{
		eBlendMode = BlendModeNormal;
		iWidth = iHeight = 16;
		return 0;
	}
}
/********************************************************************************************/
int TextureManager::getFileHeight(const char* pcsAnimName)
{
	TextureAnimSequence* pAnimSeq = myCacher.getCached(pcsAnimName);
	if(!pAnimSeq)
	{
		pAnimSeq = getTexture(pcsAnimName);
		if(pAnimSeq)
			myCacher.addToCache(pcsAnimName, pAnimSeq);
	}

	if(!pAnimSeq)
	{
		_ASSERT(0);
		return 32;
	}
	else
	{
		pAnimSeq->ensureLoaded();
		return pAnimSeq->getFileHeight();
	}
}
/********************************************************************************************/
int TextureManager::getWidth(const char* pcsAnimName)
{
	TextureAnimSequence* pAnimSeq = myCacher.getCached(pcsAnimName);
	if(!pAnimSeq)
	{
		pAnimSeq = getTexture(pcsAnimName);
		if(pAnimSeq)
			myCacher.addToCache(pcsAnimName, pAnimSeq);
	}

	pAnimSeq->ensureLoaded();
	return pAnimSeq->getWidth();
}
/********************************************************************************************/
int TextureManager::getHeight(const char* pcsAnimName)
{
	TextureAnimSequence* pAnimSeq = myCacher.getCached(pcsAnimName);
	if(!pAnimSeq)
	{
		pAnimSeq = getTexture(pcsAnimName);
		if(pAnimSeq)
			myCacher.addToCache(pcsAnimName, pAnimSeq);
	}

	pAnimSeq->ensureLoaded();
	return pAnimSeq->getHeight();
}
/********************************************************************************************/
int TextureManager::getFileWidth(const char* pcsAnimName)
{
	TextureAnimSequence* pAnimSeq = myCacher.getCached(pcsAnimName);
	if(!pAnimSeq)
	{
		pAnimSeq = getTexture(pcsAnimName);
		if(pAnimSeq)
			myCacher.addToCache(pcsAnimName, pAnimSeq);
	}

	if(!pAnimSeq)
	{
		_ASSERT(0);
		return 32;
	}
	else
	{
		pAnimSeq->ensureLoaded();
		return pAnimSeq->getFileWidth();
	}
}
/********************************************************************************************/
void TextureManager::getCornerSize(const char* pcsAnimName, int &iRealW, int &iRealH)
{
	TextureAnimSequence* pAnimSeq = myCacher.getCached(pcsAnimName);
	if(!pAnimSeq)
	{
		pAnimSeq = getTexture(pcsAnimName);
		if(pAnimSeq)
			myCacher.addToCache(pcsAnimName, pAnimSeq);
	}

	if(pAnimSeq)
		pAnimSeq->getCornerSize(iRealW, iRealH);
	ELSE_ASSERT;

}
/********************************************************************************************/
void TextureManager::getTextureRealDims(const char* pcsAnimName, int &iRealW, int &iRealH) const
{
	const TextureAnimSequence* pAnimSeq = myCacher.getCached(pcsAnimName);
	if(!pAnimSeq)
	{
		pAnimSeq = getTexture(pcsAnimName);
		if(pAnimSeq)
			myCacher.addToCache(pcsAnimName, const_cast<TextureAnimSequence*>(pAnimSeq));
	}

	if(pAnimSeq)
		pAnimSeq->getRealImageDims(iRealW, iRealH);
	ELSE_ASSERT;
}
/********************************************************************************************/
TX_MAN_RETURN_TYPE TextureManager::getTextureIndex(const char* pcsAnimName)
{
	TextureAnimSequence* pAnimSeq = myCacher.getCached(pcsAnimName);
	if(!pAnimSeq)
	{
		pAnimSeq = getTexture(pcsAnimName);
		if(pAnimSeq)
			myCacher.addToCache(pcsAnimName, pAnimSeq);
	}

	if(pAnimSeq)
	{
		if(!pAnimSeq->getIsLoaded())
			pAnimSeq->ensureLoaded();
		SUVSet svDummyUVs;
		return pAnimSeq->getFrame(0, svDummyUVs);
	}
	else
		return 0;
}
/********************************************************************************************/
TX_MAN_RETURN_TYPE TextureManager::getTextureIndex(const char* pcsAnimName, int iFrame, int& iWidth, int& iHeight, SUVSet& uvsOut, BlendModeType& eBlendMode, TextureAnimSequence *pOptExistingPointer)
{
	TextureAnimSequence* pAnimSeq = pOptExistingPointer;
	if(!pAnimSeq)
		pAnimSeq = myCacher.getCached(pcsAnimName);
	if(!pAnimSeq)
	{
		pAnimSeq = getTexture(pcsAnimName);
		if(pAnimSeq)
			myCacher.addToCache(pcsAnimName, pAnimSeq);
	}

	if(pAnimSeq)
	{
		pAnimSeq->ensureLoaded();
		eBlendMode = pAnimSeq->getBlendMode();
		iWidth = pAnimSeq->getWidth();
		iHeight = pAnimSeq->getHeight();
		return pAnimSeq->getFrame(iFrame, uvsOut);
	}
	else
	{
		eBlendMode = BlendModeNormal;
		iWidth = iHeight = 16;
		return 0;
	}

}
/********************************************************************************************/
void TextureManager::getTextureNameFromProgress(const char* pcsAnimName, FLOAT_TYPE fProgress, string& strOut)
{
	/*
	TTexturesMap::iterator mi;
	mi = myTextures.find(eType);
	if(mi == myTextures.end())
	{
	strOut = "";
	return;
	}

	TextureAnimSequence* pAnimSeq = mi->second; */

	TextureAnimSequence* pAnimSeq = getTexture(pcsAnimName);
	if(pAnimSeq)
		pAnimSeq->getNameFromProgress(fProgress, strOut);
	else
		strOut = "";
}
/********************************************************************************************/
void TextureManager::ensureLoaded(const char* pcsAnimName)
{
	TextureAnimSequence* pAnimSeq = getTexture(pcsAnimName);
	if(pAnimSeq)
	{
		pAnimSeq->ensureLoaded();
		pAnimSeq->markUsed();
	}
}
/********************************************************************************************/
TextureAnimSequence* TextureManager::getTexture(const char* pcsAnimName)
{
	TTexturesMap::iterator mi = myTextures.find(pcsAnimName);
	if(mi == myTextures.end())
		return NULL;
	else
		return mi->second;
}
/********************************************************************************************/
const TextureAnimSequence* TextureManager::getTexture(const char* pcsAnimName) const
{
	TTexturesMap::const_iterator mi = myTextures.find(pcsAnimName);
	if(mi == myTextures.end())
		return NULL;
	else
		return mi->second;
}
/********************************************************************************************/
bool TextureManager::isTextureAnAtlas(const char* pcsAnimName)
{
	TextureAnimSequence* pAnimSeq = getTexture(pcsAnimName);
	if(pAnimSeq)
		return pAnimSeq->isTextureAnAtlas();
	else
		return false;
}
/********************************************************************************************/
FLOAT_TYPE* TextureManager::getSubtileMap(const char* pcsAnimName, int& iMapWidth, int& iMapHeight)
{
	TTexturesMap::iterator mi = myTextures.find(pcsAnimName);
	if(mi == myTextures.end())
		return NULL;
	else
		return mi->second->getSubtileMap(iMapWidth, iMapHeight);
}
/********************************************************************************************/
bool TextureManager::getIsTextureUnloadable(const char* pcsAnimName)
{
	TTexturesMap::iterator mi = myTextures.find(pcsAnimName);
	if(mi == myTextures.end())
		return false;
	else
		return mi->second->getIsTextureUnloadable();
}
/********************************************************************************************/
bool TextureManager::getForceAlphaTest(const char* pcsAnimName)
{
	TTexturesMap::iterator mi = myTextures.find(pcsAnimName);
	if(mi == myTextures.end())
		return 1;
	else
		return mi->second->getForceAlphaTest();
}
/********************************************************************************************/
bool TextureManager::getDisableAlphaTest(const char* pcsAnimName)
{
	TTexturesMap::iterator mi = myTextures.find(pcsAnimName);
	if(mi == myTextures.end())
		return 1;
	else
		return mi->second->getDisableAlphaTest();
}
/********************************************************************************************/
int TextureManager::getTextureRenderStage(const char* pcsAnimName)
{
	TTexturesMap::iterator mi = myTextures.find(pcsAnimName);
	if(mi == myTextures.end())
		return 1;
	else
		return mi->second->getRenderStage();
}
/********************************************************************************************/
void TextureManager::setTextureBlendMode(const char* pcsAnimName, BlendModeType eMode)
{
	TextureAnimSequence* pAnimSeq = getTexture(pcsAnimName);
	if(pAnimSeq)
		pAnimSeq->setBlendMode(eMode);
}
/********************************************************************************************/
void TextureManager::getTextureName(const char* pcsAnimName, int iFrame, string& strOut)
{
	/*
	TTexturesMap::iterator mi;
	mi = myTextures.find(eType);
	if(mi == myTextures.end())
	{
	strOut = "";
	return;
	}

	TextureAnimSequence* pAnimSeq = mi->second; */

	TextureAnimSequence* pAnimSeq = getTexture(pcsAnimName);
	if(pAnimSeq)
		pAnimSeq->getName(iFrame, strOut);
	else
		strOut = "";

	//return myTextures[eType]->getName(iFrame, strOut);
	//return myTextures[pcsAnimName]->getName(iFrame, strOut);
}
/********************************************************************************************/
void TextureManager::initFromCollection(ResourceCollection* pCollection, const CHAR_TYPE* pcsOptTargetFolderDataPath, IProgressCallbacks* pCallbacks)
{
	Application::lockGlobal();
	myIsInitializing = true;

	BlendModeType eBlendMode;
	string strType, strFormat, strBlendType;
	//ResourceItem* pCurrItem = pCollection->traverseBegin();
	ResourceItem* pCurrItem = NULL;
	ResourceCollection::Iterator ci;
	Application::unlockGlobal();

	bool bIsPVR;

	//while(pCurrItem)
	for(ci = pCollection->itemsBegin(); !ci.isEnd(); ci++)
	{
		Application::lockGlobal();
		pCurrItem = ci.getItem();
		// If there is no anim file name, skip it. We must be doing the room items.
		// Or we forgot, but oh well...
		if(pCurrItem->doesPropertyExist(PropertyFile))
		{
			strFormat = pCurrItem->getStringProp(PropertyFormat);
			if(!IS_VALID_STRING_AND_NOT_NONE(strFormat.c_str()))
				strFormat = "png";

			bIsPVR = (strFormat != "png");

#if !defined(WIN32) && !defined(LINUX)
			// If the texture is compressed, it came frmo PVRText, which uses straight alpha, no premult one.
			// This means we have to set the text blending mode.
			if(bIsPVR)
				pCurrItem->setStringProp(PropertyBlendingMode, g_pcsBlendModes[BlendModeText]);
#endif

			strBlendType = pCurrItem->getStringProp(PropertyBlendingMode);
			//eBlendMode = mapStringToBlendMode(strBlendType);
			eBlendMode = mapStringToType<BlendModeType>(strBlendType.c_str(), g_pcsBlendModes, BlendModeLastPlaceholder);

			pCurrItem->getAsString(PropertyId, strType);
			this->initAnimation(strType.c_str(), pCurrItem->getStringProp(PropertyFile), bIsPVR, eBlendMode, pCurrItem->getBoolProp(PropertyIsUnloadable), pCurrItem, pcsOptTargetFolderDataPath);

			// Now, check if our skin manager has more, and load those:
			SkinManager::getInstance()->onTextureInit(this, strType.c_str(), pCurrItem, bIsPVR, eBlendMode);
		}

		if(pCallbacks)
			pCallbacks->onItemCompleted(pCollection);

		//pCurrItem = pCollection->traverseNext();
		Application::unlockGlobal();
	}

	myIsInitializing = false;
}
/********************************************************************************************/
#ifdef _DEBUG
void TextureManager::dbCheckConsistency()
{
	TTexturesMap rTestMap;
	int iSize1 = rTestMap.size();
	TextureAnimSequence*pTest = rTestMap[""];
	int iSize2 = rTestMap.size();

	TTexturesMap::iterator mi;
	for(mi = myTextures.begin(); mi != myTextures.end(); mi++)
	{
		TextureAnimSequence* pDbSeq = mi->second;
		const char* pcsKey = mi->first.c_str();
		_ASSERT(pcsKey && strlen(pcsKey) > 0);
		_ASSERT(pDbSeq);
	}
}
#endif
/********************************************************************************************/
void TextureManager::reloadAll()
{
	TTexturesMap::iterator mi;
	for(mi = myTextures.begin(); mi != myTextures.end(); mi++)
	{
		if(mi->second->getIsLoaded())
		{
			mi->second->unloadTexture();
			mi->second->ensureLoaded();
		}
	}
}
/********************************************************************************************/
void TextureManager::loadFromFile(const char* pcsAnimName, const char* pcsFileName)
{
	TextureAnimSequence* pAnimSeq = getTexture(pcsAnimName);
	if(!pAnimSeq)
		initAnimation(pcsAnimName, pcsFileName, false, BlendModeNormal, true, NULL, NULL);
	reload(pcsAnimName);
}
/*****************************************************************************/
void TextureManager::reload(const char* pcsAnimName)
{
	TextureAnimSequence* pAnimSeq = getTexture(pcsAnimName);
	if(pAnimSeq)
	{
		if(pAnimSeq->getIsLoaded())
		{
			getParentWindow()->getDrawingCache()->unloadTexture(pcsAnimName);
			pAnimSeq->unloadTexture();
		}
		pAnimSeq->ensureLoaded();
		pAnimSeq->markUsed();
	}
}
/*****************************************************************************/
void TextureManager::unloadAll()
{
	TTexturesMap::iterator mi;
	for(mi = myTextures.begin(); mi != myTextures.end(); mi++)
	{
		if(mi->second->shouldDoEmergencyUnload())
			mi->second->unloadTexture();
	}
}
/********************************************************************************************/
};
