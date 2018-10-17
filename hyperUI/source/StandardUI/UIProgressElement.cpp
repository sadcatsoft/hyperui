#include "stdafx.h"

namespace HyperUI
{
#ifndef MAC_BUILD
string UIProgressElement::theSharedString;
string UIProgressElement::theSharedString2;
#endif
/*****************************************************************************/
UIProgressElement::UIProgressElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIProgressElement::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);

	myMinProgress = 0.0;
	myMaxProgress = 1.0;
	myCurrProgress = 0.0;
	myIsInverted = false;

	myDirection = ProgressBarStyleFromLeft;

//myMinProgress = 0.0;
//myMaxProgress = 100.0;
//myCurrProgress = 53.3;
}
/*****************************************************************************/
void UIProgressElement::postInit(void)
{
	UIElement::postInit();

	// See what kind we are
	if(this->doesPropertyExist(PropertyDirection))
		myDirection = mapStringToType<ProgressBarStyleType>(this->getStringProp(PropertyDirection), g_pcsBarStyleStrings, ProgressBarStyleFromLeft);
}
/*****************************************************************************/
void UIProgressElement::setMinProgress(FLOAT_TYPE fValue)
{
	myMinProgress = fValue;
}
/*****************************************************************************/
void UIProgressElement::setMaxProgress(FLOAT_TYPE fValue)
{
	myMaxProgress = fValue;
}
/*****************************************************************************/
void UIProgressElement::setProgress(FLOAT_TYPE fValue)
{
	myCurrProgress = fValue;
	ActivityManager::getInstance()->singleValueChanged();
}
/*****************************************************************************/
void UIProgressElement::render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	preRender(svScroll, fOpacity, fScale);

	FLOAT_TYPE fPercProgress = 0.0;
	if(fabs(myMaxProgress - myMinProgress) > FLOAT_EPSILON)
	{
		FLOAT_TYPE fCurrProg = myCurrProgress;
// 		if(myProgressDataSource.getIsValid())
// 			fCurrProg = myProgressDataSource.getAsNumber();
		fPercProgress = (fCurrProg - myMinProgress)/(myMaxProgress - myMinProgress);
	}
	if(fPercProgress < 0.0)
		fPercProgress = 0;
	if(fPercProgress > 1.0)
		fPercProgress = 1.0;

	if(myIsInverted)
		fPercProgress = 1.0 - fPercProgress;

	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity;

	if(this->getParent())
		svPos *= fScale;

	theSharedString2 = this->getStringProp(PropertySecondaryImage);
	//theSharedString = this->getStringProp(PropertyObjAnim);
	this->getFullTopAnimName(theSharedString);
	SVector2D svCenter(svPos.x + svScroll.x, svPos.y + svScroll.y);
	int iRealW = RenderUtils::renderProgressBar(getParentWindow(), theSharedString2, theSharedString, fPercProgress, svCenter, fFinalOpac, fScale*fLocScale, myDirection);

	/*
	// Empty bitmap
	theSharedString2 = this->getStringProp(PropertyObjBaseAnim);
	g_pDrawingCache->addSprite(theSharedString2.c_str(), svPos.x + svScroll.x, svPos.y + svScroll.y,
		fFinalOpac, 0, fScale, 0, true);

	// Full bitmap
	SUVSet rUVs;
	theSharedString = this->getStringProp(PropertyObjAnim);

	int iBitmapW, iBitmapH;
	int iRealW, iRealH;
	g_pDrawingCache->getImageSize(theSharedString.c_str(), iBitmapW, iBitmapH);
	g_pTextureManager->getTextureRealDims(theSharedString.c_str(), iRealW, iRealH);

	// Now that we know all the dimensions, figure our how much to scale by...
	FLOAT_TYPE fUVActualImageSize = (FLOAT_TYPE)iRealW/(FLOAT_TYPE)iBitmapW;
	FLOAT_TYPE fUVEmptyHalf = (1.0 - fUVActualImageSize)/2.0;
	rUVs.myStartX = fUVEmptyHalf;
	rUVs.myEndX = fUVEmptyHalf + fUVActualImageSize*fPercProgress;

	// Now we need to figure out the x scale
	FLOAT_TYPE fScaleX = ((FLOAT_TYPE)iRealW/(FLOAT_TYPE)iBitmapW)*fPercProgress;
	FLOAT_TYPE fExtraXOffset = (1.0 - fPercProgress)*(FLOAT_TYPE)iRealW/2.0;
	
	SVector2D svFinalPos;
	svFinalPos.x = svPos.x + svScroll.x - fExtraXOffset;
	svFinalPos.y = svPos.y + svScroll.y;
	g_pDrawingCache->addSprite(theSharedString.c_str(), svFinalPos.x, svFinalPos.y,
		fFinalOpac, 0, fScaleX, fScale, 0, true, &rUVs);
*/
	// Now, render the text next to it. This is beautiful.
	if(this->doesPropertyExist(PropertyText))
	{
		SVector2D svTextScroll;
		svTextScroll = svPos + svScroll;

		// Add a bit of offset.
		svTextScroll.x += (iRealW/2.0 + upToScreen(8))*fScale*fLocScale; // + 28;

		// Subtract our own pos since text drawing code
		// adds it back.
		svTextScroll -= svPos;

		// Note: we need to override the text since it's really a format
		// specifier here.
		char pcsTempBuff[64];
		this->getTextAsString(theSharedString);
		if(theSharedString.find("%d") != string::npos)
		{
			// We're doing integers.
			int iTemp = myCurrProgress;
			sprintf(pcsTempBuff, theSharedString.c_str(), iTemp);
		}
		else
			sprintf(pcsTempBuff, theSharedString.c_str(), myCurrProgress);
		theSharedString = pcsTempBuff;
		this->renderTextInternalFinal(theSharedString, svTextScroll, fFinalOpac, fScale, false);
	}
	postRender(svScroll, fOpacity, fScale);
}
/*****************************************************************************
void UIProgressElement::setProgressDataSource(DataSource& rSource)
{
	myProgressDataSource = rSource;
}
/*****************************************************************************/
void UIProgressElement::setIsInverted(bool bValue)
{
	myIsInverted = bValue;
}
/*****************************************************************************/
};