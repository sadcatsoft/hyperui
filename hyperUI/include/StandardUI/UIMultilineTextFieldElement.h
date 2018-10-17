#pragma once

/*****************************************************************************/
class HYPERUI_API UIMultilineTextFieldElement : public UITextFieldElement
{
public:
	DECLARE_STANDARD_UIELEMENT(UIMultilineTextFieldElement, UiElemMultilineTextField);
	virtual bool canAccept(int iKey);
	virtual bool onKeyUp(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl);

	virtual bool applyMouseWheelDelta(FLOAT_TYPE fDeltaMulted);
	virtual bool getAllowWheelScroll() const { return true; }

	virtual void modifyTextBeforeRender(SVector2D& svGlobalTextPos, string& strText);

	virtual void onTextWidthUpdated();

protected:

	virtual bool getAllowTextCutting() { return false; }

	// Disable, do our own wrapping
	virtual FLOAT_TYPE getTextWidthForRendering() const { return 0; }

	virtual bool getAllowTextShortening() const { return false; }
	virtual bool getAllowTextScrolling() const { return true; }
	virtual bool onTextPreRender(string& strText, SColor& scolText, int &iCursorPosOut);
	virtual void onTextPostRender();
};
/*****************************************************************************/