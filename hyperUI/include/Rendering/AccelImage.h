#pragma once

/*
Image class which contains hardware-accelerated image.
May consist of several chunks.
Only one for now.
*/

/*****************************************************************************/
class HYPERUI_API AccelImage
{
public:
    AccelImage();
    virtual ~AccelImage();

	void updateFrom(const IBaseImage* pImage, const SRect2D* pOptRect = NULL, bool bOnlySetData = false, int iVisibleChannels = ChannelsRGB);
	inline void setFromImage(const IBaseImage* pImage, const SRect2D* pOptRect = NULL, bool bOnlySetData = false, int iVisibleChannels = ChannelsRGB) { updateFrom(pImage, pOptRect, bOnlySetData, iVisibleChannels); }

	void render(IWindowBase* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fOpacity) const;
    void clear(bool bClearChunks = true);

    static void getChunkSizesFor(int iW, int iH, int& iChunkSizeXOut, int& iChunkSizeYOut);

    void getDimensions(int &iW, int& iH) const { iW = myW; iH = myH; }
	void setSamplingMode(TextureSamplingType eType);

	void getColorAt(int iX, int iY, SColor& scolOut);
	void getBBox(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, SRect2D& srOut);

	inline int getWidth() const { return myW; }
	inline int getHeight() const { return myH; }

	virtual AccelImageChunk* allocateChunk(int iChunkWeight, int iChunkHeight) = 0;

protected:

    inline int getOffset(int iX, int iY) const { return iY*myNumColumns + iX; }
	void setFromImageInternal(const IBaseImage* pImage, const SRect2D* pOptRect, bool bOnlySetData, int iVisibleChannels);

	void deleteChunks();
	void ensureFinished();

protected:

    // 2D array in flat form.
    TImageChunks myChunks;
	// This is necessary because on a Mac, we can only
	// delete textures from the rendering thread.
	TImageChunks myChunksToDelete;
    int myNumRows, myNumColumns;

    // Maximum texture dims
    static int theMaxChunkSize;
    int myChunkSizeX, myChunkSizeY;

    // Actual image size
    int myW, myH;
	//int myX, myY;

	TextureSamplingType mySamplingMode;

	TUCharLossyDynamicArray myStorage;

	// True when only data has been set and we need to finish GL
	// texture creation, false otherwise.
	mutable bool myIsUnfinished;
	mutable bool myIsDoEraseChunks;

	//mutable boost::recursive_mutex myImageLock;
};
/*****************************************************************************/
