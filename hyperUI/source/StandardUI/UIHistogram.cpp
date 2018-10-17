#include "stdafx.h"

//#define TEST_DATA

#define RESAMPLING_DIVISOR		2.0
#define LINE_OPACITY			0.65  // 0.55

#define TOP_PADDING			upToScreen(20.0)
#define BOTTOM_PADDING		upToScreen(5.0)
#define LEFT_PADDING		upToScreen(5.0)
#define RIGHT_PADDING		upToScreen(0.0)

#define LINE_TICK_SIZE		upToScreen(2.0)
#define AXES_THICKNESS		upToScreen(0.51)

namespace HyperUI
{

const SColor g_colAxesColor(1.0, 1, 1, 0.40);

#define DRAW_OUTLINE

ColorPolyline2D SHistSDataSeries::thePolyline;
THistDataEntries SHistSDataSeries::theChannelMap;

const char* g_pcsChildNames[] = { "defHistR", "defHistG", "defHistB", "defHistRGB" };
HyperUI::ChannelType g_eChannelTypes[] = { ChannelRed, ChannelGreen, ChannelBlue, ChannelIntensity };
/*****************************************************************************/
UIHistogram::UIHistogram(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
UIHistogram::~UIHistogram()
{
	onDeallocated();
}
/*****************************************************************************/
void UIHistogram::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
	theSmoothingKernel = NULL;
	thePrevKernelSize = 0;
}
/*****************************************************************************/
void UIHistogram::onDeallocated(void)
{
	clearDataSeries();

	delete theSmoothingKernel;
	theSmoothingKernel = NULL;
}
/*****************************************************************************/
void UIHistogram::postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	SCOPED_MUTEX_LOCK(&myHistogramDataLock);
	//SCOPED_LOCK(myHistogramDataLock);

	SRect2D srOwnRect;
	getGlobalRectangle(srOwnRect);

#ifdef TEST_DATA
	static bool bDidSetData = false;
	if(!bDidSetData)
	{
		bDidSetData = true;
		
		// For now, really simple test
// 		TIntIntMap rTestMap;
// 		rTestMap[2] = 14;
// 		rTestMap[3] = 4;
// 		rTestMap[4] = 10;
// 		addDataSeries(rTestMap, &SColor::Red);

		Image image;
		image.read("photo.jpg");
		//image.read("grad_16bit.png");
		//setHistogramSource(image, ChannelRed | ChannelGreen | ChannelBlue);
		setHistogramSource(&image, false, ChannelIntensity);

	}
#endif

	int iNumVerts;
	SVertexInfo* pVertexInfo;
	int iCurr, iNum = myDataSeries.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(!myDataSeries[iCurr]->myIsVisible)
			continue;

		pVertexInfo = myDataSeries[iCurr]->getGeometry(this, srOwnRect, iNumVerts);
		if(!pVertexInfo)
			continue;

		// Render it!
		getDrawingCache()->addCustomObject2D(pVertexInfo, iNumVerts);

#ifdef DRAW_OUTLINE
		getDrawingCache()->flush();
		getDrawingCache()->addPathTrail(&myDataSeries[iCurr]->myLine, SVector2D::Zero, fOpacity, fScale, NULL, false );
#endif
	}

	// Draw the axes
	getDrawingCache()->addLine(srOwnRect.x + LINE_TICK_SIZE, srOwnRect.y + TOP_PADDING, srOwnRect.x + LINE_TICK_SIZE, srOwnRect.y + srOwnRect.h - BOTTOM_PADDING, g_colAxesColor, AXES_THICKNESS);
	getDrawingCache()->addLine(srOwnRect.x, srOwnRect.y + TOP_PADDING, srOwnRect.x + LINE_TICK_SIZE, srOwnRect.y + TOP_PADDING, g_colAxesColor, AXES_THICKNESS);
	getDrawingCache()->addLine(srOwnRect.x, srOwnRect.y + srOwnRect.h - BOTTOM_PADDING, srOwnRect.x + LINE_TICK_SIZE, srOwnRect.y + srOwnRect.h - BOTTOM_PADDING, g_colAxesColor, AXES_THICKNESS);

	getDrawingCache()->addLine(srOwnRect.x + LEFT_PADDING, srOwnRect.y + srOwnRect.h - LINE_TICK_SIZE, srOwnRect.x + srOwnRect.w - RIGHT_PADDING, srOwnRect.y + srOwnRect.h - LINE_TICK_SIZE, g_colAxesColor, AXES_THICKNESS);
	getDrawingCache()->addLine(srOwnRect.x + LEFT_PADDING, srOwnRect.y + srOwnRect.h, srOwnRect.x + LEFT_PADDING, srOwnRect.y + srOwnRect.h - LINE_TICK_SIZE, g_colAxesColor, AXES_THICKNESS);
	getDrawingCache()->addLine(srOwnRect.x + srOwnRect.w - RIGHT_PADDING, srOwnRect.y + srOwnRect.h, srOwnRect.x + srOwnRect.w - RIGHT_PADDING, srOwnRect.y + srOwnRect.h - LINE_TICK_SIZE, g_colAxesColor, AXES_THICKNESS);

	getDrawingCache()->flush();

#ifdef _DEBUG
	SVector2D svDbSize(srOwnRect.w, srOwnRect.h);
//	this->renderBBoxOfSize(svDbSize, SColor::White, false);
#endif

	UIElement::postRender(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
void UIHistogram::clearDataSeries()
{
	//SCOPED_LOCK(myHistogramDataLock);
	SCOPED_MUTEX_LOCK(&myHistogramDataLock);
	clearAndDeleteContainer(myDataSeries);
}
/*****************************************************************************/
void UIHistogram::addDataSeries(const TRawHistDataEntries& rData, const SColor* scolOptColor, HyperUI::ChannelType eChannel)
{
	//SCOPED_LOCK(myHistogramDataLock);
	SCOPED_MUTEX_LOCK(&myHistogramDataLock);

	SHistSDataSeries *pNew = new SHistSDataSeries;
	if(scolOptColor)
		pNew->myColor = *scolOptColor;
	else
		pNew->myColor = SColor::White;
	pNew->myData = rData;

	pNew->myLine.setThickness(upToScreen(0.51));
	pNew->myLine.setColor(pNew->myColor.r, pNew->myColor.g, pNew->myColor.b, LINE_OPACITY);
	//pNew->myLine.setColor(0,0,1,1);
	pNew->myLine.setConstOpacity(1.0);
	pNew->myLine.setFadeSpeed(0.0);

	pNew->myChannelType = eChannel;
	myDataSeries.push_back(pNew);
}
/*****************************************************************************/
void UIHistogram::recomputeDataSeriesGeometry()
{
	//SCOPED_LOCK(myHistogramDataLock);
	SCOPED_MUTEX_LOCK(&myHistogramDataLock);

	SRect2D srOwnRect;
	getGlobalRectangle(srOwnRect);

	int iCurr, iNum = myDataSeries.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		myDataSeries[iCurr]->recomputeGeometry(this, srOwnRect);
}
/*****************************************************************************/
void UIHistogram::setShowChannels(int iChannels)
{
	//SCOPED_LOCK(myHistogramDataLock);
	SCOPED_MUTEX_LOCK(&myHistogramDataLock);

	int iCurr, iNum = myDataSeries.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		myDataSeries[iCurr]->myIsVisible = ((myDataSeries[iCurr]->myChannelType & iChannels) != 0);

	UIButtonElement* pElem;
	for(iCurr = 0; iCurr < 4; iCurr++)
	{
		pElem = this->getChildById<UIButtonElement>(g_pcsChildNames[iCurr], true, true);
		if(!pElem)
			ASSERT_CONTINUE;
		pElem->setIsPushed( (g_eChannelTypes[iCurr] & iChannels) != 0 );
	}
}
/*****************************************************************************/
bool UIHistogram::handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData)
{
	bool bRes = false;
	if(strAction == "toggleHistogramChannel")
	{
		const char* pcsValue = pSourceElem->getStringProp(PropertyActionValue);
		HyperUI::ChannelType eChannel = mapStringToChannelType(pcsValue);
		UIButtonElement* pButton = as<UIButtonElement>(pSourceElem);
		if(pButton)
			setShowForSingleChannel(eChannel, pButton->getIsPushed());
		bRes = true;
	}
	return bRes;
}
/*****************************************************************************/
bool UIHistogram::getIsChannelShown(int iChannels)
{
	//SCOPED_LOCK(myHistogramDataLock);
	SCOPED_MUTEX_LOCK(&myHistogramDataLock);

	int iCurr, iNum = myDataSeries.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if((myDataSeries[iCurr]->myChannelType & iChannels) != 0)
			return myDataSeries[iChannels]->myIsVisible;
	}

	return false;
}
/*****************************************************************************/
void UIHistogram::setShowForSingleChannel(HyperUI::ChannelType eChannel, bool bIsVisible)
{
	//SCOPED_LOCK(myHistogramDataLock);
	SCOPED_MUTEX_LOCK(&myHistogramDataLock);

	int iCurr, iNum = myDataSeries.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(myDataSeries[iCurr]->myChannelType == eChannel)
			myDataSeries[iCurr]->myIsVisible = bIsVisible;
	}
}
/*****************************************************************************/
void UIHistogram::updateOwnData(SUpdateInfo& rRefreshInfo)
{
// 	gLog("Event type: %d\n", rRefreshInfo.mySourceEventType);
// 	if((rRefreshInfo.mySourceEventType != EventNodeParmsChanging && rRefreshInfo.mySourceEventType != EventUIParmChanging) && getTargetIdentifier())
// 		getTargetIdentifier()->refreshUIFromStoredValue(this);
}
/*****************************************************************************/
void UIHistogram::syncChannelVisibilityWithUIValues()
{

	UIButtonElement* pElem;
	int iCurr;
	
	for(iCurr = 0; iCurr < 4; iCurr++)
	{
		pElem = this->getChildById<UIButtonElement>(g_pcsChildNames[iCurr], true, true);
		if(!pElem)
			ASSERT_CONTINUE;
		setShowForSingleChannel(g_eChannelTypes[iCurr], pElem->getIsPushed());		
	}
}
/*****************************************************************************/
int UIHistogram::generateLineTentKernel(int iKernelSize, TFloatTypeLossyDynamicArray& rKernelOut)
{
	if(iKernelSize < 3)
		iKernelSize = 3;
	if(iKernelSize % 2 == 0)
		iKernelSize++;

	int iKernelElem;
	FLOAT_TYPE *fKernel = rKernelOut.getArray(iKernelSize);

	FLOAT_TYPE fTotalSum = 0;
	for(iKernelElem = 0; iKernelElem < (int)(iKernelSize/2); iKernelElem++)
	{
		fKernel[iKernelElem] = iKernelElem + 1;
		fKernel[iKernelSize - iKernelElem - 1] = iKernelElem + 1;

		fTotalSum += fKernel[iKernelElem]*2.0;
	}
	fKernel[(int)(iKernelSize/2)] = (int)(iKernelSize/2) + 1;
	fTotalSum += fKernel[(int)(iKernelSize/2)];
	for(iKernelElem = 0; iKernelElem < iKernelSize; iKernelElem++)
		fKernel[iKernelElem] /= fTotalSum;

	return iKernelSize;
}
/*****************************************************************************/
FLOAT_TYPE UIHistogram::compressHistogramPixelCounts(const TRawHistDataEntries& rChannelIn, int iMaxBuckets, THistDataEntries& rChannelOut)
{
	rChannelOut.clear();

	int iMaxKey = this->getMaxRangeValue();
	int iDivisor = 1.0;

	// Ensure positive - arbitrary number
	if(iMaxBuckets <= 0)
		iMaxBuckets = 256;

	if(iMaxBuckets < iMaxKey)
	{
		iDivisor = (int)((FLOAT_TYPE)iMaxKey / (FLOAT_TYPE)iMaxBuckets) + 1;
		if(iDivisor <= 0)
			iDivisor = 1.0;
		iMaxBuckets = (FLOAT_TYPE)iMaxKey/(FLOAT_TYPE)iDivisor + 1;
	}

	if(iMaxBuckets <= 0)
		iMaxBuckets = 256;

	rChannelOut.resize(iMaxBuckets);
	int iCounter;
 	for(iCounter = 0; iCounter < iMaxBuckets; iCounter++)
	{
		rChannelOut[iCounter] = 0;
// 		rChannelOut[iCounter].myValue = iCounter;
// 		rChannelOut[iCounter].myCount = 0;
	}

#ifdef _DEBUG
	{
		FILE *out = fopen("db_hist_raw.txt", "w");
		int iDbCurr, iDbNum = rChannelIn.size();
		for(iDbCurr = 0; iDbCurr < iDbNum; iDbCurr++)
			fprintf(out, "%d, %d\n", rChannelIn[iDbCurr].myValue, rChannelIn[iDbCurr].myCount);
		fclose(out);
	}
#endif

	TRawHistDataEntries::const_iterator mi;
	int iCount;
	int iNewKey;
	int iMinSubRange, iMaxSubRange;
	for(mi = rChannelIn.begin(), iCount = 0; mi != rChannelIn.end(); iCount++) //  mi++, iCount++)
	{
		iNewKey = mi->myValue/iDivisor;

		iMinSubRange = iNewKey*iDivisor;
		iMaxSubRange = (iNewKey + 1)*iDivisor;
		for( ; mi != rChannelIn.end() && mi->myValue < iMaxSubRange; mi++)
		{
//			_ASSERT(rChannelOut[iNewKey].myValue ==  iNewKey);
			//rChannelOut[iNewKey].myCount += mi->second;
			rChannelOut[iNewKey] += mi->myCount;
		}			
	}

	// First, get rid of all cases where we have one or two zeros 
	//THistDataEntries rTemp = rChannelOut;

	int iKernelSize = iMaxBuckets*0.03;
	if(!theSmoothingKernel)
		theSmoothingKernel = new TFloatTypeLossyDynamicArray;
	if(thePrevKernelSize != iKernelSize)
	{
		iKernelSize = generateLineTentKernel(iKernelSize, *theSmoothingKernel);
		thePrevKernelSize = iKernelSize;
	}
	/*
	int iKernelSize = iMaxBuckets*0.03;
	if(iKernelSize < 3)
		iKernelSize = 3;
	if(iKernelSize % 2 == 0)
		iKernelSize++;

	int iKernelElem;
	if(!theSmoothingKernel)
		theSmoothingKernel = new TFloatTypeLossyDynamicArray;

	FLOAT_TYPE *fKernel = theSmoothingKernel->getArray(iKernelSize);
	if(thePrevKernelSize != iKernelSize)
	{
		FLOAT_TYPE fTotalSum = 0;
		for(iKernelElem = 0; iKernelElem < (int)(iKernelSize/2); iKernelElem++)
		{
			fKernel[iKernelElem] = iKernelElem + 1;
			fKernel[iKernelSize - iKernelElem - 1] = iKernelElem + 1;

			fTotalSum += fKernel[iKernelElem]*2.0;
		}
		fKernel[(int)(iKernelSize/2)] = (int)(iKernelSize/2) + 1;
		fTotalSum += fKernel[(int)(iKernelSize/2)];
		for(iKernelElem = 0; iKernelElem < iKernelSize; iKernelElem++)
			fKernel[iKernelElem] /= fTotalSum;

		thePrevKernelSize = iKernelSize;
	}
	*/

	FLOAT_TYPE *fKernel = theSmoothingKernel->getArray(iKernelSize);
	int iKernelElem;
	FLOAT_TYPE fNewRes;
	THistDataEntries rTemp = rChannelOut;
	int iCurr, iNum = rChannelOut.size();
	int iSrcIndex;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		fNewRes = 0;
		for(iKernelElem = 0; iKernelElem < iKernelSize; iKernelElem++)
		{
			iSrcIndex = iCurr - (iKernelSize/2) + iKernelElem;
			if(iSrcIndex >= 0 && iSrcIndex < iNum)
				fNewRes += rTemp[iSrcIndex]*fKernel[iKernelElem];
		}

		rChannelOut[iCurr] = fNewRes;
	}

	/*
	// Do some cleanup so that we look better
	THistDataEntries rTemp = rChannelOut;
	int iCurr, iNum = rChannelOut.size();
	for(iCurr = 1; iCurr < iNum - 1; iCurr++)
		rChannelOut[iCurr] = rTemp[iCurr - 1]*0.3 + rTemp[iCurr]*0.4 + rTemp[iCurr + 1]*0.3;
	*/

#ifdef _DEBUG
	{
		FILE *out = fopen("db_hist_compressed.txt", "w");
		int iDbCurr, iDbNum = rChannelOut.size();
		for(iDbCurr = 0; iDbCurr < iDbNum; iDbCurr++)
			fprintf(out, "%d, %d\n", iDbCurr, rChannelOut[iDbCurr]);
		fclose(out);
	}
#endif


	return iDivisor;

/*
	// This works, but adjusts to the size of the map; this means
	// if we have just one color, the result will be wrong.
	int iMaxKey = rChannelIn.size();
	int iDivisor = 1.0;
	if(iMaxBuckets < iMaxKey)
	{
		iDivisor = (int)((FLOAT_TYPE)iMaxKey / (FLOAT_TYPE)iMaxBuckets) + 1;
		iMaxBuckets = (FLOAT_TYPE)iMaxKey/(FLOAT_TYPE)iDivisor + 1;
	}

//	for(int iCounter = 0; iCounter < iMaxBuckets; iCounter++)
//		rChannelOut[iCounter] = 0;

	TIntIntMap::const_iterator mi;
	int iCount;
	int iNewKey;
	for(mi = rChannelIn.begin(), iCount = 0; mi != rChannelIn.end(); ) //  mi++, iCount++)
	{
		iNewKey = mi->first/iDivisor;
		for(int iSubCount = 0; iSubCount < iDivisor && mi != rChannelIn.end(); iSubCount++, mi++, iCount++)
 			rChannelOut[iNewKey] += mi->second;
	}

	return iDivisor;
	*/

/*
	// Init the map
	int iCounter;
 	for(iCounter = 0; iCounter < iMaxBuckets; iCounter++)
 		rChannelOut[iCounter] = 0;

	int iMaxKey = iMaxBuckets;
	TIntIntMap::const_iterator mi = rChannelIn.end();
	if(rChannelIn.size() > 0)
	{
		mi--;
		iMaxKey = mi->first;
	}

	int iNewKey;
	for(mi = rChannelIn.begin(); mi != rChannelIn.end(); mi++)
	{
// 		iNewKey = (FLOAT_TYPE)mi->first/(FLOAT_TYPE)iMaxKey*iMaxBuckets;
// 		rChannelOut[iNewKey] += mi->second;
		rChannelOut[mi->first] = mi->second;
	}
	*/
	

	/*
	// Smooth out single gaps
	TIntIntMap::iterator mi2;
	TIntIntMap::iterator mi_prev;
	TIntIntMap::iterator mi_next;
	for(mi2 = rChannelOut.begin(); mi2 != rChannelOut.end(); mi2++)
	{
		if(mi2->second == 0 && mi2 != rChannelOut.begin())
		{
			mi_prev = mi2;
			mi_prev--;
			mi_next = mi2;
			mi_next++;
			if(mi_prev->second > 0 && mi_next != rChannelOut.end() && mi_next->second > 0)
			{
				// Interp
				mi2->second = (mi_prev->second + mi_next->second)/2.0;
			}
		}
	}
	*/
	//_ASSERT(rChannelOut.size() <= iMaxBuckets);
}
/*****************************************************************************/
void SHistSDataSeries::recomputeGeometry(UIHistogram* pHistogram, const SRect2D& srWindowRect)
{
	myCachedRect = srWindowRect;
	clear();
	if(myData.size() == 0)
		return;

	const FLOAT_TYPE fMaxRangeValue = pHistogram->getMaxRangeValue();

	SRect2D srActualGraphRect = srWindowRect;
	srActualGraphRect.y += TOP_PADDING;
	srActualGraphRect.h -= TOP_PADDING + BOTTOM_PADDING;
	srActualGraphRect.x += LEFT_PADDING;
	srActualGraphRect.w -= LEFT_PADDING + RIGHT_PADDING;

	FLOAT_TYPE fValueMult = pHistogram->compressHistogramPixelCounts(myData, srActualGraphRect.w/RESAMPLING_DIVISOR, theChannelMap);
	//theChannelMap = myData;

	FLOAT_TYPE fXStep = srActualGraphRect.w;
	int iNumResampled = theChannelMap.size();
	if(iNumResampled > 1)
		fXStep /= (FLOAT_TYPE)(iNumResampled - 1);

	// Now, triangulate the damn thing
	// For this, we need a vector of data
	SVector2D svCurrPoint;
	thePolyline.clear();
	//THistDataEntries::const_iterator mi;
	int iCurrIndex, iNumIndices = theChannelMap.size();
	int iCount = 0;

	int iMaxVal = 0;
	//for(mi = theChannelMap.begin(); mi != theChannelMap.end(); mi++)
	for(iCurrIndex = 0; iCurrIndex < iNumIndices; iCurrIndex++)
	{
		//if(mi->second > iMaxVal)
// 		if(mi->myCount > iMaxVal)
// 			iMaxVal = mi->myCount;
		if(theChannelMap[iCurrIndex] > iMaxVal)
			iMaxVal = theChannelMap[iCurrIndex];
	}

	myLine.clear();

	SColor scolBottom = myColor;
	scolBottom.alpha = 0;

	SVector2D svNextPoint;
	SColorPoint rTempPoint;
// 	THistDataEntries::const_iterator mi2 = theChannelMap.end();
// 	THistDataEntries::const_iterator mi_next;
	int iEndIndex = iNumIndices;
	if(iNumResampled > 0)
	{
		//mi2--;
		iEndIndex--;
		int iNumTris = (iNumResampled - 1)*2;
		myNumVerts = iNumTris*3;
		myVertexInfo = new SVertexInfo[myNumVerts];
	}

	FLOAT_TYPE fYBase = srActualGraphRect.y + srActualGraphRect.h;

	int iNextIndex;
	int iArrayOffset = 0;
	//for(mi = theChannelMap.begin(), iCount = 0; mi != mi2; mi++, iCount++)
	for(iCurrIndex = 0; iCurrIndex < iEndIndex; iCurrIndex++)
	{
		iNextIndex = iCurrIndex + 1;
// 		mi_next = mi;
// 		mi_next++;

		//svCurrPoint.x = fXStep*iCount + srActualGraphRect.x;
		svCurrPoint.x = (iCurrIndex*fValueMult/fMaxRangeValue)*srActualGraphRect.w + srActualGraphRect.x;
		svCurrPoint.y = srActualGraphRect.y + srActualGraphRect.h - srActualGraphRect.h*((FLOAT_TYPE)theChannelMap[iCurrIndex]/(FLOAT_TYPE)iMaxVal); //  - upToScreen(0.5);

		//svNextPoint.x = fXStep*(iCount + 1) + srActualGraphRect.x;
		svNextPoint.x = (iNextIndex*fValueMult/fMaxRangeValue)*srActualGraphRect.w + srActualGraphRect.x;
		svNextPoint.y = srActualGraphRect.y + srActualGraphRect.h - srActualGraphRect.h*((FLOAT_TYPE)theChannelMap[iNextIndex]/(FLOAT_TYPE)iMaxVal); //  - upToScreen(0.5);		
		
		rTempPoint.setPoint(svCurrPoint.x, fYBase, scolBottom);
		myVertexInfo[iArrayOffset].copyFrom(rTempPoint);
		rTempPoint.setPoint(svCurrPoint.x, svCurrPoint.y, myColor);
		myVertexInfo[iArrayOffset + 1].copyFrom(rTempPoint);
		rTempPoint.setPoint(svNextPoint.x, svNextPoint.y, myColor);
		myVertexInfo[iArrayOffset + 2].copyFrom(rTempPoint);
		iArrayOffset += 3;

		rTempPoint.setPoint(svCurrPoint.x, fYBase, scolBottom);
		myVertexInfo[iArrayOffset].copyFrom(rTempPoint);
		rTempPoint.setPoint(svNextPoint.x, svNextPoint.y, myColor);
		myVertexInfo[iArrayOffset + 1].copyFrom(rTempPoint);
		rTempPoint.setPoint(svNextPoint.x, fYBase, scolBottom);
		myVertexInfo[iArrayOffset + 2].copyFrom(rTempPoint);
		iArrayOffset += 3;


// 		rTempPoint.setPoint(svCurrPoint.x, svCurrPoint.y, myColor);
// 		thePolyline.push_back(rTempPoint);

		myLine.addPoint(svCurrPoint.x + 0.5, svCurrPoint.y + 0.5, 0, true);
	}

	// Now, push back the other two vertices that form the bottom
/*
	rTempPoint.setPoint(srActualGraphRect.x + srActualGraphRect.w, srActualGraphRect.y + srActualGraphRect.h, scolBottom);
	thePolyline.push_back(rTempPoint);
	rTempPoint.setPoint(srActualGraphRect.x, srActualGraphRect.y + srActualGraphRect.h, scolBottom);
	thePolyline.push_back(rTempPoint);

	myVertexInfo = thePolyline.triangulateAsPolygon(myNumVerts);
*/
}
/*****************************************************************************/
SVertexInfo* SHistSDataSeries::getGeometry(UIHistogram* pHistogram, const SRect2D& srWindowRect, int& iNumVertsOut)
{
	if(!myVertexInfo || myCachedRect != srWindowRect)
		recomputeGeometry(pHistogram, srWindowRect);
	iNumVertsOut = myNumVerts;
	return myVertexInfo;
}
/*****************************************************************************/
};