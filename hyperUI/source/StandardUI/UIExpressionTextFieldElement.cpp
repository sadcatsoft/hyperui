#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UIExpressionTextFieldElement::UIExpressionTextFieldElement(UIPlane* pParentPlane)
	: UITextFieldElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
UIExpressionTextFieldElement::~UIExpressionTextFieldElement()
{
	onDeallocated();
}
/*****************************************************************************/
void UIExpressionTextFieldElement::onAllocated(IBaseObject* pData)
{
	UITextFieldElement::onAllocated(pData);

	myExpression = NULL;
}
/*****************************************************************************/
void UIExpressionTextFieldElement::onDeallocated()
{
	UITextFieldElement::onDeallocated();
	clearExpression();
}
/*****************************************************************************/
void UIExpressionTextFieldElement::clearExpression()
{
	if(myExpression)
		delete myExpression;
	myExpression = NULL;
}
/*****************************************************************************/
void UIExpressionTextFieldElement::updateExpressionOnTextChange(const char* pcsOptTextIn)
{
	// Get our text:
	string strText;

	if(pcsOptTextIn)
		strText = pcsOptTextIn;
	else
		// Careful! This might cause extra evals and just revert to expression as text:
		UITextFieldElement::getTextAsStringOrExpression(strText);

	const CHAR_TYPE* pcsOldType = EMPTY_STRING;
	if(myExpression)
		pcsOldType = myExpression->getType();

	const CHAR_TYPE* pcsNewType = ExpressionTypeManager::getInstance()->getExpressionType(strText.c_str());

	if(!IS_STRING_EQUAL(pcsOldType, pcsNewType))
	{
		clearExpression();
		myExpression = IExpression::createExpressionFrom(strText.c_str());
	}
	else if(myExpression)
		myExpression->updateWithText(strText.c_str(), false);

	_ASSERT( ( !IS_VALID_STRING_AND_NOT_NONE(pcsNewType) && !myExpression)
		|| ( IS_VALID_STRING_AND_NOT_NONE(pcsNewType) && myExpression) );

	/*
	ExpressionType eOldType = ExpressionLastPlaceholder;
	if(myExpression)
		eOldType = myExpression->getType();

	ExpressionType eNewType = IExpression::getExpressionType(strText.c_str());

	if(eOldType != eNewType)
	{
		clearExpression();
		myExpression = IExpression::createExpressionFrom(strText.c_str());
	}
	else if(myExpression)
		myExpression->updateWithText(strText.c_str(), false);

	_ASSERT( (eNewType == ExpressionLastPlaceholder && !myExpression)
		|| (eNewType != ExpressionLastPlaceholder && myExpression) );
		*/

	setTopAnim(myExpression ? "exprTextBox" : "textBox");
	recomputeShortenedString();
}
/*****************************************************************************/
void UIExpressionTextFieldElement::postInit()
{
	UITextFieldElement::postInit();
	updateExpressionOnTextChange(NULL);
}
/*****************************************************************************/
void UIExpressionTextFieldElement::setText(const char* pcsText)
{
	UITextFieldElement::setText(pcsText);
	updateExpressionOnTextChange(pcsText);
}
/*****************************************************************************/
void UIExpressionTextFieldElement::getTextAsString(string &strOut, const char* pcsOptFloatConvFormat)
{
	if(myExpression)
		strOut = getText();
	else
		UITextFieldElement::getTextAsString(strOut, pcsOptFloatConvFormat);
}
/*****************************************************************************/
FLOAT_TYPE UIExpressionTextFieldElement::getTextAsNumber()
{
	if(myExpression)
		return atof(getText());
	else
		return UITextFieldElement::getTextAsNumber();
}
/*****************************************************************************/
const char* UIExpressionTextFieldElement::getText()
{
	if(myExpression)
	{
		const char* pcsStringRes = myExpression->getResultAsString();

		// See if we have an error
		UIElement* pChild = getParent<UIElement>()->setIsVisibleForChild("defErrorIcon", myExpression->getHaveError());
		if(pChild && myExpression->getHaveError())
			pChild->setStringProp(PropertyTooltip, myExpression->getLastError());

		// If we're numeric, update our numeric result
		if(getIsNumeric())
			updateNumericValue(atof(pcsStringRes));
		return pcsStringRes;
	}
	else
		return UITextFieldElement::getText();
}
/*****************************************************************************/
void UIExpressionTextFieldElement::onGainedFocus()
{
	if(!myExpression)
	{
		UITextFieldElement::onGainedFocus();
		return;
	}

	UIElement* pEditWindow = getUIPlane()->getElementById("editExpressionWindow");
	UITextFieldElement* pTextBox = pEditWindow->getChildById<UITextFieldElement>("editExpressionTextBox");

	pTextBox->setGenericDataSource(this->getStringProp(PropertyId));

	// Try to get size of us, and of our top element
	UIElement* pTopMostParent = this->getTopmostParent<UIElement>();
	UIElement* pParentTabHolder = this->getParentOfType<UITabWindowElement>();
	UIElement* pDialogChild = pTopMostParent ? pTopMostParent->getChildById("defDialogWindow") : NULL;

	SRect2D srParentRect;
	if(pParentTabHolder)
		pParentTabHolder->getGlobalRectangle(srParentRect);
	else if(pDialogChild)
		pDialogChild->getGlobalRectangle(srParentRect);
	else if(pTopMostParent)
		pTopMostParent->getGlobalRectangle(srParentRect);
	ELSE_ASSERT;

	// Get our own global rect, too:
	SRect2D srOwnRect;
	this->getGlobalRectangle(srOwnRect);

	// Now, figure out our size and center for the popup window:
	SVector2D svCenter, svSize;
	svCenter.x = srParentRect.center().x;
	svCenter.y = srOwnRect.center().y;
	svSize.x = srParentRect.w;
	svSize.y = pEditWindow->getNumProp(PropertyHeight);

	SRect2D srWindowRect;
	srWindowRect.setFromCenterAndSize(svCenter, svSize);
	pEditWindow->setWindowRect(srWindowRect);

	pTextBox->setText(myExpression->getRawExpressionContentsOnly());

	// If we have an expression, pop up the editor
	getUIPlane()->showUI("uilEditExpressionWindow");
	
	this->setDisableTextUpdate(true);
	getParentWindow()->setFocusElement(pTextBox);
	this->setDisableTextUpdate(false);
}
/*****************************************************************************/
void UIExpressionTextFieldElement::getTextAsStringOrExpression(string &strOut, const char* pcsOptFloatConvFormat)
{
	if(myExpression)
		myExpression->getRawExpressionWithPrefix(strOut);
	else
		UITextFieldElement::getTextAsStringOrExpression(strOut, pcsOptFloatConvFormat);
}
/*****************************************************************************/
void UIExpressionTextFieldElement::setExpression(const char* pcsText)
{
	this->setText(pcsText);
	// We must do this, because if we're a numeric text
	// entry, we might avoid calling this, and marking
	// the whole thing dirty in the base level.
	this->UIElement::setText(pcsText);
	this->updateText();
}
/*****************************************************************************/
void UIExpressionTextFieldElement::copyExpressionContext(const IExpressionContext* pSrcContext)
{
	myExpression->setContext(pSrcContext ? pSrcContext->cloneSelf() : NULL);
}
/*****************************************************************************/
};