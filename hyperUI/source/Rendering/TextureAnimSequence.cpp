#include "stdafx.h"

#if !defined(WIN32) && !defined(LINUX)
#include "MacUtils.h"
#endif

// Unload ticks when getting the mem low message.
#define TEXTURE_EMERGENCY_UNLOAD_TICKS		2

// In seconds
// This is the normal time
#if defined(WIN32) || defined(MAC_BUILD)
	#define TEXTURE_UNLOAD_TIME     15.0
#else
	#define TEXTURE_UNLOAD_TIME     5.0
#endif

// Turn on to debug texture loading/unloading in VS.
//#define VS_DEBUG_TEXTURE_UNLOADING

namespace HyperUI
{
/********************************************************************************************/
TextureAnimSequence::TextureAnimSequence(const char* pcsAnimName, const char* pcsFileName, int iFirstFrame, int iLastFrame,
										 int iFrameStep, FLOAT_TYPE iFrameWidth, FLOAT_TYPE iFrameHeight, bool bIsCompressed, BlendModeType eBlendMode,
										 bool bResampleNearest, int iRenderStage, bool bIsUnloadable, const ResourceItem* pAnimItem, const CHAR_TYPE* pcsOptTargetFolderDataPath)
{
//	Application::lockGlobalDisplay();

	myResampleNearest = bResampleNearest;
	myIsCompressed = bIsCompressed;
	myRefCount = 0;
	//myRenderStage = (RenderStageType)iRenderStage;
	myIsUnloadable = bIsUnloadable;
	myRenderStage = iRenderStage;
	myFirstFrame = iFirstFrame;
	myNumFrames = (iLastFrame - iFirstFrame)/iFrameStep + 1;
	myLastUsedTime = 0;
	myDisableAlphaTest = pAnimItem->getBoolProp(PropertyDisableAlphaTest);
	myForceAlphaTest = pAnimItem->getBoolProp(PropertyForceAlphaTest);

	if(pAnimItem->doesPropertyExist(PropertySubtileMap))
	{
		mySubtileMap = pAnimItem->getArray2DProp(PropertySubtileMap, mySubtileMapWidth, mySubtileMapHeight);
	}
	else
	{
		mySubtileMapWidth = 0;
		mySubtileMapHeight = 0;
		mySubtileMap = NULL;
	}

	myAnimName = pcsAnimName;
	myBlendMode = eBlendMode;

	myRealWidth = 0;
	myRealHeight = 0;

#if !defined(WIN32) && !defined(LINUX)
	myCompressedImageData = NULL;
#endif

	int iNumFiles = myNumFrames;
	myFrameWidth = iFrameWidth;
	myFrameHeight = iFrameHeight;
	if(myFrameWidth > 0 && myFrameHeight > 0)
	{
		iLastFrame = iFirstFrame;
		// Paged animations only have one file
		iNumFiles = 1;
	}

#ifdef DIRECTX_PIPELINE
	// This will be set when we're first laoding a texture.
	myFrames = NULL;
#else
	myFrames = new GLuint[iNumFiles];
#endif
	myBaseName = pcsFileName;
	myFrameStep = iFrameStep;

	myNumFiles = iNumFiles;

	myIsLoaded = false;

	if(IS_VALID_STRING_AND_NOT_NONE(pcsOptTargetFolderDataPath))
		myOptTargetFolderDataPath = pcsOptTargetFolderDataPath;
	
    // On Mac, we need to be in the display thread to load the actual texture.
#if !defined(OSX_BUILD) && !defined(IOS_BUILD)
#if defined(ENABLE_TEXTURE_UNLOADING)
	if(bIsUnloadable == false)
#endif
		ensureLoaded();
#endif
    
#if 0
	glGenTextures(iNumFiles, myFrames);

	GLuint iTexIdx;
	int iCurrFrame;
	string strName;
	// char pcsBuff[256];
	int iArrayIdx = 0;
	for(iCurrFrame = iFirstFrame; iCurrFrame < iFirstFrame + iNumFiles; iCurrFrame += iFrameStep)
	{
		makeImagePath(pcsFileName, iCurrFrame, bIsCompressed, myOptTargetFolderDataPath.c_str(), strName);


#if defined(WIN32) || defined(MAC_BUILD)
		iTexIdx = initGLTexture(strName.c_str(), myFrames[iArrayIdx], bResampleNearest);
#else
		if(bIsCompressed)
			iTexIdx = initCompressedGLTexture(strName.c_str(), myFrames[iArrayIdx], bResampleNearest);
		else
			iTexIdx = initGLTexture(strName.c_str(), myFrames[iArrayIdx], bResampleNearest);
#endif

		iArrayIdx++;

#ifdef _DEBUG
#if 0
		static int iMemCount = 0;
		int iOwnMem = myWidth * myWidth * 4;
		iMemCount += iOwnMem;
		NSLog(@"Loaded: %s Own Mem: %d Total Mem: %d\n", strName.c_str(), iOwnMem/1024, iMemCount/1024);
#endif
#endif
	}

	myIsLoaded = true;
#endif
//	Application::unlockGlobalDisplay();

}
/********************************************************************************************/
TextureAnimSequence::~TextureAnimSequence()
{
	delete[] myFrames;

#if !defined(WIN32) && !defined(LINUX)
	if(myCompressedImageData)
	{
		releaseAsMutableArray(myCompressedImageData);
	}
#endif
}
/********************************************************************************************/
void TextureAnimSequence::makeImagePath(const char* pName, int iFrame, bool bIsCompressed, const CHAR_TYPE* pcsOptTargetFolderDataPath, string& strFinalPathOut)
{
	string strDataPathPrefix;
	string strGraphicsFolder;
	string strFullPath;

	if(IS_VALID_STRING_AND_NOT_NONE(pcsOptTargetFolderDataPath))
	{
		strFullPath = "";
		strDataPathPrefix = "";
		strGraphicsFolder = pcsOptTargetFolderDataPath;
	}
	else
	{
		strFullPath = ResourceManager::getFullAppPath();
		strDataPathPrefix = ResourceManager::getStandardDataPathPrefix();
		strGraphicsFolder = IMAGES_PREFIX_PATH;
	}


	// First, try our graphics folder path with the density suffix.
	strFinalPathOut = strFullPath;
	strFinalPathOut += strDataPathPrefix;
	if(strGraphicsFolder.length() > 0)
	{
		strFinalPathOut += strGraphicsFolder;
		STRING_TYPE strResSuffix;
		ResourceManager::getDensityFileSuffix(strResSuffix);
		strFinalPathOut += strResSuffix;
		strFinalPathOut += FOLDER_SEP;
	}

	strFinalPathOut += pName;
#if !defined(WIN32) && !defined(MAC_BUILD) && !defined(LINUX)
	if(bIsCompressed)
		strFinalPathOut += ".pvr";
	else
#endif
		strFinalPathOut += ".png";

	if(FileUtils::doesFileExist(strFinalPathOut.c_str(), FileSourceFullPath))
		return;

	// Otherwise, try the literal file. Note that this basically ignores the 
	// density suffix.
	strFinalPathOut = strFullPath;
	strFinalPathOut += strDataPathPrefix;
	if(strGraphicsFolder.length() > 0)
	{
		strFinalPathOut += strGraphicsFolder;
		strFinalPathOut += FOLDER_SEP;
	}
	strFinalPathOut += pName;
#if !defined(WIN32) && !defined(MAC_BUILD) && !defined(LINUX)
	if(bIsCompressed)
		strFinalPathOut += ".pvr";
	else
#endif
		strFinalPathOut += ".png";

	if(FileUtils::doesFileExist(strFinalPathOut.c_str(), FileSourceFullPath))
		return;

	// If not, try the actual path in as the filename as a last resort.
	strFinalPathOut = pName;
	return;
}
/********************************************************************************************/
void TextureAnimSequence::setRealImageDims(int iRealW, int iRealH)
{
	myRealWidth = iRealW;
	myRealHeight = iRealH;
}
/********************************************************************************************/
void TextureAnimSequence::setCornerSize(int iRealW, int iRealH)
{
	myCornerWidth = iRealW;
	myCornerHeight = iRealH;
}
/********************************************************************************************/
void TextureAnimSequence::getCornerSize(int &iRealW, int &iRealH)
{
	iRealW = myCornerWidth;
	iRealH = myCornerHeight;
}
/********************************************************************************************/
void TextureAnimSequence::getRealImageDims(int &iRealW, int &iRealH) const
{
	// Can't have a real dim for animated texture.
	// Sure we can now.
	//_ASSERT(myFrameWidth <= 0);
	iRealW = myRealWidth;
	iRealH = myRealHeight;
}
/********************************************************************************************/
void TextureAnimSequence::markUsed(void)
{
	myLastUsedTime = Application::getInstance()->getGlobalTime(ClockUniversal);
}
/********************************************************************************************/
void TextureAnimSequence::addRef(void)
{
	myRefCount++;
}
/********************************************************************************************/
void TextureAnimSequence::release(void)
{
	myRefCount--;
	if(myRefCount <= 0)
		delete this;
}
/********************************************************************************************/
void TextureAnimSequence::getNameFromProgress(FLOAT_TYPE fProgress, string& strOut)
{
	// Paged animations do not support this
	_ASSERT(myFrameWidth <= 0 && myFrameHeight <= 0);

	int iFrame = fProgress*(FLOAT_TYPE)myNumFrames;
	iFrame *= myFrameStep;
	getName(iFrame, strOut);
}
/********************************************************************************************/
void TextureAnimSequence::getName(int iFrame, string& strOut)
{
	// Paged animations do not support this
	_ASSERT(myFrameWidth <= 0 && myFrameHeight <= 0);

	char pcsBuff[256];
	sprintf(pcsBuff, "%.4d", iFrame + myFirstFrame);
	strOut = myBaseName;
	strOut += pcsBuff;
	strOut += ".png";
}
/********************************************************************************************/
bool TextureAnimSequence::isTextureAnAtlas(void)
{
	return (myFrameHeight > 0 && myFrameWidth > 0);
}
/********************************************************************************************/
bool TextureAnimSequence::shouldDoEmergencyUnload()
{
	if(!myIsLoaded)
		return false;

	if(!myIsUnloadable)
		return false;

	GTIME lTime = Application::getInstance()->getGlobalTime(ClockUniversal);
	return myLastUsedTime > 0 && myLastUsedTime + TEXTURE_EMERGENCY_UNLOAD_TICKS < lTime;
}
/********************************************************************************************/
bool TextureAnimSequence::shouldUnload(void)
{
	if(!myIsLoaded)
		return false;

	if(!myIsUnloadable)
		return false;

	GTIME lTime = Application::getInstance()->getGlobalTime(ClockUniversal);
	return myLastUsedTime > 0 && myLastUsedTime + Application::secondsToTicks(TEXTURE_UNLOAD_TIME) < lTime;
}
/********************************************************************************************/
FLOAT_TYPE* TextureAnimSequence::getSubtileMap(int& iMapWidth, int& iMapHeight)
{
	iMapWidth = mySubtileMapWidth;
	iMapHeight = mySubtileMapHeight;
	return mySubtileMap;
}
/********************************************************************************************/
bool TextureAnimSequence::getIsTextureUnloadable(void)
{
	return myIsUnloadable;
}
/********************************************************************************************/
int TextureAnimSequence::getRenderStage(void)
{
	return myRenderStage;
}
/********************************************************************************************/
BlendModeType TextureAnimSequence::getBlendMode(void)
{
	return myBlendMode;
}
/********************************************************************************************/
int TextureAnimSequence::getFileHeight(void)
{
	return myHeight;
}
/********************************************************************************************/
int TextureAnimSequence::getFileWidth(void)
{
	return myWidth;
}
/********************************************************************************************/
FLOAT_TYPE TextureAnimSequence::getWidth(void)
{
	if(myFrameWidth > 0 && myFrameHeight > 0)
		return myFrameWidth;
	else
		return myWidth;
}
/********************************************************************************************/
FLOAT_TYPE TextureAnimSequence::getHeight(void)
{
	if(myFrameWidth > 0 && myFrameHeight > 0)
		return myFrameHeight;
	else
		return myHeight;
}
/********************************************************************************************
int TextureAnimSequence::getFrameNumberFromProgress(FLOAT_TYPE fProgress)
{
return 0;
}
/********************************************************************************************/
TX_MAN_RETURN_TYPE TextureAnimSequence::getFrameFromProgress(FLOAT_TYPE fProgress, SUVSet& uvsOut)
{
	int iFrame = fProgress*(FLOAT_TYPE)myNumFrames;
	return getFrame(iFrame, uvsOut);
}
/********************************************************************************************/
TX_MAN_RETURN_TYPE TextureAnimSequence::getFrame(int iFrame, SUVSet& uvsOut)
{
	if(myFrameWidth > 0 && myFrameHeight > 0)
	{
		// See where we are in the animation
		// TODO: iAnimFrame 1 or 0 based?
		// below code assumes 0.
		// fArrayOut is assumed to be a single-dimensional float array for a quad, two indices per point

		int iSizeW = myWidth/myFrameWidth;
		int iRow = iFrame/iSizeW;
		int iCol = iFrame - iRow*iSizeW;

		uvsOut.myStartX = ((FLOAT_TYPE)((FLOAT_TYPE)iCol*myFrameWidth))/(FLOAT_TYPE)myWidth;
		uvsOut.myStartY = ((FLOAT_TYPE)((FLOAT_TYPE)iRow*myFrameHeight))/(FLOAT_TYPE)myHeight;
		uvsOut.myEndX = uvsOut.myStartX + ((FLOAT_TYPE)myFrameWidth)/(FLOAT_TYPE)myWidth;
		uvsOut.myEndY = uvsOut.myStartY + ((FLOAT_TYPE)myFrameHeight)/(FLOAT_TYPE)myHeight;

#ifdef DIRECTX_PIPELINE
		return &(myFrames[0]);
#else
		return myFrames[0];
#endif
	}

	uvsOut.reset();
	int iIdx = iFrame;
	if(iIdx < 0)
		iIdx = 0;
	if(iIdx > myNumFrames - 1)
		iIdx = myNumFrames - 1;
#ifdef DIRECTX_PIPELINE
	return &(myFrames[iIdx]);
#else
	return myFrames[iIdx];
#endif
}
/********************************************************************************************/
void TextureAnimSequence::unloadTexture(void)
{

	if(!myIsLoaded)
		return;

#ifdef HARDCORE_LINUX_DEBUG_MODE
	Logger::log("TextureAnimSequence::unloadTexture for %s texIdx = %d", myBaseName.c_str(), myFrames[0]);
#endif

#ifdef VS_DEBUG_TEXTURE_UNLOADING
#ifdef _DEBUG
	char pcsBuff[1024];
	sprintf(pcsBuff,  "UNLOADED TEXTURE: %s (%d, first = %d) context = %x\n", this->myAnimName.c_str(), myNumFiles, myFrames[0], wglGetCurrentContext());
	gLog(pcsBuff);
#endif
#endif

#ifdef DIRECTX_PIPELINE

	// ComPtr<ID3D11Texture2D> texture
	int iFrame;
	int bp = 0;
	for(iFrame = 0; iFrame < myNumFiles; iFrame++)
	{
		if(myFrames[iFrame])
			myFrames[iFrame]->Release();
	}
	//g_pDxRenderer->getD3dDevice()->CreateBuffer\

	delete[] myFrames;
	myFrames = NULL;
#else
	glDeleteTextures(myNumFiles, myFrames);
#endif
	myIsLoaded = false;
}
/********************************************************************************************/
void TextureAnimSequence::ensureLoaded(void)
{
	//gLog("TextureAnimSequence::ensureLoaded for %s isLoaded = %s\n", myBaseName.c_str(), myIsLoaded ? "YES" : "NO");

	if(myIsLoaded)
		return;

	//Application::lockGlobalDisplay();

#ifdef _DEBUG
	if(myAnimName == "Light_button")
	{
		int bp = 0;
	}
#endif

#ifdef HARDCORE_LINUX_DEBUG_MODE
	Logger::log("TextureAnimSequence::ensureLoaded for %s", myBaseName.c_str());
#endif

#ifdef DIRECTX_PIPELINE
	if(!myFrames)
		myFrames = new ID3D11ShaderResourceView* [myNumFiles];
	memset(myFrames, 0, sizeof(ID3D11ShaderResourceView*)*myNumFiles);
	//myFrames = new Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>[myNumFiles];
#else
	glGenTextures(myNumFiles, myFrames);
	GLuint iTexIdx;
#endif
	int iCurrFrame;
	string strName;
	int iArrayIdx = 0;
	for(iCurrFrame = myFirstFrame; iCurrFrame < myFirstFrame + myNumFiles; iCurrFrame += myFrameStep)
	{
		makeImagePath(myBaseName.c_str(), iCurrFrame, myIsCompressed, myOptTargetFolderDataPath.c_str(), strName);
        //gLog("Image: chose %s\n", strName.c_str());

#if defined(DIRECTX_PIPELINE)
		initDxTexture(strName.c_str(), &(myFrames[iArrayIdx]), myResampleNearest);
#elif defined(WIN32) || defined(MAC_BUILD) || defined(LINUX)
		iTexIdx = initGLTexture(strName.c_str(), myFrames[iArrayIdx], myResampleNearest);
#else
		if(myIsCompressed)
			iTexIdx = initCompressedGLTexture(strName.c_str(), myFrames[iArrayIdx], myResampleNearest);
		else
			iTexIdx = initGLTexture(strName.c_str(), myFrames[iArrayIdx], myResampleNearest);
#endif

#ifdef HARDCORE_LINUX_DEBUG_MODE
		Logger::log("TextureAnimSequence::ensureLoaded for %s, result = %d, path = %s", myBaseName.c_str(), iTexIdx, strName.c_str());
#endif


		iArrayIdx++;

#ifdef _DEBUG
#if 0
		static int iMemCount = 0;
		int iOwnMem = myWidth * myWidth * 4;
		iMemCount += iOwnMem;
		NSLog(@"Loaded: %s Own Mem: %d Total Mem: %d\n", strName.c_str(), iOwnMem/1024, iMemCount/1024);
#endif
#endif
	}

#ifdef VS_DEBUG_TEXTURE_UNLOADING
#ifdef _DEBUG
	char pcsBuff[1024];
	sprintf(pcsBuff,  "LOADED TEXTURE: %s (%d) context = %x\n", this->myAnimName.c_str(), myFrames[0], wglGetCurrentContext());
	gLog(pcsBuff);
#endif
#endif

	myIsLoaded = true;
	markUsed();
//	Application::unlockGlobalDisplay();
}
/********************************************************************************************/
};
