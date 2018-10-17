#pragma once

class UIHistogram;
/*****************************************************************************/
struct HYPERUI_API SHistSDataSeries
{
	SHistSDataSeries() { myVertexInfo = NULL; myNumVerts = 0; myIsVisible = true; }
	~SHistSDataSeries() { clear(); }

	void clear() { delete[] myVertexInfo; myVertexInfo = NULL; myNumVerts = 0; }
	void recomputeGeometry(UIHistogram* pHistogram, const SRect2D& srWindowRect);
	SVertexInfo* getGeometry(UIHistogram* pHistogram, const SRect2D& srWindowRect, int &iNumVertsOut);

	TRawHistDataEntries myData;
	SColor myColor;

	SRect2D myCachedRect;
	PathTrail myLine;
	SVertexInfo* myVertexInfo;
	int myNumVerts;
	bool myIsVisible;

	HyperUI::ChannelType myChannelType;

	static ColorPolyline2D thePolyline;
	static THistDataEntries theChannelMap;
};
typedef vector < SHistSDataSeries* > THistDataSeries;
/*****************************************************************************/
class HYPERUI_API UIHistogram: public UIElement
{
public:
	DECLARE_STANDARD_UIELEMENT_NO_DESTRUCTOR_DEF(UIHistogram, UiElemHistogram);
	virtual void onDeallocated(void);

	virtual void postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	void clearDataSeries();

	void setShowChannels(int iChannels);
	void setShowForSingleChannel(HyperUI::ChannelType eChannel, bool bIsVisible);
	bool getIsChannelShown(int iChannels);

	virtual bool handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData);
	FLOAT_TYPE compressHistogramPixelCounts(const TRawHistDataEntries& rChannelIn, int iMaxBuckets, THistDataEntries& rChannelOut);
	virtual void updateOwnData(SUpdateInfo& rRefreshInfo);

	virtual FLOAT_TYPE getMaxRangeValue() const { return 255.0; }

	//boost::recursive_mutex& getLock() { return myHistogramDataLock; }
	Mutex& getLock() { return myHistogramDataLock; }

protected:

	void addDataSeries(const TRawHistDataEntries& rData, const SColor* scolOptColor = NULL, HyperUI::ChannelType eChannel = ChannelLastPlaceholder);
	void syncChannelVisibilityWithUIValues();

private:
	void recomputeDataSeriesGeometry();
	int generateLineTentKernel(int iKernelSize, TFloatTypeLossyDynamicArray& rKernelOut);

private:

	THistDataSeries myDataSeries;

	//boost::recursive_mutex myHistogramDataLock;
	Mutex myHistogramDataLock;

	TFloatTypeLossyDynamicArray *theSmoothingKernel;
	int thePrevKernelSize;
};
/*****************************************************************************/
