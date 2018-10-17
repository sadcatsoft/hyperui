/*****************************************************************************

Disclaimer: This software is supplied to you by Sad Cat Software
("Sad Cat") in consideration of your agreement to the following terms, and 
your use, installation, modification or redistribution of this Sad Cat software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Sad Cat software.

This software is provided "as is". Sad Cat Software makes no warranties, 
express or implied, including without limitation the implied warranties
of non-infringement, merchantability and fitness for a particular
purpose, regarding Sad Cat's software or its use and operation alone
or in combination with other hardware or software products.

In no event shall Sad Cat Software be liable for any special, indirect,
incidental, or consequential damages (including, but not limited to, 
procurement of substitute goods or services; loss of use, data, or profits;
or business interruption) arising in any way out of the use, reproduction,
modification and/or distribution of Sad Cat's software however caused and
whether under theory of contract, tort (including negligence), strict
liability or otherwise, even if Sad Cat Software has been advised of the
possibility of such damage.

Copyright (C) 2012, Sad Cat Software. All Rights Reserved.

*****************************************************************************/
#include "stdafx.h"

#define PARM_STRING_BEGIN_MARKER				"___((("
#define PARM_STRING_END_MARKER					")))___"
#define PARM_STRING_TEMP_TOKEN_PREFIX			"____tempToken____"

namespace HyperUI
{

TCharPtrVector UIRichTextElement::theTokens;
TCharPtrVector UIRichTextElement::theInnerTokens;

/*****************************************************************************/
UIRichTextElement::UIRichTextElement(UIPlane* pParentPlane)
	: UIZoomWindowElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIRichTextElement::onAllocated(IBaseObject* pData)
{
	UIZoomWindowElement::onAllocated(pData);
	myAreObjectsDirty = true;
}
/*****************************************************************************/
UIRichTextElement::~UIRichTextElement() 
{ 
	onDeallocated();  
}
/*****************************************************************************/
void UIRichTextElement::onDeallocated(void)
{
	clearAndDeleteContainer(myObjects);
}
/*****************************************************************************/
void UIRichTextElement::postInit(void)
{
	UIZoomWindowElement::postInit();
	// Do not do this! First, because this gets run for all elems on startup.
	// Seconds, because of the above, it gets run when UI plane, and its allocator,
	// have not been finished constructing, and if any custom elems need to be allocated,
	// it will crash.
// 	if(this->doesPropertyExist(PropertyUiObjText))
	// 		parseRichText(this->getUiText(), myObjects);
//	myAreObjectsDirty = false;
	myAreObjectsDirty = true;
}
/*****************************************************************************/
void UIRichTextElement::updateOwnData(SUpdateInfo& rRefreshInfo) 
{
	if(myAreObjectsDirty && this->doesPropertyExist(PropertyText))
	{
		string strTemp;
		this->getTextAsString(strTemp);
		parseRichText(strTemp.c_str(), myObjects);
	}
	UIZoomWindowElement::updateOwnData(rRefreshInfo);
}
/*****************************************************************************/
void UIRichTextElement::getContentSize(SVector2D& svOut)
{
//	_ASSERT(!myAreObjectsDirty);
	if(myAreObjectsDirty && this->doesPropertyExist(PropertyText))
	{
		string strTemp;
		this->getTextAsString(theCommonString3);
		parseRichText(strTemp.c_str(), myObjects);
	}

	this->getBoxSize(svOut);
	svOut.y = myTotalHeight;
}
/*****************************************************************************/
void UIRichTextElement::renderZoomedContents(const SRect2D& srWindowRect, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	SVector2D svScreenZero;
	windowToScreen(svScreenZero, svScreenZero);

	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	getGlobalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity;
	SVector2D svSize;
	this->getBoxSize(svSize);
	svSize *= fLocScale*fScale;

	SVector2D svStartPos;
	svStartPos = svPos - svSize*0.5;;
	svStartPos.y += svScreenZero.y;

	FLOAT_TYPE fTextWidth = this->getTextWidth();

	SVector2D svShadowOffset;
	getCachedShadowOffset(svShadowOffset);

	SColor scolSelCol(-1,-1,-1);
	SColor scolText;
	if(this->doesPropertyExist(PropertySelectedTextColor))
		this->getAsColor(PropertySelectedTextColor, scolSelCol);
	this->getAsColor(PropertyTextColor, scolText);
	if(scolSelCol.r < 0)
		scolSelCol = scolText;

	const char* pcsFont = getCachedFont();
	int iFontSize = getCachedFontSize();

	SColor *pShadowColor = NULL;
	SColor scolActualShadowColor(-1, -1, -1, -1);
	if(this->doesPropertyExist(PropertyTextShadowColor))
		this->getAsColor(PropertyTextShadowColor, scolActualShadowColor);
	if(scolActualShadowColor.r >= 0)
	{
		scolActualShadowColor.alpha *= fFinalOpac;
		pShadowColor = &scolActualShadowColor;
	}

	scolText.alpha *= fFinalOpac;
	scolSelCol.alpha *= fFinalOpac;

	// Now, render the text and other elements that we have
	SVector2D svTempElemPos;
	SRichObject* pObj;
	int iObj, iNumObjects = myObjects.size();
	for(iObj = 0; iObj < iNumObjects; iObj++)
	{
		pObj = myObjects[iObj];

		if(pObj->myType == RichObjectText)
		{
			getDrawingCache()->addOrMeasureText(pObj->myString.c_str(), pcsFont, iFontSize, pObj->myPos.x + svStartPos.x, pObj->myPos.y + svStartPos.y, scolText, HorAlignLeft, VertAlignTop, 0, NULL, pShadowColor, &svShadowOffset, fTextWidth, false, &scolSelCol);
		}
		else if(pObj->myType == RichObjectUserElem)
		{
			if(pObj->myElem->getIsVisible())
			{
				svTempElemPos.set(pObj->myPos.x + svStartPos.x, pObj->myPos.y + svStartPos.y);
				pObj->myElem->render(svTempElemPos, fFinalOpac, fScale);
			}
		}
		ELSE_ASSERT;
	}
}
/*****************************************************************************
void UIRichTextElement::render(SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	// Pre-render
	preRender(svScroll, fOpacity, fScale);	


	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	getGlobalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity;
	SVector2D svSize;
	this->getBoxSize(svSize);
	svSize *= fLocScale*fScale;

	SVector2D svStartPos;
	svStartPos = svPos - svSize*0.5;
	
	FLOAT_TYPE fTextWidth = this->getTextWidth();

	SVector2D svShadowOffset;
	getShadowOffset(svShadowOffset);

	SColor scolSelCol(-1,-1,-1);
	SColor scolText;
	if(this->doesPropertyExist(PropertyUiObjTextSelColor))
		this->getAsColor(PropertyUiObjTextSelColor, scolSelCol);
	this->getAsColor(PropertyUiObjTextColor, scolText);
	if(scolSelCol.r < 0)
		scolSelCol = scolText;

	const char* pcsFont = getCachedFont();

	SColor *pShadowColor = NULL;
	SColor scolActualShadowColor(-1, -1, -1, -1);
	if(this->doesPropertyExist(PropertyUiObjTextShadowColor))
		this->getAsColor(PropertyUiObjTextShadowColor, scolActualShadowColor);
	if(scolActualShadowColor.r >= 0)
	{
		scolActualShadowColor.alpha *= fFinalOpac;
		pShadowColor = &scolActualShadowColor;
	}

	scolText.alpha *= fFinalOpac;
	scolSelCol.alpha *= fFinalOpac;

	// Now, render the text and other elements that we have
	SRichObject* pObj;
	int iObj, iNumObjects = myObjects.size();
	for(iObj = 0; iObj < iNumObjects; iObj++)
	{
		pObj = &myObjects[iObj];

		if(pObj->myType == RichObjectText)
		{
			getDrawingCache()->addOrMeasureText(pObj->myString.c_str(), pcsFont, pObj->myPos.x + svStartPos.x, pObj->myPos.y + svStartPos.y, scolText, 
				HorAlignLeft, VertAlignTop, 0, NULL, pShadowColor, &svShadowOffset, fTextWidth, false, &scolSelCol);
		}
		ELSE_ASSERT;
	}

	// Post-render
	renderChildren(svScroll, fOpacity, fScale);
	postRender(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
void UIRichTextElement::setText(const char* pcsText)
{
	UIZoomWindowElement::setText(pcsText);
	parseRichText(pcsText, myObjects);
}
/*****************************************************************************/
void UIRichTextElement::parseRichText(const char* pcsText, TRichObjects& rVecOut)
{
	theCommonString = pcsText;
	theCommonString2 = "\n^";
	theTokens.clear();

	// Since multiple line breaks in sequence would be thrown out by the tokenizer,
	// insert our own for any occurence past the first one
	int iChar, iLen = theCommonString.size();
	bool bWasPrevLineBreak = false;
	for(iChar = 0; iChar < iLen; iChar++)
	{
		if(theCommonString[iChar] != '^' && theCommonString[iChar] != '\n')
		{
			bWasPrevLineBreak = false;
			continue;
		}

		if(!bWasPrevLineBreak)
			bWasPrevLineBreak = true;
		else
		{
			// Replace this one
			theCommonString = theCommonString.replace(iChar, 1, RTE_LINE_BREAK "\n");
			iChar += strlen(RTE_LINE_BREAK "\n") - 1;
		}
	}


	// Now tokenize
	TokenizeUtils::tokenizeStringToCharPtrsInPlace(theCommonString, theCommonString2, theTokens);

	clearAndDeleteContainer(myObjects);

	//FLOAT_TYPE fWidth = this->getNumProp(PropertyUiObjTextWidth);
	FLOAT_TYPE fWidth = this->getTextWidth();
	const char* pcsFont = this->getCachedFont();
	int iFontSize = getCachedFontSize();

	FLOAT_TYPE fLineHeight = upToScreen(5.0);
#ifdef USE_FREETYPE_FONTS
	const CachedFontInfo* pFontInfo = getParentWindow()->getFontManager()->getFontInfo(pcsFont);
	if(pFontInfo)
		fLineHeight = pFontInfo->getLineHeight(iFontSize) + pFontInfo->getExtraLineSpacing(iFontSize);
#else
	ResourceItem* pFont = g_pMainEngine->findResourceItem(ResourceRasterFonts, pcsFont);
	if(pFont)
		fLineHeight = pFont->getNumProp(PropertyExtraLineSpacing) + pFont->getNumProp(PropertyFtHeight);
#endif
	UIAllocator* pAllocator = getUIPlane()->getAllocator();

	// So now, go over every paragraph, and estimate its size:
	FLOAT_TYPE fTextLeftPadding = this->getNumProp(PropertyRtPaddingLeft);
	FLOAT_TYPE fCumulXLeftPos = 0;
	FLOAT_TYPE fCumulYTopPos = 0;
	SVector2D svDims;
	const char* pcsToken;
	const char* pcsElemIdPtr;
	char* pcsFirstSpace;
	char* pcsClosingBrace;
	UIElement* pResElem;
	ResourceCollection* pUIColl = NULL;
	ResourceItem* pInlineUIElemDef;
	ResourceItem rTempPropsItem;
	string strTempMapKey;
	ResourceType eCollType = this->getUIPlane()->getInitCollectionType();
	TStringStringMap mapMultilineTokens;
	int iToken, iNumTokens = theTokens.size();
	FLOAT_TYPE fCurrLineMaxHeight = 0;
	Window* pWindow = getParentWindow();
	for(iToken = 0; iToken < iNumTokens; iToken++)
	{
		pcsToken = theTokens[iToken];

#ifdef _DEBUG
		if(strstr(pcsToken, "richTextHeader"))
		{
			int bp = 0;
		}
#endif

		// See if it's a token we recognize
		if(IS_STRING_EQUAL(pcsToken, RTE_LINE_BREAK))
		{
			fCumulYTopPos += fCurrLineMaxHeight;
			fCurrLineMaxHeight = 0;
			fCumulYTopPos += fLineHeight;
			fCumulXLeftPos = 0;
			continue;
		}

		pInlineUIElemDef = NULL;
		pcsElemIdPtr = strstr(pcsToken, "{");
		if(pcsElemIdPtr)
		{
			pcsClosingBrace = const_cast<char*>(strstr(pcsElemIdPtr, "}"));
			if(pcsClosingBrace)
			{
				pcsFirstSpace = const_cast<char*>(strstr(pcsElemIdPtr, " "));
				if(pcsFirstSpace && pcsFirstSpace < pcsClosingBrace)
					(*pcsFirstSpace) = 0;
				else
				{
					pcsFirstSpace = NULL;
					(*pcsClosingBrace) = 0;
				}
					
				pcsElemIdPtr++;
				// This might be an object. See if we can find it.
				pUIColl = ResourceManager::getInstance()->getCollection(eCollType);
				pInlineUIElemDef = pUIColl->getItemById(pcsElemIdPtr);

				if(pInlineUIElemDef && pcsFirstSpace)
				{
					// Tokenize the rest
					theInnerTokens.clear();
					theCommonString3 = pcsFirstSpace + 1;
					theCommonString2 = " =}";

					// Now, we support the multineline props here... Replace each multiline prop with a token,
					// tokenize it, then map it back:
					mapMultilineTokens.clear();
					int iPos, iEndPos, iCount;
					for(iCount = 0, iPos = theCommonString3.find(PARM_STRING_BEGIN_MARKER); iPos != string::npos; iPos = theCommonString3.find(PARM_STRING_BEGIN_MARKER, iPos), iCount++)
					{
						StringUtils::numberToString(iCount, strTempMapKey);
						strTempMapKey = PARM_STRING_TEMP_TOKEN_PREFIX + strTempMapKey;

						iEndPos = theCommonString3.find(PARM_STRING_END_MARKER);
						mapMultilineTokens[strTempMapKey] = theCommonString3.substr(iPos + strlen(PARM_STRING_BEGIN_MARKER), iEndPos - iPos - strlen(PARM_STRING_BEGIN_MARKER));

						theCommonString3 = theCommonString3.replace(iPos, iEndPos + strlen(PARM_STRING_END_MARKER) - iPos, strTempMapKey);

						iPos = 0;
					}

					TokenizeUtils::tokenizeStringToCharPtrsInPlace(theCommonString3, theCommonString2, theInnerTokens);

					// Parse properties onto the element
					copyTokenPropsToResourceItem(theInnerTokens, mapMultilineTokens, rTempPropsItem);
				}
			}
		}

		if(pInlineUIElemDef)
		{
			// We have a custom element. What we need to do 
			// is create an instance of it as an element, add it
			// to our object, copy any props onto it.
			pResElem = pUIColl->createItemOfType<UIElement>(pInlineUIElemDef->getStringProp(PropertyId), getUIPlane(), NULL, pAllocator);
			pResElem->mergeResourcesFrom(rTempPropsItem, true);
			pResElem->resetEvalCache(true);
			pResElem->show(true);
			pResElem->setParent(this);

			// Override the layer so it's not none, otherwise we won't render
			pResElem->setStringProp(PropertyLayer, "___richTextLayer___");

			pResElem->getBoxSize(svDims);
			if((int)svDims.x == 0 && (int)svDims.y == 0 && pResElem->doesPropertyExist(PropertyText))
			{
				// Attempt to measure its text
				pResElem->getTextAsString(theCommonString3);
				RenderUtils::measureText(pWindow, theCommonString3.c_str(), pResElem->getCachedFont(), pResElem->getCachedFontSize(), pResElem->getTextWidth(), svDims);
				// Set the x to zero, so we don't depend on the length of text for our position
				svDims.x = 0;
			}

			SRichObject *rObj = new SRichObject;

			if(rTempPropsItem.doesPropertyExist(PropertyRtPaddingLeft))
			{
				if(rTempPropsItem.getPropertyDataType(PropertyRtPaddingLeft) == PropertyDataNumber)
					fCumulXLeftPos += rTempPropsItem.getNumProp(PropertyRtPaddingLeft);
				else
					fCumulXLeftPos += convertUPNumberToScreenPixels(rTempPropsItem.getStringProp(PropertyRtPaddingLeft));
			}
			rObj->myPos.set(fCumulXLeftPos + svDims.x/2.0, fCumulYTopPos + svDims.y/2.0);
			fCumulXLeftPos += svDims.x;

			if(svDims.y > fCurrLineMaxHeight)
				fCurrLineMaxHeight = svDims.y;

			rObj->myType = RichObjectUserElem;
			rObj->myElem = pResElem;

			myObjects.push_back(rObj);
		}
		else
		{
			// It's plain text

			SRichObject *rObj = new SRichObject;
			rObj->myType = RichObjectText;
			rObj->myString = pcsToken;

			// Get its size
			// Note that if we use the rough estimate, we may get some bullet points spaced
			// with an extra newline. But not others.
			RenderUtils::measureText(pWindow, pcsToken, pcsFont, iFontSize, fWidth, svDims);

			rObj->myPos.set(fCumulXLeftPos + fTextLeftPadding, fCumulYTopPos);
			fCumulYTopPos += svDims.y;

			myObjects.push_back(rObj);
		}

		// Do not add anything here, continue is above!
	}

	myTotalHeight = fCumulYTopPos + fCurrLineMaxHeight; 
	myAreObjectsDirty = false;

	updateRelatedSlider();
}
/*****************************************************************************/
void UIRichTextElement::copyTokenPropsToResourceItem(TCharPtrVector& rTokens, TStringStringMap& mapTempTokens, ResourceItem& rPropsOut)
{
	int iCurr, iNum = rTokens.size();
	_ASSERT(iNum % 2 == 0);
	rPropsOut.clearEverything();

	TStringStringMap::iterator mi;
	PropertyType eProp;
	PropertyMapper* pPropertyMapper = PropertyMapper::getInstance();
	for(iCurr = 0; iCurr < iNum; iCurr += 2)
	{
		theCommonString2 = rTokens[iCurr];
		eProp = pPropertyMapper->mapProperty(theCommonString2);
		if(eProp == PropertyNull)
			continue;

		// First, see if we can find this in our map
		mi = mapTempTokens.find(rTokens[iCurr + 1]);
		if(mi == mapTempTokens.end())
			rPropsOut.setFromString(eProp, rTokens[iCurr + 1]);
		else
			rPropsOut.setFromString(eProp, mi->second.c_str());
		//rPropsOut.setStringProp(eProp, rTokens[iCurr + 1]);
	}

}
/*****************************************************************************/
UIElement* UIRichTextElement::getObjectAt(FLOAT_TYPE fScreenX, FLOAT_TYPE fScreenY)
{
	SVector2D svScreenZero;
	windowToScreen(svScreenZero, svScreenZero);
	SVector2D svPos;
	FLOAT_TYPE fLocScale;
	getGlobalPosition(svPos, NULL, &fLocScale);
	SVector2D svSize;
	this->getBoxSize(svSize);
	svSize *= fLocScale;
	SVector2D svStartPos;
	svStartPos = svPos - svSize*0.5;;
	svStartPos.y += svScreenZero.y;

	SVector2D svLocalCenter;
	SVector2D svLocalSize;
	SVector2D svTestPoint(fScreenX, fScreenY);
	UIElement* pElem;
	SRect2D srBox;
	int iObj, iNumObj = myObjects.size();
	for(iObj = 0; iObj < iNumObj; iObj++)
	{
		pElem = myObjects[iObj]->myElem;
		if(!pElem)
			continue;

		if(!pElem->getIsVisible())
			continue;

		pElem->getLocalPosition(svLocalCenter);
		pElem->getBoxSize(svLocalSize);
		srBox.x = svLocalCenter.x - svLocalSize.x/2.0;
		srBox.y = svLocalCenter.y - svLocalSize.y/2.0;
		srBox.w = svLocalSize.x;
		srBox.h = svLocalSize.y;	

		//pElem->getCurrBBox(srBox);
		srBox.x += myObjects[iObj]->myPos.x + svStartPos.x;
		srBox.y += myObjects[iObj]->myPos.y + svStartPos.y;

		if(srBox.doesContain(svTestPoint))
			return pElem;
	}

	return NULL;
}
/*****************************************************************************/
UIElement* UIRichTextElement::getChildAtRecursive(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bFirstNonPassthrough, bool bGlidingOnly)
{
	UIElement* pRes = getObjectAt(fX, fY);
	if(pRes)
		return pRes;
	else
		return UIElement::getChildAtRecursive(fX, fY, bFirstNonPassthrough, bGlidingOnly);
}
/*****************************************************************************/
UIElement* UIRichTextElement::getChildAtRecursiveInternal(const SVector2D& svPoint, const SVector2D& svScroll, bool bFirstNonPassthrough, bool bIgnoreChildren, bool bGlidingOnly)
{
	UIElement* pRes = getObjectAt(svPoint.x, svPoint.y);
	if(pRes)
		return pRes;
	else
		return UIElement::getChildAtRecursiveInternal(svPoint, svScroll, bFirstNonPassthrough, bIgnoreChildren, bGlidingOnly);
}
/*****************************************************************************/
UIElement* UIRichTextElement::getObjectById(const char* pcsId)
{
	UIElement* pElem;
	int iObj, iNumObj = myObjects.size();
	for(iObj = 0; iObj < iNumObj; iObj++)
	{
		pElem = myObjects[iObj]->myElem;
		if(!pElem || !pElem->doesPropertyExist(PropertyId))
			continue;
		if(IS_STRING_EQUAL(pElem->getStringProp(PropertyId), pcsId))
			return pElem;
	}

	return NULL;
}
/*****************************************************************************/
void UIRichTextElement::hideObjectGroup(const char* pcsGroupId)
{
	UIElement* pElem;
	int iObj, iNumObj = myObjects.size();
	for(iObj = 0; iObj < iNumObj; iObj++)
	{
		pElem = myObjects[iObj]->myElem;
		if(!pElem || !pElem->doesPropertyExist(PropertyRtGroupId))
			continue;
		if(IS_STRING_EQUAL(pElem->getStringProp(PropertyRtGroupId), pcsGroupId))
		{
			pElem->setIsVisible(false);
		}
	}
}
/*****************************************************************************/
UIElement* UIRichTextElement::setTextForChild(const char* pcsChildId, const char* pcsText, SColor* pOptColor)
{
	UIElement* pOwnElem = getObjectById(pcsChildId);
	if(pOwnElem)
	{
		if(IS_STRING_EQUAL(pcsText, PROPERTY_NONE))
			pcsText = "";
		pOwnElem->setText(pcsText);
		if(pOptColor)
			pOwnElem->setAsColor(PropertyTextColor, *pOptColor);

		return pOwnElem;
	}
	else
		return UIZoomWindowElement::setTextForChild(pcsChildId, pcsText, pOptColor);
}
/*****************************************************************************/
UIElement* UIRichTextElement::setTextForChild(const char* pcsChildId, string& strText, SColor* pOptColor, FLOAT_TYPE fShortenTextMaxLen)
{
	UIElement* pOwnElem = getObjectById(pcsChildId);
	if(pOwnElem)
	{
		if(strText == PROPERTY_NONE)
			strText = "";
		if(fShortenTextMaxLen > 0)
			RenderUtils::shortenStringFromTheEnd(getParentWindow(), strText, pOwnElem->getCachedFont(), pOwnElem->getCachedFontSize(), fShortenTextMaxLen, "...", false);

		pOwnElem->setText(strText.c_str());
		if(pOptColor)
			pOwnElem->setAsColor(PropertyTextColor, *pOptColor);
		return pOwnElem;
	}
	else
		return UIZoomWindowElement::setTextForChild(pcsChildId, strText, pOptColor, fShortenTextMaxLen);

}
/*****************************************************************************/
UIElement* UIRichTextElement::setTextForChild(const char* pcsChildId, const char* pcsSubChildId, const char* pcsText, SColor* pOptColor)
{
	UIElement* pElem = getObjectById(pcsChildId);
	if(pElem)
	{
		pElem = pElem->getChildById(pcsSubChildId, true, true);
		if(pElem)
		{
			if(IS_STRING_EQUAL(pcsText, PROPERTY_NONE))
				pcsText = "";
			pElem->setText(pcsText);
			if(pOptColor)
				pElem->setAsColor(PropertyTextColor, *pOptColor);

			return pElem;
		}
	}

	return UIZoomWindowElement::setTextForChild(pcsChildId, pcsSubChildId, pcsText, pOptColor);
}
/*****************************************************************************/
UIElement* UIRichTextElement::setIsVisibleForChild(const char* pcsChildId, bool bIsVisible)
{
	UIElement* pOwnElem = getObjectById(pcsChildId);
	if(pOwnElem)
	{
		pOwnElem->setIsVisible(bIsVisible);
		return pOwnElem;
	}
	else
		return UIZoomWindowElement::setIsVisibleForChild(pcsChildId, bIsVisible);
}
/*****************************************************************************/
bool UIRichTextElement::applyMouseWheelDelta(FLOAT_TYPE fDeltaMulted)
{
	// If this asserts, we likely forgot to add a related scroll bar (slider).
	UISliderElement* pSlider = getRelatedSlider(NULL);
	if(!pSlider)
		ASSERT_RETURN_FALSE;

	SVector2D svPan;
	getPan(svPan);

	svPan.y -= fDeltaMulted;
	if(svPan.y < 0)
		svPan.y = 0;

	setPan(svPan);

	if(pSlider && pSlider->getIsEnabled())
	{
		FLOAT_TYPE fMaxValue = pSlider->getMaxValue();
		if(svPan.y > fMaxValue)
			svPan.y = fMaxValue;

		pSlider->setValue(svPan.y);
	}

	return true;

	/*
	LinkedToSelfElemIterator sli = linkedToSelfElemIteratorFirst(pParent);
	if(!sli.isEnd())
	{
		UISliderElement* pSlider = as<UISliderElement>(sli.getItem());
		_ASSERT(pSlider);
		if(pSlider)
		{
			FLOAT_TYPE fMaxValue = pSlider->getMaxValue();
			if(svPan.y > fMaxValue)
				svPan.y = fMaxValue;

			pSlider->setValue(svPan.y);
		}
	}
	*/
}
/*****************************************************************************/
void UIRichTextElement::updateRelatedSlider()
{
	// Find one. 
	UISliderElement* pSlider = getRelatedSlider(NULL);
	if(!pSlider)
		return;

	// Otherwise, keep the slider between 0 and 1, and adjust our scroll.
	SVector2D svOwnSize;
	getBoxSize(svOwnSize);
	int iMaxScroll = myTotalHeight - svOwnSize.y;

	SVector2D svPan;
	getPan(svPan);

	FLOAT_TYPE fSliderVal;
	if(iMaxScroll <= 0)
	{
		pSlider->setIsEnabled(false);
		pSlider->setMaxValue(1.0);
		fSliderVal = 0;
	}
	else
	{
		pSlider->setIsEnabled(true);
		pSlider->setMaxValue(iMaxScroll);
		fSliderVal = svPan.y;
		//fSliderVal = svPan.y/(FLOAT_TYPE)iMaxScroll;
	}

/*
	if(fSliderVal < 0)
		fSliderVal = 0;
	else if(fSliderVal > 1.0)
		fSliderVal = 1.0;
*/

	pSlider->setValue(fSliderVal);
}
/*****************************************************************************/
void UIRichTextElement::scrollOnSlider(UISliderElement* pSlider)
{
	SVector2D svPan;
	getPan(svPan);
	svPan.y = pSlider->getValue();
	setPan(svPan);
}
/*****************************************************************************/
void UIRichTextElement::getEntireText(string& strOut)
{
	strOut = "";
	int iObj, iNumObj = myObjects.size();
	for(iObj = 0; iObj < iNumObj; iObj++)
	{
		if(myObjects[iObj]->myType == RichObjectText)
		{
			strOut += myObjects[iObj]->myString;
			strOut += "\n\n";
		}
	}
}
/*****************************************************************************/
};