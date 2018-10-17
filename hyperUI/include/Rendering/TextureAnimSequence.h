#pragma once

/********************************************************************************************/
class HYPERUI_API TextureAnimSequence
{
public:
	TextureAnimSequence(const char* pcsAnimName, const char* pcsFileName, int iFirstFrame, int iLastFrame, int iFrameStep, FLOAT_TYPE iFrameWidth, FLOAT_TYPE iFrameHeight, bool bIsCompressed, BlendModeType eBlendMode, bool bResampleNearest, int iRenderStage, bool bIsUnloadable, const ResourceItem* pAnimItem, const CHAR_TYPE* pcsOptTargetFolderDataPath);
	~TextureAnimSequence();

	void addRef(void);
	void release(void);

	void setRealImageDims(int iRealW, int iRealH);
	void getRealImageDims(int &iRealW, int &iRealH) const;
	void getCornerSize(int &iRealW, int &iRealH);
	void setCornerSize(int iRealW, int iRealH);

	TX_MAN_RETURN_TYPE getFrameFromProgress(FLOAT_TYPE fProgress, SUVSet& uvsOut);
	TX_MAN_RETURN_TYPE getFrame(int iFrame, SUVSet& uvsOut);
	FLOAT_TYPE getWidth(void);
	FLOAT_TYPE getHeight(void);
	int getFileWidth(void);
	int getFileHeight(void);
	void getName(int iFrame, string& strOut);
	inline const char* getName() const { return myAnimName.c_str(); }
	inline const char* getId() const { return myAnimName.c_str(); }
	void getNameFromProgress(FLOAT_TYPE fProgress, string& strOut);
	BlendModeType getBlendMode(void);
	int getRenderStage(void);
	bool isTextureAnAtlas(void);
	bool getIsTextureUnloadable();
	FLOAT_TYPE* getSubtileMap(int& iMapWidth, int& iMapHeight);

	bool shouldUnload();
	bool shouldDoEmergencyUnload();

	void setBlendMode(BlendModeType eMode) { myBlendMode = eMode; }
	void unloadTexture(void);
	void ensureLoaded(void);

	void markUsed(void);

	bool getForceAlphaTest() { return myForceAlphaTest; }
	bool getDisableAlphaTest() { return myDisableAlphaTest; }
	bool getIsLoaded() { return myIsLoaded; }
	inline bool getResampleNearest() { return myResampleNearest; }
private:

#ifdef DIRECTX_PIPELINE
	void initDxTexture(const char* pcsName, ID3D11ShaderResourceView** spriteTexture, bool bResampleNearest);
#else
	GLuint initGLTexture(const char* pcsName, GLuint spriteTexture, bool bResampleNearest);
#endif

#if !defined(WIN32) && !defined(MAC_BUILD) && !defined(LINUX)
	GLuint initCompressedGLTexture(const char* pcsName, GLuint spriteTexture, bool bResampleNearest);
#endif

	void makeImagePath(const char* pName, int iFrame, bool bIsCompressed, const CHAR_TYPE* pcsOptTargetFolderDataPath, string& strFinalPathOut);


private:

	// Assumes the width and the height is constant for all frames
	int myWidth, myHeight;
	string myBaseName;

	int myRefCount;

	string myAnimName;

	string myOptTargetFolderDataPath;

	int myRenderStage;
	bool myIsUnloadable;
	bool myIsCompressed, myResampleNearest;

	BlendModeType myBlendMode;

	int myRealWidth, myRealHeight;
	int myCornerWidth, myCornerHeight;

	// For paged animated textures, this will be non-zero
	FLOAT_TYPE myFrameWidth, myFrameHeight;

	bool myIsLoaded;

	GTIME myLastUsedTime;
	// Only for persp and deep sequences
	bool myDisableAlphaTest, myForceAlphaTest;

	//string theCommonString;

	int myFrameStep;
	int myFirstFrame, myNumFrames;
	int myNumFiles;

	int mySubtileMapWidth;
	int mySubtileMapHeight;
	FLOAT_TYPE *mySubtileMap;

#ifdef DIRECTX_PIPELINE
	ID3D11ShaderResourceView** myFrames;
#else
	GLuint *myFrames;
#endif

#if !defined(WIN32) && !defined(LINUX)
	// NOTE: This is actually an NSMutableArray!
	void* myCompressedImageData;
#endif

};
typedef map < string, TextureAnimSequence* > TTexturesMap;
typedef StringMapCacher < TextureAnimSequence > TAnimSeqCacher;
// typedef map < AnimationType, TextureAnimSequence* > TTexturesMap;
/********************************************************************************************/
