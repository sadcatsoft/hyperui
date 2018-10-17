#pragma once

/*****************************************************************************/
class HYPERUI_API UIExpressionTextFieldElement : public UITextFieldElement
{
public:
	DECLARE_STANDARD_UIELEMENT_NO_CONSTRUCTOR_DESTRUCTOR_DEF(UIExpressionTextFieldElement, UiElemExpressionTextField);

	virtual void postInit();

	void clearExpression();
	virtual void onDeallocated();
	virtual void setText(const char* pcsText);

	virtual void setExpression(const char* pcsText);

	virtual void getTextAsString(string &strOut, const char* pcsOptFloatConvFormat = NULL);
	virtual FLOAT_TYPE getTextAsNumber();
	virtual const char* getText();
	virtual void getTextAsStringOrExpression(string &strOut, const char* pcsOptFloatConvFormat = NULL);

	virtual void onGainedFocus();

	// Do not make this non-const! Only use setUiText to change the expression.
	inline const IExpression* getExpression() const { return myExpression; }
	void copyExpressionContext(const IExpressionContext* pSrcContext);

	inline bool getHaveValidExpression() const { return myExpression != NULL; }

	virtual bool getAllowTextCaching() const { return !getHaveValidExpression(); }

protected:
	void updateExpressionOnTextChange(const char* pcsOptTextIn);

private:
	IExpression* myExpression;
};
/*****************************************************************************/