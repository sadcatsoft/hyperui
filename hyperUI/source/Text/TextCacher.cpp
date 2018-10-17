#include "stdafx.h"

#include <math.h>


#define CLEAN_TICKS_INTERVAL				60
#define UNUSED_TEXTURE_DELETION_TIME			((int)(4.0*GAME_FRAMERATE)) // 15
#define UNUSED_LARGE_TEXTURE_DELETION_TIME		((int)(7.0*GAME_FRAMERATE)) // 8

#ifdef DIRECTX_PIPELINE
#include "D3DRenderer.h"
extern D3DRenderer* g_pDxRenderer;
#endif

// DEBUG
//#define CLEAN_TICKS_INTERVAL				2
//#define UNUSED_TEXTURE_DELETION_TIME		((int)(0.25*GAME_FRAMERATE))

//#ifndef MAC_BUILD
//string TextCacher::theSharedString;
//string TextCacher::theSharedString2;
//#endif
namespace HyperUI
{
/*****************************************************************************/
TextCacher::TextCacher(Window* pWindow)
: IOffscreenCacher(pWindow), myTextParmsMemoryCacher(64)
{
	myMemoryUsed = 0;

}
/*****************************************************************************/
TextCacher::~TextCacher()
{
	cleanAll();
}
/*****************************************************************************/
void TextCacher::checkForCleaning()
{	
	GTIME lTime = Application::getInstance()->getGlobalTime(ClockUniversal);
	if(lTime % CLEAN_TICKS_INTERVAL == 0)
	{
		cleanUnused();
	}
}
/*****************************************************************************
void TextCacher::markAllUnused(void)
{
	TTextParmsMap::iterator curr;
	for(curr = myEntries.begin(); curr != myEntries.end(); curr++)
		curr->second->myUsed = false;
}
/*****************************************************************************/
void TextCacher::cleanAll()
{
	TTextParmsMap::iterator curr;
	for(curr = myEntries.begin(); curr != myEntries.end(); curr++)
	{
#ifdef DIRECTX_PIPELINE
		curr->second->myTextureIndex = NULL;
		if(curr->second->myActualTexturePtr)
			curr->second->myActualTexturePtr->Release();
		curr->second->myActualTexturePtr = NULL;
#else
		glDeleteTextures(1, &curr->second->myTextureIndex);
#endif
		myTextParmsMemoryCacher.deleteObject(curr->second);
	}
	myEntries.clear();
	myMemoryUsed = 0;

	myCacher.clearAll();
}
/*****************************************************************************/
void TextCacher::cleanUnused()
{
	GTIME lTime = Application::getInstance()->getGlobalTime(ClockUniversal);
	TTextParmsMap::iterator curr;
	for(curr = myEntries.end(); curr != myEntries.begin(); curr--)
	{
		if(curr == myEntries.end())
			continue;

		// if(curr->second.myUsed == false)
		if(lTime - curr->second->myLastUsedTime > UNUSED_TEXTURE_DELETION_TIME ||
		   // Dispose of large textures faster
		   (max(curr->second->myBitmapsDims.x, curr->second->myBitmapsDims.y) > 255 &&
		   lTime - curr->second->myLastUsedTime > UNUSED_LARGE_TEXTURE_DELETION_TIME)
		   )
		{
			myCacher.clear(curr->first.c_str());
#ifdef DIRECTX_PIPELINE
			curr->second->myTextureIndex = NULL;
			if(curr->second->myActualTexturePtr)
				curr->second->myActualTexturePtr->Release();
			curr->second->myActualTexturePtr = NULL;
#else
			glDeleteTextures(1, &curr->second->myTextureIndex);
#endif
			myTextParmsMemoryCacher.deleteObject(curr->second);
			myMemoryUsed -= curr->second->getBytesUsed();

			// Remove this texture
			myEntries.erase(curr);

			// Start over, since the tree may have changed.
			curr = myEntries.end();
		}
	}
}
/*****************************************************************************/
TX_MAN_RETURN_TYPE TextCacher::getTextureFor(string& strText, STextParams& rParms, SVector2D& svTextDimsOut,
								 SVector2D& svBitmapDimsOut, bool bCreate)
{
	STextParams* pResParms = NULL;
	pResParms = myCacher.getCached(strText.c_str());
	if(pResParms && !pResParms->isEqualTo(rParms))
		pResParms = NULL;

	if(!pResParms)
	{
		// Look for the texture in our existing map
		TTextParmsMap::iterator first, last, curr_it;
		///theSharedString = pcsText;
		first = myEntries.lower_bound(strText);
		last = myEntries.upper_bound(strText);

		if(first != myEntries.end()) //  && last != myEntries.end())
		{
			for(curr_it = first; curr_it != last; curr_it++)
			{
				if(curr_it->second->isEqualTo(rParms))
					pResParms = curr_it->second;
			}
		}
	}

	if(!pResParms)
	{
		// No such entry. Make one.
		if(bCreate)
		{
			theSharedString2 = strText;
			pResParms = this->addTextureFor(theSharedString2.c_str(), rParms);
		}
		else
			return 0;
	}

	if(!pResParms)
	{
		_ASSERT(0);
		return 0;
	}

	//pResParms->myUsed = true;
	pResParms->myLastUsedTime = Application::getInstance()->getGlobalTime(ClockUniversal);
	svTextDimsOut = pResParms->myTextDims;
	svBitmapDimsOut = pResParms->myBitmapsDims;
	return pResParms->myTextureIndex;

}
/*****************************************************************************/
STextParams* TextCacher::addTextureFor(const char* pcsText, STextParams& rParms)
{
	// We need this since we will insert newlines for the text (possibly)
	// in measuring it.
	theSharedString = pcsText;

	SVector2D svDummy1, svDummy2;

	if(getTextureFor(theSharedString, rParms, svDummy1, svDummy2, false) > 0)
	{
		// We already have this texture. This might happen if the same frame queues up
		// two identical pieces of text at the same time. To avoid one of them getting
		// "lost", we check for existence.
		return NULL;
	}

	STextParams *rPushParms = myTextParmsMemoryCacher.getNewObject();
	// MUST preserve ids:
	long lOldMemId = rPushParms->getMemoryBlockId();
	*rPushParms = rParms;
	rPushParms->setMemoryBlockId(lOldMemId);

	SVector2D svTextSize;
#ifdef _DEBUG
	if(IS_STRING_EQUAL(pcsText, "Save"))
	{
		int bp = 0;
	}
#endif

	SCachedTextObject rCachedParms;
	rCachedParms.myCenter.set(0, 0);
	rCachedParms.myBaseInfo = rParms;
	rCachedParms.myHorAlign = HorAlignLeft;
	rCachedParms.myVertAlign = VertAlignTop;
	_ASSERT(rCachedParms.myScale == 1.0);
	RenderUtils::measureText(getParentWindow(), pcsText, rCachedParms, svTextSize);
	rPushParms->myTextDims = svTextSize;

	int xSize, ySize;
	getGlBitmapSize(svTextSize, xSize, ySize);
	rPushParms->myBitmapsDims.set(xSize, ySize);

	unsigned char* pStorage = new unsigned char[ xSize * ySize * 4 ];

#ifdef DIRECTX_PIPELINE
	// We clear after the text now, not before...
	///g_pDxRenderer->Clear(true, true);
	g_pDxRenderer->startBlendModeOverride(BlendModeText);
#else
	rPushParms->myTextureIndex = GraphicsUtils::createTextureFrom(xSize, ySize, pStorage, rParms.myResampleNearest);
	/*
	GraphicsUtils::generateTextures(1, &rPushParms->myTextureIndex);
	Graphics::setDiffuseTexture(rPushParms->myTextureIndex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, xSize, ySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, pStorage);

	if(rParms.myResampleNearest)
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	*/
	//clean up
	delete[] pStorage;

	glClear(GL_COLOR_BUFFER_BIT);
#endif
	rCachedParms.myColor.set(1,1,1,1);
	// TEMP hack - the problem is we can change the color, and the bitmap is
	// independent of it!
	//rCachedParms.myHighlightColor.set(0.996, 0.894, 0.443, 1.0);
	// Since we've already inserted newlines where needed, we can
	// tell the renderer there are really no widht requirements now.
	rCachedParms.myBaseInfo.myWidth = 0;
	// We always render at scale 1.0 for cached textures, then scale them
	// later, when actually rendering on screen.
	_ASSERT(rCachedParms.myScale == 1.0);
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	RenderUtils::drawText(getParentWindow()->getDrawingCache(), pcsText, rCachedParms);
	getParentWindow()->getDrawingCache()->flush();

	// Save data to texture using glCopyTexImage2D
#ifdef DIRECTX_PIPELINE

	g_pDxRenderer->endBlendModeOverride();

	// Generate the texture
	D3D11_SUBRESOURCE_DATA textureSubresourceData = {0};
	//memset(pStorage, 255, sizeof(unsigned char)*xSize*ySize*4);
	textureSubresourceData.pSysMem = pStorage;

	// Specify the size of a row in bytes, known a priori about the texture data.
	textureSubresourceData.SysMemPitch = xSize*4;

	// As this is not a texture array or 3D texture, this parameter is ignored.
	textureSubresourceData.SysMemSlicePitch = 0;

	// Create a texture description from information known a priori about the data.
	// Generalized texture loading code can be found in the Resource Loading sample.
	// DXGI_FORMAT_R8G8B8A8_TYPELESS
	D3D11_TEXTURE2D_DESC textureDesc = {0};
	textureDesc.Width = xSize;
	textureDesc.Height = ySize;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0; /// D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

	// Most textures contain more than one MIP level.  For simplicity, this sample uses only one.
	textureDesc.MipLevels = 1;

	// As this will not be a texture array, this parameter is ignored.
	textureDesc.ArraySize = 1;

	// Don't use multi-sampling.
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	// Allow the texture to be bound as a shader resource.
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; //  | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	DX::ThrowIfFailed(
		g_pDxRenderer->getD3dDevice()->CreateTexture2D(&textureDesc, &textureSubresourceData, &texture)
		);


	// Get a pointer to the back buffer texture
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	DX::ThrowIfFailed(
		g_pDxRenderer->getSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer)
		);

	D3D11_BOX sourceRegion;
	sourceRegion.left = 0;
	sourceRegion.top = 0;
	sourceRegion.right = xSize;
	sourceRegion.bottom = ySize;
	sourceRegion.front = 0;
	sourceRegion.back = 1;

	// Once the texture is created, we must create a shader resource view of it
	// so that shaders may use it.  In general, the view description will match
	// the texture description.
	D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc;
	ZeroMemory(&textureViewDesc, sizeof(textureViewDesc));
	textureViewDesc.Format = textureDesc.Format;
	textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	textureViewDesc.Texture2D.MostDetailedMip = 0;

	DX::ThrowIfFailed(
		g_pDxRenderer->getD3dDevice()->CreateShaderResourceView(texture.Get(), &textureViewDesc, &rPushParms->myActualTexturePtr)
		);
	rPushParms->myTextureIndex = &rPushParms->myActualTexturePtr;

	g_pDxRenderer->getD3dContext()->CopySubresourceRegion(texture.Get(), 0, 0, 0, 0, backBuffer.Get(), 0, &sourceRegion);

	delete[] pStorage;
	g_pDxRenderer->Clear(true, true);

#else

	//glBindTexture(GL_TEXTURE_2D, rPushParms->myTextureIndex);
	GraphicsUtils::setDiffuseTexture(rPushParms->myTextureIndex);

	SRect2D srSrcScreenArea;
	getSrcScreenArea(xSize, ySize, srSrcScreenArea);

#ifdef USE_OPENGL2
	SVector2D svScreen;
	getParentWindow()->getSize(svScreen);
	// Here we're starting from the lower-left corner... 
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, srSrcScreenArea.x, svScreen.y - srSrcScreenArea.h, (int)srSrcScreenArea.w, (int)srSrcScreenArea.h, 0);
	//glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, srSrcScreenArea.x, srSrcScreenArea.y, (int)srSrcScreenArea.w, (int)srSrcScreenArea.h, 0);
#else
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, srSrcScreenArea.x, srSrcScreenArea.y, (int)srSrcScreenArea.w, (int)srSrcScreenArea.h, 0);
#endif
#endif


	// Finally, push it onto our map
	// Note that we need to insert the oriignal string here, not the one (pcsText) that was
	// possibly modified during measurements.
//	TTextParmsMap::iterator new_elem = myEntries.insert(TTextParmsMap::value_type(pcsText, rPushParms));

	TTextParmsMap::iterator new_elem = myEntries.insert(TTextParmsMap::value_type(theSharedString, rPushParms));
	myMemoryUsed += rPushParms->getBytesUsed();

//	myCacher.addToCache(theSharedString.c_str(), rPushParms);

	return new_elem->second;
}
/*****************************************************************************/
void TextCacher::queueForCreation(const char* pcsText, STextParams& rParms, bool bForceCaching)
{
	if(!getIsCachingEnabled())
		return;

	// NOTE: We need to force-queue strings if we're in the clipping region,
	// since otherwise they won't be cut by the region.
	if(!bForceCaching)
	{
		// Don't queue short strings and strings that are pure numbers, since
		// these could be changing fast. Include , in pure numbers test to account
		// for scores.
		int iLen = strlen(pcsText);
		// Do not cache short strings.
		if(iLen <= 3)
			return;

		// See if it's a relatively longer number
		if(iLen <= 7 && StringUtils::isANumberIgnoreSpaces(pcsText, true, true))
			return;
	}

	myToBeCreatedStrings.push_back(pcsText);
	myToBeCreated.push_back(rParms);

	ActivityManager::getInstance()->singleValueChanged();
}
/*****************************************************************************/
void TextCacher::createQueued()
{
	getParentWindow()->getDrawingCache()->setAllowFlushingText(false);
	getParentWindow()->getDrawingCache()->flush();

#ifdef DIRECTX_PIPELINE

#else
#if defined(WIN32) || defined(MAC_BUILD)

#ifdef USE_OPENGL2
	RenderStateManager::getInstance()->setShaderType(ShaderSetFlatTexture);

	SVector2D svScreenDims;
	getParentWindow()->getSize(svScreenDims);
	GraphicsUtils::transformForGlOrtho(svScreenDims.x, svScreenDims.y);
#else
	glLoadIdentity();
#endif

#endif
#endif

	int iCurr, iNum = myToBeCreated.size();
	// This might prevent cases where the text has
	// black background
	if(iNum > 0)
	{
#ifdef DIRECTX_PIPELINE
		g_pDxRenderer->Clear(true, true);
#else
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
#endif
	}

	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		addTextureFor(myToBeCreatedStrings[iCurr].c_str(), myToBeCreated[iCurr]);
	}
	myToBeCreated.clear();
	myToBeCreatedStrings.clear();


	getParentWindow()->getDrawingCache()->setAllowFlushingText(true);

}
/*****************************************************************************/
};