/*
 *  TextureManager.h
 *  SpaceX
 *
 *  Created by Oleg Samus on 12/01/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#ifndef WIN32
	//#include "Common.h"
#endif

#if !defined(DOING_VIDEO_DEMO) && !defined(LINUX)
	#define ENABLE_TEXTURE_UNLOADING
#endif

class Window;
/********************************************************************************************/
class HYPERUI_API TextureManager
{
public:
	TextureManager(Window* pParentWindow);
	~TextureManager();

	void initFromCollection(ResourceCollection* pCollection, const CHAR_TYPE* pcsOptTargetFolderDataPath, IProgressCallbacks* pCallbacks);
	void initAnimation(const char* pcsAnimName, const CHAR_TYPE* pcsFileName, bool bIsCompressed, BlendModeType eBlendMode, bool bIsUnloadable, const ResourceItem* pItem, const CHAR_TYPE* pcsOptTargetFolderDataPath);

	void getCornerSize(const char* pcsAnimName, int &iRealW, int &iRealH);
	void getTextureRealDims(const char* pcsAnimName, int &iRealW, int &iRealH) const;
	int getWidth(const char* pcsAnimName);
	int getHeight(const char* pcsAnimName);
	int getFileWidth(const char* pcsAnimName);
	int getFileHeight(const char* pcsAnimName);
	TX_MAN_RETURN_TYPE getTextureIndex(const char* pcsAnimName);
	TX_MAN_RETURN_TYPE getTextureIndex(const char* pcsAnimName, int iFrame, int& iWidth, int& iHeight, SUVSet& uvsOut, BlendModeType& eBlendMode, TextureAnimSequence *pOptExistingPointer = NULL);
	TX_MAN_RETURN_TYPE getTextureIndexFromProgress(const char* pcsAnimName, FLOAT_TYPE fProgress, int& iWidth, int& iHeight, SUVSet& uvsOut, BlendModeType& eBlendMode, TextureAnimSequence *pOptExistingPointer = NULL);

	void getTextureName(const char* pcsAnimName, int iFrame, string& strOut);
	void getTextureNameFromProgress(const char* pcsAnimName, FLOAT_TYPE fProgress, string& strOut);

	bool getForceAlphaTest(const char* pcsAnimName);
	bool getDisableAlphaTest(const char* pcsAnimName);
	int getTextureRenderStage(const char* pcsAnimName);
	bool getIsTextureUnloadable(const char* pcsAnimName);
	FLOAT_TYPE* getSubtileMap(const char* pcsAnimName, int& iMapWidth, int& iMapHeight);

	void setTextureBlendMode(const char* pcsAnimName, BlendModeType eMode);

	bool isTextureAnAtlas(const char* pcsAnimName);
	bool isInitializing(void) { return myIsInitializing; }

	TextureAnimSequence* getTexture(const char* pcsAnimName);
	const TextureAnimSequence* getTexture(const char* pcsAnimName) const;

	void ensureLoaded(const char* pcsAnimName);
	void reload(const char* pcsAnimName);
	void loadFromFile(const char* pcsAnimName, const char* pcsFileName);
	void reloadAll();
	void unloadAll();

	inline const Window* getParentWindow() const { return myParentWindow; }
	inline Window* getParentWindow() { return myParentWindow; }

#ifdef _DEBUG
	void dbCheckConsistency();
#endif

private:

	//TextureAnimSequence *myTextures[MAX_NUM_TEXTURES];
	TTexturesMap myTextures;
	TTexturesMap myLoadedFiles;

	bool myIsInitializing;

	mutable TAnimSeqCacher myCacher;
	Window* myParentWindow;
};
/*****************************************************************************/
