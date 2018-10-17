#pragma once

#define SCISSOR_RECT_PADDING		-upToScreen(2.0)
#define FILE_SELECTED_ACTION		"fileSelectedAction"

class UndoBlock;
/*****************************************************************************/
class HYPERUI_API UITextFieldElement : public UIElement
{
public:

	DECLARE_STANDARD_UIELEMENT_NO_CONSTRUCTOR_DESTRUCTOR_DEF(UITextFieldElement, UiElemTextField);
	virtual void onDeallocated();

	virtual void onPressed(TTouchVector& vecTouches);
	virtual void onMouseMove(TTouchVector& vecTouches);
	virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);
	virtual void onTimerTick(GTIME lGlobalTime);
	virtual void onDoubleClick(TTouchVector& vecTouches, bool bIgnoreActions);

	virtual void changeValueTo(FLOAT_TYPE fAmount, UIElement* pOptSourceElem, bool bAnimate, bool bIsChangingContinuously);
	virtual void postInit();

	virtual void setText(const char* pcsText);
	//virtual void handleTargetElementUpdate();

	virtual void onGainedFocus();
	virtual void onLostFocus(bool bHasCancelled);

	virtual bool getAllowKeepingFocus() { return true; }
	virtual bool getAllowKeyboardEntry() { return true; }

	void setNumericMinValue(FLOAT_TYPE fVal) { myNumMinValue = fVal; }
	void setNumericMaxValue(FLOAT_TYPE fVal) { myNumMaxValue = fVal; }

	virtual void setNumericValue(FLOAT_TYPE fValue, UnitType eUnits, bool bIsChangingContinuously = false);
	virtual FLOAT_TYPE getNumericValue(UnitType eUnits, FLOAT_TYPE fDpi = FLOAT_TYPE_MAX);
	bool getIsNumeric();
	
	virtual bool onKeyUp(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl);
	virtual bool onTextInput(int iKey);

	void switchToUnits(UnitType eNewUnits, FLOAT_TYPE fDpi);
	void updateLinkedElement(bool bAnimate, bool bIsChangingContinuously);

	inline UnitType getCurrentUnits() const { return myNumValue.getUnits(); }
	void selectAll();

	void setCursorPosFromScreenPoint(const SVector2D& svPoint);
	void scrollOnSlider(UISliderElement* pSlider);

	void setCursorToEnd();
	void scrollToEnd();

	void updateText();

protected:

	void scrolledToScreenSpace(const SVector2D& svPointIn, SVector2D& svPointOut);
	void screenToScrolledSpace(const SVector2D& svPointIn, SVector2D& svPointOut);

	virtual FLOAT_TYPE getCursorOpacity(void);
	virtual bool onTextPreRender(string& strText, SColor& scolText, int &iCursorPosOut);
	virtual void updateOwnData(SUpdateInfo& rRefreshInfo);

	void clampNumericToBounds();
	virtual bool canAccept(int iKey);

	void incrementCurrentUiTextBy(FLOAT_TYPE fAmount);

	virtual void onTextContentsChangedByTyping() { }
	virtual void onCursorPosChangedByTyping() { }

	int getRawCursorPos() const { return myCursorPos; }
	virtual FLOAT_TYPE getMinCursorOpacity() { return 0.45; }

	//int getAdjustedCursorPos() const { if(myCutStartPosition > 0) return myCursorPos - myCutStartPosition; else return myCursorPos; }
	//void setAdjustedCursorPos(int iValue) { if(myCutStartPosition > 0) iValue += myCutStartPosition; myCursorPos = iValue; }

	void invalidateCursorPos() { myCursorPos = -1; }
	void setCursorPos(int iPos) { myCursorPos = iPos; }

	virtual void getRelativeCursorPos(SVector2D& svOut);

	inline SMeasuredString* getMeasuredString() { return myMeasuredString; }

	void clearSelection() { mySelectionStartPos = mySelectionEndPos = -1; }
	virtual void renderSelection(const SVector2D& svTextPos, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, bool bXorMode);

	void deleteSelectedText();
	virtual bool getHaveSelection() const { return mySelectionStartPos >= 0; }

	int getCharPosFromScreenPoint(const SVector2D& svScreenPoint, bool bClampToTextLength);

	void manageSelectionWhenCursorMovesLeft(int iOldPos);
	void manageSelectionWhenCursorMovesRight(int iOldPos);

	virtual bool getAllowTextShortening() const { return true; }
	virtual bool getAllowTextScrolling() const { return false; }

	void recomputeShortenedString();

	virtual void getScrolledTextPosition(SVector2D& svInOut) const;

	bool getIsReadOnly() const;
	void updateLinkedSlider();
	UISliderElement* getRelatedSlider();

	virtual bool getAllowValuePropagation(FLOAT_TYPE fNewValue, bool bIsChangingContinuously, UIElement* pOptSourceElem, UIElement* pLinkedToElem);

	FLOAT_TYPE getMaxTextScroll();
	void ensureMeasuredStringUpdated();
	virtual void onSizeChanged();

	void ensureScrollToMakeCursorVisible();
	void protectString(string& strTextInOut);

	virtual bool getAllowTextCutting() { return true; }

protected:
	SVector2D myTextScroll;

	void setDisableTextUpdate(bool bValue) { myDisableTextUpdate = bValue; }

	void updateNumericValue(FLOAT_TYPE fValue);

	virtual const char* getFontForMeasuredString() { return getCachedFont(); }

private:
	AnimatedValue myCursorAnim;

	// Doesn't exit by default, but if it does, replaces the 
	// text.
	//FLOAT_TYPE myNumValue;
	SUnitNumber myNumValue;
	FLOAT_TYPE myNumMinValue, myNumMaxValue;

	UNIQUEID_TYPE myCurrUndoBlockId;

	// Note that this cursor is a position in the wrapped coordinates...
	int myCursorPos;
	bool myIsEnteringText;

	SMeasuredString *myMeasuredString;

	int mySelectionStartPos, mySelectionEndPos;

	int myMouseDragStartCharPos;
	bool myDidLockMouseCursor;

	string myShortenedString;
	// Start index of the cut from the front
	// Relative to the original string.
	// This is also not counting the elipsis.
	int myCutStartPosition;
	SVector2D myElipsisDims;
	SVector2D myCutPortionSize;

	bool myDisableTextUpdate;
};
/*****************************************************************************/