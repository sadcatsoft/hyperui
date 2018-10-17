#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UIMultilineTextFieldElement::UIMultilineTextFieldElement(UIPlane* pParentPlane)
	: UITextFieldElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIMultilineTextFieldElement::onAllocated(IBaseObject* pData)
{
	UITextFieldElement::onAllocated(pData);
}
/*****************************************************************************/
bool UIMultilineTextFieldElement::canAccept(int iKey)
{
	if(UITextFieldElement::canAccept(iKey))
		return true;

	if(iKey == SilentKeyCarriageReturn)
		return true;

	return false;
}
/*****************************************************************************/
bool UIMultilineTextFieldElement::onKeyUp(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl)
{
	bool bHandled = UITextFieldElement::onKeyUp(iKey, bControl, bAlt, bShift, bMacActualControl);
	if(bHandled)
		return true;

	if(iKey == SilentKeyUpArrow || iKey == SilentKeyDownArrow)
	{
		// Try to find our pos:
		FLOAT_TYPE fLineHeight = getMeasuredString()->getMeasuredLineHeight(NULL);
		SVector2D svRelPos;
		getRelativeCursorPos(svRelPos);
		screenToScrolledSpace(svRelPos, svRelPos);
		if(iKey == SilentKeyUpArrow)
			svRelPos.y -= fLineHeight;
		else
			svRelPos.y += fLineHeight;
		//int iNewPos = getMeasuredString()->getCursorPosForRelativePoint(false, false, true, svRelPos);
		int iNewPos = getMeasuredString()->getCursorPosForRelativePoint(false, false, false, svRelPos);

		if(!bShift)
			clearSelection();

		if(iNewPos >= 0)
		{
			int iOldPos = getRawCursorPos();
			setCursorPos(iNewPos);


/*
			// If we're on the last line, scroll down:
			if(getAllowTextScrolling())
			{
				SRect2D srOwnRect;
				getGlobalRectangle(srOwnRect);
				srOwnRect.enlargeAllSidesBy(SCISSOR_RECT_PADDING);
				if(iKey == SilentKeyUpArrow)
				{
					if(svRelPos.y - myTextScroll.y <= fLineHeight)
						myTextScroll.y -= fLineHeight;
					if(myTextScroll.y < 0)
						myTextScroll.y = 0;
				}
				else
				{
					if(svRelPos.y >= srOwnRect.h)
						myTextScroll.y += fLineHeight;
				}
				updateLinkedSlider();
			}
*/
			ensureScrollToMakeCursorVisible();

			if(bShift && iNewPos != iOldPos)
			{
				if(iKey == SilentKeyUpArrow)
					manageSelectionWhenCursorMovesLeft(iOldPos);
				else
					manageSelectionWhenCursorMovesRight(iOldPos);
			}

			onCursorPosChangedByTyping();
			bHandled = true;
		}
	}

	return bHandled;
}
/*****************************************************************************/
void UIMultilineTextFieldElement::onTextWidthUpdated()
{
#ifdef _DEBUG
	if(IS_OF_TYPE("chatMainEntryCONSTEST"))
	{
		int bp = 0;
	}
#endif
	if(getMeasuredString())
		getMeasuredString()->setMaxWidth(getTextWidth());
}
/*****************************************************************************/
bool UIMultilineTextFieldElement::onTextPreRender(string& strText, SColor& scolText, int &iCursorPosOut)
{
	bool bRes = UITextFieldElement::onTextPreRender(strText, scolText, iCursorPosOut);

/*
// No - it's now in the adjusted space always.
	// Adjust the cursor for wrapped strings 
	if(getTextWidth() > 0 && getMeasuredString())
	{
		ensureMeasuredStringUpdated();
		iCursorPosOut = getMeasuredString()->getAdjustedCursorIndex();
	}
*/

	SRect2D srOwnRect;
	getGlobalRectangle(srOwnRect);
	srOwnRect.enlargeAllSidesBy(SCISSOR_RECT_PADDING);

	RenderUtils::beginScissorRectangle(srOwnRect, getDrawingCache());

	return bRes;
}
/*****************************************************************************/
void UIMultilineTextFieldElement::onTextPostRender()
{
	UITextFieldElement::onTextPostRender();
	RenderUtils::endScissorRectangle(getDrawingCache());
}
/*****************************************************************************/
bool UIMultilineTextFieldElement::applyMouseWheelDelta(FLOAT_TYPE fDeltaMulted)
{
	myTextScroll.y -= fDeltaMulted;
	if(myTextScroll.y < 0)
		myTextScroll.y = 0;
	FLOAT_TYPE fMaxScroll = getMaxTextScroll();
	if(myTextScroll.y > fMaxScroll)
		myTextScroll.y = fMaxScroll;
	updateLinkedSlider();
	return true;
}
/*****************************************************************************/
void UIMultilineTextFieldElement::modifyTextBeforeRender(SVector2D& svGlobalTextPos, string& strText)
{
	if(strText.length() == 0)
		return;

	// Try to cut our text to just what we see in the window. This is necessary because 
	// we may exceed the height of our off-screen buffer, and start drawing blanks
	// for larger texts, particularly in the console.
	SRect2D srGlobalRect;
	getGlobalRectangle(srGlobalRect);

	// If we're wrapping, grab the text from the measured string:
	if(getTextWidth() > 0 && getMeasuredString())
	{
		ensureMeasuredStringUpdated();
		getMeasuredString()->getWrappedString(NULL, strText);
	}

	int iLen = strText.length();
	if(iLen == 0)
		return;

	SVector2D svRelPointStart(0.0, srGlobalRect.y - svGlobalTextPos.y);
	SVector2D svRelPointEnd(0.0, srGlobalRect.y + srGlobalRect.h - svGlobalTextPos.y);

	SMeasuredString rBrokenString(getParentWindow(), OpenGLStringMeasurer::getInstance(), strText.c_str(), getCachedFont(), getTextWidth());
	int iStartPos = rBrokenString.getCursorPosForRelativePoint(true, true, true, svRelPointStart);
	int iEndPos = rBrokenString.getCursorPosForRelativePoint(true, true, true, svRelPointEnd);
	iStartPos = HyperCore::clampToRange(0, iLen - 1, iStartPos);
	iEndPos = HyperCore::clampToRange(0, iLen - 1, iEndPos);

	int iNumLines = rBrokenString.getNumLines();
	int iStartLine = rBrokenString.getLineIndexForPos(iStartPos);
	int iEndLine = rBrokenString.getLineIndexForPos(iEndPos);

	iStartLine = HyperCore::clampToRange(0, iNumLines - 1, iStartLine - 1);
	iEndLine = HyperCore::clampToRange(0, iNumLines - 1, iEndLine + 1);

	iStartPos = rBrokenString.getLineInfo(iStartLine)->myStartChar;
	iEndPos = rBrokenString.getLineInfo(iEndLine)->myEndChar;

	strText = strText.substr(iStartPos, iEndPos - iStartPos + 1);
	svGlobalTextPos.y += rBrokenString.getMeasuredLineHeight(NULL)*iStartLine;
}
/*****************************************************************************/
};