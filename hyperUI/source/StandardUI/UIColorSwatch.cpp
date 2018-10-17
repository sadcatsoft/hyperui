#include "stdafx.h"

#define COLOR_PADDING		upToScreen(0.5)

#define UITAG_RED		"redchannel"
#define UITAG_GREEN		"greenchannel"
#define UITAG_BLUE		"bluechannel"

namespace HyperUI
{
/*****************************************************************************/
UIColorSwatch::UIColorSwatch(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{

}
/*****************************************************************************/
void UIColorSwatch::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
	myClickAction = ClickActionOpenColorPicker;
	myColorCallback = NULL;
}
/*****************************************************************************/
void UIColorSwatch::render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
//	UIElement::render(svScroll, fOpacity, fScale);

	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity;

	if(this->getParent())
		svPos *= fScale;
	fScale *= fLocScale;

	SVector2D svCenter(svPos.x + svScroll.x, svPos.y + svScroll.y);
	SVector2D svSize;
	getBoxSize(svSize);
	svSize *= fLocScale*fScale;

	// Render our swatch.
	SColor svFinalColor(myColor);
	svFinalColor.alpha *= fFinalOpac;
	getDrawingCache()->addRectangle(svCenter.x - svSize.x/2.0 + COLOR_PADDING, svCenter.y - svSize.y/2.0 + COLOR_PADDING, svSize.x - COLOR_PADDING*2, svSize.y - COLOR_PADDING*2, svFinalColor);
	// Make sure we will be overwritten by our border...
	getDrawingCache()->flush();
	this->getFullTopAnimName(theCommonString);
	//getDrawingCache()->addSprite(theCommonString.c_str(), svCenter.x, svCenter.y, fFinalOpac, 0, fScale, fScale, 1.0, true);
	getDrawingCache()->addScalableButton(theCommonString.c_str(), svCenter.x, svCenter.y, svSize.x, svSize.y, fOpacity);

	renderChildren(svScroll, fOpacity, fScale*fLocScale);
}
/*****************************************************************************/
void UIColorSwatch::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{
	UIElement::onReleased(vecTouches, bIgnoreActions);
	if(getIsEnabled())
		simulateClick();
}
/*****************************************************************************/
void UIColorSwatch::simulateClick()
{
	if(myClickAction == ClickActionOpenColorPicker)
	{
		// Now, get the color:
		SColor scolNew;
		if(Application::getColorFromColorPicker(myColor, this->getStringProp(PropertyId), this->getParentWindow()->getId(), scolNew))
			changeColorFromUI(scolNew, false);
	}
	else if(myClickAction == ClickActionUpdateColorPickerIfOpened)
	{
		// If we're opened, update the color in the existing color picker and the element
		UIColorPicker* pPicker = UIPlane::getCurrentlyOpenColorPicker();
		if(pPicker)
		{
			pPicker->setColor(myColor);
		}
		else
		{
			SColor scolNew;
			if(Application::getColorFromColorPicker(myColor, this->getStringProp(PropertyId), this->getParentWindow()->getId(), scolNew))
				changeColorFromUI(scolNew, false);
		}
	}
	else if(myClickAction == ClickActionExecuteUiAction)
	{
		theCommonString3 = COLOR_SWATCH_CLICK_ACTION;
		handleActionUpParentChain(theCommonString3, false);
	}
	ELSE_ASSERT;
}
/*****************************************************************************/
void UIColorSwatch::setColor(const SColor& scolIn)
{
	myColor = scolIn;
	myColor.alpha = 1.0;
	updateLinkedElements(false);
}
/*****************************************************************************/
void UIColorSwatch::updateLinkedElements(bool bIsChangingContinuously)
{
	// See if we have siblings with our components.
	UITextFieldElement* pElem;
	UIElement* pParent = this->getParent<UIElement>();
	if(!pParent)
		return;

	pElem = pParent->findChildWithTag<UITextFieldElement>(UITAG_RED);
	if(pElem)
	{		
		StringUtils::numberToNiceString(SColor::convertFloatToChar(myColor.r), 0, false, theCommonString);
		pElem->setText(theCommonString.c_str());		
	}

	pElem = pParent->findChildWithTag<UITextFieldElement>(UITAG_GREEN);
	if(pElem)
	{		
		StringUtils::numberToNiceString(SColor::convertFloatToChar(myColor.g), 0, false, theCommonString);
		pElem->setText(theCommonString.c_str());		
	}

	pElem = pParent->findChildWithTag<UITextFieldElement>(UITAG_BLUE);
	if(pElem)
	{		
		StringUtils::numberToNiceString(SColor::convertFloatToChar(myColor.b), 0, false, theCommonString);
		pElem->setText(theCommonString.c_str());		
	}

	handleTargetElementUpdate(bIsChangingContinuously);
}
/*****************************************************************************/
void UIColorSwatch::changeValueTo(FLOAT_TYPE fAmount, UIElement* pOptSourceElem, bool bAnimate, bool bIsChangingContinuously)
{
	// Change our value first
	if(pOptSourceElem && !myIsCallingChangeValue)
	{
		if(pOptSourceElem->hasTag(UITAG_RED))
			myColor.r = fAmount/255.0;
		else if(pOptSourceElem->hasTag(UITAG_GREEN))
			myColor.g = fAmount/255.0;
		else if(pOptSourceElem->hasTag(UITAG_BLUE))
			myColor.b = fAmount/255.0;
	}

	UIElement::changeValueTo(fAmount, pOptSourceElem, bAnimate, bIsChangingContinuously);
}
/*****************************************************************************/
void UIColorSwatch::changeColorFromUI(const SColor& scolNew, bool bIsChangingContinuously)
{
	// Create an undo if we can
	UNIQUEID_TYPE lUndoBlockId = -1;
	UIElement* pParmElem = getLinkedToElementWithValidTarget();
	if(pParmElem && UndoManager::canAcceptNewUndoNow())
		lUndoBlockId = UndoManager::addUndoItemToCurrentManager(pParmElem->getUndoStringForSelfChange(), pParmElem->createUndoItemForSelfChange(), true, getParentWindow(), NULL);	

	myColor = scolNew;
	onColorChanged();
	if(myColorCallback)
		myColorCallback->onColorChanged(myColor, this);
	updateLinkedElements(bIsChangingContinuously);

	if(lUndoBlockId >= 0)
		UndoManager::endUndoBlockInCurrentManager(lUndoBlockId, getParentWindow(), NULL);

}
/*****************************************************************************
void UIColorSwatch::handleTargetElementUpdate()
{
	Node* pNode;
	StringResourceProperty* pParm = getTargetParm(pNode);
	if(!pParm)
		return;

	pParm->setAsColor(myColor);
	pNode->markCacheDirty();
	pNode->onParmChanged(pParm->getPropertyName());
}
/*****************************************************************************/
void UIColorSwatch::updateOwnData(SUpdateInfo& rRefreshInfo)
{
	// See if we have a target element, and if so, set our value from it.
	// We need this here because if we set the color parm ourselves, 
	// and call onParmChanged(), UI won't refresh unless we do this here.
	// The example is Photo Filter node.
	if(getHaveValidTargetElement())
		getTargetIdentifier()->refreshUIFromStoredValue(this);
}
/*****************************************************************************/
};