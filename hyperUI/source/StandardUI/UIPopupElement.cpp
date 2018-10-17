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

namespace HyperUI
{
/*****************************************************************************/
UIPopupElement::UIPopupElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIPopupElement::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
}
/*****************************************************************************/
void UIPopupElement::onPostUpdateChildData()
{
	UIElement::onPostUpdateChildData();

	// See the size of our contents
	SVector2D svContSize;
	if(!this->getContentSize(svContSize))
		return;

	// Now, resize us to fit.
	this->setNumProp(PropertyWidth, svContSize.x);
	this->setNumProp(PropertyHeight, svContSize.y);
	this->resetEvalCache(true);
	this->adjustToScreen();
}
/*****************************************************************************/
void UIPopupElement::postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	UIElement::postRender(svScroll, fOpacity, fScale);

	// Draw our nose
	if(this->doesPropertyExist(PropertyObjThirdAnim))
	{
		SideType eSide = mapStringToType(this->getStringProp(PropertyPopupPosition), g_pcsSideStrings, SideTop);
		SVector2D svOffset, svNoseOffset;
		svNoseOffset.x = this->getNumProp(PropertyObjThirdAnimOffsetX);
		svNoseOffset.y = this->getNumProp(PropertyObjThirdAnimOffsetY);

		SVector2D svDirMult(1,1);
		SVector2D svOwnSize;
		this->getBoxSize(svOwnSize);
		SVector2D svOwnPos;
		this->getGlobalPosition(svOwnPos);

		UIPopupElement::drawNose(getDrawingCache(), this->getStringProp(PropertyObjThirdAnim), svOwnPos, svOwnSize, svNoseOffset, fOpacity, fScale, eSide);
/*
		FLOAT_TYPE fDegRotation = 0;
		if(eSide == SideTop || eSide == SideBottom)
		{
			//svOffset.x += svOwnSize.x/2.0;
			if(eSide == SideTop)
			{
				svOffset.y += svOwnSize.y/2.0;
				fDegRotation = 180;
				svDirMult.y = -1;
			}
			else
			{
				svOffset.y -= svOwnSize.y/2.0;
				fDegRotation = 0;
			}
		}
		else if(eSide == SideLeft || eSide == SideRight)
		{
			//svOffset.y += svOwnSize.y/2.0;
			if(eSide == SideRight)
			{
				svOffset.x += svOwnSize.x/2.0;
				fDegRotation = 90;
			}
			else
			{
				svOffset.x -= svOwnSize.x/2.0;
				fDegRotation = 270;
				svDirMult.x = -1;
			}
		}
		ELSE_ASSERT;

		svOffset += svOwnPos;
		svOffset.x += svNoseOffset.x*svDirMult.x;
		svOffset.y += svNoseOffset.y*svDirMult.y;
		getDrawingCache()->addSprite(this->getStringProp(PropertyObjThirdAnim), svOffset.x, svOffset.y, fOpacity, fDegRotation, fScale, 1.0, true);
		*/
	}
}
/*****************************************************************************/
void UIPopupElement::drawNose(DrawingCache* pDrawingCache, const char* pcsAnim, const SVector2D& svOwnPos, const SVector2D& svOwnSize, const SVector2D& svNoseOffset, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, SideType eSide)
{
	SVector2D svDirMult(1,1);
	SVector2D svOffset;
	FLOAT_TYPE fDegRotation = 0;
	if(eSide == SideTop || eSide == SideBottom)
	{
		if(eSide == SideTop)
		{
			svOffset.y += svOwnSize.y/2.0;
			fDegRotation = 180;
			svDirMult.y = -1;
		}
		else
		{
			svOffset.y -= svOwnSize.y/2.0;
			fDegRotation = 0;
		}
	}
	else if(eSide == SideLeft || eSide == SideRight)
	{
		if(eSide == SideRight)
		{
			svOffset.x += svOwnSize.x/2.0;
			fDegRotation = 90;
		}
		else
		{
			svOffset.x -= svOwnSize.x/2.0;
			fDegRotation = 270;
			svDirMult.x = -1;
		}
	}
	ELSE_ASSERT;

	svOffset += svOwnPos;
	svOffset.x += svNoseOffset.x*svDirMult.x;
	svOffset.y += svNoseOffset.y*svDirMult.y;
	pDrawingCache->addSprite(pcsAnim, svOffset.x, svOffset.y, fOpacity, fDegRotation, fScale, 1.0, true);
}
/*****************************************************************************/
void UIPopupElement::getTotalPadding(SVector2D& svPaddingOut)
{
	int iCornerW, iCornerH;
	STRING_TYPE strFullAnimName;
	this->getFullTopAnimName(strFullAnimName);
	getTextureManager()->getCornerSize(strFullAnimName.c_str(), iCornerW, iCornerH);
	svPaddingOut.set(iCornerW, iCornerH);

	svPaddingOut.x += this->getNumProp(PropertyUioPaddingHor)*2.0;
	svPaddingOut.y += this->getNumProp(PropertyUioPaddingVert)*2.0;
}
/*****************************************************************************/
bool UIPopupElement::getContentSize(SVector2D& svOut)
{
	UIElement* pElem = NULL;
	if(this->doesPropertyExist(PropertyUioMeasureSource))
		pElem = this->getElementAtPath(this->getStringProp(PropertyUioMeasureSource));

	if(!pElem)
		pElem = this;

	if(!pElem->doesPropertyExist(PropertyText))
		return false;
	
	pElem->getTextAsString(theCommonString);
	RenderUtils::measureTextRough(getParentWindow(), theCommonString.c_str(), pElem->getCachedFont(), pElem->getCachedFontSize(), pElem->getTextWidth(), svOut);
	SVector2D svPadding;
	getTotalPadding(svPadding);
	svOut += svPadding;

	ResourceItem* pDefItem = this->getOwnDefinition();
	if(pDefItem)
	{
		SVector2D svMinSize;
		svMinSize.x = pDefItem->getNumProp(PropertyWidth);
		svMinSize.y = pDefItem->getNumProp(PropertyHeight);

		if(svMinSize.x > 0 && svOut.x < svMinSize.x)
			svOut.x = svMinSize.x;
		if(svMinSize.y > 0 && svOut.y < svMinSize.y)
			svOut.y = svMinSize.y;
	}

	return true;
}
/*****************************************************************************/
};