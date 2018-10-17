#pragma once

/*****************************************************************************/
class HYPERUI_API UIExpressionEditTextFieldElement : public UITextFieldElement
{
public:
	DECLARE_STANDARD_UIELEMENT_NO_CONSTRUCTOR_DESTRUCTOR_DEF(UIExpressionEditTextFieldElement, UiElemExpressionEditTextField);
	virtual void onLostFocus(bool bHasCancelled);
};
/*****************************************************************************/