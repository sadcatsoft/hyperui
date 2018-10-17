#pragma once

#define	ACELL_FLOAT_TYPE		float

class HYPERUI_API AccelImage;
/*****************************************************************************/
class HYPERUI_API AccelImageChunk
{
public:
    AccelImageChunk(int iChunkSizeX, int iChunkSizeY);
    virtual ~AccelImageChunk();

	void render(IWindowBase* pWindow, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fOpacity) const;

	void setFrom(const IBaseImage* pImage, int iStartX, int iStartY, int iW, int iH, TUCharLossyDynamicArray* pStorage, bool bOnlySetData, int iVisibleChannels);
	void updateFrom(const IBaseImage* pImage, int iStartX, int iStartY, int iW, int iH, TUCharLossyDynamicArray* pStorage, bool bOnlySetData, int iVisibleChannels);
    void clear(bool bDeleteTexture = true);

	void finishTextureCreation(bool bUpdate, unsigned char *pRawData);

	inline bool getDidUpdateLast() const { return myDidUpdateLast; }
	void getColorAt(int iX, int iY, SColor& scolOut);

protected:

	friend class AccelImage;
	inline TX_MAN_RETURN_TYPE getTextureId() const { _ASSERT(myTextureId > 0); return myTextureId; }

	virtual void setDataInternal(const IBaseImage* pImage, int iStartX, int iStartY, int iW, int iH, bool bUpdate, TUCharLossyDynamicArray* pStorage, bool bOnlySetData, int iVisibleChannels) = 0;

protected:

    TX_MAN_RETURN_TYPE myTextureId;
#ifdef DIRECTX_PIPELINE
	ID3D11ShaderResourceView* myActualTexturePtr;
#endif
    int myChunkSizeX, myChunkSizeY;

	// These are used only for data holding
	TUCharLossyDynamicArray* myOwnTempStorage;
	bool myDidUpdateLast;
};
/*****************************************************************************/
typedef vector < AccelImageChunk* > TImageChunks;
/*****************************************************************************/