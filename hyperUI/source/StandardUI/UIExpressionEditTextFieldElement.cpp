#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UIExpressionEditTextFieldElement::UIExpressionEditTextFieldElement(UIPlane* pParentPlane)
	: UITextFieldElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
UIExpressionEditTextFieldElement::~UIExpressionEditTextFieldElement()
{
	onDeallocated();
}
/*****************************************************************************/
void UIExpressionEditTextFieldElement::onAllocated(IBaseObject* pData)
{
	UITextFieldElement::onAllocated(pData);
	setForceDrawUncachedText(true);
}
/*****************************************************************************/
void UIExpressionEditTextFieldElement::onLostFocus(bool bHasCancelled)
{
	UITextFieldElement::onLostFocus(bHasCancelled);

	// Now, see what we set back
	UIExpressionTextFieldElement* pTargetElem = getUIPlane()->getElementById<UIExpressionTextFieldElement>(this->getGenericDataSource());
	if(pTargetElem)
	{
		//const char* pcsPrefix = IExpression::getPrefixString(pTargetElem->getExpression()->getType());
		theCommonString = pTargetElem->getExpression()->getType();
		theCommonString += EXPRESSION_PREFIX_SEP;
		this->getTextAsString(theCommonString2);
		theCommonString += theCommonString2;

		pTargetElem->setExpression(theCommonString.c_str());
	}
	ELSE_ASSERT;

	// And hide our parent
	getUIPlane()->hideUI(this->getParent()->getStringProp(PropertyLayer));
}
/*****************************************************************************/
};